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


#ifndef PIPP_SER_H
#define PIPP_SER_H

#include <QCoreApplication>
#include <stdint.h>
#include "pipp_buffer.h"


// Codes for ColourID
#define COLOURID_MONO          0
#define COLOURID_BAYER_RGGB    8
#define COLOURID_BAYER_GRBG    9
#define COLOURID_BAYER_GBRG    10
#define COLOURID_BAYER_BGGR    11
#define COLOURID_BAYER_CYYM    16
#define COLOURID_BAYER_YCMY    17
#define COLOURID_BAYER_YMCY    18
#define COLOURID_BAYER_MYYC    19
#define COLOURID_RGB           100
#define COLOURID_BGR           101


class c_pipp_ser {
    Q_DECLARE_TR_FUNCTIONS(c_pipp_ser)

    // ------------------------------------------
    // Private definitions
    // ------------------------------------------
    private:
        struct s_ser_header {
            int32_t lu_id; // Lumenera camera series ID
            int32_t colour_id;  // Colour ID
            int32_t little_endian;  // Little Endian
            int32_t image_width;  // Image Width
            int32_t image_height;  // Image Height
            int32_t pixel_depth;  // Pixel Depth
            int32_t frame_count;  // Frame Count
            char observer[40];  // Name of observer
            char instrument[40];  // Name of camera
            char telescope[40];  // Name of telescope
            uint32_t date_time_lsw;  // Date and time - least significant word
            uint32_t date_time_msw;  // Date and time - most significant word
            uint32_t date_time_utc_lsw;  // Date and time in UTC - least significant word
            uint32_t date_time_utc_msw;  // Date and time in UTC - most significant word
        };

        // Check structure is the correct size
        static_assert (sizeof(s_ser_header) == 7 * 4 + 3 * 40 + 2 * 8, "Structure s_ser_header has not been packed");

        // Member variables
        std::string m_filename;
        FILE *mp_ser_file;
        int64_t m_filesize;
        uint32_t m_framesize_in;
        s_ser_header m_header;
        int32_t m_byte_depth_in;
        int32_t m_byte_depth_out;
        int32_t m_colour;
        uint32_t m_current_frame;
        uint64_t m_timestamp;
        int32_t m_fps_rate;  // Frame Per Second Rate
        int32_t m_fps_scale;  // Frame Per Second Scale

        c_pipp_buffer m_temp_buffer;
        c_pipp_buffer m_timestamp_buffer;
        uint64_t *mp_timestamp;
        int64_t m_timestamp_correction_value;
        int64_t m_utc_to_local_offset;
        std::string m_error_string;
        std::string m_file_id;
        bool m_big_endian_processor;
        bool m_same_data_and_processor_endian;


    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
        enum e_error_code {
            ERROR_NO_ERROR = 1,
            ERROR_ZERO_FRAME_COUNT = -1,
            ERROR_CANNOT_OPEN_FILE = -2,
            ERROR_FILE_TOO_SHORT = -3,
            ERROR_FILE_TOO_SHORT_FOR_FRAMES = -4,
            ERROR_INVALID_HEADER_VALUE = -5};
    
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_pipp_ser() :
            mp_ser_file(nullptr),
            m_filesize(0),
            m_byte_depth_in(0),
            m_byte_depth_out(0),
            m_colour(0),
            mp_timestamp(nullptr),
            m_error_string(""),
            m_same_data_and_processor_endian(false)
        {
            // Detect endianess of the processor
            m_big_endian_processor = (*(uint16_t *)"\0\xff" < 0x100);
        }


        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        ~c_pipp_ser() {
        }


        // ------------------------------------------
        // Open SER file
        // ------------------------------------------
        int32_t open(
            const std::string &filename_utf8,
            int32_t bpp,
            int32_t quiet);

        
        // ------------------------------------------
        // Close AVI file
        // ------------------------------------------
        int32_t close();


        // ------------------------------------------
        // Fix broken SER file
        // ------------------------------------------
        static int32_t fix_broken_ser_file(
            const std::string &filename_utf8);


        // ------------------------------------------
        // Get filename
        // ------------------------------------------
        std::string get_filename(){
            return m_filename;
        }



        // ------------------------------------------
        // Get size of buffer required to store frame
        // ------------------------------------------
        int32_t get_buffer_size();


        // ------------------------------------------
        // Get width
        // ------------------------------------------
        int32_t get_width() {
            return m_header.image_width;
        }


        // ------------------------------------------
        // Get height
        // ------------------------------------------
        int32_t get_height() {
            return m_header.image_height;
        }


        // ------------------------------------------
        // Get fps_rate
        // ------------------------------------------
        int32_t get_fps_rate() {
            return m_fps_rate;
        }


        // ------------------------------------------
        // Get fps_scale
        // ------------------------------------------
        int32_t get_fps_scale() {
            return m_fps_scale;
        }


        // ------------------------------------------
        // Get colour_id
        // ------------------------------------------
        int32_t get_colour_id() {
            return m_header.colour_id;
        }


