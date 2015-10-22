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


#ifndef PERSISTENT_DATA_H
#define PERSISTENT_DATA_H

#include <QString>


class c_persistent_data
{
public:
    //
    // Persistent data
    //
    static uint m_last_ver_check_time;
    static QString m_selected_language;
    static QString m_ser_directory;
    static QString m_new_version;
    static QString m_last_save_folder;
    static bool m_check_for_updates;
    static bool m_enable_debayering;
    static bool m_repeat;
    static int m_play_direction;
    static bool m_markers_enabled;


    //
    // Load persistent data from file
    //
    static void load();


    //
    // Save persistent data to file
    //
    static void save();
};


#endif  // PERSISTENT_DATA_H
