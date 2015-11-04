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
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

#include "processing_options_dialog.h"
#include "persistent_data.h"
#include "icon_groupbox.h"


c_processing_options_dialog::c_processing_options_dialog(QWidget *parent)
    : QDialog(parent),
      m_data_has_bayer_pattern(false),
      m_data_is_colour(false)
{
    setWindowTitle(tr("Processing"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);


    // Debayer checkbox
    mp_debayer_CheckBox = new QCheckBox(tr("Enable Debayering"));
    mp_debayer_CheckBox->setChecked(true);
    connect(mp_debayer_CheckBox, SIGNAL(toggled(bool)), this, SLOT(debayer_enable_slot(bool)));

    QHBoxLayout *debayer_HLayout1 = new QHBoxLayout;
    debayer_HLayout1->setMargin(5);
    debayer_HLayout1->addWidget(mp_debayer_CheckBox);

    mp_debayer_GroupBox = new c_icon_groupbox(this);
    mp_debayer_GroupBox->setTitle(tr("Colour Debayer"));
    mp_debayer_GroupBox->set_icon(":/res/resources/debayer_icon.png");
    mp_debayer_GroupBox->setLayout(debayer_HLayout1);


    // Invert checkbox
    mp_invert_CheckBox = new QCheckBox(tr("Invert Frames"));
    mp_invert_CheckBox->setChecked(false);
    connect(mp_invert_CheckBox, SIGNAL(toggled(bool)), this, SIGNAL(invert_frames(bool)));

    QHBoxLayout *invert_HLayout1 = new QHBoxLayout;
    invert_HLayout1->setMargin(5);
    invert_HLayout1->addWidget(mp_invert_CheckBox);

    c_icon_groupbox *invert_GroupBox = new c_icon_groupbox;
    invert_GroupBox->setTitle(tr("Frame Inversion"));
    invert_GroupBox->set_icon(":/res/resources/invert_icon.png");
    invert_GroupBox->setLayout(invert_HLayout1);


    // Gain widgets
    mp_gain_Slider = new QSlider(Qt::Horizontal);
    mp_gain_Slider->setRange(0, 300);
    mp_gain_Slider->setValue(100);
    mp_gain_Slider->setMinimumWidth(mp_gain_Slider->sizeHint().width() * 2);
    connect(mp_gain_Slider, SIGNAL(sliderMoved(int)), this, SLOT(gain_slider_changed_slot(int)));
    mp_gain_DSpinbox = new QDoubleSpinBox;
    mp_gain_DSpinbox->setRange(0.0, 3.0);
    mp_gain_DSpinbox->setSingleStep(0.01);
    mp_gain_DSpinbox->setValue(1.0);
    connect(mp_gain_DSpinbox, SIGNAL(valueChanged(double)), this, SLOT(gain_spinbox_changed_slot(double)));

    // Gamma widgets
    mp_gamma_Slider = new QSlider(Qt::Horizontal);
    mp_gamma_Slider->setRange(10, 300);
    mp_gamma_Slider->setValue(100);
    mp_gamma_Slider->setMinimumWidth(mp_gamma_Slider->sizeHint().width() * 2);
    connect(mp_gamma_Slider, SIGNAL(sliderMoved(int)), this, SLOT(gamma_slider_changed_slot(int)));
    mp_gamma_DSpinbox = new QDoubleSpinBox;
    mp_gamma_DSpinbox->setRange(0.1, 3.0);
    mp_gamma_DSpinbox->setSingleStep(0.01);
    mp_gamma_DSpinbox->setValue(1.0);
    connect(mp_gamma_DSpinbox, SIGNAL(valueChanged(double)), this, SLOT(gamma_spinbox_changed_slot(double)));

    QGridLayout *gain_and_gamma_GLayout = new QGridLayout;
    gain_and_gamma_GLayout->setVerticalSpacing(10);
    gain_and_gamma_GLayout->setHorizontalSpacing(10);
    gain_and_gamma_GLayout->addWidget(new QLabel(tr("Gain")), 0, 0);
    gain_and_gamma_GLayout->addWidget(mp_gain_Slider, 0, 1);
    gain_and_gamma_GLayout->addWidget(mp_gain_DSpinbox, 0, 2);
    gain_and_gamma_GLayout->addWidget(new QLabel(tr("Gamma")), 1, 0);
    gain_and_gamma_GLayout->addWidget(mp_gamma_Slider, 1, 1);
    gain_and_gamma_GLayout->addWidget(mp_gamma_DSpinbox, 1, 2);

    QPushButton *reset_gain_and_gamma_Button = new QPushButton(tr("Reset"));
    reset_gain_and_gamma_Button->setAutoDefault(false);
    connect(reset_gain_and_gamma_Button, SIGNAL(clicked()), this, SLOT(reset_gain_and_gamma_slot()));

    QHBoxLayout *gamma_HLayout2 = new QHBoxLayout;
    gamma_HLayout2->addWidget(reset_gain_and_gamma_Button);
    gamma_HLayout2->addStretch();

    QVBoxLayout *gain_and_gamma_Vlayout = new QVBoxLayout;
    gain_and_gamma_Vlayout->setMargin(5);
    gain_and_gamma_Vlayout->setSpacing(10);
    gain_and_gamma_Vlayout->addLayout(gain_and_gamma_GLayout);
    gain_and_gamma_Vlayout->addLayout(gamma_HLayout2);

    c_icon_groupbox *gain_and_gammaGroupBox = new c_icon_groupbox;
    gain_and_gammaGroupBox->setTitle(tr("Gain and Gamma"));
    gain_and_gammaGroupBox->set_icon(":/res/resources/gain_icon.png");
    gain_and_gammaGroupBox->setLayout(gain_and_gamma_Vlayout);


    //
    // Monochrome Conversion
    //
    QLabel *monochrome_conversion_Label = new QLabel(tr("Convert Colour To Monochrome Using: "));

    mp_monochrome_conversion_Combobox = new QComboBox;
    mp_monochrome_conversion_Combobox->addItem(tr("RG & B Channels"));
    mp_monochrome_conversion_Combobox->addItem(tr("R Channel Only"));
    mp_monochrome_conversion_Combobox->addItem(tr("G Channel Only"));
    mp_monochrome_conversion_Combobox->addItem(tr("B Channel Only"));
    mp_monochrome_conversion_Combobox->addItem(tr("R & G Channels"));
    mp_monochrome_conversion_Combobox->addItem(tr("R & B Channels"));
    mp_monochrome_conversion_Combobox->addItem(tr("G & B Channels"));
    connect(mp_monochrome_conversion_Combobox, SIGNAL(activated(int)), this, SLOT(monochrome_conversion_changed_slot()));

    QHBoxLayout *monochrome_conversion_GroupBox_Hlayout = new QHBoxLayout;
    monochrome_conversion_GroupBox_Hlayout->setMargin(5);
    monochrome_conversion_GroupBox_Hlayout->addWidget(monochrome_conversion_Label);
    monochrome_conversion_GroupBox_Hlayout->addWidget(mp_monochrome_conversion_Combobox);


    mp_monochrome_conversion_GroupBox = new c_icon_groupbox;
    mp_monochrome_conversion_GroupBox->setTitle(tr("Monochrome Conversion"));
    mp_monochrome_conversion_GroupBox->set_icon(":/res/resources/monochrome_conversion_icon.png");
    mp_monochrome_conversion_GroupBox->setCheckable(true);
    mp_monochrome_conversion_GroupBox->setChecked(false);
    mp_monochrome_conversion_GroupBox->setLayout(monochrome_conversion_GroupBox_Hlayout);
    connect(mp_monochrome_conversion_GroupBox, SIGNAL(toggled(bool)), this, SLOT(monochrome_conversion_changed_slot()));
    connect(mp_monochrome_conversion_GroupBox, SIGNAL(toggled(bool)), this, SLOT(colour_saturation_spinbox_changed_slot()));
    connect(mp_monochrome_conversion_GroupBox, SIGNAL(toggled(bool)), this, SLOT(red_balance_spinbox_changed_slot()));


    //
    // Colour Saturation
    //
    mp_colsat_Slider = new QSlider(Qt::Horizontal);
    mp_colsat_Slider->setRange(0, 1500);
    mp_colsat_Slider->setValue(100);
    mp_colsat_Slider->setMinimumWidth(mp_colsat_Slider->sizeHint().width() * 2);
    connect(mp_colsat_Slider, SIGNAL(sliderMoved(int)), this, SLOT(colour_saturation_slider_changed_slot(int)));
    mp_colsat_DSpinbox = new QDoubleSpinBox;
    mp_colsat_DSpinbox->setRange(0.0, 15.0);
    mp_colsat_DSpinbox->setSingleStep(0.01);
    mp_colsat_DSpinbox->setValue(1.0);

    connect(mp_colsat_DSpinbox, SIGNAL(valueChanged(double)), this, SLOT(colour_saturation_spinbox_changed_slot()));

    QHBoxLayout *colsat_hlayout1 = new QHBoxLayout;
    colsat_hlayout1->addWidget(new QLabel(tr("Saturation")));
    colsat_hlayout1->addWidget(mp_colsat_Slider);
    colsat_hlayout1->addWidget(mp_colsat_DSpinbox);

    QPushButton *reset_colour_saturation_button = new QPushButton(tr("Reset"));
    reset_colour_saturation_button->setAutoDefault(false);
    connect(reset_colour_saturation_button, SIGNAL(clicked()), this, SLOT(reset_colour_saturation_slot()));
    QHBoxLayout *colsat_hlayout2 = new QHBoxLayout;
    colsat_hlayout2->addWidget(reset_colour_saturation_button);
    colsat_hlayout2->addStretch();
    QVBoxLayout *colsat_vlayout = new QVBoxLayout;
    colsat_vlayout->setMargin(5);
    colsat_vlayout->setSpacing(10);
    colsat_vlayout->addLayout(colsat_hlayout1);
    colsat_vlayout->addLayout(colsat_hlayout2);

    mp_colour_saturation_GroupBox = new c_icon_groupbox;
    mp_colour_saturation_GroupBox->setTitle(tr("Colour Saturation"));
    mp_colour_saturation_GroupBox->set_icon(":/res/resources/saturation_icon.png");
    mp_colour_saturation_GroupBox->setLayout(colsat_vlayout);


    //
    // Colour balance
    //
    mp_red_balance_Slider = new QSlider(Qt::Horizontal);
    mp_red_balance_Slider->setRange(-100, 100);
    mp_red_balance_Slider->setValue(0);
    mp_red_balance_Slider->setMinimumWidth(mp_red_balance_Slider->sizeHint().width() * 2);
    connect(mp_red_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(red_balance_slider_changed_slot(int)));
    mp_red_balance_SpinBox = new QSpinBox;
    mp_red_balance_SpinBox->setRange(-100, 100);
    mp_red_balance_SpinBox->setValue(0);
    connect(mp_red_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(red_balance_spinbox_changed_slot()));

    mp_green_balance_Slider = new QSlider(Qt::Horizontal);
    mp_green_balance_Slider->setRange(-100, 100);
    mp_green_balance_Slider->setValue(0);
    connect(mp_green_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(green_balance_slider_changed_slot(int)));
    mp_green_balance_SpinBox = new QSpinBox;
    mp_green_balance_SpinBox->setRange(-100, 100);
    mp_green_balance_SpinBox->setValue(0);
    connect(mp_green_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(green_balance_spinbox_changed_slot()));

    mp_blue_balance_Slider = new QSlider(Qt::Horizontal);
    mp_blue_balance_Slider->setRange(-100, 100);
    mp_blue_balance_Slider->setValue(0);
    connect(mp_blue_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(blue_balance_slider_changed_slot(int)));
    mp_blue_balance_SpinBox = new QSpinBox;
    mp_blue_balance_SpinBox->setRange(-100, 100);
    mp_blue_balance_SpinBox->setValue(0);
    connect(mp_blue_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(blue_balance_spinbox_changed_slot()));

    QGridLayout *colour_balance_GLayout = new QGridLayout;
    colour_balance_GLayout->setVerticalSpacing(10);
    colour_balance_GLayout->setHorizontalSpacing(10);
    colour_balance_GLayout->addWidget(new QLabel(tr("Red")), 0, 0);
    colour_balance_GLayout->addWidget(mp_red_balance_Slider, 0, 1);
    colour_balance_GLayout->addWidget(mp_red_balance_SpinBox, 0, 2);
    colour_balance_GLayout->addWidget(new QLabel(tr("Green")), 1, 0);
    colour_balance_GLayout->addWidget(mp_green_balance_Slider, 1, 1);
    colour_balance_GLayout->addWidget(mp_green_balance_SpinBox, 1, 2);
    colour_balance_GLayout->addWidget(new QLabel(tr("Blue")), 2, 0);
    colour_balance_GLayout->addWidget(mp_blue_balance_Slider, 2, 1);
    colour_balance_GLayout->addWidget(mp_blue_balance_SpinBox, 2, 2);

    QPushButton *reset_colour_balance_button = new QPushButton(tr("Reset"));
    reset_colour_balance_button->setAutoDefault(false);
    connect(reset_colour_balance_button, SIGNAL(clicked()), this, SLOT(reset_colour_balance_slot()));
    QPushButton *estimate_colour_balance_button = new QPushButton(tr("Estimate", "Estimate Colour Balance Button"));
    estimate_colour_balance_button->setAutoDefault(false);
    connect(estimate_colour_balance_button, SIGNAL(clicked()), this, SIGNAL(estimate_colour_balance()));

    QHBoxLayout *colour_balance_HLayout = new QHBoxLayout;
    colour_balance_HLayout->setMargin(0);
    colour_balance_HLayout->setSpacing(5);
    colour_balance_HLayout->addWidget(reset_colour_balance_button);
    colour_balance_HLayout->addWidget(estimate_colour_balance_button);
    colour_balance_HLayout->addStretch();

    QVBoxLayout *colour_balance_VLayout = new QVBoxLayout;
    colour_balance_VLayout->setMargin(5);
    colour_balance_VLayout->setSpacing(10);
    colour_balance_VLayout->addLayout(colour_balance_GLayout);
    colour_balance_VLayout->addLayout(colour_balance_HLayout);

    mp_colour_balance_GroupBox = new c_icon_groupbox;
    mp_colour_balance_GroupBox->setTitle(tr("Colour Balance"));
    mp_colour_balance_GroupBox->set_icon(":/res/resources/colour_balance_icon.png");
    mp_colour_balance_GroupBox->setLayout(colour_balance_VLayout);


    QVBoxLayout *dialog_vlayout = new QVBoxLayout;
    dialog_vlayout->setMargin(10);
    dialog_vlayout->setSpacing(15);
    dialog_vlayout->addWidget(mp_debayer_GroupBox);
//    dialog_vlayout->addSpacing(15);
    dialog_vlayout->addWidget(mp_monochrome_conversion_GroupBox);
//    dialog_vlayout->addSpacing(15);
    dialog_vlayout->addWidget(invert_GroupBox);
//    dialog_vlayout->addSpacing(15);
    dialog_vlayout->addWidget(gain_and_gammaGroupBox);
//    dialog_vlayout->addSpacing(15);
    dialog_vlayout->addWidget(mp_colour_saturation_GroupBox);
//    dialog_vlayout->addSpacing(15);
    dialog_vlayout->addWidget(mp_colour_balance_GroupBox);

    setLayout(dialog_vlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


void c_processing_options_dialog::debayer_enable_slot(bool enable)
{
    bool bayer_enabled = m_data_has_bayer_pattern && enable;
    enable_and_disable_controls();
    emit debayer_enable(bayer_enabled);
}



void c_processing_options_dialog::gain_slider_changed_slot(int gain)
{
    mp_gain_DSpinbox->setValue(((double)gain/100.0));
}


void c_processing_options_dialog::gain_spinbox_changed_slot(double gain)
{
    mp_gain_Slider->setValue(100 * gain);
    emit gain_changed(gain);
}


void c_processing_options_dialog::reset_gain_and_gamma_slot()
{
    mp_gain_DSpinbox->setValue(1.0);
    mp_gamma_DSpinbox->setValue(1.0);
}


void c_processing_options_dialog::gamma_slider_changed_slot(int gamma)
{
    mp_gamma_DSpinbox->setValue(((double)gamma/100.0));
}


void c_processing_options_dialog::gamma_spinbox_changed_slot(double gamma)
{
    mp_gamma_Slider->setValue(100 * gamma);
    emit gamma_changed(gamma);
}


void c_processing_options_dialog::monochrome_conversion_changed_slot()
{
    enable_and_disable_controls();
    emit monochrome_conversion_changed(mp_monochrome_conversion_GroupBox->isChecked(), mp_monochrome_conversion_Combobox->currentIndex());
}


void c_processing_options_dialog::colour_saturation_slider_changed_slot(int sat)
{
    mp_colsat_DSpinbox->setValue(((double)sat/100.0));
}


void c_processing_options_dialog::colour_saturation_spinbox_changed_slot()
{
    mp_colsat_Slider->setValue(100 * mp_colsat_DSpinbox->value());
    if (mp_colsat_DSpinbox->isEnabled()) {
        // Control is anabled
        emit colour_saturation_changed(mp_colsat_DSpinbox->value());
    } else {
        // Control is not enabled
        emit colour_saturation_changed(1.0);
    }
}


void c_processing_options_dialog::red_balance_slider_changed_slot(int balance)
{
    mp_red_balance_SpinBox->setValue(balance);
}

void c_processing_options_dialog::red_balance_spinbox_changed_slot()
{
    mp_red_balance_Slider->setValue(mp_red_balance_SpinBox->value());
    if (mp_red_balance_SpinBox->isEnabled()) {
        emit colour_balance_changed(
                    1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                    1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                    1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
    } else {
        emit colour_balance_changed(1.0, 1.0, 1.0);
    }
}


void c_processing_options_dialog::green_balance_slider_changed_slot(int balance)
{
    mp_green_balance_SpinBox->setValue(balance);
}


void c_processing_options_dialog::green_balance_spinbox_changed_slot()
{
    mp_green_balance_Slider->setValue(mp_green_balance_SpinBox->value());
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_processing_options_dialog::blue_balance_slider_changed_slot(int balance)
{
    mp_blue_balance_SpinBox->setValue(balance);
}

void c_processing_options_dialog::blue_balance_spinbox_changed_slot()
{
    mp_blue_balance_Slider->setValue(mp_blue_balance_SpinBox->value());
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_processing_options_dialog::reset_colour_saturation_slot()
{
    mp_colsat_DSpinbox->setValue(1.0);
}


void c_processing_options_dialog::reset_colour_balance_slot()
{
    mp_red_balance_SpinBox->setValue(0);
    mp_green_balance_SpinBox->setValue(0);
    mp_blue_balance_SpinBox->setValue(0);
}


void c_processing_options_dialog::reset_all_slot()
{
    mp_debayer_CheckBox->setChecked(true);
    mp_invert_CheckBox->setChecked(false);
    mp_monochrome_conversion_GroupBox->setChecked(false);
    mp_monochrome_conversion_Combobox->setCurrentIndex(0);
    reset_gain_and_gamma_slot();
    reset_colour_saturation_slot();
    reset_colour_balance_slot();
}


void c_processing_options_dialog::set_colour_balance(double red, double green, double blue)
{
    int red_int = (red - 1.0) * 300;
    int green_int = (green - 1.0) * 300;
    int blue_int = (blue - 1.0) * 300;

    int max_int = red_int;
    if (green_int > max_int) {
        max_int = red_int;
    }

    if (blue_int > max_int) {
        max_int = blue_int;
    }

    if (max_int > 100) {
        // This is greater than the max gain possible - try to reduce other gains
        int min_int = red_int;
        if (green_int < min_int) {
            min_int = green_int;
        }

        if (blue_int < min_int) {
            min_int = blue_int;
        }

        int reduction = max_int - 100;
        if (min_int - reduction < -100) {
            int over_reduction = reduction - min_int - 100;

            // Reduce reduction by half of the over reduction
            reduction -= (over_reduction / 2);
        }


        red_int -= reduction;
        green_int -= reduction;
        blue_int -= reduction;
    }


    red_int < (mp_red_balance_SpinBox->minimum()) ? mp_red_balance_SpinBox->minimum() : red_int;
    green_int < (mp_green_balance_SpinBox->minimum()) ? mp_green_balance_SpinBox->minimum() : green_int;
    blue_int < (mp_blue_balance_SpinBox->minimum()) ? mp_blue_balance_SpinBox->minimum() : blue_int;

    red_int > (mp_red_balance_SpinBox->maximum()) ? mp_red_balance_SpinBox->maximum() : red_int;
    green_int > (mp_green_balance_SpinBox->maximum()) ? mp_green_balance_SpinBox->maximum() : green_int;
    blue_int > (mp_blue_balance_SpinBox->maximum()) ? mp_blue_balance_SpinBox->maximum() : blue_int;

    mp_red_balance_SpinBox->setValue(red_int);
    mp_green_balance_SpinBox->setValue(green_int);
    mp_blue_balance_SpinBox->setValue(blue_int);
}


void c_processing_options_dialog::set_data_has_bayer_pattern(bool bayer_pattern) {
    m_data_has_bayer_pattern = bayer_pattern;
    enable_and_disable_controls();
}

void c_processing_options_dialog::set_data_is_colour(bool colour) {
    m_data_is_colour = colour;
    enable_and_disable_controls();
}


bool c_processing_options_dialog::get_debayer_enable()
{
    return m_data_has_bayer_pattern && mp_debayer_CheckBox->isChecked();
}


void c_processing_options_dialog::enable_and_disable_controls()
{
    mp_debayer_CheckBox->setEnabled(m_data_has_bayer_pattern);
    mp_debayer_GroupBox->setVisible(m_data_has_bayer_pattern);

    bool enable_monochrome_conversion_control = false;
    if (m_data_is_colour) {
        // This is colour data
        enable_monochrome_conversion_control = true;
    } else if (m_data_has_bayer_pattern && mp_debayer_CheckBox->isChecked()) {
        enable_monochrome_conversion_control = true;
    }

    mp_monochrome_conversion_GroupBox->setEnabled(enable_monochrome_conversion_control);
    mp_monochrome_conversion_GroupBox->setVisible(enable_monochrome_conversion_control);

    bool enable_colour_controls;
    if (mp_monochrome_conversion_GroupBox->isChecked()) {
        // The data must be monochrome if this control is checked
        enable_colour_controls = false;
    } else if (m_data_is_colour) {
        // This is colour data
        enable_colour_controls = true;
    } else if (m_data_has_bayer_pattern && mp_debayer_CheckBox->isChecked()) {
        // Debayered data with bayer pattern is colour data
        enable_colour_controls = true;
    } else {
        // Otherwise this is monochrome data
        enable_colour_controls = false;
    }

    mp_colour_saturation_GroupBox->setEnabled(enable_colour_controls);
    mp_colour_saturation_GroupBox->setVisible(enable_colour_controls);
    mp_colour_balance_GroupBox->setEnabled(enable_colour_controls);
    mp_colour_balance_GroupBox->setVisible(enable_colour_controls);
}
