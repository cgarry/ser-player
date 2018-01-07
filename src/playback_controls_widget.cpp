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

#include <QDebug>

#include <Qt>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>

#include "playback_controls_widget.h"
#include "pipp_timestamp.h"
#include "persistent_data.h"
#include "frame_slider.h"


c_playback_controls_widget::c_playback_controls_widget(QWidget *parent)
    : QWidget(parent),
      m_framecount_label_min_width(0),
      m_timestamp_label_min_width(0)
{
    //setWindowTitle(tr("Playback"));
    //QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_play_direction = c_persistent_data::m_play_direction;
    m_back_button_held = false;
    m_forward_button_held = false;
    m_current_state = STATE_NO_FILE;
    
    mp_frame_Slider = new c_frame_slider(this);
    mp_frame_Slider->set_maximum_frame(99);
    mp_frame_Slider->set_direction(m_play_direction);
    mp_frame_Slider->set_repeat(c_persistent_data::m_repeat);
    //connect(mp_markers_dialog_Act, SIGNAL(triggered(bool)), mp_frame_Slider, SLOT(show_markers_dialog(bool)));
    //connect(mp_frame_Slider, SIGNAL(markers_dialog_closed()), this, SLOT(markers_dialog_closed_slot()));
    
    m_play_Pixmap = QPixmap(":/res/resources/play_button.png");
    m_pause_Pixmap = QPixmap(":/res/resources/pause_button.png");

    mp_forward_PushButton = new QPushButton;
    QPixmap forward_Pixmap = QPixmap(":/res/resources/forward_button.png");
    mp_forward_PushButton->setIcon(forward_Pixmap);
    mp_forward_PushButton->setIconSize(forward_Pixmap.size());
    mp_forward_PushButton->setFixedSize(forward_Pixmap.size() + QSize(10, 10));  // Nice and small
    mp_forward_PushButton->setToolTip(tr("Click to advance 1 frame\nShift-Click to advance multiple frames", "Button Tool tip"));  // Nice and small

    mp_back_PushButton = new QPushButton;
    QPixmap back_Pixmap = QPixmap(":/res/resources/back_button.png");
    mp_back_PushButton->setIcon(back_Pixmap);
    mp_back_PushButton->setIconSize(back_Pixmap.size());
    mp_back_PushButton->setFixedSize(back_Pixmap.size() + QSize(10, 10));
    mp_back_PushButton->setToolTip(tr("Click to go back 1 frame\nShift-Click to go back multiple frames", "Button Tool tip"));

    mp_play_PushButton = new QPushButton;
    mp_play_PushButton->setIcon(m_play_Pixmap);
    mp_play_PushButton->setIconSize(m_play_Pixmap.size());
    mp_play_PushButton->setFixedSize(m_play_Pixmap.size() + QSize(10, 10));
    mp_play_PushButton->setToolTip(tr("Play/Pause", "Button Tool tip"));

    mp_stop_PushButton = new QPushButton;
    QPixmap stop_Pixmap = QPixmap(":/res/resources/stop_button.png");
    mp_stop_PushButton->setIcon(stop_Pixmap);
    mp_stop_PushButton->setIconSize(stop_Pixmap.size());
    mp_stop_PushButton->setFixedSize(stop_Pixmap.size() + QSize(10, 10));
    mp_stop_PushButton->setToolTip(tr("Stop", "Button Tool tip"));

    mp_repeat_PushButton = new QPushButton;
    QPixmap repeat_Pixmap = QPixmap(":/res/resources/repeat_button.png");
    mp_repeat_PushButton->setIcon(repeat_Pixmap);
    mp_repeat_PushButton->setIconSize(repeat_Pixmap.size());
    mp_repeat_PushButton->setFixedSize(repeat_Pixmap.size() + QSize(10, 10));
    mp_repeat_PushButton->setCheckable(true);
    mp_repeat_PushButton->setChecked(c_persistent_data::m_repeat);
    mp_repeat_PushButton->setToolTip(tr("Repeat On/Off", "Button Tool tip"));

    mp_play_direction_PushButton = new QPushButton;
    m_forward_play_Pixmap = QPixmap(":/res/resources/play_forward.png");
    m_reverse_play_Pixmap = QPixmap(":/res/resources/play_reverse.png");
    m_forward_and_reverse_play_Pixmap = QPixmap(":/res/resources/play_forward_and_reverse.png");
    switch (m_play_direction) {
    case 0:
        mp_play_direction_PushButton->setIcon(m_forward_play_Pixmap);
        break;
    case 1:
        mp_play_direction_PushButton->setIcon(m_reverse_play_Pixmap);
        break;
    default:
        mp_play_direction_PushButton->setIcon(m_forward_and_reverse_play_Pixmap);
    }

    mp_play_direction_PushButton->setIconSize(m_forward_play_Pixmap.size());
    mp_play_direction_PushButton->setFixedSize(m_forward_play_Pixmap.size() + QSize(10, 10));
    mp_play_direction_PushButton->setToolTip(tr("Play Direction", "Button Tool tip"));
    connect(mp_play_direction_PushButton, SIGNAL(clicked()), this, SLOT(play_direction_button_pressed_slot()));

    m_framecount_label_String = tr("%1/%2", "Frame number/Frame count label");
    mp_framecount_Label = new QLabel;
    mp_framecount_Label->setText(m_framecount_label_String.arg("-").arg("----"));
    mp_framecount_Label->setToolTip(tr("Frame number/Total Frames", "Tool tip"));
    mp_framecount_Label->setAlignment(Qt::AlignRight);

    mp_fps_Label = new QLabel;
    m_fps_label_String = tr("%1 FPS", "Framerate label");
    mp_fps_Label->setText(m_fps_label_String.arg("--"));
    mp_fps_Label->setToolTip(tr("Display Frame rate", "Tool tip"));

    mp_colour_id_Label = new QLabel;
    mp_colour_id_Label->setText("----");
    mp_colour_id_Label->setToolTip(tr("Colour ID", "Tool tip"));

    m_zoom_label_String = tr("%1%", "Zoom level label");
    mp_zoom_Label =  new QLabel;
    mp_zoom_Label->setText(m_zoom_label_String.arg(100));
    mp_zoom_Label->setToolTip(tr("Display Zoom Level", "Tool tip"));

    m_frame_size_label_String = tr("%1x%2", "Frame size label");
    mp_frame_size_Label = new QLabel;
    mp_frame_size_Label->setText(m_frame_size_label_String.arg("---").arg("---"));
    mp_frame_size_Label->setToolTip(tr("Frame size", "Tool tip"));

    m_pixel_depth_label_String = tr("%1-Bit", "Pixel depth label");
    mp_pixel_depth_Label = new QLabel;
    mp_pixel_depth_Label->setText(m_pixel_depth_label_String.arg("-"));
    mp_pixel_depth_Label->setToolTip(tr("Pixel bit depth", "Tool tip"));

    m_timestamp_label_String = tr("%3/%2/%1 %4:%5:%6.%7 UT", "Timestamp label");
    m_no_timestamp_label_String = tr("No Timestamp", "Timestamp label for no timestamp");
    mp_timestamp_Label = new QLabel;
    mp_timestamp_Label->setText(m_no_timestamp_label_String);
    mp_timestamp_Label->setToolTip(tr("Frame timestamp", "Tool tip"));
    mp_timestamp_Label->setAlignment(Qt::AlignRight);

    QHBoxLayout *slider_h_layout = new QHBoxLayout;
    slider_h_layout->setSpacing(0);
    slider_h_layout->setMargin(0);
    slider_h_layout->addWidget(mp_back_PushButton);
#ifdef __APPLE__
    slider_h_layout->addSpacing(15);
#else
    slider_h_layout->addSpacing(4);
#endif
    slider_h_layout->addWidget(mp_forward_PushButton);
#ifdef __APPLE__
    slider_h_layout->addSpacing(15);
#else
    slider_h_layout->addSpacing(4);
#endif
    slider_h_layout->addWidget(mp_frame_Slider);

    QHBoxLayout *controls_h_layout1 = new QHBoxLayout;
    controls_h_layout1->setSpacing(8);
    controls_h_layout1->setMargin(0);
    controls_h_layout1->addStretch(1);
    controls_h_layout1->addWidget(mp_zoom_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_frame_size_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_pixel_depth_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_colour_id_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_framecount_Label, 0, Qt::AlignTop | Qt::AlignRight);

    QHBoxLayout *controls_h_layout2 = new QHBoxLayout;
    controls_h_layout2->setMargin(0);
    controls_h_layout2->setSpacing(0);
    controls_h_layout2->setMargin(0);
    controls_h_layout2->addStretch();
    controls_h_layout2->addWidget(mp_fps_Label, 0, Qt::AlignRight);
    controls_h_layout2->addSpacing(8);
    controls_h_layout2->addWidget(mp_timestamp_Label, 0, Qt::AlignRight);

    QVBoxLayout *controls_v_layout1 = new QVBoxLayout;
    controls_v_layout1->setSpacing(0);
    controls_v_layout1->setMargin(0);
    controls_v_layout1->addLayout(controls_h_layout1);
    controls_v_layout1->addLayout(controls_h_layout2);

    QHBoxLayout *controls_h_layout = new QHBoxLayout;
#ifdef __APPLE__
    controls_h_layout->setSpacing(20);
#else
    controls_h_layout->setSpacing(4);
#endif
    controls_h_layout->setMargin(0);
    controls_h_layout->addWidget(mp_play_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addWidget(mp_stop_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addWidget(mp_repeat_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addWidget(mp_play_direction_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addStretch();
    controls_h_layout->addLayout(controls_v_layout1);

    QVBoxLayout *controls_main_layout = new QVBoxLayout;
    controls_main_layout->setSpacing(5);
    controls_main_layout->setMargin(5);
    controls_main_layout->addLayout(slider_h_layout);
    controls_main_layout->addLayout(controls_h_layout);
    
    setLayout(controls_main_layout);

    // Connect up frame slider signals
    connect(mp_frame_Slider, SIGNAL(start_marker_changed(int)), this, SIGNAL(start_marker_changed(int)));
    connect(mp_frame_Slider, SIGNAL(end_marker_changed(int)), this, SIGNAL(end_marker_changed(int)));
    connect(mp_frame_Slider, SIGNAL(markers_dialog_closed()), this, SIGNAL(markers_dialog_closed()));
    connect(mp_frame_Slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed_slot(int)));

    // Connect up button signals
    connect (mp_repeat_PushButton, SIGNAL(toggled(bool)), this, SLOT(repeat_button_toggled_slot(bool)));
    connect(mp_forward_PushButton, SIGNAL(pressed()), this, SLOT(forward_button_pressed_slot()));
    connect(mp_forward_PushButton, SIGNAL(released()), this, SLOT(forward_button_released_slot()));
    connect(mp_back_PushButton, SIGNAL(pressed()), this, SLOT(back_button_pressed_slot()));
    connect(mp_back_PushButton, SIGNAL(released()), this, SLOT(back_button_released_slot()));
    connect(mp_play_PushButton, SIGNAL(pressed()), this, SLOT(play_button_pressed_slot()));
    connect(mp_stop_PushButton, SIGNAL(pressed()), this, SLOT(stop_button_pressed_slot()));

    setToolTip(tr("Double-Click to Attach/Detach Playback Controls"));
}


void c_playback_controls_widget::slider_value_changed_slot(int value)
{
    update_framecount_label(value, mp_frame_Slider->maximum());
    //mp_framecount_Label->setText(m_framecount_label_String.arg(value).arg(mp_frame_Slider->maximum()));
    emit slider_value_changed(value);
}



int c_playback_controls_widget::slider_value()
{
    return mp_frame_Slider->value();
}


bool c_playback_controls_widget::get_markers_enable()
{
    return mp_frame_Slider->get_markers_enable();
}


void c_playback_controls_widget::set_repeat(bool repeat)
{
    return mp_frame_Slider->set_repeat(repeat);
}


void c_playback_controls_widget::goto_first_frame()
{
    mp_frame_Slider->goto_first_frame();
}


bool c_playback_controls_widget::goto_next_frame()
{
    bool ret = mp_frame_Slider->goto_next_frame();
    if (!ret) {
        emit stop_playing_signal();
        m_current_state = STATE_FINISHED;
    }

    return ret;
}


int c_playback_controls_widget::get_start_frame()
{
    return mp_frame_Slider->get_start_frame();
}


int c_playback_controls_widget::get_end_frame()
{
    return mp_frame_Slider->get_end_frame();
}


void c_playback_controls_widget::show_markers_dialog(bool show)
{
    return mp_frame_Slider->show_markers_dialog(show);
}


void c_playback_controls_widget::set_markers_show(bool show)
{
    return mp_frame_Slider->set_markers_show(show);
}


void c_playback_controls_widget::set_markers_enable(bool active)
{
    return mp_frame_Slider->set_markers_enable(active);
}


void c_playback_controls_widget::set_maximum_frame(int max_frame)
{
    return mp_frame_Slider->set_maximum_frame(max_frame);
}


void c_playback_controls_widget::set_start_marker_to_current()
{
    return mp_frame_Slider->set_start_marker_to_current();
}


void c_playback_controls_widget::set_start_marker_slot(int frame)
{
    return mp_frame_Slider->set_start_marker_slot(frame);
}


void c_playback_controls_widget::set_end_marker_to_current()
{
    return mp_frame_Slider->set_end_marker_to_current();
}


void c_playback_controls_widget::set_end_marker_slot(int frame)
{
    return mp_frame_Slider->set_end_marker_slot(frame);
}


void c_playback_controls_widget::reset_all_markers_slot()
{
    return mp_frame_Slider->reset_all_markers_slot();
}


void c_playback_controls_widget::play_direction_button_pressed_slot()
{
    switch (m_play_direction) {
    case 0:
        m_play_direction = 1;
        mp_play_direction_PushButton->setIcon(m_reverse_play_Pixmap);
        break;
    case 1:
        m_play_direction = 2;
        mp_play_direction_PushButton->setIcon(m_forward_and_reverse_play_Pixmap);
        break;
    default:
        m_play_direction = 0;
        mp_play_direction_PushButton->setIcon(m_forward_play_Pixmap);
    }


    c_persistent_data::m_play_direction = m_play_direction;
    mp_frame_Slider->set_direction(m_play_direction);
}


void c_playback_controls_widget::repeat_button_toggled_slot(bool checked)
{
    c_persistent_data::m_repeat = checked;
    mp_frame_Slider->set_repeat(checked);
}


void c_playback_controls_widget::forward_button_pressed_slot()
{
    if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
        bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
        int value = mp_frame_Slider->value();

        if (shift_key) {
            value += 50;
        } else {
            value++;
        }

        if (value > mp_frame_Slider->maximum()) {
            value = mp_frame_Slider->maximum();
        }

        mp_frame_Slider->setValue(value);

        // Start repeat timer
        m_forward_button_held = true;
        QTimer::singleShot(500, this, SLOT(forward_button_held_slot()));
    }
}


void c_playback_controls_widget::forward_button_released_slot()
{
    m_forward_button_held = false;
}


void c_playback_controls_widget::forward_button_held_slot()
{
    if (m_forward_button_held &&
        mp_forward_PushButton->rect().contains(mp_forward_PushButton->mapFromGlobal(QCursor::pos())) &&
        QApplication::mouseButtons() & Qt::LeftButton) {
        // Forward button is still being held
        if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
            bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
            int value = mp_frame_Slider->value();

            if (shift_key) {
                value += 50;
            } else {
                value++;
            }

            if (value > mp_frame_Slider->maximum()) {
                value = mp_frame_Slider->maximum();
            }

            mp_frame_Slider->setValue(value);
        }

        // Re-set repeat timer
        QTimer::singleShot(100, this, SLOT(forward_button_held_slot()));
    }
}


void c_playback_controls_widget::back_button_pressed_slot()
{
    if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
        bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
        int value = mp_frame_Slider->value();

        if (shift_key) {
            value -= 50;
        } else {
            value--;
        }

        if (value < mp_frame_Slider->minimum()) {
            value = mp_frame_Slider->minimum();
        }

        mp_frame_Slider->setValue(value);

        // Start repeat timer
        m_back_button_held = true;
        QTimer::singleShot(500, this, SLOT(back_button_held_slot()));
    }
}


