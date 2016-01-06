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


#ifndef SAVE_FRAMES_PROGRESS_DIALOG_H
#define SAVE_FRAMES_PROGRESS_DIALOG_H

#include <QDialog>

class QLabel;
class QProgressBar;
class QPushButton;


class c_save_frames_progress_dialog : public QDialog
{
    Q_OBJECT

public:
    c_save_frames_progress_dialog(QWidget *parent,
                                  int min_value,
                                  int max_value);
    void set_value(int value);
    void set_button_label(QString label);
    bool was_cancelled();
    void set_complete();

signals:

    
private slots:
    void cancel_button_clicked_slot();

    
private:
    // Private methods
    
    // Widgets
    QLabel *mp_text_label;
    QProgressBar *mp_progress_bar;
    QPushButton *mp_abort_close_button;

    int m_cancelled;
    int m_total_frames;

};

#endif // SAVE_FRAMES_PROGRESS_DIALOG_H
