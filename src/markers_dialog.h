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


#ifndef MARKERS_DIALOG_H
#define MARKERS_DIALOG_H

#include <QDialog>


class QGroupBox;
class QSpinBox;
class QLabel;


class c_markers_dialog : public QDialog
{
    Q_OBJECT

public:
    c_markers_dialog(QWidget *parent = 0);
    void set_maximum_frame(int value);
    bool get_markers_enabled();
//    ~c_markers_dialog();


signals:
    void start_marker_changed(int value);
    void end_marker_changed(int value);
    void markers_enabled_changed(bool enabled);

public slots:
    void reset_markers_slot();
    void set_start_marker_slot(int value);
    void set_end_marker_slot(int value);


private slots:
    void marker_changed_slot();

    
private:
    // Private methods
    void helper_method();
    
    // Widgets
    QGroupBox *mp_markers_GroupBox;
    QLabel *mp_start_market_Label;
    QSpinBox *mp_start_marker_SpinBox;
    QLabel *mp_end_market_Label;
    QSpinBox *mp_end_marker_SpinBox;
    QLabel *mp_selected_count_Label;
};

#endif // MARKERS_DIALOG_H
