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

class c_image_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit c_image_Widget(QWidget *parent = 0);
    const QPixmap* pixmap() const;
    int get_zoom_level();
    QSize get_image_size();

signals:
    void double_click_signal();

public slots:
    void setPixmap(const QPixmap&);

protected:
    void paintEvent(QPaintEvent *);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    int heightForWidth(int width) const;
    int widthForHeight(int height) const;
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);

private:
    QPixmap m_image_Pixmap;
    QSize m_image_size;
    QSize m_current_Size;
    int m_zoom_level;
};

#endif // IMAGE_WIDGET_H
