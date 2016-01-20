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


#ifndef SELECTION_BOX_DIALOG_H
#define SELECTION_BOX_DIALOG_H

#include <QPoint>
#include <QDialog>

class QGroupBox;
class QSpinBox;
class QComboBox;


class c_selection_box_dialog : public QDialog
{
    Q_OBJECT

public:
    c_selection_box_dialog(QWidget *parent = 0);
    QColor get_selection_colour();


signals:
    void selection_box_changed(QRect selection_rect);
    void selection_box_complete(bool accepted, QRect selection_rect);
    void update_request_signal();


public slots:
    void start_get_selection_box_slot(int width, int height);
    void set_selection_box_slot(const QRect &selection_rect);
    void cancel_get_selection_box_slot();

    void reset_selection_box_slot();
    void update_selection_box(const QPoint &top_left_corner, const QPoint &bottom_right_corner);


private slots:
    void spinbox_changed_slot();
    void selection_box_colour_changed();

protected:
    void reject();
    void accept();
    
private:
    // Private methods
//    void helper_method();
    
    // Widgets
    QGroupBox *mp_area_select_GroupBox;
    QSpinBox *mp_x_pos_SpinBox;
    QSpinBox *mp_y_pos_SpinBox;
    QSpinBox *mp_width_SpinBox;
    QSpinBox *mp_height_SpinBox;
    QComboBox *mp_selection_colour_CBox;

    // Variables
    int m_width;
    int m_height;
    bool m_spinbox_values_valid;

};

#endif // SELECTION_BOX_DIALOG_H
