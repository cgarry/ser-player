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


#ifndef HEADER_DETAILS_DIALOG_H
#define HEADER_DETAILS_DIALOG_H

#include <QDialog>
#include <cstdint>

class QTextEdit;


class c_header_details_dialog : public QDialog
{
    Q_OBJECT

public:
    c_header_details_dialog(QWidget *parent = 0);

    void set_details(QString filename,
                     int filesize,
                     QString file_id,
                     int lu_id,
                     int colour_id,
                     int little_endian,
                     int image_width,
                     int image_height,
                     int pixel_depth,
                     int frame_count,
                     QString observer,
                     QString instrument,
                     QString telescope,
                     uint64_t date_time,
                     uint64_t date_time_utc,
                     QString timestamp_info);


signals:


public slots:

    
private slots:

    
private:
    // Private methods
    
    // Widgets
    QTextEdit *mp_header_details_Tedit;
};

#endif // HEADER_DETAILS_DIALOG_H