        // ------------------------------------------
        // Get file_id
        // ------------------------------------------
        std::string get_file_id() {
            return m_file_id;
        }


        // ------------------------------------------
        // Get lu_id
        // ------------------------------------------
        int32_t get_lu_id() {
            return m_header.lu_id;
        }


        // ------------------------------------------
        // Get colour
        // ------------------------------------------
        int32_t get_colour() {
            return m_colour;
        }


        // ------------------------------------------
        // Get little_endian
        // ------------------------------------------
        int32_t get_little_endian() {
            return m_header.little_endian;
        }


        // ------------------------------------------
        // Get byte_depth
        // ------------------------------------------
        int32_t get_byte_depth() {
            return m_byte_depth_out;
        }


        // ------------------------------------------
        // Get pixel_depth
        // ------------------------------------------
        int32_t get_pixel_depth() {
            return m_header.pixel_depth;
        }


        // ------------------------------------------
        // Get observer strings
        // ------------------------------------------
        std::string get_observer_string();


        // ------------------------------------------
        // Get instrument strings
        // ------------------------------------------
        std::string get_instrument_string();


        // ------------------------------------------
        // Get telescope strings
        // ------------------------------------------
        std::string get_telescope_string();


        // ------------------------------------------
        // Get information about timestamps
        // ------------------------------------------
        std::string get_timestamp_info();


        // ------------------------------------------
        // Get date_time
        // ------------------------------------------
        uint64_t get_data_time()
        {
            return (uint64_t)m_header.date_time_msw << 32 | m_header.date_time_lsw;
        }


        // ------------------------------------------
        // Get date_time_utc
        // ------------------------------------------
        uint64_t get_data_time_utc()
        {
            return (uint64_t)m_header.date_time_utc_msw << 32 | m_header.date_time_utc_lsw;
        }


        // ------------------------------------------
        // Get error string
        // ------------------------------------------
        std::string get_error_string();


        // ------------------------------------------
        // Get next frame from SER file
        // ------------------------------------------
        int32_t get_frame (
            uint8_t *buffer);


        // ------------------------------------------
        // Get particular frame from SER file
        // ------------------------------------------
        int32_t get_frame (
            uint32_t frame_number,
            uint8_t *buffer);

        //
        // Return is SER file has timestamps
        //
        bool has_timestamps() {
            return mp_timestamp != nullptr;
        }


        //
        // Get timestamp from SER file
        //
        uint64_t get_timestamp() {
            return m_timestamp + m_timestamp_correction_value;
        }


        //
        // Get a specific timestamp from SER file
        //
        uint64_t get_timestamp(uint32_t frame_number) {
            uint32_t temp = frame_number;

            if (frame_number >= (uint32_t)m_header.frame_count) {
                temp = (uint32_t)m_header.frame_count - 1;
            }

            uint64_t * p_timestamp_buffer = (uint64_t *)m_timestamp_buffer.get_buffer_ptr();
            return p_timestamp_buffer[temp];
        }


        //
        // Get diff between universal time and local time
        //
        int64_t get_utc_to_local_diff() {
            return m_utc_to_local_offset;
        }


    private:
        //
        // Find pixel depth from specified frame
        //
        int32_t find_pixel_depth(
            uint32_t frame_number);

        template <typename T>
        static T swap_endianess(T data)
        {
            int32_t ret;
            uint8_t *p_read = (uint8_t *)&data;
            p_read += sizeof(data) - 1;
            uint8_t *p_write = (uint8_t *)&ret;
            for (int x = 0; x < sizeof(data); x++) {
                *p_write++ = *p_read--;
            }

            return ret;
        }

        // Change from little-endian to big-endian on big-endian systems
        static void swap_header_endianess(s_ser_header *p_header)
        {
            p_header->lu_id            = swap_endianess(p_header->lu_id);
            p_header->colour_id        = swap_endianess(p_header->colour_id);
            p_header->little_endian    = swap_endianess(p_header->little_endian);
            p_header->image_width      = swap_endianess(p_header->image_width);
            p_header->image_height     = swap_endianess(p_header->image_height);
            p_header->pixel_depth      = swap_endianess(p_header->pixel_depth);
            p_header->frame_count      = swap_endianess(p_header->frame_count);
            p_header->date_time_lsw     = swap_endianess(p_header->date_time_lsw);
            p_header->date_time_msw     = swap_endianess(p_header->date_time_msw);
            p_header->date_time_utc_lsw = swap_endianess(p_header->date_time_utc_lsw);
            p_header->date_time_utc_msw = swap_endianess(p_header->date_time_utc_msw);
        }

        static void swap_timestamps_endianess(uint64_t *p_timestamps, int32_t framecount)
        {
            for (int x = 0; x < framecount; x++) {
                *p_timestamps = swap_endianess(*p_timestamps);
                p_timestamps++;
            }
        }
};

    
#endif  // PIPP_SER_H
