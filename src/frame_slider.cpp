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

#include <QDebug>

#include <Qt>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <QStyleOptionSlider>

#include "frame_slider.h"



c_frame_slider::c_frame_slider(QWidget *parent)
    : QSlider(parent),
      m_start_marker(-1),
      m_end_marker(-1),
      m_repeat(false),
      m_direction(0),
      m_current_direction(0),
      m_moving_start_marker(false),
      m_moving_end_marker(false)
{
    // Set up context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect handle_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
}


void c_frame_slider::set_repeat(bool repeat)
{
    m_repeat = repeat;
}


void c_frame_slider::set_direction(int dir)
{
    m_direction = dir;
}


void c_frame_slider::goto_first_frame()
{
    if (m_direction == 0) {
        int start_frame = (m_start_marker == -1) ? minimum() : m_start_marker;
        setValue(start_frame);
    } else if (m_direction == 1) {
        int end_frame = (m_end_marker == -1) ? maximum() : m_end_marker;
        setValue(end_frame);
    } else if (m_direction == 2) {
        // Forward + Reverse play
        m_current_direction = 0;
        int start_frame = (m_start_marker == -1) ? minimum() : m_start_marker;
        setValue(start_frame);
    } else {
        qDebug() << "Unsupported direction!";
        exit(-1);
    }
}


bool c_frame_slider::goto_next_frame()
{
    bool ret = true;  // Default return value
    int current_value = value();
    int end_frame = (m_end_marker == -1) ? maximum() : m_end_marker;
    int start_frame = (m_start_marker == -1) ? minimum() : m_start_marker;

    if (m_direction == 0) {
        // Forward play
        if (current_value < start_frame) {
            // Move to start frame
            setValue(start_frame);
        } else if (current_value < end_frame) {
            // Not at end of forward playback
            setValue(current_value + 1);
        } else {
            // At end of forward playback
            if (m_repeat) {
                // Repeat is on, go back to start
                setValue(start_frame);
            } else {
                // Signal that playback has completed
                ret = false;
            }
        }
    } else if (m_direction == 1) {
        // Reverse play
        if (current_value > end_frame) {
            // Move to end frame which is the start position in reverse playback
            setValue(end_frame);
        } else if (current_value > start_frame) {
            // Not at end of reverse playback
            setValue(current_value - 1);
        } else {
            // At end of reverse playback
            if (m_repeat) {
                // Repeat is on, go back to start
                setValue(end_frame);
            } else {
                // Signal that playback has completed
                ret = false;
            }
        }
    } else if (m_direction == 2) {
        // Forward + Reverse play
        if (m_current_direction == 0) {
            // Currently playing forward
            if (current_value < start_frame) {
                // Move to start frame
                setValue(start_frame);
            } else if (current_value < end_frame) {
                // Not at end of forward playback
                setValue(current_value + 1);
            } else {
                // At end of forward playback - play backwards
                m_current_direction = 1;
                setValue(current_value);
            }
        } else {
            // Currently playing reverse
            if (current_value > end_frame) {
                // Move to end frame which is the start position in reverse playback
                setValue(end_frame);
            } else if (current_value > start_frame) {
                // Not at end of reverse playback
                setValue(current_value - 1);
            } else {
                // At end of reverse playback
                if (m_repeat) {
                    // Repeat is on, start playback again
                    m_current_direction = 0;
                    setValue(current_value);
                } else {
                    // Signal that playback has completed
                    ret = false;
                }
            }
        }
    } else {
        qDebug() << "Unsupported direction!";
        exit(-1);
    }

    return ret;
}


int c_frame_slider::get_start_frame()
{
    if (m_start_marker == -1) {
      return minimum();
    } else {
        return m_start_marker;
    }
}


int c_frame_slider::get_end_frame()
{
    if (m_end_marker == -1) {
      return maximum();
    } else {
        return m_end_marker;
    }
}


bool c_frame_slider::set_start_marker_slot(int frame)
{
    bool ret = true;
    if (frame < minimum() || frame > maximum()) {
        // Start marker could not be set
        m_start_marker = -1;
        ret = false;
    } else if (m_end_marker != -1 && frame > m_end_marker) {
        m_start_marker = m_end_marker;
    } else {
        // Set start marker
        m_start_marker = frame;
    }

    emit start_marker_changed(m_start_marker);
    update();
    return ret;
}


