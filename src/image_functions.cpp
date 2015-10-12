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
#include <cmath>
#include "image_functions.h"
#include "pipp_ser.h"


namespace image_functions {


template <typename T>
void debayer_pixel_bilinear(
    uint32_t bayer,
    int32_t x,
    int32_t y,
    T *raw_data,
    T *rgb_data,
    struct s_image_details &image_details)
{
    T *raw_data_ptr = ((T *)raw_data) + ((y * image_details.width + x) * 3);
    T *rgb_data_ptr = ((T *)rgb_data) + ((y * image_details.width + x) * 3);

    uint32_t count;
    uint32_t total;

    // Blue channel
    switch (bayer) {
        case 0:
            // Blue - Average of 4 corners;
            count = 0;
            total = 0;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-3*image_details.width-3);
                count++;
            }

            if (y > 0 && x < image_details.width-1) {
                total += *(raw_data_ptr-3*image_details.width+3);
                count++;
            }

            if (y < image_details.height-1 && x > 0) {
                total += *(raw_data_ptr+3*image_details.width-3);
                count++;
            }

            if (y < image_details.height-1 && x < image_details.width-1) {
                total += *(raw_data_ptr+3*image_details.width+3);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Blue

            // Green - Average of 4 nearest neighbours
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-3);
                count++;
            }

            if (x < image_details.width-1) {
                total += *(raw_data_ptr+3);
                count++;
            }

            if (y < image_details.height-1) {
                total += *(raw_data_ptr+3*image_details.width);
                count++;
            }

            if (y > 0) {
                total += *(raw_data_ptr-3*image_details.width);
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
                total += *(raw_data_ptr-3*image_details.width);
                count++;
            }

            if (y < image_details.height-1) {
                total += *(raw_data_ptr+3*image_details.width);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Blue

            // Green - Average of 4 corners and this position
            count = 1;
            total = *raw_data_ptr;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-3*image_details.width-3);
                count++;
            }

            if (y > 0 && x < image_details.width-1) {
                total += *(raw_data_ptr-3*image_details.width+3);
                count++;
            }

            if (y < image_details.height-1 && x > 0) {
                total += *(raw_data_ptr+3*image_details.width-3);
                count++;
            }

            if (y < image_details.height-1 && x < image_details.width-1) {
                total += *(raw_data_ptr+3*image_details.width+3);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Average of left and right pixels
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-3);
                count++;
            }

            if (x < image_details.width-1) {
                total += *(raw_data_ptr+3);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Red
            break;

        case 2:
            // Blue - Average of left and right pixels
            count = 0;
            total = 0;
            if (x > 0) {
                total += *(raw_data_ptr-3);
                count++;
            }

            if (x < image_details.width-1) {
                total += *(raw_data_ptr+3);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Blue

            // Green - Average of 4 corners and this position
            count = 1;
            total = *raw_data_ptr;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-3*image_details.width-3);
                count++;
            }

            if (y > 0 && x < image_details.width-1) {
                total += *(raw_data_ptr-3*image_details.width+3);
                count++;
            }

            if (y < image_details.height-1 && x > 0) {
                total += *(raw_data_ptr+3*image_details.width-3);
                count++;
            }

            if (y < image_details.height-1 && x < image_details.width-1) {
                total += *(raw_data_ptr+3*image_details.width+3);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Average of above and below pixels
            count = 0;
            total = 0;
            if (y > 0) {
                total += *(raw_data_ptr-3*image_details.width);
                count++;
            }

            if (y < image_details.height-1) {
                total += *(raw_data_ptr+3*image_details.width);
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
                total += *(raw_data_ptr-3);
                count++;
            }

            if (x < image_details.width-1) {
                total += *(raw_data_ptr+3);
                count++;
            }

            if (y < image_details.height-1) {
                total += *(raw_data_ptr+3*image_details.width);
                count++;
            }

            if (y > 0) {
                total += *(raw_data_ptr-3*image_details.width);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Green

            // Red - Average of 4 corners;
            count = 0;
            total = 0;
            if (x > 0 && y > 0) {
                total += *(raw_data_ptr-3*image_details.width-3);
                count++;
            }

            if (y > 0 && x < image_details.width-1) {
                total += *(raw_data_ptr-3*image_details.width+3);
                count++;
            }

            if (y < image_details.height-1 && x > 0) {
                total += *(raw_data_ptr+3*image_details.width-3);
                count++;
            }

            if (y < image_details.height-1 && x < image_details.width-1) {
                total += *(raw_data_ptr+3*image_details.width+3);
                count++;
            }

            *rgb_data_ptr++ = total / count;  // Red
            break;
    }
}


