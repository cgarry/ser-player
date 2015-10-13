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
#include <QPainter>
#include <QSlider>
#include <QStyleOptionSlider>

#include "frame_slider.h"



c_frame_slider::c_frame_slider(QWidget *parent)
    : QSlider(parent)
{
    qDebug() << "c_frame_slider::c_frame_slider()";
}


int c_frame_slider::positionForValue(int val) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    opt.subControls = QStyle::SC_All;
    int available = opt.rect.width() - style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
    return QStyle::sliderPositionFromValue(opt.minimum, opt.maximum, val, available);
}


void c_frame_slider::paintEvent(QPaintEvent *ev) {
    QSlider::paintEvent(ev);
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
    if (tickPosition() != NoTicks) {
      opt.subControls |= QStyle::SC_SliderTickmarks;
    }

    QRect groove_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect handle_rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    qDebug() << "groove_rect: " << groove_rect;
    qDebug() << "handle_rect: " << handle_rect;
    QRect rect(groove_rect.left() + groove_rect.width() / 10,
               groove_rect.top(),
               8 * groove_rect.width() / 10,
               groove_rect.height());

    QPainter painter(this);

    QBrush brush = QBrush(Qt::green);
    brush.setStyle(Qt::Dense5Pattern);
    //brush.setStyle(Qt::LinearGradientPattern);

    painter.fillRect(rect, brush);

    for (int x = this->minimum(); x <= this->maximum(); x++) {
        int slider_pos = positionForValue(x) + handle_rect.width() / 2;
                //QStyle::sliderPositionFromValue(this->minimum(), this->maximum(), x, groove_rect.width());
        qDebug() << "slider_pos: " << x << " = " << slider_pos;
        painter.drawLine(groove_rect.left() + slider_pos,
                         groove_rect.top(),
                         groove_rect.left() + slider_pos,
                         groove_rect.bottom());
    }


    QSlider::paintEvent(ev);

}
