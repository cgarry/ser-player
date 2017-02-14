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
QStringList c_persistent_data::m_recent_ser_files = QStringList();
QStringList c_persistent_data::m_recent_save_folders = QStringList();

bool c_persistent_data::m_check_for_updates = true;
bool c_persistent_data::m_disconnect_playback_controls = false;
bool c_persistent_data::m_repeat = false;
int c_persistent_data::m_play_direction = 0;
bool c_persistent_data::m_histogram_enabled = false;
bool c_persistent_data::m_markers_enabled = false;
int c_persistent_data::m_selection_box_colour = 0;


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

    if (settings.value("recent_ser_files") != QVariant::Invalid) {
        m_recent_ser_files = settings.value("recent_ser_files").toStringList();
    }

    if (settings.value("recent_save_folders") != QVariant::Invalid) {
        m_recent_save_folders = settings.value("recent_save_folders").toStringList();
    }

    if (settings.value("check_for_updates") != QVariant::Invalid) {
        m_check_for_updates = settings.value("check_for_updates").toBool();
    }


    if (settings.value("disconnect_playback_controls") != QVariant::Invalid) {
        m_disconnect_playback_controls = settings.value("disconnect_playback_controls").toBool();
    }

    if (settings.value("playback_repeat") != QVariant::Invalid) {
        m_repeat = settings.value("playback_repeat").toBool();
    }

    if (settings.value("play_direction") != QVariant::Invalid) {
        m_play_direction = settings.value("play_direction").toInt();
    }

    if (settings.value("histogram_enabled") != QVariant::Invalid) {
        m_histogram_enabled = settings.value("histogram_enabled").toBool();
    }

    if (settings.value("markers_enabled") != QVariant::Invalid) {
        m_markers_enabled = settings.value("markers_enabled").toBool();
    }

    if (settings.value("selection_box_colour") != QVariant::Invalid) {
        m_selection_box_colour = settings.value("selection_box_colour").toInt();
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
    settings.setValue("recent_ser_files", m_recent_ser_files);
    settings.setValue("recent_save_folders", m_recent_save_folders);
    settings.setValue("check_for_updates", m_check_for_updates);
    settings.setValue("disconnect_playback_controls", m_disconnect_playback_controls);
    settings.setValue("playback_repeat", m_repeat);
    settings.setValue("play_direction", m_play_direction);   
    settings.setValue("histogram_enabled", m_histogram_enabled);
    settings.setValue("markers_enabled", m_markers_enabled);
    settings.setValue("selection_box_colour", m_selection_box_colour);
}
