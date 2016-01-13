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


#ifndef PROCESSING_OPTIONS_H
#define PROCESSING_OPTIONS_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QSlider;
class QSpinBox;
class c_icon_groupbox;


class c_processing_options_dialog : public QDialog
{
    Q_OBJECT

public:
    c_processing_options_dialog(QWidget *parent = 0);
    void set_colour_balance(double red, double green, double blue);
    void set_data_has_bayer_pattern(bool bayer_pattern);
    void set_data_is_colour(bool colour);
    bool get_debayer_enable();
    int get_debayer_pattern();


signals:
    void debayer_enable(bool);
    void debayer_pattern_specified(bool specified, int colour_id);
    void invert_frames(bool);
    void gain_changed(double gain);
    void gamma_changed(double gamma);
    void monochrome_conversion_changed(bool enabled, int selection);
    void colour_saturation_changed(double saturation);
    void colour_balance_changed(double red, double green, double blue);
    void estimate_colour_balance();
    void colour_align_changed(int red_align_x, int red_align_y, int blue_align_x, int blus_align_y);


public slots:
    void reset_gain_and_gamma_slot();
    void reset_colour_saturation_slot();
    void reset_colour_balance_slot();
    void reset_colour_align_slot();
    void reset_all_slot();

    
private slots:
    void debayer_controls_changed_slot();
    void gain_slider_changed_slot(int gain);
    void gain_spinbox_changed_slot(double gain);
    void gamma_slider_changed_slot(int gain);
    void gamma_spinbox_changed_slot(double gain);
    void monochrome_conversion_changed_slot();
    void colour_saturation_slider_changed_slot(int sat);
    void colour_saturation_spinbox_changed_slot();
    void red_balance_slider_changed_slot(int balance);
    void green_balance_slider_changed_slot(int balance);
    void blue_balance_slider_changed_slot(int balance);
    void red_balance_spinbox_changed_slot();
    void green_balance_spinbox_changed_slot();
    void blue_balance_spinbox_changed_slot();
    void colour_align_changed_slot();

private:
    void enable_and_disable_controls();

    
private:
    //
    // Widgets
    //
    c_icon_groupbox *mp_debayer_GroupBox;
    QComboBox *mp_bayer_pattern_Combobox;
    QCheckBox *mp_invert_CheckBox;
    // Gain and Gamma
    QSlider *mp_gain_Slider;
    QDoubleSpinBox *mp_gain_DSpinbox;
    QSlider *mp_gamma_Slider;
    QDoubleSpinBox *mp_gamma_DSpinbox;
    // Colour align
    c_icon_groupbox *mp_colour_align_GroupBox;
    // Monochrome Conversion
    QComboBox *mp_monochrome_conversion_Combobox;
    c_icon_groupbox *mp_monochrome_conversion_GroupBox;
    // Colour Saturation
    c_icon_groupbox *mp_colour_saturation_GroupBox;
    QSlider *mp_colsat_Slider;
    QDoubleSpinBox *mp_colsat_DSpinbox;
    // Colour balance
    c_icon_groupbox *mp_colour_balance_GroupBox;
    QSlider *mp_red_balance_Slider;
    QSlider *mp_green_balance_Slider;
    QSlider *mp_blue_balance_Slider;
    QSpinBox *mp_red_balance_SpinBox;
    QSpinBox *mp_green_balance_SpinBox;
    QSpinBox *mp_blue_balance_SpinBox;
    // Colour Channel Align
    QSpinBox *mp_blue_x_Spinbox;
    QSpinBox *mp_blue_y_Spinbox;
    QSpinBox *mp_red_x_Spinbox;
    QSpinBox *mp_red_y_Spinbox;

    // Other
    bool m_data_has_bayer_pattern;
    bool m_data_is_colour;
};

#endif // PROCESSING_OPTIONS_H
