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
    int screen_right_edge = widget.availableGeometry().right();
    QPoint playback_pos = parentWidget()->geometry().bottomRight();
    playback_pos.setY(playback_pos.y() - frameGeometry().height());
    if ((playback_pos.x() + frameGeometry().width()) > screen_right_edge) {
        int new_x = screen_right_edge - frameGeometry().width();
        playback_pos.setX(new_x);
    }

    move(playback_pos);
}
