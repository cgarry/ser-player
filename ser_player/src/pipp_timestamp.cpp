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


#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <ctime>
#include "pipp_timestamp.h"


//
// Convert timestamp difference to real time
//
void c_pipp_timestamp::ts_diff_to_time(
    uint64_t ts_diff,
    int32_t *p_days,
    int32_t *p_hours,
    int32_t *p_minutes,
    int32_t *p_seconds,
    int32_t *p_microsecs) 
{
    uint64_t diff = ts_diff;

    *p_days = (int32_t)(diff / m_septaseconds_per_day);
    diff = diff % m_septaseconds_per_day;
    *p_hours = (int32_t)(diff / m_septaseconds_per_hour);
    diff = diff % m_septaseconds_per_hour;
    *p_minutes = (int32_t)(diff / m_septaseconds_per_minute);
    diff = diff % m_septaseconds_per_minute;
    *p_seconds = (int32_t)(diff / C_SEPASECONDS_PER_SECOND);
    diff = diff % C_SEPASECONDS_PER_SECOND;
    *p_microsecs = (int32_t)(diff / m_sepaseconds_per_microsecond);
}


//
// Convert timestamp to real time
//
void c_pipp_timestamp::timestamp_to_date(
    uint64_t ts,
    int32_t *p_year,
    int32_t *p_month,
    int32_t *p_day,
    int32_t *p_hour,
    int32_t *p_minute,
    int32_t *p_second,
    int32_t *p_microsec)
{
    // Get time part of timestamp
    uint64_t time_ts = ts % m_septaseconds_per_day;
    uint64_t hours = time_ts / m_septaseconds_per_hour;
    uint64_t minutes = (time_ts % m_septaseconds_per_hour) / m_septaseconds_per_minute;
    uint64_t seconds = (time_ts % m_septaseconds_per_minute) / C_SEPASECONDS_PER_SECOND;
    uint64_t microsecs = (time_ts % C_SEPASECONDS_PER_SECOND) / m_sepaseconds_per_microsecond;

    // Get the year
    uint64_t days_in_ts = ts / m_septaseconds_per_day;
    uint32_t year;

    for (year = 1; year < 9999; year += 400) {
        if (days_in_ts >= m_days_in_400_years) {
            // We have enough days for this 400 year chunk
            days_in_ts -= m_days_in_400_years;
        } else {
            // Not enough days for this 400 year chunk
            break;  // Exit this loop
        }
    }

    for ( ; year < 9999; year++) {
        uint32_t days_this_year = 365;
        if (is_leap_year(year)) {
            days_this_year = 366;
        }

        if (days_in_ts >= days_this_year) {
            // There are enough days left for this year
            days_in_ts -= days_this_year;
        } else {
            // Not enough days for the full year, this is the year!
            break;  // Exit from loop
        }
    }

    // Get the month
    uint32_t month;
    for (month = 1; month <= 12; month++) {
        uint32_t days_this_month = 31;
        switch (month) {
        case 4:   // April
        case 6:   // June
        case 9:   // September
        case 11:  // Novenber
            days_this_month = 30;
            break;
        case 2:  // Feburary
            if (is_leap_year(year)) {
                days_this_month = 29;
            } else {
                days_this_month = 28;
            }

            break;
        }

        if (days_in_ts >= days_this_month) {
            // There are enough days for this month
            days_in_ts -= days_this_month;
        } else {
            // Not enough days for this complete month, this is the month!
            break;  // Exit from loop
        } 
    }

    uint32_t day = (uint32_t)days_in_ts + 1;
    
    // Fill in supplied fields
    *p_year     = year;
    *p_month    = month;
    *p_day      = day;
    *p_hour     = (int32_t)hours;
    *p_minute   = (int32_t)minutes;
    *p_second   = (int32_t)seconds;
    *p_microsec = (int32_t)microsecs;
}


