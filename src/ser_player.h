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


#ifndef SER_PLAYER_H
#define SER_PLAYER_H

#include <QMainWindow>
#include <QFile>
#include <cstdint>

class QAction;
class QActionGroup;
class QLabel;
class c_pipp_ser;
class c_frame_slider;
class QPushButton;
class QImage;
class QTimer;

class c_header_details_dialog;
class c_histogram_dialog;
class c_processing_options_dialog;
class c_colour_dialog;
class c_image_Widget;
class c_image;
class c_histogram_thread;


class c_ser_player : public QMainWindow
{
    Q_OBJECT

private:
    static const QString C_WINDOW_TITLE_QSTRING;

    // Menus
    QAction *mp_save_frames_as_images_Act;
    QAction *mp_save_frames_as_ser_Act;
    QMenu *mp_recent_ser_files_Menu;
    QActionGroup *mp_recent_ser_files_ActGroup;
    QMenu *mp_recent_save_folders_Menu;
    QActionGroup *mp_recent_save_folders_ActGroup;
    QMenu *mp_framerate_Menu;
    QAction *mp_header_details_Act;
    QAction *mp_histogram_viewer_Act;
    QAction *mp_gain_gamma_settings_Act;
    QAction *mp_colour_settings_Act;
    QAction *mp_markers_dialog_Act;

    // Dialogs
    c_header_details_dialog *mp_header_details_dialog;
    c_processing_options_dialog *mp_gain_and_gamma_Dialog;
    c_colour_dialog *mp_colour_settings_Dialog;
    c_histogram_dialog *mp_histogram_dialog;

    // Threads
    c_histogram_thread *mp_histogram_thread;

    // Widgets
    QPixmap m_no_file_open_Pixmap;
    QPixmap m_play_Pixmap;
    QPixmap m_pause_Pixmap;
    QPixmap m_forward_play_Pixmap;
    QPixmap m_reverse_play_Pixmap;
    QPixmap m_forward_and_reverse_play_Pixmap;
    QPushButton *mp_forward_PushButton;
    QPushButton *mp_back_PushButton;
    QPushButton *mp_play_PushButton;
    QPushButton *mp_stop_PushButton;
    QPushButton *mp_repeat_PushButton;
    QPushButton *mp_play_direction_PushButton;
    c_image_Widget *mp_frame_image_Widget;
    QTimer *mp_frame_Timer;
    c_frame_slider *mp_frame_Slider;
    QString m_zoom_label_String;
    QLabel *mp_zoom_Label;
    QString m_frame_size_label_String;
    QLabel *mp_frame_size_Label;
    QString m_pixel_depth_label_String;
    QLabel *mp_pixel_depth_Label;
    QLabel *mp_colour_id_Label;
    QString m_fps_label_String;
    QLabel *mp_fps_Label;
    QString m_framecount_label_String;
    QLabel *mp_framecount_Label;
    QString m_timestamp_label_String;
    QLabel *mp_timestamp_Label;
    QTimer *mp_resize_Timer;

    // Other
    bool m_forward_button_held;
    bool m_back_button_held;

    c_pipp_ser *mp_ser_file;
    c_image *mp_frame_image;
    QString m_ser_directory;
    enum e_state {STATE_NO_FILE, STATE_STOPPED, STATE_PLAYING, STATE_PAUSED, STATE_FINISHED};
    enum e_state m_current_state;
    int m_total_frames;
    int m_display_framerate;
    int m_display_frame_time;
    bool m_is_colour;
    bool m_has_bayer_pattern;
    int m_play_direction;

    bool m_monochrome_conversion_enable;
    int m_monochrome_conversion_type;
    double m_colour_saturation;

public:
    c_ser_player(QWidget *parent = 0);
    ~c_ser_player();
    void open_ser_file(const QString &filename);

public slots:
    void fps_changed_slot(QAction *);
    void header_details_dialog_closed_slot();
    void header_details_dialog_slot(bool checked);
    void histogram_viewer_closed_slot();
    void histogram_viewer_slot(bool checked);
    void gain_and_gamma_settings_closed_slot();
    void gain_and_gamma_settings_slot(bool checked);
    void invert_changed_slot(bool invert);
    void gain_changed_slot(double gain);
    void gamma_changed_slot(double gamma);
    void monochrome_conversion_changed_slot(bool enabled, int selection);
    void colour_settings_closed_slot();
    void colour_settings_slot(bool checked);
    void colour_saturation_changed_slot(double saturation);
    void colour_balance_changed_slot(double red, double green, double blue);
    void estimate_colour_balance();
    void zoom_changed_slot(QAction *);
    void language_changed_slot(QAction *);
    void open_ser_file_slot();
    void open_ser_file_slot(QAction *action);
    void save_frames_as_ser_slot();
    void save_frames_slot_as_images_slot();
    void open_save_folder_slot(QAction *);
    void frame_timer_timeout_slot();
    void resize_timer_timeout_slot();
    void frame_slider_changed_slot();
    void markers_dialog_closed_slot();
    void forward_button_pressed_slot();
    void forward_button_released_slot();
    void back_button_pressed_slot();
    void back_button_released_slot();
    void back_button_held_slot();
    void play_button_pressed_slot();
    void stop_button_pressed_slot();
    void play_direction_button_pressed_slot();
    void repeat_button_toggled_slot(bool checked);
    void resize_window_100_percent_slot();
    void check_for_updates_slot(bool enabled);
    void debayer_enable_slot(bool enabled);
    void new_version_available_slot(QString version);
    void about_qt();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void handle_arguments();
    void about_ser_player();
    void forward_button_held_slot();
    void histogram_done_slot();


protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    void add_string_to_stringlist(QStringList &string_list, QString string);
    void update_recent_ser_files_menu();
    void populate_recent_ser_files_menu();
    void update_recent_save_folders_menu();
    void populate_recent_save_folders_menu();
    void create_no_file_open_image();
    bool get_and_process_frame(int frame_number, bool conv_to_8_bit, bool do_processing);
    void calculate_display_framerate();
    void resize_window_with_zoom(int zoom);
};

#endif // SER_PLAYER_H
