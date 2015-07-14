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
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>


c_image_Widget::c_image_Widget(QWidget *parent) :
    QWidget(parent),
    m_zoom_level(100)
{
    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    p.setWidthForHeight(true);
    setSizePolicy(p);
    m_image_size  = QSize(1, 1);
    m_current_Size = QSize(1, 1);
}


int c_image_Widget::get_zoom_level()
{
    return m_zoom_level;
}


QSize c_image_Widget::get_image_size()
{
    return m_image_size;
}


void c_image_Widget::mouseDoubleClickEvent(QMouseEvent * event)
{
    (void)event;  // Remove unused parameter warning
    emit double_click_signal();
}


void c_image_Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_image_Pixmap.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QSize pixSize = m_image_Pixmap.size();
    pixSize.scale(event->rect().size(), Qt::KeepAspectRatio);

    m_zoom_level = (pixSize.width() * 100) / m_image_Pixmap.size().width();

    QPixmap scaled_Pixmap = m_image_Pixmap.scaled(pixSize,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);

    // Calculate position to draw image in middle of widget
    int x = (event->rect().size().width() - scaled_Pixmap.size().width()) / 2;
    int y = (event->rect().size().height() - scaled_Pixmap.size().height()) / 2;

    painter.drawPixmap(QPoint(x, y), scaled_Pixmap);
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

