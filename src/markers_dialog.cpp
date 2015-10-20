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
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "markers_dialog.h"


c_markers_dialog::c_markers_dialog(QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
{
    setWindowTitle(tr("Start/End Markers"));

    mp_start_market_Label = new QLabel(tr("Start Marker:"));
    mp_start_marker_SpinBox = new QSpinBox;
    mp_start_marker_SpinBox->setRange(1, 1);
    mp_red_text_Palette = new QPalette();
    mp_red_text_Palette->setColor(QPalette::Text,Qt::red);
    mp_black_text_Palette = new QPalette();
    mp_black_text_Palette->setColor(QPalette::Text,Qt::black);
    mp_start_marker_SpinBox->setPalette(*mp_red_text_Palette);
    connect(mp_start_marker_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(start_marker_changed_slot(int)));

    mp_end_market_Label = new QLabel(tr("End Marker:"));
    mp_end_marker_SpinBox = new QSpinBox;
    mp_end_marker_SpinBox->setRange(1, 1);
    connect(mp_end_marker_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(end_marker_changed_slot(int)));

    mp_selected_count_Label = new QLabel("1");

    QFormLayout *markers_FLayout = new QFormLayout;
    markers_FLayout->setMargin(00);
    markers_FLayout->setSpacing(10);
    markers_FLayout->addRow(mp_start_market_Label, mp_start_marker_SpinBox);
    markers_FLayout->addRow(mp_end_market_Label, mp_end_marker_SpinBox);
    markers_FLayout->addRow(tr("Marked Frames:"), mp_selected_count_Label);

    QPushButton *reset_markers_Button = new QPushButton(tr("Reset"));
    connect(reset_markers_Button, SIGNAL(clicked()), this, SLOT(reset_markers_slot()));

    QHBoxLayout *markers_reset_button_HLayout = new QHBoxLayout;
    markers_reset_button_HLayout->setMargin(0);
    markers_reset_button_HLayout->setSpacing(0);
    markers_reset_button_HLayout->addWidget(reset_markers_Button, 0, Qt::AlignBottom);
    markers_reset_button_HLayout->addStretch();

    QHBoxLayout *markers_HLayout = new QHBoxLayout;
    markers_HLayout->setMargin(10);
    markers_HLayout->setSpacing(10);
    markers_HLayout->addLayout(markers_FLayout);
    markers_HLayout->addSpacing(20);
    markers_HLayout->addLayout(markers_reset_button_HLayout);

    mp_markers_GroupBox = new QGroupBox(tr("Enable Markers"));
    mp_markers_GroupBox->setLayout(markers_HLayout);
    mp_markers_GroupBox->setCheckable(true);
    mp_markers_GroupBox->setChecked(false);
    connect(mp_markers_GroupBox, SIGNAL(clicked(bool)), this, SIGNAL(markers_enabled_changed(bool)));

    QPushButton *close_Button = new QPushButton(tr("Close"));
    connect(close_Button, SIGNAL(clicked()), this, SLOT(hide()));

    QHBoxLayout *buttons_HLayout = new QHBoxLayout;
    buttons_HLayout->setMargin(0);
    buttons_HLayout->addStretch();
    buttons_HLayout->addWidget(close_Button);

    QVBoxLayout *main_VLayout = new QVBoxLayout;
    main_VLayout->setMargin(5);
    main_VLayout->setSpacing(15);
    main_VLayout->addWidget(mp_markers_GroupBox);
    main_VLayout->addStretch();
    main_VLayout->addLayout(buttons_HLayout);

    setLayout(main_VLayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);  // No resizing
}


void c_markers_dialog::set_maximum_frame(int value)
{
    mp_start_marker_SpinBox->setMaximum(value);
    mp_end_marker_SpinBox->setMaximum(value);
}


bool c_markers_dialog::get_markers_enabled()
{
    return mp_markers_GroupBox->isChecked();
}


void c_markers_dialog::start_marker_changed_slot(int value)
{
    // Update spinbox handling to allow non-valid combinations to exist while entering
    if (mp_start_marker_SpinBox->value() > mp_end_marker_SpinBox->value()) {
        // Value is not currently valid
        mp_start_marker_SpinBox->setPalette(*mp_red_text_Palette);
        mp_end_marker_SpinBox->setPalette(*mp_red_text_Palette);
    } else {
        mp_start_marker_SpinBox->setPalette(*mp_black_text_Palette);
        mp_end_marker_SpinBox->setPalette(*mp_black_text_Palette);
        emit start_marker_changed(value);
        emit end_marker_changed(mp_end_marker_SpinBox->value());
    }
}


void c_markers_dialog::end_marker_changed_slot(int value)
{
    if (mp_start_marker_SpinBox->value() > mp_end_marker_SpinBox->value()) {
        // value is not currently valid
        mp_start_marker_SpinBox->setPalette(*mp_red_text_Palette);
        mp_end_marker_SpinBox->setPalette(*mp_red_text_Palette);
    } else {
        mp_start_marker_SpinBox->setPalette(*mp_black_text_Palette);
        mp_end_marker_SpinBox->setPalette(*mp_black_text_Palette);
        emit end_marker_changed(value);
        emit start_marker_changed(mp_start_marker_SpinBox->value());
    }
}


void c_markers_dialog::reset_markers_slot()
{
    mp_start_marker_SpinBox->setValue(mp_start_marker_SpinBox->minimum());
    mp_end_marker_SpinBox->setValue(mp_end_marker_SpinBox->maximum());
}


void c_markers_dialog::set_start_marker_slot(int value)
{
    mp_start_marker_SpinBox->setValue(value);
    int marked_frames = mp_end_marker_SpinBox->value() - mp_start_marker_SpinBox->value() + 1;
    mp_selected_count_Label->setText(QString::number(marked_frames));
}


void c_markers_dialog::set_end_marker_slot(int value)
{
    mp_end_marker_SpinBox->setValue(value);
    int marked_frames = mp_end_marker_SpinBox->value() - mp_start_marker_SpinBox->value() + 1;
    mp_selected_count_Label->setText(QString::number(marked_frames));
}
