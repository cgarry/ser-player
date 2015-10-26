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
#include "persistent_data.h"


c_markers_dialog::c_markers_dialog(QWidget *parent)
    : QDialog(parent)  //, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
{
    setWindowTitle(tr("Start/End Markers"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mp_start_market_Label = new QLabel(tr("Start Marker:"));
    mp_start_marker_SpinBox = new QSpinBox;
    mp_start_marker_SpinBox->setRange(1, 1);
    connect(mp_start_marker_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(marker_changed_slot()));

    QPushButton *start_marker_mode_Button = new QPushButton(tr("Move To Current Frame"));
    start_marker_mode_Button->setAutoDefault(false);
    connect(start_marker_mode_Button, SIGNAL(clicked()), this, SIGNAL(set_start_marker_to_current()));

    mp_end_market_Label = new QLabel(tr("End Marker:"));
    mp_end_marker_SpinBox = new QSpinBox;
    mp_end_marker_SpinBox->setRange(1, 1);
    connect(mp_end_marker_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(marker_changed_slot()));

    QPushButton *end_marker_mode_Button = new QPushButton(tr("Move To Current Frame"));
    end_marker_mode_Button->setAutoDefault(false);
    connect(end_marker_mode_Button, SIGNAL(clicked()), this, SIGNAL(set_end_marker_to_current()));

    mp_selected_count_Label = new QLabel("1");

    QPushButton *reset_markers_Button = new QPushButton(tr("Reset"));
    reset_markers_Button->setAutoDefault(false);
    connect(reset_markers_Button, SIGNAL(clicked()), this, SLOT(reset_markers_slot()));

    QGridLayout *markers_GLayout = new QGridLayout;
    markers_GLayout->setMargin(00);
    markers_GLayout->setSpacing(10);
    markers_GLayout->addWidget(mp_start_market_Label, 0, 0);
    markers_GLayout->addWidget(mp_start_marker_SpinBox, 0, 1);
    markers_GLayout->addWidget(start_marker_mode_Button, 0, 2);
    markers_GLayout->addWidget(mp_end_market_Label, 1, 0);
    markers_GLayout->addWidget(mp_end_marker_SpinBox, 1, 1);
    markers_GLayout->addWidget(end_marker_mode_Button, 1, 2);
    markers_GLayout->addWidget(new QLabel(tr("Marked Frames:")), 2, 0);
    markers_GLayout->addWidget(mp_selected_count_Label, 2, 1);
    markers_GLayout->addWidget(reset_markers_Button, 2, 2);

    QHBoxLayout *markers_HLayout = new QHBoxLayout;
    markers_HLayout->setMargin(10);
    markers_HLayout->setSpacing(10);
    markers_HLayout->addLayout(markers_GLayout);

    mp_markers_GroupBox = new QGroupBox(tr("Enable Markers"));
    mp_markers_GroupBox->setLayout(markers_HLayout);
    mp_markers_GroupBox->setCheckable(true);
    mp_markers_GroupBox->setChecked(c_persistent_data::m_markers_enabled);
    connect(mp_markers_GroupBox, SIGNAL(clicked(bool)), this, SIGNAL(markers_enabled_changed(bool)));

    QVBoxLayout *main_VLayout = new QVBoxLayout;
    main_VLayout->setMargin(10);
    main_VLayout->setSpacing(15);
    main_VLayout->addWidget(mp_markers_GroupBox);

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


void c_markers_dialog::marker_changed_slot()
{
    // Update spinbox handling to allow non-valid combinations to exist while entering
    QPalette text_Palette;
    if (mp_start_marker_SpinBox->value() > mp_end_marker_SpinBox->value()) {
        // Value is not currently valid
        text_Palette.setColor(QPalette::Text,Qt::red);
        mp_start_marker_SpinBox->setPalette(text_Palette);
        mp_end_marker_SpinBox->setPalette(text_Palette);
    } else {
        text_Palette.setColor(QPalette::Text,Qt::black);
        mp_start_marker_SpinBox->setPalette(text_Palette);
        mp_end_marker_SpinBox->setPalette(text_Palette);
        emit start_marker_changed(mp_start_marker_SpinBox->value());
        emit end_marker_changed(mp_end_marker_SpinBox->value());
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
