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
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>

#include "selection_box_dialog.h"
#include "persistent_data.h"


c_selection_box_dialog::c_selection_box_dialog(QWidget *parent)
    : QDialog(parent),
      m_width(10),
      m_height(10)
{
    setWindowTitle(tr("Selection Box"));
    QDialog::setWindowFlags((QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint));

    mp_x_pos_SpinBox = new QSpinBox;
    mp_x_pos_SpinBox->setRange(0, 9);
    mp_x_pos_SpinBox->setValue(0);
    connect(mp_x_pos_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_changed_slot()));

    mp_y_pos_SpinBox = new QSpinBox;
    mp_y_pos_SpinBox->setRange(0, 9);
    mp_y_pos_SpinBox->setValue(0);
    connect(mp_y_pos_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_changed_slot()));

    mp_width_SpinBox = new QSpinBox;
    mp_width_SpinBox->setRange(1, 10);
    mp_width_SpinBox->setValue(10);
    connect(mp_width_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_changed_slot()));

    mp_height_SpinBox = new QSpinBox;
    mp_height_SpinBox->setRange(1, 10);
    mp_height_SpinBox->setValue(10);
    connect(mp_height_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinbox_changed_slot()));

    QGridLayout *area_select_GLayout = new QGridLayout;
    area_select_GLayout->setMargin(0);
    area_select_GLayout->setHorizontalSpacing(10);
    area_select_GLayout->setVerticalSpacing(8);
    area_select_GLayout->addWidget(new QLabel(tr("X Position:")), 0, 0, 1, 1, Qt::AlignRight);
    area_select_GLayout->addWidget(mp_x_pos_SpinBox, 0, 1);
    area_select_GLayout->addItem(new QSpacerItem(10, 1), 0, 2);
    area_select_GLayout->addWidget(new QLabel(tr("Width")), 0, 3, 1, 1, Qt::AlignRight);
    area_select_GLayout->addWidget(mp_width_SpinBox, 0, 4);

    area_select_GLayout->addWidget(new QLabel(tr("Y Position:")), 1, 0, 1, 1, Qt::AlignRight);
    area_select_GLayout->addWidget(mp_y_pos_SpinBox, 1, 1);
    area_select_GLayout->addItem(new QSpacerItem(10, 1), 1, 2);
    area_select_GLayout->addWidget(new QLabel(tr("Height")), 1, 3, 1, 1, Qt::AlignRight);
    area_select_GLayout->addWidget(mp_height_SpinBox, 1, 4);

    mp_selection_colour_CBox = new QComboBox;
    mp_selection_colour_CBox->addItem(tr("Red"), QVariant(QColor(Qt::red)));
    mp_selection_colour_CBox->addItem(tr("Green"), QVariant(QColor(Qt::green)));
    mp_selection_colour_CBox->addItem(tr("Blue"), QVariant(QColor(Qt::blue)));
    mp_selection_colour_CBox->addItem(tr("Yellow"), QVariant(QColor(Qt::yellow)));
    mp_selection_colour_CBox->addItem(tr("Magenta"), QVariant(QColor(Qt::magenta)));
    mp_selection_colour_CBox->addItem(tr("Cyan"), QVariant(QColor(Qt::cyan)));
    mp_selection_colour_CBox->addItem(tr("White"),QVariant(QColor(Qt::white)));
    mp_selection_colour_CBox->addItem(tr("Black"), QVariant(QColor(Qt::black)));
    mp_selection_colour_CBox->setCurrentIndex(c_persistent_data::m_selection_box_colour);
    connect(mp_selection_colour_CBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selection_box_colour_changed()));

    QHBoxLayout *colour_select_HLayout = new QHBoxLayout;
    colour_select_HLayout->setMargin(0);
    colour_select_HLayout->setSpacing(5);
    colour_select_HLayout->addWidget(new QLabel(tr("Colour:")));
    colour_select_HLayout->addWidget(mp_selection_colour_CBox);
    colour_select_HLayout->addStretch();

    QPushButton *reset_Button = new QPushButton(tr("Reset"));
    connect(reset_Button, SIGNAL(clicked(bool)), this, SLOT(reset_selection_box_slot()));

    QHBoxLayout *reset_button_HLayout = new QHBoxLayout;
    reset_button_HLayout->setMargin(0);
    reset_button_HLayout->setSpacing(5);
    reset_button_HLayout->addWidget(reset_Button);
    reset_button_HLayout->addStretch();

    QVBoxLayout *gbox_VLayout = new QVBoxLayout;
    gbox_VLayout->setMargin(8);
    gbox_VLayout->setSpacing(15);
    gbox_VLayout->addLayout(area_select_GLayout);
    gbox_VLayout->addLayout(colour_select_HLayout);
    gbox_VLayout->addLayout(reset_button_HLayout);

    mp_area_select_GroupBox = new QGroupBox(tr("Selection Box"));
    mp_area_select_GroupBox->setLayout(gbox_VLayout);


    QPushButton *accept_Button = new QPushButton(tr("Accept"));
    QPushButton *cancel_Button = new QPushButton(tr("Cancel"));
    connect(accept_Button, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(cancel_Button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    QHBoxLayout *accept_button_HLayout = new QHBoxLayout;
    accept_button_HLayout->setMargin(0);
    accept_button_HLayout->setSpacing(10);
    accept_button_HLayout->addStretch();
    accept_button_HLayout->addWidget(accept_Button);
    accept_button_HLayout->addWidget(cancel_Button);

    QVBoxLayout *main_VLayout = new QVBoxLayout;
    main_VLayout->setMargin(10);
    main_VLayout->setSpacing(10);
    main_VLayout->addWidget(mp_area_select_GroupBox);
    main_VLayout->addLayout(accept_button_HLayout);

    setLayout(main_VLayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);  // No resizing
}


QColor c_selection_box_dialog::get_selection_colour()
{
    return mp_selection_colour_CBox->currentData().value<QColor>();
}


void c_selection_box_dialog::start_get_selection_box_slot(int width, int height)
{
    m_width = width;
    m_height = height;
    mp_x_pos_SpinBox->setRange(0, m_width-1);
    mp_y_pos_SpinBox->setRange(0, m_height-1);
    mp_width_SpinBox->setRange(1, m_width);
    mp_height_SpinBox->setRange(1, m_height);
    reset_selection_box_slot();
    show();
}


void c_selection_box_dialog::set_selection_box_slot(const QRect &selection_rect)
{
    mp_x_pos_SpinBox->setValue(selection_rect.left());
    mp_y_pos_SpinBox->setValue(selection_rect.top());
    mp_width_SpinBox->setValue(selection_rect.width());
    mp_height_SpinBox->setValue(selection_rect.height());
}


void c_selection_box_dialog::cancel_get_selection_box_slot()
{
    reset_selection_box_slot();
    hide();
}


void c_selection_box_dialog::reset_selection_box_slot()
{
    mp_x_pos_SpinBox->setValue(0);
    mp_y_pos_SpinBox->setValue(0);
    mp_width_SpinBox->setValue(m_width);
    mp_height_SpinBox->setValue(m_height);
}


void c_selection_box_dialog::update_selection_box(
    const QPoint &top_left_corner,
    const QPoint &bottom_right_corner)
{
    int x = top_left_corner.x();
    int y = top_left_corner.y();
    int width =  bottom_right_corner.x() - top_left_corner.x() + 1;
    int height = bottom_right_corner.y() - top_left_corner.y() + 1;
    mp_x_pos_SpinBox->setValue(x);
    mp_y_pos_SpinBox->setValue(y);
    mp_width_SpinBox->setValue(width);
    mp_height_SpinBox->setValue(height);
}


//
// Protected methods
//
void c_selection_box_dialog::reject()
{
    emit selection_box_complete(false, QRect());
    QDialog::reject();
}

void c_selection_box_dialog::accept()
{
    QRect new_selected_area = QRect(mp_x_pos_SpinBox->value(),
                                    mp_y_pos_SpinBox->value(),
                                    mp_width_SpinBox->value(),
                                    mp_height_SpinBox->value());
    emit selection_box_complete(true, new_selected_area);
    QDialog::accept();
}



//
// Private slots
//
void c_selection_box_dialog::spinbox_changed_slot()
{
    QRect new_selected_area = QRect(mp_x_pos_SpinBox->value(),
                                    mp_y_pos_SpinBox->value(),
                                    mp_width_SpinBox->value(),
                                    mp_height_SpinBox->value());

    emit selection_box_changed(new_selected_area);
}


void c_selection_box_dialog::selection_box_colour_changed()
{
    c_persistent_data::m_selection_box_colour = mp_selection_colour_CBox->currentIndex();
    emit update_request_signal();
}
