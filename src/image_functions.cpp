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

    if (image_details.bytes_per_sample == 1) {
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

    } else {  // Bytes per sample == 2
        int32_t x, y;
        uint16_t *data_16 = (uint16_t *)image_details.p_buffer;
        uint16_t *raw_data_ptr;
        uint16_t *rgb_data_ptr1;

        uint32_t bayer_x = bayer_code % 2;
        uint32_t bayer_y = ((bayer_code/2) % 2) ^ (image_details.height % 2);

        // Buffer to create RGB image in
        uint16_t *rgb_data = new uint16_t[3 * image_details.width * image_details.height];

        // Debayer bottom line
        y = 0;
        for (x = 0; x < image_details.width; x++) {
            uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
            debayer_pixel_bilinear <uint16_t> (bayer, x, y, data_16, (uint16_t *)rgb_data, image_details);
        }

        // Debayer top line
        y = image_details.height -1;
        for (x = 0; x < image_details.width; x++) {
            uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
            debayer_pixel_bilinear <uint16_t> (bayer, x, y, data_16, (uint16_t *)rgb_data, image_details);
        }

        // Debayer left edge
        x = 0;
        for (y = 1; y < image_details.height-1; y++) {
            uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
            debayer_pixel_bilinear <uint16_t> (bayer, x, y, data_16, (uint16_t *)rgb_data, image_details);
        }

        // Debayer right edge
        x = image_details.width-1;
        for (y = 1; y < image_details.height-1; y++) {
            uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
            debayer_pixel_bilinear <uint16_t> (bayer, x, y, data_16, (uint16_t *)rgb_data, image_details);
        }

        // Debayer main image to create blue, green and red data
        rgb_data_ptr1 = rgb_data + (3 * (image_details.width + 1));
        raw_data_ptr = data_16 + (3 * (image_details.width + 1));
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
                        *rgb_data_ptr1++ = *raw_data_ptr++;

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
        image_details.p_buffer = (uint8_t *)rgb_data;
    }

    return true;
}


