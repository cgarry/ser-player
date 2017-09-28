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
class QDoubleSpinBox;
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
    enum e_save_type {SAVE_IMAGES, SAVE_SER, SAVE_AVI, SAVE_GIF};

    c_save_frames_dialog(QWidget *parent,
                         e_save_type save_type,
                         int frame_width,
                         int frame_height,
                         int total_frames,
                         bool ser_has_timestamps,
                         double ser_framerate = 0,
                         QString observer_string = "",
                         QString instrument_string = "",
                         QString telescope_string = "");

    ~c_save_frames_dialog();

    void set_markers(int marker_start_frame,
                     int marker_end_frame,
                     bool markers_enabled);

    void set_processed_frame_size(int frame_width,
                                  int frame_height);

    void set_gif_frametime(double frametime);

    void set_colour_details(bool is_colour_raw,
                            bool is_colour_processed);

    int get_active_width();
    int get_active_height();
    int get_total_width();
    int get_total_height();
    double get_gif_frametime();
    double get_gif_final_frametime();
    int get_gif_unchanged_border_tolerance();
    bool get_gif_transparent_pixel_enable();
    int get_gif_transparent_pixel_tolerance();
    int get_gif_colour_quantisation_type();
    QString get_gif_colour_quantisation_name();
    int get_gif_pixel_bit_depth();
    int get_gif_lossy_compression_level();
    bool get_gif_test_run()
    {
        return m_test_run;
    }

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

    // AVI file options
    double get_avi_framerate();
    bool get_avi_old_format();
    int get_avi_max_size();

    // Last save directory
    void set_last_save_directory(QString dir)
    {
        m_last_save_dir = dir;
    }

    QString get_last_save_directory()
    {
        return m_last_save_dir;
    }


signals:


private slots:
    void spinbox_changed_slot();
    void update_num_frames_slot();
    void next_button_clicked_slot();
    void resize_control_handler();
    void gif_apply_preset_options();
    void gif_unchanged_border_tolerance_changed_slot();
    void gif_test_options_button_pressed_slot();
//    void multiple_files_frames_changed_slot();
//    void multiple_files_files_changed_slot();
//    void multiple_files_overlap_frames_changed_slot();

    
private:
    // Private methods
    void helper_method();
    void colour_updated();
    bool is_select_radio_button_checked();
    
    // Widgets
    QRadioButton *mp_save_current_frame_RButton;
    QRadioButton *mp_save_all_frames_RButton;
    QRadioButton *mp_save_marked_frames_RButton;
    QRadioButton *mp_save_frame_range_RButton;
    QSpinBox *mp_start_Spinbox;
    QSpinBox *mp_end_Spinbox;
    QLabel *mp_selected_frames_Label;
/*
    QSpinBox *mp_multiple_files_frames_Spinbox;
    QSpinBox *mp_multiple_files_files_Spinbox;
    QGroupBox *mp_save_multiple_files_GBox;
    QLabel *mp_save_multiple_files_Label;
    QSpinBox *mp_multiple_files_overlap_Spinbox;
*/
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

    // AVI Options
    QCheckBox *mp_avi_old_format_CBox;
    QComboBox *mp_avi_max_size_Combox;
    QDoubleSpinBox *mp_avi_framerate_DSpinbox;

    // Animated GIF options
    QDoubleSpinBox *mp_gif_frame_delay_DSpinBox;
    QDoubleSpinBox *mp_gif_final_frame_delay_DSpinBox;
    QLabel *mp_gif_colour_quantisation_type_Label;
    QComboBox *mp_gif_colour_quantisation_type_ComboBox;
    QComboBox *mp_gif_preset_options_ComboBox;
    QCheckBox *mp_gif_unchanged_border_tolerance_CBox;
    QSpinBox *mp_gif_unchanged_border_tolerance_SpinBox;
    QCheckBox *mp_gif_transparent_tolerance_CBox;
    QSpinBox *mp_gif_transparent_tolerance_SpinBox;
    QCheckBox *mp_gif_lossy_compression_level_CBox;
    QSpinBox *mp_gif_lossy_compression_level_SpinBox;
    QCheckBox *mp_gif_reduce_pixel_depth_CBox;
    QSpinBox *mp_gif_reduce_pixel_depth_SpinBox;


    QLabel *mp_total_frames_to_save_Label;

    c_utf8_validator *mp_utf8_validator;

    e_save_type m_save_type;
    bool m_is_colour_raw;
    bool m_is_colour_processed;
    int m_raw_frame_width;
    int m_raw_frame_height;
    int m_processed_frame_width;
    int m_processed_frame_height;
    int m_frame_width;
    int m_frame_height;
    int m_total_frames;
    int m_marker_start_frame;
    int m_marker_end_frame;
    bool m_ser_has_timestamps;
    double m_ser_framerate;
    int m_start_frame;
    int m_end_frame;
    int m_total_selected_frames;
    bool m_frame_start_end_spin_boxes_valid;
//    bool m_multiple_files_spin_boxes_valid;
    bool m_test_run;
    QString m_last_save_dir;
//    bool m_multiple_files_by_frames;
};

#endif // SAVE_FRAMES_DIALOG_H
