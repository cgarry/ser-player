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
#include <QVBoxLayout>


#include "colour_dialog.h"

c_colour_dialog::c_colour_dialog(QWidget *parent)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
    //: QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint)//| Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
//    setModal(true);
    setWindowTitle(tr("Colour Settings"));

    // Colour saturation label and spinbox
    QLabel *colsat_Label = new QLabel(tr("Colour Saturation"));
    mp_colsat_DSpinbox = new QDoubleSpinBox;
    mp_colsat_DSpinbox->setMinimum(0.0);
    mp_colsat_DSpinbox->setMaximum(15.0);
    mp_colsat_DSpinbox->setSingleStep(0.25);
    mp_colsat_DSpinbox->setValue(1.0);
    connect(mp_colsat_DSpinbox, SIGNAL(valueChanged(double)), this, SIGNAL(colour_saturation_changed(double)));
    QHBoxLayout *colsat_hlayout = new QHBoxLayout;
    colsat_hlayout->addWidget(colsat_Label);
    colsat_hlayout->addWidget(mp_colsat_DSpinbox);

    // Colour balance
    mp_red_balance_Slider = new QSlider(Qt::Horizontal);
    mp_red_balance_Slider->setRange(-255, +255);
    mp_red_balance_Slider->setValue(0);
    mp_red_balance_Slider->setFixedWidth(mp_red_balance_Slider->sizeHint().width() * 2);
    connect(mp_red_balance_Slider, SIGNAL(valueChanged(int)), this, SLOT(red_balanced_changed_slot(int)));
    int min_label_width = QLabel(QString::number(-255)).sizeHint().width();
    mp_red_balance_Label = new QLabel(QString::number(0));
    mp_red_balance_Label->setFixedWidth(min_label_width);
    mp_green_balance_Slider = new QSlider(Qt::Horizontal);
    mp_green_balance_Slider->setRange(-255, +255);
    mp_green_balance_Slider->setValue(0);
    connect(mp_green_balance_Slider, SIGNAL(valueChanged(int)), this, SLOT(green_balanced_changed_slot(int)));
    mp_green_balance_Label = new QLabel(QString::number(0));
    mp_green_balance_Label->setFixedWidth(min_label_width);
    mp_blue_balance_Slider = new QSlider(Qt::Horizontal);
    mp_blue_balance_Slider->setRange(-255, +255);
    mp_blue_balance_Slider->setValue(0);
    connect(mp_blue_balance_Slider, SIGNAL(valueChanged(int)), this, SLOT(blue_balanced_changed_slot(int)));
    mp_blue_balance_Label = new QLabel(QString::number(0));
    mp_blue_balance_Label->setFixedWidth(min_label_width);

    QGridLayout *colour_balance_Layout = new QGridLayout;
    colour_balance_Layout->setVerticalSpacing(5);
    colour_balance_Layout->setHorizontalSpacing(10);
    colour_balance_Layout->addWidget(new QLabel(tr("Red")), 0, 0);
    colour_balance_Layout->addWidget(mp_red_balance_Slider, 0, 1);
    colour_balance_Layout->addWidget(mp_red_balance_Label, 0, 2);
    colour_balance_Layout->addWidget(new QLabel(tr("Green")), 1, 0);
    colour_balance_Layout->addWidget(mp_green_balance_Slider, 1, 1);
    colour_balance_Layout->addWidget(mp_green_balance_Label, 1, 2);
    colour_balance_Layout->addWidget(new QLabel(tr("Blue")), 2, 0);
    colour_balance_Layout->addWidget(mp_blue_balance_Slider, 2, 1);
    colour_balance_Layout->addWidget(mp_blue_balance_Label, 2, 2);

    QGroupBox *colour_balance_GroupBox = new QGroupBox(tr("Colour Balance"));
    colour_balance_GroupBox->setLayout(colour_balance_Layout);

    // Reset and Close buttons
    QPushButton *reset_button = new QPushButton(tr("Reset"));
    connect(reset_button, SIGNAL(clicked()), this, SLOT(reset_dialog()));

    QPushButton *close_button = new QPushButton(tr("Close"));
    connect(close_button, SIGNAL(clicked()), this, SLOT(hide()));
    QHBoxLayout *buttons_hlayout = new QHBoxLayout;
    buttons_hlayout->addWidget(reset_button);
    buttons_hlayout->addWidget(close_button);

    QVBoxLayout *dialog_vlayout = new QVBoxLayout;
    dialog_vlayout->setMargin(5);
    dialog_vlayout->setSpacing(15);
    dialog_vlayout->addLayout(colsat_hlayout);
    dialog_vlayout->addWidget(colour_balance_GroupBox);
    dialog_vlayout->addLayout(buttons_hlayout);

    setLayout(dialog_vlayout);
}


void c_colour_dialog::red_balanced_changed_slot(int balance) {
    mp_red_balance_Label->setText(QString::number(balance));
    emit colour_balance_changed(
                mp_red_balance_Slider->value(),
                mp_green_balance_Slider->value(),
                mp_blue_balance_Slider->value());
}


void c_colour_dialog::green_balanced_changed_slot(int balance) {
    mp_green_balance_Label->setText(QString::number(balance));
    emit colour_balance_changed(
                mp_red_balance_Slider->value(),
                mp_green_balance_Slider->value(),
                mp_blue_balance_Slider->value());
}


void c_colour_dialog::blue_balanced_changed_slot(int balance) {
    mp_blue_balance_Label->setText(QString::number(balance));
    emit colour_balance_changed(
                mp_red_balance_Slider->value(),
                mp_green_balance_Slider->value(),
                mp_blue_balance_Slider->value());
}


void c_colour_dialog::reset_dialog() {
    mp_colsat_DSpinbox->setValue(1.0);
    mp_red_balance_Slider->setValue(0);
    mp_green_balance_Slider->setValue(0);
    mp_blue_balance_Slider->setValue(0);
}
