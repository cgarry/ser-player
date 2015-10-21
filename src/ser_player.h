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
#include "image_functions.h"

class QAction;
class QLabel;
class c_pipp_ser;
class c_frame_slider;
class QPushButton;
class QMutex;
class QImage;
class QTimer;

class c_colour_dialog;
class c_image_Widget;


class c_ser_player : public QMainWindow
{
    Q_OBJECT

private:
    static const QString C_WINDOW_TITLE_QSTRING;

    // Menus
    QAction *mp_save_frames_Act;
    QAction *mp_open_save_folder_Act;
    QMenu *mp_framerate_Menu;
    QAction *m_debayer_Act;
    QAction *mp_colour_settings_action;
    QAction *mp_markers_dialog_action;

    // Dialogs
    c_colour_dialog *mp_colour_settings_Dialog;

    // Other
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
//    QPushButton *mp_start_marker_PushButton;
//    QPushButton *mp_end_marker_PushButton;
    c_image_Widget *mp_frame_image_Widget;
    QImage *mp_frame_Image;
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

    QMutex *mp_ser_file_Mutex;
    QMutex *mp_frame_slider_changed_Mutex;
    bool m_forward_button_held;
    bool m_back_button_held;

    c_pipp_ser *mp_ser_file;
    QString m_ser_directory;
    enum e_state {STATE_NO_FILE, STATE_STOPPED, STATE_PLAYING, STATE_PAUSED, STATE_FINISHED};
    enum e_state m_current_state;
    int m_total_frames;
    int m_display_framerate;
    int m_display_frame_time;
    struct image_functions::s_image_details m_frame_details;
    bool m_is_colour;
    bool m_has_bayer_pattern;
    int m_play_direction;

    double m_colour_saturation;

public:
    c_ser_player(QWidget *parent = 0);
    ~c_ser_player();
    void open_ser_file(const QString &filename);

public slots:
    void fps_changed_slot(QAction *);
    void colour_settings_slot();
    void colour_saturation_changed_slot(double saturation);
    void colour_balance_changed_slot(double red, double green, double blue);
    void estimate_colour_balance();
    void zoom_changed_slot(QAction *);
    void language_changed_slot(QAction *);
    void open_ser_file_slot();
    void save_frames_slot();
    void open_save_folder_slot();
    void frame_timer_timeout_slot();
    void resize_timer_timeout_slot();
    void frame_slider_changed_slot();
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


protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    void create_no_file_open_image();
    QImage *get_frame_as_qimage(int frame_number);
    void calculate_display_framerate();
    void resize_window_with_zoom(int zoom);
};

#endif // SER_PLAYER_H
