#include "pipp_ser_write.h"
#include "pipp_utf8.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <QDebug>

using namespace std;


// 64-bit fseek for various platforms
#ifdef __linux__
#define fseek64 fseeko64  // Linux
#define ftell64 ftello64  // Linux
#elif defined (__APPLE__)
#define fseek64 fseeko  // OS X
#define ftell64 ftello  // OS X
#else
#define fseek64 _fseeki64  // Windows
#define ftell64 _ftelli64  // Windows
#endif


// ------------------------------------------
// Constructor
// ------------------------------------------
c_pipp_ser_write::c_pipp_ser_write() :
    mp_ser_file(nullptr),
    m_open(false),
    m_file_write_error(false)
{
    // Clear header details
    memset(&m_header, 0, sizeof(s_ser_header));
}


// ------------------------------------------
// Create a new SER file
// ------------------------------------------
bool c_pipp_ser_write::create(
    const QString &filename,
    int32_t  width,
    int32_t  height,
    bool     colour,
    int32_t  byte_depth)
{
    // Set member variables
    m_width = width;
    m_height = height;
    m_colour = colour;
    m_open = false;
    m_date_time_utc = 0L;

    m_bytes_per_sample = byte_depth;
    if (colour) {
        m_bytes_per_sample *= 3;
    }

    // Open new file
    mp_ser_file = fopen_utf8(filename.toUtf8().data(), "wb+");

    // Check file opened
    // Return if file did not open
    if (!mp_ser_file) {
        return true;
    }
    
    // Generate temp index filename
    int32_t filename_len = filename.length();
    mp_index_filename.reset(new char[filename_len + 5]);
    strcpy(mp_index_filename.get(), filename.toUtf8().constData());
    strcat(mp_index_filename.get(), ".idx");

    // Open index file
    mp_ser_index_file = fopen_utf8(mp_index_filename.get(), "wb");

    // Check file opened
    // Return if file did not open
    if (!mp_ser_index_file) {
        // Close main SER file first
        fclose(mp_ser_file);
        return true;
    }

    // Write SER FILE ID to start of the file
    fwrite_error_check("LUCAM-RECORDER" , 1 , 14 , mp_ser_file );

    // Write dummy header to file - to be overwritten later
    fwrite_error_check(&m_header, 1, sizeof(s_ser_header), mp_ser_file );

    if (m_file_write_error) {
        // There were file errors, handle them
        fclose(mp_ser_file);
        fclose(mp_ser_index_file);
    } else {
        m_open = true;
    }

    bool ret = m_file_write_error;
    m_file_write_error = false;
    return ret;
}


// ------------------------------------------
// Write frame to SER file
// ------------------------------------------
bool c_pipp_ser_write::write_frame(
    uint8_t *data,
    uint64_t timestamp)
{
    // Early return if the file is not open
    if (!m_open) {
        return true;
    }

    // Grab first timestamp
    if (m_header.frame_count == 0) {
        m_date_time_utc = timestamp;
    }

    // Generate buffer
    std::unique_ptr<uint8_t[]> p_buffer(new uint8_t[m_width * m_height * m_bytes_per_sample]);

    if (m_bytes_per_sample == 1) {
        // 8-bit mono data
        uint8_t *write_ptr = p_buffer.get();
        uint8_t *read_ptr;

        for (int32_t y = m_height-1; y >= 0; y--) {
            read_ptr = data + (y * m_width);
            memcpy(write_ptr, read_ptr, m_width);
            write_ptr += m_width;
        }
    } else if (m_bytes_per_sample == 2) {
        // 16-bit mono data write
        uint16_t *write_ptr = (uint16_t *)p_buffer.get();
        uint16_t *read_ptr;
        uint16_t *data_16 = (uint16_t *)data;
        for (int32_t y = m_height-1; y >= 0; y--) {
            read_ptr = data_16 + (y * m_width);
            memcpy(write_ptr, read_ptr, m_width * 2);
            write_ptr += m_width;
        }
    } else if (m_bytes_per_sample == 3) {
        // 24-bit colour data
        uint8_t *write_ptr = p_buffer.get();
        uint8_t *read_ptr;

        for (int32_t y = m_height-1; y >= 0; y--) {
            read_ptr = data + (y * m_width * 3);
            memcpy(write_ptr, read_ptr, m_width * 3);
            write_ptr += m_width * 3;
        }
    } else if (m_bytes_per_sample == 6) {
        // 48-bit colour data
        uint16_t *write_ptr = (uint16_t *)p_buffer.get();
        uint16_t *read_ptr;
        uint16_t *data_16 = (uint16_t *)data;
        for (int32_t y = m_height-1; y >= 0; y--) {
            read_ptr = data_16 + y * m_width * 3;
            memcpy(write_ptr, read_ptr, m_width * 3 * 2);
            write_ptr += m_width * 3;
        }
    }

    fwrite_error_check(p_buffer.get(), 1, m_width * m_height * m_bytes_per_sample, mp_ser_file );
    p_buffer.reset(nullptr);

    if (m_date_time_utc != 0) {
        // Write timestamp to temp timestamp file
        fwrite_error_check(&timestamp, 8, 1, mp_ser_index_file);
    }

    // Increment frame count
    m_header.frame_count++;

    // Tidy up after write failures
    if (m_file_write_error) {
        fclose(mp_ser_file);
        fclose(mp_ser_index_file);
        m_open = false;
    }

    bool ret = m_file_write_error;
    m_file_write_error = false;
    return ret;
}


