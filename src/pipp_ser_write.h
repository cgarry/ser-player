#ifndef PIPP_SER_WRITE_H
#define PIPP_SER_WRITE_H

#include <cstdint>
#include <memory>
#include <QString>


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


class c_pipp_ser_write {
    // ------------------------------------------
    // Private definitions
    // ------------------------------------------
    private:
#pragma pack (push, 1)
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
            uint64_t date_time;  // Date and time
            uint64_t date_time_utc;  // Date and time in UTC
        };
#pragma pack (pop)

        // Member variables
        FILE *mp_ser_file;
        std::unique_ptr<char[]> mp_index_filename;
        FILE *mp_ser_index_file;
        s_ser_header m_header;
        bool m_open;
        int32_t m_width;
        int32_t m_height;
        bool m_colour;
        int32_t m_bytes_per_sample;
        int64_t m_date_time_utc;
        bool m_file_write_error;
        bool m_big_endian_processor;


    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
    
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_pipp_ser_write();


        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        ~c_pipp_ser_write() {
        }


        // ------------------------------------------
        // Return the open state of the SER file
        // ------------------------------------------
        bool get_open () {
            return m_open;
        }


        // ------------------------------------------
        // Create a new SER file
        // ------------------------------------------
        bool create(
            const QString &filename,
            int32_t  width,
            int32_t  height,
            bool     colour,
            int32_t  byte_depth);
            

        // ------------------------------------------
        // Write frame to SER file
        // ------------------------------------------
        bool write_frame(
            uint8_t  *data,
            uint64_t timestamp);
            
          
        // ------------------------------------------
        // Set details for SER file
        // ------------------------------------------
        bool set_details(
            int32_t lu_id,
            int32_t colour_id,
            int64_t utc_to_local_diff,
            QString observer,
            QString instrument,
            QString telescope);
          
        
        // ------------------------------------------
        // Write header and close SER file
        // ------------------------------------------
        bool close();


    private:
        // ------------------------------------------
        // Private definitions
        // ------------------------------------------

        // ------------------------------------------
        // fwrite() function with error checking
        // ------------------------------------------
        void fwrite_error_check(
                const void *ptr,
                size_t size,
                size_t count,
                FILE *p_stream);


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
            p_header->date_time        = swap_endianess(p_header->date_time);
            p_header->date_time_utc    = swap_endianess(p_header->date_time_utc);
        }

};

    
#endif  // PIPP_SER_WRITE__H
