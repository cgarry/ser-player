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


#include <QDebug>
#include <cstring>  // memset()
#include <cmath>  // sqrt()

#include "image.h"
#include "pipp_ser.h"



template <typename T>
void c_image::debayer_pixel_bilinear(
    uint32_t bayer,
    int32_t x,
    int32_t y,
    T *raw_data,
    T *rgb_data)
{
    T *raw_data_ptr = ((T *)raw_data) + (y * m_width + x);
    T *rgb_data_ptr = ((T *)rgb_data) + ((y * m_width + x) * 3);

    uint32_t count;
    uint32_t total;

    // Blue channel
    switch (bayer) {
        case 0:
            // Blue - Average of 4 corners;
            count = 0;
            total = 0;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-m_width-1);
                count++;
            }

            if (y > 0 && x < m_width-1) {
                total += *(raw_data_ptr-m_width+1);
                count++;
            }

            if (y < m_height-1 && x > 0) {
                total += *(raw_data_ptr+m_width-1);
                count++;
            }

            if (y < m_height-1 && x < m_width-1) {
                total += *(raw_data_ptr+m_width+1);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Blue

            // Green - Average of 4 nearest neighbours
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-1);
                count++;
            }

            if (x < m_width-1) {
                total += *(raw_data_ptr+1);
                count++;
            }

            if (y < m_height-1) {
                total += *(raw_data_ptr+m_width);
                count++;
            }

            if (y > 0) {
                total += *(raw_data_ptr-m_width);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Simple case just return data at this position
            *rgb_data_ptr = *raw_data_ptr;
            break;

        case 1:
            // Blue - Average of above and below pixels
            count = 0;
            total = 0;
            if (y > 0) {
                total += *(raw_data_ptr-m_width);
                count++;
            }

            if (y < m_height-1) {
                total += *(raw_data_ptr+m_width);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Blue

            // Green - Average of 4 corners and this position
            count = 1;
            total = *raw_data_ptr;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-m_width-1);
                count++;
            }

            if (y > 0 && x < m_width-1) {
                total += *(raw_data_ptr-m_width+1);
                count++;
            }

            if (y < m_height-1 && x > 0) {
                total += *(raw_data_ptr+m_width-1);
                count++;
            }

            if (y < m_height-1 && x < m_width-1) {
                total += *(raw_data_ptr+m_width+1);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Average of left and right pixels
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-1);
                count++;
            }

            if (x < m_width-1) {
                total += *(raw_data_ptr+1);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Red
            break;

        case 2:
            // Blue - Average of left and right pixels
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-1);
                count++;
            }

            if (x < m_width-1) {
                total += *(raw_data_ptr+1);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Blue

            // Green - Average of 4 corners and this position
            count = 1;
            total = *raw_data_ptr;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-m_width-1);
                count++;
            }

            if (y > 0 && x < m_width-1) {
                total += *(raw_data_ptr-m_width+1);
                count++;
            }

            if (y < m_height-1 && x > 0) {
                total += *(raw_data_ptr+m_width-1);
                count++;
            }

            if (y < m_height-1 && x < m_width-1) {
                total += *(raw_data_ptr+m_width+1);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Average of above and below pixels
            count = 0;
            total = 0;
            if (y > 0) {
                total += *(raw_data_ptr-m_width);
                count++;
            }

            if (y < m_height-1) {
                total += *(raw_data_ptr+m_width);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Red
            break;

        default:
            // Blue - Simple case just return data at this position
            *rgb_data_ptr++ = *raw_data_ptr;

            // Green - Return average of 4 nearest neighbours
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-1);
                count++;
            }

            if (x < m_width-1) {
                total += *(raw_data_ptr+1);
                count++;
            }

            if (y < m_height-1) {
                total += *(raw_data_ptr+m_width);
                count++;
            }

            if (y > 0) {
                total += *(raw_data_ptr-m_width);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Average of 4 corners;
            count = 0;
            total = 0;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-m_width-1);
                count++;
            }

            if (y > 0 && x < m_width-1) {
                total += *(raw_data_ptr-m_width+1);
                count++;
            }

            if (y < m_height-1 && x > 0) {
                total += *(raw_data_ptr+m_width-1);
                count++;
            }

            if (y < m_height-1 && x < m_width-1) {
                total += *(raw_data_ptr+m_width+1);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Red
            break;
    }
}



