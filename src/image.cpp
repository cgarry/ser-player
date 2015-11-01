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
                                int32_t colour_id,
                                bool colour)
{
    m_width = width;
    m_height = height;
    m_byte_depth = byte_depth;
    m_colour_id = colour_id;
    if (m_colour_id == COLOURID_RGB) {
        // Colour images are always in BGR format as they are converted in the SER class
        m_colour_id = COLOURID_BGR;
    }

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


void c_image::set_gain(
        double gain)
{
    m_gain = gain;
    setup_luts();
}


void c_image::set_gamma(
        double gamma)
{
    m_gamma = gamma;
    setup_luts();
}


void c_image::set_colour_balance(
    double red_gain,
    double green_gain,
    double blue_gain)
{
    m_red_gain = red_gain;
    m_green_gain = green_gain;
    m_blue_gain = blue_gain;
    if (red_gain == 1.0 && green_gain == 1.0 && blue_gain == 1.0) {
        m_colour_balance_enabled = false;
    } else {
        m_colour_balance_enabled = true;
    }

    setup_luts();
}


void c_image::setup_luts()
{
    for (int x = 0; x < 256; x++) {
        // Calculate colour balance gains and main gain
        double temp_r, temp_g, temp_b, temp_m;
        temp_r = m_red_gain * m_gain * x;
        temp_g = m_green_gain * m_gain * x;
        temp_b = m_blue_gain * m_gain * x;
        temp_m = m_gain * x;

        // Clamp values
        temp_r = (temp_r > 255) ? 255 : temp_r;
        temp_g = (temp_g > 255) ? 255 : temp_g;
        temp_b = (temp_b > 255) ? 255 : temp_b;
        temp_m = (temp_m > 255) ? 255 : temp_m;

        // Calculate gamma
        temp_r = pow((double)(temp_r / 255.0), (double)(1 / m_gamma)) * 255.0 + 0.5;
        temp_g = pow((double)(temp_g / 255.0), (double)(1 / m_gamma)) * 255.0 + 0.5;
        temp_b = pow((double)(temp_b / 255.0), (double)(1 / m_gamma)) * 255.0 + 0.5;
        temp_m = pow((double)(temp_m / 255.0), (double)(1 / m_gamma)) * 255.0 + 0.5;

        // Clamp values
        temp_r = (temp_r > 255) ? 255 : temp_r;
        temp_g = (temp_g > 255) ? 255 : temp_g;
        temp_b = (temp_b > 255) ? 255 : temp_b;
        temp_m = (temp_m > 255) ? 255 : temp_m;

        // Write to LUTs
        m_red_lut[x] = (uint8_t)temp_r;
        m_green_lut[x] = (uint8_t)temp_g;
        m_blue_lut[x] = (uint8_t)temp_b;
        m_mono_lut[x] = (uint8_t)temp_m;
    }
}


void c_image::monochrome_conversion(int conv_type)
{
    if (m_colour) {
        switch (conv_type) {
        case 0:  // conv_type 0 - make mono from all RGB channels
            if (m_byte_depth == 1) {
                uint8_t *write_data_ptr = mp_buffer;
                uint8_t *read_data_ptr = mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RGB values to luminace
                    uint32_t luminance = (114 * *read_data_ptr + 587 * *(read_data_ptr+1) + 299 * *(read_data_ptr+2)) / 1000;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint8_t)luminance;
                }
            } else {
                uint16_t *write_data_ptr = (uint16_t *)mp_buffer;
                uint16_t *read_data_ptr = (uint16_t *)mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RGB values to luminace
                    uint32_t luminance = (114 * *read_data_ptr + 587 * *(read_data_ptr+1) + 299 * *(read_data_ptr+2)) / 1000;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint16_t)luminance;
                }
            }

            m_colour_id = COLOURID_MONO;
            m_colour = false;
            break;

        case 1:  // conv_type 1 - make mono from all R channel only
        case 2:  // conv_type 2 - make mono from all G channel only
        case 3:  // conv_type 3 - make mono from all B channel only
            if (m_byte_depth == 1) {
                uint8_t *write_data_ptr = mp_buffer;
                uint8_t *read_data_ptr = mp_buffer + (3 - conv_type);  // Start on correct coloured pixel
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RG or B values to luminace
                    *write_data_ptr++ = *read_data_ptr;
                    read_data_ptr += 3;
                }
            } else {
                uint16_t *write_data_ptr = (uint16_t *)mp_buffer;
                uint16_t *read_data_ptr = (uint16_t *)mp_buffer + (3 - conv_type);  // Start on correct coloured pixel
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RG or B values to luminace
                    *write_data_ptr++ = *read_data_ptr;
                    read_data_ptr += 3;
                }
            }

            m_colour_id = COLOURID_MONO;
            m_colour = false;
            break;

        case 4:  // R and G
            if (m_byte_depth == 1) {
                uint8_t *write_data_ptr = mp_buffer;
                uint8_t *read_data_ptr = mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RG values to luminace
                    uint32_t luminance = (587 * *(read_data_ptr+1) + 299 * *(read_data_ptr+2)) / 886;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint8_t)luminance;
                }

            } else {
                uint16_t *write_data_ptr = (uint16_t *)mp_buffer;
                uint16_t *read_data_ptr = (uint16_t *)mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RG values to luminace
                    uint32_t luminance = (587 * *(read_data_ptr+1) + 299 * *(read_data_ptr+2)) / 886;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint16_t)luminance;
                }
            }

            m_colour_id = COLOURID_MONO;
            m_colour = false;
            break;
        case 5:  // R and B
            if (m_byte_depth == 1) {
                uint8_t *write_data_ptr = mp_buffer;
                uint8_t *read_data_ptr = mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RB values to luminace
                    uint32_t luminance = (114 * *(read_data_ptr) + 299 * *(read_data_ptr+2)) / 413;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint8_t)luminance;
                }

            } else {
                uint16_t *write_data_ptr = (uint16_t *)mp_buffer;
                uint16_t *read_data_ptr = (uint16_t *)mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RB values to luminace
                    uint32_t luminance = (114 * *(read_data_ptr) + 299 * *(read_data_ptr+2)) / 413;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint16_t)luminance;
                }
            }

            m_colour_id = COLOURID_MONO;
            m_colour = false;
            break;
        case 6:  // G and B
            if (m_byte_depth == 1) {
                uint8_t *write_data_ptr = mp_buffer;
                uint8_t *read_data_ptr = mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert GB values to luminace
                    uint32_t luminance = (114 * *(read_data_ptr) + 587 * *(read_data_ptr+1)) / 701;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint8_t)luminance;
                }

            } else {
                uint16_t *write_data_ptr = (uint16_t *)mp_buffer;
                uint16_t *read_data_ptr = (uint16_t *)mp_buffer;
                for (int32_t x = 0; x < m_height * m_width; x++) {
                    // Convert RB values to luminace
                    uint32_t luminance = (114 * *(read_data_ptr) + 587 * *(read_data_ptr+1)) / 701;
                    read_data_ptr += 3;
                    *write_data_ptr++ = (uint16_t)luminance;
                }
            }

            m_colour_id = COLOURID_MONO;
            m_colour = false;
            break;

        default:
            break;
            // Do nothing
        }
    }
}


