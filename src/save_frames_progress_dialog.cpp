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

#include <QApplication>
#include <Qt>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include "save_frames_progress_dialog.h"


c_save_frames_progress_dialog::c_save_frames_progress_dialog(QWidget *parent, int min_value, int max_value)
    : QDialog(parent),
      m_cancelled(false),
      m_total_frames(max_value - min_value + 1)
{
    setWindowTitle(tr("Save Frames As Images"));

    mp_text_label = new QLabel(tr("Saving %1 frames").arg(max_value - min_value + 1));
    mp_text_label->setMinimumWidth(mp_text_label->sizeHint().width() * 3);
    mp_progress_bar = new QProgressBar;
    mp_progress_bar->setRange(min_value, max_value);

    mp_cancel_ok_button = new QPushButton(tr("Cancel"));
    connect(mp_cancel_ok_button, SIGNAL(clicked()), this, SLOT(cancel_button_clicked_slot()));
    
    QVBoxLayout *dialog_vlayout = new QVBoxLayout;
    dialog_vlayout->setMargin(15);
    dialog_vlayout->setSpacing(0);
    dialog_vlayout->addWidget(mp_text_label, 0, Qt::AlignLeft);
    dialog_vlayout->addSpacing(15);
    dialog_vlayout->addWidget(mp_progress_bar);
    dialog_vlayout->addSpacing(25);
    dialog_vlayout->addWidget(mp_cancel_ok_button, 0, Qt::AlignRight);
    dialog_vlayout->addStretch();

    setLayout(dialog_vlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


void c_save_frames_progress_dialog::set_value(int value)
{
    mp_progress_bar->setValue(value);
    qApp->processEvents();
}


void c_save_frames_progress_dialog::set_complete()
{
    mp_text_label->setText(mp_text_label->text() + ": " + tr("Complete"));
    mp_cancel_ok_button->setText(tr("Ok"));
}


bool c_save_frames_progress_dialog::was_cancelled()
{
    qApp->processEvents();
    return m_cancelled;
}


void c_save_frames_progress_dialog::cancel_button_clicked_slot()
{
    m_cancelled = true;
}
