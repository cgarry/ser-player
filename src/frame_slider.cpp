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
#include <QPainterPath>
#include <QSlider>
#include <QStyleOptionSlider>

#include <cassert>

#include "persistent_data.h"
#include "frame_slider.h"
#include "markers_dialog.h"


c_frame_slider::c_frame_slider(QWidget *parent)
    : QSlider(parent),
      m_show_markers(false),
      m_markers_enabled(c_persistent_data::m_markers_enabled),
      m_start_marker(1),
      m_end_marker(1),
      m_repeat(false),
      m_direction(0),
      m_current_direction(0)
{
    setMinimum(1);
    setOrientation(Qt::Horizontal);

    // Set up context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    // Create an instance of the start/end markers dialog
    mp_markers_Dialog = new c_markers_dialog(this);
    mp_markers_Dialog->hide();
    connect(this, SIGNAL(start_marker_changed(int)), mp_markers_Dialog, SLOT(set_start_marker_slot(int)));
    connect(this, SIGNAL(end_marker_changed(int)), mp_markers_Dialog, SLOT(set_end_marker_slot(int)));
    connect(mp_markers_Dialog, SIGNAL(start_marker_changed(int)), this, SLOT(set_start_marker_slot(int)));
    connect(mp_markers_Dialog, SIGNAL(end_marker_changed(int)), this, SLOT(set_end_marker_slot(int)));
    connect(mp_markers_Dialog, SIGNAL(set_start_marker_to_current()), this, SLOT(set_start_marker_to_current()));
    connect(mp_markers_Dialog, SIGNAL(set_end_marker_to_current()), this, SLOT(set_end_marker_to_current()));
    connect(mp_markers_Dialog, SIGNAL(markers_enabled_changed(bool)), this, SLOT(set_markers_enable(bool)));
    connect(mp_markers_Dialog, SIGNAL(rejected()), this, SIGNAL(markers_dialog_closed()));
}


void c_frame_slider::set_markers_show(bool show)
{
    m_show_markers = show;
}



void c_frame_slider::set_markers_enable(bool enabled)
{
    m_markers_enabled = enabled;
    c_persistent_data::m_markers_enabled = m_markers_enabled;
    update();
}


bool c_frame_slider::get_markers_enable()
{
    return m_markers_enabled;
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
    assert(m_direction >= 0 && m_direction <= 2);  // Unsupported direction

    if (m_direction == 0) {
        setValue(get_start_frame());
    } else if (m_direction == 1) {
        setValue(get_end_frame());
    } else if (m_direction == 2) {
        // Forward + Reverse play
        m_current_direction = 0;
        setValue(get_start_frame());
    }
}


