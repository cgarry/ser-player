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
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <cmath>

#include "save_frames_dialog.h"
#include "utf8_validator.h"

#define INSIDE_GBOX_SPACING 8
#define INSIDE_GBOX_MARGIN 10


c_save_frames_dialog::c_save_frames_dialog(QWidget *parent,
                                           e_save_type save_type,
                                           int total_frames,
                                           int marker_start_frame,
                                           int marker_end_frame,
                                           bool markers_enabled,
                                           bool ser_has_timestamps,
                                           QString observer_string,
                                           QString instrument_string,
                                           QString telescope_string)
    : QDialog(parent),
      m_total_frames(total_frames),
      m_marker_start_frame(marker_start_frame),
      m_marker_end_frame(marker_end_frame),
      m_start_frame(1),
      m_end_frame(total_frames),
      m_spin_boxes_valid(true)
{
    switch (save_type) {
    case SAVE_IMAGES:
        setWindowTitle(tr("Save Frames As Images"));
        break;
    case SAVE_SER:
        setWindowTitle(tr("Save Frames As SER File"));
        break;
    }

    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);
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

    // Hide save current frame button when this is a save as SER file dilalog
    if (save_type == SAVE_SER) {
        mp_save_current_frame_RButton->hide();
    }

    mp_start_Spinbox = new QSpinBox;
    mp_start_Spinbox->setMinimum(1);
    mp_start_Spinbox->setMaximum(total_frames);
    mp_start_Spinbox->setValue(1);
    connect(mp_start_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_changed_slot()));
    mp_end_Spinbox = new QSpinBox;
    mp_end_Spinbox->setMinimum(1);
    mp_end_Spinbox->setMaximum(total_frames);
    mp_end_Spinbox->setValue(total_frames);
    connect(mp_end_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_changed_slot()));

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


    //---------------------
    // Save frames options
    //---------------------

    //
    // Frame Decimation
    //
    QLabel *frame_decimation_Label = new QLabel(tr("Keep 1 frame in every"));
    frame_decimation_Label->setToolTip(tr("Reduce the number of saved frames by only saving 1 frame"
                                          " for every specified number of frames.") + "<b></b>");

    mp_frame_decimation_SpinBox = new QSpinBox;
    mp_frame_decimation_SpinBox->setMinimum(1);
    mp_frame_decimation_SpinBox->setMaximum(total_frames);
    mp_frame_decimation_SpinBox->setValue(2);
    connect(mp_frame_decimation_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(update_num_frames_slot()));
    mp_frame_decimation_SpinBox->setToolTip(tr("Reduce the number of saved frames by only saving 1 frame"
                                               " for every specified number of frames.") + "<b></b>");

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
    mp_frame_decimation_GBox->setToolTip(tr("Reduce the number of saved frames by only saving 1 frame"
                                            " for every specified number of frames.") + "<b></b>");

    //
    // Sequence Direction
    //
    mp_forwards_sequence_RButton = new QRadioButton(tr("Forwards"));
    mp_forwards_sequence_RButton->setToolTip(tr("Save the frames in their forward order.") + "<b></b>");

    mp_reverse_sequence_RButton = new QRadioButton(tr("Reverse"));
    mp_reverse_sequence_RButton->setToolTip(tr("Save the frames in their reverse order.") + "<b></b>");

    mp_forwards_then_reverse_sequence_RButton = new QRadioButton(tr("Forwards Then Reverse"));
    mp_forwards_then_reverse_sequence_RButton->setToolTip(tr("Save the frames in their forward order"
                                                             " and then in their reverse order. "
                                                             "This will result in twice as many frames being saved.") + "<b></b>");
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

    //
    // Filename Generation - Only for saving as image files
    //
    mp_use_framenumber_in_filename = new QCheckBox(tr("Use Framenumber In Filename (Instead Of A Sequential Count)"));
    mp_use_framenumber_in_filename->setChecked(true);
    mp_use_framenumber_in_filename->setToolTip(tr("This option controls whether generated image files will have their frame number "
                                                  "or a simple sequential count appended to their filename in order to make the "
                                                  "image filenames unique.") + "<b></b>");
    if (ser_has_timestamps) {
        mp_append_timestamp_CBox = new QCheckBox(tr("Append Timestamp To Filename"));
        mp_append_timestamp_CBox->setEnabled(true);
    } else {
        mp_append_timestamp_CBox = new QCheckBox(tr("Append Timestamp To Filename") + " (" + tr("No Timestamps In SER File") + ")");
        mp_append_timestamp_CBox->setEnabled(false);
    }

    mp_append_timestamp_CBox->setToolTip(tr("Append each image filename with its timestamp") + "<b></b>");

    QVBoxLayout *filename_generation_VLayout = new QVBoxLayout;
    filename_generation_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    filename_generation_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    filename_generation_VLayout->addWidget(mp_use_framenumber_in_filename);
    filename_generation_VLayout->addWidget(mp_append_timestamp_CBox);

    QGroupBox *filename_generation_GBox = new QGroupBox(tr("Filename Generation"));
    filename_generation_GBox->setLayout(filename_generation_VLayout);
    if (save_type == SAVE_SER) {
        filename_generation_GBox->hide();
    }

    //
    // SER file saving specific options
    //
    if (ser_has_timestamps) {
        mp_include_timestamps = new QCheckBox(tr("Include Timestamps"));
        mp_include_timestamps->setChecked(true);
    } else {
        mp_include_timestamps = new QCheckBox(tr("Include Timestamps") + " (" + tr("No Timestamps In Source SER File") + ")");
        mp_include_timestamps->setEnabled(false);
    }

    mp_include_timestamps->setToolTip(tr("Timestamps are optional in SER Files."
                                         "  This option controls whether or not timestamps are included in the generated SER file") + "<b></b>");

    mp_utf8_validator = new c_utf8_validator;

    mp_observer_LEdit = new QLineEdit;
    mp_observer_LEdit->setText(observer_string.trimmed());
    mp_observer_LEdit->setValidator(mp_utf8_validator);
    mp_observer_LEdit->setToolTip(tr("Set the text to be written to the 'Observer' string in the generated SER file") + "<b></b>");

    mp_instrument_LEdit = new QLineEdit;
    mp_instrument_LEdit->setText(instrument_string.trimmed());
    mp_instrument_LEdit->setValidator(mp_utf8_validator);
    mp_instrument_LEdit->setToolTip(tr("Set the text to be written to the 'Instrument' string in the generated SER file") + "<b></b>");

    mp_telescope_LEdit = new QLineEdit;
    mp_telescope_LEdit->setText(telescope_string.trimmed());
    mp_telescope_LEdit->setValidator(mp_utf8_validator);
    mp_telescope_LEdit->setToolTip(tr("Set the text to be written to the 'Telescope' string in the generated SER file") + "<b></b>");

    QFormLayout  *header_fields_FLayout = new QFormLayout;
    header_fields_FLayout->setVerticalSpacing(5);
    header_fields_FLayout->addRow(tr("Observer:"), mp_observer_LEdit);
    header_fields_FLayout->addRow(tr("Instrument:"), mp_instrument_LEdit);
    header_fields_FLayout->addRow(tr("Telescope:"), mp_telescope_LEdit);

    QVBoxLayout *ser_file_options_VLayout = new QVBoxLayout;
    ser_file_options_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    ser_file_options_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    ser_file_options_VLayout->addWidget(mp_include_timestamps);
    ser_file_options_VLayout->addLayout(header_fields_FLayout);

    QGroupBox *ser_file_options_GBox = new QGroupBox(tr("SER File Options"));
    ser_file_options_GBox->setLayout(ser_file_options_VLayout);
    // Hide save current frame button when this is a save as SER file dilalog
    if (save_type == SAVE_IMAGES) {
        ser_file_options_GBox->hide();
    }


    //
    // Frames to be saved label
    //
    mp_total_frames_to_save_Label = new QLabel(tr("xxxx frames will be saved"));


    //
    // Dialog buttons
    //

    QPushButton *cancel_PButton = new QPushButton(tr("Cancel"));
    QPushButton *next_PButton = new QPushButton(tr("Next"));
    cancel_PButton->setAutoDefault(false);
    next_PButton->setAutoDefault(false);
    connect(cancel_PButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(next_PButton, SIGNAL(clicked()), this, SLOT(next_button_clicked_slot()));

    QHBoxLayout *buttons_HLayout = new QHBoxLayout;
    buttons_HLayout->addStretch();
    buttons_HLayout->setSpacing(0);
    buttons_HLayout->addWidget(next_PButton);
    buttons_HLayout->addSpacing(10);
    buttons_HLayout->addWidget(cancel_PButton);
    
    QVBoxLayout *dialog_VLayout = new QVBoxLayout;
    dialog_VLayout->setMargin(10);
    dialog_VLayout->setSpacing(10);
    dialog_VLayout->addWidget(save_optionsGBox);
    dialog_VLayout->addWidget(mp_frame_decimation_GBox);
    dialog_VLayout->addWidget(mp_sequence_direction_GBox);
    dialog_VLayout->addWidget(filename_generation_GBox);
    dialog_VLayout->addWidget(ser_file_options_GBox);
    dialog_VLayout->addSpacing(5);
    dialog_VLayout->addWidget(mp_total_frames_to_save_Label, 0, Qt::AlignRight);
    dialog_VLayout->addSpacing(5);
    dialog_VLayout->addStretch();
    dialog_VLayout->addLayout(buttons_HLayout);
    
    setLayout(dialog_VLayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // Ensure mp_num_frames_Label is set and
    // Select the default frame selection option depending on whether of not the markers are enabled
    if (markers_enabled) {
        mp_save_marked_frames_RButton->click();
    } else {
        if (save_type == SAVE_IMAGES) {
            mp_save_current_frame_RButton->click();
        } else {
            mp_save_all_frames_RButton->click();
        }
    }
}


c_save_frames_dialog::~c_save_frames_dialog()
{
    delete mp_utf8_validator;
}


void c_save_frames_dialog::spinbox_changed_slot()
{
    QPalette text_Palette;
    if (mp_start_Spinbox->value() > mp_end_Spinbox->value()) {
        // Invalid values
        text_Palette.setColor(QPalette::Text,Qt::red);
        m_spin_boxes_valid = false;
    } else {
        text_Palette.setColor(QPalette::Text,Qt::black);
        m_spin_boxes_valid = true;
    }

    mp_start_Spinbox->setPalette(text_Palette);
    mp_end_Spinbox->setPalette(text_Palette);
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
        if (m_spin_boxes_valid) {
            m_total_selected_frames = mp_end_Spinbox->value() - mp_start_Spinbox->value() + 1;
        } else {
            m_total_selected_frames = 0;
        }

        mp_selected_frames_Label->setText(tr("%1 frames selected").arg(m_total_selected_frames));
    }

    if (m_total_selected_frames == 1) {
        mp_frame_decimation_GBox->setEnabled(false);
        mp_sequence_direction_GBox->setEnabled(false);
    } else {
        mp_frame_decimation_GBox->setEnabled(true);
        mp_sequence_direction_GBox->setEnabled(true);
    }

    if (!mp_save_current_frame_RButton->isChecked() &&
        (!mp_sequence_direction_GBox->isEnabled() || mp_forwards_sequence_RButton->isChecked())) {
        // Use frame number rather than a sequential count in filename
        mp_use_framenumber_in_filename->setEnabled(true);
        // Give option to include timestamps in SER file
        mp_include_timestamps->setEnabled(true);
    } else {
        mp_use_framenumber_in_filename->setEnabled(false);
        // Timestamps make to sense when frames are not in their natural order
        mp_include_timestamps->setEnabled(false);
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

    if (m_total_selected_frames > 0) {
        accept();
    }
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


int c_save_frames_dialog::get_required_digits_for_number()
{
    int digits;  // Can't calculate number of zeros required
    if (mp_use_framenumber_in_filename->isEnabled() && mp_use_framenumber_in_filename->isChecked()) {
        // File numbering is using frame numbers, allow enough zeros for the maximum
        // frame number of the subsequently saved sequences of images will have the same
        // number of leading zeros
        digits = (int)ceil(log10((double)(m_total_frames + 1)));
    } else {
        // File numbering is not using actual frame numbers, so just allow enough zeros
        // for the count being used to save this sequence
        digits = (int)ceil(log10((double)(get_frames_to_be_saved() + 1)));
    }

    return digits;
}


bool c_save_frames_dialog::get_use_framenumber_in_name()
{
    bool use_framenumber_in_name = false;
    if (mp_use_framenumber_in_filename->isEnabled() && mp_use_framenumber_in_filename->isChecked()) {
        use_framenumber_in_name = true;
    }

    return use_framenumber_in_name;
}


bool c_save_frames_dialog::get_include_timestamps_in_ser_file()
{
    bool include_timestamps_in_ser_file = mp_include_timestamps->isChecked();
    if (!mp_include_timestamps->isEnabled()) {
        include_timestamps_in_ser_file = false;
    }

    return include_timestamps_in_ser_file;
}

QString c_save_frames_dialog::get_observer_string()
{
    return mp_observer_LEdit->text();
}


QString c_save_frames_dialog::get_instrument_string()
{
    return mp_instrument_LEdit->text();
}


QString c_save_frames_dialog::get_telescope_string()
{
    return mp_telescope_LEdit->text();
}

