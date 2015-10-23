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

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include "pipp_ser.h"
#include "pipp_timestamp.h"
#include "pipp_utf8.h"

#ifdef DLL_BUILD
#include "pipp_dll.h"
#endif

#ifdef DLL_BUILD
#define printf //
#endif


using namespace std;


// ------------------------------------------
// Open SER file
// ------------------------------------------
int32_t c_pipp_ser::open(
    const char *filename,
    int32_t bpp,
    int32_t quiet)
{
    (void)quiet;  // Remove unused parameter warning
    int32_t ret;
    m_current_frame = 0;
    m_fps_rate = 0;
    m_fps_scale = 1;
    m_utc_to_local_offset = 0L;
    m_timestamp_correction_value = 0L;

    // Ensure no previous file is still open
    if (mp_ser_file != NULL) {
        fclose(mp_ser_file);  // Close file
    }

    // Open SER file
    mp_ser_file = fopen_utf8(filename, "rb");

    // Return if file did not open
    if (!mp_ser_file) {
        m_error_string += QCoreApplication::tr("Error: Could not open file '%1'", "SER file error message")
                          .arg(filename);
        m_error_string += '\n';
        return 0;
    }

    // Get file size
    fseek64(mp_ser_file, 0, SEEK_END);
    m_filesize = ftell64(mp_ser_file);
    fseek64(mp_ser_file, 0, SEEK_SET);

    if (m_filesize < (int64_t)(14 + sizeof(m_header))) {
        // File is too short to contain File ID and header
        m_error_string += QCoreApplication::tr("Error: File '%1' is too short to contain SER header", "SER File error message")
                          .arg(filename);
        m_error_string += '\n';
        mp_ser_file = NULL;
        return 0;
    }

    // Read File ID
    char file_id[15];
    ret = fread(file_id, 1, 14, mp_ser_file);
    file_id[14] = 0;

    // Read the rest of the header
    ret = fread(&m_header, 1, sizeof(m_header), mp_ser_file);

    if (m_header.frame_count <= 0) {
        // Invalid frame count
        m_error_string += QCoreApplication::tr("Error: File '%1' has an invalid frame count of %2", "SER File error message")
                          .arg(filename)
                          .arg(m_header.frame_count);
        m_error_string += "\n";
        return 0;
    }

    if (m_header.little_endian < 0 || m_header.little_endian > 1) {
        // Invalid little endian
        m_error_string += QCoreApplication::tr("Error: File '%1' has an invalid little endian value of %2", "SER File error message")
                          .arg(filename)
                          .arg(m_header.little_endian);
        m_error_string += "\n";
        return 0;
    }

    if (m_header.image_width <= 0) {
        // Invalid image width
        m_error_string += QCoreApplication::tr("Error: File '%1' has an invalid image width of %2", "SER File error message")
                          .arg(filename)
                          .arg(m_header.image_width);
        m_error_string += "\n";
        return 0;
    }

    if (m_header.image_height <= 0) {
        // Invalid image height
        m_error_string += QCoreApplication::tr("Error: File '%1' has an invalid height width of %2", "SER File error message")
                          .arg(filename)
                          .arg(m_header.image_height);
        m_error_string += "\n";
        return 0;
    }

    if (m_header.pixel_depth < 1 || m_header.pixel_depth > 16) {
        // Invalid pixel depth
        m_error_string += QCoreApplication::tr("Error: File '%1' has an invalid pixel depth of %2", "SER File error message")
                          .arg(filename)
                          .arg(m_header.pixel_depth);
        m_error_string += "\n";
        return 0;
    }

#ifndef DLL_BUILD
    // Override pixel depth if required
    if (bpp >= 8 && bpp <= 16)
    {
        m_header.pixel_depth = bpp;
    }
#endif

    // Check the pixel depth
    if (m_header.pixel_depth > 8) {
        m_bytes_per_sample = 2;
    } else {
        m_bytes_per_sample = 1;
    }

    // Decide if this is a colour or mono image (raw colour is mono!)
    switch(m_header.colour_id) {
    case COLOURID_RGB:
    case COLOURID_BGR:
        m_colour = 1;
        break;
    default:
        m_colour = 0;
        break;
    }

    int32_t total_bytes_per_sample = m_bytes_per_sample * (1 + m_colour * 2);

    // Check that the file is large enough to hold all the frames
    if (m_filesize < (m_header.frame_count * m_header.image_height * m_header.image_width * total_bytes_per_sample + 178)) {
        m_error_string += QCoreApplication::tr("Error: File '%1' is too short to hold all the frames", "SER File error message")
                          .arg(filename);
        m_error_string += "\n";
        return 0;
    }


    // Store size of frame
    m_framesize = m_header.image_width * m_header.image_height;
    if (m_header.pixel_depth > 8) {
        // 2 bytes per pixel
        m_framesize *= 2;
    }

    if (m_header.colour_id == COLOURID_RGB || m_header.colour_id == COLOURID_BGR) {
        m_framesize *= 3;
    }

    // Check for timestamps
    if (m_header.date_time[1] != 0 || m_header.date_time[0] != 0) {
        // Timestamps should exist

        // Check file is large enough to have timestamps
        if (m_filesize >= (178 + m_header.frame_count * m_header.image_height * m_header.image_width * total_bytes_per_sample + 8 * m_header.frame_count)) {

            // Get current position in file
            uint64_t start_of_image_data_pos = ftell64(mp_ser_file);

            // Seek to start of timestamps
            ret = fseek64(
                mp_ser_file,
                (int64_t)m_header.frame_count * m_header.image_height * m_header.image_width * total_bytes_per_sample,
                SEEK_CUR);

            // Get buffer to store timestamps in
            mp_timestamp = (uint64_t *)m_timestamp_buffer.get_buffer(8 * m_header.frame_count);

            // Load timestamp data into buffer
            ret = fread(mp_timestamp, 1, 8 * m_header.frame_count, mp_ser_file);

            // Debug Start
            if (ret != 8 * m_header.frame_count) {
                m_error_string += QCoreApplication::tr("Error: SER timestamp read failed for file '%1'", "SER File error message")
                                  .arg(filename);
                m_error_string += "\n";
                return 0;
            }

            // Seek back to start of image data
            fseek64(mp_ser_file, start_of_image_data_pos, SEEK_SET);

            // Analyse timestamps to ensure that they are all increasing and in order
            // Plus get earliest ts
            uint64_t first_ts = *mp_timestamp;
            uint64_t min_ts = *mp_timestamp;
            uint64_t last_ts = *(mp_timestamp + (m_header.frame_count - 1));

            uint64_t last_current_ts = 0;
            for (int32_t ts_count = 0; ts_count < m_header.frame_count; ts_count++) {
                uint64_t current_ts = *(mp_timestamp + ts_count);

                if (current_ts < min_ts) {
                    min_ts = current_ts;  // Get earliest timestamp
                }

                if (current_ts < last_current_ts) {
                    last_ts = first_ts;  // Out of order
                }

                last_current_ts = current_ts;  // This is not the last timestamp
            }

            // Check if timestamps are local time instead as universal time
            int64_t start_time_uct_minus_min_ts = (uint64_t)(m_header.date_time_utc[1]) << 32 | m_header.date_time_utc[0];
            int64_t start_time_minus_min_ts = (uint64_t)(m_header.date_time[1]) << 32 | m_header.date_time[0];
            m_utc_to_local_offset = start_time_uct_minus_min_ts - start_time_minus_min_ts;

            start_time_uct_minus_min_ts -= min_ts;
            if (start_time_uct_minus_min_ts < 0) {
                start_time_uct_minus_min_ts *= -1L;
            }

            start_time_minus_min_ts -= min_ts;
            if (start_time_minus_min_ts < 0) {
                start_time_minus_min_ts *= -1L;
            }

            if (start_time_uct_minus_min_ts <= start_time_minus_min_ts) {
                // Timestamps are in universal time
                m_timestamp_correction_value = 0L;
            } else {
                m_timestamp_correction_value = m_utc_to_local_offset;
            }

            uint64_t diff_ts = (last_ts - first_ts) / 1000;  // Now in units of 100 us

            if (diff_ts > 0) {
                // There is a positive time difference between first and last timestamps
                // We can calculate a frames per second value
                double d_fps = ((double)(m_header.frame_count - 1) * 10000) / (double)diff_ts;
                m_fps_rate = (int32_t)(d_fps * 1000.0);
                m_fps_scale = 1000;
            } else {
                // The time difference between first and last timestamps is 0 or -ve
                // No valid frames per second value can be calculated
                m_fps_rate = -1;
                m_fps_scale = 1;
            }
        } else {
            // Timestamps should be present but are not
            mp_timestamp = NULL;
        }
    } else {
        // No timestamps present
        mp_timestamp = NULL;
    }

    // Code to check m_header.pixel_depth since many software packages seem to set this incorrectly
    if (m_bytes_per_sample == 2 && m_header.frame_count > 0) {
        const int FRAMES_TO_CHECK_FOR_PIXEL_DEPTH = 10;
        int32_t pixel_depth[FRAMES_TO_CHECK_FOR_PIXEL_DEPTH];
        pixel_depth[0] = find_pixel_depth(1);  // First frame
        for (int x = 1; x < FRAMES_TO_CHECK_FOR_PIXEL_DEPTH-1; x++){  // Middle frames
            int32_t frame_to_check = (m_header.frame_count * x)/(FRAMES_TO_CHECK_FOR_PIXEL_DEPTH-1);
            if (frame_to_check == 0) {
                frame_to_check = 1;
            }

            pixel_depth[x] = find_pixel_depth(frame_to_check);
        }

        pixel_depth[FRAMES_TO_CHECK_FOR_PIXEL_DEPTH-1] = find_pixel_depth(m_header.frame_count);    // Last frame

        int32_t max_pixel_depth = pixel_depth[0];
        for (int x = 1; x < FRAMES_TO_CHECK_FOR_PIXEL_DEPTH; x++) {
            if (pixel_depth[x] > max_pixel_depth) {
                max_pixel_depth = pixel_depth[x];
            }
        }

        // Use largest pixel depth found instead of the value from the SER header field
        m_header.pixel_depth = max_pixel_depth;
    }

    return m_header.frame_count;
}