void c_image::do_lut_based_processing()
{
    if (m_byte_depth == 1) {
        // 8-bit version just uses LUTs
        if (!m_colour) {
            // Mono images just use 1 LUT
            if (m_gain != 1.0 || m_gamma != 1.0) {
                uint8_t *p_frame_data = mp_buffer;
                for (int pixel = 0; pixel < m_width * m_height; pixel++) {
                    *p_frame_data = m_mono_lut[*p_frame_data];
                    p_frame_data++;
                }
            }
        } else {
            // Colour images use all 3 LUTs
            if ((m_colour_balance_enabled && m_colour) || m_gain != 1.0 || m_gamma != 1.0) {
                uint8_t *p_frame_data = mp_buffer;
                for (int pixel = 0; pixel < m_width * m_height; pixel++) {
                    *p_frame_data = m_blue_lut[*p_frame_data];
                    p_frame_data++;
                    *p_frame_data = m_green_lut[*p_frame_data];
                    p_frame_data++;
                    *p_frame_data = m_red_lut[*p_frame_data];
                    p_frame_data++;
                }
            }
        }
    } else {
        // 16-bit version
        if (!m_colour) {
            // Monochrome processing
            uint16_t *data_ptr = (uint16_t *)mp_buffer;
            for (int x = 0; x < m_width * m_height; x++) {
                double mono_data = *data_ptr;

                // Apply main gain
                mono_data *= m_gain;
                mono_data = (mono_data > 65535.0) ? 65535.0 : mono_data;

                // Apply gamma
                mono_data = (uint16_t)(pow((double)(mono_data / 65535.0), (double)(1 / m_gamma)) * 65535.0 + 0.5);
                mono_data = (mono_data > 65535.0) ? 65535.0 : mono_data;

                *data_ptr++ = mono_data;
            }
        } else {
            // Colour processing
            uint16_t *data_ptr = (uint16_t *)mp_buffer;
            for (int x = 0; x < m_width * m_height; x++) {
                double b_data = *data_ptr;
                double g_data = *(data_ptr + 1);
                double r_data = *(data_ptr + 2);

                // Apply colour balance gains and main gain
                b_data *=  m_blue_gain * m_gain;
                g_data *=  m_green_gain * m_gain;
                r_data *=  m_red_gain * m_gain;
                b_data = (b_data > 65535.0) ? 65535.0 : b_data;
                g_data = (g_data > 65535.0) ? 65535.0 : g_data;
                r_data = (r_data > 65535.0) ? 65535.0 : r_data;

                // Apply gamma
                b_data = pow((double)(b_data / 65535.0), (double)(1 / m_gamma)) * 65535.0 + 0.5;
                g_data = pow((double)(g_data / 65535.0), (double)(1 / m_gamma)) * 65535.0 + 0.5;
                r_data = pow((double)(r_data / 65535.0), (double)(1 / m_gamma)) * 65535.0 + 0.5;
                b_data = (b_data > 65535.0) ? 65535.0 : b_data;
                g_data = (g_data > 65535.0) ? 65535.0 : g_data;
                r_data = (r_data > 65535.0) ? 65535.0 : r_data;

                *data_ptr++ = (uint16_t)b_data;
                *data_ptr++ = (uint16_t)g_data;
                *data_ptr++ = (uint16_t)r_data;
            }
        }
    }
}