void c_image::set_image_details(int32_t width,
                                int32_t height,
                                int32_t byte_depth,
                                bool colour)
{
    m_width = width;
    m_height = height;
    m_byte_depth = byte_depth;
    m_colour = colour;
    
    int32_t frame_size = m_width * m_height * m_byte_depth;
    if (m_colour) {
        frame_size *= 3;
    }
    
    set_buffer_size(frame_size);
}


void c_image::convert_image_to_8bit()
{
    if (m_byte_depth == 2) { 
        uint16_t *p_read_data = (uint16_t *)mp_buffer;
        uint8_t *p_write_data = mp_buffer;
        int pixel_count = m_width * m_height;
        if (m_colour) {
            pixel_count *= 3;
        }

        for (int pixel = 0; pixel < pixel_count; pixel++) {
            *p_write_data++ = (*p_read_data++) >> 8;
        }
        
        m_byte_depth = 1;
    }
}


void c_image::estimate_colour_balance(
    double &red_gain,
    double &green_gain,
    double &blue_gain)
{
    const int PIXEL_COUNT = 25;

    uint8_t *blue_data_ptr;
    uint8_t *green_data_ptr;
    uint8_t *red_data_ptr;

    // Data order is always BGR
    blue_data_ptr = mp_buffer;
    green_data_ptr = mp_buffer + 1;
    red_data_ptr = mp_buffer + 2;

    // Clear histgrams
    int32_t blue_table[256];
    int32_t green_table[256];
    int32_t red_table[256];
    memset(blue_table, 0, 256 * sizeof(int32_t));
    memset(green_table, 0, 256 * sizeof(int32_t));
    memset(red_table, 0, 256 * sizeof(int32_t));

    // Create histograms
    for (int pixel = 0; pixel < m_width * m_height; pixel++) {
        blue_table[*blue_data_ptr]++;
        green_table[*green_data_ptr]++;
        red_table[*red_data_ptr]++;
        blue_data_ptr += 3;
        green_data_ptr += 3;
        red_data_ptr += 3;
    }

    // Get averages for max PIXEL_COUNT pixels
    uint32_t blue_max_average = 0;
    uint32_t count = PIXEL_COUNT;
    for (int x = 255; x > 0; x--) {
        if (count <= (uint32_t)blue_table[x]) {
            blue_max_average += count * x;
            break;
        } else {
            blue_max_average += blue_table[x] * x;
            count -= blue_table[x];
        }
    }

    uint32_t green_max_average = 0;
    count = PIXEL_COUNT;
    for (int x = 255; x > 0; x--) {
        if (count <= (uint32_t)green_table[x]) {
            green_max_average += count * x;
            break;
        } else {
            green_max_average += green_table[x] * x;
            count -= green_table[x];
        }
    }

    uint32_t red_max_average = 0;
    count = PIXEL_COUNT;
    for (int x = 255; x > 0; x--) {
        if (count <= (uint32_t)red_table[x]) {
            red_max_average += count * x;
            break;
        } else {
            red_max_average += red_table[x] * x;
            count -= red_table[x];
        }
    }

    uint32_t max_max_average = blue_max_average;
    if (green_max_average > max_max_average) {
        max_max_average = green_max_average;
    }

    if (red_max_average > max_max_average) {
        max_max_average = red_max_average;
    }

    // Prevent divide by zero errors
    blue_max_average = (blue_max_average == 0) ? max_max_average : blue_max_average;
    green_max_average = (green_max_average == 0) ? max_max_average : green_max_average;
    red_max_average = (red_max_average == 0) ? max_max_average : red_max_average;

    // Pass estimated gains back to caller
    blue_gain = (double)max_max_average / blue_max_average;
    green_gain = (double)max_max_average / green_max_average;
    red_gain = (double)max_max_average / red_max_average;
}


void c_image::set_colour_balance_luts(
    double red_gain,
    double green_gain,
    double blue_gain)
{
    if (red_gain == 1.0 && green_gain == 1.0 && blue_gain == 1.0) {
        m_colour_balance_enabled = false;
    } else {
        m_colour_balance_enabled = true;
        for (int x = 0; x < 256; x++) {
            // Calculate individual gains
            double temp_r, temp_g, temp_b;
            temp_r = red_gain * x;
            temp_g = green_gain * x;
            temp_b = blue_gain * x;

            // Clamp values
            temp_r = (temp_r > 255) ? 255 : temp_r;
            temp_g = (temp_g > 255) ? 255 : temp_g;
            temp_b = (temp_b > 255) ? 255 : temp_b;

            // Write to LUTs
            m_colbal_r_lut[x] = (uint8_t)temp_r;
            m_colbal_g_lut[x] = (uint8_t)temp_g;
            m_colbal_b_lut[x] = (uint8_t)temp_b;
        }
    }
}


