// ---------------------------------------------------------------------
// Copyright (C) 2015 Chris Garry
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
// ---------------------------------------------------------------------


#include "gif_write.h"
#include "pipp_utf8.h"
#include "lzw_compressor.h"

#include <QCoreApplication>
#include <QDebug>


c_gif_write::c_gif_write() :
    mp_gif_file(nullptr),
    p_last_image(nullptr),
    mp_colour_table(nullptr)
{
    // Header structure fixed fields
    m_gif_header.m_signature[0] = 'G';
    m_gif_header.m_signature[1] = 'I';
    m_gif_header.m_signature[2] = 'F';
    m_gif_header.m_version[0] = '8';
    m_gif_header.m_version[1] = '9';
    m_gif_header.m_version[2] = 'a';
    m_gif_header.m_pixel_aspect_ratio = 0;

    // Netscape extension structure fixed fields
    m_netscape_extension.m_extension_label = 0x21;
    m_netscape_extension.app_extension_label = 0xFF;
    m_netscape_extension.m_block_size = 0x0B;
    m_netscape_extension.m_app_identifier[0] = 'N';
    m_netscape_extension.m_app_identifier[1] = 'E';
    m_netscape_extension.m_app_identifier[2] = 'T';
    m_netscape_extension.m_app_identifier[3] = 'S';
    m_netscape_extension.m_app_identifier[4] = 'C';
    m_netscape_extension.m_app_identifier[5] = 'A';
    m_netscape_extension.m_app_identifier[6] = 'P';
    m_netscape_extension.m_app_identifier[7] = 'E';
    m_netscape_extension.m_app_auth_code[0] = '2';
    m_netscape_extension.m_app_auth_code[1] = '.';
    m_netscape_extension.m_app_auth_code[2] = '0';
    m_netscape_extension.m_sub_block_size = 0x03;
    m_netscape_extension.m_sub_block_id = 0x01;
    m_netscape_extension.m_block_terminator = 0x00;

    // Graphic control extension structure fixed fields
    m_graphic_control_extension.m_extension_label = 0x21;
    m_graphic_control_extension.m_graphic_control_label = 0xF9;
    m_graphic_control_extension.m_block_size = 0x04;
    m_graphic_control_extension.m_block_terminator = 0x00;

    // Image descriptor structure fixed fields
    m_image_descriptor.m_image_seperator = 0x2C;

    // Comment extension structure fixed fields
#ifdef GIF_COMMENT_STRING
    m_comment_extension.m_extension_label = 0x21;
    m_comment_extension.m_comment_label = 0xFE;
    m_comment_extension.m_comment_length = sizeof(GIF_COMMENT_STRING) - 1;
    strncpy((char *)m_comment_extension.m_comment_data, GIF_COMMENT_STRING, sizeof(GIF_COMMENT_STRING) - 1);
    m_comment_extension.m_block_terminator = 0x00;
#endif


    // Header structure variable fields
    m_gif_header.m_logical_screen_width[0] = 0;
    m_gif_header.m_logical_screen_width[1] = 0;
    m_gif_header.m_logical_screen_height[0] = 0;
    m_gif_header.m_logical_screen_height[1] = 0;
    m_gif_header.m_packed_fields = 0;
    m_gif_header.m_background_colour_index = 0;

    // Netscape extension variable fields
    m_netscape_extension.m_loop_count[0] = 0;
    m_netscape_extension.m_loop_count[1] = 0;

    // Graphic control extension structure variables fields
    m_graphic_control_extension.m_packed_field = 0;
    m_graphic_control_extension.m_delay_time[0] = 0;
    m_graphic_control_extension.m_delay_time[1] = 0;
    m_graphic_control_extension.m_transparent_colour_index = 0;

    // Image descriptor structure variable fields
    m_image_descriptor.m_image_left_position[0] = 0;
    m_image_descriptor.m_image_left_position[1] = 0;
    m_image_descriptor.m_image_top_position[0] = 0;
    m_image_descriptor.m_image_top_position[1] = 0;
    m_image_descriptor.m_image_width[0] = 0;
    m_image_descriptor.m_image_width[1] = 0;
    m_image_descriptor.m_image_height[0] = 0;
    m_image_descriptor.m_image_height[1] = 0;
    m_image_descriptor.m_packed_fields = 0;

    union u_test{
        uint8_t m_uint8[2];
        uint16_t m_uint16;
    };
}


