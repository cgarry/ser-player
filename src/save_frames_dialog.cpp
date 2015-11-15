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
                                           int frame_width,
                                           int frame_height,
                                           int total_frames,
                                           bool ser_has_timestamps,
                                           QString observer_string,
                                           QString instrument_string,
                                           QString telescope_string)
    : QDialog(parent),
      m_save_type(save_type),
      m_frame_width(frame_width),
      m_frame_height(frame_height),
      m_total_frames(total_frames),
      m_marker_start_frame(0),
      m_marker_end_frame(total_frames-1),
      m_ser_has_timestamps(ser_has_timestamps),
      m_start_frame(1),
      m_end_frame(total_frames),
      m_spin_boxes_valid(true)
{
    switch (save_type) {
    case SAVE_IMAGES:
        setWindowTitle(tr("Save Frames As Images", "Save frames dialog"));
        break;
    case SAVE_SER:
        setWindowTitle(tr("Save Frames As SER File", "Save frames dialog"));
        break;
    }

    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QDialog::setModal(true);


    //
    // Frames to save options
    //
    mp_save_current_frame_RButton = new QRadioButton(tr("Save Current Frame Only", "Save frames dialog"));
    connect(mp_save_current_frame_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
    mp_save_all_frames_RButton = new QRadioButton(tr("Save All %1 Frames", "Save frames dialog").arg(total_frames));
    connect(mp_save_all_frames_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));
    mp_save_marked_frames_RButton = new QRadioButton(tr("Start/End Markers Disabled", "Save frames dialog"));
    mp_save_marked_frames_RButton->setEnabled(false);
    connect(mp_save_marked_frames_RButton, SIGNAL(clicked()), this, SLOT(update_num_frames_slot()));

    mp_save_frame_range_RButton = new QRadioButton(tr("Save Frames From: ", "Save frames dialog"));
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
    custom_range_HLayout->addWidget(new QLabel(tr(" to ", "Save frames dialog")));
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
    
    QGroupBox *save_optionsGBox = new QGroupBox(tr("Select frames to save", "Save frames dialog"));
    save_optionsGBox->setLayout(save_range_VLayout);
    save_optionsGBox->setMinimumWidth((save_optionsGBox->minimumSizeHint().width() * 5) / 4);


    //---------------------
    // Save frames options
    //---------------------

    //
    // Frame Decimation
    //
    QLabel *frame_decimation_Label = new QLabel(tr("Keep 1 frame in every", "Save frames dialog"));
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

    mp_frame_decimation_GBox = new QGroupBox(tr("Enable Frame Decimation", "Save frames dialog"));
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
    mp_forwards_sequence_RButton = new QRadioButton(tr("Forwards", "Save frames dialog"));
    mp_forwards_sequence_RButton->setToolTip(tr("Save the frames in their forward order.") + "<b></b>");

    mp_reverse_sequence_RButton = new QRadioButton(tr("Reverse", "Save frames dialog"));
    mp_reverse_sequence_RButton->setToolTip(tr("Save the frames in their reverse order.") + "<b></b>");

    mp_forwards_then_reverse_sequence_RButton = new QRadioButton(tr("Forwards Then Reverse", "Save frames dialog"));
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

    mp_sequence_direction_GBox = new QGroupBox(tr("Sequence Direction", "Save frames dialog"));
    mp_sequence_direction_GBox->setLayout(sequence_direction_VLayout);
    mp_sequence_direction_GBox->setMinimumWidth((mp_sequence_direction_GBox->minimumSizeHint().width() * 5) / 4);


    //
    // Image processing
    //
    mp_processing_enable_CBox = new QCheckBox(tr("Apply Processing To Frames Before Saving", "Save frames dialog"));
    mp_processing_enable_CBox->setChecked(true);

    QVBoxLayout *processing_enable_VLayout = new QVBoxLayout;
    processing_enable_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    processing_enable_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    processing_enable_VLayout->addWidget(mp_processing_enable_CBox);

    mp_processing_GBox = new QGroupBox(tr("Image Processing", "Save frames dialog"));
    mp_processing_GBox->setLayout(processing_enable_VLayout);
    mp_processing_GBox->setToolTip(tr("This option controls whether active processing options are performed "
                                             "on the frames before saving.  If this option is disabled then "
                                             "the frames saved will be the original frames from the source SER file.  "
                                             "If it is enabled then any processing options that are active, such as "
                                             "debayering, gamma, gain, colour saturation or colour balance, will be "
                                             "applied to the frames before saving.") + "<b></b>");


    //
    //  Resize Frame
    //
    mp_resize_width_Spinbox = new QSpinBox;
    mp_resize_width_Spinbox->setRange(10, frame_width);
    mp_resize_width_Spinbox->setValue(frame_width);

    mp_resize_height_Spinbox = new QSpinBox;
    mp_resize_height_Spinbox->setRange(10, frame_height);
    mp_resize_height_Spinbox->setValue(frame_height);

    mp_resize_units_ComboBox = new QComboBox;
    mp_resize_units_ComboBox->addItem(tr("Pixels"));
    mp_resize_units_ComboBox->addItem(tr("Percent"));

    mp_resize_constrain_propotions_CBox = new QCheckBox(tr("Keep Original Aspect Ratio", "Resize Frames Control"));
    mp_resize_constrain_propotions_CBox->setChecked(true);

    mp_resize_add_black_bars_CBox = new QCheckBox(tr("Add Black Bars To Keep Original Aspert Ratio"));
    mp_resize_add_black_bars_CBox->setChecked(false);

    QGridLayout *resize_frame_GLayout = new QGridLayout;
    resize_frame_GLayout->addWidget(new QLabel(tr("Width:", "Resize Frames Control")), 0, 0);
    resize_frame_GLayout->addWidget(mp_resize_width_Spinbox, 0, 1);

    resize_frame_GLayout->addWidget(new QLabel(tr("Height:", "Resize Frames Control")), 0, 2);
    resize_frame_GLayout->addWidget(mp_resize_height_Spinbox, 0, 3);

    resize_frame_GLayout->addWidget(new QLabel(" "), 0, 4);

    resize_frame_GLayout->addWidget(mp_resize_units_ComboBox, 0, 5);

    resize_frame_GLayout->addWidget(new QLabel(" "), 0, 6);
    resize_frame_GLayout->setColumnStretch(6, 2);

    resize_frame_GLayout->addWidget(mp_resize_constrain_propotions_CBox, 1, 0, 1, 6);

    resize_frame_GLayout->addWidget(mp_resize_add_black_bars_CBox, 2, 0, 1, 6);

    mp_resize_GBox = new QGroupBox(tr("Resize Frames"));
    mp_resize_GBox->setCheckable(true);
    mp_resize_GBox->setChecked(false);
    mp_resize_GBox->setLayout(resize_frame_GLayout);

    connect(mp_resize_constrain_propotions_CBox, SIGNAL(toggled(bool)), this, SLOT(resize_control_handler()));
    connect(mp_resize_width_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(resize_control_handler()));
    connect(mp_resize_height_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(resize_control_handler()));
    connect(mp_resize_add_black_bars_CBox, SIGNAL(toggled(bool)), this, SLOT(resize_control_handler()));
    connect(mp_resize_units_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(resize_control_handler()));


    //
    // Filename Generation - Only for saving as image files
    //
    mp_use_framenumber_in_filename = new QCheckBox(tr("Use Framenumber In Filename (Instead Of A Sequential Count)"));
    mp_use_framenumber_in_filename->setChecked(true);
    mp_use_framenumber_in_filename->setToolTip(tr("This option controls whether generated image files will have their frame number "
                                                  "or a simple sequential count appended to their filename in order to make the "
                                                  "image filenames unique.") + "<b></b>");
    if (ser_has_timestamps) {
        mp_append_timestamp_CBox = new QCheckBox(tr("Append Frame Timestamp To Filename", "Save frames dialog"));
        mp_append_timestamp_CBox->setEnabled(true);
    } else {
        mp_append_timestamp_CBox = new QCheckBox(tr("Append Frame Timestamp To Filename", "Save frames dialog") + " (" + tr("No Frame Timestamps In SER File", "Save frames dialog") + ")");
        mp_append_timestamp_CBox->setEnabled(false);
    }

    mp_append_timestamp_CBox->setToolTip(tr("Append each image filename with its timestamp", "Save frames dialog") + "<b></b>");

    QVBoxLayout *filename_generation_VLayout = new QVBoxLayout;
    filename_generation_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    filename_generation_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    filename_generation_VLayout->addWidget(mp_use_framenumber_in_filename);
    filename_generation_VLayout->addWidget(mp_append_timestamp_CBox);

    QGroupBox *filename_generation_GBox = new QGroupBox(tr("Filename Generation", "Save frames dialog"));
    filename_generation_GBox->setLayout(filename_generation_VLayout);
    if (save_type == SAVE_SER) {
        filename_generation_GBox->hide();
        filename_generation_GBox->setFixedHeight(0);
    }


    //
    // SER file saving specific options
    //
    if (ser_has_timestamps) {
        mp_include_timestamps_CBox = new QCheckBox(tr("Include Frame Timestamps", "Save frames dialog"));
        mp_include_timestamps_CBox->setChecked(true);
    } else {
        mp_include_timestamps_CBox = new QCheckBox(tr("Include Frame Timestamps", "Save frames dialog") + " (" + tr("No Frame Timestamps In Source SER File", "Save frames dialog") + ")");
        mp_include_timestamps_CBox->setEnabled(false);
    }

    mp_include_timestamps_CBox->setToolTip(tr("Frame Timestamps are optional in SER Files."
                                         "  This option controls whether or not timestamps are included in the generated SER file") + "<b></b>");

    mp_utf8_validator = new c_utf8_validator;

    mp_observer_LEdit = new QLineEdit;
    mp_observer_LEdit->setText(observer_string.trimmed());
    mp_observer_LEdit->setValidator(mp_utf8_validator);
    mp_observer_LEdit->setMinimumWidth((2 * mp_observer_LEdit->sizeHint().width()));
    mp_observer_LEdit->setToolTip(tr("Set the text to be written to the 'Observer' string in the generated SER file") + "<b></b>");

    mp_instrument_LEdit = new QLineEdit;
    mp_instrument_LEdit->setText(instrument_string.trimmed());
    mp_instrument_LEdit->setValidator(mp_utf8_validator);
    mp_instrument_LEdit->setToolTip(tr("Set the text to be written to the 'Instrument' string in the generated SER file") + "<b></b>");

    mp_telescope_LEdit = new QLineEdit;
    mp_telescope_LEdit->setText(telescope_string.trimmed());
    mp_telescope_LEdit->setValidator(mp_utf8_validator);
    mp_telescope_LEdit->setToolTip(tr("Set the text to be written to the 'Telescope' string in the generated SER file") + "<b></b>");

    QGridLayout *header_fields_GLayout = new QGridLayout;
    header_fields_GLayout->addWidget(new QLabel(tr("Observer:", "Save frames dialog")), 0, 0);
    header_fields_GLayout->addWidget(mp_observer_LEdit, 0, 1);
    header_fields_GLayout->addWidget(new QLabel(tr("Instrument:", "Save frames dialog")), 1, 0);
    header_fields_GLayout->addWidget(mp_instrument_LEdit, 1, 1);
    header_fields_GLayout->addWidget(new QLabel(tr("Telescope:", "Save frames dialog")), 2, 0);
    header_fields_GLayout->addWidget(mp_telescope_LEdit, 2, 1);

    QGroupBox *header_fields_GBox = new QGroupBox(tr("SER Header Information Fields", "Save frames dialog"));
    header_fields_GBox->setLayout(header_fields_GLayout);

    QVBoxLayout *ser_file_options_VLayout = new QVBoxLayout;
    ser_file_options_VLayout->setMargin(INSIDE_GBOX_MARGIN);
    ser_file_options_VLayout->setSpacing(INSIDE_GBOX_SPACING);
    ser_file_options_VLayout->addWidget(mp_include_timestamps_CBox);
    ser_file_options_VLayout->addWidget(header_fields_GBox);

    QGroupBox *ser_file_options_GBox = new QGroupBox(tr("SER File Options", "Save frames dialog"));
    ser_file_options_GBox->setLayout(ser_file_options_VLayout);
    // Hide save current frame button when this is a save as SER file dilalog
    if (save_type == SAVE_IMAGES) {
        ser_file_options_GBox->hide();
        ser_file_options_GBox->setFixedHeight(0);
    }


    // List of group boxes to be displayed
    QList<QGroupBox *> groupbox_list;
    groupbox_list << save_optionsGBox;
    groupbox_list << mp_frame_decimation_GBox;
    groupbox_list << mp_sequence_direction_GBox;
    groupbox_list << mp_processing_GBox;
    groupbox_list << mp_resize_GBox;
    groupbox_list << filename_generation_GBox;
    groupbox_list << ser_file_options_GBox;


    //
    // Frames to be saved label
    //
    mp_total_frames_to_save_Label = new QLabel(tr("xxxx frames will be saved", "Save frames dialog"));


    //
    // Dialog buttons
    //
    QPushButton *cancel_PButton = new QPushButton(tr("Cancel", "Save frames dialog"));
    QPushButton *next_PButton = new QPushButton(tr("Next", "Save frames dialog"));
    cancel_PButton->setAutoDefault(false);
    next_PButton->setAutoDefault(false);
    connect(cancel_PButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(next_PButton, SIGNAL(clicked()), this, SLOT(next_button_clicked_slot()));

    QHBoxLayout *buttons_HLayout = new QHBoxLayout;
    buttons_HLayout->addStretch();
    buttons_HLayout->setSpacing(0);
    buttons_HLayout->addWidget(mp_total_frames_to_save_Label);
    buttons_HLayout->addSpacing(10);
    buttons_HLayout->addWidget(next_PButton);
    buttons_HLayout->addSpacing(10);
    buttons_HLayout->addWidget(cancel_PButton);

    QVBoxLayout *lhs_VLayout = new QVBoxLayout;
    lhs_VLayout->setMargin(0);
    lhs_VLayout->setSpacing(10);

    QVBoxLayout *rhs_VLayout = new QVBoxLayout;
    rhs_VLayout->setMargin(0);
    rhs_VLayout->setSpacing(lhs_VLayout->spacing());

    // Add groupboxes to left and right columns, roughly half the height to each
    int total_gbox_height = 0;
    for (int i = 0; i < groupbox_list.size(); i++) {
        if (groupbox_list.at(i)->height() != 0) {
            total_gbox_height += groupbox_list.at(i)->sizeHint().height() + lhs_VLayout->spacing();
        }
    }

    int current_gbox_height = 0;
    for (int i = 0; i < groupbox_list.size(); i++) {
        if (current_gbox_height < total_gbox_height / 2) {
            lhs_VLayout->addWidget(groupbox_list.at(i));
        } else {
            rhs_VLayout->addWidget(groupbox_list.at(i));
        }

        if (groupbox_list.at(i)->height() != 0) {
            current_gbox_height += groupbox_list.at(i)->sizeHint().height();
        }
    }

    lhs_VLayout->addStretch();
    rhs_VLayout->addStretch();

    QHBoxLayout *lhs_rhs_HLayout = new QHBoxLayout;
    lhs_rhs_HLayout->setMargin(0);
    lhs_rhs_HLayout->setSpacing(10);
    lhs_rhs_HLayout->addLayout(lhs_VLayout);
    lhs_rhs_HLayout->addLayout(rhs_VLayout);
    
    QVBoxLayout *dialog_VLayout = new QVBoxLayout;
    dialog_VLayout->setMargin(10);
    dialog_VLayout->setSpacing(10);
    dialog_VLayout->addLayout(lhs_rhs_HLayout);
    dialog_VLayout->addStretch();
    dialog_VLayout->addLayout(buttons_HLayout);

    setLayout(dialog_VLayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // Ensure mp_num_frames_Label is set and
    if (save_type == SAVE_IMAGES) {
        mp_save_current_frame_RButton->click();
    } else {
        mp_save_all_frames_RButton->click();
    }

    resize_control_handler();
}


c_save_frames_dialog::~c_save_frames_dialog()
{
    delete mp_utf8_validator;
}


void c_save_frames_dialog::set_markers(int marker_start_frame,
                 int marker_end_frame,
                 bool markers_enabled)
{
    m_marker_start_frame = marker_start_frame;
    m_marker_end_frame = marker_end_frame;

    if (markers_enabled) {
        // Markers are selected
        mp_save_marked_frames_RButton->setText(tr("Save Frames Selected By Start/End Markers (%1 to %2)", "Save frames dialog")
                                                  .arg(marker_start_frame).arg(marker_end_frame));
        mp_save_marked_frames_RButton->setEnabled(true);
        mp_save_marked_frames_RButton->click();
    } else {
        // No markers enabled
        mp_save_marked_frames_RButton->setText(tr("Start/End Markers Disabled", "Save frames dialog"));
        mp_save_marked_frames_RButton->setEnabled(false);
        if (m_save_type == SAVE_IMAGES) {
            mp_save_current_frame_RButton->click();
        } else {
            mp_save_all_frames_RButton->click();
        }
    }
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
    } else {
        mp_use_framenumber_in_filename->setEnabled(false);
    }

    if (!mp_sequence_direction_GBox->isEnabled() || mp_forwards_sequence_RButton->isChecked()) {
        // Give option to include timestamps in SER file
        mp_include_timestamps_CBox->setEnabled(m_ser_has_timestamps);
    } else {
        // Timestamps make no sense when frames are not in their natural order
        mp_include_timestamps_CBox->setEnabled(false);
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


void c_save_frames_dialog::resize_control_handler()
{
    static int resize_units_ComboBox = -1;
    static bool resize_constrain_propotions = false;
    static bool resize_add_black_bars = false;
    static int resize_width = 0;
    static int resize_height = 0;

    if (resize_units_ComboBox !=  mp_resize_units_ComboBox->currentIndex()) {
        resize_units_ComboBox = mp_resize_units_ComboBox->currentIndex();
        // User has changed pixels/percent selection
        if (mp_resize_units_ComboBox->currentIndex() == 0 && mp_resize_width_Spinbox->minimum() != 10) {
            // Selection changed from percent to pixels
            int new_width = (m_frame_width * mp_resize_width_Spinbox->value()) / 100;
            int new_height = (m_frame_height * mp_resize_height_Spinbox->value()) / 100;
            mp_resize_width_Spinbox->setRange(10, m_frame_width);
            mp_resize_height_Spinbox->setRange(10, m_frame_height);
            mp_resize_width_Spinbox->setSuffix("");
            mp_resize_height_Spinbox->setSuffix("");
            mp_resize_width_Spinbox->setValue(new_width);
            mp_resize_height_Spinbox->setValue(new_height);
        } else if (mp_resize_units_ComboBox->currentIndex() == 1 && mp_resize_width_Spinbox->minimum() != 1) {
            // Selection changed from pixels to percent
            int new_width = (100 * mp_resize_width_Spinbox->value()) / m_frame_width;
            int new_height = (100 * mp_resize_height_Spinbox->value()) / m_frame_height;
            mp_resize_width_Spinbox->setRange(1, 100);
            mp_resize_height_Spinbox->setRange(1, 100);
            mp_resize_width_Spinbox->setSuffix("%");
            mp_resize_height_Spinbox->setSuffix("%");
            mp_resize_width_Spinbox->setValue(new_width);
            mp_resize_height_Spinbox->setValue(new_height);
        }
    }

    bool change_height_to_match_aspect_ratio = false;
    bool percent_mode = mp_resize_units_ComboBox->currentIndex() == 1;

    if (resize_constrain_propotions != mp_resize_constrain_propotions_CBox->isChecked()) {
        resize_constrain_propotions = mp_resize_constrain_propotions_CBox->isChecked();
        // User has toggled mp_resize_constrain_propotions_CBox
        mp_resize_add_black_bars_CBox->setEnabled(mp_resize_constrain_propotions_CBox->isChecked());
        change_height_to_match_aspect_ratio = true;
    }

    if (resize_add_black_bars !=  mp_resize_add_black_bars_CBox->isChecked()) {
        resize_add_black_bars = mp_resize_add_black_bars_CBox->isChecked();
        // User has toggled mp_resize_add_black_bars_CBox
        if (!mp_resize_add_black_bars_CBox->isChecked()) {
            change_height_to_match_aspect_ratio = true;
        }
    }

    if ((resize_width != mp_resize_width_Spinbox->value()) || change_height_to_match_aspect_ratio) {
        resize_width = mp_resize_width_Spinbox->value();
        // User has changed mp_resize_width_Spinbox value
        if (mp_resize_constrain_propotions_CBox->isChecked() && !mp_resize_add_black_bars_CBox->isChecked()) {
            if (percent_mode) {
                int new_height = mp_resize_width_Spinbox->value();
                resize_height = new_height;
                mp_resize_height_Spinbox->setValue(new_height);
            } else {
                // Keep aspect ratio constant
                int new_height = (mp_resize_width_Spinbox->value() * m_frame_height) / m_frame_width;
                resize_height = new_height;
                mp_resize_height_Spinbox->setValue(new_height);
            }
        }
    }

    if (resize_height != mp_resize_height_Spinbox->value()) {
        resize_height = mp_resize_height_Spinbox->value();
        // User has changed mp_resize_height_Spinbox value
        if (mp_resize_constrain_propotions_CBox->isChecked() && !mp_resize_add_black_bars_CBox->isChecked()) {
            if (percent_mode) {
                int new_width = mp_resize_height_Spinbox->value();
                resize_width = new_width;
                mp_resize_width_Spinbox->setValue(new_width);
            } else {
                // Keep aspect ratio constant
                int new_width = (mp_resize_height_Spinbox->value() * m_frame_width) / m_frame_height;
                resize_width = new_width;
                mp_resize_width_Spinbox->setValue(new_width);
            }
        }
    }
}


int c_save_frames_dialog::get_active_width()
{
    int width = get_total_width();
    if (mp_resize_add_black_bars_CBox->isEnabled() && mp_resize_add_black_bars_CBox->isChecked()) {
        // Calculate width for current height
        int calc_width = (m_frame_width * get_total_height()) / m_frame_height;
        int calc_height = (m_frame_height * get_total_width()) / m_frame_width;
        if (calc_width != width && calc_height != get_total_height()) {
            if (width > calc_width) {
                width = calc_width;
            }
        }
    }

    return width;
}


int c_save_frames_dialog::get_active_height()
{
    int height = get_total_height();
    if (mp_resize_add_black_bars_CBox->isEnabled() && mp_resize_add_black_bars_CBox->isChecked()) {
        // Calculate height for current width
        int calc_width = (m_frame_width * get_total_height()) / m_frame_height;
        int calc_height = (m_frame_height * get_total_width()) / m_frame_width;
        if (calc_width != get_total_width() && calc_height != height) {
            if (height > calc_height) {
                height = calc_height;
            }
        }
    }

    return height;
}


int c_save_frames_dialog::get_total_width()
{
    int width = m_frame_width;  // No resize by default
    if (mp_resize_GBox->isChecked()) {
        // Resize frames enabled
        if (mp_resize_units_ComboBox->currentIndex() == 0) {
            // Spinbox values are pixels
            width =  mp_resize_width_Spinbox->value();
        } else {
            // Spinbox values are percent
            width =  (mp_resize_width_Spinbox->value() * m_frame_width) / 100;
        }
    }

    return width;
}


int c_save_frames_dialog::get_total_height()
{
    int height = m_frame_height;  // No resize by default
    if (mp_resize_GBox->isChecked()) {
        // Resize frames enabled
        if (mp_resize_units_ComboBox->currentIndex() == 0) {
            // Spinbox values are pixels
            height =  mp_resize_height_Spinbox->value();
        } else {
            // Spinbox values are percent
            height =  (mp_resize_height_Spinbox->value() * m_frame_height) / 100;
        }
    }

    return height;
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


bool c_save_frames_dialog::get_processing_enable()
{
    bool ret = false;
    if (mp_processing_enable_CBox->isEnabled() && mp_processing_enable_CBox->isChecked())
    {
        ret = true;
    }

    return ret;
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
    bool include_timestamps_in_ser_file = mp_include_timestamps_CBox->isChecked();
    if (!mp_include_timestamps_CBox->isEnabled()) {
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

