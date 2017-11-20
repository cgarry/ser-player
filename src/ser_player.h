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


#ifndef SER_PLAYER_H
#define SER_PLAYER_H

#include <QMainWindow>
#include <QFile>
#include <cstdint>

class QAction;
class QActionGroup;
class QLabel;
class QPushButton;
class QImage;
class QTimer;
class QVBoxLayout;

class c_pipp_ser;
class c_playback_controls_dialog;
class c_playback_controls_widget;
class c_header_details_dialog;
class c_histogram_dialog;
class c_processing_options_dialog;
class c_colour_dialog;
class c_save_frames_dialog;
class c_image_Widget;
class c_image;
class c_histogram_thread;


class c_ser_player : public QMainWindow
{
    Q_OBJECT

private:
    static const QString C_WINDOW_TITLE_QSTRING;
    static const QString C_DEBIAN_XML_TEXT1;
    static const QString C_DEBIAN_XML_TEXT2;
    static const QString C_DEBIAN_XML_TEXT3;

    // Menus
    QAction *mp_save_frames_as_images_Act;
    QAction *mp_save_frames_as_ser_Act;
    QAction *mp_save_frames_as_avi_Act;
    QAction *mp_save_frames_as_gif_Act;
    QMenu *mp_recent_ser_files_Menu;
    QActionGroup *mp_recent_ser_files_ActGroup;
    QMenu *mp_recent_save_folders_Menu;
    QActionGroup *mp_recent_save_folders_ActGroup;
    QMenu *mp_framerate_Menu;
    QAction *mp_header_details_Act;
    QAction *mp_histogram_viewer_Act;
    QAction *mp_processing_options_Act;
    QAction *mp_markers_dialog_Act;
    QAction *mp_detach_playback_controls_Act;

    // Dialogs
    c_playback_controls_dialog *mp_playback_controls_dialog;
    c_header_details_dialog *mp_header_details_dialog;
    c_processing_options_dialog *mp_processing_options_Dialog;
    c_histogram_dialog *mp_histogram_dialog;
    c_save_frames_dialog *mp_save_frames_as_ser_Dialog;
    c_save_frames_dialog *mp_save_frames_as_avi_Dialog;
    c_save_frames_dialog *mp_save_frames_as_gif_Dialog;
    c_save_frames_dialog *mp_save_frames_as_images_Dialog;

    // Threads
    c_histogram_thread *mp_histogram_thread;

    // Widgets
    c_playback_controls_widget *mp_playback_controls_widget;
    QPixmap m_no_file_open_Pixmap;
    c_image_Widget *mp_frame_image_Widget;
    QTimer *mp_frame_Timer;
//    QTimer *mp_resize_Timer;

    QVBoxLayout *mp_main_vlayout;

    // Other
    bool m_ser_file_loaded;
    c_pipp_ser *mp_ser_file;
    c_image *mp_frame_image;
    QString m_ser_directory;
    int m_total_frames;
    int m_display_framerate;
    int m_display_frame_time;
    bool m_is_colour;
    bool m_has_bayer_pattern;

    bool m_monochrome_conversion_enable;
    int m_monochrome_conversion_type;
    bool m_crop_enable;
    int m_crop_x_pos;
    int m_crop_y_pos;
    int m_crop_width;
    int m_crop_height;
    int m_requested_zoom;


public:
    c_ser_player(QWidget *parent = 0);
    ~c_ser_player();
    void open_ser_file(const QString &filename);

public slots:
    void fps_changed_slot(QAction *);
    void header_details_dialog_closed_slot();
    void header_details_dialog_slot(bool checked);
    void histogram_viewer_closed_slot();
    void histogram_viewer_slot(bool checked);
    void detach_playback_controls_slot(bool detach);
    void playback_controls_closed_slot();
    void processing_options_slot(bool checked);
    void processor_options_closed_slot();
    void invert_changed_slot(bool invert);
    void gain_changed_slot(double gain);
    void gamma_changed_slot(double gamma);
    void crop_changed_slot(bool crop_enable, int crop_x, int crop_y, int crop_width, int crop_height);
    void monochrome_conversion_changed_slot(bool enabled, int selection);
    void colour_balance_changed_slot(double red, double green, double blue);
    void estimate_colour_balance();
    void colour_align_changed_slot(int red_align_x, int red_align_y, int blue_align_x, int blue_align_y);
    void zoom_changed_slot(QAction *);
    void language_changed_slot(QAction *);
    void open_ser_file_slot();
    void open_ser_file_slot(QAction *action);
    void save_frames_as_ser_slot();
    void save_frames_as_avi_slot();
    void save_frames_as_gif_slot();
    void save_frames_as_images_slot();
    void open_save_folder_slot(QAction *);
    void frame_timer_timeout_slot();
//void resize_timer_timeout_slot();
    void frame_slider_changed_slot();
    void markers_dialog_closed_slot();
    void resize_window_100_percent_slot();
    void check_for_updates_slot(bool enabled);
    void debayer_enable_slot();
    void new_version_available_slot(QString version);
    void about_qt();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void handle_arguments();
    void about_ser_player();
    void histogram_done_slot();
    void start_playing_slot();
    void stop_playing_slot();
    void playback_controls_double_clicked_slot();


protected:
//    virtual void resizeEvent(QResizeEvent *event);
    virtual void changeEvent (QEvent *event);

private:
    void add_string_to_stringlist(QStringList &string_list, QString string);
    void update_recent_ser_files_menu();
    void populate_recent_ser_files_menu();
    void update_recent_save_folders_menu();
    void populate_recent_save_folders_menu();
    void create_no_file_open_image();
    bool get_and_process_frame(int frame_number, bool conv_to_8_bit, bool do_processing);
    void calculate_display_framerate();
    void resize_window_with_zoom(int zoom);
    void set_defaut_histogram_position();
};

#endif // SER_PLAYER_H