// ------------------------------------------
// Create a new GIF file
// ------------------------------------------
bool c_gif_write::create(
        const QString &filename,
        int width,
        int height,
        int byte_depth,
        bool colour,
        int repeat_count)
{
    // Check for unsupported arguments and do early return if required
    if (colour) {
        qDebug() << "c_gif_write::create(): Colour images not supported yet";
        return false;
    }

    if (width > 0xFFFF || height > 0xFFFF) {
        qDebug() << "c_gif_write::create(): GIF size too large";
        return false;
    }

    if (byte_depth != 1 && byte_depth != 2) {
        qDebug() << "c_gif_write::create(): Only byte depths of 1 and 2 are supported";
        return false;
    }

    if (repeat_count > 0xFFFF) {
        qDebug() << "c_gif_write::create(): Repeat count greater then maximum";
        return false;
    }

    // Clear last image pointer
    p_last_image = nullptr;

    // Set member variables
    m_width = width;
    m_height = height;
    m_colour = colour;

    m_bytes_per_sample = byte_depth;
    if (colour) {
        m_bytes_per_sample *= 3;
    }

    // Open new GIF file
    mp_gif_file = fopen_utf8(filename.toUtf8().data(), "wb+");

    // Check file opened
    // Return if file did not open
    if (mp_gif_file == nullptr) {
        m_error_string += QCoreApplication::tr("Error: could not open file '%1' for writing", "GIF write file error message")
                          .arg(filename);
        m_error_string += "\n";
        return false;
    }

    // Update header structure variable fields now we have more information
    m_gif_header.m_logical_screen_width[0] = (uint8_t)(m_width & 0xFF);
    m_gif_header.m_logical_screen_width[1] = (uint8_t)(m_width >> 8);
    m_gif_header.m_logical_screen_height[0] = (uint8_t)(m_height & 0xFF);
    m_gif_header.m_logical_screen_height[1] = (uint8_t)(m_height >> 8);

    if (!m_colour) {
        // Header packed_fields byte for monochrome encoding
        m_gif_header.m_packed_fields  = 1 << 7;  // Global Color Table Flag
        m_gif_header.m_packed_fields |= 0x7 << 4;  // Color Resolution: 8-bits per pixel
        m_gif_header.m_packed_fields |= 0 << 3;  // Sort Flag: Not sorted
        m_gif_header.m_packed_fields |= 0x7 << 0;  // Size of Global Color Table: 256 entries
    }

    m_gif_header.m_background_colour_index = 0;  // We do not use background colour pixels as yet

    // Write GIF header to the file
    fwrite(&m_gif_header, 1, sizeof(m_gif_header), mp_gif_file);

    if (!m_colour) {
        // Create global colour table and write to the file
        mp_colour_table = new uint8_t[256 * 3];
        for (int i = 0; i < 256; i++) {
            mp_colour_table[i*3 + 0] = i;
            mp_colour_table[i*3 + 1] = i;
            mp_colour_table[i*3 + 2] = i;
        }

        fwrite(mp_colour_table, 1, 256 * 3, mp_gif_file);
        delete [] mp_colour_table;  // Only global colour table used for monochrome data
    }

    // Update Netscape extension and write to file
    // Netscape extension variable fields
    m_netscape_extension.m_loop_count[0] = (uint8_t)(repeat_count & 0xFF);
    m_netscape_extension.m_loop_count[1] = (uint8_t)(repeat_count >> 8);
    fwrite(&m_netscape_extension, 1, sizeof(m_netscape_extension), mp_gif_file);

    return true;
}