bool c_frame_slider::set_end_marker_slot(int frame)
{
    bool ret = true;
    if (frame < minimum() || frame > maximum()) {
        // End marker could not be set
        m_end_marker = -1;
        ret = false;
    } else if (frame < m_start_marker) {
        m_end_marker = m_start_marker;
    } else {
        // Set end marker
        m_end_marker = frame;
    }

    emit end_marker_changed(m_end_marker);
    update();
    return ret;
}


void c_frame_slider::delete_all_markers_slot()
{
    m_start_marker = -1;
    m_end_marker = -1;
}


int c_frame_slider::position_for_value(int val) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    opt.subControls = QStyle::SC_All;
    int available = opt.rect.width() - style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
    QRect groove_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    return QStyle::sliderPositionFromValue(opt.minimum, opt.maximum, val, available) + groove_rect.left();
}


int c_frame_slider::value_for_position(int position) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    opt.subControls = QStyle::SC_All;
    int available = opt.rect.width() - style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
    QRect groove_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    return QStyle::sliderValueFromPosition(opt.minimum, opt.maximum, position, available);
}


void c_frame_slider::ShowContextMenu(const QPoint& pos) // this is a slot
{
    // for most widgets
    QPoint globalPos = mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
    bool inside_start_marker_area = false;
    bool inside_end_marker_area = false;
    int start_frame_at_mouse_pos = value_for_position(pos.x());
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect handle_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    int end_frame_at_mouse_pos = value_for_position(pos.x() - handle_rect.width());

    if (m_start_marker > 0) {
        int start_marker_pos = position_for_value(m_start_marker);
        if (pos.x() <= start_marker_pos) {
            inside_start_marker_area = true;
        }
    }

    if (m_end_marker > 0) {
        int end_marker_pos = position_for_value(m_end_marker);
        if (pos.x() >= end_marker_pos) {
            inside_end_marker_area = true;
        }
    }

    QMenu markers_Menu;
    QAction *move_start_marker_Act = NULL;
    QAction *move_end_marker_Act = NULL;
    if (inside_start_marker_area || (m_start_marker != -1 && !inside_end_marker_area)) {
        if (start_frame_at_mouse_pos != minimum()) {
            move_start_marker_Act = markers_Menu.addAction(tr("Move Start Marker To Here (Frame %1)").arg(start_frame_at_mouse_pos));
        }
    }

    if (inside_end_marker_area || (m_end_marker != -1 && !inside_start_marker_area)) {
        if (end_frame_at_mouse_pos != maximum()) {
            move_end_marker_Act = markers_Menu.addAction(tr("Move End Marker To Here (Frame %1)").arg(end_frame_at_mouse_pos));
        }
    }

    if (move_start_marker_Act != NULL || move_end_marker_Act != NULL) {
        QAction* selectedItem = markers_Menu.exec(globalPos);
        if (selectedItem != NULL) {
            if (selectedItem == move_start_marker_Act) {
                set_start_marker_slot(start_frame_at_mouse_pos);
            } else if (selectedItem == move_end_marker_Act) {
                set_end_marker_slot(end_frame_at_mouse_pos);
            }

            update();
        }
    }
}


void c_frame_slider::mousePressEvent(
        QMouseEvent * ev)
{
    bool pass_event_on = true;
    if (ev->button() == Qt::LeftButton &&
        m_start_marker != -1 &&
        m_start_marker_rect.contains(ev->pos()))
    {
        // Start dragging the start marker
        m_moving_start_marker = true;
        m_start_marker_initial_pos = ev->pos();
        m_start_marker_current_pos = m_start_marker_initial_pos;
        pass_event_on = false;
    } else if (ev->button() == Qt::LeftButton &&
               m_end_marker != -1 &&
               m_end_marker_rect.contains(ev->pos()))
    {
        // Start dragging the end marker
        m_moving_end_marker = true;
        m_end_marker_initial_pos = ev->pos();
        m_end_marker_current_pos = m_end_marker_initial_pos;
        pass_event_on = false;
    }

    // Pass event on
    if (pass_event_on) {
        QSlider::mousePressEvent(ev);
    }
}


void c_frame_slider:: mouseMoveEvent(
        QMouseEvent * ev)
{
    bool pass_event_on = true;
    if ((ev->buttons() & Qt::LeftButton) && m_moving_start_marker) {
        m_start_marker_current_pos = ev->pos();
        update();
        pass_event_on = false;
    } else if ((ev->buttons() & Qt::LeftButton) && m_moving_end_marker) {
        m_end_marker_current_pos = ev->pos();
        update();
        pass_event_on = false;
    }

    // Pass event on
    if (pass_event_on) {
        QSlider::mouseMoveEvent(ev);
    }
}


