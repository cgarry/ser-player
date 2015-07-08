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


#ifndef PIPP_TIMESTAMP_H
#define PIPP_TIMESTAMP_H

#include <cstdint>
#include <string>


class c_pipp_timestamp {
    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
        // Public constants
        static const uint64_t C_SEPASECONDS_PER_SECOND = 10000000;

    //
    // Convert timestamp difference to real time
    //
    static void ts_diff_to_time(
        uint64_t ts_diff,
        int32_t *p_days,
        int32_t *p_hours,
        int32_t *p_minutes,
        int32_t *p_seconds,
        int32_t *p_microsecs);


    //
    // Convert timestamp to real time
    //
    static void timestamp_to_date(
        uint64_t ts,
        int32_t *p_year,
        int32_t *p_month,
        int32_t *p_day,
        int32_t *p_hour,
        int32_t *p_minute,
        int32_t *p_second,
        int32_t *p_microsec);


    //
    // Convert real time to timestamp
    //
    static void date_to_timestamp(
        int32_t year,
        int32_t month,
        int32_t day,
        int32_t hour,
        int32_t minute,
        int32_t second,
        int32_t microsec,
        uint64_t *p_ts);


    //
    // Convert filepath to timestamp
    //
    static int32_t filepath_to_timestamp(
        bool lt2ut,
        std::string const &date_time_mask,
        std::string const &filepath,
        uint64_t *p_ts);


    //
    // Generate WinJUPOS friendly timestamp
    //
    static const char *get_winjupos_friendly_prefix(
        uint64_t timestamp);


    // ------------------------------------------
    // Private definitions
    // ------------------------------------------
    private:
        //
        // Calculate if a year is a leap yer
        ///
        static bool is_leap_year(uint32_t year);

        // Constants
        static const uint64_t m_sepaseconds_per_microsecond = 10;
        static const uint64_t m_septaseconds_per_part_minute = C_SEPASECONDS_PER_SECOND * 6;
        static const uint64_t m_septaseconds_per_minute = C_SEPASECONDS_PER_SECOND * 60;
        static const uint64_t m_septaseconds_per_hour = C_SEPASECONDS_PER_SECOND * 60 * 60;
        static const uint64_t m_septaseconds_per_day = m_septaseconds_per_hour * 24;
        static const uint32_t m_days_in_400_years = 303 * 365 + 97 * 366;
        static const uint64_t m_septaseconds_per_400_years = m_days_in_400_years * m_septaseconds_per_day;
};


#endif  // PIPP_TIMESTAMP_H
