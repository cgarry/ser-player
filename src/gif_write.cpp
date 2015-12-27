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

#define NUMBER_COLOURS 256


c_gif_write::c_gif_write() :
    mp_gif_file(nullptr),
    m_open(false),
    mp_colour_table(nullptr),
    mp_mono_table(nullptr),
    mp_rev_mono_table(nullptr),
    mp_last_image(nullptr)
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
        int repeat_count,
        int unchanged_border_tolerance,
        bool use_transparent_pixels,
        int transparent_tolerence,
        int lossy_compression_level,
        int bit_depth)
{
    // Check for unsupported arguments and do early return if required
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
    mp_last_image = nullptr;

    // Set member variables
    m_width = width;
    m_height = height;
    m_colour = colour;

    m_bytes_per_sample = byte_depth;
    if (colour) {
        m_bytes_per_sample *= 3;
    }

    m_use_transparent_pixels = use_transparent_pixels;
    m_transparent_index = (use_transparent_pixels) ? 0 : -1;
    m_unchanged_border_tolerance = unchanged_border_tolerance;
    m_transparent_tolerence = transparent_tolerence;
    m_lossy_compression_level = lossy_compression_level;
    m_bit_depth = bit_depth;

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
        m_gif_header.m_packed_fields |= (m_bit_depth - 1) << 0;  // Size of Global Color Table: 256 entries
    } else {
        // Header packed fields byte for colour encoding
        m_gif_header.m_packed_fields  = 0 << 7;  // Global Color Table Flag
        m_gif_header.m_packed_fields |= 0x7 << 4;  // Color Resolution: 8-bits per pixel
        m_gif_header.m_packed_fields |= 0 << 3;  // Sort Flag: Not sorted
        m_gif_header.m_packed_fields |= 0 << 0;  // Size of Global Color Table: Not used
    }

    m_gif_header.m_background_colour_index = 0;  // We do not use background colour pixels as yet

    // Write GIF header to the file
    fwrite(&m_gif_header, 1, sizeof(m_gif_header), mp_gif_file);

    if (!m_colour) {
        const int colour_table_entries = 1 << m_bit_depth;

        // Create global colour table and write to the file
        uint8_t *p_global_colour_table = new uint8_t[colour_table_entries * 3];
        if (mp_mono_table != nullptr) {
            delete [] mp_mono_table;
        }

        mp_mono_table = new uint8_t[colour_table_entries];

        // Create monochrome LUT
        for (int i = 0; i < colour_table_entries; i++) {
            uint8_t table_value = (i << (8 - m_bit_depth)) | (i >> m_bit_depth);
            p_global_colour_table[i*3 + 0] = table_value;
            p_global_colour_table[i*3 + 1] = table_value;
            p_global_colour_table[i*3 + 2] = table_value;
            mp_mono_table[i] = table_value;
        }

        if (m_use_transparent_pixels) {
            // Make entry [1] == entry [0] to free entry 0 for transparency
            p_global_colour_table[3 + 0] = mp_mono_table[0];
            p_global_colour_table[3 + 1] = mp_mono_table[0];
            p_global_colour_table[3 + 2] = mp_mono_table[0];
            mp_mono_table[1] = mp_mono_table[0];
        }

        fwrite(p_global_colour_table, 1, colour_table_entries * 3, mp_gif_file);
        delete [] p_global_colour_table;  // Global colour table is only used for monochrome data

        // Create a reverse LUT from LUT
        mp_rev_mono_table = new uint8_t[256];
        for (int i = 0; i < 256; i++) {
            int best_error = 255;
            uint8_t best_index = 0;
            for (int j = 0; j < colour_table_entries; j++) {
                if (j == m_transparent_index) {
                    // Do not reverse look up to trnasparent index
                    continue;
                }

                int new_error = abs(i - mp_mono_table[j]);
                if (new_error == 0) {
                    // An exact match
                    best_index = j;
                    break;
                }

                if (new_error < best_error) {
                    // Best match so far
                    best_error = new_error;
                    best_index = j;
                }
            }

            mp_rev_mono_table[i] = best_index;
        }
    }

    // Update Netscape extension and write to file
    // Netscape extension variable fields
    m_netscape_extension.m_loop_count[0] = (uint8_t)(repeat_count & 0xFF);
    m_netscape_extension.m_loop_count[1] = (uint8_t)(repeat_count >> 8);
    fwrite(&m_netscape_extension, 1, sizeof(m_netscape_extension), mp_gif_file);

    m_open = true;
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

    //  Create buffer for indexed pixels image
    uint8_t *p_index_image = new uint8_t[m_width * m_height];

    // Scan top/bottom lines and left/right columns to check for lines/columns identical to previous frame
    // These areas do not need to be encoded.
    uint16_t x_start = 0;
    uint16_t x_end = m_width-1;
    uint16_t y_start = 0;
    uint16_t y_end = m_height-1;

    if (!m_colour) {
        // Monochorome data - convert values to indexed values
        uint8_t *p_current_data = p_data;
        for (int x = 0; x < m_width * m_height; x++) {
            *p_current_data = mp_rev_mono_table[*p_current_data];
            p_current_data++;
        }

        detect_unchanged_border(
            p_data,  // const uint8_t *p_this_image
            mp_last_image,  //const uint8_t *p_last_image
            x_start,  // uint16_t &x_start
            x_end,  // uint16_t &x_end
            y_start,  // uint16_t &y_start
            y_end);  // uint16_t &y_end
    } else {
        // Colour data
        detect_unchanged_border(
            p_data,  // const uint8_t *p_this_image
            mp_last_image,  //const uint8_t *p_last_image
            x_start,  // uint16_t &x_start
            x_end,  // uint16_t &x_end
            y_start,  // uint16_t &y_start
            y_end);  // uint16_t &y_end

        int num_colours = 256;
        if (m_use_transparent_pixels) {
            m_transparent_index = 0xFF;
            num_colours = 255;
        }

        // Colour data - convert values to indexed values
        quantize_colours(
            p_data,  // uint8_t *p_data
            x_start,  // uint16_t x_start,
            x_end,  // uint16_t x_end,
            y_start,  // uint16_t y_start,
            y_end,  // uint16_t y_end,
            num_colours);  // int number_of_colours


        // Buffer to keep image data for processing the next frame
        bool first_frame = false;
        if (mp_last_image == nullptr) {
            // Create buffer if required
            mp_last_image = new uint8_t[m_width * m_height * 3];
            first_frame = true;
        }

        //
        // Create an indexed version of the image including transparent pixels if required
        //
        // Create a buffer for the index image
        uint8_t *p_write_data = p_index_image;
        for (int y = y_start; y <= y_end; y++) {
            int x = x_start;
            uint8_t *p_current_data = p_data + (y * m_width + x) * 3;
            uint8_t *p_last_data = mp_last_image + (y * m_width + x) * 3;
            for ( ; x <= x_end; x++) {
                if (m_use_transparent_pixels && !first_frame) {
                    uint8_t b = *p_current_data++;
                    int diff = abs(b - (*p_last_data++));
                    uint8_t g = *p_current_data++;
                    diff += abs(g - (*p_last_data++));
                    uint8_t r = *p_current_data++;
                    diff += abs(r - (*p_last_data++));
                    if (diff <= m_transparent_tolerence) {
                        // This pixel is close enough to the previous pixel to be transparent
                        *p_write_data++ = m_transparent_index;
                        // Note that the last image pixel value is left unchanged
                    } else {
                        // This pixel is not transparent
                        *p_write_data++ = mp_rev_colour_table->r[r >> 3].g[g >> 3].b[b >> 3];
                        // Update last image pixel for comparison with the next frame
                        *(p_last_data - 3) = b;
                        *(p_last_data - 2) = g;
                        *(p_last_data - 1) = r;
                    }
                } else {  // Not using transparent pixels or first frame
                    uint8_t b = *p_current_data++;
                    uint8_t g = *p_current_data++;
                    uint8_t r = *p_current_data++;

                    // Write indexed data to buffer ready for compression
                    *p_write_data++ = mp_rev_colour_table->r[r >> 3].g[g >> 3].b[b >> 3];

                    // Write these pixels to last image buffer
                    *p_last_data++ = b;
                    *p_last_data++ = g;
                    *p_last_data++ = r;
                }
            }
        }
    }

    printf("Active area: (%d, %d) - (%d, %d)\n", x_start, x_end, y_start, y_end);


    //
    // Support for transparent pixels
    //
    if (m_use_transparent_pixels && !m_colour) {
        if (mp_last_image != nullptr) {
            // Replace unchanged pixels with transparent pixels ready for compression
            // Update last image with changed pixels
            for (int y = y_start; y <= y_end; y++) {
                uint8_t *p_last_data = mp_last_image + y * m_width + x_start;
                uint8_t *p_current_data = p_data + y * m_width + x_start;
                for (int x = x_start; x <= x_end; x++) {
                    //if (*p_current_data == *p_last_data) {
                    if (abs((int)(mp_mono_table[*p_current_data]) - (int)(mp_mono_table[*p_last_data])) <= m_transparent_tolerence) {
                        // The pixels are the same or close enough, use transparent pixel
                        *p_current_data = (uint8_t)m_transparent_index;
                    } else {
                        // The pixels are different, update pixel in last image buffer for next time
                        *p_last_data = *p_current_data;
                    }

                    p_current_data++;
                    p_last_data++;
                }
            }
        }
    }

    // Update graphic control extension structure variables fields
    m_graphic_control_extension.m_packed_field  = 0 << 2;  // Disposal method: None specified
    m_graphic_control_extension.m_packed_field |= 0 << 1;  // User Input Flag: No user input expected

    if (m_use_transparent_pixels && mp_last_image != nullptr) {
        m_graphic_control_extension.m_packed_field |= 1 << 0;  // Transparent colour flag
    }

    m_graphic_control_extension.m_delay_time[0] = display_time & 0xFF;
    m_graphic_control_extension.m_delay_time[1] = display_time >> 8;
    m_graphic_control_extension.m_transparent_colour_index = (uint8_t)m_transparent_index;

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
        // Image descriptor packed fields byte for monochrome encoding
        m_image_descriptor.m_packed_fields  = 0 << 7;  // Local Color Table Flag - No local colour table for monochrome
        m_image_descriptor.m_packed_fields |= 0 << 6;  // Interlace flag - No interlacing
        m_image_descriptor.m_packed_fields |= 0 << 5;  // Sort flag - Colour table is not sorted
        m_image_descriptor.m_packed_fields |= 0 << 0;  // Size of local colour table
    } else {
        // Image descriptor packed fields byte for colour encoding
        m_image_descriptor.m_packed_fields  = 1 << 7;  // Local Color Table Flag - Use local colour table for colour
        m_image_descriptor.m_packed_fields |= 0 << 6;  // Interlace flag - No interlacing
        m_image_descriptor.m_packed_fields |= 0 << 5;  // Sort flag - Colour table is not sorted
        m_image_descriptor.m_packed_fields |= 7 << 0;  // Size of local colour table
    }

    // Write image descriptor to the file
    fwrite(&m_image_descriptor, 1, sizeof(m_image_descriptor), mp_gif_file);

    if (m_colour) {
        // Write local colour table to file
        fwrite(mp_colour_table, 1, 256 * 3, mp_gif_file);
    }

    // Write LZW minimum code size to file
    fputc(m_bit_depth, mp_gif_file);

    // Compress image data and write to file
    bool all_compressed = false;

    c_lzw_compressor lzw_compressor(
                x_end - x_start + 1,  // m_width,
                y_end - y_start + 1,  // m_height
                0,  // x_start,
                x_end - x_start,  // x_end,
                0,  // y_start,
                y_end - y_start,  // y_end,
                m_bit_depth,
                p_index_image,
                m_lossy_compression_level,
                mp_mono_table,
                mp_rev_mono_table,
                m_transparent_index);

    uint8_t *p_compress_buffer = new uint8_t[260];
    while (!all_compressed) {
        // Compress up to 255 byte block of data (256 with byte count header)
        all_compressed = lzw_compressor.compress_data(p_compress_buffer);

        // Write block to file
        fwrite(p_compress_buffer, 1, p_compress_buffer[0] + 1, mp_gif_file);
    }

    // Data for this frame has been written to the file, free buffers
    delete [] p_index_image;
    delete [] p_compress_buffer;


    // Keep this image data for processing the next frame
    if (!m_colour) {
        if (mp_last_image == nullptr) {
            // Create buffer if required
            mp_last_image = new uint8_t[m_width * m_height];

            // Copy image data into last frame buffer
            std::copy(p_data, p_data + m_width * m_height, mp_last_image);
        } else if (!m_use_transparent_pixels) {
            // Copy image data into last frame buffer
            std::copy(p_data, p_data + m_width * m_height, mp_last_image);
        }
    }

    // Write block terminator to file
    fputc(0x00, mp_gif_file);

    return true;
}