void c_frame_slider::mouseReleaseEvent(
        QMouseEvent *ev)
{
    bool pass_event_on = true;
    if (ev->button() == Qt::LeftButton && m_moving_start_marker) {
        m_moving_start_marker = false;
        set_start_marker_slot(value_for_position(ev->pos().x()));
        pass_event_on = false;
    } else if (ev->button() == Qt::LeftButton && m_moving_end_marker) {
        m_moving_end_marker = false;
        set_end_marker_slot(value_for_position(ev->pos().x() - m_handle_width));
        pass_event_on = false;
    }

    // Pass event on
    if (pass_event_on) {
        QSlider::mouseReleaseEvent(ev);
    }
}


void c_frame_slider::draw_start_marker(int x_pos)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect groove_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QPainter painter(this);

    painter.drawLine(x_pos,
                     groove_rect.top(),
                     x_pos,
                     groove_rect.bottom());

    // Draw start marker triangle
    QPolygon start_triangle;
    start_triangle << QPoint(x_pos, groove_rect.bottom())
                   << QPoint(x_pos - 6, groove_rect.bottom())
                   << QPoint(x_pos, groove_rect.bottom() - 6);

    // Brush
    QBrush brush;
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);

    // Draw triangle
    painter.drawPolygon(start_triangle);

    // Fill triangle
    QPainterPath path;
    path.addPolygon(start_triangle);
    painter.fillPath(path, brush);
}


void c_frame_slider::draw_end_marker(int x_pos)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect groove_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QPainter painter(this);

    painter.drawLine(x_pos,
                     groove_rect.top(),
                     x_pos,
                     groove_rect.bottom());

    // Draw start marker triangle
    QPolygon end_triangle;
    end_triangle << QPoint(x_pos, groove_rect.bottom())
                 << QPoint(x_pos + 6, groove_rect.bottom())
                 << QPoint(x_pos, groove_rect.bottom() - 6);

    // Brush
    QBrush brush;
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);

    // Draw triangle
    painter.drawPolygon(end_triangle);

    // Fill triangle
    QPainterPath path;
    path.addPolygon(end_triangle);
    painter.fillPath(path, brush);
}


void c_frame_slider::paintEvent(QPaintEvent *ev)
{
    QSlider::paintEvent(ev);
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
    if (tickPosition() != NoTicks) {
      opt.subControls |= QStyle::SC_SliderTickmarks;
    }

    // Get details of slider layout
    QRect groove_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect handle_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    m_handle_width = handle_rect.width();

    QPainter painter(this);

    // Draw frames excluded from start rectangle
    if (m_start_marker >= minimum()) {
        int start_pos = position_for_value(minimum());
        int end_pos = position_for_value(m_start_marker) + handle_rect.width()/2 - 1;
        QRect rect(start_pos,
                   groove_rect.top(),
                   end_pos - start_pos,
                   groove_rect.height());
        painter.fillRect(rect, QBrush(QColor(255, 0, 0, 64)));
    }

    // Draw frames excluded from end rectangle
    if (m_end_marker != -1 && m_end_marker <= maximum()) {
        int start_pos = position_for_value(m_end_marker) + handle_rect.width()/2;
        int end_pos = position_for_value(maximum()) + handle_rect.width() - 1;
        QRect rect(start_pos,
                   groove_rect.top(),
                   end_pos - start_pos,
                   groove_rect.height());
        painter.fillRect(rect, QBrush(QColor(255, 0, 0, 64)));
    }

    // Draw start marker when dragging
    if (m_moving_start_marker) {
        int x_pos = m_start_marker_current_pos.x();
        draw_start_marker(x_pos);
    }

    // Draw start marker
    if (m_start_marker >= minimum()) {
        int slider_pos = position_for_value(m_start_marker) + handle_rect.width()/2 - 1;
        draw_start_marker(slider_pos);
        m_start_marker_rect = QRect(slider_pos - 6, groove_rect.top(), 6, groove_rect.height());
    }

    // Draw start marker when moving
    if (m_moving_end_marker) {
        int x_pos = m_end_marker_current_pos.x();
        draw_end_marker(x_pos);
    }

    // Draw end marker
    if (m_end_marker != -1 && m_end_marker <= maximum()) {
        // Draw start marker
        int slider_pos = position_for_value(m_end_marker) + handle_rect.width()/2;
        draw_end_marker(slider_pos);
        m_end_marker_rect = QRect(slider_pos, groove_rect.top(), 6, groove_rect.height());
    }


}