void c_image::change_colour_saturation(
    double saturation)
{
    if (m_byte_depth == 1) {
        return change_colour_saturation_int <uint8_t> (saturation);
    } else {
        return change_colour_saturation_int <uint16_t> (saturation);
    }
}


template <typename T>
void c_image::change_colour_saturation_int(
    double saturation)
{
    // Only chnage colour saturation for colour images
    // saturation == 1.0 means no change so do nothing
    if (m_colour && saturation != 1.0) {
        const double C_Pr = .299;
        const double C_Pg = .587;
        const double C_Pb = .114;

        T *p_frame_data = (T *)mp_buffer;
        for (int pixel = 0; pixel < m_width * m_height; pixel++) {
            T *p_blue = p_frame_data++;
            T *p_green = p_frame_data++;
            T *p_red = p_frame_data++;

            if (*p_blue != *p_green || *p_blue != *p_red) {
                // This is not a monochrome pixel - apply colour saturation
                double P = sqrt( C_Pr * (*p_red) * (*p_red) +
                                 C_Pg * (*p_green) * (*p_green) +
                                 C_Pb * (*p_blue) * (*p_blue) );

                double dred = P + ((double)(*p_red) - P) * saturation;
                double dgreen = P + ((double)(*p_green) - P) * saturation;
                double dblue = P + ((double)(*p_blue) - P) * saturation;

                // Clip values in 0 to 255 range
                dred = (dred < 0) ? 0 : dred;
                dgreen = (dgreen < 0) ? 0 : dgreen;
                dblue = (dblue < 0) ? 0 : dblue;

                dred = (dred > std::numeric_limits<T>::max()) ? std::numeric_limits<T>::max() : dred;
                dgreen = (dgreen > std::numeric_limits<T>::max()) ? std::numeric_limits<T>::max() : dgreen;
                dblue = (dblue > std::numeric_limits<T>::max()) ? std::numeric_limits<T>::max() : dblue;

                *p_red = (T)dred;
                *p_green = (T)dgreen;
                *p_blue = (T)dblue;
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

    if (m_colour) {
        // Colour data needs to be changed from BGR to RGB format and flipped vertically
        uint8_t *p_write_data = p_output_buffer;
        if (m_byte_depth == 1) {
            // 8-bit data
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
            // 16-bit data
            for (int32_t y = m_height - 1; y >= 0; y--) {
                uint16_t *p_read_data = ((uint16_t *)mp_buffer) + y * m_width * 3;
                for (int32_t x = 0; x < m_width; x++) {
                    uint8_t b_pixel = (*p_read_data++) >> 8;
                    uint8_t g_pixel = (*p_read_data++) >> 8;
                    uint8_t r_pixel = (*p_read_data++) >> 8;
                    *p_write_data++ = r_pixel;
                    *p_write_data++ = g_pixel;
                    *p_write_data++ = b_pixel;
                }

                for (int32_t x = 0; x < line_pad; x++) {
                    *p_write_data++ = 0;
                }
            }
        }
    } else {
        // Monochrome data just needs to be flipped vertically
        uint8_t *p_write_data = p_output_buffer;
        if (m_byte_depth == 1) {
            // 8-bit data
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
        } else {
            // 16-bit data
            for (int32_t y = m_height - 1; y >= 0; y--) {
                uint16_t *p_read_data = ((uint16_t *)mp_buffer) + y * m_width;
                for (int32_t x = 0; x < m_width; x++) {
                    uint8_t temp = (*p_read_data++) >> 8;
                    *p_write_data++ = temp;
                    *p_write_data++ = temp;
                    *p_write_data++ = temp;
                }

                for (int32_t x = 0; x < line_pad; x++) {
                    *p_write_data++ = 0;
                }
            }
        }
    }

    delete[] mp_buffer;  // Free input buffer
    mp_buffer = p_output_buffer;  // Update pointer to output buffer
    m_buffer_size = buffer_size;
}


bool c_image::debayer_image_bilinear(int32_t colour_id)
{
    if (m_byte_depth == 1) {
        // 8-bit data
        return debayer_image_bilinear_int <uint8_t> (colour_id);
    } else {
        // 16-bit data
        return debayer_image_bilinear_int <uint16_t> (colour_id);
    }
}


template <typename T>
bool c_image::debayer_image_bilinear_int(int32_t colour_id)
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
    T *raw_data_ptr;
    T *rgb_data_ptr1;

    uint32_t bayer_x = bayer_code % 2;
    uint32_t bayer_y = ((bayer_code/2) % 2) ^ (m_height % 2);

    // Buffer to create RGB image in
    T *rgb_data = (T *)new uint8_t[3 * m_width * m_height * m_byte_depth];

    // Debayer bottom line
    y = 0;
    for (x = 0; x < m_width; x++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <T> (bayer, x, y, (T *)mp_buffer, rgb_data);
    }

    // Debayer top line
    y = m_height -1;
    for (x = 0; x < m_width; x++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <T> (bayer, x, y, (T *)mp_buffer, rgb_data);
    }

    // Debayer left edge
    x = 0;
    for (y = 1; y < m_height-1; y++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <T> (bayer, x, y, (T *)mp_buffer, rgb_data);
    }

    // Debayer right edge
    x = m_width-1;
    for (y = 1; y < m_height-1; y++) {
        uint32_t bayer = ((x + bayer_x) % 2) + (2 * ((y + bayer_y) % 2));
        debayer_pixel_bilinear <T> (bayer, x, y, (T *)mp_buffer, rgb_data);
    }

    // Debayer to create blue, green and red data
    rgb_data_ptr1 = rgb_data + (3 * (m_width + 1));
    raw_data_ptr = ((T *)mp_buffer) + (m_width + 1);

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
    mp_buffer = (uint8_t *)rgb_data;
    m_colour_id = COLOURID_BGR;
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