// ------------------------------------------
// Finish and close GIF file
// ------------------------------------------
bool c_gif_write::close()
{
    if (mp_mono_table != nullptr) {
        delete [] mp_mono_table;
        mp_mono_table = nullptr;
    }

    if (mp_rev_mono_table != nullptr) {
        delete [] mp_rev_mono_table;
        mp_rev_mono_table = nullptr;
    }

    if (mp_last_image != nullptr) {
        delete [] mp_last_image;
        mp_last_image = nullptr;
    }

    // Write comment block out if defined
#ifdef GIF_COMMENT_STRING
    fwrite(&m_comment_extension, 1, sizeof(m_comment_extension), mp_gif_file);
#endif

    if (mp_gif_file != nullptr) {
        // Write file terminator to file
        fputc(0x3B, mp_gif_file);

        // Close the file
        fclose(mp_gif_file);  // Close file
        mp_gif_file = nullptr;
    }

    m_open = false;
    return true;
}


void c_gif_write::quantize_colours(
        uint8_t *p_data,
        uint16_t x_start,
        uint16_t x_end,
        uint16_t y_start,
        uint16_t y_end,
        int number_of_colours)
{
    uint32_t *p_histogram_lut = new uint32_t[0x8000]();
    uint32_t *p_index_lut = new uint32_t[0x8000];
    for (int x = 0; x < 0x8000; x++) {
        p_index_lut[x] = x;
    }

    // Create histogram
    for (int y = y_start; y <= y_end; y++) {
        int x = x_start;
        uint8_t *p_data_ptr = p_data + (y * m_width + x) * 3;
        for ( ; x <= x_end; x++) {
            uint8_t b = (*p_data_ptr++) >> 3;
            uint8_t g = (*p_data_ptr++) >> 3;
            uint8_t r = (*p_data_ptr++) >> 3;
            uint32_t index = (r << 10) | (g << 5) | (b << 0);
            p_histogram_lut[index]++;
        }
    }

    // Remove zero entries from table
    int number_of_hist_entries = 0;
    for (int x = 0; x < 0x8000; x++) {
        p_histogram_lut[number_of_hist_entries] = p_histogram_lut[x];
        p_index_lut[number_of_hist_entries] = p_index_lut[x];
        if (p_histogram_lut[number_of_hist_entries] > 0) {
            number_of_hist_entries++;
        }
    }

    printf("number_of_hist_entries: %d\n", number_of_hist_entries);

    // Sort LUTs by histogram value
    // Nasty bubble sort - to be replaced with a faster algorithm!
    bool did_swap;
    do {
        did_swap = false;
        for (int i = 0; i < number_of_hist_entries - 1; i++) {
            if (p_histogram_lut[i] < p_histogram_lut[i+1]) {
                uint32_t temp = p_histogram_lut[i];
                p_histogram_lut[i] = p_histogram_lut[i+1];
                p_histogram_lut[i+1] = temp;

                temp = p_index_lut[i];
                p_index_lut[i] = p_index_lut[i+1];
                p_index_lut[i+1] = temp;
                did_swap = true;
            }
        }
    } while (did_swap);

    // Delete histogram table as only the index table is required from here
    delete [] p_histogram_lut;

    // Create colour table and reverse colour table
    uint8_t *p_colour_r_palette = new uint8_t[256];
    uint8_t *p_colour_g_palette = new uint8_t[256];
    uint8_t *p_colour_b_palette = new uint8_t[256];
    mp_rev_colour_table = new s_rev_colour_index;

    int number_of_palette_colours = 0;
    bool colour_found = false;

    // Scan through index table
    int hist_entry;
    int colour_found_count = 0;
    for (hist_entry = 0; hist_entry < number_of_hist_entries; hist_entry++) {
        // Extract 5-bit RGB values from index table
        uint32_t temp = p_index_lut[hist_entry];
        uint8_t b = temp & 0x1F;
        temp >>= 5;
        uint8_t g = temp & 0x1F;
        temp >>= 5;
        uint8_t r = temp;

        // Check if this colour or a similar one is already in the colour palette
        // Similar means total difference in colours <= 2
        colour_found = false;
        int palette_entry;
        for (palette_entry = 0; palette_entry < number_of_palette_colours; palette_entry++) {
            int diff = abs((int)r - p_colour_r_palette[palette_entry]);
            diff += abs((int)g - p_colour_g_palette[palette_entry]);
            diff += abs((int)b - p_colour_b_palette[palette_entry]);

            if (diff <= 1) {
                // Close enough colour found in colour palette
                colour_found = true;
                break;
            }
        }

        if (colour_found) {
            // There was a similar colour already in the colour palette
            // Update the colour to index LUT for this colour
            mp_rev_colour_table->r[r].g[g].b[b] = (uint8_t)palette_entry;
            colour_found_count++;  // debug
        } else {
            // No similar colour was found in the colour palette
            // Add this colour to the colour palette
            p_colour_r_palette[number_of_palette_colours] = r;
            p_colour_g_palette[number_of_palette_colours] = g;
            p_colour_b_palette[number_of_palette_colours] = b;

            // Also update the colour to index LUT for this colour
            mp_rev_colour_table->r[r].g[g].b[b] = (uint8_t)number_of_palette_colours;

            // Increase palette colour count
            number_of_palette_colours++;

            if (number_of_palette_colours == number_of_colours) {
                // The maximum number of colours has been added to the colour palette
                // exit the loop
                hist_entry++;
                break;
            }
        }

    }

    printf("colour_found_count: %d\n", colour_found_count);
    printf("number_of_palette_colours: %d\n", number_of_palette_colours);
    printf("Remaining histogram entries: %d\n", number_of_hist_entries - hist_entry);

    // We have now filled the colour palette but need to find the closest values in the
    // colour palette for the remaining values in the histogram LUT to add to the
    // colour to index LUT
    for ( ; hist_entry < number_of_hist_entries; hist_entry++) {
        // Extract RGB values from index table
        uint32_t temp = p_index_lut[hist_entry];
        uint8_t b = temp & 0x1F;
        temp >>= 5;
        uint8_t g = temp & 0x1F;
        temp >>= 5;
        uint8_t r = temp;

        // Find the closest colour for this colour from the index table
        int best_diff = 0xFF;
        int best_entry = 0;
        for (int palette_entry = 0; palette_entry < number_of_palette_colours; palette_entry++) {
            int diff = abs((int)r - p_colour_r_palette[palette_entry]);
            diff += abs((int)g - p_colour_g_palette[palette_entry]);
            diff += abs((int)b - p_colour_b_palette[palette_entry]);

            if (diff < best_diff) {
                best_diff = diff;
                best_entry = palette_entry;
            }
        }

        // Update the colour to index LUT for this colour
        mp_rev_colour_table->r[r].g[g].b[b] = (uint8_t)best_entry;
    }

    // Delete index table as it has done its job in creating the colour palette and reverse colour palettes
    delete [] p_index_lut;

    // Create the real colour table
    // Update colour values to use all 8 bits rather than just 5 bits
    // The bottom 3 bits are just copies of the original top 3 bits
    if (mp_colour_table != nullptr) {
        delete [] mp_colour_table;
    }

    mp_colour_table = new uint8_t[3 * 256];
    uint8_t *p_colour_table_entry = mp_colour_table;
    for (int palette_entry = 0; palette_entry < number_of_palette_colours; palette_entry++) {
        uint8_t r_top_3_bits = p_colour_r_palette[palette_entry];
        uint8_t g_top_3_bits = p_colour_g_palette[palette_entry];
        uint8_t b_top_3_bits = p_colour_b_palette[palette_entry];
        r_top_3_bits >>= 2;
        g_top_3_bits >>= 2;
        b_top_3_bits >>= 2;
        *p_colour_table_entry++ = (int)(p_colour_r_palette[palette_entry] << 3) | r_top_3_bits;
        *p_colour_table_entry++ = (int)(p_colour_g_palette[palette_entry] << 3) | g_top_3_bits;
        *p_colour_table_entry++ = (int)(p_colour_b_palette[palette_entry] << 3) | b_top_3_bits;
    }

    // Free memory used for temp palettes
    delete [] p_colour_r_palette;
    delete [] p_colour_g_palette;
    delete [] p_colour_b_palette;

//    delete [] mp_colour_table;
}