// ------------------------------------------
// Set details for SER file
// ------------------------------------------
bool c_pipp_ser_write::set_details(
    int32_t lu_id,
    int32_t colour_id,
    int64_t utc_to_local_diff,
    QString observer,
    QString instrument,
    QString telescope)
{
    m_header.lu_id = lu_id;
    m_header.little_endian = 0;
    m_header.image_width = m_width;
    m_header.image_height = m_height;
    if (!m_colour) {
        m_header.pixel_depth = 8 * m_bytes_per_sample;
        if (colour_id >= 0) {
            m_header.colour_id = colour_id;  // Keep original colour_id from SER file
        } else {
            m_header.colour_id = 0;  // There is no original colour_id
        }
    } else {
        m_header.pixel_depth = (8 * m_bytes_per_sample) / 3; 
        m_header.colour_id = COLOURID_BGR;  // We only support this value for colour files
    }

    m_header.date_time = m_date_time_utc - utc_to_local_diff;
    m_header.date_time_utc = m_date_time_utc;

    memset(m_header.observer, 0, 40);
    memcpy(m_header.observer, observer.toUtf8().constData(), 40);
    memset(m_header.instrument, 0, 40);
    memcpy(m_header.instrument, instrument.toUtf8().constData(), 40);
    memset(m_header.telescope, 0, 40);
    memcpy(m_header.telescope, telescope.toUtf8().constData(), 40);

    return false;
}


// ------------------------------------------
// Write header and close AVI file
// ------------------------------------------
bool c_pipp_ser_write::close()
{
    if (m_open) {
        // Close index file which may be empty
        fclose(mp_ser_index_file);

        // Open index file to read if we are using indexes
        if (m_date_time_utc != 0) {
            // Open index file to read
            mp_ser_index_file = fopen_utf8(mp_index_filename.get(), "rb");

            // Get file size
            fseek64(mp_ser_index_file, 0, SEEK_END);
            uint64_t filesize = ftell64(mp_ser_index_file);
            fseek64(mp_ser_index_file, 0, SEEK_SET);

            // Get buffer to store index in
            std::unique_ptr<uint8_t[]> p_buffer(new uint8_t[(uint32_t)filesize]);

            // Read data into buffer
            fread(p_buffer.get(), 1, (uint32_t)filesize, mp_ser_index_file);
            fclose(mp_ser_index_file);

            // Write index data to output file
            fwrite_error_check(p_buffer.get(), 1, (uint32_t)filesize, mp_ser_file);
            p_buffer.reset(nullptr);
        }

        // Close index file and remove it
        remove_utf8(mp_index_filename.get());

        // Goto start of file after SER FILE ID field
        fseek64(mp_ser_file, 14, SEEK_SET);

        // Write header to file
        fwrite_error_check(&m_header, 1, sizeof(s_ser_header), mp_ser_file );

        // Note that the SER file is closed
        m_open = false;

        fclose(mp_ser_file);
        mp_ser_file = nullptr;
    }

    // Release filename memory
    mp_index_filename.reset(nullptr);

    bool ret = m_file_write_error;
    m_file_write_error = false;
    return ret;
}


// ------------------------------------------
// fwrite() function with error checking
// ------------------------------------------
// ------------------------------------------
// fwrite() function with error checking
// ------------------------------------------
void c_pipp_ser_write::fwrite_error_check(
    const void *ptr,
    size_t size,
    size_t count,
    FILE *p_stream)
{
    if (!m_file_write_error) {  // Do not continue writing after an error has occured
        size_t size_written = fwrite(ptr, size, count, p_stream);
        if (size_written != count) {
            m_file_write_error = true;
        }
    }
}
