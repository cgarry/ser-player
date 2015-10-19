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
#include <QSettings>
#include "persistent_data.h"

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif


//
// Default values, used only on first run
//

uint c_persistent_data::m_last_ver_check_time = 0;

QString c_persistent_data::m_selected_language = "auto";

#if QT_VERSION >= 0x050000
QString c_persistent_data::m_ser_directory = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
#else
QString c_persistent_data::m_ser_directory = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif

QString c_persistent_data::m_new_version = "v1.0.0";

bool c_persistent_data::m_check_for_updates = true;
bool c_persistent_data::m_enable_debayering = true;
bool c_persistent_data::m_hide_markers = false;
bool c_persistent_data::m_repeat = false;


//
// Load persistent data
//
void c_persistent_data::load()
{
    QSettings settings;
    if (settings.value("last_ver_check_time") != QVariant::Invalid) {
        m_last_ver_check_time = settings.value("last_ver_check_time").toUInt();
    }

    if (settings.value("selected_language") != QVariant::Invalid) {
        m_selected_language = settings.value("selected_language").toString();
    }

    if (settings.value("ser_directory") != QVariant::Invalid) {
        m_ser_directory = settings.value("ser_directory").toString();
    }

    if (settings.value("new_version") != QVariant::Invalid) {
        m_new_version = settings.value("new_version").toString();
    }

    if (settings.value("check_for_updates") != QVariant::Invalid) {
        m_check_for_updates = settings.value("check_for_updates").toBool();
    }

    if (settings.value("enable_debayering") != QVariant::Invalid) {
        m_enable_debayering = settings.value("enable_debayering").toBool();
    }

    if (settings.value("hide_markers") != QVariant::Invalid) {
        m_hide_markers = settings.value("hide_markers").toBool();
    }

    if (settings.value("playback_repeat") != QVariant::Invalid) {
        m_repeat = settings.value("playback_repeat").toBool();
    }
}
	
	
//
// Save persistent data
//
void c_persistent_data::save()
{
    QSettings settings;
    settings.setValue("last_ver_check_time", m_last_ver_check_time);
    settings.setValue("selected_language", m_selected_language);
    settings.setValue("ser_directory", m_ser_directory);
    settings.setValue("new_version", m_new_version);
    settings.setValue("check_for_updates", m_check_for_updates);
    settings.setValue("enable_debayering", m_enable_debayering);
    settings.setValue("hide_markers", m_hide_markers);
    settings.setValue("playback_repeat", m_repeat);
}
