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


#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stddef.h>



class c_image {
    private:
        // Member variables
        int32_t m_width;
        int32_t m_height;
        int32_t m_byte_depth;
        int32_t m_colour_id;
        bool m_colour;
        uint8_t *mp_buffer;
        int32_t m_buffer_size;
        uint8_t m_colbal_r_lut[256];
        uint8_t m_colbal_g_lut[256];
        uint8_t m_colbal_b_lut[256];
        bool m_colour_balance_enabled;
        double m_red_gain;
        double m_green_gain;
        double m_blue_gain;
        double m_gain;
        double m_gamma;


    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
    
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_image() :
            m_width(10),
            m_height(10),
            m_byte_depth(1),
            m_colour_id(0),
            m_colour(false),
            mp_buffer(NULL),
            m_buffer_size(0),
            m_colour_balance_enabled(false),
            m_red_gain(1.0),
            m_green_gain(1.0),
            m_blue_gain(1.0),
            m_gain(1.0),
            m_gamma(1.0)
        {
        }



        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        ~c_image() {
            delete [] mp_buffer;
        }

        
        void set_image_details(int32_t width,
                               int32_t height,
                               int32_t byte_depth,
                               int32_t colour_id,
                               bool colour);
                      

        int32_t get_width()
        {
            return m_width;
        }
        
        
        int32_t get_height()
        {
            return m_height;
        }

        int32_t get_colour_id()
        {
            return m_colour_id;
        }

        bool get_colour()
        {
            return m_colour;
        }


        int32_t get_byte_depth()
        {
            return m_byte_depth;
        }


        uint8_t *get_p_buffer()
        {
            return mp_buffer;
        }

        
        void convert_image_to_8bit();

        bool debayer_image_bilinear(int32_t colour_id);
        
        void estimate_colour_balance(
            double &red_gain,
            double &green_gain,
            double &blue_gain);


        void set_gain(
                double gain);


        void set_gamma(
                double gamma);


        void monochrome_conversion(int conv_type);
            
            
        void set_colour_balance(
            double red_gain,
            double green_gain,
            double blue_gain);


        void change_colour_balance();


        void change_colour_saturation(
            double saturation);

        void conv_data_ready_for_qimage();
        
        
    private:
        void set_buffer_size(int32_t size);
        void setup_luts();

        template <typename T>
        void debayer_pixel_bilinear(
            uint32_t bayer,
            int32_t x,
            int32_t y,
            T *raw_data,
            T *rgb_data);
};

    
#endif  // IMAGE_H
