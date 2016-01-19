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


#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include <QWidget>

// Forward declarations
class c_selection_box_dialog;


class c_image_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit c_image_Widget(QWidget *parent = 0);
    const QPixmap* pixmap() const;
    int get_zoom_level();
    QSize get_image_size();
    void disable_area_selection();

signals:
    void double_click_signal();
    void selection_box_complete_signal(bool accepted, QRect selection_rect);

public slots:
    void setPixmap(const QPixmap&);
    void enable_area_selection_slot(const QSize &frame_size, const QRect &selected_area);
    void set_selection_slot(QRect selection);

protected:
    void paintEvent(QPaintEvent *);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    int heightForWidth(int width) const;
    int widthForHeight(int height) const;
    void resizeEvent(QResizeEvent *p_event);
    void mousePressEvent(QMouseEvent *p_event);
    void mouseMoveEvent(QMouseEvent *p_event);
    void mouseReleaseEvent(QMouseEvent *p_event);
    void mouseDoubleClickEvent(QMouseEvent *p_event);

private:
    void c_image_Widget::draw_selection_rectangle(QPixmap &pixmap);

    c_selection_box_dialog *mp_selection_box_dialog;
    QPixmap m_image_Pixmap;
    QSize m_image_size;
    QSize m_current_Size;
    int m_zoom_level;
    bool m_selected_area_enable;
    QPoint m_selected_area_top_left;
    QPoint m_selected_area_bottom_right;
    QRect m_sel_top_left_Rect;
    QRect m_sel_top_right_Rect;
    QRect m_sel_bottom_left_Rect;
    QRect m_sel_bottom_right_Rect;
    QRect m_sel_top_centre_Rect;
    QRect m_sel_bottom_centre_Rect;
    QRect m_sel_middle_left_Rect;
    QRect m_sel_middle_right_Rect;
    QRect m_sel_area_Rect;

    // Dragging variables
    qreal m_scale_factor;
    int m_active_drag_handle;
    QPoint m_drag_start_point;
};

#endif // IMAGE_WIDGET_H
