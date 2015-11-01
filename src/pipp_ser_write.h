#ifndef PIPP_SER_WRITE_H
#define PIPP_SER_WRITE_H

#include <QCoreApplication>
#include <QString>
#include <cstdint>
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


class c_pipp_ser_write {
    Q_DECLARE_TR_FUNCTIONS(c_pipp_ser_write)

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
        char *mp_index_filename;
        FILE *mp_ser_index_file;
        s_ser_header m_header;
        bool m_open;
        int32_t m_width;
        int32_t m_height;
        bool m_colour;
        int32_t m_bytes_per_sample;
        int64_t m_date_time_utc;
        c_pipp_buffer m_temp_buffer;
        QString m_error_string;


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
        // Get error string
        // ------------------------------------------
        QString get_error_string()
        {
            QString ret_string = m_error_string;
            m_error_string.clear();
            return ret_string;
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
        int32_t create(
            const QString &filename,
            int32_t  width,
            int32_t  height,
            bool     colour,
            int32_t  byte_depth);
            

        // ------------------------------------------
        // Write frame to SER file
        // ------------------------------------------
        int32_t write_frame(
            uint8_t  *data,
            uint64_t timestamp);
            
          
        // ------------------------------------------
        // Set details for SER file
        // ------------------------------------------
        int32_t set_details(
            int32_t lu_id,
            int32_t colour_id,
            int64_t utc_to_local_diff,
            QString observer,
            QString instrument,
            QString telescope);
          
        
        // ------------------------------------------
        // Write header and close SER file
        // ------------------------------------------
        int32_t close();

};

    
#endif  // PIPP_SER_WRITE__H
