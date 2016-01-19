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


#include "image_widget.h"
#include "selection_box_dialog.h"

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

namespace {
    enum e_drag_handles {
        m_drag_handle_none = 0,
        m_drag_handle_all,
        m_drag_handle_top_left,
        m_drag_handle_top_centre,
        m_drag_handle_top_right,
        m_drag_handle_middle_left,
        m_drag_handle_middle_right,
        m_drag_handle_bottom_left,
        m_drag_handle_bottom_centre,
        m_drag_handle_bottom_right};

    const int drag_box_size = 15;
}


c_image_Widget::c_image_Widget(QWidget *parent) :
    QWidget(parent),
    m_zoom_level(100),
    m_scale_factor(1.0),
    m_active_drag_handle(m_drag_handle_bottom_right)
{
    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    p.setWidthForHeight(true);
    setSizePolicy(p);

    mp_selection_box_dialog = new c_selection_box_dialog(this);
    m_image_size  = QSize(1, 1);
    m_current_Size = QSize(1, 1);
    m_selected_area_top_left = QPoint(0, 0);
    m_selected_area_bottom_right = QPoint(0, 0);
    m_sel_top_left_Rect = QRect(0, 0, 0, 0);
    m_sel_top_right_Rect = QRect(0, 0, 0, 0);
    m_sel_bottom_left_Rect = QRect(0, 0, 0, 0);
    m_sel_bottom_right_Rect = QRect(0, 0, 0, 0);
    m_sel_top_centre_Rect = QRect(0, 0, 0, 0);
    m_sel_bottom_centre_Rect = QRect(0, 0, 0, 0);
    m_sel_middle_left_Rect = QRect(0, 0, 0, 0);
    m_sel_middle_right_Rect = QRect(0, 0, 0, 0);
    m_sel_area_Rect = QRect(0, 0, 0, 0);
    connect(mp_selection_box_dialog, SIGNAL(selection_box_changed(QRect)), this, SLOT(set_selection_slot(QRect)));
    connect(mp_selection_box_dialog, SIGNAL(selection_box_complete(bool,QRect)), this, SIGNAL(selection_box_complete_signal(bool,QRect)));
}


void c_image_Widget::enable_area_selection_slot(const QSize &frame_size, const QRect &selected_area)
{
    m_selected_area_top_left = selected_area.topLeft();
    m_selected_area_bottom_right = selected_area.bottomRight();
    mp_selection_box_dialog->start_get_selection_box_slot(frame_size.width(), frame_size.height());
    mp_selection_box_dialog->set_selection_box_slot(selected_area);
    update();
}


void c_image_Widget::cancel_area_selection_slot()
{
    mp_selection_box_dialog->cancel_get_selection_box_slot();
    update();
}


void c_image_Widget::set_selection_slot(QRect selection)
{
    m_selected_area_top_left = selection.topLeft();
    m_selected_area_bottom_right = selection.bottomRight();
    update();
}


void c_image_Widget::disable_area_selection()
{
    mp_selection_box_dialog->hide();
    update();
}


int c_image_Widget::get_zoom_level()
{
    return m_zoom_level;
}


QSize c_image_Widget::get_image_size()
{
    return m_image_size;
}


void c_image_Widget::mousePressEvent(QMouseEvent *p_event)
{
    // Early return
    if (!mp_selection_box_dialog->isVisible()) {
        return;
    }

    if (m_sel_top_left_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_top_left;
    } else if (m_sel_top_right_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_top_right;
    } else if (m_sel_bottom_left_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_bottom_left;
    } else if (m_sel_bottom_right_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_bottom_right;
    } else if (m_sel_top_centre_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_top_centre;
    } else if (m_sel_bottom_centre_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_bottom_centre;
    } else if (m_sel_middle_left_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_middle_left;
    } else if (m_sel_middle_right_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_middle_right;
    } else if (m_sel_area_Rect.contains(p_event->pos())) {
        m_active_drag_handle = m_drag_handle_all;
    }

    if (m_active_drag_handle != m_drag_handle_none) {
        m_drag_start_point = p_event->pos();
    }
}