// ------------------------------------------
// Write frame to GIF file
// ------------------------------------------
bool c_gif_write::write_frame(
        uint8_t *p_data,
        uint16_t display_time)
{
    // Early return checks
    if (mp_gif_file == nullptr) {
        // No GIF file open
        return false;
    }

    if (p_data == nullptr) {
        // Bad data pointer
        return false;
    }

    // Scan top/bottom lines and left/right columns to check for lines/columns identical to previous frame
    // These areas do not need to be encoded.
    uint16_t x_start = 0;
    uint16_t x_end = m_width-1;
    uint16_t y_start = 0;
    uint16_t y_end = m_height-1;

    int bg_tolerance = 5;
    if (p_last_image != nullptr) {
        uint8_t *p_last_data = p_last_image;
        uint8_t *p_current_data = p_data;
        // Scan top lines
        bool mismatch = false;
        for (y_start = 0; y_start <= y_end; y_start++) {
            for (int x = 0; x < m_width; x++) {
                mismatch |= abs((int)(*p_current_data++) - (int)(*p_last_data++)) > bg_tolerance;
            }

            // Break out of loop on mismatch
            if (mismatch) {
                break;
            }
        }

        if (y_start > y_end) {
            // The 2 frames are exactly the same
            // Save a minimal sized image
            y_start = 0;
            y_end = 1;
            x_start = 0;
            x_end = 1;
        } else {
            // Scan bottom lines
            mismatch = false;
            for ( ; y_end > y_start; y_end--) {
                p_last_data = p_last_image + y_end * m_width;
                p_current_data = p_data + y_end * m_width;
                for (int x = 0; x < m_width; x++) {
                    mismatch |= abs((int)(*p_current_data++) - (int)(*p_last_data++)) > bg_tolerance;
                }

                // Break out of look on mismatch
                if (mismatch) {
                    break;
                }
            }

            // Scan left columns
            mismatch = false;
            for (x_start = 0; x_start < m_width; x_start++) {
                p_last_data = p_last_image + y_start * m_width + x_start;
                p_current_data = p_data + y_start * m_width + x_start;
                for (int y = y_start; y <= y_end; y++) {
                    mismatch |= abs((int)(*p_current_data) - (int)(*p_last_data)) > bg_tolerance;
                    p_current_data += m_width;
                    p_last_data += m_width;
                }

                // Break out of look on mismatch
                if (mismatch) {
                    break;
                }
            }

            // Scan right columns
            mismatch = false;
            for (; x_end > x_start; x_end--) {
                p_last_data = p_last_image + y_start * m_width + x_end;
                p_current_data = p_data + y_start * m_width + x_end;
                for (int y = y_start; y <= y_end; y++) {
                    mismatch |= abs((int)(*p_current_data) - (int)(*p_last_data)) > bg_tolerance;
                    p_current_data += m_width;
                    p_last_data += m_width;
                }

                // Break out of look on mismatch
                if (mismatch) {
                    break;
                }
            }
        }

        // At this point x_start, x_end, y_start and y_end should be updated to allow for
        // an unchanged border for this frame
    }

//    qDebug() << "(" << x_start << ", " << y_start << ") - (" << x_end << ", " << y_end << ")";

    // Update graphic control extension structure variables fields
    m_graphic_control_extension.m_packed_field  = 0 << 2;  // Disposal method: None specified
    m_graphic_control_extension.m_packed_field |= 0 << 1;  // User Input Flag: No user input expected
    m_graphic_control_extension.m_packed_field |= 0 << 0;  // Transparent colour flag: No transparent index is given
    m_graphic_control_extension.m_delay_time[0] = display_time & 0xFF;
    m_graphic_control_extension.m_delay_time[1] = display_time >> 8;
    m_graphic_control_extension.m_transparent_colour_index = 0;

    // Write graphic control extension to the file
    fwrite(&m_graphic_control_extension, 1, sizeof(m_graphic_control_extension), mp_gif_file);

    // Update image descriptor structure variable fields
    m_image_descriptor.m_image_left_position[0] = (uint8_t)(x_start & 0xFF);
    m_image_descriptor.m_image_left_position[1] = (uint8_t)(x_start >> 8);
    m_image_descriptor.m_image_top_position[0] = (uint8_t)(y_start & 0xFF);
    m_image_descriptor.m_image_top_position[1] = (uint8_t)(y_start >> 8);
    uint16_t active_width = 1 + x_end - x_start;
    m_image_descriptor.m_image_width[0] = (uint8_t)(active_width & 0xFF);
    m_image_descriptor.m_image_width[1] = (uint8_t)(active_width >> 8);
    uint16_t active_height = 1 + y_end - y_start;
    m_image_descriptor.m_image_height[0] = (uint8_t)(active_height & 0xFF);
    m_image_descriptor.m_image_height[1] = (uint8_t)(active_height >> 8);
    if (!m_colour) {
        // Image descriptor packed fileds byte for monochrome encoding
        m_image_descriptor.m_packed_fields  = 0 << 7;  // Local Color Table Flag - No local colour table for monochrome
        m_image_descriptor.m_packed_fields |= 0 << 6;  // Interlace flag - No interlacing
        m_image_descriptor.m_packed_fields |= 0 << 5;  // Sort flag - Colour table is not sorted
        m_image_descriptor.m_packed_fields |= 0 << 0;  // Size of local colour table
    }

    // Write image descriptor to the file
    fwrite(&m_image_descriptor, 1, sizeof(m_image_descriptor), mp_gif_file);

    if (m_colour) {
        // Create local colour table - Monochrome

        // Write local colour table to file
    }

    // Write LZW minimum code size to file
    fputc(0x08, mp_gif_file);

    // Compress image data and write to file
    bool all_compressed = false;
    c_lzw_compressor lzw_compressor(
                m_width,
                m_height,
                x_start,
                x_end,
                y_start,
                y_end,
                8,
                p_data);
    uint8_t *p_compress_buffer = new uint8_t[260];
    while (!all_compressed) {
        // Compress up to 255 byte block of data (256 with byte count header)
        all_compressed = lzw_compressor.compress_data(p_compress_buffer);

        // Write block to file
        fwrite(p_compress_buffer, 1, p_compress_buffer[0] + 1, mp_gif_file);
    }

    delete [] p_compress_buffer;


    // Keep this image data for processing the next frame
    if (p_last_image == nullptr) {
        // Create buffer if required
        p_last_image = new uint8_t[m_width * m_height];
    }

    // Copy image data into last frame buffer
    std::copy(p_data, p_data + m_width * m_height, p_last_image);

    // Write block terminator to file
    fputc(0x00, mp_gif_file);

    // Write comment block out if defined
#ifdef GIF_COMMENT_STRING
    fwrite(&m_comment_extension, 1, sizeof(m_comment_extension), mp_gif_file);
#endif

    return true;
}


// ------------------------------------------
// Finish and close GIF file
// ------------------------------------------
bool c_gif_write::close()
{
    if (p_last_image != nullptr) {
        delete [] p_last_image;
        p_last_image = nullptr;
    }

    if (mp_gif_file != nullptr) {
        // Write file terminator to file
        fputc(0x3B, mp_gif_file);

        // Close the file
        fclose(mp_gif_file);  // Close file
        mp_gif_file = nullptr;
    }

    return true;
}