int32_t c_pipp_ser::find_pixel_depth(
    uint32_t frame_number)
{
    int32_t pixel_depth = m_header.pixel_depth;
    uint8_t *p_temp_buffer = new uint8_t[m_header.image_width * m_header.image_height * 2 * 3];
    int32_t stored_pixel_depth = m_header.pixel_depth;
    m_header.pixel_depth = 16;  // Do not shift data this time
    get_frame(frame_number, p_temp_buffer);  // Get the first frame to analyse
    m_header.pixel_depth = stored_pixel_depth;  // Restore pixel depth

    uint16_t max_pixel = 0;
    uint16_t *p_temp_ptr = (uint16_t *)p_temp_buffer;
    for (int x = 0; x < m_header.image_width * m_header.image_height * 3; x++) {
        uint16_t pixel = *p_temp_ptr++;
        if (pixel > max_pixel) {
            max_pixel = pixel;
            if (max_pixel >= 0x8000) {
                // Max pixel value is already 16-bits
                break;
            }
        }
    }

    for (int x = 15; x >= 8; x--) {
        if (max_pixel >= (1 << x)) {
            pixel_depth = x + 1;
            break;
        }
    }

    delete [] p_temp_buffer;
    return pixel_depth;
}


// ------------------------------------------
// Get size of buffer required to store frame
// ------------------------------------------
int32_t c_pipp_ser::get_buffer_size()
{
    int size = m_header.image_width * m_header.image_height * m_bytes_per_sample;

    if (m_header.colour_id == COLOURID_RGB || m_header.colour_id == COLOURID_BGR) {
        size *= 3;
    }

    return size;
}


