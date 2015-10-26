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
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "histogram_dialog.h"


c_histogram_dialog::c_histogram_dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Histogram"));
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mp_histogram_Label = new QLabel("Histogram");

    QVBoxLayout *dialog_vlayout = new QVBoxLayout;
    dialog_vlayout->addWidget(mp_histogram_Label);
    dialog_vlayout->setMargin(0);
    dialog_vlayout->setSpacing(0);
   
    setLayout(dialog_vlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


void c_histogram_dialog::set_pixmap(QPixmap histogram)
{
    mp_histogram_Label->setPixmap(histogram);
}