void c_image::change_colour_balance()
{
    if (m_colour_balance_enabled && m_colour) {
        uint8_t *p_frame_data = mp_buffer;
        for (int pixel = 0; pixel < m_width * m_height; pixel++) {
            *p_frame_data = m_colbal_b_lut[*p_frame_data];
            p_frame_data++;
            *p_frame_data = m_colbal_g_lut[*p_frame_data];
            p_frame_data++;
            *p_frame_data = m_colbal_r_lut[*p_frame_data];
            p_frame_data++;
        }
    }
}


void c_image::change_colour_saturation(
    double saturation)
{
    // Only chnage colour saturation for colour images
    // saturation == 1.0 means no change so do nothing
    if (m_colour && saturation != 1.0) {
        const double C_Pr = .299;
        const double C_Pg = .587;
        const double C_Pb = .114;

        // 8-bit data always
        uint8_t *p_frame_data = mp_buffer;
        for (int pixel = 0; pixel < m_width * m_height; pixel++) {
            uint8_t *p_blue = p_frame_data++;
            uint8_t *p_green = p_frame_data++;
            uint8_t *p_red = p_frame_data++;

            if (*p_blue != *p_green || *p_blue != *p_red) {
                // This is not a monochrome pixel - apply colour saturation
                double P = sqrt( (*p_red) * (*p_red) * C_Pr +
                                 (*p_green) * (*p_green) * C_Pg +
                                 (*p_blue) * (*p_blue) * C_Pb );

                double dred = P + ((double)(*p_red) - P) * saturation;
                double dgreen = P + ((double)(*p_green) - P) * saturation;
                double dblue = P + ((double)(*p_blue) - P) * saturation;

                // Clip values in 0 to 255 range
                dred = (dred < 0) ? 0 : dred;
                dgreen = (dgreen < 0) ? 0 : dgreen;
                dblue = (dblue < 0) ? 0 : dblue;
                dred = (dred > 255) ? 255 : dred;
                dgreen = (dgreen > 255) ? 255 : dgreen;
                dblue = (dblue > 255) ? 255 : dblue;

                *p_red = (uint8_t)dred;
                *p_green = (uint8_t)dgreen;
                *p_blue = (uint8_t)dblue;
            }
        }
    }
}


void c_image::conv_data_ready_for_qimage()
{
    // Create buffer for converted data
    int line_pad = (m_width * 3) % 4;
    if (line_pad != 0) {
        line_pad = 4 - line_pad;
    }

    int32_t buffer_size = (m_width + line_pad) * m_height * 3;
    uint8_t *p_output_buffer = new uint8_t [buffer_size];

    // 1 byte per sample
    if (m_colour) {
        // Colour data needs to be changed from BGR to RGB format and flipped vertically
        uint8_t *p_write_data = p_output_buffer;
        for (int32_t y = m_height - 1; y >= 0; y--) {
            uint8_t *p_read_data = mp_buffer + y * m_width * 3;
            for (int32_t x = 0; x < m_width; x++) {
                uint8_t b_pixel = *p_read_data++;
                uint8_t g_pixel = *p_read_data++;
                uint8_t r_pixel = *p_read_data++;
                *p_write_data++ = r_pixel;
                *p_write_data++ = g_pixel;
                *p_write_data++ = b_pixel;
            }

            for (int32_t x = 0; x < line_pad; x++) {
                *p_write_data++ = 0;
            }
        }
    } else {
        // Monochrome data just needs to be flipped vertically
        uint8_t *p_write_data = p_output_buffer;
        for (int32_t y = m_height - 1; y >= 0; y--) {
            uint8_t *p_read_data = mp_buffer + y * m_width;
            for (int32_t x = 0; x < m_width; x++) {
                *p_write_data++ = *p_read_data;
                *p_write_data++ = *p_read_data;
                *p_write_data++ = *p_read_data++;
            }

            for (int32_t x = 0; x < line_pad; x++) {
                *p_write_data++ = 0;
            }
        }
    }

    delete[] mp_buffer;  // Free input buffer
    mp_buffer = p_output_buffer;  // Update pointer to output buffer
    m_buffer_size = buffer_size;
}


