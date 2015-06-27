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


#include <QFileOpenEvent>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "application.h"
#include "persistent_data.h"
#include "ser_player.h"

#include <QDebug>


c_application::c_application(int &argc, char **argv)
    : QApplication(argc, argv),
      mp_win(NULL)
{
    setOrganizationName("PIPP");
    setApplicationName(tr("SER Player"));
    c_persistent_data::load();  // Load persistent data

    // Load translation files stored in the QT Resource system
    QString locale = c_persistent_data::m_selected_language;

    if (locale == QString("auto")) {
        locale = QLocale::system().name();
    }

//    bool ret = m_qt_translator.load("qt_"+locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    bool ret = m_qt_translator.load("qt_"+locale, ":/res/translations/");
    ret = installTranslator(&m_qt_translator);

    // Try to load translations from same directory as executable initially
    ret = m_ser_player_translator.load("ser_player_" + locale);

    if (!ret) {
        // Else load from Qt resource system
        ret = m_ser_player_translator.load("ser_player_" + locale, ":/res/translations/");
    }

    ret = installTranslator(&m_ser_player_translator);

    mp_win = new c_ser_player;
    mp_win->show();
}


c_application::~c_application()
{
    delete mp_win;
    c_persistent_data::save();  // Save persistent data before exiting
}


bool c_application::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FileOpen:
        mp_win->open_ser_file(static_cast<QFileOpenEvent *>(event)->file());
        return true;
    default:
        return QApplication::event(event);
    }
}
