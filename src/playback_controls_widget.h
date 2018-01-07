// ---------------------------------------------------------------------
// Copyright (C) 2017 Chris Garry
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


#ifndef PLAYBACK_CONTROLS_WIDGET_H
#define PLAYBACK_CONTROLS_WIDGET_H

#include <QWidget>
#include <cstdint>

class QLabel;
class QPushButton;
class c_frame_slider;


class c_playback_controls_widget : public QWidget
{
    Q_OBJECT

public:
    c_playback_controls_widget(QWidget *parent = 0);
    void start_playback();
    void stop_playback();
    void finish_playback();
    void pause_payback();
    int slider_value();
    bool get_markers_enable();
    void set_repeat(bool repeat);
    void goto_first_frame();
    bool goto_next_frame();
    int get_start_frame();
    int get_end_frame();
    bool is_playing();
    void reset_labels();
    void update_framecount_label(int count, int maxcount);
    void update_colour_id_label(QString colour_id);
    void update_pixel_depth_label(int depth);
    void update_frame_size_label(int width, int height);
    void update_fps_label(int fps);
    void update_timestamp_label(uint64_t timestamp);

signals:
    void start_marker_changed(int frame);
    void end_marker_changed(int frame);
    void markers_dialog_closed();
    void slider_value_changed(int);
    void start_playing_signal();
    void stop_playing_signal();
    void open_ser_file_signal();
    void double_clicked_signal();

public slots:
    void show_markers_dialog(bool show);
    void set_markers_show(bool show);
    void set_markers_enable(bool active);
    void set_maximum_frame(int max_frame);
    void set_start_marker_to_current();
    void set_start_marker_slot(int frame);
    void set_end_marker_to_current();
    void set_end_marker_slot(int frame);
    void reset_all_markers_slot();
    void update_zoom_label_slot(int zoom);
    
private slots:
    void play_direction_button_pressed_slot();
    void repeat_button_toggled_slot(bool checked);
    void forward_button_pressed_slot();
    void forward_button_released_slot();
    void forward_button_held_slot();
    void back_button_pressed_slot();
    void back_button_released_slot();
    void back_button_held_slot();
    void play_button_pressed_slot();
    void stop_button_pressed_slot();
    void slider_value_changed_slot(int value);

protected:
    void mouseDoubleClickEvent(QMouseEvent *p_event);

private:
    //
    // Widgets
    //
    c_frame_slider *mp_frame_Slider;
    
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
    int m_framecount_label_min_width;
    QString m_datestamp_label_String;
    QLabel *mp_datestamp_Label;
    int m_datestamp_label_min_width;
    QString m_timestamp_label_String;
    QString m_no_timestamp_label_String;
    QLabel *mp_timestamp_Label;
    int m_timestamp_label_min_width;
    QTimer *mp_resize_Timer;

    int m_play_direction;
    bool m_forward_button_held;
    bool m_back_button_held;
    enum e_state {STATE_NO_FILE, STATE_STOPPED, STATE_PLAYING, STATE_PAUSED, STATE_FINISHED};
    enum e_state m_current_state;
    

};

#endif // PLAYBACK_CONTROLS_WIDGET_H