bool c_image::debayer_image_bilinear(int32_t colour_id)
{
    uint32_t bayer_code;
    switch (colour_id) {
    case COLOURID_BAYER_RGGB:
        bayer_code = 2;
        break;
    case COLOURID_BAYER_GRBG:
        bayer_code = 3;
        break;
    case COLOURID_BAYER_GBRG:
        bayer_code = 0;
        break;
    case COLOURID_BAYER_BGGR:
        bayer_code = 1;
        break;
    default:
        // We only debayer these types
        return false;
    }

    int32_t x, y;
    uint8_t *raw_data_ptr;
    uint8_t *rgb_data_ptr1;

    uint32_t bayer_x = bayer_code % 2;
    uint32_t bayer_y = ((bayer_code/2) % 2) ^ (m_height % 2);

    // Buffer to create RGB image in
    uint8_t *rgb_data = new uint8_t[3 * m_width * m_height];

    // Debayer bottom line
    y = 0;
    for (x = 0; x < m_width; x++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, mp_buffer, rgb_data);
    }

    // Debayer top line
    y = m_height -1;
    for (x = 0; x < m_width; x++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, mp_buffer, rgb_data);
    }

    // Debayer left edge
    x = 0;
    for (y = 1; y < m_height-1; y++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, mp_buffer, rgb_data);
    }

    // Debayer right edge
    x = m_width-1;
    for (y = 1; y < m_height-1; y++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, mp_buffer, rgb_data);
    }

    // Debayer to create blue, green and red data
    rgb_data_ptr1 = rgb_data + (3 * (m_width + 1));
    raw_data_ptr = mp_buffer + (m_width + 1);

    for (y = 1; y < (m_height-1); y++) {
        for (x = 1; x < (m_width-1); x++) {
            uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
            // Blue channel
            switch (bayer) {
                case 0:
                    // Blue - Average of 4 corners;
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-m_width-1) + *(raw_data_ptr-m_width+1) +
                                         *(raw_data_ptr+m_width-1) + *(raw_data_ptr+m_width+1) ) / 4;

                    // Green - Return average of 4 nearest neighbours
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-1) + *(raw_data_ptr+1) +
                                         *(raw_data_ptr+m_width) + *(raw_data_ptr-m_width) ) /4;

                    // Red - Simple case just return data at this position
                    *rgb_data_ptr1++ = *raw_data_ptr++;
                    break;

                case 1:
                    // Blue - Average of above and below pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-m_width) + *(raw_data_ptr+m_width) ) / 2;

                    // Green - just this position
                    *rgb_data_ptr1++ = *raw_data_ptr;

                    // Red - Average of left and right pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-1) + *(raw_data_ptr+1) ) / 2;
                    raw_data_ptr++;
                    break;

                case 2:
                    // Blue - Average of left and right pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-1) + *(raw_data_ptr+1) ) / 2;

                    // Green - just this position
                    *rgb_data_ptr1++ = *raw_data_ptr;

                    // Red - Average of above and below pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-m_width) + *(raw_data_ptr+m_width) ) / 2;
                    raw_data_ptr++;
                    break;

                default:
                    // Blue - Simple case just return data at this position
                    *rgb_data_ptr1++ = *raw_data_ptr;

                    // Green - Return average of 4 nearest neighbours
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-1) + *(raw_data_ptr+1) +
                                         *(raw_data_ptr+m_width) + *(raw_data_ptr+m_width) ) /4;

                    // Red - Average of 4 corners;
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-m_width-1) + *(raw_data_ptr-m_width+1) +
                                         *(raw_data_ptr+m_width-1) + *(raw_data_ptr+m_width+1) ) / 4;
                    raw_data_ptr++;
                    break;
            }
        }

        rgb_data_ptr1 += 6;
        raw_data_ptr += 2;
    }

    // Make new debayered data the frame buffer data
    delete[] mp_buffer;
    mp_buffer = rgb_data;
    m_colour = true;
    return true;
}


//
// Private functions below here
//

void c_image::set_buffer_size(int32_t size)
{
    if (size > m_buffer_size) {
        delete [] mp_buffer;
        mp_buffer = new uint8_t[size];
        m_buffer_size = size;
    }
}