bool c_frame_slider::goto_next_frame()
{
    assert(m_direction >= 0 && m_direction <= 2);  // Unsupported direction

    bool ret = true;  // Default return value
    int current_value = value();

    if (m_direction == 0) {
        // Forward play
        if (current_value < get_start_frame()) {
            // Move to start frame
            setValue(get_start_frame());
        } else if (current_value < get_end_frame()) {
            // Not at end of forward playback
            setValue(current_value + 1);
        } else {
            // At end of forward playback
            if (m_repeat) {
                // Repeat is on, go back to start
                setValue(get_start_frame());
            } else {
                // Signal that playback has completed
                ret = false;
            }
        }
    } else if (m_direction == 1) {
        // Reverse play
        if (current_value > get_end_frame()) {
            // Move to end frame which is the start position in reverse playback
            setValue(get_end_frame());
        } else if (current_value > get_start_frame()) {
            // Not at end of reverse playback
            setValue(current_value - 1);
        } else {
            // At end of reverse playback
            if (m_repeat) {
                // Repeat is on, go back to start
                setValue(get_end_frame());
            } else {
                // Signal that playback has completed
                ret = false;
            }
        }
    } else if (m_direction == 2) {
        // Forward + Reverse play
        if (m_current_direction == 0) {
            // Currently playing forward
            if (current_value < get_start_frame()) {
                // Move to start frame
                setValue(get_start_frame());
            } else if (current_value < get_end_frame()) {
                // Not at end of forward playback
                setValue(current_value + 1);
            } else {
                // At end of forward playback - play backwards
                m_current_direction = 1;
                setValue(current_value);
            }
        } else {
            // Currently playing reverse
            if (current_value > get_end_frame()) {
                // Move to end frame which is the start position in reverse playback
                setValue(get_end_frame());
            } else if (current_value > get_start_frame()) {
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
    }

    return ret;
}


int c_frame_slider::get_start_frame()
{
    int ret;
    if (m_markers_enabled) {
        ret = m_start_marker;
    } else {
        ret = minimum();
    }

    return ret;
}


int c_frame_slider::get_end_frame()
{
    int ret;
    if (m_markers_enabled) {
        ret = m_end_marker;
    } else {
        ret = maximum();
    }

    return ret;
}


void c_frame_slider::show_markers_dialog(bool show)
{
    mp_markers_Dialog->setVisible(show);
}


void c_frame_slider::set_maximum_frame(int max_frame)
{
    setMaximum(max_frame);
    mp_markers_Dialog->set_maximum_frame(max_frame);
}


void c_frame_slider::set_start_marker_to_current()
{
    set_start_marker_slot(value());
}


void c_frame_slider::set_start_marker_slot(int value)
{
    if (value < minimum()) {
        m_start_marker = minimum();
    } else if (value > get_end_frame()) {
        set_end_marker_slot(maximum());
        m_start_marker = value;
    } else {
        // Set start marker
        m_start_marker = value;
    }

    emit start_marker_changed(m_start_marker);
    update();
}


void c_frame_slider::set_end_marker_to_current()
{
    set_end_marker_slot(value());
}


void c_frame_slider::set_end_marker_slot(int value)
{
    if (value > maximum()) {
        m_end_marker =  maximum();
    } else if (value < get_start_frame()) {
        set_start_marker_slot(minimum());
        m_end_marker = value;
    } else {
        // Set end marker
        m_end_marker = value;
    }

    emit end_marker_changed(m_end_marker);
    update();
}


void c_frame_slider::reset_all_markers_slot()
{
    m_start_marker = minimum();
    m_end_marker = maximum();
    mp_markers_Dialog->reset_markers_slot();
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


void c_frame_slider::ShowContextMenu(const QPoint& pos) // this is a slot
{
    if (m_show_markers && m_markers_enabled) {
        // for most widgets
        QPoint globalPos = mapToGlobal(pos);
        // for QAbstractScrollArea and derived classes you would use:
        // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
        bool inside_start_marker = m_start_marker_rect.contains(pos);
        bool inside_end_marker = m_end_marker_rect.contains(pos);

        if (inside_start_marker) {
            // Reset start marker
            set_start_marker_slot(minimum());
            update();
        } else if (inside_end_marker) {
            // Reset end marker
            set_end_marker_slot(maximum());
            update();
        } else {
            QMenu markers_Menu;
            QAction *move_start_marker_to_current_Act = nullptr;
            QAction *move_end_marker_to_current_Act = nullptr;
            QAction *reset_markers_Act = nullptr;
            move_start_marker_to_current_Act = markers_Menu.addAction(tr("Move Start Marker To Current Frame"));
            move_end_marker_to_current_Act = markers_Menu.addAction(tr("Move End Marker To Current Frame"));
            reset_markers_Act = markers_Menu.addAction(tr("Reset Markers"));

            QAction* selectedItem = markers_Menu.exec(globalPos);
            if (selectedItem != nullptr) {
                if (selectedItem == move_start_marker_to_current_Act) {
                    set_start_marker_slot(value());
                } else if (selectedItem == move_end_marker_to_current_Act) {
                    set_end_marker_slot(value());
                } else if (selectedItem == reset_markers_Act) {
                    set_start_marker_slot(minimum());
                    set_end_marker_slot(maximum());
                }

                update();
            }
        }
    }
}


void c_frame_slider::draw_start_marker(int x_pos)
{
    QPainter painter(this);

    painter.drawLine(x_pos,
                     rect().top(),
                     x_pos,
                     rect().bottom());

    // Draw start marker triangle
    QPolygon start_triangle;
    start_triangle << QPoint(x_pos, rect().bottom())
                   << QPoint(x_pos - 6, rect().bottom())
                   << QPoint(x_pos, rect().bottom() - 6);

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
    QPainter painter(this);

    painter.drawLine(x_pos,
                     rect().top(),
                     x_pos,
                     rect().bottom());

    // Draw start marker triangle
    QPolygon end_triangle;
    end_triangle << QPoint(x_pos, rect().bottom())
                 << QPoint(x_pos + 6, rect().bottom())
                 << QPoint(x_pos, rect().bottom() - 6);

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
    QRect handle_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    m_handle_width = handle_rect.width();

    QPainter painter(this);

    if (m_show_markers && m_markers_enabled) {
        // Draw rectangle for frames excluded by start marker
        if (m_start_marker >= minimum()) {
            int start_pos = position_for_value(minimum());
            int end_pos = position_for_value(m_start_marker) + handle_rect.width()/2 - 1;
            QRect rect(start_pos,
                       this->rect().top(),
                       end_pos - start_pos,
                       this->rect().height());
            painter.fillRect(rect, QBrush(QColor(255, 0, 0, 64)));
        }

        // Draw rectangle for frames excluded by end marker
        if (m_end_marker != -1 && m_end_marker <= maximum()) {
            int start_pos = position_for_value(m_end_marker) + handle_rect.width()/2;
            int end_pos = position_for_value(maximum()) + handle_rect.width() - 1;
            QRect rect(start_pos,
                       this->rect().top(),
                       end_pos - start_pos,
                       this->rect().height());
            painter.fillRect(rect, QBrush(QColor(255, 0, 0, 64)));
        }

        // Draw start marker
        if (m_start_marker >= minimum()) {
            int slider_pos = position_for_value(m_start_marker) + handle_rect.width()/2 - 1;
            draw_start_marker(slider_pos);
            m_start_marker_rect = QRect(slider_pos - 10, rect().top(), 10, rect().height());
        }

        // Draw end marker
        if (m_end_marker != -1 && m_end_marker <= maximum()) {
            // Draw start marker
            int slider_pos = position_for_value(m_end_marker) + handle_rect.width()/2;
            draw_end_marker(slider_pos);
            m_end_marker_rect = QRect(slider_pos, rect().top(), 10, rect().height());
        }
    }
}
