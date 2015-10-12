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

#ifndef IMAGE_FUNCTIONS_H
#define IMAGE_FUNCTIONS_H

#include <stdint.h>


namespace image_functions {
    struct s_image_details {
        int32_t width;
        int32_t height;
//        int32_t bytes_per_sample;
        int32_t colour_id;
        uint8_t *p_buffer;
    };


    void convert_image_to_8bit(
         struct s_image_details &image_details);

    bool debayer_image_bilinear(
        struct s_image_details &image_details);

    void estimate_colour_balance(
        double &red_gain,
        double &green_gain,
        double &blue_gain,
        const struct s_image_details &image_details);

    void set_colour_balance_luts(
        double red_gain,
        double green_gain,
        double blue_gain);


    void change_colour_balance(
        const struct s_image_details &image_details);

    void change_colour_balance(
        double red,
        double green,
        double blue,
        const struct s_image_details &image_details);

    void change_colour_saturation(
        double change,
        const struct s_image_details &image_details);

    void conv_data_ready_for_qimage(
        bool image_debayered,
        struct s_image_details &image_details);
}


#endif // IMAGE_FUNCTIONS_H