void convert_image_to_8bit(
     struct s_image_details &image_details)
{
    uint16_t *p_read_data = (uint16_t *)image_details.p_buffer;
    uint8_t *p_write_data = image_details.p_buffer;
    for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
        *p_write_data++ = (*p_read_data++) >> 8;
        *p_write_data++ = (*p_read_data++) >> 8;
        *p_write_data++ = (*p_read_data++) >> 8;
    }
}


bool debayer_image_bilinear(
    struct s_image_details &image_details)
{
    uint32_t bayer_code;
    switch (image_details.colour_id) {
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
    uint32_t bayer_y = ((bayer_code/2) % 2) ^ (image_details.height % 2);

    // Buffer to create RGB image in
    uint8_t *rgb_data = new uint8_t[3 * image_details.width * image_details.height];

    // Debayer bottom line
    y = 0;
    for (x = 0; x < image_details.width; x++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, image_details.p_buffer, rgb_data, image_details);
    }

    // Debayer top line
    y = image_details.height -1;
    for (x = 0; x < image_details.width; x++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, image_details.p_buffer, rgb_data, image_details);
    }

    // Debayer left edge
    x = 0;
    for (y = 1; y < image_details.height-1; y++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, image_details.p_buffer, rgb_data, image_details);
    }

    // Debayer right edge
    x = image_details.width-1;
    for (y = 1; y < image_details.height-1; y++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <uint8_t> (bayer, x, y, image_details.p_buffer, rgb_data, image_details);
    }

    // Debayer to create blue, green and red data
    rgb_data_ptr1 = rgb_data + (3 * (image_details.width + 1));
    raw_data_ptr = image_details.p_buffer + (3 * (image_details.width + 1));
    for (y = 1; y < (image_details.height-1); y++) {
        for (x = 1; x < (image_details.width-1); x++) {
            uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
            // Blue channel
            switch (bayer) {
                case 0:
                    // Blue - Average of 4 corners;
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3*image_details.width-3) + *(raw_data_ptr-3*image_details.width+3) +
                                         *(raw_data_ptr+3*image_details.width-3) + *(raw_data_ptr+3*image_details.width+3) ) / 4;
                    raw_data_ptr++;

                    // Green - Return average of 4 nearest neighbours
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3) + *(raw_data_ptr+3) +
                                         *(raw_data_ptr+3*image_details.width) + *(raw_data_ptr-3*image_details.width) ) /4;
                    raw_data_ptr++;

                    // Red - Simple case just return data at this position
                    *rgb_data_ptr1++ = *raw_data_ptr++;
                    break;

                case 1:
                    // Blue - Average of above and below pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3*image_details.width) + *(raw_data_ptr+3*image_details.width) ) / 2;
                    raw_data_ptr++;

                    // Green - just this position
                    *rgb_data_ptr1++ = *raw_data_ptr++;;

                    // Red - Average of left and right pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3) + *(raw_data_ptr+3) ) / 2;
                    raw_data_ptr++;
                    break;

                case 2:
                    // Blue - Average of left and right pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3) + *(raw_data_ptr+3) ) / 2;
                    raw_data_ptr++;

                    // Green - just this position
                    *rgb_data_ptr1++ = *raw_data_ptr++;

                    // Red - Average of above and below pixels
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3*image_details.width) + *(raw_data_ptr+3*image_details.width) ) / 2;
                    raw_data_ptr++;
                    break;

                default:
                    // Blue - Simple case just return data at this position
                    *rgb_data_ptr1++ = *raw_data_ptr++;

                    // Green - Return average of 4 nearest neighbours
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3) + *(raw_data_ptr+3) +
                                         *(raw_data_ptr+3*image_details.width) + *(raw_data_ptr-3*image_details.width) ) /4;
                    raw_data_ptr++;

                    // Red - Average of 4 corners;
                    *rgb_data_ptr1++ = ( *(raw_data_ptr-3*image_details.width-3) + *(raw_data_ptr-3*image_details.width+3) +
                                         *(raw_data_ptr+3*image_details.width-3) + *(raw_data_ptr+3*image_details.width+3) ) / 4;
                    raw_data_ptr++;
                    break;
            }
        }

        rgb_data_ptr1 += 6;
        raw_data_ptr += 6;
    }

    // Make new debayered data the frame buffer data
    delete[] image_details.p_buffer;
    image_details.p_buffer = rgb_data;

    return true;
}


