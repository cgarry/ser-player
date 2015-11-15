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
#include <QString>


class QRadioButton;
class QSpinBox;
class QLabel;
class QLineEdit;
class QGroupBox;
class QCheckBox;
class QComboBox;
class c_utf8_validator;


class c_save_frames_dialog : public QDialog
{
    Q_OBJECT

public:
    enum e_save_type {SAVE_IMAGES, SAVE_SER};

    c_save_frames_dialog(QWidget *parent,
                         e_save_type save_type,
                         int frame_width,
                         int frame_height,
                         int total_frames,
                         bool ser_has_timestamps,
                         QString observer_string = "",
                         QString instrument_string = "",
                         QString telescope_string = "");

    ~c_save_frames_dialog();

    void set_markers(int marker_start_frame,
                     int marker_end_frame,
                     bool markers_enabled);

    int get_active_width();
    int get_active_height();
    int get_total_width();
    int get_total_height();

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
    bool get_processing_enable();
    bool get_append_timestamp_to_filename();
    int get_frames_to_be_saved();
    int get_required_digits_for_number();
    bool get_use_framenumber_in_name();
    bool get_include_timestamps_in_ser_file();
    QString get_observer_string();
    QString get_instrument_string();
    QString get_telescope_string();


signals:


private slots:
    void spinbox_changed_slot();
    void update_num_frames_slot();
    void next_button_clicked_slot();
    void resize_control_handler();

    
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

    QCheckBox *mp_processing_enable_CBox;
    QGroupBox *mp_processing_GBox;

    QSpinBox *mp_resize_width_Spinbox;
    QSpinBox *mp_resize_height_Spinbox;
    QComboBox *mp_resize_units_ComboBox;
    QCheckBox *mp_resize_constrain_propotions_CBox;
    QCheckBox *mp_resize_add_black_bars_CBox;
    QGroupBox *mp_resize_GBox;

    QCheckBox *mp_use_framenumber_in_filename;
    QCheckBox *mp_append_timestamp_CBox;

    QCheckBox *mp_include_timestamps_CBox;
    QLineEdit *mp_observer_LEdit;
    QLineEdit *mp_instrument_LEdit;
    QLineEdit *mp_telescope_LEdit;

    QLabel *mp_total_frames_to_save_Label;

    c_utf8_validator *mp_utf8_validator;

    e_save_type m_save_type;
    int m_frame_width;
    int m_frame_height;
    int m_total_frames;
    int m_marker_start_frame;
    int m_marker_end_frame;
    bool m_ser_has_timestamps;
    int m_start_frame;
    int m_end_frame;
    int m_total_selected_frames;
    bool m_spin_boxes_valid;
};

#endif // SAVE_FRAMES_DIALOG_H
