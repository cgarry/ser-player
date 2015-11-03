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
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

#include "processing_options_dialog.h"
#include "persistent_data.h"


c_processing_options_dialog::c_processing_options_dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Processing Options"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);


    // Debayer checkbox
    QCheckBox *mp_debayer_CheckBox = new QCheckBox(tr("Enable Debayering"));
    mp_debayer_CheckBox->setChecked(c_persistent_data::m_enable_debayering);
    connect(mp_debayer_CheckBox, SIGNAL(toggled(bool)), this, SLOT(debayer_enable_slot(bool)));

    QHBoxLayout *debayer_HLayout1 = new QHBoxLayout;
    debayer_HLayout1->addWidget(mp_debayer_CheckBox);

    QGroupBox *debayer_GroupBox = new QGroupBox(tr("Colour Debayer"));
    debayer_GroupBox->setLayout(debayer_HLayout1);


    // Invert checkbox
    QCheckBox *mp_invert_CheckBox = new QCheckBox(tr("Invert Frames"));
    mp_invert_CheckBox->setChecked(false);
    connect(mp_invert_CheckBox, SIGNAL(toggled(bool)), this, SIGNAL(invert_frames(bool)));

    QHBoxLayout *invert_HLayout1 = new QHBoxLayout;
    invert_HLayout1->addWidget(mp_invert_CheckBox);

    QGroupBox *invert_GroupBox = new QGroupBox(tr("Frame Inversion"));
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

    QHBoxLayout *gain_HLayout1 = new QHBoxLayout;
    gain_HLayout1->addWidget(new QLabel(tr("Gain")));
    gain_HLayout1->addWidget(mp_gain_Slider);
    gain_HLayout1->addWidget(mp_gain_DSpinbox);

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

    QHBoxLayout *gamma_HLayout1 = new QHBoxLayout;
    gamma_HLayout1->addWidget(new QLabel(tr("Gamma")));
    gamma_HLayout1->addWidget(mp_gamma_Slider);
    gamma_HLayout1->addWidget(mp_gamma_DSpinbox);

    QPushButton *reset_gain_and_gamma_Button = new QPushButton(tr("Reset"));
    reset_gain_and_gamma_Button->setAutoDefault(false);
    connect(reset_gain_and_gamma_Button, SIGNAL(clicked()), this, SLOT(reset_gain_and_gamma_slot()));

    QHBoxLayout *gamma_HLayout2 = new QHBoxLayout;
    gamma_HLayout2->addWidget(reset_gain_and_gamma_Button);
    gamma_HLayout2->addStretch();

    QVBoxLayout *gain_and_gamma_Vlayout = new QVBoxLayout;
    gain_and_gamma_Vlayout->setSpacing(10);
    gain_and_gamma_Vlayout->addLayout(gain_HLayout1);
    gain_and_gamma_Vlayout->addLayout(gamma_HLayout1);
    gain_and_gamma_Vlayout->addLayout(gamma_HLayout2);

    QGroupBox *gain_and_gammaGroupBox = new QGroupBox(tr("Gain And Gamma"));
    gain_and_gammaGroupBox->setLayout(gain_and_gamma_Vlayout);

    QVBoxLayout *dialog_vlayout = new QVBoxLayout;
    dialog_vlayout->setMargin(10);
    dialog_vlayout->setSpacing(15);
    dialog_vlayout->addWidget(debayer_GroupBox);
    dialog_vlayout->addWidget(invert_GroupBox);
    dialog_vlayout->addWidget(gain_and_gammaGroupBox);

    setLayout(dialog_vlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


void c_processing_options_dialog::debayer_enable_slot(bool enable)
{
    c_persistent_data::m_enable_debayering = enable;
    emit debayer_enable(enable);
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