void estimate_colour_balance(
    double &red_gain,
    double &green_gain,
    double &blue_gain,
    const struct s_image_details &image_details)
{
    const int PIXEL_COUNT = 25;
    int32_t blue_table[256];
    int32_t green_table[256];
    int32_t red_table[256];

    uint8_t *blue_data_ptr;
    uint8_t *green_data_ptr;
    uint8_t *red_data_ptr;

    // Data order is always BGR
    blue_data_ptr = image_details.p_buffer;
    green_data_ptr = image_details.p_buffer + 1;
    red_data_ptr = image_details.p_buffer + 2;

    // Clear histgrams
    memset(blue_table, 0, 256 * sizeof(int32_t));
    memset(green_table, 0, 256 * sizeof(int32_t));
    memset(red_table, 0, 256 * sizeof(int32_t));

    // Create histograms
    for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
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


uint8_t colbal_r_lut[256];
uint8_t colbal_g_lut[256];
uint8_t colbal_b_lut[256];
bool colour_balance_enabled = false;
void set_colour_balance_luts(
    double red_gain,
    double green_gain,
    double blue_gain)
{
    if (red_gain == 1.0 && green_gain == 1.0 && blue_gain == 1.0) {
        colour_balance_enabled = false;
    } else {
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
            colbal_r_lut[x] = (uint8_t)temp_r;
            colbal_g_lut[x] = (uint8_t)temp_g;
            colbal_b_lut[x] = (uint8_t)temp_b;
        }
        colour_balance_enabled = true;
    }
}


void change_colour_balance(
    const struct s_image_details &image_details)
{
    if (colour_balance_enabled) {
        uint8_t *p_frame_data = image_details.p_buffer;
        for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
            *p_frame_data = colbal_b_lut[*p_frame_data];
            p_frame_data++;
            *p_frame_data = colbal_g_lut[*p_frame_data];
            p_frame_data++;
            *p_frame_data = colbal_r_lut[*p_frame_data];
            p_frame_data++;
        }
    }
}


void change_colour_balance(
    double red_gain,
    double green_gain,
    double blue_gain,
    const struct s_image_details &image_details)
{
    if (red_gain == 1.0 && green_gain == 1.0 && blue_gain == 1.0) {
        // Early return
        return;
    }

    // 8-bit data
    uint8_t *p_frame_data = image_details.p_buffer;
    for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
        // Get current pixel from buffer
        int32_t blue = *(p_frame_data+0);
        int32_t green = *(p_frame_data+1);
        int32_t red = *(p_frame_data+2);

        // Apply individual gains
        blue = (int32_t)(blue_gain * blue);
        green = (int32_t)(green_gain * green);
        red = (int32_t)(red_gain * red);

        // Clamp to valid range
        blue = (blue > 0xFF) ? 0xFF : blue;
        green = (green > 0xFF) ? 0xFF : green;
        red = (red > 0xFF) ? 0xFF : red;
        blue = (blue < 0) ? 0 : blue;
        green = (green < 0) ? 0 : green;
        red = (red < 0) ? 0 : red;

        // Write updated pixel back to the buffer
        *p_frame_data++ = blue;
        *p_frame_data++ = green;
        *p_frame_data++ = red;
    }
}


void change_colour_saturation(
    double saturation,
    const struct s_image_details &image_details)
{
    const double C_Pr = .299;
    const double C_Pg = .587;
    const double C_Pb = .114;

    if (saturation == 1.0) {
        // Early return
        return;
    }

    // 8-bit data always
    uint8_t *p_frame_data = image_details.p_buffer;
    for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
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


void conv_data_ready_for_qimage(
    bool image_debayered,
    struct s_image_details &image_details)
{
    // Create buffer for converted data
    int line_pad = (image_details.width * 3) % 4;
    if (line_pad != 0) {
        line_pad = 4 - line_pad;
    }

    int32_t buffer_size = (image_details.width + line_pad) * image_details.height * 3;
    uint8_t *p_output_buffer = new uint8_t [buffer_size];

    // 1 byte per sample
    if (image_debayered || image_details.colour_id == COLOURID_RGB || image_details.colour_id == COLOURID_BGR) {
        // Data needs to be changed from BGR to RGB format and flipped vertically
        uint8_t *p_write_data = p_output_buffer;
        for (int32_t y = image_details.height - 1; y >= 0; y--) {
            uint8_t *p_read_data = image_details.p_buffer + y * image_details.width * 3;
            for (int32_t x = 0; x < image_details.width; x++) {
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
        for (int32_t y = image_details.height - 1; y >= 0; y--) {
            uint8_t *p_read_data = image_details.p_buffer + y * image_details.width * 3;
            memcpy(p_write_data, p_read_data, image_details.width * 3);
            p_write_data += image_details.width * 3;

            for (int32_t x = 0; x < line_pad; x++) {
                *p_write_data++ = 0;
            }
        }
    }

    delete[] image_details.p_buffer;  // Free input buffer
    image_details.p_buffer = p_output_buffer;  // Update pointer to output buffer
}


}  // namespace image_functions
    
