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
class QLabel;
class c_pipp_ser;
class QSlider;
class QPushButton;
class QMutex;
class QImage;
class c_image_Widget;


class c_ser_player : public QMainWindow
{
    Q_OBJECT

private:
    static const QString C_WINDOW_TITLE_QSTRING;

    QMenu *mp_framerate_Menu;
    QAction *m_debayer_Act;
    QMenu *mp_colour_saturation_Menu;

    QPixmap m_no_file_open_Pixmap;
    QPixmap m_play_Pixmap;
    QPixmap m_pause_Pixmap;
    QPushButton *mp_play_PushButton;
    QPushButton *mp_stop_PushButton;
    QPushButton *mp_repeat_PushButton;
    QPushButton *mp_forward_PushButton;
    QPushButton *mp_back_PushButton;
    c_image_Widget *mp_frame_image_Widget;
    QImage *mp_frame_Image;
    QTimer *mp_frame_Timer;
    QSlider *mp_count_Slider;
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
    QMutex *mp_ser_file_Mutex;
    QString m_timestamp_label_String;
    QLabel *mp_timestamp_Label;
    QTimer *mp_resize_Timer;

    c_pipp_ser *mp_ser_file;
    QString m_ser_directory;
    enum e_state {STATE_NO_FILE, STATE_STOPPED, STATE_PLAYING, STATE_PAUSED, STATE_FINISHED};
    enum e_state m_current_state;
    int m_framecount;
    int m_total_frames;
    int m_display_framerate;
    double m_colour_saturation;
    int m_display_frame_time;
    uint8_t *mp_frame_buffer;
    int32_t m_frame_width;
    int32_t m_frame_height;
    int32_t m_bytes_per_sample;
    int32_t m_colour_id;

public:
    c_ser_player(QWidget *parent = 0);
    ~c_ser_player();
    void open_ser_file(const QString &filename);

public slots:
    void fps_changed_slot(QAction *);
    void colour_saturation_changed(double);
    void zoom_changed_slot(QAction *);
    void language_changed_slot(QAction *);
    void open_ser_file_slot();
    void save_frame_slot();
    void frame_timer_timeout_slot();
    void resize_timer_timeout_slot();
    void frame_slider_changed_slot();
    void forward_button_pressed_slot();
    void back_button_pressed_slot();
    void play_button_pressed_slot();
    void stop_button_pressed_slot();
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


protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    void create_no_file_open_image();
    void conv_data_ready_for_qimage(bool image_debayered);
    bool debayer_image_bilinear();
    void change_colour_saturation(double change);
    void calculate_display_framerate();
    void resize_window_with_zoom(int zoom);

    template <typename T>
    void debayer_pixel_bilinear(uint32_t bayer, int32_t x, int32_t y, T *raw_data, T *rgb_data);

};

#endif // SER_PLAYER_H