// ------------------------------------------
// Get header strings
// ------------------------------------------
void c_pipp_ser::get_header_strings(
    char *observer_string,
    char *instrument_string,
    char *telescope_string)
{
    // Copy header strings to supplied buffers
    strncpy(observer_string, m_header.observer, 40);
    strncpy(instrument_string, m_header.instrument, 40);
    strncpy(telescope_string, m_header.telescope, 40);
}


#ifdef DLL_BUILD
// ------------------------------------------
// Get info about a SER file
// ------------------------------------------
int32_t c_pipp_ser::get_ser_info(
    const char *ser_filename,
    const char *tmp_filename)
{
    FILE *tmp_fp;
    tmp_fp = fopen_utf8(tmp_filename, "w");

    int32_t ret;

    // Output filename
    fprintf(tmp_fp, "Filename: %s\n", ser_filename);

    // Open SER file
    FILE *info_fp;
    info_fp = fopen_utf8(ser_filename, "rb");

    // Return if file did not open
    if (!info_fp) {
        fprintf(tmp_fp, "Error: Could not open file\n");
        fclose(tmp_fp);
        return -1;
    }

    // Get file size
    fseek64(info_fp, 0, SEEK_END);
    int64_t filesize = ftell64(info_fp);
    fseek64(info_fp, 0, SEEK_SET);
    fprintf(tmp_fp, "Filesize: %lld bytes.\n\n", filesize);

    if (filesize < 14 + sizeof(s_ser_header)) {
        fprintf(tmp_fp, "Error: Filesize is too short\n");
        fprintf(tmp_fp, "Filesize with SER header alone requires %d bytes\n", 14 + sizeof(s_ser_header));
        fclose(info_fp);
        fclose(tmp_fp);
        return -1;
    }

    // Read File ID
    char file_id[15];
    ret = fread(file_id, 1, 14, info_fp);
    file_id[14] = 0;

    if (ret != 14) {
        fprintf(tmp_fp, "Error: Reading from SER file (FileID) failed\n");
        fclose(info_fp);
        fclose(tmp_fp);
        return -1;
    }

    fprintf(tmp_fp, "Header Details:\n");
    fprintf(tmp_fp, " * FileID: %s\n", file_id);

    // Read the rest of the header
    s_ser_header header ;
    ret = fread(&header, 1, sizeof(header), info_fp);

    if (ret != sizeof(header)) {
        fprintf(tmp_fp, "Error: Reading from SER file (Header) failed\n");
        fclose(info_fp);
        fclose(tmp_fp);
        return -1;
    }

    fprintf(tmp_fp, " * LuID: 0x%x\n", header.lu_id);
    fprintf(tmp_fp, " * ColorID: %d ", header.colour_id);
    switch (header.colour_id) {
    case COLOURID_MONO:       fprintf(tmp_fp, "(MONO)\n"); break;
    case COLOURID_BAYER_RGGB: fprintf(tmp_fp, "(BAYER_RGGB)\n");  break;
    case COLOURID_BAYER_GRBG: fprintf(tmp_fp, "(BAYER_GRBG)\n");  break;
    case COLOURID_BAYER_GBRG: fprintf(tmp_fp, "(BAYER_GBRG)\n");  break;
    case COLOURID_BAYER_BGGR: fprintf(tmp_fp, "(BAYER_BGGR)\n");  break;
    case COLOURID_BAYER_CYYM: fprintf(tmp_fp, "(BAYER_CYYM)\n");  break;
    case COLOURID_BAYER_YCMY: fprintf(tmp_fp, "(BAYER_YCMY)\n");  break;
    case COLOURID_BAYER_YMCY: fprintf(tmp_fp, "(BAYER_YMCY)\n");  break;
    case COLOURID_BAYER_MYYC: fprintf(tmp_fp, "(BAYER_MYYC)\n");  break;
    case COLOURID_RGB:        fprintf(tmp_fp, "(RGB)\n");  break;
    case COLOURID_BGR:        fprintf(tmp_fp, "(BGR)\n");  break;
    default:  fprintf(tmp_fp, "(Unknown)\n");  break;
    }

    fprintf(tmp_fp, " * LittleEndian: %d\n", header.little_endian);
    fprintf(tmp_fp, " * ImageWidth: %d\n", header.image_width);
    fprintf(tmp_fp, " * ImageHeight: %d\n", header.image_height);
    fprintf(tmp_fp, " * PixelDepth: %d\n", header.pixel_depth);
    fprintf(tmp_fp, " * FrameCount: %d\n", header.frame_count);
    fprintf(tmp_fp, " * Observer: %.40s\n", header.observer);
    fprintf(tmp_fp, " * Instrument: %.40s\n", header.instrument);
    fprintf(tmp_fp, " * Telescope: %.40s\n", header.telescope);

    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t microsec;

    if (header.date_time[1] == 0 && header.date_time[0] == 0) {
        fprintf(tmp_fp, " * DateTime: 0x%08x%08x\n", header.date_time[1], header.date_time[0]);
    } else {
        uint64_t ts = (uint64_t)(header.date_time[1]) << 32 | header.date_time[0];
        c_pipp_timestamp::timestamp_to_date(
            ts,
            &year,
            &month,
            &day,
            &hour,
            &minute,
            &second,
            &microsec);

        fprintf(tmp_fp, " * DateTime: %02d/%02d/%04d %02d:%02d:%02d.%06d (0x%08x%08x)\n",
            day,
            month,
            year,
            hour,
            minute,
            second,
            microsec,
            header.date_time[1],
            header.date_time[0]);
    }

    if (header.date_time_utc[1] == 0 && header.date_time_utc[0] == 0) {
        fprintf(tmp_fp, " * DateTime_UTC: 0x%08x%08x\n\n", header.date_time_utc[1], header.date_time_utc[0]);
    } else {
        uint64_t ts = (uint64_t)(header.date_time_utc[1]) << 32 | header.date_time_utc[0];
        c_pipp_timestamp::timestamp_to_date(
            ts,
            &year,
            &month,
            &day,
            &hour,
            &minute,
            &second,
            &microsec);

        fprintf(tmp_fp, " * DateTime_UTC: %02d/%02d/%04d %02d:%02d:%02d.%06d (0x%08x%08x)\n",
            day,
            month,
            year,
            hour,
            minute,
            second,
            microsec,
            header.date_time_utc[1],
            header.date_time_utc[0]);
    }

    fprintf(tmp_fp, "\n");

    // Check for zero frames
    if (header.frame_count == 0) {
        fprintf(tmp_fp, "Error: FrameCount is 0\n");
        fclose(info_fp);
        fclose(tmp_fp);
        return -1;
    }

    // Calculate bytes per sample
    uint32_t bytes_per_sample;
    if (header.pixel_depth > 8) {
        bytes_per_sample = 2;
    } else {
        bytes_per_sample = 1;
    }

    // Decide if this is a colour or mono image (raw colour is mono!)
    if (header.colour_id == COLOURID_RGB || header.colour_id == COLOURID_BGR) {
        bytes_per_sample *= 3;
    }

    // Check that the file is large enough to hold all the frames
    if (filesize < (header.frame_count * header.image_height * header.image_width * bytes_per_sample + 178)) {
        fprintf(tmp_fp,"Error: Filesize is too short to hold all the frames\n");
        fclose(info_fp);
        fclose(tmp_fp);
        return -1;
    }

    // Check for timestamps
    if (header.date_time[1] != 0 || header.date_time[0] != 0) {
        // Timestamps should exist

        // Check file is large enough to have timestamps
        if (filesize < (178 + header.frame_count * header.image_height * header.image_width * bytes_per_sample + 8 * header.frame_count)) {
            fprintf(tmp_fp, "Error: Timestamps should be present but file is too short for this\n");
            fclose(info_fp);
            fclose(tmp_fp);
            return -1;
        } else {
            // Get current position in file
            uint64_t start_of_image_data_pos = ftell64(info_fp);

            // Seek to start of timestamps
            ret = fseek64(
                info_fp,
                (int64_t)header.frame_count * header.image_height * header.image_width * bytes_per_sample,
                SEEK_CUR);

            // Get a buffer to store timestamps in
            uint64_t *timestamp_buff = new uint64_t[8 * header.frame_count];

            // Load timestamp data into buffer
            ret = fread(timestamp_buff, 1, 8 * header.frame_count, info_fp);

            if (ret != 8 * header.frame_count) {
                fprintf(tmp_fp, "Error: Reading from SER file (Timestamps) failed\n");
                delete[] timestamp_buff;  // Release memory
                fclose(info_fp);
                fclose(tmp_fp);
                return -1;
            }

            fprintf(tmp_fp, "Timestamps:\n");
            // Scan through timestamp list and check for:
            // * timestamps in order
            uint64_t *timestamp_ptr = timestamp_buff;
            bool timestamps_in_order = true;
            uint64_t previous_ts = 0L;
            uint64_t min_ts = *timestamp_ptr;
            uint64_t max_ts = *timestamp_ptr;
            for (int x = 0; x < header.frame_count; x++) {
                if (*timestamp_ptr < previous_ts) {
                    // Timestamps are not in order
                    timestamps_in_order = false;
                }

                previous_ts = *timestamp_ptr;

                // Keep track of maximum timestamp value
                if (*timestamp_ptr > max_ts) {
                    max_ts = *timestamp_ptr;
                }

                // Keep track of minimum timestamp value
                if (*timestamp_ptr < min_ts) {
                    min_ts = *timestamp_ptr;
                }

                timestamp_ptr++;
            }

            // Check if timestamps are local time instead as universal time
            int64_t start_time_uct_minus_min_ts = (uint64_t)(header.date_time_utc[1]) << 32 | header.date_time_utc[0];
            start_time_uct_minus_min_ts -= min_ts;
            if (start_time_uct_minus_min_ts < 0) {
                start_time_uct_minus_min_ts *= -1L;
            }

            int64_t start_time_minus_min_ts = (uint64_t)(header.date_time[1]) << 32 | header.date_time[0];
            start_time_minus_min_ts -= min_ts;
            if (start_time_minus_min_ts < 0) {
                start_time_minus_min_ts *= -1L;
            }

            if (start_time_uct_minus_min_ts > start_time_minus_min_ts) {
                // Timestamps are in local time - yuck!
                fprintf(tmp_fp, " * Timestamps are in local time (Should be in universal time)\n");
            }

            if (timestamps_in_order) {
                if (min_ts == max_ts) {
                    fprintf(tmp_fp, " * Timestamps are all identical\n");
                } else {
                    fprintf(tmp_fp, " * Timestamps are all in order\n");
                }
            } else {
                fprintf(tmp_fp, " * Out of order timestamps detected\n");
            }

            c_pipp_timestamp::timestamp_to_date(
                min_ts,
                &year,
                &month,
                &day,
                &hour,
                &minute,
                &second,
                &microsec);

            fprintf(tmp_fp, " * Min timestamp: %02d/%02d/%04d %02d:%02d:%02d.%06d (0x%016llx)\n",
                day,
                month,
                year,
                hour,
                minute,
                second,
                microsec,
                min_ts);

            c_pipp_timestamp::timestamp_to_date(
                max_ts,
                &year,
                &month,
                &day,
                &hour,
                &minute,
                &second,
                &microsec);

            fprintf(tmp_fp, " * Max timestamp: %02d/%02d/%04d %02d:%02d:%02d.%06d (0x%016llx)\n",
                day,
                month,
                year,
                hour,
                minute,
                second,
                microsec,
                max_ts);

            // Calculate timestamp diff
            uint64_t ts_diff = max_ts - min_ts;

            int32_t diff_days;
            int32_t diff_hours;
            int32_t diff_minutes;
            int32_t diff_seconds;
            int32_t diff_microsecs;

            c_pipp_timestamp::ts_diff_to_time(
                ts_diff,
                &diff_days,
                &diff_hours,
                &diff_minutes,
                &diff_seconds,
                &diff_microsecs);

            double d_secs = ((double)diff_microsecs / 1000000.0) + diff_seconds;

            if (diff_days != 0) {
                fprintf(tmp_fp, " * Min to Max timestamp difference: %d days %d hours %d mins %f s\n", diff_days, diff_hours, diff_minutes, d_secs);
            } else if (diff_hours != 0) {
                fprintf(tmp_fp, " * Min to Max timestamp difference: %d hours %d mins %f s\n", diff_hours, diff_minutes, d_secs);
            } else if (diff_minutes != 0) {
                fprintf(tmp_fp, " * Min to Max timestamp difference: %d mins %f s\n", diff_minutes, d_secs);
            } else {
                fprintf(tmp_fp, " * Min to Max timestamp difference: %f s\n", d_secs);
            }

            if (ts_diff != 0 && header.frame_count > 1) {
                double d_fps = (double)(header.frame_count - 1) / ((double)ts_diff / (double)c_pipp_timestamp::C_SEPASECONDS_PER_SECOND);
                fprintf(tmp_fp, " * Average frames per second: %f\n", d_fps);
            }
            
            fprintf(tmp_fp, "\n");


            // Printout timestamp list
            timestamp_ptr = timestamp_buff;
            fprintf(tmp_fp, "Timestamps List:\n");
            for (int x = 0; x < header.frame_count; x++) {
                c_pipp_timestamp::timestamp_to_date(
                    *timestamp_ptr,
                    &year,
                    &month,
                    &day,
                    &hour,
                    &minute,
                    &second,
                    &microsec);
                int32_t fc_width = (int32_t)log10((double)header.frame_count) + 1;
                fprintf(
                    tmp_fp,
                    " * %.*d: %02d/%02d/%04d %02d:%02d:%02d.%06d (0x%016llx)\n",
                    fc_width,
                    x+1,
                    day,
                    month,
                    year,
                    hour,
                    minute,
                    second,
                    microsec,
                    *timestamp_ptr);
                timestamp_ptr++;
            }

            delete[] timestamp_buff;  // Release memory
        }
    }

    fclose(info_fp);
    fclose(tmp_fp);
    return 0;
}
#endif


