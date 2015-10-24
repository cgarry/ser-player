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
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include "colour_dialog.h"


c_colour_dialog::c_colour_dialog(QWidget *parent)
    : QDialog(parent)
//    : QDialog(parent,   Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
{
    setWindowTitle(tr("Colour Settings"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Colour saturation label and spinbox
    mp_colsat_Slider = new QSlider(Qt::Horizontal);
    mp_colsat_Slider->setRange(0, 1500);
    mp_colsat_Slider->setValue(100);
    mp_colsat_Slider->setMinimumWidth(mp_colsat_Slider->sizeHint().width() * 2);
    connect(mp_colsat_Slider, SIGNAL(sliderMoved(int)), this, SLOT(colour_saturation_slider_changed_slot(int)));
    mp_colsat_DSpinbox = new QDoubleSpinBox;
    mp_colsat_DSpinbox->setRange(0.0, 15.0);
    mp_colsat_DSpinbox->setSingleStep(0.01);
    mp_colsat_DSpinbox->setValue(1.0);

    connect(mp_colsat_DSpinbox, SIGNAL(valueChanged(double)), this, SLOT(colour_saturation_spinbox_changed_slot(double)));
    QHBoxLayout *colsat_hlayout1 = new QHBoxLayout;
    colsat_hlayout1->addWidget(new QLabel(tr("Saturation")));
    colsat_hlayout1->addWidget(mp_colsat_Slider);
    colsat_hlayout1->addWidget(mp_colsat_DSpinbox);

    QPushButton *reset_colour_saturation_button = new QPushButton(tr("Reset"));
    connect(reset_colour_saturation_button, SIGNAL(clicked()), this, SLOT(reset_colour_saturation_slot()));
    QHBoxLayout *colsat_hlayout2 = new QHBoxLayout;
    colsat_hlayout2->addWidget(reset_colour_saturation_button);
    colsat_hlayout2->addStretch();
    QVBoxLayout *colsat_vlayout = new QVBoxLayout;
    colsat_vlayout->setSpacing(15);
    colsat_vlayout->addLayout(colsat_hlayout1);
    colsat_vlayout->addLayout(colsat_hlayout2);
    QGroupBox *colout_saturation_GroupBox = new QGroupBox(tr("Colour Saturation"));
    colout_saturation_GroupBox->setLayout(colsat_vlayout);

    // Colour balance
    mp_red_balance_Slider = new QSlider(Qt::Horizontal);
    mp_red_balance_Slider->setRange(-100, 100);
    mp_red_balance_Slider->setValue(0);
    mp_red_balance_Slider->setMinimumWidth(mp_red_balance_Slider->sizeHint().width() * 2);
    connect(mp_red_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(red_balance_slider_changed_slot(int)));
    mp_red_balance_SpinBox = new QSpinBox;
    mp_red_balance_SpinBox->setRange(-100, 100);
    mp_red_balance_SpinBox->setValue(0);
    connect(mp_red_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(red_balance_spinbox_changed_slot(int)));

    mp_green_balance_Slider = new QSlider(Qt::Horizontal);
    mp_green_balance_Slider->setRange(-100, 100);
    mp_green_balance_Slider->setValue(0);
    connect(mp_green_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(green_balance_slider_changed_slot(int)));
    mp_green_balance_SpinBox = new QSpinBox;
    mp_green_balance_SpinBox->setRange(-100, 100);
    mp_green_balance_SpinBox->setValue(0);
    connect(mp_green_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(green_balance_spinbox_changed_slot(int)));

    mp_blue_balance_Slider = new QSlider(Qt::Horizontal);
    mp_blue_balance_Slider->setRange(-100, 100);
    mp_blue_balance_Slider->setValue(0);
    connect(mp_blue_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(blue_balance_slider_changed_slot(int)));
    mp_blue_balance_SpinBox = new QSpinBox;
    mp_blue_balance_SpinBox->setRange(-100, 100);
    mp_blue_balance_SpinBox->setValue(0);
    connect(mp_blue_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(blue_balance_spinbox_changed_slot(int)));

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
    connect(reset_colour_balance_button, SIGNAL(clicked()), this, SLOT(reset_colour_balance_slot()));
    QPushButton *estimate_colour_balance_button = new QPushButton(tr("Estimate", "Estimate Colour Balance Button"));
    connect(estimate_colour_balance_button, SIGNAL(clicked()), this, SIGNAL(estimate_colour_balance()));

    QHBoxLayout *colour_balance_HLayout = new QHBoxLayout;
    colour_balance_HLayout->setMargin(0);
    colour_balance_HLayout->setSpacing(5);
    colour_balance_HLayout->addWidget(reset_colour_balance_button);
    colour_balance_HLayout->addWidget(estimate_colour_balance_button);
    colour_balance_HLayout->addStretch();

    QVBoxLayout *colour_balance_VLayout = new QVBoxLayout;
    colour_balance_VLayout->setSpacing(15);
    colour_balance_VLayout->addLayout(colour_balance_GLayout);
    colour_balance_VLayout->addLayout(colour_balance_HLayout);

    QGroupBox *colour_balance_GroupBox = new QGroupBox(tr("Colour Balance"));
    colour_balance_GroupBox->setLayout(colour_balance_VLayout);

    // Reset and Close buttons
    QPushButton *close_button = new QPushButton(tr("Close"));
    connect(close_button, SIGNAL(clicked()), this, SLOT(hide()));
    QHBoxLayout *buttons_hlayout = new QHBoxLayout;
    buttons_hlayout->addStretch();
    buttons_hlayout->addWidget(close_button);

    QVBoxLayout *dialog_vlayout = new QVBoxLayout;
    dialog_vlayout->setMargin(10);
    dialog_vlayout->setSpacing(15);
    dialog_vlayout->addWidget(colout_saturation_GroupBox);
    dialog_vlayout->addWidget(colour_balance_GroupBox);
    dialog_vlayout->addStretch();
    dialog_vlayout->addLayout(buttons_hlayout);

    setLayout(dialog_vlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


void c_colour_dialog::colour_saturation_slider_changed_slot(int sat)
{
    mp_colsat_DSpinbox->setValue(((double)sat/100.0));
}


void c_colour_dialog::colour_saturation_spinbox_changed_slot(double sat)
{
    mp_colsat_Slider->setValue(100 * sat);
    emit colour_saturation_changed(sat);
}


void c_colour_dialog::red_balance_slider_changed_slot(int balance)
{
    mp_red_balance_SpinBox->setValue(balance);
}

void c_colour_dialog::red_balance_spinbox_changed_slot(int balance)
{
    mp_red_balance_Slider->setValue(balance);
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_colour_dialog::green_balance_slider_changed_slot(int balance)
{
    mp_green_balance_SpinBox->setValue(balance);
}


void c_colour_dialog::green_balance_spinbox_changed_slot(int balance)
{
    mp_green_balance_Slider->setValue(balance);
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_colour_dialog::blue_balance_slider_changed_slot(int balance)
{
    mp_blue_balance_SpinBox->setValue(balance);
}

void c_colour_dialog::blue_balance_spinbox_changed_slot(int balance)
{
    mp_blue_balance_Slider->setValue(balance);
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_colour_dialog::reset_colour_saturation_slot()
{
    mp_colsat_DSpinbox->setValue(1.0);
}


void c_colour_dialog::reset_colour_balance_slot()
{
    mp_red_balance_SpinBox->setValue(0);
    mp_green_balance_SpinBox->setValue(0);
    mp_blue_balance_SpinBox->setValue(0);
}


void c_colour_dialog::set_colour_balance(double red, double green, double blue)
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
