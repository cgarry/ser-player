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


#ifndef SAVE_FRAMES_DIALOG_H
#define SAVE_FRAMES_DIALOG_H

#include <QDialog>


class QRadioButton;
class QSpinBox;


class c_save_frames_dialog : public QDialog
{
    Q_OBJECT

public:
    c_save_frames_dialog(QWidget *parent,
                         int total_frames,
                         int marker_start_frame,
                         int marker_end_frame);


    int get_start_frame()
    {
        return m_start_frame;
    }

    int get_end_frame()
    {
        return m_end_frame;
    }


signals:


    
private slots:
    void next_button_clicked_slot();

    
private:
    // Private methods
    void helper_method();
    
    // Widgets
    QRadioButton *mp_save_current_frame_RButton;
    QRadioButton *mp_save_all_frames_RButton;
    QRadioButton *mp_save_marked_frames_RButton;
    QRadioButton *mp_save_frame_range_RButton;
    QSpinBox *mp_start_Spinbox;
    QSpinBox *mp_end_Spinbox;

    int m_total_frames;
    int m_marker_start_frame;
    int m_marker_end_frame;
    int m_start_frame;
    int m_end_frame;
};

#endif // SAVE_FRAMES_DIALOG_H
