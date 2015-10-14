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
      m_current_direction(0)
{
    // Set up context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

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
        if (current_value < end_frame) {
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
        if (current_value > start_frame) {
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
            if (current_value < end_frame) {
                // Not at end of forward playback
                setValue(current_value + 1);
            } else {
                // At end of forward playback - play backwards
                m_current_direction = 1;
                setValue(current_value);
            }
        } else {
            // Currently playing reverse
            if (current_value > start_frame) {
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


void c_frame_slider::set_start_marker(int frame)
{
    if (frame <= minimum()) {
        m_start_marker = -1;
    } else if (frame <= m_end_marker) {
        m_start_marker = frame;
    }
}


void c_frame_slider::set_end_marker(int frame)
{
    if (frame >= maximum()) {
        m_end_marker = -1;
    } else if (frame >= m_start_marker) {
        m_end_marker = frame;
    }
}


void c_frame_slider::delete_all_markers()
{
    m_start_marker = -1;
    m_end_marker = -1;
}


int c_frame_slider::positionForValue(int val) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    opt.subControls = QStyle::SC_All;
    int available = opt.rect.width() - style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
    return QStyle::sliderPositionFromValue(opt.minimum, opt.maximum, val, available);
}


void c_frame_slider::ShowContextMenu(const QPoint& pos) // this is a slot
{
    // for most widgets
    QPoint globalPos = mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    QMenu markers_Menu;
    QAction *set_start_marker_Act = NULL;
    QAction *clear_start_marker_Act = NULL;
    QAction *set_end_marker_Act = NULL;
    QAction *clear_end_marker_Act = NULL;
    if (m_end_marker == -1 || value() <= m_end_marker) {
        set_start_marker_Act = markers_Menu.addAction(tr("Set Start Marker"));
    }

    if (m_start_marker != -1) {
        clear_start_marker_Act = markers_Menu.addAction(tr("Delete Start Marker"));
    }

    if (value() >= m_start_marker) {
        set_end_marker_Act = markers_Menu.addAction(tr("Set End Marker"));
    }

    if (m_end_marker != -1) {
        clear_end_marker_Act = markers_Menu.addAction(tr("Delete End Marker"));
    }

    QAction* selectedItem = markers_Menu.exec(globalPos);
    if (selectedItem != NULL) {
        if (selectedItem == set_start_marker_Act) {
            if (value() == minimum()) {
                m_start_marker = -1;
            } else {
                m_start_marker = value();
            }
        } else if (selectedItem == clear_start_marker_Act) {
            m_start_marker = -1;
        } else if (selectedItem == set_end_marker_Act) {
            if (value() == maximum()) {

            }
            m_end_marker = value();
        } else if (selectedItem == clear_end_marker_Act) {
            m_end_marker = -1;
        }
        else {
            // nothing was chosen
        }
    }
}


void c_frame_slider::paintEvent(QPaintEvent *ev) {
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


//    qDebug() << "groove_rect: " << groove_rect;
//    qDebug() << "handle_rect: " << handle_rect;
    QPainter painter(this);

    // Draw frames excluded from start rectangle
    if (m_start_marker > minimum()) {
        int start_pos = positionForValue(minimum());
        int end_pos = positionForValue(m_start_marker) - 1;
        QRect rect(start_pos,
                   groove_rect.top(),
                   end_pos - start_pos,
                   groove_rect.height());
        painter.fillRect(rect, QBrush(QColor(255, 0, 0, 64)));
    }

    // Draw frames excluded from end rectangle
    if (m_end_marker > m_start_marker && m_end_marker < maximum()) {
        int start_pos = positionForValue(m_end_marker) + handle_rect.width();
        int end_pos = positionForValue(maximum()) + handle_rect.width() - 1;
        QRect rect(start_pos,
                   groove_rect.top(),
                   end_pos - start_pos,
                   groove_rect.height());
        painter.fillRect(rect, QBrush(QColor(255, 0, 0, 64)));
    }

    // Draw start marker
    if (m_start_marker >= 0) {
        // Draw start marker
        int slider_pos = positionForValue(m_start_marker) - 1;
        painter.drawLine(groove_rect.left() + slider_pos,
                         groove_rect.top(),
                         groove_rect.left() + slider_pos,
                         groove_rect.bottom());
    }

    // Draw end marker
    if (m_end_marker >= 0) {
        // Draw start marker
        int slider_pos = positionForValue(m_end_marker) + handle_rect.width();
        painter.drawLine(groove_rect.left() + slider_pos,
                         groove_rect.top(),
                         groove_rect.left() + slider_pos,
                         groove_rect.bottom());
    }

    //QSlider::paintEvent(ev);
}
