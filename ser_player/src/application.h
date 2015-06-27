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


#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QTranslator>

class c_ser_player;

class c_application : public QApplication
{
    Q_OBJECT
public:
    c_application(int & argc, char ** argv);
    virtual ~c_application();

protected:
    bool event(QEvent *e);

private:
    c_ser_player *mp_win;
    QTranslator m_qt_translator;
    QTranslator m_ser_player_translator;
};

#endif // APPLICATION_H
