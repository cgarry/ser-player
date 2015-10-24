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


#ifndef FRAME_SLIDER_H
#define FRAME_SLIDER_H

#include <QSlider>


class c_markers_dialog;


class c_frame_slider : public QSlider
{
    Q_OBJECT

public:
    c_frame_slider(QWidget *parent = 0);
    bool get_markers_enable();
    void set_repeat(bool repeat);
    void set_direction(int dir);
    void goto_first_frame();
    bool goto_next_frame();
    int get_start_frame();
    int get_end_frame();

signals:
    void start_marker_changed(int frame);
    void end_marker_changed(int frame);

public slots:
    void show_markers_dialog();
    void set_markers_show(bool show);
    void set_markers_enable(bool active);
    void set_maximum_frame(int max_frame);
    void set_start_marker_to_current();
    void set_start_marker_slot(int frame);
    void set_end_marker_to_current();
    void set_end_marker_slot(int frame);
    void reset_all_markers_slot();

private slots:
    void ShowContextMenu(const QPoint& pos);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int position_for_value(int val) const;
    void draw_start_marker(int x_pos);
    void draw_end_marker(int x_pos);

    c_markers_dialog *mp_markers_Dialog;

    bool m_show_markers;
    bool m_markers_enabled;
    int m_start_marker;
    int m_end_marker;
    bool m_repeat;
    int m_direction;
    int m_current_direction;
    int m_handle_width;
    QRect m_start_marker_rect;
    QRect m_end_marker_rect;
};

#endif // FRAME_SLIDER_H
