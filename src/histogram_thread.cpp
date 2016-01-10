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
#include <QtConcurrent>
#include <QPainter>
#include <QPixmap>
#include <QThread>
#include <cmath>

#include "histogram_thread.h"
#include "image.h"


c_histogram_thread::c_histogram_thread()
    : m_is_running(false)
{
    // Initialise histogram base images - graphs are painted on these images
    mp_histogram_base_colour_Pixmap = new QPixmap(":/res/resources/histogram_colour.png");
    mp_histogram_base_mono_Pixmap = new QPixmap(":/res/resources/histogram_mono.png");
}


c_histogram_thread::~c_histogram_thread()
{
    // Free up histogram base images
    delete mp_histogram_base_colour_Pixmap;
    delete mp_histogram_base_mono_Pixmap;
}



void c_histogram_thread::generate_histogram(c_image *p_image, int frame_number)
{
    if (!m_is_running) {
        m_is_running = true;

        // Copy image details
        m_frame_number = frame_number;
        m_width = p_image->get_width();
        m_height = p_image->get_height();
        m_colour = p_image->get_colour();
        m_byte_depth = p_image->get_byte_depth();
        m_buffer_size = m_width * m_height * m_byte_depth;
        m_buffer_size = (m_colour) ? m_buffer_size * 3 : m_buffer_size;
        mp_buffer.reset(new uint8_t[m_buffer_size]);

        // Copy image data to local buffer
        memcpy(mp_buffer.get(), p_image->get_p_buffer(), m_buffer_size);


        generate_histogram_data_thread = QtConcurrent::run(this, &c_histogram_thread::calculate_pixmap_data);
    }
}


void c_histogram_thread::calculate_pixmap_data()
{
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
        if (m_byte_depth == 1) {
            uint8_t *p_data = mp_buffer.get();
            for (int i = 0; i < m_width * m_height; i++) {
                m_blue_table[*p_data++]++;
                m_green_table[*p_data++]++;
                m_red_table[*p_data++]++;
            }
        } else {  // m_byte_depth == 2
            uint16_t *p_data = (uint16_t *)mp_buffer.get();
            for (int i = 0; i < m_width * m_height; i++) {
                m_blue_table[(*p_data++) >> 8]++;
                m_green_table[(*p_data++) >> 8]++;
                m_red_table[(*p_data++) >> 8]++;
            }
        }

        mp_buffer.reset(nullptr);  // Free image buffer

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
    } else {
        //
        // Generate monochrome histogram
        //
        memset(m_blue_table, 0, 256 * sizeof(int32_t));

        // Create histogram table
        if (m_byte_depth == 1) {
            uint8_t *p_data = mp_buffer.get();
            for (int i = 0; i < m_width * m_height; i++) {
                m_blue_table[*p_data++]++;
            }
        } else {  // m_byte_depth == 2
            uint16_t *p_data = (uint16_t *)mp_buffer.get();
            for (int i = 0; i < m_width * m_height; i++) {
                m_blue_table[(*p_data++) >> 8]++;
            }
        }

        mp_buffer.reset(nullptr);  // Free image buffer

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
    }

    // A a short delay to limit the rate that histograms are generated and displayed
    // This creates the same user experience but gives the processor an easier time
    QThread::msleep(50);
    emit histogram_done();  // Signal that the processing is done
}


// Method to draw histogram on pixmap
void c_histogram_thread::draw_histogram_pixmap(QPixmap &histogram_Pixmap)
{
    const int HISTO_HEIGHT_MONO = 150;
    const int HISTO_HEIGHT_COLOUR = 300;

    if (m_colour) {
        // Create an instance of QPixmap with base histogram image on it to render the histogram graph on
        histogram_Pixmap = *mp_histogram_base_colour_Pixmap;

        // Create an instance of QPainter to draw on the QPixmap instance
        QPainter histo_paint(&histogram_Pixmap);

        // Draw histogram graph except the last column
        for (int x = 0; x < 255; x++) {
            histo_paint.setPen(QColor(2*x/3, 2*x/3, 2*x/3, 220));
            histo_paint.drawLine(x, HISTO_HEIGHT_COLOUR-12, x, HISTO_HEIGHT_COLOUR-12-m_blue_table[x]);
            histo_paint.drawLine(x, (2*HISTO_HEIGHT_COLOUR)/3-12, x, (2*HISTO_HEIGHT_COLOUR)/3-12-m_green_table[x]);
            histo_paint.drawLine(x, HISTO_HEIGHT_COLOUR/3-12, x, HISTO_HEIGHT_COLOUR/3-12-m_red_table[x]);
        }

        // Draw final column of histogram graph in red to indicate potential clipping
        histo_paint.setPen(QColor(QColor(255, 0, 0, 255)));
        histo_paint.drawLine(255, HISTO_HEIGHT_COLOUR-12, 255, HISTO_HEIGHT_COLOUR-12-m_blue_table[255]);
        histo_paint.drawLine(255, (2*HISTO_HEIGHT_COLOUR)/3-12, 255, (2*HISTO_HEIGHT_COLOUR)/3-12-m_green_table[255]);
        histo_paint.drawLine(255, HISTO_HEIGHT_COLOUR/3-12, 255, HISTO_HEIGHT_COLOUR/3-12-m_red_table[255]);
    } else {
        // Monochrome
        // Create an instance of QPixmap with base histogram image on it to render the histogram graph on
        histogram_Pixmap = *mp_histogram_base_mono_Pixmap;

        // Create an instance of QPainter to draw on the QPixmap instance
        QPainter paint(&histogram_Pixmap);

        // Draw histogram graph except the last column
        for (int x = 0; x < 255; x++) {
            paint.setPen(QColor(2*x/3, 2*x/3, 2*x/3, 220));
            paint.drawLine(x, HISTO_HEIGHT_MONO-12, x, HISTO_HEIGHT_MONO-12-m_blue_table[x]);
        }

        // Debug - Display frame number on histogram
        //paint.drawText(255-40, 0, 40, 10, Qt::AlignVCenter | Qt::AlignRight, QString::number(m_frame_number));
        // Debug - End

        // Draw final column of histogram graph in red to indicate potential clipping
        paint.setPen(QColor(QColor(255, 0, 0, 255)));
        paint.drawLine(255, HISTO_HEIGHT_MONO-12, 255, HISTO_HEIGHT_MONO-12-m_blue_table[255]);
    }

    m_is_running = false;  // Signal that the next conversion can start
}