void c_gif_write::detect_unchanged_border(
            const uint8_t *p_this_image,
            const uint8_t *p_last_image,
            uint16_t &x_start,
            uint16_t &x_end,
            uint16_t &y_start,
            uint16_t &y_end)
{
    if (!m_colour) {
        // Monochome version of the code
        if (p_last_image != nullptr) {  // Only currently supported for monochrome
            const uint8_t *p_last_data = p_last_image;
            const uint8_t *p_current_data = p_this_image;
            // Scan top lines
            bool mismatch = false;
            for (y_start = 0; y_start <= y_end; y_start++) {
                for (int x = 0; x < m_width; x++) {
                    mismatch |= abs((int)(mp_mono_table[*p_current_data++]) - (int)(mp_mono_table[*p_last_data++])) > m_unchanged_border_tolerance;
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
                    p_current_data = p_this_image + y_end * m_width;
                    for (int x = 0; x < m_width; x++) {
                        mismatch |= abs((int)(mp_mono_table[*p_current_data++]) - (int)(mp_mono_table[*p_last_data++])) > m_unchanged_border_tolerance;
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
                    p_current_data = p_this_image + y_start * m_width + x_start;
                    for (int y = y_start; y <= y_end; y++) {
                        mismatch |= abs((int)(mp_mono_table[*p_current_data]) - (int)(mp_mono_table[*p_last_data])) > m_unchanged_border_tolerance;
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
                    p_current_data = p_this_image + y_start * m_width + x_end;
                    for (int y = y_start; y <= y_end; y++) {
                        mismatch |= abs((int)(mp_mono_table[*p_current_data]) - (int)(mp_mono_table[*p_last_data])) > m_unchanged_border_tolerance;
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
    } else {
        // Colour version of code
        if (p_last_image != nullptr) {
            const uint8_t *p_last_data = p_last_image;
            const uint8_t *p_current_data = p_this_image;
            // Scan top lines
            bool mismatch = false;
            for (y_start = 0; y_start <= y_end; y_start++) {
                for (int x = 0; x < m_width; x++) {
                    int diff = abs((int)(*p_current_data++) - (*p_last_data++));
                    diff += abs((int)(*p_current_data++) - (*p_last_data++));
                    diff += abs((int)(*p_current_data++) - (*p_last_data++));
                    mismatch |= diff > m_unchanged_border_tolerance;
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
                    p_last_data = p_last_image + y_end * m_width * 3;
                    p_current_data = p_this_image + y_end * m_width * 3;
                    for (int x = 0; x < m_width; x++) {
                        int diff = abs((int)(*p_current_data++) - (*p_last_data++));
                        diff += abs((int)(*p_current_data++) - (*p_last_data++));
                        diff += abs((int)(*p_current_data++) - (*p_last_data++));
                        mismatch |= diff > m_unchanged_border_tolerance;
                    }

                    // Break out of look on mismatch
                    if (mismatch) {
                        break;
                    }
                }

                // Scan left columns
                mismatch = false;
                for (x_start = 0; x_start < m_width; x_start++) {
                    p_last_data = p_last_image + (y_start * m_width + x_start) * 3;
                    p_current_data = p_this_image + (y_start * m_width + x_start) * 3;
                    for (int y = y_start; y <= y_end; y++) {
                        int diff = abs((int)(*p_current_data) - (*p_last_data));
                        diff += abs((int)(*p_current_data) - (*p_last_data));
                        diff += abs((int)(*p_current_data) - (*p_last_data));
                        mismatch |= diff > m_unchanged_border_tolerance;
                        p_current_data += m_width * 3;
                        p_last_data += m_width * 3;
                    }

                    // Break out of look on mismatch
                    if (mismatch) {
                        break;
                    }
                }

                // Scan right columns
                mismatch = false;
                for (; x_end > x_start; x_end--) {
                    p_last_data = p_last_image + (y_start * m_width + x_end) * 3;
                    p_current_data = p_this_image + (y_start * m_width + x_end) * 3;
                    for (int y = y_start; y <= y_end; y++) {
                        int diff = abs((int)(*p_current_data) - (*p_last_data));
                        diff += abs((int)(*p_current_data) - (*p_last_data));
                        diff += abs((int)(*p_current_data) - (*p_last_data));
                        mismatch |= diff > m_unchanged_border_tolerance;
                        p_current_data += m_width * 3;
                        p_last_data += m_width * 3;
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
    }
}

