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
#include <QFuture>
#include <cstdint>
#include <memory>


class c_image;


class c_histogram_thread : public QObject
{
    Q_OBJECT


public:
    // Constructor
    c_histogram_thread();

    // Destructor
    ~c_histogram_thread();

    // Method to initiate histogram data generation
    void generate_histogram(c_image *p_image, int frame_number);

    // Method to draw histogram on pixmap
    void draw_histogram_pixmap(QPixmap &histogram);

    // Methed to return frame number for the last histogram generated or the
    // histogram currently being generated
    int get_frame_number()
    {
        return m_frame_number;
    }

    // Method to return if the thread is ready to generate a new histogram
    bool is_running()
    {
        return m_is_running;
//        return generate_histogram_data_thread.isRunning();
    }
    
    
private:
    void calculate_pixmap_data();


signals:
    // Signal to pass the histogram pixmap back once it is generated
    void histogram_done();

public slots:

    
private slots:

    
private:
    bool m_is_running;
    int m_frame_number;
    int32_t m_width;
    int32_t m_height;
    bool m_colour;
    int32_t m_byte_depth;
    std::unique_ptr<uint8_t[]> mp_buffer;
    int32_t m_buffer_size;
    int32_t m_red_table[256];
    int32_t m_green_table[256];
    int32_t m_blue_table[256];
    QPixmap *mp_histogram_base_colour_Pixmap;
    QPixmap *mp_histogram_base_mono_Pixmap;
    QFuture<void> generate_histogram_data_thread;
};

#endif // HISTOGRAM_THREAD_H
