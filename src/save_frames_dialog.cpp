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
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "save_frames_dialog.h"


c_save_frames_dialog::c_save_frames_dialog(QWidget *parent, int total_frames, int marker_start_frame, int marker_end_frame)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint),
      m_total_frames(total_frames),
      m_marker_start_frame(marker_start_frame),
      m_marker_end_frame(marker_end_frame),
      m_start_frame(1),
      m_end_frame(total_frames)
{
    setWindowTitle(tr("Save Frames As Images"));
    QDialog::setModal(true);

    QLabel *save_frames_Label = new QLabel(tr("Select which frames to save:"));
    
    mp_save_current_frame_RButton = new QRadioButton(tr("Save Current Frame Only"));
    mp_save_all_frames_RButton = new QRadioButton(tr("Save All %1 Frames").arg(total_frames));
    if (marker_start_frame == 1 && marker_end_frame == total_frames) {
        // No markers enabled
        mp_save_marked_frames_RButton = new QRadioButton(tr("No Frames Selected By Start/End Markers"));
        mp_save_marked_frames_RButton->setEnabled(false);
    } else {
        mp_save_marked_frames_RButton = new QRadioButton(tr("Save Frames Selected By Start/End Markers (%1 to %2)")
                                                         .arg(marker_start_frame).arg(marker_end_frame));
        // Markers are selected
    }

    mp_save_frame_range_RButton = new QRadioButton(tr("Save Frames From: "));
    mp_save_current_frame_RButton->setChecked(true);
    mp_start_Spinbox = new QSpinBox;
    mp_start_Spinbox->setMinimum(1);
    mp_start_Spinbox->setMaximum(total_frames);
    mp_start_Spinbox->setValue(1);
    connect(mp_start_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(start_Spinbox_changed_slot(int)));
    mp_end_Spinbox = new QSpinBox;
    mp_end_Spinbox->setMinimum(1);
    mp_end_Spinbox->setMaximum(total_frames);
    mp_end_Spinbox->setValue(total_frames);
    connect(mp_end_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(end_Spinbox_changed_slot(int)));

    QHBoxLayout *custom_range_HLayout = new QHBoxLayout;
    custom_range_HLayout->setMargin(0);
    custom_range_HLayout->setSpacing(0);
    custom_range_HLayout->addWidget(mp_save_frame_range_RButton);
    custom_range_HLayout->addWidget(mp_start_Spinbox);
    custom_range_HLayout->addWidget(new QLabel(tr(" to ")));
    custom_range_HLayout->addWidget(mp_end_Spinbox);
    custom_range_HLayout->addStretch();
    
    QVBoxLayout *save_options_VLayout = new QVBoxLayout;
    save_options_VLayout->addWidget(mp_save_current_frame_RButton);
    save_options_VLayout->addWidget(mp_save_marked_frames_RButton);
    save_options_VLayout->addLayout(custom_range_HLayout);
    save_options_VLayout->addWidget(mp_save_all_frames_RButton);
    
    QGroupBox *save_optionsGBox = new QGroupBox;
    save_optionsGBox->setLayout(save_options_VLayout);
    
    QPushButton *cancel_PButton = new QPushButton(tr("Cancel"));
    QPushButton *next_PButton = new QPushButton(tr("Next"));
    next_PButton->setDefault(true);
    connect(cancel_PButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(next_PButton, SIGNAL(clicked()), this, SLOT(next_button_clicked_slot()));

    QHBoxLayout *buttons_HLayout = new QHBoxLayout;
    buttons_HLayout->addStretch();
    buttons_HLayout->addWidget(cancel_PButton);
    buttons_HLayout->addWidget(next_PButton);
    
    QVBoxLayout *dialog_VLayout = new QVBoxLayout;
    dialog_VLayout->addWidget(save_frames_Label);
    dialog_VLayout->addWidget(save_optionsGBox);
    dialog_VLayout->addStretch();
    dialog_VLayout->addLayout(buttons_HLayout);
    
    setLayout(dialog_VLayout);
}


void c_save_frames_dialog::start_Spinbox_changed_slot(int value)
{
    if (value > mp_end_Spinbox->value()) {
        mp_end_Spinbox->setValue(value);
    }
}


void c_save_frames_dialog::end_Spinbox_changed_slot(int value)
{
    if (value < mp_start_Spinbox->value()) {
        mp_start_Spinbox->setValue(value);
    }
}


void c_save_frames_dialog::next_button_clicked_slot()
{
    if (mp_save_current_frame_RButton->isChecked()) {
        m_start_frame = -1;
        m_end_frame = -1;
    } else if (mp_save_all_frames_RButton->isChecked()) {
        m_start_frame = 1;
        m_end_frame = m_total_frames;
    } else if (mp_save_marked_frames_RButton->isChecked()) {
        m_start_frame = m_marker_start_frame;
        m_end_frame = m_marker_end_frame;
    } else { // mp_save_frame_range_RButton
        m_start_frame = mp_start_Spinbox->value();
        m_end_frame = mp_end_Spinbox->value();
    }

    accept();
}
