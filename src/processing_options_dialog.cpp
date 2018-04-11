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
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "processing_options_dialog.h"
#include "persistent_data.h"
#include "icon_groupbox.h"
#include "pipp_ser.h"


c_processing_options_dialog::c_processing_options_dialog(QWidget *parent)
    : QDialog(parent),
      m_frame_width(100),
      m_frame_height(100),
      m_data_is_colour(false),
      m_data_has_bayer_pattern(false)
{
    setWindowTitle(tr("Processing"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mp_bayer_pattern_Combobox = new QComboBox;
    mp_bayer_pattern_Combobox->addItem(tr("Auto"), -1);
    mp_bayer_pattern_Combobox->addItem(tr("RGGB"), COLOURID_BAYER_RGGB);
    mp_bayer_pattern_Combobox->addItem(tr("GRBG"), COLOURID_BAYER_GRBG);
    mp_bayer_pattern_Combobox->addItem(tr("GBRG"), COLOURID_BAYER_GBRG);
    mp_bayer_pattern_Combobox->addItem(tr("BGGR"), COLOURID_BAYER_BGGR);
    mp_bayer_pattern_Combobox->addItem(tr("CYYM"), COLOURID_BAYER_CYYM);
    mp_bayer_pattern_Combobox->addItem(tr("YCMY"), COLOURID_BAYER_YCMY);
    mp_bayer_pattern_Combobox->addItem(tr("YMCY"), COLOURID_BAYER_YMCY);
    mp_bayer_pattern_Combobox->addItem(tr("MYYC"), COLOURID_BAYER_MYYC);
    connect(mp_bayer_pattern_Combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(debayer_controls_changed_slot()));
    mp_bayer_pattern_Combobox->setToolTip(tr("This control allows the frames to be debayered using a different bayer pattern than specified in the SER file header"));

    QFormLayout *bayer_pattern_FLayout = new QFormLayout;
    bayer_pattern_FLayout->setMargin(5);
    bayer_pattern_FLayout->setSpacing(5);
    bayer_pattern_FLayout->addRow(tr("Bayer Pattern:"), mp_bayer_pattern_Combobox);

    mp_debayer_GroupBox = new c_icon_groupbox(this);
    mp_debayer_GroupBox->setTitle(tr("Enable Debayering"));
    mp_debayer_GroupBox->set_icon(":/res/resources/debayer_icon.png");
    mp_debayer_GroupBox->setLayout(bayer_pattern_FLayout);
    mp_debayer_GroupBox->setCheckable(true);
    mp_debayer_GroupBox->setChecked(true);
    connect(mp_debayer_GroupBox, SIGNAL(toggled(bool)), this, SLOT(debayer_controls_changed_slot()));

    // Invert checkbox
    mp_invert_CheckBox = new QCheckBox(tr("Invert Frames"));
    mp_invert_CheckBox->setChecked(false);
    connect(mp_invert_CheckBox, SIGNAL(toggled(bool)), this, SIGNAL(invert_frames(bool)));

    QHBoxLayout *invert_HLayout1 = new QHBoxLayout;
    invert_HLayout1->setMargin(5);
    invert_HLayout1->addWidget(mp_invert_CheckBox);

    c_icon_groupbox *invert_GroupBox = new c_icon_groupbox;
    invert_GroupBox->setTitle(tr("Frame Inversion"));
    invert_GroupBox->set_icon(":/res/resources/invert_icon.png");
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

    QGridLayout *gain_and_gamma_GLayout = new QGridLayout;
    gain_and_gamma_GLayout->setVerticalSpacing(10);
    gain_and_gamma_GLayout->setHorizontalSpacing(10);
    gain_and_gamma_GLayout->addWidget(new QLabel(tr("Gain")), 0, 0);
    gain_and_gamma_GLayout->addWidget(mp_gain_Slider, 0, 1);
    gain_and_gamma_GLayout->addWidget(mp_gain_DSpinbox, 0, 2);
    gain_and_gamma_GLayout->addWidget(new QLabel(tr("Gamma")), 1, 0);
    gain_and_gamma_GLayout->addWidget(mp_gamma_Slider, 1, 1);
    gain_and_gamma_GLayout->addWidget(mp_gamma_DSpinbox, 1, 2);

    QPushButton *reset_gain_and_gamma_Button = new QPushButton(tr("Reset"));
    reset_gain_and_gamma_Button->setAutoDefault(false);
    connect(reset_gain_and_gamma_Button, SIGNAL(clicked()), this, SLOT(reset_gain_and_gamma_slot()));

    QHBoxLayout *gamma_HLayout2 = new QHBoxLayout;
    gamma_HLayout2->addWidget(reset_gain_and_gamma_Button);
    gamma_HLayout2->addStretch();

    QVBoxLayout *gain_and_gamma_Vlayout = new QVBoxLayout;
    gain_and_gamma_Vlayout->setMargin(5);
    gain_and_gamma_Vlayout->setSpacing(10);
    gain_and_gamma_Vlayout->addLayout(gain_and_gamma_GLayout);
    gain_and_gamma_Vlayout->addLayout(gamma_HLayout2);

    c_icon_groupbox *gain_and_gammaGroupBox = new c_icon_groupbox;
    gain_and_gammaGroupBox->setTitle(tr("Gain and Gamma"));
    gain_and_gammaGroupBox->set_icon(":/res/resources/gain_icon.png");
    gain_and_gammaGroupBox->setLayout(gain_and_gamma_Vlayout);


    //
    // Monochrome Conversion
    //
    QLabel *monochrome_conversion_Label = new QLabel(tr("Convert To Monochrome Using: "));

    mp_monochrome_conversion_Combobox = new QComboBox;
    mp_monochrome_conversion_Combobox->addItem(tr("RG & B Channels"));
    mp_monochrome_conversion_Combobox->addItem(tr("R Channel Only"));
    mp_monochrome_conversion_Combobox->addItem(tr("G Channel Only"));
    mp_monochrome_conversion_Combobox->addItem(tr("B Channel Only"));
    mp_monochrome_conversion_Combobox->addItem(tr("R & G Channels"));
    mp_monochrome_conversion_Combobox->addItem(tr("R & B Channels"));
    mp_monochrome_conversion_Combobox->addItem(tr("G & B Channels"));
    connect(mp_monochrome_conversion_Combobox, SIGNAL(activated(int)), this, SLOT(monochrome_conversion_changed_slot()));

    QHBoxLayout *monochrome_conversion_GroupBox_Hlayout = new QHBoxLayout;
    monochrome_conversion_GroupBox_Hlayout->setMargin(5);
    monochrome_conversion_GroupBox_Hlayout->addWidget(monochrome_conversion_Label);
    monochrome_conversion_GroupBox_Hlayout->addWidget(mp_monochrome_conversion_Combobox);


    mp_monochrome_conversion_GroupBox = new c_icon_groupbox;
    mp_monochrome_conversion_GroupBox->setTitle(tr("Monochrome Conversion"));
    mp_monochrome_conversion_GroupBox->set_icon(":/res/resources/monochrome_conversion_icon.png");
    mp_monochrome_conversion_GroupBox->setCheckable(true);
    mp_monochrome_conversion_GroupBox->setChecked(false);
    mp_monochrome_conversion_GroupBox->setLayout(monochrome_conversion_GroupBox_Hlayout);
    connect(mp_monochrome_conversion_GroupBox, SIGNAL(toggled(bool)), this, SLOT(monochrome_conversion_changed_slot()));
    connect(mp_monochrome_conversion_GroupBox, SIGNAL(toggled(bool)), this, SLOT(colour_saturation_spinbox_changed_slot()));
    connect(mp_monochrome_conversion_GroupBox, SIGNAL(toggled(bool)), this, SLOT(red_balance_spinbox_changed_slot()));


    //
    // Colour channel align
    //
    mp_blue_x_Spinbox = new QSpinBox;
    mp_blue_x_Spinbox->setRange(-255, +255);

    mp_blue_y_Spinbox = new QSpinBox;
    mp_blue_y_Spinbox->setRange(-255, +255);

    mp_red_x_Spinbox = new QSpinBox;
    mp_red_x_Spinbox->setRange(-255, +255);

    mp_red_y_Spinbox = new QSpinBox;
    mp_red_y_Spinbox->setRange(-255, +255);

    QPushButton *red_left_PushButton = new QPushButton;
    QPixmap left_Pixmap = QPixmap(":/res/resources/back_button.png");
    red_left_PushButton->setIcon(left_Pixmap);
    red_left_PushButton->setIconSize(left_Pixmap.size());
    red_left_PushButton->setFixedSize(left_Pixmap.size() + QSize(10, 10));  // Nice and small

    QPushButton *red_right_PushButton = new QPushButton;
    QPixmap right_Pixmap = QPixmap(":/res/resources/forward_button.png");
    red_right_PushButton->setIcon(right_Pixmap);
    red_right_PushButton->setIconSize(right_Pixmap.size());
    red_right_PushButton->setFixedSize(right_Pixmap.size() + QSize(10, 10));  // Nice and small

    QPushButton *red_up_PushButton = new QPushButton;
    QPixmap up_Pixmap = QPixmap(":/res/resources/up_button.png");
    red_up_PushButton->setIcon(up_Pixmap);
    red_up_PushButton->setIconSize(up_Pixmap.size());
    red_up_PushButton->setFixedSize(up_Pixmap.size() + QSize(10, 10));  // Nice and small

    QPushButton *red_down_PushButton = new QPushButton;
    QPixmap down_Pixmap = QPixmap(":/res/resources/down_button.png");
    red_down_PushButton->setIcon(down_Pixmap);
    red_down_PushButton->setIconSize(down_Pixmap.size());
    red_down_PushButton->setFixedSize(down_Pixmap.size() + QSize(10, 10));  // Nice and small

    QGridLayout *red_buttons_GLayout = new QGridLayout;
#ifdef __APPLE__
    red_buttons_GLayout->setVerticalSpacing(12);
    red_buttons_GLayout->setHorizontalSpacing(12);
#else
    red_buttons_GLayout->setVerticalSpacing(0);
    red_buttons_GLayout->setHorizontalSpacing(0);
#endif
    red_buttons_GLayout->setMargin(0);
    red_buttons_GLayout->addWidget(red_up_PushButton, 0, 1);
    red_buttons_GLayout->addWidget(red_left_PushButton, 1, 0);
    red_buttons_GLayout->addWidget(red_right_PushButton, 1, 2);
    red_buttons_GLayout->addWidget(red_down_PushButton, 2, 1);

    QFormLayout *red_spinboxes_FLayout = new QFormLayout;
    red_spinboxes_FLayout->setMargin(0);
    red_spinboxes_FLayout->setSpacing(5);
    red_spinboxes_FLayout->addRow("x:", mp_red_x_Spinbox);
    red_spinboxes_FLayout->addRow("y:", mp_red_y_Spinbox);

    QVBoxLayout *red_spinboxes_VLayout = new QVBoxLayout;
    red_spinboxes_VLayout->setMargin(0);
    red_spinboxes_VLayout->setSpacing(0);
    red_spinboxes_VLayout->addStretch();
    red_spinboxes_VLayout->addLayout(red_spinboxes_FLayout);
    red_spinboxes_VLayout->addStretch();

    QHBoxLayout *red_controls_HLayout = new QHBoxLayout;
    red_controls_HLayout->setMargin(5);
    red_controls_HLayout->addLayout(red_buttons_GLayout);
    red_controls_HLayout->addLayout(red_spinboxes_VLayout);

    QGroupBox *red_align_GBox = new QGroupBox(tr("Red Channel"));
    red_align_GBox->setLayout(red_controls_HLayout);

    QPushButton *blue_left_PushButton = new QPushButton;
    blue_left_PushButton->setIcon(left_Pixmap);
    blue_left_PushButton->setIconSize(left_Pixmap.size());
    blue_left_PushButton->setFixedSize(left_Pixmap.size() + QSize(10, 10));  // Nice and small

    QPushButton *blue_right_PushButton = new QPushButton;
    blue_right_PushButton->setIcon(right_Pixmap);
    blue_right_PushButton->setIconSize(right_Pixmap.size());
    blue_right_PushButton->setFixedSize(right_Pixmap.size() + QSize(10, 10));  // Nice and small

    QPushButton *blue_up_PushButton = new QPushButton;
    blue_up_PushButton->setIcon(up_Pixmap);
    blue_up_PushButton->setIconSize(up_Pixmap.size());
    blue_up_PushButton->setFixedSize(up_Pixmap.size() + QSize(10, 10));  // Nice and small

    QPushButton *blue_down_PushButton = new QPushButton;
    blue_down_PushButton->setIcon(down_Pixmap);
    blue_down_PushButton->setIconSize(down_Pixmap.size());
    blue_down_PushButton->setFixedSize(down_Pixmap.size() + QSize(10, 10));  // Nice and small

    QGridLayout *blue_buttons_GLayout = new QGridLayout;
#ifdef __APPLE__
    blue_buttons_GLayout->setVerticalSpacing(12);
    blue_buttons_GLayout->setHorizontalSpacing(12);
#else
    blue_buttons_GLayout->setVerticalSpacing(0);
    blue_buttons_GLayout->setHorizontalSpacing(0);
#endif
    blue_buttons_GLayout->setMargin(0);
    blue_buttons_GLayout->addWidget(blue_up_PushButton, 0, 1);
    blue_buttons_GLayout->addWidget(blue_left_PushButton, 1, 0);
    blue_buttons_GLayout->addWidget(blue_right_PushButton, 1, 2);
    blue_buttons_GLayout->addWidget(blue_down_PushButton, 2, 1);

    QFormLayout *blue_spinboxes_FLayout = new QFormLayout;
    blue_spinboxes_FLayout->setMargin(0);
    blue_spinboxes_FLayout->setSpacing(5);
    blue_spinboxes_FLayout->addRow("x:", mp_blue_x_Spinbox);
    blue_spinboxes_FLayout->addRow("y:", mp_blue_y_Spinbox);

    QVBoxLayout *blue_spinboxes_VLayout = new QVBoxLayout;
    blue_spinboxes_VLayout->setMargin(0);
    blue_spinboxes_VLayout->setSpacing(0);
    blue_spinboxes_VLayout->addStretch();
    blue_spinboxes_VLayout->addLayout(blue_spinboxes_FLayout);
    blue_spinboxes_VLayout->addStretch();

    QHBoxLayout *blue_controls_HLayout = new QHBoxLayout;
    blue_controls_HLayout->setMargin(5);
    blue_controls_HLayout->addLayout(blue_buttons_GLayout);
    blue_controls_HLayout->addLayout(blue_spinboxes_VLayout);

    QGroupBox *blue_align_GBox = new QGroupBox(tr("Blue Channel"));
    blue_align_GBox->setLayout(blue_controls_HLayout);

    QHBoxLayout *colour_align_HLayout = new QHBoxLayout;
    colour_align_HLayout->addWidget(red_align_GBox);
    colour_align_HLayout->addWidget(blue_align_GBox);

    QPushButton *colour_align_reset_Button = new QPushButton(tr("Reset"));

    QVBoxLayout *colour_align_VLayout = new QVBoxLayout;
    colour_align_VLayout->setMargin(5);
    colour_align_VLayout->setSpacing(5);
    colour_align_VLayout->addLayout(colour_align_HLayout);
    colour_align_VLayout->addWidget(colour_align_reset_Button, 0, Qt::AlignLeft | Qt::AlignVCenter);

    mp_colour_align_GroupBox = new c_icon_groupbox;
    mp_colour_align_GroupBox->setTitle(tr("Colour Channel Align"));
    mp_colour_align_GroupBox->set_icon(":/res/resources/colour_align.png");
    mp_colour_align_GroupBox->setLayout(colour_align_VLayout);

    connect(red_left_PushButton, SIGNAL(pressed()), mp_red_x_Spinbox, SLOT(stepDown()));
    connect(red_right_PushButton, SIGNAL(pressed()), mp_red_x_Spinbox, SLOT(stepUp()));
    connect(red_down_PushButton, SIGNAL(pressed()), mp_red_y_Spinbox, SLOT(stepDown()));
    connect(red_up_PushButton, SIGNAL(pressed()), mp_red_y_Spinbox, SLOT(stepUp()));

    connect(blue_left_PushButton, SIGNAL(pressed()), mp_blue_x_Spinbox, SLOT(stepDown()));
    connect(blue_right_PushButton, SIGNAL(pressed()), mp_blue_x_Spinbox, SLOT(stepUp()));
    connect(blue_down_PushButton, SIGNAL(pressed()), mp_blue_y_Spinbox, SLOT(stepDown()));
    connect(blue_up_PushButton, SIGNAL(pressed()), mp_blue_y_Spinbox, SLOT(stepUp()));

    connect(colour_align_reset_Button, SIGNAL(pressed()), this, SLOT(reset_colour_align_slot()));

    connect(mp_blue_x_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(colour_align_changed_slot()));
    connect(mp_blue_y_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(colour_align_changed_slot()));
    connect(mp_red_x_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(colour_align_changed_slot()));
    connect(mp_red_y_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(colour_align_changed_slot()));


    //
    // Colour Saturation
    //
    mp_colsat_Slider = new QSlider(Qt::Horizontal);
    mp_colsat_Slider->setRange(0, 1500);
    mp_colsat_Slider->setValue(100);
    mp_colsat_Slider->setMinimumWidth(mp_colsat_Slider->sizeHint().width() * 2);
    connect(mp_colsat_Slider, SIGNAL(sliderMoved(int)), this, SLOT(colour_saturation_slider_changed_slot(int)));
    mp_colsat_DSpinbox = new QDoubleSpinBox;
    mp_colsat_DSpinbox->setRange(0.0, 15.0);
    mp_colsat_DSpinbox->setSingleStep(0.01);
    mp_colsat_DSpinbox->setValue(1.0);

    connect(mp_colsat_DSpinbox, SIGNAL(valueChanged(double)), this, SLOT(colour_saturation_spinbox_changed_slot()));

    QHBoxLayout *colsat_hlayout1 = new QHBoxLayout;
    colsat_hlayout1->addWidget(new QLabel(tr("Saturation")));
    colsat_hlayout1->addWidget(mp_colsat_Slider);
    colsat_hlayout1->addWidget(mp_colsat_DSpinbox);

    QPushButton *reset_colour_saturation_button = new QPushButton(tr("Reset"));
    reset_colour_saturation_button->setAutoDefault(false);
    connect(reset_colour_saturation_button, SIGNAL(clicked()), this, SLOT(reset_colour_saturation_slot()));
    QHBoxLayout *colsat_hlayout2 = new QHBoxLayout;
    colsat_hlayout2->addWidget(reset_colour_saturation_button);
    colsat_hlayout2->addStretch();
    QVBoxLayout *colsat_vlayout = new QVBoxLayout;
    colsat_vlayout->setMargin(5);
    colsat_vlayout->setSpacing(10);
    colsat_vlayout->addLayout(colsat_hlayout1);
    colsat_vlayout->addLayout(colsat_hlayout2);

    mp_colour_saturation_GroupBox = new c_icon_groupbox;
    mp_colour_saturation_GroupBox->setTitle(tr("Colour Saturation"));
    mp_colour_saturation_GroupBox->set_icon(":/res/resources/saturation_icon.png");
    mp_colour_saturation_GroupBox->setLayout(colsat_vlayout);


    //
    // Colour balance
    //
    mp_red_balance_Slider = new QSlider(Qt::Horizontal);
    mp_red_balance_Slider->setRange(-100, 100);
    mp_red_balance_Slider->setValue(0);
    mp_red_balance_Slider->setMinimumWidth(mp_red_balance_Slider->sizeHint().width() * 2);
    connect(mp_red_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(red_balance_slider_changed_slot(int)));
    mp_red_balance_SpinBox = new QSpinBox;
    mp_red_balance_SpinBox->setRange(-100, 100);
    mp_red_balance_SpinBox->setValue(0);
    connect(mp_red_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(red_balance_spinbox_changed_slot()));

    mp_green_balance_Slider = new QSlider(Qt::Horizontal);
    mp_green_balance_Slider->setRange(-100, 100);
    mp_green_balance_Slider->setValue(0);
    connect(mp_green_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(green_balance_slider_changed_slot(int)));
    mp_green_balance_SpinBox = new QSpinBox;
    mp_green_balance_SpinBox->setRange(-100, 100);
    mp_green_balance_SpinBox->setValue(0);
    connect(mp_green_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(green_balance_spinbox_changed_slot()));

    mp_blue_balance_Slider = new QSlider(Qt::Horizontal);
    mp_blue_balance_Slider->setRange(-100, 100);
    mp_blue_balance_Slider->setValue(0);
    connect(mp_blue_balance_Slider, SIGNAL(sliderMoved(int)), this, SLOT(blue_balance_slider_changed_slot(int)));
    mp_blue_balance_SpinBox = new QSpinBox;
    mp_blue_balance_SpinBox->setRange(-100, 100);
    mp_blue_balance_SpinBox->setValue(0);
    connect(mp_blue_balance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(blue_balance_spinbox_changed_slot()));

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
    reset_colour_balance_button->setAutoDefault(false);
    connect(reset_colour_balance_button, SIGNAL(clicked()), this, SLOT(reset_colour_balance_slot()));
    QPushButton *estimate_colour_balance_button = new QPushButton(tr("Estimate", "Estimate Colour Balance Button"));
    estimate_colour_balance_button->setAutoDefault(false);
    connect(estimate_colour_balance_button, SIGNAL(clicked()), this, SIGNAL(estimate_colour_balance()));

    QHBoxLayout *colour_balance_HLayout = new QHBoxLayout;
    colour_balance_HLayout->setMargin(0);
    colour_balance_HLayout->setSpacing(5);
    colour_balance_HLayout->addWidget(reset_colour_balance_button);
    colour_balance_HLayout->addWidget(estimate_colour_balance_button);
    colour_balance_HLayout->addStretch();

    QVBoxLayout *colour_balance_VLayout = new QVBoxLayout;
    colour_balance_VLayout->setMargin(5);
    colour_balance_VLayout->setSpacing(10);
    colour_balance_VLayout->addLayout(colour_balance_GLayout);
    colour_balance_VLayout->addLayout(colour_balance_HLayout);

    mp_colour_balance_GroupBox = new c_icon_groupbox;
    mp_colour_balance_GroupBox->setTitle(tr("Colour Balance"));
    mp_colour_balance_GroupBox->set_icon(":/res/resources/colour_balance_icon.png");
    mp_colour_balance_GroupBox->setLayout(colour_balance_VLayout);


    //
    // Crop controls
    //
    mp_crop_x_start_Spinbox = new QSpinBox;
    mp_crop_y_start_Spinbox = new QSpinBox;
    mp_crop_width_Spinbox = new QSpinBox;
    mp_crop_height_Spinbox = new QSpinBox;
    setup_crop_spinboxes();

    QGridLayout *frame_crop_GLayout = new QGridLayout;   
    frame_crop_GLayout->setMargin(0);
    frame_crop_GLayout->setHorizontalSpacing(10);
    frame_crop_GLayout->setVerticalSpacing(8);
    frame_crop_GLayout->addWidget(new QLabel(tr("X Position:")), 0, 0, 1, 1, Qt::AlignRight);
    frame_crop_GLayout->addWidget(mp_crop_x_start_Spinbox, 0, 1);
    frame_crop_GLayout->addItem(new QSpacerItem(10, 1), 0, 2);
    frame_crop_GLayout->addWidget(new QLabel(tr("Width")), 0, 3, 1, 1, Qt::AlignRight);
    frame_crop_GLayout->addWidget(mp_crop_width_Spinbox, 0, 4);

    frame_crop_GLayout->addWidget(new QLabel(tr("Y Position:")), 1, 0, 1, 1, Qt::AlignRight);
    frame_crop_GLayout->addWidget(mp_crop_y_start_Spinbox, 1, 1);
    frame_crop_GLayout->addItem(new QSpacerItem(10, 1), 1, 2);
    frame_crop_GLayout->addWidget(new QLabel(tr("Height")), 1, 3, 1, 1, Qt::AlignRight);
    frame_crop_GLayout->addWidget(mp_crop_height_Spinbox, 1, 4);

    QHBoxLayout *frame_crop_HLayout = new QHBoxLayout;
    frame_crop_HLayout->setMargin(5);
    frame_crop_HLayout->setSpacing(0);
    frame_crop_HLayout->addLayout(frame_crop_GLayout);
    frame_crop_HLayout->addStretch();

    QPushButton *crop_set_with_selection_box_button = new QPushButton(tr("Set With Selection Box"));
    connect(crop_set_with_selection_box_button, SIGNAL(clicked(bool)), this, SLOT(crop_selection_button_pressed_slot()));

    QHBoxLayout *crop_buttons_hlayout = new QHBoxLayout;
    crop_buttons_hlayout->setMargin(5);
    crop_buttons_hlayout->setSpacing(0);
    crop_buttons_hlayout->addWidget(crop_set_with_selection_box_button);
    crop_buttons_hlayout->addStretch();

    QVBoxLayout *crop_groupbox_vlayout = new QVBoxLayout;
    crop_groupbox_vlayout->setMargin(0);
    crop_groupbox_vlayout->setSpacing(0);
    crop_groupbox_vlayout->addLayout(frame_crop_HLayout);
    crop_groupbox_vlayout->addLayout(crop_buttons_hlayout);

    mp_crop_Groupbox = new c_icon_groupbox;
    mp_crop_Groupbox->setTitle(tr("Frame Crop"));
    mp_crop_Groupbox->set_icon(":/res/resources/crop_icon.png");
    mp_crop_Groupbox->setCheckable(true);
    mp_crop_Groupbox->setChecked(false);
    mp_crop_Groupbox->setLayout(crop_groupbox_vlayout);

    connect(mp_crop_Groupbox, SIGNAL(toggled(bool)), this, SLOT(crop_changed_slot()));
    connect(mp_crop_x_start_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(crop_changed_slot()));
    connect(mp_crop_y_start_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(crop_changed_slot()));
    connect(mp_crop_width_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(crop_changed_slot()));
    connect(mp_crop_height_Spinbox, SIGNAL(valueChanged(int)), this, SLOT(crop_changed_slot()));


    QVBoxLayout *dialog_lhs_vlayout = new QVBoxLayout;
    dialog_lhs_vlayout->setMargin(0);
    dialog_lhs_vlayout->setSpacing(10);
    dialog_lhs_vlayout->addWidget(mp_debayer_GroupBox);
    dialog_lhs_vlayout->addWidget(invert_GroupBox);
    dialog_lhs_vlayout->addWidget(gain_and_gammaGroupBox);
    dialog_lhs_vlayout->addWidget(mp_colour_align_GroupBox);
    dialog_lhs_vlayout->addStretch();

    QVBoxLayout *dialog_rhs_vlayout = new QVBoxLayout;
    dialog_rhs_vlayout->setMargin(0);
    dialog_rhs_vlayout->setSpacing(10);

    dialog_rhs_vlayout->addWidget(mp_monochrome_conversion_GroupBox);
    dialog_rhs_vlayout->addWidget(mp_colour_saturation_GroupBox);
    dialog_rhs_vlayout->addWidget(mp_colour_balance_GroupBox);
    dialog_rhs_vlayout->addWidget(mp_crop_Groupbox);
    dialog_rhs_vlayout->addStretch();

    QHBoxLayout *dialog_hlayout = new QHBoxLayout;
    dialog_hlayout->setMargin(10);
    dialog_hlayout->setSpacing(10);
    dialog_hlayout->addLayout(dialog_lhs_vlayout);
    dialog_hlayout->addLayout(dialog_rhs_vlayout);

    setLayout(dialog_hlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    // Debug
}


void c_processing_options_dialog::crop_changed_slot()
{
    QPalette text_Palette;
    bool crop_spinbox_values_valid = true;
    if (mp_crop_x_start_Spinbox->value() + mp_crop_width_Spinbox->value() > m_frame_width) {
        // Value is not currently valid
        crop_spinbox_values_valid = false;
        text_Palette.setColor(QPalette::Text,Qt::red);
        mp_crop_x_start_Spinbox->setPalette(text_Palette);
        mp_crop_width_Spinbox->setPalette(text_Palette);
    } else {
        text_Palette.setColor(QPalette::Text,QApplication::palette().text().color());
        mp_crop_x_start_Spinbox->setPalette(text_Palette);
        mp_crop_width_Spinbox->setPalette(text_Palette);
    }

    if (mp_crop_y_start_Spinbox->value() + mp_crop_height_Spinbox->value() > m_frame_height) {
        // Value is not currently valid
        crop_spinbox_values_valid = false;

        text_Palette.setColor(QPalette::Text,Qt::red);
        mp_crop_y_start_Spinbox->setPalette(text_Palette);
        mp_crop_height_Spinbox->setPalette(text_Palette);
    } else {
        text_Palette.setColor(QPalette::Text,QApplication::palette().text().color());
        mp_crop_y_start_Spinbox->setPalette(text_Palette);
        mp_crop_height_Spinbox->setPalette(text_Palette);
    }

    bool enabled = mp_crop_Groupbox->isChecked() & isEnabled();
    if (crop_spinbox_values_valid || !enabled) {
        // Emit signal if all spinbox values are valid or crop is not enabled
        emit crop_changed(
                    enabled,
                    mp_crop_x_start_Spinbox->value(),
                    mp_crop_y_start_Spinbox->value(),
                    mp_crop_width_Spinbox->value(),
                    mp_crop_height_Spinbox->value());
    }
}


void c_processing_options_dialog::debayer_controls_changed_slot()
{
    enable_and_disable_controls();
    emit update_image_req();
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


void c_processing_options_dialog::monochrome_conversion_changed_slot()
{
    enable_and_disable_controls();
    emit monochrome_conversion_changed(mp_monochrome_conversion_GroupBox->isChecked(), mp_monochrome_conversion_Combobox->currentIndex());
}


void c_processing_options_dialog::colour_saturation_slider_changed_slot(int sat)
{
    mp_colsat_DSpinbox->setValue(((double)sat/100.0));
}


void c_processing_options_dialog::colour_saturation_spinbox_changed_slot()
{
    mp_colsat_Slider->setValue(100 * mp_colsat_DSpinbox->value());
    emit this->update_image_req();
}


void c_processing_options_dialog::red_balance_slider_changed_slot(int balance)
{
    mp_red_balance_SpinBox->setValue(balance);
}

void c_processing_options_dialog::red_balance_spinbox_changed_slot()
{
    mp_red_balance_Slider->setValue(mp_red_balance_SpinBox->value());
    if (mp_red_balance_SpinBox->isEnabled()) {
        emit colour_balance_changed(
                    1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                    1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                    1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
    } else {
        emit colour_balance_changed(1.0, 1.0, 1.0);
    }
}


void c_processing_options_dialog::green_balance_slider_changed_slot(int balance)
{
    mp_green_balance_SpinBox->setValue(balance);
}


void c_processing_options_dialog::green_balance_spinbox_changed_slot()
{
    mp_green_balance_Slider->setValue(mp_green_balance_SpinBox->value());
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_processing_options_dialog::blue_balance_slider_changed_slot(int balance)
{
    mp_blue_balance_SpinBox->setValue(balance);
}

void c_processing_options_dialog::blue_balance_spinbox_changed_slot()
{
    mp_blue_balance_Slider->setValue(mp_blue_balance_SpinBox->value());
    emit colour_balance_changed(
                1.0 + (double)mp_red_balance_SpinBox->value() / 300,
                1.0 + (double)mp_green_balance_SpinBox->value() / 300,
                1.0 + (double)mp_blue_balance_SpinBox->value() / 300);
}


void c_processing_options_dialog::colour_align_changed_slot()
{
    emit colour_align_changed(mp_red_x_Spinbox->value(),
                              mp_red_y_Spinbox->value(),
                              mp_blue_x_Spinbox->value(),
                              mp_blue_y_Spinbox->value());
}


void c_processing_options_dialog::crop_selection_button_pressed_slot()
{
    setEnabled(false);  // Disable this processing options dialog
    crop_changed_slot();  // Signal that the crop has changed (no longer enabled)
    QSize frame_size = QSize(m_frame_width, m_frame_height);
    QRect selected_area = QRect(mp_crop_x_start_Spinbox->value(),
                                mp_crop_y_start_Spinbox->value(),
                                mp_crop_width_Spinbox->value(),
                                mp_crop_height_Spinbox->value());
    emit enable_area_selection_signal(frame_size, selected_area);
}


void c_processing_options_dialog::crop_selection_complete_slot(bool accepted, QRect selected_area)
{
    if (accepted) {
        mp_crop_x_start_Spinbox->setValue(selected_area.left());
        mp_crop_y_start_Spinbox->setValue(selected_area.top());
        mp_crop_width_Spinbox->setValue(selected_area.width());
        mp_crop_height_Spinbox->setValue(selected_area.height());
    }

    setEnabled(true);
    crop_changed_slot();  // Signal that the crop has changed
}


void c_processing_options_dialog::reset_colour_saturation_slot()
{
    mp_colsat_DSpinbox->setValue(1.0);
}


void c_processing_options_dialog::reset_colour_balance_slot()
{
    mp_red_balance_SpinBox->setValue(0);
    mp_green_balance_SpinBox->setValue(0);
    mp_blue_balance_SpinBox->setValue(0);
}

void c_processing_options_dialog::reset_colour_align_slot()
{
    mp_blue_x_Spinbox->setValue(0);
    mp_blue_y_Spinbox->setValue(0);
    mp_red_x_Spinbox->setValue(0);
    mp_red_y_Spinbox->setValue(0);
}


void c_processing_options_dialog::reset_all_slot()
{
    mp_debayer_GroupBox->setChecked(true);
    mp_invert_CheckBox->setChecked(false);
    mp_monochrome_conversion_GroupBox->setChecked(false);
    mp_monochrome_conversion_Combobox->setCurrentIndex(0);
    reset_gain_and_gamma_slot();
    reset_colour_saturation_slot();
    reset_colour_balance_slot();
    reset_colour_align_slot();
    mp_crop_Groupbox->setChecked(false);
}


void c_processing_options_dialog::set_frame_size(int width, int height)
{
    m_frame_width = width;
    m_frame_height = height;
    setup_crop_spinboxes();
}


void c_processing_options_dialog::setup_crop_spinboxes()
{
    mp_crop_x_start_Spinbox->setRange(0, m_frame_width - 1);
    mp_crop_x_start_Spinbox->setValue(0);

    mp_crop_y_start_Spinbox->setRange(0, m_frame_height - 1);
    mp_crop_y_start_Spinbox->setValue(0);

    mp_crop_width_Spinbox->setRange(15, m_frame_width);
    mp_crop_width_Spinbox->setValue(m_frame_width);

    mp_crop_height_Spinbox->setRange(15, m_frame_height);
    mp_crop_height_Spinbox->setValue(m_frame_height);
}


void c_processing_options_dialog::set_colour_balance(double red, double green, double blue)
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


void c_processing_options_dialog::set_data_has_bayer_pattern(bool bayer_pattern) {
    m_data_has_bayer_pattern = bayer_pattern;
    enable_and_disable_controls();
}


void c_processing_options_dialog::set_data_is_colour(bool colour) {
    m_data_is_colour = colour;
    enable_and_disable_controls();
}


bool c_processing_options_dialog::get_debayer_enable()
{
    return m_data_has_bayer_pattern && mp_debayer_GroupBox->isChecked();
}


int c_processing_options_dialog::get_debayer_pattern()
{
    return mp_bayer_pattern_Combobox->currentData().toInt();
}


double c_processing_options_dialog::get_colour_saturation()
{
    double colour_saturation;
    if (mp_colsat_DSpinbox->isEnabled()) {
        // Control is enabled
        colour_saturation = mp_colsat_DSpinbox->value();
    } else {
        // Control is not enabled
        colour_saturation = 1.0;
    }

    return colour_saturation;
}


bool c_processing_options_dialog::get_processed_data_is_colour()
{
    bool data_is_colour = m_data_is_colour;
    if (m_data_has_bayer_pattern)
    {
        if (mp_debayer_GroupBox->isChecked())
        {
            data_is_colour = true;
        }
    }

    if (mp_monochrome_conversion_GroupBox->isChecked())
    {
        data_is_colour = false;
    }

    return data_is_colour;
}


void c_processing_options_dialog::enable_and_disable_controls()
{
    mp_debayer_GroupBox->setEnabled(m_data_has_bayer_pattern);
    mp_debayer_GroupBox->setVisible(m_data_has_bayer_pattern);

    bool enable_monochrome_conversion_control = false;
    if (m_data_is_colour) {
        // This is colour data
        enable_monochrome_conversion_control = true;
    } else if (m_data_has_bayer_pattern && mp_debayer_GroupBox->isChecked()) {
        enable_monochrome_conversion_control = true;
    }

    mp_colour_align_GroupBox->setVisible(enable_monochrome_conversion_control);

    mp_monochrome_conversion_GroupBox->setEnabled(enable_monochrome_conversion_control);
    mp_monochrome_conversion_GroupBox->setVisible(enable_monochrome_conversion_control);

    bool enable_colour_controls;
    if (mp_monochrome_conversion_GroupBox->isChecked()) {
        // The data must be monochrome if this control is checked
        enable_colour_controls = false;
    } else if (m_data_is_colour) {
        // This is colour data
        enable_colour_controls = true;
    } else if (m_data_has_bayer_pattern && mp_debayer_GroupBox->isChecked()) {
        // Debayered data with bayer pattern is colour data
        enable_colour_controls = true;
    } else {
        // Otherwise this is monochrome data
        enable_colour_controls = false;
    }

    mp_colour_saturation_GroupBox->setEnabled(enable_colour_controls);
    mp_colour_saturation_GroupBox->setVisible(enable_colour_controls);
    mp_colour_balance_GroupBox->setEnabled(enable_colour_controls);
    mp_colour_balance_GroupBox->setVisible(enable_colour_controls);
}


void c_processing_options_dialog::reject()
{
    if (!isEnabled()) {
        // Only allow dialog to be closed if it is enabled
        emit cancel_selected_area_signal();
        crop_selection_complete_slot(false, QRect());
    }

    QDialog::reject();
}