void c_image_Widget::mouseMoveEvent(QMouseEvent *p_event)
{
    // Early return
    if (!mp_selection_box_dialog->isVisible()) {
        return;
    }

    QPoint delta = p_event->pos() - m_drag_start_point;
    delta /= m_scale_factor;
    m_drag_start_point = p_event->pos();
    switch (m_active_drag_handle) {
    case m_drag_handle_top_centre:
        m_selected_area_top_left.setY(m_selected_area_top_left.y() + delta.y());
        break;
    case m_drag_handle_bottom_centre:
        m_selected_area_bottom_right.setY(m_selected_area_bottom_right.y() + delta.y());
        break;
    case m_drag_handle_middle_left:
        m_selected_area_top_left.setX(m_selected_area_top_left.x() + delta.x());
        break;
    case m_drag_handle_middle_right:
        m_selected_area_bottom_right.setX(m_selected_area_bottom_right.x() + delta.x());
        break;
    case m_drag_handle_top_left:
        m_selected_area_top_left += delta;
        break;
    case m_drag_handle_top_right:
        m_selected_area_top_left.setY(m_selected_area_top_left.y() + delta.y());
        m_selected_area_bottom_right.setX(m_selected_area_bottom_right.x() + delta.x());
        break;
    case m_drag_handle_bottom_left:
        m_selected_area_top_left.setX(m_selected_area_top_left.x() + delta.x());
        m_selected_area_bottom_right.setY(m_selected_area_bottom_right.y() + delta.y());
        break;
    case m_drag_handle_bottom_right:
        m_selected_area_bottom_right += delta;
        break;
    case m_drag_handle_all:
        m_selected_area_top_left += delta;
        m_selected_area_bottom_right += delta;
        break;
    }

    if (m_active_drag_handle != m_drag_handle_none) {
        if (m_active_drag_handle == m_drag_handle_all) {
            // Check for limits when moving the entire area
            if (m_selected_area_top_left.x() < 0) {
                QPoint correction = QPoint(-m_selected_area_top_left.x(), 0);
                m_selected_area_top_left += correction;
                m_selected_area_bottom_right += correction;
            }

            if (m_selected_area_top_left.y() < 0) {
                QPoint correction = QPoint(0, -m_selected_area_top_left.y());
                m_selected_area_top_left += correction;
                m_selected_area_bottom_right += correction;
            }

            int max_x = (m_image_Pixmap.width() - 1);
            if (m_selected_area_bottom_right.x() > max_x) {
                QPoint correction = QPoint(max_x - m_selected_area_bottom_right.x(), 0);
                m_selected_area_top_left += correction;
                m_selected_area_bottom_right += correction;
            }

            int max_y = (m_image_Pixmap.height() - 1);
            if (m_selected_area_bottom_right.y() > max_y) {
                QPoint correction = QPoint(0, max_y - m_selected_area_bottom_right.y());
                m_selected_area_top_left += correction;
                m_selected_area_bottom_right += correction;
            }
        } else {
            // Check for limits when moving a normal drag box
            if (m_selected_area_top_left.x() < 0) {
                m_selected_area_top_left.setX(0);
            }

            if (m_selected_area_top_left.y() < 0) {
                m_selected_area_top_left.setY(0);
            }

            int max_x = (qreal)(m_image_Pixmap.width() - 1);
            if (m_selected_area_bottom_right.x() > max_x) {
                m_selected_area_bottom_right.setX(max_x);
            }

            int max_y = (qreal)(m_image_Pixmap.height() - 1);
            if (m_selected_area_bottom_right.y() > max_y) {
                m_selected_area_bottom_right.setY(max_y);
            }

            if (m_selected_area_bottom_right.x() < m_selected_area_top_left.x() + 3 * drag_box_size) {
                switch (m_active_drag_handle) {
                case m_drag_handle_top_right:
                case m_drag_handle_middle_right:
                case m_drag_handle_bottom_right:
                    m_selected_area_bottom_right.setX(m_selected_area_top_left.x() + 3 * drag_box_size);
                    break;
                default:
                    m_selected_area_top_left.setX(m_selected_area_bottom_right.x() - 3 * drag_box_size);
                }
            }

            if (m_selected_area_bottom_right.y() < m_selected_area_top_left.y() + 3 * drag_box_size) {
                switch (m_active_drag_handle) {
                case m_drag_handle_bottom_left:
                case m_drag_handle_bottom_centre:
                case m_drag_handle_bottom_right:
                    m_selected_area_bottom_right.setY(m_selected_area_top_left.y() + 3 * drag_box_size);
                    break;
                default:
                    m_selected_area_top_left.setY(m_selected_area_bottom_right.y() - 3 * drag_box_size);
                }
            }
        }

        QRect selected_area = QRect(m_selected_area_top_left, m_selected_area_bottom_right);
        mp_selection_box_dialog->set_selection_box_slot(selected_area);
        update();
    }
}