void c_playback_controls_widget::back_button_released_slot()
{
    m_back_button_held = false;
}



void c_playback_controls_widget::back_button_held_slot()
{
    if (m_back_button_held &&
        mp_back_PushButton->rect().contains(mp_back_PushButton->mapFromGlobal(QCursor::pos())) &&
        QApplication::mouseButtons() & Qt::LeftButton) {
        // Forward button is still being held
        if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
            bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
            int value = mp_frame_Slider->value();

            if (shift_key) {
                value -= 50;
            } else {
                value--;
            }

            if (value < mp_frame_Slider->minimum()) {
                value = mp_frame_Slider->minimum();
            }

            mp_frame_Slider->setValue(value);
        }

        // Re-set repeat timer
        QTimer::singleShot(100, this, SLOT(back_button_held_slot()));
    }
}


void c_playback_controls_widget::play_button_pressed_slot()
{
    if (m_current_state == STATE_NO_FILE)
    {
        emit open_ser_file_signal();
    } else {
        if (m_current_state == STATE_PLAYING) {
            // Pause playing
            m_current_state = STATE_PAUSED;
            mp_play_PushButton->setIcon(m_play_Pixmap);
            emit stop_playing_signal();
        } else if (m_current_state == STATE_FINISHED) {
            // Start playing from start
            m_current_state = STATE_PLAYING;
            mp_play_PushButton->setIcon(m_pause_Pixmap);
            mp_frame_Slider->goto_first_frame();
            emit start_playing_signal();
        } else {
            // Start playing from current position if not before start marker
            m_current_state = STATE_PLAYING;
            mp_play_PushButton->setIcon(m_pause_Pixmap);
            mp_frame_Slider->goto_next_frame();
            emit start_playing_signal();
        }
    }
}