// ------------------------------------------
// Close file
// ------------------------------------------
int32_t c_pipp_ser::close() {
    if (mp_ser_file != NULL) {
        fclose(mp_ser_file);
        mp_ser_file = NULL;
    }

    m_error_string.clear();

    return 0;
}


// ------------------------------------------
// Get error string
// ------------------------------------------
QString& c_pipp_ser::get_error_string()
{
    return m_error_string;
}


// ------------------------------------------
// Get particular frame from SER file
// ------------------------------------------
int32_t c_pipp_ser::get_frame (
    uint32_t frame_number,
    uint8_t *buffer)
{
    // Ensure frame is in range
    if (frame_number > (uint32_t)m_header.frame_count) {
        frame_number = (uint32_t)m_header.frame_count;
    }

    if (frame_number != m_current_frame + 1) {
        // This is not the next frame, seek to the correct frame
        m_current_frame = frame_number - 1;
        uint64_t offset = ((uint64_t)m_current_frame * (uint64_t)m_framesize) + 178;
        fseek64(mp_ser_file, offset, SEEK_SET);

        // Update timestamp pointer
        if (mp_timestamp != NULL) {
            mp_timestamp = (uint64_t *)(m_timestamp_buffer.get_buffer_ptr() + (8 * m_current_frame));
        }
    }

    // Actually get the frame
    return get_frame(buffer);
}


