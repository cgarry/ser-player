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
class QLabel;
class QGroupBox;
class QCheckBox;


class c_save_frames_dialog : public QDialog
{
    Q_OBJECT

public:
    c_save_frames_dialog(QWidget *parent,
                         int total_frames,
                         int marker_start_frame,
                         int marker_end_frame,
                         bool markers_enabled,
                         bool ser_has_timestamps);


    int get_start_frame()
    {
        return m_start_frame;
    }

    int get_end_frame()
    {
        return m_end_frame;
    }

    int get_frame_decimation();
    int get_sequence_direction();
    bool get_append_timestamp_to_filename();
    int get_frames_to_be_saved();
    int get_required_digits_for_number();
    bool get_use_framenumber_in_name();


signals:


private slots:
    void spinbox_changed_slot();
    void update_num_frames_slot();
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
    QLabel *mp_selected_frames_Label;

    QGroupBox *mp_frame_decimation_GBox;
    QSpinBox *mp_frame_decimation_SpinBox;

    QGroupBox *mp_sequence_direction_GBox;
    QRadioButton *mp_forwards_sequence_RButton;
    QRadioButton *mp_reverse_sequence_RButton;
    QRadioButton *mp_forwards_then_reverse_sequence_RButton;

    QCheckBox *mp_use_framenumber_in_filename;
    QCheckBox *mp_append_timestamp_CBox;

    QLabel *mp_total_frames_to_save_Label;

    int m_total_frames;
    int m_marker_start_frame;
    int m_marker_end_frame;
    int m_start_frame;
    int m_end_frame;
    int m_total_selected_frames;
    bool m_spin_boxes_valid;
};

#endif // SAVE_FRAMES_DIALOG_H
