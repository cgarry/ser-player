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


#ifndef PLAYBACK_CONTROLS_DIALOG_H
#define PLAYBACK_CONTROLS_DIALOG_H

#include <QDialog>

class QLabel;
class QVBoxLayout;


class c_playback_controls_dialog : public QDialog
{
    Q_OBJECT

public:
    c_playback_controls_dialog(QWidget *parent = 0);
    void add_controls_widget(QWidget *p_widget);
    void remove_controls_widget(QWidget  *p_widget);

    
signals:


public slots:

    
private slots:

    
private:    
    QLabel *mp_dummy_label;
    QVBoxLayout *mp_dialog_vlayout;

};

#endif // PLAYBACK_CONTROLS_DIALOG_H