void c_playback_controls_widget::stop_button_pressed_slot()
{
    stop_playback();
}


void c_playback_controls_widget::start_playback()
{
    if (m_current_state == STATE_FINISHED) {
        // Start playing from start
        m_current_state = STATE_PLAYING;
        mp_play_PushButton->setIcon(m_pause_Pixmap);
        mp_frame_Slider->goto_first_frame();
        emit start_playing_signal();
    } else {
        // Start playing from current position if not before start marker
        m_current_state = STATE_PLAYING;
        mp_play_PushButton->setIcon(m_pause_Pixmap);
        mp_frame_Slider->goto_next_frame();
        emit start_playing_signal();
    }
}


void c_playback_controls_widget::pause_payback()
{
    if (m_current_state == STATE_PLAYING) {
        // Pause playing
        m_current_state = STATE_PAUSED;
        mp_play_PushButton->setIcon(m_play_Pixmap);
        emit stop_playing_signal();
    }
}


void c_playback_controls_widget::stop_playback()
{
    if (m_current_state != STATE_NO_FILE) {
        mp_play_PushButton->setIcon(m_play_Pixmap);
        emit stop_playing_signal();
        if (m_current_state != STATE_FINISHED)
        {
            m_current_state = STATE_STOPPED;
            mp_frame_Slider->goto_first_frame();
        }
    }
}