//
// Convert real time to timestamp
//
void c_pipp_timestamp::date_to_timestamp(
    int32_t year,
    int32_t month,
    int32_t day,
    int32_t hour,
    int32_t minute,
    int32_t second,
    int32_t microsec,
    uint64_t *p_ts)
{
    uint64_t ts = 0;
    int32_t yr;

    // Add 400 year blocks 
    for (yr = 1; yr < (year - 400); yr += 400) {
        ts += m_septaseconds_per_400_years;
    }

    // Add 1 years
    for ( ; yr < year; yr++) {
        uint32_t days_this_year = 365;
        if (is_leap_year(yr)) {
            days_this_year = 366;
        }

        ts += (days_this_year * m_septaseconds_per_day);
    }

    // Add months
    for (int mon = 1; mon < month; mon++) {
        switch (mon) {
        case 4:   // April
        case 6:   // June
        case 9:   // September
        case 11:  // Novenber
            ts += (30 * m_septaseconds_per_day);
            break;
        case 2:  // Feburary
            if (is_leap_year(year)) {
                ts += (29 * m_septaseconds_per_day);
            } else {
                ts += (28 * m_septaseconds_per_day);
            }

            break;
        default:
            ts += (31 * m_septaseconds_per_day);
            break;
        }
    }

    // Add days
    ts += ((day - 1) * m_septaseconds_per_day);

    // Add hours
    ts += (hour * m_septaseconds_per_hour);

    // Add minutes
    ts += (minute * m_septaseconds_per_minute);

    // Add seconds
    ts += (second * C_SEPASECONDS_PER_SECOND);

    // Micro seconds
    ts += (microsec * m_sepaseconds_per_microsecond);

    // Output result
    *p_ts = ts;
}