void estimate_colour_balance(
    double &red_gain,
    double &green_gain,
    double &blue_gain,
    const struct s_image_details &image_details)
{
    if (image_details.bytes_per_sample == 2) {
        // Convert data from 16-bit to 8 bit
        uint16_t *p_read_data = (uint16_t *)image_details.p_buffer;
        uint8_t *p_write_data = image_details.p_buffer;
        for (int pixel = 0; pixel < image_details.width * image_details.height * 3; pixel++) {
            *p_write_data++ = (*p_read_data++) >> 8;
        }
    }

    const int PIXEL_COUNT = 25;
    int32_t blue_table[256];
    int32_t green_table[256];
    int32_t red_table[256];

    uint8_t *blue_data_ptr;
    uint8_t *green_data_ptr;
    uint8_t *red_data_ptr;
    if (image_details.colour_id == COLOURID_RGB) {
        // Data order RGB
        blue_data_ptr = image_details.p_buffer + 2;
        green_data_ptr = image_details.p_buffer + 1;
        red_data_ptr = image_details.p_buffer;
    } else {
        // Data order BGR
        blue_data_ptr = image_details.p_buffer;
        green_data_ptr = image_details.p_buffer + 1;
        red_data_ptr = image_details.p_buffer + 2;
    }

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


void change_colour_balance(
    double red,
    double green,
    double blue,
    const struct s_image_details &image_details)
{
    if (red == 1.0 && green == 1.0 && blue == 1.0) {
        // Early return
        return;
    }

    double balance[3];
    if (image_details.colour_id == COLOURID_RGB) {
        // Data is in RGB format
        balance[0] = red;
        balance[1] = green;
        balance[2] = blue;
    } else {
        // Data is in BGR format
        balance[0] = blue;
        balance[1] = green;
        balance[2] = red;
    }

    if (image_details.bytes_per_sample == 1) {
        // 8-bit data
        uint8_t *p_frame_data = image_details.p_buffer;
        for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
            int32_t colour0 = *(p_frame_data+0);
            int32_t colour1 = *(p_frame_data+1);
            int32_t colour2 = *(p_frame_data+2);
            colour0 = (int32_t)(balance[0] * colour0);
            colour1 = (int32_t)(balance[1] * colour1);
            colour2 = (int32_t)(balance[2] * colour2);
            colour0 = (colour0 > 0xFF) ? 0xFF : colour0;
            colour1 = (colour1 > 0xFF) ? 0xFF : colour1;
            colour2 = (colour2 > 0xFF) ? 0xFF : colour2;
            colour0 = (colour0 < 0) ? 0 : colour0;
            colour1 = (colour1 < 0) ? 0 : colour1;
            colour2 = (colour2 < 0) ? 0 : colour2;
            *p_frame_data++ = colour0;
            *p_frame_data++ = colour1;
            *p_frame_data++ = colour2;
        }
    } else {
        // 16-bit data
        uint16_t *p_frame_data = (uint16_t *)image_details.p_buffer;
        for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
            int32_t colour0 = *(p_frame_data+0);
            int32_t colour1 = *(p_frame_data+1);
            int32_t colour2 = *(p_frame_data+2);
            colour0 = (int32_t)(balance[0] * colour0);
            colour1 = (int32_t)(balance[1] * colour1);
            colour2 = (int32_t)(balance[2] * colour2);
            colour0 = (colour0 > 0xFFFF) ? 0xFFFF : colour0;
            colour1 = (colour1 > 0xFFFF) ? 0xFFFF : colour1;
            colour2 = (colour2 > 0xFFFF) ? 0xFFFF : colour2;
            colour0 = (colour0 < 0) ? 0 : colour0;
            colour1 = (colour1 < 0) ? 0 : colour1;
            colour2 = (colour2 < 0) ? 0 : colour2;
            *p_frame_data++ = colour0;
            *p_frame_data++ = colour1;
            *p_frame_data++ = colour2;
        }
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

    double mults[3];
    if (image_details.colour_id == COLOURID_RGB) {
        // Data is in RGB format
        mults[2] = C_Pb;
        mults[1] = C_Pg;
        mults[0] = C_Pr;
    } else {
        // Data is in BGR format
        mults[0] = C_Pb;
        mults[1] = C_Pg;
        mults[2] = C_Pr;
    }

    if (image_details.bytes_per_sample == 1) {
        // 8-bit data
        uint8_t *p_frame_data = image_details.p_buffer;
        for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
            uint8_t *p_col0 = p_frame_data++;
            uint8_t *p_col1 = p_frame_data++;
            uint8_t *p_col2 = p_frame_data++;

            if (*p_col0 != *p_col1 || *p_col0 != *p_col2) {
                // This is not a monochrome pixel - apply colour saturation
                double P = sqrt( (*p_col2) * (*p_col2) * mults[2] +
                                 (*p_col1) * (*p_col1) * mults[1] +
                                 (*p_col0) * (*p_col0) * mults[0] );

                double dcol2 = P + ((double)(*p_col2) - P) * saturation;
                double dcol1 = P + ((double)(*p_col1) - P) * saturation;
                double dcol0 = P + ((double)(*p_col0) - P) * saturation;

                // Clip values in 0 to 255 range
                dcol2 = (dcol2 < 0) ? 0 : dcol2;
                dcol1 = (dcol1 < 0) ? 0 : dcol1;
                dcol0 = (dcol0 < 0) ? 0 : dcol0;
                dcol2 = (dcol2 > 255) ? 255 : dcol2;
                dcol1 = (dcol1 > 255) ? 255 : dcol1;
                dcol0 = (dcol0 > 255) ? 255 : dcol0;

                *p_col2 = (uint8_t)dcol2;
                *p_col1 = (uint8_t)dcol1;
                *p_col0 = (uint8_t)dcol0;
            }
        }
    } else {
        // 16-bit data
        uint16_t *p_frame_data = (uint16_t *)image_details.p_buffer;
        for (int pixel = 0; pixel < image_details.width * image_details.height; pixel++) {
            uint16_t *p_col0 = p_frame_data++;
            uint16_t *p_col1 = p_frame_data++;
            uint16_t *p_col2 = p_frame_data++;

            if (*p_col0 != *p_col1 || *p_col0 != *p_col2) {
                // This is not a monochrome pixel - apply colour saturation
                double P = sqrt( mults[2] * (*p_col2) * (*p_col2) +
                                 mults[1] * (*p_col1) * (*p_col1) +
                                 mults[0] * (*p_col0) * (*p_col0) );

                double dcol2 = P + ((double)(*p_col2) - P) * saturation;
                double dcol1 = P + ((double)(*p_col1) - P) * saturation;
                double dcol0 = P + ((double)(*p_col0) - P) * saturation;

                // Clip values in 0 to 65535 range

                dcol2 = (dcol2 < 0) ? 0 : dcol2;
                dcol1 = (dcol1 < 0) ? 0 : dcol1;
                dcol0 = (dcol0 < 0) ? 0 : dcol0;
                dcol2 = (dcol2 > 65535) ? 65535 : dcol2;
                dcol1 = (dcol1 > 65535) ? 65535 : dcol1;
                dcol0 = (dcol0 > 65535) ? 65535 : dcol0;

                *p_col2 = (uint16_t)dcol2;
                *p_col1 = (uint16_t)dcol1;
                *p_col0 = (uint16_t)dcol0;
            }
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

    if (image_details.bytes_per_sample == 1) {
        // 1 byte per sample
        if (image_debayered || image_details.colour_id == COLOURID_BGR) {
            // Data needs to be in RGB format and flipped vertically
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
            // Data just needs to be flipped vertically
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
    } else {
        // 2 bytes per sample
        if (image_debayered || image_details.colour_id == COLOURID_BGR) {
            // Data needs to be in RGB format and flipped vertically
            uint8_t *p_write_data = p_output_buffer;
            for (int32_t y = image_details.height - 1; y >= 0; y--) {
                uint16_t *p_read_data = (uint16_t *)(image_details.p_buffer + y * image_details.width * 3 * 2);
                for (int32_t x = 0; x < image_details.width; x++) {
                    uint8_t b_pixel = *p_read_data++ >> 8;
                    uint8_t g_pixel = *p_read_data++ >> 8;
                    uint8_t r_pixel = *p_read_data++ >> 8;
                    *p_write_data++ = r_pixel;
                    *p_write_data++ = g_pixel;
                    *p_write_data++ = b_pixel;
                }

                for (int32_t x = 0; x < line_pad; x++) {
                    *p_write_data++ = 0;
                }
            }
        } else {
            // Data just needs to be flipped vertically
            uint8_t *p_write_data = p_output_buffer;
            for (int32_t y = image_details.height - 1; y >= 0; y--) {
                uint16_t *p_read_data = (uint16_t *)(image_details.p_buffer + y * image_details.width * 3 * 2);
                for (int32_t x = 0; x < image_details.width; x++) {
                    *p_write_data++ = *p_read_data++ >> 8;
                    *p_write_data++ = *p_read_data++ >> 8;
                    *p_write_data++ = *p_read_data++ >> 8;
                }

                for (int32_t x = 0; x < line_pad; x++) {
                    *p_write_data++ = 0;
                }
            }
        }
    }


    delete[] image_details.p_buffer;  // Free input buffer
    image_details.p_buffer = p_output_buffer;  // Update pointer to output buffer
}

}  // namespace image_functions
    