bool c_playback_controls_widget::is_playing()
{
    return (m_current_state == STATE_PLAYING);
}


void c_playback_controls_widget::update_framecount_label(int count, int maxcount)
{
    if (m_framecount_label_min_width < mp_framecount_Label->width())
    {
        m_framecount_label_min_width = mp_framecount_Label->width();
        mp_framecount_Label->setMinimumSize(m_framecount_label_min_width, 0);
    }

    mp_framecount_Label->setText(m_framecount_label_String.arg(count).arg(maxcount));
}

void c_playback_controls_widget::update_colour_id_label(QString colour_id)
{
    mp_colour_id_Label->setText(colour_id);
}


void c_playback_controls_widget::update_pixel_depth_label(int depth)
{
    mp_pixel_depth_Label->setText(m_pixel_depth_label_String.arg(depth));
}


void c_playback_controls_widget::update_frame_size_label(int width, int height)
{
    mp_frame_size_Label->setText(m_frame_size_label_String.arg(width).arg(height));
}


void c_playback_controls_widget::update_zoom_label_slot(int zoom)
{
    mp_zoom_Label->setText(m_zoom_label_String.arg(zoom));
}


void c_playback_controls_widget::update_fps_label(int fps)
{
    mp_fps_Label->setText(m_fps_label_String.arg(fps));
}


