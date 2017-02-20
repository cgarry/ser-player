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
#include <QDesktopWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "playback_controls_dialog.h"


c_playback_controls_dialog::c_playback_controls_dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(" ");
    QDialog::setWindowFlags(QDialog::windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mp_dummy_label = new QLabel("Dummy");

    mp_dialog_vlayout = new QVBoxLayout;
    mp_dialog_vlayout->addWidget(mp_dummy_label);
    mp_dialog_vlayout->setMargin(0);
    mp_dialog_vlayout->setSpacing(0);
   
    setLayout(mp_dialog_vlayout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void c_playback_controls_dialog::add_controls_widget(QWidget  *p_widget)
{
    mp_dummy_label->hide();
    mp_dialog_vlayout->removeWidget(mp_dummy_label);
    mp_dialog_vlayout->insertWidget(0, p_widget);
    show();
}


void c_playback_controls_dialog::remove_controls_widget(QWidget  *p_widget)
{
    mp_dummy_label->show();
    mp_dialog_vlayout->removeWidget(p_widget);
    mp_dialog_vlayout->insertWidget(0, mp_dummy_label);
    hide();
}


void c_playback_controls_dialog::move_to_default_position()
{
    QDesktopWidget widget;
    int screen_bottom_edge = widget.availableGeometry().bottom();
    QPoint dialog_pos = parentWidget()->pos();
    if (parentWidget()->windowState() != Qt::WindowMaximized) {
        dialog_pos.setY(dialog_pos.y() + parentWidget()->frameGeometry().height());
        if ((dialog_pos.y() + frameGeometry().height()) > screen_bottom_edge) {
            int new_y_pos = screen_bottom_edge - frameGeometry().height();
            dialog_pos.setY(new_y_pos);
        }
    } else {
        dialog_pos.setY(dialog_pos.y() + parentWidget()->frameGeometry().height() - frameGeometry().height());
    }

    move(dialog_pos);
}

