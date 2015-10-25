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


#ifndef GAIN_AND_GAMMA_H
#define GAIN_AND_GAMMA_H

#include <QDialog>

class QDoubleSpinBox;
class QSlider;


class c_gain_and_gamma_dialog : public QDialog
{
    Q_OBJECT

public:
    c_gain_and_gamma_dialog(QWidget *parent = 0);


signals:
    void gain_changed(double gain);
    void gamma_changed(double gamma);


public slots:
    void reset_gain_slot();
    void reset_gamma_slot();

    
private slots:
    void gain_slider_changed_slot(int gain);
    void gain_spinbox_changed_slot(double gain);
    void gamma_slider_changed_slot(int gain);
    void gamma_spinbox_changed_slot(double gain);

    
private:
    // Widgets
    QSlider *mp_gain_Slider;
    QDoubleSpinBox *mp_gain_DSpinbox;
    QSlider *mp_gamma_Slider;
    QDoubleSpinBox *mp_gamma_DSpinbox;
};

#endif // GAIN_AND_GAMMA_H