void c_playback_controls_widget::update_timestamp_label(uint64_t timestamp)
{
    if (timestamp > 0) {
        if (m_timestamp_label_min_width < mp_timestamp_Label->width())
        {
            m_timestamp_label_min_width = mp_timestamp_Label->width();
            mp_timestamp_Label->setMinimumSize(m_timestamp_label_min_width, 0);
        }

        int32_t ts_year, ts_month, ts_day, ts_hour, ts_minute, ts_second, ts_microsec;
        c_pipp_timestamp::timestamp_to_date(
            timestamp,
            &ts_year,
            &ts_month,
            &ts_day,
            &ts_hour,
            &ts_minute,
            &ts_second,
            &ts_microsec);

        int32_t ts_millisec = ts_microsec / 1000;
        mp_timestamp_Label->setText(m_timestamp_label_String
                                 .arg(ts_year, 4, 10, QLatin1Char( '0' ))
                                 .arg(ts_month, 2, 10, QLatin1Char( '0' ))
                                 .arg(ts_day, 2, 10, QLatin1Char( '0' ))
                                 .arg(ts_hour, 2, 10, QLatin1Char( '0' ))
                                 .arg(ts_minute, 2, 10, QLatin1Char( '0' ))
                                 .arg(ts_second, 2, 10, QLatin1Char( '0' ))
                                 .arg(ts_millisec, 3, 10, QLatin1Char( '0' )));
    } else {
        m_timestamp_label_min_width = 0;
        mp_timestamp_Label->setMinimumSize(0, 0);
        mp_timestamp_Label->setText(m_no_timestamp_label_String);
    }
}


void c_playback_controls_widget::mouseDoubleClickEvent(QMouseEvent *p_event)
{
    (void)p_event;
    emit double_clicked_signal();
}