// ------------------------------------------
// Get frame from SER file
// ------------------------------------------
int32_t c_pipp_ser::get_frame (
    uint8_t *buffer)
{
    // Check that we still have frames left
    if (m_current_frame >= (uint32_t)m_header.frame_count) {
        // Out of frames
        return -1;
    }

    m_current_frame++;

     // Handle timestamps
    // Todo - ensure we have not gone past the end of the timestamp buffer
    if (mp_timestamp == NULL) {
        m_timestamp = 0L;
    } else {
        m_timestamp = *mp_timestamp++;
    }

    if (m_header.pixel_depth > 8) {
        // More than 8 bits per pixel
        if (m_header.colour_id == COLOURID_RGB) {
            // Colour RGB data

            // Skip frame if required
            if (buffer == NULL) {
                fseek64(mp_ser_file, m_header.image_width * m_header.image_height * 6 ,SEEK_CUR);
                return 0;
            }

            // Create a temp buffer and load frame from file into it
            uint16_t *temp_buffer_ptr = (uint16_t *)m_temp_buffer.get_buffer(m_header.image_width * m_header.image_height * 2 * 3);
            fread(temp_buffer_ptr, 1, m_header.image_width * m_header.image_height * 2 * 3, mp_ser_file);

            // Copy data into supplied buffer 
            uint8_t *read_ptr8;
            uint16_t *read_ptr;
            uint16_t *write_ptr = (uint16_t *)buffer;

            if (m_header.pixel_depth == 16) {
                if (m_header.little_endian == 0) {
                    // Big endian, 16-bit data
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr = temp_buffer_ptr + y * m_header.image_width * 3;
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t r = *read_ptr++;
                            uint16_t g = *read_ptr++;
                            uint16_t b = *read_ptr++;

                            *write_ptr++ = b;
                            *write_ptr++ = g;
                            *write_ptr++ = r;
                        }
                    }
                } else {
                    // Little endian, 16-bit data
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr8 = (uint8_t *)(temp_buffer_ptr + y * m_header.image_width * 3);
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t r = (*read_ptr8++) << 8;
                            r += *read_ptr8++;
                            uint16_t g = (*read_ptr8++) << 8;
                            g += *read_ptr8++;
                            uint16_t b = (*read_ptr8++) << 8;
                            b += *read_ptr8++;

                            *write_ptr++ = b;
                            *write_ptr++ = g;
                            *write_ptr++ = r;
                        }
                    }
                }
            } else {
                if (m_header.little_endian == 0) {
                    // Big endian, bits per pixel > 8 but < 16
                    uint16_t r, g, b;
                    uint32_t shift1 = 16 - m_header.pixel_depth;
                    uint32_t shift2 = m_header.pixel_depth - shift1;
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr = temp_buffer_ptr + y * m_header.image_width * 3;
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            r = *read_ptr++;
                            r = (r << shift1) + (r >> shift2);
                            g = *read_ptr++;
                            g = (g << shift1) + (g >> shift2);
                            b = *read_ptr++;
                            b = (b << shift1) + (b >> shift2);

                            *write_ptr++ = b;
                            *write_ptr++ = g;
                            *write_ptr++ = r;
                        }
                    }
                } else {
                    // Little endian, bits per pixel > 8 but < 16
                    uint32_t shift1 = 16 - m_header.pixel_depth;
                    uint32_t shift2 = m_header.pixel_depth - shift1;
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr8 = (uint8_t *)(temp_buffer_ptr + y * m_header.image_width * 3);
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t r = (*read_ptr8++) << 8;
                            r += *read_ptr8++;
                            uint16_t g = (*read_ptr8++) << 8;
                            g += *read_ptr8++;
                            uint16_t b = (*read_ptr8++) << 8;
                            b += *read_ptr8++;

                            *write_ptr++ = (b << shift1) + (b >> shift2);
                            *write_ptr++ = (g << shift1) + (g >> shift2);
                            *write_ptr++ = (r << shift1) + (r >> shift2);
                        }
                    }
                }
            }

        } else if (m_header.colour_id == COLOURID_BGR) {
            // Colour BGR data

            // Skip frame if required
            if (buffer == NULL) {
                fseek64(mp_ser_file, m_header.image_width * m_header.image_height * 6 ,SEEK_CUR);
                return 0;
            }

            // Create a temp buffer and load frame from file into it
            uint16_t *temp_buffer_ptr = (uint16_t *)m_temp_buffer.get_buffer(m_header.image_width * m_header.image_height * 2 * 3);
            fread(temp_buffer_ptr, 1, m_header.image_width * m_header.image_height * 2 * 3, mp_ser_file);

            // Copy data into supplied buffer 
            uint8_t *read_ptr8;
            uint16_t *read_ptr;
            uint16_t *write_ptr = (uint16_t *)buffer;

            if (m_header.pixel_depth == 16) {
                if (m_header.little_endian == 0) {
                    // Big endian, 16-bit data
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr = temp_buffer_ptr + y * m_header.image_width * 3;
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            *write_ptr++ = *read_ptr++;
                            *write_ptr++ = *read_ptr++;
                            *write_ptr++ = *read_ptr++;
                        }
                    }
                } else {
                    // Little endian, 16-bit data
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr8 = (uint8_t *)(temp_buffer_ptr + y * m_header.image_width * 3);
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t b = (*read_ptr8++) << 8;
                            b += *read_ptr8++;
                            uint16_t g = (*read_ptr8++) << 8;
                            g += *read_ptr8++;
                            uint16_t r = (*read_ptr8++) << 8;
                            r += *read_ptr8++;

                            *write_ptr++ = b;
                            *write_ptr++ = g;
                            *write_ptr++ = r;
                        }
                    }
                }
            } else {
                if (m_header.little_endian == 0) {
                    // Big endian, bits per pixel > 8 but < 16
                    uint16_t r, g, b;
                    uint32_t shift1 = 16 - m_header.pixel_depth;
                    uint32_t shift2 = m_header.pixel_depth - shift1;
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr = temp_buffer_ptr + y * m_header.image_width * 3;
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            b = *read_ptr++;
                            b = (b << shift1) + (b >> shift2);
                            g = *read_ptr++;
                            g = (g << shift1) + (g >> shift2);
                            r = *read_ptr++;
                            r = (r << shift1) + (r >> shift2);

                            *write_ptr++ = b;
                            *write_ptr++ = g;
                            *write_ptr++ = r;
                        }
                    }
                } else {
                    // Little endian, bits per pixel > 8 but < 16
                    uint32_t shift1 = 16 - m_header.pixel_depth;
                    uint32_t shift2 = m_header.pixel_depth - shift1;
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr8 = (uint8_t *)(temp_buffer_ptr + y * m_header.image_width * 3);
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t b = (*read_ptr8++) << 8;
                            b += *read_ptr8++;
                            b = (b << shift1) + (b >> shift2);
                            uint16_t g = (*read_ptr8++) << 8;
                            g += *read_ptr8++;
                            g = (g << shift1) + (g >> shift2);
                            uint16_t r = (*read_ptr8++) << 8;
                            r += *read_ptr8++;
                            r = (r << shift1) + (r >> shift2);

                            *write_ptr++ = b;
                            *write_ptr++ = g;
                            *write_ptr++ = r;
                        }
                    }
                }
            }

        } else {
            // Mono data

            // Skip frame if required
            if (buffer == NULL) {
                fseek64(mp_ser_file, m_header.image_width * m_header.image_height * 2 ,SEEK_CUR);
                return 0;
            }

            // Create a temp buffer and load frame from file into it
            uint16_t *temp_buffer_ptr = (uint16_t *)m_temp_buffer.get_buffer(m_header.image_width * m_header.image_height * 2);
            fread(temp_buffer_ptr, 1, m_header.image_width * m_header.image_height * 2, mp_ser_file);

            // Copy data into supplied buffer 
            uint8_t *read_ptr8;
            uint16_t *read_ptr;
            uint16_t *write_ptr = (uint16_t *)buffer;

            if (m_header.pixel_depth == 16) {
                if (m_header.little_endian == 0) {
                    // Big endian, 16-bit data
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr = temp_buffer_ptr + y * m_header.image_width;
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            *write_ptr++ = *read_ptr++;
                        }
                    }
                } else {
                    // Little endian, 16-bit data
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr8 = (uint8_t *)(temp_buffer_ptr + y * m_header.image_width);
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t value = (*read_ptr8++) << 8;
                            value += *read_ptr8++;
                            *write_ptr++ = value;
                        }
                    }
                }
            } else {
                if (m_header.little_endian == 0) {
                    // Big endian, bits per pixel > 8 but < 16
                    uint16_t value;
                    uint32_t shift1 = 16 - m_header.pixel_depth;
                    uint32_t shift2 = m_header.pixel_depth - shift1;
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr = temp_buffer_ptr + y * m_header.image_width;
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            value = *read_ptr++;
                            value = (value << shift1) + (value >> shift2);
                            *write_ptr++ = value;
                        }
                    }
                } else {
                    // Little endian, bits per pixel > 8 but < 16
                    uint32_t shift1 = 16 - m_header.pixel_depth;
                    uint32_t shift2 = m_header.pixel_depth - shift1;
                    for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                        read_ptr8 = (uint8_t *)(temp_buffer_ptr + y * m_header.image_width);
                        for (int32_t x = 0; x < m_header.image_width; x++) {
                            uint16_t value = (*read_ptr8++) << 8;
                            value += *read_ptr8++;
                            value = (value << shift1) + (value >> shift2);
                            *write_ptr++ = value;
                        }
                    }
                }
            }
        }
    } else {
        // 8 bits per pixel
        if (m_header.colour_id == COLOURID_RGB) {
            // 24-bit RGB data

            // Skip frame if required
            if (buffer == NULL) {
                fseek64(mp_ser_file, m_header.image_width * m_header.image_height * 3, SEEK_CUR);
                return 0;
            }

            // Create a temp buffer to load frame from file into 
            uint8_t *temp_buffer_ptr = m_temp_buffer.get_buffer(m_header.image_width * m_header.image_height * 3);
            fread(temp_buffer_ptr, 1, m_header.image_width * m_header.image_height * 3, mp_ser_file);

            // Copy data into supplied buffer 
            uint8_t *read_ptr;
            uint8_t *write_ptr = buffer;
            for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                read_ptr = temp_buffer_ptr + y * m_header.image_width * 3;
                for (int32_t x = 0; x < m_header.image_width; x++) {
                    uint8_t r = *read_ptr++;
                    uint8_t g = *read_ptr++;
                    uint8_t b = *read_ptr++;
                    *write_ptr++ = b;
                    *write_ptr++ = g;
                    *write_ptr++ = r;
                }
            }
        } else if (m_header.colour_id == COLOURID_BGR) {
            // 24-bit BGR data

            // Skip frame if required
            if (buffer == NULL) {
                fseek64(mp_ser_file, m_header.image_width * m_header.image_height * 3, SEEK_CUR);
                return 0;
            }

            // Create a temp buffer to load frame from file into 
            uint8_t *temp_buffer_ptr = m_temp_buffer.get_buffer(m_header.image_width * m_header.image_height * 3);
            fread(temp_buffer_ptr, 1, m_header.image_width * m_header.image_height * 3, mp_ser_file);

            // Copy data into supplied buffer 
            uint8_t *read_ptr;
            uint8_t *write_ptr = buffer;
            int32_t line_size = m_header.image_width * 3;
            for (int32_t y = m_header.image_height-1; y >= 0; y--) {
                read_ptr = temp_buffer_ptr + y * m_header.image_width * 3;
                memcpy(write_ptr, read_ptr, line_size);
                write_ptr += line_size;
            }
        } else {
            // 8-bit mono data

            // Skip frame if required
            if (buffer == NULL) {
                fseek64(mp_ser_file, m_header.image_width * m_header.image_height, SEEK_CUR);
                return 0;
            }

            // Create a temp buffer to load frame from file into 
            uint8_t *temp_buffer_ptr = m_temp_buffer.get_buffer(m_header.image_width * m_header.image_height);
            fread(temp_buffer_ptr, 1, m_header.image_width * m_header.image_height, mp_ser_file);

            // Copy data into supplied buffer 
            uint8_t *read_ptr;
            uint8_t *write_ptr = buffer;
            for (int32_t y = 0; y < m_header.image_height; y++) {
                read_ptr = temp_buffer_ptr + (m_header.image_height - 1 - y) * m_header.image_width;
                memcpy(write_ptr, read_ptr, m_header.image_width);
                write_ptr += m_header.image_width;
            }
        }
    }

    return 0;
}

