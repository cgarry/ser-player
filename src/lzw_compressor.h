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

#ifndef LZW_COMPRESSOR_H
#define LZW_COMPRESSOR_H

#include <QString>
#include <cstdint>

#define GIF_COMMENT_STRING "Created by SER Player"
// Comment out GIF_COMMENT_STRING #define to disable adding a comment extension to the generated gif file


class c_lzw_compressor {

    public:
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_lzw_compressor(
                uint16_t width,
                uint16_t height,
                uint16_t x_start,
                uint16_t x_end,
                uint16_t y_start,
                uint16_t y_end,
                uint8_t bit_depth,
                uint8_t *p_image_data);


        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        ~c_lzw_compressor();


        // ------------------------------------------
        // Set lossy compression details
        // ------------------------------------------
        void set_lossy_details(
                int lossy_compression_level,
                bool colour,
                uint8_t *p_index_lut,
                uint8_t *p_rev_index_lut,
                uint8_t *p_index_to_index_colour_difference_lut,
                int transparent_index);


        // ------------------------------------------
        // compress data
        // ------------------------------------------
        bool compress_data();


        // ------------------------------------------
        // Get compressed data pointer
        // ------------------------------------------
        uint8_t *get_compressed_data_ptr() {
            return p_compressed_data_buffer;
        }


    private:
        //
        // Private functions
        //

        // ------------------------------------------
        // Output code to output buffer
        // ------------------------------------------
        void output_code_to_buffer(
                uint32_t code,
                uint32_t code_length,
                uint8_t *p_output_buffer);

        //
        // Private structures
        //

        // LZW dictionary
        struct s_lzw_node
        {
            uint16_t m_next[256];
        };

        struct s_lzw_tree
        {
            struct s_lzw_node m_current[4096];
        };

        //
        // Private member variables
        //
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_x_start;
        uint16_t m_x_end;
        uint16_t m_y_start;
        uint16_t m_y_end;
        uint8_t m_bit_depth;
        uint8_t *mp_image_data;
        int m_lossy_compression_level;
        bool m_colour;
        uint8_t *mp_index_lut;
        uint8_t *mp_rev_index_lut;
        uint8_t *mp_index_to_index_colour_difference_lut;
        int m_transparent_index;
        uint8_t *p_compressed_data_buffer;

        // Special codes
        uint32_t m_clear_code;
        uint32_t m_end_of_information_code;
        uint32_t m_next_free_code;
        uint32_t m_code_length;
        uint32_t m_current_code;

        // LZW dictonary tree
        s_lzw_tree *mp_lzw_tree;

        int m_input_x;
        int m_input_y;
        int m_output_bit;
};


#endif  // LZW_COMPRESSOR_H