void c_image_Widget::mouseReleaseEvent(QMouseEvent *p_event)
{
    (void)p_event;  // Remove unused parameter warning

    // Early return
    if (!mp_selection_box_dialog->isVisible()) {
        return;
    }

    m_active_drag_handle = m_drag_handle_none;
}


void c_image_Widget::mouseDoubleClickEvent(QMouseEvent *p_event)
{
    (void)p_event;  // Remove unused parameter warning
    emit double_click_signal();
}


void c_image_Widget::paintEvent(QPaintEvent *p_event)
{
    QWidget::paintEvent(p_event);

    // Early return
    if (m_image_Pixmap.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QSize pixSize = m_image_Pixmap.size();
    pixSize.scale(p_event->rect().size(), Qt::KeepAspectRatio);

    m_zoom_level = (pixSize.width() * 100) / m_image_Pixmap.size().width();

    QPixmap scaled_Pixmap = m_image_Pixmap.scaled(pixSize,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);

    if (mp_selection_box_dialog->isVisible()) {
        draw_selection_rectangle(scaled_Pixmap);
    }

    // Calculate position to draw image in middle of widget
    int x = (p_event->rect().size().width() - scaled_Pixmap.size().width()) / 2;
    int y = (p_event->rect().size().height() - scaled_Pixmap.size().height()) / 2;
    m_sel_top_left_Rect.translate(x, y);
    m_sel_top_right_Rect.translate(x, y);
    m_sel_bottom_left_Rect.translate(x, y);
    m_sel_bottom_right_Rect.translate(x, y);
    m_sel_top_centre_Rect.translate(x, y);
    m_sel_bottom_centre_Rect.translate(x, y);
    m_sel_middle_left_Rect.translate(x, y);
    m_sel_middle_right_Rect.translate(x, y);
    m_sel_area_Rect.translate(x, y);

    painter.drawPixmap(QPoint(x, y), scaled_Pixmap);
}


void c_image_Widget::draw_selection_rectangle(QPixmap &pixmap)
{
    int x_scale_num = pixmap.width()-1;
    int x_scale_denum = m_image_Pixmap.size().width()-1;
    int y_scale_num = pixmap.height()-1;
    int y_scale_denum = m_image_Pixmap.size().height()-1;

    m_scale_factor;
    if (y_scale_num > x_scale_num) {
        m_scale_factor = qreal(y_scale_num) / y_scale_denum;
    } else {
        m_scale_factor = qreal(x_scale_num) / x_scale_denum;
    }

    QPoint selected_area_top_left = QPoint((m_selected_area_top_left.x() * x_scale_num) / x_scale_denum,
                                           (m_selected_area_top_left.y() * y_scale_num) / y_scale_denum);
    QPoint selected_area_bottom_right = QPoint((m_selected_area_bottom_right.x() * x_scale_num) / x_scale_denum,
                                               (m_selected_area_bottom_right.y() * y_scale_num) / y_scale_denum);

    int selected_width = selected_area_bottom_right.x() - selected_area_top_left.x() + 1;
    int selected_height = selected_area_bottom_right.y() - selected_area_top_left.y() + 1;
    m_sel_area_Rect = QRect(selected_area_top_left, QSize(selected_width-1, selected_height-1));

    // Drag handle rectangles
    m_sel_top_left_Rect = QRect(selected_area_top_left, QSize(drag_box_size, drag_box_size));

    m_sel_top_right_Rect = m_sel_top_left_Rect;
    m_sel_top_right_Rect.translate(selected_width - drag_box_size - 1, 0);

    m_sel_bottom_left_Rect = m_sel_top_left_Rect;
    m_sel_bottom_left_Rect.translate(0, selected_height - drag_box_size - 1);

    m_sel_bottom_right_Rect = m_sel_bottom_left_Rect;
    m_sel_bottom_right_Rect.translate(selected_width - drag_box_size - 1, 0);

    m_sel_top_centre_Rect = m_sel_top_left_Rect;
    m_sel_bottom_centre_Rect = m_sel_bottom_left_Rect;
    int shift_to_centre = (selected_width - drag_box_size) / 2;
    m_sel_top_centre_Rect.translate(shift_to_centre, 0);
    m_sel_bottom_centre_Rect.translate(shift_to_centre, 0);

    m_sel_middle_left_Rect = m_sel_top_left_Rect;
    m_sel_middle_right_Rect = m_sel_top_right_Rect;
    int shift_to_middle = (selected_height - drag_box_size) / 2;
    m_sel_middle_left_Rect.translate(0, shift_to_middle);
    m_sel_middle_right_Rect.translate(0, shift_to_middle);

    QPainter p(&pixmap);
    p.setPen(QPen(Qt::red, 1, Qt::DashLine));
    // Draw main rectangle
    p.drawRect(m_sel_area_Rect);

    // Draw drag boxes
    p.drawRect(m_sel_top_left_Rect);
    p.drawRect(m_sel_top_right_Rect);
    p.drawRect(m_sel_bottom_right_Rect);
    p.drawRect(m_sel_bottom_left_Rect);
    p.drawRect(m_sel_top_centre_Rect);
    p.drawRect(m_sel_bottom_centre_Rect);
    p.drawRect(m_sel_middle_left_Rect);
    p.drawRect(m_sel_middle_right_Rect);

    p.end();
}


const QPixmap* c_image_Widget::pixmap() const
{
    return &m_image_Pixmap;
}


QSize c_image_Widget::minimumSizeHint() const
{
    return QSize(100, 100);
}


QSize c_image_Widget::sizeHint() const
{
    if (m_image_Pixmap.isNull()) {
        return QSize(100, 100);
    } else {
        // To do
        // When zooming is active, return zoomed dimensions rather than original dimensions.
        // This allows adjustSize() to be called on the main window to make it all fit nicely.
        // Note that a new funtion will be required to return the zoom to 100% when required.
        //wibble
        return m_current_Size;
    }
}


void c_image_Widget::resizeEvent(QResizeEvent *e)
{
    int w = e->size().width();
    int h = heightForWidth(w);
    m_current_Size = QSize(w, h);
    updateGeometry();
}


int c_image_Widget::heightForWidth(int width) const
{
    int height = ((qreal)m_image_Pixmap.height()*width)/m_image_Pixmap.width();
    return height;
}


int c_image_Widget::widthForHeight(int height) const
{
    int width = ((qreal)m_image_Pixmap.width()*height)/m_image_Pixmap.height();
    return width;
}


void c_image_Widget::setPixmap (const QPixmap &pixmap){
    m_image_Pixmap = pixmap;
    m_image_size = pixmap.size();
    m_current_Size = pixmap.size();
    updateGeometry();
    repaint();
}