//
// Convert filepath to timestamp
//
int32_t c_pipp_timestamp::filepath_to_timestamp(
    bool lt2ut,
    std::string const &date_time_mask,
    std::string const &filepath,
    uint64_t *p_ts)
{
    // By default return 0 timestamp
    *p_ts = 0L;

    // Check for no supplied date/time mask
    if (date_time_mask.length() == 0) {
        return -1;
    }

    const char *mask = date_time_mask.c_str();
    char temp[1024];
    
    // Copy filepath to a temp area
    strcpy(temp, filepath.c_str());

    // Replace all '\'s with '/'s
    for (uint32_t x = 0; x < strlen(temp); x++) {
        if (temp[x] == '\\') {
            temp[x] = '/';
        }
    }

    // Split complete filename into filepath and filename
    char *char_ptr = strrchr(temp, '/');
    if (char_ptr == NULL) {
        // There is no '/' in string so assume no path
        char_ptr = temp;
    } else {
        // There is a '/' in the filename
        // Point to next character
        char_ptr++;
    }

    // Count the number of field in the mask
    int32_t year_count = 0;
    int32_t month_count = 0;
    int32_t day_count = 0;
    int32_t hour_count = 0;
    int32_t minute_count = 0;
    int32_t second_count = 0;
    int32_t time_count = 0;
    int32_t N_count = 0;
    for (uint32_t x = 0; x < strlen(mask); x++) {
        switch (mask[x]) {
        case 'Y':
            year_count++;
            break;
        case 'M':
            month_count++;
            break;
        case 'D':
            day_count++;
            break;
        case 'h':
            hour_count++;
            break;
        case 'm':
            minute_count++;
            break;
        case 's':
            second_count++;
            break;
        case 't':
            time_count++;
            break;
        case 'N':
            N_count++;
            break;
        }
    }

    // Check the required characters present in the date+time mask
    if (year_count != 2 && year_count != 4) {
        printf("Error: Require 2 or 4 year characters, %d found\n", year_count);
        return -1;
    }

    if (month_count != 2) {
        printf("Error: Require 2 month characters, %d found\n", month_count);
        return -1;
    }

    if (day_count != 2) {
        printf("Error: Require 2 day characters, %d found\n", day_count);
        return -1;
    }

    if (hour_count != 2) {
        printf("Error: Require 2 hour characters, %d found\n", hour_count);
        return -1;
    }

    if (minute_count != 2) {
        printf("Error: Require 2 minute characters, %d found\n", minute_count);
        return -1;
    }

    if (second_count != 2 && time_count != 1) {
        printf("Error: Require either 2 second characters (%d found) or 1 fraction of a minute character (%d found)\n", second_count, time_count);
        return -1;
    }

    if (N_count > 1) {
        printf("Error: Only 1 N character can be used in the time/date mask (%d found)\n", N_count);
        return -1;
    }

    // Scan through filename and find first position that matches the mask
    int32_t y_count = year_count;
    int32_t m_count = month_count;
    int32_t d_count = day_count;
    int32_t h_count = hour_count;
    int32_t min_count = minute_count;
    int32_t s_count = second_count;
    bool match_failed;

    // Copy mask into new string, expanding N into multiple times
    int32_t n_repeats = 0;
    int32_t exit_loop = 0;
    char new_mask[1024];

    do {
        n_repeats++;

        if ((date_time_mask.length() + n_repeats + 1) > filepath.length())
        {
            // Mask is longer than filename - no match
            return -1;
        }

        int pos = 0;
        for (uint32_t x = 0; x <= date_time_mask.length(); x++) {
            if (date_time_mask[x] == 'N') {
                for (int32_t repeats = 0; repeats < n_repeats; repeats++) {
                  new_mask[pos++] = 'n';
                }
            } else {
                new_mask[pos++] = date_time_mask[x];
            }
        }
    
        // Scan through filename and find first position that matches the mask
        y_count = year_count;
        m_count = month_count;
        d_count = day_count;
        h_count = hour_count;
        min_count = minute_count;
        s_count = second_count;

        if (strlen(char_ptr) < strlen(new_mask)) {
            if ( N_count == 0) {
              return -1;
            }

            break;
        }

        for (uint32_t x = 0; x < strlen(char_ptr) - strlen(new_mask); x++) {
            match_failed = false;
            for (uint32_t y = 0; y < strlen(new_mask); y++) {
                if (new_mask[y] == 'Y') {
                    if (y_count-- == 4) {
                        if (char_ptr[x + y] < '1' || char_ptr[x + y] > '2') {
                            // Not a valid year character
                            match_failed = true;
                            break;
                        }
                    } else {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                            // Not a valid year character
                            match_failed = true;
                            break;
                        }
                    }
                } else if (new_mask[y] == 'M') {
                    if (m_count-- == 2) {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '1') {
                            // Not a valid major month character
                            match_failed = true;
                            break;
                        }
                    } else {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                            // Not a valid minor month character
                            match_failed = true;
                            break;
                        }
                    }
                } else if (new_mask[y] == 'D') {
                    if (d_count-- == 2) {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '3') {
                            // Not a valid major day character
                            match_failed = true;
                            break;
                        }
                    } else {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                            // Not a valid minor day character
                            match_failed = true;
                            break;
                        }
                    }
                } else if (new_mask[y] == 'h') {
                    if (h_count-- == 2) {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '2') {
                            // Not a valid major hour character
                            match_failed = true;
                            break;
                        }
                    } else {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                            // Not a valid minor hour character
                            match_failed = true;
                            break;
                        }
                    }
                } else if (new_mask[y] == 'm') {
                    if (min_count-- == 2) {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '5') {
                            // Not a valid major minute character
                            match_failed = true;
                            break;
                        }
                    } else {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                            // Not a valid minor minute character
                            match_failed = true;
                            break;
                        }
                    }
                } else if (new_mask[y] == 's') {
                    if (s_count-- == 2) {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '5') {
                            // Not a valid major second character
                            match_failed = true;
                            break;
                        }
                    } else {
                        if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                            // Not a valid minor second character
                            match_failed = true;
                            break;
                        }
                    }
                } else if (new_mask[y] == 't') {
                    if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                        // Not a valid fraction of a minute character
                        match_failed = true;
                        break;
                    }
                } else if (new_mask[y] == 'n') {
                    if (char_ptr[x + y] < '0' || char_ptr[x + y] > '9') {
                        // Not a valid digit character
                        match_failed = true;
                        break;
                    }
                } else if (new_mask[y] == '?') {
                    // Any character
                    break;
                } else {
                    // Spacer character
                    if (char_ptr[x + y] != new_mask[y]) {
                        match_failed = true;
                        break;
                    }
                }
            }

            if (match_failed == false) {
                // Match did not fail, therefore it passed
                // Break out of loop
                char_ptr = char_ptr + x;
                exit_loop = 1;
                break;
            }
        }

        if (match_failed == true) {
            if ( N_count == 0) {
              return -1;
            }

            continue;
        }
    } while (exit_loop == 0 && N_count != 0);

    y_count = year_count;
    m_count = month_count;
    d_count = day_count;
    h_count = hour_count;
    min_count = minute_count;
    struct tm datetime;

    datetime.tm_year = 0;
    datetime.tm_mon = 0;
    datetime.tm_mday = 0;
    datetime.tm_hour = 0;
    datetime.tm_min = 0;
    datetime.tm_sec = 0;

    datetime.tm_isdst = -1;  // Auto detect daylight savings time

    s_count = second_count;
    uint32_t time_fraction = 0;
    for (uint32_t x = 0; x < strlen(new_mask); x++) {
        switch (new_mask[x]) {
        case 'Y':
            datetime.tm_year += (int)(pow(10.0, y_count - 1)) * (char_ptr[x] - '0');
            y_count--;
            break;
        case 'M':
            datetime.tm_mon += (int)(pow(10.0, m_count - 1)) * (char_ptr[x] - '0');
            m_count--;
            break;
        case 'D':
            datetime.tm_mday += (int)(pow(10.0, d_count - 1)) * (char_ptr[x] - '0');
            d_count--;
            break;
        case 'h':
            datetime.tm_hour += (int)(pow(10.0, h_count - 1)) * (char_ptr[x] - '0');
            h_count--;
            break;
        case 'm':
            datetime.tm_min += (int)(pow(10.0, min_count - 1)) * (char_ptr[x] - '0');
            min_count--;
            break;
        case 's':
            datetime.tm_sec += (int)(pow(10.0, s_count - 1)) * (char_ptr[x] - '0');
            s_count--;
            break;
        case 't':
            time_fraction = char_ptr[x] - '0';
            break;
        }
    }

    if (time_fraction != 0) {
        datetime.tm_sec = (60 * time_fraction) / 10;
    }

    // Handle 2 character dates with a best guess at the upper 2 characters
    if (datetime.tm_year < 70) {
        datetime.tm_year += 2000;
    } else if (datetime.tm_year < 100) {
        datetime.tm_year += 1900;
    }
