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


#ifndef HISTOGRAM_THREAD_H
#define HISTOGRAM_THREAD_H

#include <QDialog>
#include <QThread>
#include <cstdint>


class c_image;


class c_histogram_thread : public QThread
{
    Q_OBJECT


public:
    c_histogram_thread();
    void generate_histogram(c_image *p_image);
    bool is_histogram_done()
    {
        return m_histogram_done;
    }
    
    
protected:
    void run();


signals:
    void histogram_done(QPixmap histogram);

public slots:

    
private slots:

    
private:
    bool m_histogram_done;
    int m_run_count;
    int32_t m_width;
    int32_t m_height;
    bool m_colour;
    int32_t m_byte_depth;
    uint8_t *mp_buffer;
    int32_t m_buffer_size;
    int32_t m_red_table[256];
    int32_t m_green_table[256];
    int32_t m_blue_table[256];
};

#endif // HISTOGRAM_THREAD_H
