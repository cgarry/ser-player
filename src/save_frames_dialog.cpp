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
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <cmath>

#include "save_frames_dialog.h"

#define INSIDE_GBOX_SPACING 8
#define INSIDE_GBOX_MARGIN 10


c_save_frames_dialog::c_save_frames_dialog(QWidget *parent,
                                           int total_frames,
                                           int marker_start_frame,
                                           int marker_end_frame,
                                           bool markers_enabled,
                                           bool ser_has_timestamps)
    : QDialog(parent),
//    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint),
      m_total_frames(total_frames),
      m_marker_start_frame(marker_start_frame),
      m_marker_end_frame(marker_end_frame),
      m_start_frame(1),
      m_end_frame(total_frames)
{
    setWindowTitle(tr("Save Frames As Images"));
    QDialog::setModal(true);
    

    //
    // Frames to save options
    //

    mp_save_current_frame_RButton = new QRadioButton(tr("Save Current Frame Only"));
    connect(mp_save_current_frame_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
    mp_save_all_frames_RButton = new QRadioButton(tr("Save All %1 Frames").arg(total_frames));
    connect(mp_save_all_frames_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
    if (!markers_enabled) {
        // No markers enabled
        mp_save_marked_frames_RButton = new QRadioButton(tr("Start/End Markers Disabled"));
        mp_save_marked_frames_RButton->setEnabled(false);
    } else {
        mp_save_marked_frames_RButton = new QRadioButton(tr("Save Frames Selected By Start/End Markers (%1 to %2)")
                                                         .arg(marker_start_frame).arg(marker_end_frame));
        connect(mp_save_marked_frames_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
        // Markers are selected
    }

    mp_save_frame_range_RButton = new QRadioButton(tr("Save Frames From: "));
    connect(mp_save_frame_range_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));

    // Select the default frame selection option depending on whether of not the markers are enabled
    if (markers_enabled) {
        mp_save_marked_frames_RButton->setChecked(true);
    } else {
        mp_save_current_frame_RButton->setChecked(true);
    }

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

    mp_selected_frames_Label = new QLabel;

    QHBoxLayout *save_current_HLayout = new QHBoxLayout;
    save_current_HLayout->setMargin(0);
    save_current_HLayout->setSpacing(0);
    save_current_HLayout->addWidget(mp_save_current_frame_RButton, 0, Qt::AlignLeft);
    save_current_HLayout->addStretch(0);

    QHBoxLayout *save_marked_HLayout = new QHBoxLayout;
    save_marked_HLayout->setMargin(0);
    save_marked_HLayout->setSpacing(0);
    save_marked_HLayout->addWidget(mp_save_marked_frames_RButton, 0, Qt::AlignLeft);
    save_marked_HLayout->addStretch(0);

    QHBoxLayout *custom_range_HLayout = new QHBoxLayout;
    custom_range_HLayout->setMargin(0);
    custom_range_HLayout->setSpacing(0);
    custom_range_HLayout->addWidget(mp_save_frame_range_RButton, 0, Qt::AlignLeft);
    custom_range_HLayout->addWidget(mp_start_Spinbox);
    custom_range_HLayout->addWidget(new QLabel(tr(" to ")));
    custom_range_HLayout->addWidget(mp_end_Spinbox);
    custom_range_HLayout->addStretch(0);

    QHBoxLayout *save_all_HLayout = new QHBoxLayout;
    save_all_HLayout->setMargin(0);
    save_all_HLayout->setSpacing(0);
    save_all_HLayout->addWidget(mp_save_all_frames_RButton, 0, Qt::AlignLeft);
    save_all_HLayout->addStretch(0);
    
    QVBoxLayout *save_range_VLayout = new QVBoxLayout;
    save_range_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    save_range_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    save_range_VLayout->addLayout(save_current_HLayout);
    save_range_VLayout->addLayout(save_marked_HLayout);
    save_range_VLayout->addLayout(custom_range_HLayout);
    save_range_VLayout->addLayout(save_all_HLayout);
    save_range_VLayout->addWidget(mp_selected_frames_Label, 0, Qt::AlignRight);
    
    QGroupBox *save_optionsGBox = new QGroupBox(tr("Select frames to save"));
    save_optionsGBox->setLayout(save_range_VLayout);
    save_optionsGBox->setMinimumWidth((save_optionsGBox->minimumSizeHint().width() * 5) / 4);


    //
    // Save frames options
    //

    // Frame Decimation
    QLabel *frame_decimation_Label = new QLabel(tr("Keep 1 frame in every"));
    mp_frame_decimation_SpinBox = new QSpinBox;
    mp_frame_decimation_SpinBox->setMinimum(1);
    mp_frame_decimation_SpinBox->setMaximum(total_frames);
    mp_frame_decimation_SpinBox->setValue(2);
    connect(mp_frame_decimation_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(update_num_frames_slot()));

    QHBoxLayout *frame_decimation_HLayout = new QHBoxLayout;
    frame_decimation_HLayout->setMargin(INSIDE_GBOX_MARGIN);
    frame_decimation_HLayout->setSpacing(INSIDE_GBOX_SPACING);
    frame_decimation_HLayout->addWidget(frame_decimation_Label);
    frame_decimation_HLayout->addWidget(mp_frame_decimation_SpinBox);
    frame_decimation_HLayout->addStretch();

    mp_frame_decimation_GBox = new QGroupBox(tr("Enable Frame Decimation"));
    mp_frame_decimation_GBox->setCheckable(true);
    mp_frame_decimation_GBox->setChecked(false);
    mp_frame_decimation_GBox->setLayout(frame_decimation_HLayout);
    mp_frame_decimation_GBox->setMinimumWidth((mp_frame_decimation_GBox->minimumSizeHint().width() * 5) / 4);
    connect(mp_frame_decimation_GBox, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));


    // Sequence Direction
    mp_forwards_sequence_RButton = new QRadioButton(tr("Forwards"));
    mp_reverse_sequence_RButton = new QRadioButton(tr("Reverse"));
    mp_forwards_then_reverse_sequence_RButton = new QRadioButton(tr("Forwards Then Reverse"));
    mp_forwards_sequence_RButton->setChecked(true);
    connect(mp_forwards_sequence_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
    connect(mp_reverse_sequence_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
    connect(mp_forwards_then_reverse_sequence_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));

    QVBoxLayout *sequence_direction_VLayout = new QVBoxLayout;
    sequence_direction_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    sequence_direction_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    sequence_direction_VLayout->addWidget(mp_forwards_sequence_RButton);
    sequence_direction_VLayout->addWidget(mp_reverse_sequence_RButton);
    sequence_direction_VLayout->addWidget(mp_forwards_then_reverse_sequence_RButton);

    mp_sequence_direction_GBox = new QGroupBox(tr("Sequence Direction"));
    mp_sequence_direction_GBox->setLayout(sequence_direction_VLayout);
    mp_sequence_direction_GBox->setMinimumWidth((mp_sequence_direction_GBox->minimumSizeHint().width() * 5) / 4);


    // Filename Generation
    if (ser_has_timestamps) {
        mp_append_timestamp_CBox = new QCheckBox(tr("Append Timestamp To Filename"));
        mp_append_timestamp_CBox->setEnabled(true);
    } else {
        mp_append_timestamp_CBox = new QCheckBox(tr("Append Timestamp To Filename") + " (" + tr("No Timestamps In SER File") + ")");
        mp_append_timestamp_CBox->setEnabled(false);
    }

    QVBoxLayout *filename_generation_VLayout = new QVBoxLayout;
    filename_generation_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    filename_generation_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    filename_generation_VLayout->addWidget(mp_append_timestamp_CBox);

    QGroupBox *filename_generation_GBox = new QGroupBox(tr("Filename Generation"));
    filename_generation_GBox->setLayout(filename_generation_VLayout);
//    filename_generation_GBox->setMinimumWidth((filename_generation_GBox->minimumSizeHint().width() * 5) / 4);


    //
    // Frames to be saved label
    //
    mp_total_frames_to_save_Label = new QLabel(tr("xxxx frames will be saved"));


    //
    // Dialog buttons
    //

    QPushButton *cancel_PButton = new QPushButton(tr("Cancel"));
    QPushButton *next_PButton = new QPushButton(tr("Next"));
    next_PButton->setDefault(true);
    connect(cancel_PButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(next_PButton, SIGNAL(clicked()), this, SLOT(next_button_clicked_slot()));

    QHBoxLayout *buttons_HLayout = new QHBoxLayout;
    buttons_HLayout->addStretch();
    buttons_HLayout->setSpacing(0);
    buttons_HLayout->addWidget(cancel_PButton);
    buttons_HLayout->addSpacing(10);
    buttons_HLayout->addWidget(next_PButton);
    
    QVBoxLayout *dialog_VLayout = new QVBoxLayout;
    dialog_VLayout->setMargin(10);
    dialog_VLayout->setSpacing(0);
    dialog_VLayout->addWidget(save_optionsGBox);
    dialog_VLayout->addSpacing(10);
    dialog_VLayout->addWidget(mp_frame_decimation_GBox);
    dialog_VLayout->addSpacing(10);
    dialog_VLayout->addWidget(mp_sequence_direction_GBox);
    dialog_VLayout->addSpacing(10);
    dialog_VLayout->addWidget(filename_generation_GBox);
    dialog_VLayout->addSpacing(15);
    dialog_VLayout->addWidget(mp_total_frames_to_save_Label, 0, Qt::AlignRight);
    dialog_VLayout->addSpacing(15);
    dialog_VLayout->addStretch();
    dialog_VLayout->addLayout(buttons_HLayout);
    
    setLayout(dialog_VLayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // Ensure mp_num_frames_Label is set
    if (markers_enabled) {
        mp_save_marked_frames_RButton->click();
    } else {
        mp_save_current_frame_RButton->click();
    }
}


void c_save_frames_dialog::start_Spinbox_changed_slot(int value)
{
    if (value > mp_end_Spinbox->value()) {
        mp_end_Spinbox->setValue(value);
    }

    update_num_frames_slot();
}


void c_save_frames_dialog::end_Spinbox_changed_slot(int value)
{
    if (value < mp_start_Spinbox->value()) {
        mp_start_Spinbox->setValue(value);
    }

    update_num_frames_slot();
}


void c_save_frames_dialog::update_num_frames_slot()
{
    if (mp_save_current_frame_RButton->isChecked()) {
        m_total_selected_frames = 1;
        mp_selected_frames_Label->setText(tr("1 frame selected"));
    } else if (mp_save_all_frames_RButton->isChecked()) {
        m_total_selected_frames = m_total_frames;
        mp_selected_frames_Label->setText(tr("%1 frames selected").arg(m_total_selected_frames));
    } else if (mp_save_marked_frames_RButton->isChecked()) {
        m_total_selected_frames = m_marker_end_frame - m_marker_start_frame + 1;
        mp_selected_frames_Label->setText(tr("%1 frames selected").arg(m_total_selected_frames));
    } else { // mp_save_frame_range_RButton
        m_total_selected_frames = mp_end_Spinbox->value() - mp_start_Spinbox->value() + 1;
        mp_selected_frames_Label->setText(tr("%1 frames selected").arg(m_total_selected_frames));
    }

    if (m_total_selected_frames == 1) {
        mp_frame_decimation_GBox->setEnabled(false);
        mp_sequence_direction_GBox->setEnabled(false);
    } else {
        mp_frame_decimation_GBox->setEnabled(true);
        mp_sequence_direction_GBox->setEnabled(true);
    }

    if (get_frames_to_be_saved() == 1) {
        mp_total_frames_to_save_Label->setText(tr("1 frame will be saved"));
    } else {
        mp_total_frames_to_save_Label->setText(tr("%1 frames will be saved").arg(get_frames_to_be_saved()));
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


int c_save_frames_dialog::get_frame_decimation()
{
    int decimate_value = 1;
    if (mp_frame_decimation_GBox->isEnabled() && mp_frame_decimation_GBox->isChecked()) {
        decimate_value = mp_frame_decimation_SpinBox->value();
    }

    return decimate_value;
}


int c_save_frames_dialog::get_sequence_direction()
{
    int direction = 0;
    if (mp_sequence_direction_GBox->isEnabled()) {
        if (mp_forwards_sequence_RButton->isChecked()) {
            // Forwards sequence
            direction = 0;
        } else if (mp_reverse_sequence_RButton->isChecked()) {
            // Reverse sequence
            direction = 1;
        } else {
            // Forwards then reverse sequence
            direction = 2;
        }
    }

    return direction;
}


bool c_save_frames_dialog::get_append_timestamp_to_filename()
{
    return mp_append_timestamp_CBox->isEnabled() & mp_append_timestamp_CBox->isChecked();
}


int c_save_frames_dialog::get_frames_to_be_saved()
{
    int decimate_value = (mp_frame_decimation_GBox->isChecked()) ? get_frame_decimation() : 1;
    int frames_to_be_saved  = (m_total_selected_frames + decimate_value - 1) / decimate_value;
    if (get_sequence_direction() == 2) {
        frames_to_be_saved *= 2;
    }

    return frames_to_be_saved;
}


int c_save_frames_dialog::get_number_of_leading_zeros()
{
    int leading_zeros = -1;  // Can't calculate number of zeros required
    if (mp_forwards_sequence_RButton->isChecked()) {
        // File numbering is using frame numbers, allow enough zeros for the maximum
        // frame number of the subsequently saved sequences of images will have the same
        // number of leading zeros
        leading_zeros = (int)ceil(log10((double)m_total_frames));
    } else {
        // File numbering is not using actual frame numbers, so just allow enough zeros
        // for the count being used to save this sequence
        leading_zeros = (int)ceil(log10((double)get_frames_to_be_saved()));
    }

    return leading_zeros;
}