/*
    printf("Year: %d, Month: %d, Day: %d, Hour: %d, Min: %d, Sec: %d\n",
        datetime.tm_year,
        datetime.tm_mon,
        datetime.tm_mday,
        datetime.tm_hour,
        datetime.tm_min,
        datetime.tm_sec);
*/
    // Subtract 1900 to convert to the correct format
    datetime.tm_year -= 1900;

    // Subtract 1 from fields that are not naturally zero indexed
    datetime.tm_mon--;

    // Make time from filled in structures
    mktime(&datetime);

    // Normalise the time structure to UTC time
    time_t rawtime = mktime(&datetime);
    struct tm *gm_datetime;// = gmtime(&rawtime);
    if (!lt2ut) {
    // Get date and time as it was in the original filename
        gm_datetime = localtime(&rawtime);
    } else {
    // Get UTC version of date and time
        gm_datetime = gmtime(&rawtime);
    }

/*
    printf("Year: %d, Month: %d, Day: %d, Hour: %d, Min: %d, Sec: %d\n",
        gm_datetime->tm_year + 1900,
        gm_datetime->tm_mon + 1,
        gm_datetime->tm_mday,
        gm_datetime->tm_hour,
        gm_datetime->tm_min,
        gm_datetime->tm_sec);
*/
    date_to_timestamp(
        gm_datetime->tm_year + 1900,
        gm_datetime->tm_mon + 1,
        gm_datetime->tm_mday,
        gm_datetime->tm_hour,
        gm_datetime->tm_min,
        gm_datetime->tm_sec,
        0,
        p_ts);

    return 0;
}


//
// Calculate if a year is a leap yer
//
bool c_pipp_timestamp::is_leap_year(uint32_t year)
{
    if ((year % 400) == 0) {
        // If year is divisible by 400 then is_leap_year
        return true;
    } else if ((year % 100) == 0) {
        // Else if year is divisible by 100 then not_leap_year
        return false;
    } else if ((year % 4) == 0) {
        // Else if year is divisible by 4 then is_leap_year
        return true;
    } else {
        // Else not_leap_year
        return false;
    }
}


//
// Generate WinJUPOS friendly timestamp
//
const char *c_pipp_timestamp::get_winjupos_friendly_prefix(
    uint64_t timestamp)
{
    // Get memory for prefic string
    char *winjupos_friendly_prefix = new char[sizeof("2012-11-25-1512_8-") + 1];

    // Round timestamp up to nearest part minute if required
    uint64_t temp = timestamp % m_septaseconds_per_part_minute;

    uint64_t rounded_timestamp = timestamp - temp;
    if (temp >= (m_septaseconds_per_part_minute / 2)) {
        // Increment to next part-minute
        rounded_timestamp += m_septaseconds_per_part_minute;
    }

    // Convert timestamp to real time
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t microsec;

    timestamp_to_date(
        rounded_timestamp,
        &year,
        &month,
        &day,
        &hour,
        &minute,
        &second,
        &microsec);

    int32_t part_minute = (second * 10) / 60;

    sprintf(winjupos_friendly_prefix,
        "%04d-%02d-%02d-%02d%02d_%01d-",
        year,
        month,
        day,
        hour,
        minute,
        part_minute);

    return winjupos_friendly_prefix;
}
