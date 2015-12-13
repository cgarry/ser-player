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

#ifndef GIF_WRITE_H
#define GIF_WRITE_H

#include <QString>
#include <cstdint>

#define GIF_COMMENT_STRING "Created by SER Player"
// Comment out GIF_COMMENT_STRING #define to disable adding a comment extension to the generated gif file


class c_gif_write {

    public:
        c_gif_write();

        // ------------------------------------------
        // Create a new GIF file
        // ------------------------------------------
        bool create(
                const QString &filename,
                int width,
                int height,
                int byte_depth,
                bool colour,
                int repeat_count);


        // ------------------------------------------
        // Write frame to GIF file
        // ------------------------------------------
        bool write_frame(
                uint8_t  *p_data,
                uint16_t display_time);


        // ------------------------------------------
        // Finish and close GIF file
        // ------------------------------------------
        bool close();



    private:
        //
        // Private functions
        //

        //
        // Private structures
        //
        struct s_gif_header {
            uint8_t m_signature[3]; // "GIF";
            uint8_t m_version[3];  // = "89a"
            uint8_t m_logical_screen_width[2];
            uint8_t m_logical_screen_height[2];
            uint8_t m_packed_fields;  // Global Color Table Flag       1 Bit
                                      // Color Resolution              3 Bits
                                      // Sort Flag                     1 Bit
                                      // Size of Global Color Table    3 Bits
            uint8_t m_background_colour_index;
            uint8_t m_pixel_aspect_ratio;
        };

        struct s_netscape_extension {
            uint8_t m_extension_label;
            uint8_t app_extension_label;
            uint8_t m_block_size;
            uint8_t m_app_identifier[8];
            uint8_t m_app_auth_code[3];
            uint8_t m_sub_block_size;
            uint8_t m_sub_block_id;
            uint8_t m_loop_count[2];
            uint8_t m_block_terminator;
        };

        struct s_graphic_control_extension {
            uint8_t m_extension_label;
            uint8_t m_graphic_control_label;
            uint8_t m_block_size;
            uint8_t m_packed_field;
            uint8_t m_delay_time[2];
            uint8_t m_transparent_colour_index;
            uint8_t m_block_terminator;
        };

        struct s_image_descriptor {
            uint8_t m_image_seperator;
            uint8_t m_image_left_position[2];
            uint8_t m_image_top_position[2];
            uint8_t m_image_width[2];
            uint8_t m_image_height[2];
            uint8_t m_packed_fields;
        };

#ifdef GIF_COMMENT_STRING
        struct s_comment_extension {
            uint8_t m_extension_label;
            uint8_t m_comment_label;
            uint8_t m_comment_length;
            uint8_t m_comment_data[sizeof(GIF_COMMENT_STRING) - 1];
            uint8_t m_block_terminator;
        };
#endif

        //
        // Private member variables
        //

        // Image details
        int m_width;
        int m_height;
        bool m_colour;
        int m_bytes_per_sample;

        // Other
        QString m_error_string;
        FILE *mp_gif_file;
        uint8_t *p_last_image;

        // GIF implementation details
        s_gif_header m_gif_header;
        s_netscape_extension m_netscape_extension;
        s_graphic_control_extension m_graphic_control_extension;
        s_image_descriptor m_image_descriptor;
#ifdef GIF_COMMENT_STRING
        s_comment_extension m_comment_extension;
        uint8_t *mp_colour_table;
#endif
};


#endif  // GIF_WRITE_H
