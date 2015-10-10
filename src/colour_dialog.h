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


#ifndef COLOUR_DIALOG_H
#define COLOUR_DIALOG_H

#include <QDialog>

class QDoubleSpinBox;
class QLabel;
class QSlider;


class c_colour_dialog : public QDialog
{
    Q_OBJECT

public:
    c_colour_dialog(QWidget *parent = 0);
//    ~c_colour_dialog();

signals:
    void colour_saturation_changed(double saturation);
    void colour_balance_changed(int red, int green, int blue);
    
private slots:
    void reset_dialog();
    void red_balanced_changed_slot(int balance);
    void green_balanced_changed_slot(int balance);
    void blue_balanced_changed_slot(int balance);
    
private:
    // Private methods
    void helper_method();
    
    // Widgets
    QDoubleSpinBox *mp_colsat_DSpinbox;
    QSlider *mp_red_balance_Slider;
    QSlider *mp_green_balance_Slider;
    QSlider *mp_blue_balance_Slider;
    QLabel *mp_red_balance_Label;
    QLabel *mp_green_balance_Label;
    QLabel *mp_blue_balance_Label;
    
};

#endif // COLOUR_DIALOG_H
