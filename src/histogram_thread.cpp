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
#include <QPixmap>
#include <QThread>
#include <cmath>

#include "histogram_thread.h"
#include "image.h"


c_histogram_thread::c_histogram_thread()
    : m_histogram_done(true),
      m_run_count(0)
{

}


void generate_histogram();



void c_histogram_thread::generate_histogram(c_image *p_image)
{
    m_histogram_done = false;

    // Copy image details
    m_width = p_image->get_width();
    m_height = p_image->get_height();
    m_colour = p_image->get_colour();
    m_byte_depth = p_image->get_byte_depth();
    m_buffer_size = m_width * m_height * m_byte_depth;
    m_buffer_size = (m_colour) ? m_buffer_size * 3 : m_buffer_size;
    mp_buffer = new uint8_t[m_buffer_size];

    // Copy image data to local buffer
    memcpy(mp_buffer, p_image->get_p_buffer(), m_buffer_size);

    start();
}


void c_histogram_thread::run()
{
    const int HISTO_WIDTH = 256 + 5;
    const int HISTO_HEIGHT_MONO = 150;
    const int HISTO_HEIGHT_COLOUR = 300;
    // Generate histogram
    int32_t max_value = 0;
    if (m_colour) {
        //
        // Generate monochrome histogram
        //
        memset(m_blue_table, 0, 256 * sizeof(int32_t));
        memset(m_green_table, 0, 256 * sizeof(int32_t));
        memset(m_red_table, 0, 256 * sizeof(int32_t));

        // Create histogram table
        uint8_t *p_data = mp_buffer;
        for (int i = 0; i < m_width * m_height; i++) {
            m_blue_table[*p_data++]++;
            m_green_table[*p_data++]++;
            m_red_table[*p_data++]++;
        }

        delete [] mp_buffer;  // Free image buffer

        // Find max value in histogram table
        for (int i = 0; i < 256; i++) {
            max_value = (m_blue_table[i] > max_value) ? m_blue_table[i] : max_value;
            max_value = (m_green_table[i] > max_value) ? m_green_table[i] : max_value;
            max_value = (m_red_table[i] > max_value) ? m_red_table[i] : max_value;
        }

        // Convert the histogram table into log10 normalised values
        double max_value_log10 = log((double)max_value) + 1.0;
        for (int i = 0; i < 256; i++) {
            if (m_blue_table[i] > 0) {
                m_blue_table[i] = (int32_t)(((log((double)m_blue_table[i]) + 1.0) / max_value_log10) * (HISTO_HEIGHT_COLOUR/3-13));
            } else {
                m_blue_table[i] = 0;
            }

            if (m_green_table[i] > 0) {
                m_green_table[i] = (int32_t)(((log((double)m_green_table[i]) + 1.0) / max_value_log10) * (HISTO_HEIGHT_COLOUR/3-13));
            } else {
                m_green_table[i] = 0;
            }

            if (m_red_table[i] > 0) {
                m_red_table[i] = (int32_t)(((log((double)m_red_table[i]) + 1.0) / max_value_log10) * (HISTO_HEIGHT_COLOUR/3-13));
            } else {
                m_red_table[i] = 0;
            }
        }

        // Create an instance of QPixmap to render the histogram image on
        QPixmap *p_histogram_Pixmap = new QPixmap(HISTO_WIDTH, HISTO_HEIGHT_COLOUR);
        p_histogram_Pixmap->fill(QColor(255-31, 255-31, 255, 255));  // B

        // Create an instance of QPainter to draw on the QPixmap instance
        QPainter *p_paint = new QPainter(p_histogram_Pixmap);
        p_paint->setPen(*(new QColor(255, 255-31, 255-31, 255)));
        for (int y = 0; y < HISTO_HEIGHT_COLOUR/3; y++) {
            p_paint->drawLine(0, y, HISTO_WIDTH, y);
        }

        p_paint->setPen(*(new QColor(255-31, 255, 255-31, 255)));
        for (int y = HISTO_HEIGHT_COLOUR/3; y < 2*HISTO_HEIGHT_COLOUR/3; y++) {
            p_paint->drawLine(0, y, HISTO_WIDTH, y);
        }

        // Draw vertical lines every 25 pixels and label them
        for (int x = 0; x < 256; x += 25) {
            p_paint->setPen(*(new QColor(196, 196, 196, 255)));
            p_paint->drawLine(x, HISTO_HEIGHT_COLOUR, x, 0);

            p_paint->setPen(*(new QColor(0, 0, 0, 255)));
            if (x == 0) {
                p_paint->drawText(QRect(x, HISTO_HEIGHT_COLOUR-11, 20, 10), Qt::AlignLeft | Qt::AlignVCenter, QString::number(x));
                p_paint->drawText(QRect(x, (2*HISTO_HEIGHT_COLOUR)/3-11, 20, 10), Qt::AlignLeft | Qt::AlignVCenter, QString::number(x));
                p_paint->drawText(QRect(x, HISTO_HEIGHT_COLOUR/3-11, 20, 10), Qt::AlignLeft | Qt::AlignVCenter, QString::number(x));
            } else {
                p_paint->drawText(QRect(x-10, HISTO_HEIGHT_COLOUR-11, 20, 10), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(x));
                p_paint->drawText(QRect(x-10, (2*HISTO_HEIGHT_COLOUR)/3-11, 20, 10), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(x));
                p_paint->drawText(QRect(x-10, HISTO_HEIGHT_COLOUR/3-11, 20, 10), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(x));
            }
        }

        // Draw histogram graph except the last column
        p_paint->setPen(*(new QColor(QColor(64, 64, 64, 192))));
        for (int x = 0; x < 255; x++) {
            p_paint->drawLine(x, HISTO_HEIGHT_COLOUR-12, x, HISTO_HEIGHT_COLOUR-12-m_blue_table[x]);
            p_paint->drawLine(x, (2*HISTO_HEIGHT_COLOUR)/3-12, x, (2*HISTO_HEIGHT_COLOUR)/3-12-m_green_table[x]);
            p_paint->drawLine(x, HISTO_HEIGHT_COLOUR/3-12, x, HISTO_HEIGHT_COLOUR/3-12-m_red_table[x]);
        }

        // Draw final column of histogram graph in red to indicate potential clipping
        p_paint->setPen(*(new QColor(QColor(255, 0, 0, 255))));
        p_paint->drawLine(255, HISTO_HEIGHT_COLOUR-12, 255, HISTO_HEIGHT_COLOUR-12-m_blue_table[255]);
        p_paint->drawLine(255, (2*HISTO_HEIGHT_COLOUR)/3-12, 255, (2*HISTO_HEIGHT_COLOUR)/3-12-m_green_table[255]);
        p_paint->drawLine(255, HISTO_HEIGHT_COLOUR/3-12, 255, HISTO_HEIGHT_COLOUR/3-12-m_red_table[255]);

        emit histogram_done(*p_histogram_Pixmap);  // Send histogram image out

        delete p_paint;
        delete p_histogram_Pixmap;

    } else {
        //
        // Generate monochrome histogram
        //
        memset(m_blue_table, 0, 256 * sizeof(int32_t));

        // Create histogram table
        uint8_t *p_data = mp_buffer;
        for (int i = 0; i < m_width * m_height; i++) {
            m_blue_table[*p_data++]++;
        }

        delete [] mp_buffer;  // Free image buffer

        // Find max value in histogram table
        for (int i = 0; i < 256; i++) {
            max_value = (m_blue_table[i] > max_value) ? m_blue_table[i] : max_value;
        }

        // Convert the histogram table into log10 normalised values
        double max_value_log10 = log((double)max_value) + 1.0;
        for (int i = 0; i < 256; i++) {
            if (m_blue_table[i] > 0) {
                m_blue_table[i] = (int32_t)(((log((double)m_blue_table[i]) + 1.0) / max_value_log10) * (HISTO_HEIGHT_MONO-13));
            } else {
                m_blue_table[i] = 0;
            }
        }

        // Create an instance of QPixmap to render the histogram image on
        QPixmap *p_histogram_Pixmap = new QPixmap(HISTO_WIDTH, HISTO_HEIGHT_MONO);
        p_histogram_Pixmap->fill(QColor(255-15, 255-15, 255-15, 255));  // Mono

        // Create an instance of QPainter to draw on the QPixmap instance
        QPainter *p_paint = new QPainter(p_histogram_Pixmap);

        // Draw vertical lines every 25 pixels and label them
        for (int x = 0; x < 256; x += 25) {
            p_paint->setPen(*(new QColor(196, 196, 196, 255)));
            p_paint->drawLine(x, HISTO_HEIGHT_MONO, x, 0);

            p_paint->setPen(*(new QColor(0, 0, 0, 255)));
            if (x == 0) {
                p_paint->drawText(QRect(x, HISTO_HEIGHT_MONO-11, 20, 10), Qt::AlignLeft | Qt::AlignVCenter, QString::number(x));
            } else {
                p_paint->drawText(QRect(x-10, HISTO_HEIGHT_MONO-11, 20, 10), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(x));
            }
        }

        // Draw histogram graph except the last column
        p_paint->setPen(*(new QColor(QColor(64, 64, 64, 192))));
        for (int x = 0; x < 255; x++) {
            p_paint->drawLine(x, HISTO_HEIGHT_MONO-12, x, HISTO_HEIGHT_MONO-12-m_blue_table[x]);
        }

        // Draw final column of histogram graph in red to indicate potential clipping
        p_paint->setPen(*(new QColor(QColor(255, 0, 0, 255))));
        p_paint->drawLine(255, HISTO_HEIGHT_MONO-12, 255, HISTO_HEIGHT_MONO-12-m_blue_table[255]);

        emit histogram_done(*p_histogram_Pixmap);  // Send histogram image out

        delete p_paint;
        delete p_histogram_Pixmap;
    }

    m_run_count++;
    m_histogram_done = true;
}
