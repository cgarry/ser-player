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


#ifndef HISTOGRAM_DIALOG_H
#define HISTOGRAM_DIALOG_H

#include <QDialog>

class QLabel;


class c_histogram_dialog : public QDialog
{
    Q_OBJECT

public:
    c_histogram_dialog(QWidget *parent = 0);
    void set_pixmap(QPixmap histogram);


signals:


public slots:

    
private slots:

    
private:    
    // Widgets
    QLabel *mp_histogram_Label;
};

#endif // HISTOGRAM_DIALOG_H
