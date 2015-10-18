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
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint),
      m_start_marker_enabled(false),
      m_end_marker_enabled(false)
{
    setWindowTitle(tr("Start/End Markers"));

    mp_start_market_Label = new QLabel(tr("Start Marker:"));
    mp_start_market_Label->setEnabled(false);
    mp_start_marker_SpinBox = new QSpinBox;
    mp_start_marker_SpinBox->setRange(1, 1);
    mp_start_marker_SpinBox->setEnabled(false);
    connect(mp_start_marker_SpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(start_marker_changed(int)));

    mp_end_market_Label = new QLabel(tr("End Marker:"));
    mp_end_market_Label->setEnabled(false);
    mp_end_marker_SpinBox = new QSpinBox;
    mp_end_marker_SpinBox->setRange(1, 1);
    mp_end_marker_SpinBox->setEnabled(false);
    connect(mp_end_marker_SpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(end_marker_changed(int)));

    mp_selected_count_Label = new QLabel("1");

    QFormLayout *markers_FLayout = new QFormLayout;
    markers_FLayout->setMargin(0);
    markers_FLayout->addRow(mp_start_market_Label, mp_start_marker_SpinBox);
    markers_FLayout->addRow(mp_end_market_Label, mp_end_marker_SpinBox);
    markers_FLayout->addRow(tr("Marked Frames:"), mp_selected_count_Label);

    QGroupBox *markers_GroupBox = new QGroupBox(tr("Markers"));
    markers_GroupBox->setLayout(markers_FLayout);

    QPushButton *close_Button = new QPushButton(tr("Close"));
    connect(close_Button, SIGNAL(clicked()), this, SLOT(hide()));

    QHBoxLayout *buttons_HLayout = new QHBoxLayout;
    buttons_HLayout->setMargin(0);
    buttons_HLayout->addStretch();
    buttons_HLayout->addWidget(close_Button);

    QVBoxLayout *main_VLayout = new QVBoxLayout;
    main_VLayout->setMargin(10);
    main_VLayout->addWidget(markers_GroupBox);
    main_VLayout->addStretch();
    main_VLayout->addLayout(buttons_HLayout);

    setLayout(main_VLayout);
}


void c_markers_dialog::set_maximum_frame(int value)
{
    mp_start_marker_SpinBox->setMaximum(value);
    mp_end_marker_SpinBox->setMaximum(value);
}


void c_markers_dialog::button_clicked()
{
    qDebug() << "Close button clicked";
}


void c_markers_dialog::set_start_marker_slot(int value)
{
    if (value == -1 || value == 1) {
        m_start_marker_enabled = false;
        mp_start_marker_SpinBox->setValue(1);
        mp_start_marker_SpinBox->setEnabled(false);
        mp_start_market_Label->setEnabled(false);
    } else {
        m_start_marker_enabled = true;
        mp_start_marker_SpinBox->setValue(value);
        mp_start_marker_SpinBox->setEnabled(true);
        mp_start_market_Label->setEnabled(true);
    }

    int marked_frames = mp_end_marker_SpinBox->value() - mp_start_marker_SpinBox->value() + 1;
    mp_selected_count_Label->setText(QString::number(marked_frames));
}


void c_markers_dialog::set_end_marker_slot(int value)
{
    if (value == -1 || value == mp_end_marker_SpinBox->maximum()) {
        m_end_marker_enabled = false;
        mp_end_marker_SpinBox->setValue(mp_end_marker_SpinBox->maximum());
        mp_end_marker_SpinBox->setEnabled(false);
        mp_end_market_Label->setEnabled(false);
    } else {
        m_end_marker_enabled = true;
        mp_end_marker_SpinBox->setValue(value);
        mp_end_marker_SpinBox->setEnabled(true);
        mp_end_market_Label->setEnabled(true);
    }

    int marked_frames = mp_end_marker_SpinBox->value() - mp_start_marker_SpinBox->value() + 1;
    mp_selected_count_Label->setText(QString::number(marked_frames));
}
