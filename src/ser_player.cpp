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


#define VERSION_STRING "v1.6.6"

#include <Qt>
#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFuture>
#include <QImage>
#include <QImageWriter>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QWidgetAction>

#include <cmath>

#include "playback_controls_dialog.h"
#include "playback_controls_widget.h"
#include "gif_write.h"
#include "histogram_thread.h"
#include "histogram_dialog.h"
#include "image.h"
#include "ser_player.h"
#include "persistent_data.h"
#include "pipp_timestamp.h"
#include "pipp_ser.h"
#include "pipp_avi_write_dib.h"
#include "pipp_ser_write.h"
#include "pipp_utf8.h"
#include "image_widget.h"
#include "processing_options_dialog.h"
#include "markers_dialog.h"
#include "save_frames_dialog.h"
#include "save_frames_progress_dialog.h"
#include "header_details_dialog.h"


#ifndef DISABLE_NEW_VERSION_CHECK
#include "new_version_checker.h"
#endif

const QString c_ser_player::C_WINDOW_TITLE_QSTRING = QString("SER Player");


c_ser_player::c_ser_player(QWidget *parent)
    : QMainWindow(parent),
      mp_playback_controls_dialog(nullptr),
      mp_save_frames_as_ser_Dialog(nullptr),
      mp_save_frames_as_avi_Dialog(nullptr),
      mp_save_frames_as_gif_Dialog(nullptr),
      mp_save_frames_as_images_Dialog(nullptr)
{
    m_requested_zoom = 100;
    m_ser_file_loaded = false;
    mp_frame_image = new c_image;
    m_is_colour = false;
    m_has_bayer_pattern = false;
    mp_histogram_thread = new c_histogram_thread;
    connect(mp_histogram_thread, SIGNAL(histogram_done()), this, SLOT(histogram_done_slot()));

    // Menu Items
    m_ser_directory = "";
    m_display_framerate = -1;
    m_monochrome_conversion_enable = false;
    m_monochrome_conversion_type = 0;

    m_crop_enable = false;
    m_crop_x_pos = 0;
    m_crop_y_pos = 0;
    m_crop_width = 10;
    m_crop_height = 10;


    //
    // File menu
    //
    QMenu *file_menu = menuBar()->addMenu(tr("File", "Menu title"));
    QAction *fileopen_Act = new QAction(tr("Open SER File", "Menu title"), this);
    file_menu->addAction(fileopen_Act);
    connect(fileopen_Act, SIGNAL(triggered()), this, SLOT(open_ser_file_slot()));

    mp_recent_ser_files_Menu = file_menu->addMenu(tr("Open Recent", "Menu title"));
    populate_recent_ser_files_menu();

    file_menu->addSeparator();

    mp_save_frames_as_ser_Act = new QAction(tr("Save Frames As SER File...", "Menu title"), this);
    mp_save_frames_as_ser_Act->setEnabled(false);
    file_menu->addAction(mp_save_frames_as_ser_Act);
    connect(mp_save_frames_as_ser_Act, SIGNAL(triggered()), this, SLOT(save_frames_as_ser_slot()));

    mp_save_frames_as_avi_Act = new QAction(tr("Save Frames As AVI File...", "Menu title"), this);
    mp_save_frames_as_avi_Act->setEnabled(false);
    file_menu->addAction(mp_save_frames_as_avi_Act);
    connect(mp_save_frames_as_avi_Act, SIGNAL(triggered()), this, SLOT(save_frames_as_avi_slot()));

    mp_save_frames_as_gif_Act = new QAction(tr("Save Frames As Animated GIF...", "Menu title"), this);
    mp_save_frames_as_gif_Act->setEnabled(false);
    file_menu->addAction(mp_save_frames_as_gif_Act);
    connect(mp_save_frames_as_gif_Act, SIGNAL(triggered()), this, SLOT(save_frames_as_gif_slot()));

    mp_save_frames_as_images_Act = new QAction(tr("Save Frames As Images...", "Menu title"), this);
    mp_save_frames_as_images_Act->setEnabled(false);
    file_menu->addAction(mp_save_frames_as_images_Act);
    connect(mp_save_frames_as_images_Act, SIGNAL(triggered()), this, SLOT(save_frames_as_images_slot()));


    mp_recent_save_folders_Menu = file_menu->addMenu(tr("Recent Save Folders", "Menu title"));
    populate_recent_save_folders_menu();

    file_menu->addSeparator();

    QAction *quit_Act = new QAction(tr("Quit", "Menu title"), this);
    file_menu->addAction(quit_Act);
    connect(quit_Act, SIGNAL(triggered()), this, SLOT(close()));


    //
    // Playback menu
    //
    QMenu *playback_menu = menuBar()->addMenu(tr("Playback", "Menu title"));

    mp_detach_playback_controls_Act = playback_menu->addAction(tr("Detach Playback Controls", "Playback menu"));
    mp_detach_playback_controls_Act->setEnabled(true);
    mp_detach_playback_controls_Act->setCheckable(true);
    connect(mp_detach_playback_controls_Act, SIGNAL(triggered(bool)), this, SLOT(detach_playback_controls_slot(bool)));

    playback_menu->addSeparator();

    const int zoom_levels[] = {25, 50, 75, 100, 125, 150, 200, 250, 300};
    QMenu *zoom_Menu = playback_menu->addMenu(tr("Zoom", "Playback menu"));
    QActionGroup *zoom_ActGroup = new QActionGroup(zoom_Menu);
    QAction *zoom_action;
    for (int x = 0; x <  (int)(sizeof(zoom_levels) / sizeof(zoom_levels[0])); x++) {
        zoom_action = new QAction(tr("%1%", "Zoom menu").arg(zoom_levels[x]), this);
        zoom_action->setData(zoom_levels[x]);
        zoom_Menu->addAction(zoom_action);
        zoom_ActGroup->addAction(zoom_action);
    }

    connect(zoom_ActGroup, SIGNAL (triggered(QAction *)), this, SLOT (zoom_changed_slot(QAction *)));

    mp_framerate_Menu = playback_menu->addMenu(tr("Framerate", "Playback menu"));
    mp_framerate_Menu->setEnabled(false);
    QActionGroup *fps_ActGroup = new QActionGroup(mp_framerate_Menu);
    fps_ActGroup->setExclusive(true);
    QAction *fps_action;

    fps_action = new QAction(tr("From Timestamps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setChecked(true);
    fps_action->setData(-1);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    const int fps_values[] = {1, 5, 10, 24, 30, 50, 75, 100, 150, 200};
    for (int x = 0; x <  (int)(sizeof(fps_values) / sizeof(fps_values[0])); x++) {
        fps_action = new QAction(tr("%1 fps", "Framerate menu").arg(fps_values[x]), this);
        fps_action->setCheckable(true);
        fps_action->setData(fps_values[x]);
        mp_framerate_Menu->addAction(fps_action);
        fps_ActGroup->addAction(fps_action);
    }

    connect(fps_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT(fps_changed_slot(QAction *)));


    //
    // Tools menu
    //
    QMenu *tools_menu = menuBar()->addMenu(tr("Tools", "Menu title"));

    // SER File Details
    mp_header_details_Act = tools_menu->addAction(tr("SER File Details", "Tools menu"));
    mp_header_details_Act->setEnabled(false);
    mp_header_details_Act->setCheckable(true);
    mp_header_details_Act->setChecked(false);
    connect(mp_header_details_Act, SIGNAL(triggered(bool)), this, SLOT(header_details_dialog_slot(bool)));
    mp_header_details_dialog = new c_header_details_dialog(this);
    mp_header_details_dialog->hide();
    connect(mp_header_details_dialog, SIGNAL(rejected()), this, SLOT(header_details_dialog_closed_slot()));

    // Detached playback controls dialog
    mp_playback_controls_dialog = new c_playback_controls_dialog(this);
    mp_playback_controls_dialog->hide();
    connect(mp_playback_controls_dialog, SIGNAL(rejected()), this, SLOT(playback_controls_closed_slot()));

    mp_playback_controls_widget = new c_playback_controls_widget(this);
    mp_playback_controls_widget->set_maximum_frame(99);
    mp_playback_controls_widget->set_repeat(c_persistent_data::m_repeat);
    connect(mp_playback_controls_widget, SIGNAL(markers_dialog_closed()), this, SLOT(markers_dialog_closed_slot()));
    connect(mp_playback_controls_widget, SIGNAL(open_ser_file_signal()), this, SLOT(open_ser_file_slot()));
    connect(mp_playback_controls_widget, SIGNAL(double_clicked_signal()), this, SLOT(playback_controls_double_clicked_slot()));

    // Histogram viewer
    mp_histogram_viewer_Act = tools_menu->addAction(tr("Histogram", "Tools menu"));
    mp_histogram_viewer_Act->setEnabled(false);
    mp_histogram_viewer_Act->setCheckable(true);
    mp_histogram_viewer_Act->setChecked(c_persistent_data::m_histogram_enabled);
    connect(mp_histogram_viewer_Act, SIGNAL(triggered(bool)), this, SLOT(histogram_viewer_slot(bool)));
    mp_histogram_dialog = new c_histogram_dialog(this);
    mp_histogram_dialog->hide();
    connect(mp_histogram_dialog, SIGNAL(rejected()), this, SLOT(histogram_viewer_closed_slot()));

    tools_menu->addSeparator();

    // Processing menu action
    mp_processing_options_Act = tools_menu->addAction(tr("Processing", "Tools menu"));
    mp_processing_options_Act->setEnabled(false);
    mp_processing_options_Act->setCheckable(true);
    mp_processing_options_Act->setChecked(false);
    connect(mp_processing_options_Act, SIGNAL(triggered(bool)), this, SLOT(processing_options_slot(bool)));
    mp_processing_options_Dialog = new c_processing_options_dialog(this);
    mp_processing_options_Dialog->hide();
    connect(mp_processing_options_Dialog, SIGNAL(crop_changed(bool,int,int,int,int)), this, SLOT(crop_changed_slot(bool,int,int,int,int)));
    connect(mp_processing_options_Dialog, SIGNAL(update_image_req()), this, SLOT(debayer_enable_slot()));
    connect(mp_processing_options_Dialog, SIGNAL(invert_frames(bool)), this, SLOT(invert_changed_slot(bool)));
    connect(mp_processing_options_Dialog, SIGNAL(gain_changed(double)), this, SLOT(gain_changed_slot(double)));
    connect(mp_processing_options_Dialog, SIGNAL(gamma_changed(double)), this, SLOT(gamma_changed_slot(double)));
    connect(mp_processing_options_Dialog, SIGNAL(monochrome_conversion_changed(bool,int)), this, SLOT(monochrome_conversion_changed_slot(bool,int)));
    connect(mp_processing_options_Dialog, SIGNAL(colour_balance_changed(double,double,double)), this, SLOT(colour_balance_changed_slot(double,double,double)));
    connect(mp_processing_options_Dialog, SIGNAL(estimate_colour_balance()), this, SLOT(estimate_colour_balance()));
    connect(mp_processing_options_Dialog, SIGNAL(colour_align_changed(int,int,int,int)), this, SLOT(colour_align_changed_slot(int,int,int,int)));
    connect(mp_processing_options_Dialog, SIGNAL(rejected()), this, SLOT(processor_options_closed_slot()));

    // Markers Dialog action
    mp_markers_dialog_Act = tools_menu->addAction(tr("Markers", "Tools menu"));
    mp_markers_dialog_Act->setEnabled(false);
    mp_markers_dialog_Act->setCheckable(true);
    mp_markers_dialog_Act->setChecked(false);
    connect(mp_markers_dialog_Act, SIGNAL(triggered(bool)), mp_playback_controls_widget, SLOT(show_markers_dialog(bool)));

    //
    // Help menu
    //
    QMenu *help_menu = menuBar()->addMenu(tr("Help", "Help menu"));

#ifndef DISABLE_NEW_VERSION_CHECK
    QAction *check_for_updates_Act = new QAction(tr("Check For Updates On Startup", "Help menu"), this);
    check_for_updates_Act->setCheckable(true);
    check_for_updates_Act->setChecked(c_persistent_data::m_check_for_updates);
    help_menu->addAction(check_for_updates_Act);
    connect(check_for_updates_Act, SIGNAL(triggered(bool)), this, SLOT(check_for_updates_slot(bool)));
#endif

    QMenu *language_menu = help_menu->addMenu(tr("Language", "Help menu"));
    language_menu->setToolTip(tr("Restart for language change to take effect"));
    QActionGroup* lang_ActGroup = new QActionGroup(language_menu);
    lang_ActGroup->setExclusive(true);


    // First language menu item is always 'Auto'
    QAction *action = new QAction(tr("Auto"), this);
    action->setCheckable(true);
    action->setData("auto");
    language_menu->addAction(action);
    lang_ActGroup->addAction(action);
    if (c_persistent_data::m_selected_language == action->data().toString()) {
        action->setChecked(true);
    }

    // 2nd language menu item is always 'English'
    //: Language name
    action = new QAction(tr("English"), this);
    action->setCheckable(true);
    action->setData("en");
    language_menu->addAction(action);
    lang_ActGroup->addAction(action);
    if (c_persistent_data::m_selected_language == action->data().toString()) {
        action->setChecked(true);
    }

    // Remaining menu items are from installed language files
    QStringList lang_files = QDir(":/res/translations/").entryList(QStringList("ser_player_*.qm"));
    QStringList lang_list;
    for (int x = 0; x < lang_files.size(); x++) {
        // get locale extracted by filename
        QString locale = lang_files[x]; // Example: "ser_player_de.qm"
        locale.truncate(locale.lastIndexOf('.')); // Example: "ser_player_de"
        locale.replace("ser_player_", ""); // Example: "de"

        QString lang;
        switch (QLocale(locale).language()) {
            case QLocale::Arabic:  //: Language name
            lang = tr("Arabic"); break;
            case QLocale::Bulgarian:  //: Language name
            lang = tr("Bulgarian"); break;
            case QLocale::Chinese:  //: Language name
            lang = tr("Chinese"); break;
            case QLocale::Croatian:  //: Language name
            lang = tr("Croatian"); break;
            case QLocale::Czech:  //: Language name
            lang = tr("Czech"); break;
            case QLocale::Danish:  //: Language name
            lang = tr("Danish"); break;
            case QLocale::Dutch:  //: Language name
            lang = tr("Dutch"); break;
            case QLocale::English:  //: Language name
            lang = tr("English"); break;
            case QLocale::Finnish:  //: Language name
            lang = tr("Finnish"); break;
            case QLocale::French:  //: Language name
            lang = tr("French"); break;
            case QLocale::German:  //: Language name
            lang = tr("German"); break;
            case QLocale::Greek:  //: Language name
            lang = tr("Greek"); break;
            case QLocale::Hebrew:  //: Language name
            lang = tr("Hebrew"); break;
            case QLocale::Hindi:  //: Language name
            lang = tr("Hindi"); break;
            case QLocale::Indonesian:  //: Language name
            lang = tr("Indonesian"); break;
            case QLocale::Irish:  //: Language name
            lang = tr("Irish"); break;
            case QLocale::Italian:  //: Language name
            lang = tr("Italian"); break;
            case QLocale::Japanese:  //: Language name
            lang = tr("Japanese"); break;
            case QLocale::Latvian:  //: Language name
            lang = tr("Latvian"); break;
            case QLocale::Lithuanian:  //: Language name
            lang = tr("Lithuanian"); break;
            case QLocale::Norwegian:  //: Language name
            lang = tr("Norwegian"); break;
            case QLocale::Polish:  //: Language name
            lang = tr("Polish"); break;
            case QLocale::Portuguese:  //: Language name
            lang = tr("Portuguese"); break;
            case QLocale::Romanian:  //: Language name
            lang = tr("Romanian"); break;
            case QLocale::Russian:  //: Language name
            lang = tr("Russian"); break;
            case QLocale::Serbian:  //: Language name
            lang = tr("Serbian"); break;
            case QLocale::Slovenian:  //: Language name
            lang = tr("Slovenian"); break;
            case QLocale::Spanish:  //: Language name
            lang = tr("Spanish"); break;
            case QLocale::Swedish:  //: Language name
            lang = tr("Swedish"); break;
            case QLocale::Thai:  //: Language name
            lang = tr("Thai"); break;
            case QLocale::Ukrainian:  //: Language name
            lang = tr("Ukrainian"); break;
            case QLocale::Urdu:  //: Language name
            lang = tr("Urdu"); break;
            case QLocale::Vietnamese:  //: Language name
            lang = tr("Vietnamese"); break;
            default: lang = QLocale::languageToString(QLocale(locale).language());  // Handle other languages in English
        }

        lang_list.append(lang + "," + locale);
    }

    lang_list.sort();  // Reorder alphabetically
    foreach (const QString &lang_details, lang_list) {
        QStringList lang = lang_details.split(',');
        action = new QAction(lang[0], this);
        action->setCheckable(true);
        action->setData(lang[1]);
        language_menu->addAction(action);
        lang_ActGroup->addAction(action);
        if (c_persistent_data::m_selected_language == action->data().toString()) {
            action->setChecked(true);
        }
    }

    connect(lang_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT(language_changed_slot(QAction *)));


    help_menu->addSeparator();


    QAction *about_ser_player_Act = new QAction(tr("About SER Player", "Help menu"), this);
    help_menu->addAction(about_ser_player_Act);
    connect(about_ser_player_Act, SIGNAL(triggered()), this, SLOT(about_ser_player()));
    QAction *about_qt_Act = new QAction(tr("About Qt", "Help menu"), this);
    help_menu->addAction(about_qt_Act);
    connect(about_qt_Act, SIGNAL(triggered()), this, SLOT(about_qt()));

    create_no_file_open_image();  // Create m_no_file_open_Pixmap

    mp_frame_image_Widget = new c_image_Widget(this);
    mp_frame_image_Widget->setPixmap(m_no_file_open_Pixmap);
    connect(mp_processing_options_Dialog, SIGNAL(enable_area_selection_signal(QSize,QRect)), mp_frame_image_Widget, SLOT(enable_area_selection_slot(QSize,QRect)));
    connect(mp_processing_options_Dialog, SIGNAL(cancel_selected_area_signal()), mp_frame_image_Widget, SLOT(cancel_area_selection_slot()));
    connect(mp_frame_image_Widget, SIGNAL(selection_box_complete_signal(bool,QRect)), mp_processing_options_Dialog, SLOT(crop_selection_complete_slot(bool,QRect)));
    connect(mp_frame_image_Widget, SIGNAL(zoom_changed_signal(int)), mp_playback_controls_widget, SLOT(update_zoom_label_slot(int)));

    mp_main_vlayout = new QVBoxLayout;
    mp_main_vlayout->setSpacing(0);
    mp_main_vlayout->setMargin(0);
    mp_main_vlayout->addWidget(mp_frame_image_Widget, 2);
    mp_main_vlayout->addWidget(mp_playback_controls_widget);

    // Set layout in QWidget
    QWidget *main_widget = new QWidget;
    main_widget->setLayout(mp_main_vlayout);

    setCentralWidget(main_widget);
    setWindowTitle(C_WINDOW_TITLE_QSTRING);

    mp_ser_file = new c_pipp_ser;

    mp_frame_Timer = new QTimer(this);
    connect(mp_frame_Timer, SIGNAL(timeout()), this, SLOT(frame_timer_timeout_slot()));

//    mp_resize_Timer = new QTimer(this);
//    mp_resize_Timer->setSingleShot(true);

//    connect(mp_resize_Timer, SIGNAL(timeout()), this, SLOT(resize_timer_timeout_slot()));

    connect(mp_playback_controls_widget, SIGNAL(slider_value_changed(int)), this, SLOT(frame_slider_changed_slot()));
    connect(mp_playback_controls_widget, SIGNAL(start_playing_signal()), this, SLOT(start_playing_slot()));
    connect(mp_playback_controls_widget, SIGNAL(stop_playing_signal()), this, SLOT(stop_playing_slot()));

    connect(mp_frame_image_Widget, SIGNAL(double_click_signal()), this, SLOT(resize_window_100_percent_slot()));

    setAcceptDrops(true);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QTimer::singleShot(50, this, SLOT(handle_arguments()));

    // Update check
#ifndef DISABLE_NEW_VERSION_CHECK
    c_new_version_checker *new_version_checker;
    if (c_persistent_data::m_check_for_updates) {
        new_version_checker = new c_new_version_checker(this, VERSION_STRING);
        new_version_checker->check();  // Check for vewer SER Player version
        connect(new_version_checker, SIGNAL(new_version_available_signal(QString)),
                this, SLOT(new_version_available_slot(QString)));
    }

    if (QSslSocket::supportsSsl()) {
        qDebug() << "SSL INFO: SSL Support Included";
    } else {
        qDebug() << "SSL INFO: SSL Support Missing";
    }
#endif
}


c_ser_player::~c_ser_player()
{
}


void c_ser_player::add_string_to_stringlist(QStringList &string_list, QString string)
{
    const int max_list_length = 10;

    // Add new string to stringlist
    string_list.prepend(string);

    // Remove duplicates from list
    string_list.removeDuplicates();

    // Limit list length to max_list_length
    while (string_list.count() > max_list_length) {
        string_list.removeLast();
    }

}


void c_ser_player::update_recent_ser_files_menu()
{
    // Delete actions from Menu
    mp_recent_ser_files_Menu->clear();

    // Delete actions
    QList<QAction *> action_list =  mp_recent_ser_files_ActGroup->actions();
    for (int i = 0; i < action_list.count(); i++) {
        delete action_list.at(i);  // Delete action
    }

    // Disconnect action group
    disconnect(mp_recent_ser_files_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT(open_ser_file_slot(QAction *)));

    // Delete action group
    delete mp_recent_ser_files_ActGroup;

    // Populate menu again
    populate_recent_ser_files_menu();
}


void c_ser_player::populate_recent_ser_files_menu()
{
    mp_recent_ser_files_ActGroup = new QActionGroup(mp_recent_ser_files_Menu);
    QAction *ser_files_action;
    if (c_persistent_data::m_recent_ser_files.count() > 0) {
        for (int x = 0; x < c_persistent_data::m_recent_ser_files.count() ; x++) {
            ser_files_action = new QAction(QString::number(x+1) + " " + c_persistent_data::m_recent_ser_files.at(x), this);
            ser_files_action->setData(c_persistent_data::m_recent_ser_files.at(x));
            mp_recent_ser_files_Menu->addAction(ser_files_action);
            mp_recent_ser_files_ActGroup->addAction(ser_files_action);
        }

        mp_recent_ser_files_Menu->addSeparator();

        ser_files_action = new QAction(tr("Clear Recent", "Save Folders menu entry"), this);
        ser_files_action->setData(QString(""));
        mp_recent_ser_files_Menu->addAction(ser_files_action);
        mp_recent_ser_files_ActGroup->addAction(ser_files_action);
        connect(mp_recent_ser_files_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT(open_ser_file_slot(QAction *)));
    } else {
        ser_files_action = new QAction(tr("No SER Files In List", "Recent SER Files menu entry"), this);
        mp_recent_ser_files_Menu->addAction(ser_files_action);
    }
}


void c_ser_player::update_recent_save_folders_menu()
{
    // Delete actions from Menu
    mp_recent_save_folders_Menu->clear();

    // Delete actions
    QList<QAction *> action_list =  mp_recent_save_folders_ActGroup->actions();
    for (int i = 0; i < action_list.count(); i++) {
        delete action_list.at(i);  // Delete action
    }

    // Disconnect action group
    disconnect(mp_recent_save_folders_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT(open_save_folder_slot(QAction *)));

    // Delete action group
    delete mp_recent_save_folders_ActGroup;

    // Populate menu again
    populate_recent_save_folders_menu();
}


void c_ser_player::populate_recent_save_folders_menu()
{
    mp_recent_save_folders_ActGroup = new QActionGroup(mp_recent_save_folders_Menu);
    QAction *save_folders_action;
    if (c_persistent_data::m_recent_save_folders.count() > 0) {
        for (int x = 0; x < c_persistent_data::m_recent_save_folders.count() ; x++) {
            save_folders_action = new QAction(QString::number(x+1) + " " + c_persistent_data::m_recent_save_folders.at(x), this);
            save_folders_action->setData(c_persistent_data::m_recent_save_folders.at(x));
            mp_recent_save_folders_Menu->addAction(save_folders_action);
            mp_recent_save_folders_ActGroup->addAction(save_folders_action);
        }

        mp_recent_save_folders_Menu->addSeparator();

        save_folders_action = new QAction(tr("Clear Recent", "Save Folders menu entry"), this);
        save_folders_action->setData(QString(""));
        mp_recent_save_folders_Menu->addAction(save_folders_action);
        mp_recent_save_folders_ActGroup->addAction(save_folders_action);
        connect(mp_recent_save_folders_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT(open_save_folder_slot(QAction *)));
    } else {
        save_folders_action = new QAction(tr("No Save Folders In List", "Recent Save Folders menu entry"), this);
        mp_recent_save_folders_Menu->addAction(save_folders_action);
    }
}


void c_ser_player::handle_arguments()
{
    // Check command line arguments for SER file to open
    for (int arg_count = 1; arg_count < QCoreApplication::arguments().count(); arg_count++) {
        QString file_name = QCoreApplication::arguments().at(arg_count);
        if (file_name.endsWith(".ser", Qt::CaseInsensitive)) {
            open_ser_file(file_name);
            break;
        }
    }

    if (c_persistent_data::m_disconnect_playback_controls)
    {
        detach_playback_controls_slot(true);
        mp_detach_playback_controls_Act->setChecked(true);
    }
}


void c_ser_player::fps_changed_slot(QAction *action)
{
    if (action != nullptr) {
        m_display_framerate = action->data().toInt();
        mp_framerate_Menu->setTitle(tr("Display Framerate"));
        calculate_display_framerate();
    }
}


void c_ser_player::playback_controls_double_clicked_slot()
{
    detach_playback_controls_slot(!mp_detach_playback_controls_Act->isChecked());
    mp_detach_playback_controls_Act->setChecked(!mp_detach_playback_controls_Act->isChecked());
}


void c_ser_player::detach_playback_controls_slot(bool detach)
{
    c_persistent_data::m_disconnect_playback_controls = detach;
    int delta_height = mp_playback_controls_widget->height();
    if (detach) {
        mp_main_vlayout->removeWidget(mp_playback_controls_widget);
        mp_playback_controls_dialog->add_controls_widget(mp_playback_controls_widget);
        delta_height = -delta_height;
    } else {
        mp_playback_controls_dialog->remove_controls_widget(mp_playback_controls_widget);
        mp_main_vlayout->insertWidget(1, mp_playback_controls_widget);
    }

    // Resize window to allow for removed/adde controls
    if (windowState() != Qt::WindowMaximized) {
        QSize new_size = size();
        new_size.setHeight(new_size.height() + delta_height);

        QSize frame_border_and_title_size = frameSize() - size();
        QDesktopWidget widget;
        QSize available_desktop_size = widget.availableGeometry().size() - frame_border_and_title_size;
        if (new_size.height() > available_desktop_size.height())
        {
            new_size.setHeight(available_desktop_size.height());
        }

        resize(new_size);
    }

    mp_playback_controls_dialog->move_to_default_position();
}


// Histogram viewer
void c_ser_player::histogram_viewer_slot(bool checked)
{
    if (checked) {
        mp_histogram_dialog->show();
        mp_histogram_dialog->move_to_default_position();
        frame_slider_changed_slot();
    } else {
        mp_histogram_dialog->hide();
    }

    c_persistent_data::m_histogram_enabled = checked;

}


void c_ser_player::header_details_dialog_closed_slot()
{
    mp_header_details_Act->setChecked(false);
}


void c_ser_player::header_details_dialog_slot(bool checked)
{
    mp_header_details_dialog->setVisible(checked);
}


void c_ser_player::histogram_viewer_closed_slot()
{
    mp_histogram_viewer_Act->setChecked(false);
    c_persistent_data::m_histogram_enabled = false;
}


// Processing Options menu QAction has been clicked
void c_ser_player::processing_options_slot(bool checked)
{
    if (checked) {
        mp_processing_options_Dialog->show();
    } else {
        mp_processing_options_Dialog->reject();
    }
}


void c_ser_player::processor_options_closed_slot()
{
    mp_processing_options_Act->setChecked(false);
}


void c_ser_player::playback_controls_closed_slot()
{
    mp_detach_playback_controls_Act->setChecked(false);
    detach_playback_controls_slot(false);
}


void c_ser_player::crop_changed_slot(bool crop_enable, int crop_x, int crop_y, int crop_width, int crop_height)
{
    m_crop_enable = crop_enable;
    m_crop_x_pos = crop_x;
    m_crop_y_pos = crop_y;
    m_crop_width = crop_width;
    m_crop_height = crop_height;

    // Update frame size label
    if (m_crop_enable) {
        mp_playback_controls_widget->update_frame_size_label(m_crop_width, m_crop_height);
    } else {
        mp_playback_controls_widget->update_frame_size_label(mp_ser_file->get_width(), mp_ser_file->get_height());
    }

    frame_slider_changed_slot();
    resize_window_100_percent_slot();
}


void c_ser_player::monochrome_conversion_changed_slot(bool enabled, int selection)
{
    m_monochrome_conversion_enable = enabled;
    m_monochrome_conversion_type = selection;
    frame_slider_changed_slot();
}


void c_ser_player::save_frames_as_ser_slot()
{
    // Pause playback if currently playing
    bool restart_playing = false;
    if (mp_playback_controls_widget->is_playing()) {
        // Pause playing while frame is saved
        restart_playing = true;
        mp_playback_controls_widget->pause_payback();
    }

    // Use save_frames dialog to get range of frames to be saved
    if (mp_save_frames_as_ser_Dialog == nullptr) {
        mp_save_frames_as_ser_Dialog = new c_save_frames_dialog(this,
                                                                c_save_frames_dialog::SAVE_SER,
                                                                mp_ser_file->get_width(),
                                                                mp_ser_file->get_height(),
                                                                m_total_frames,
                                                                mp_ser_file->has_timestamps(),
                                                                0.0,  // SER Framerate only used for AVI generation
                                                                QString::fromStdString(mp_ser_file->get_observer_string()),
                                                                QString::fromStdString(mp_ser_file->get_instrument_string()),
                                                                QString::fromStdString(mp_ser_file->get_telescope_string()));
    }

    mp_save_frames_as_ser_Dialog->set_markers(mp_playback_controls_widget->get_start_frame(),
                                              mp_playback_controls_widget->get_end_frame(),
                                              mp_playback_controls_widget->get_markers_enable());

    if (m_crop_enable) {
        mp_save_frames_as_ser_Dialog->set_processed_frame_size(m_crop_width, m_crop_height);
    } else {
        mp_save_frames_as_ser_Dialog->set_processed_frame_size(mp_ser_file->get_width(), mp_ser_file->get_height());
    }

    int ret = mp_save_frames_as_ser_Dialog->exec();

    if (ret != QDialog::Rejected &&
        m_ser_file_loaded &&
        !mp_playback_controls_widget->is_playing()) {

        int min_frame = mp_save_frames_as_ser_Dialog->get_start_frame();
        int max_frame = mp_save_frames_as_ser_Dialog->get_end_frame();

        QString last_save_directory = mp_save_frames_as_ser_Dialog->get_last_save_directory();
        QString default_filename = QString::fromStdString(mp_ser_file->get_filename());
        if (last_save_directory.length() > 0) {
            default_filename =  QFileInfo(default_filename).fileName();
            default_filename = QDir(last_save_directory).filePath(default_filename);
        }

        int required_digits_for_number = mp_save_frames_as_ser_Dialog->get_required_digits_for_number();

        if (default_filename.endsWith(".ser", Qt::CaseInsensitive)) {
            default_filename.insert(default_filename.length()-4,
                                    QString("_F%1-%2")
                                    .arg(min_frame, required_digits_for_number, 10, QChar('0'))
                                    .arg(max_frame, required_digits_for_number, 10, QChar('0')));
        } else {
            default_filename.append(QString("_F%1-%2")
                                    .arg(min_frame, required_digits_for_number, 10, QChar('0'))
                                    .arg(max_frame, required_digits_for_number, 10, QChar('0')));
            default_filename.append(".ser");
        }

        QString selected_filter;
        QFileDialog::Options save_dialog_options = 0;
        #ifdef __APPLE__
        // The native save file dialog on OS X does not fill out a default filename
        // so we use QT's save file dialog instead
        save_dialog_options |= QFileDialog::DontUseNativeDialog;
        #endif

        QString filename = QFileDialog::getSaveFileName(this, tr("Save Frames As SER File"),
                                   default_filename,
                                   tr("SER Files (*.ser)", "Filetype filter"),
                                   &selected_filter,
                                   save_dialog_options);

        if (!filename.isEmpty()) {
            // Handle the case on Linux where an extension is not added by the save file dialog
            if (!filename.endsWith(".ser", Qt::CaseInsensitive)) {
                filename = filename + ".ser";
            }

            mp_save_frames_as_ser_Dialog->set_last_save_directory(QFileInfo(filename).absolutePath());

            int frame_active_width = mp_save_frames_as_ser_Dialog->get_active_width();
            int frame_active_height = mp_save_frames_as_ser_Dialog->get_active_height();
            int frame_total_width = mp_save_frames_as_ser_Dialog->get_total_width();
            int frame_total_height = mp_save_frames_as_ser_Dialog->get_total_height();
            int decimate_value = mp_save_frames_as_ser_Dialog->get_frame_decimation();
            int sequence_direction = mp_save_frames_as_ser_Dialog->get_sequence_direction();
            int frames_to_be_saved = mp_save_frames_as_ser_Dialog->get_frames_to_be_saved();
            bool include_timestamps = mp_save_frames_as_ser_Dialog->get_include_timestamps_in_ser_file();
            bool do_frame_processing = mp_save_frames_as_ser_Dialog->get_processing_enable();

            c_pipp_ser_write ser_write_file;

            // Keep list of last saved folders up to date
            add_string_to_stringlist(c_persistent_data::m_recent_save_folders, QFileInfo(filename).absolutePath());

            // Update Save Folders Menu
            update_recent_save_folders_menu();

            // Setup progress dialog
            c_save_frames_progress_dialog save_progress_dialog(this, 1, frames_to_be_saved);
            save_progress_dialog.setWindowTitle(tr("Save Frames As SER File"));
            save_progress_dialog.show();

            int saved_frames = 0;
            bool file_create_error = false;
            bool file_write_error = false;

            // Direction loop
            int start_dir = (sequence_direction == 1) ? 1 : 0;
            int end_dir = (sequence_direction == 0) ? 0 : 1;
            bool loop_break = false;
            for (int current_dir = start_dir; current_dir <= end_dir && !loop_break; current_dir++) {
                int start_frame = min_frame;
                int end_frame = max_frame;
                if (current_dir == 1) {  // Reverse direction - count backwards
                    // Use negative numbers so for loop works counting up or down
                    start_frame = -max_frame;
                    end_frame = -min_frame;
                }

                for (int frame_number = start_frame; frame_number <= end_frame && !loop_break; frame_number += decimate_value) {
                    // Update progress bar
                    saved_frames++;
                    save_progress_dialog.set_value(saved_frames);

                    // Get frame from SER file
                    bool valid_frame = get_and_process_frame(abs(frame_number),  // frame_number
                                                             false,  // conv_to_8_bit
                                                             do_frame_processing);  // do_processing

                    mp_frame_image->resize_image(frame_active_width, frame_active_height);
                    mp_frame_image->add_bars(frame_total_width, frame_total_height);

                    if (valid_frame) {
                        // Get timestamp for frame if required
                        uint64_t timestamp = 0;
                        if (include_timestamps) {
                            timestamp = mp_ser_file->get_timestamp();
                        }

                        if (!ser_write_file.get_open()) {
                            // Create SER file - only done once
                            file_create_error |= ser_write_file.create(filename, //  QString filename
                                                                 mp_frame_image->get_width(),  // int32_t  width
                                                                 mp_frame_image->get_height(), // int32_t  height
                                                                 mp_frame_image->get_colour(),  //mp_ser_file->get_colour() != 0,  // bool     colour
                                                                 mp_frame_image->get_byte_depth());  //mp_ser_file->get_byte_depth());  // int32_t  byte_depth
                        }

                        // Write frame to SER file
                        if (!file_create_error && !file_write_error) {
                            file_write_error |= ser_write_file.write_frame(
                                mp_frame_image->get_p_buffer(),  // uint8_t  *data,
                                timestamp);  // uint64_t timestamp);
                        }
                    }

                    if (save_progress_dialog.was_cancelled() || !valid_frame || file_write_error || file_create_error) {
                        // Abort frame saving
                        loop_break = true;
                    }
                }
            }

            // Get timestamp for this frame
            int64_t utc_to_local_diff = 0;
            if (include_timestamps) {
                utc_to_local_diff = mp_ser_file->get_utc_to_local_diff();
            }

            // Set details for SER file
            file_write_error |= ser_write_file.set_details(
                0,                  // int32_t lu_id - always 0
                mp_frame_image->get_colour_id(),  // int32_t colour_id,
                utc_to_local_diff,  // int64_t utc_to_local_diff,
                mp_save_frames_as_ser_Dialog->get_observer_string(),
                mp_save_frames_as_ser_Dialog->get_instrument_string(),
                mp_save_frames_as_ser_Dialog->get_telescope_string());

            // Write header and close SER file
            file_write_error |= ser_write_file.close();

            save_progress_dialog.set_complete();

            if (!file_create_error && !file_write_error) {
                // Processing has completed with no file error
                while (!save_progress_dialog.was_cancelled()) {
                      // Wait
                }
            } else {
                // There was a file error
                save_progress_dialog.hide();
                QString error_message;
                if (file_create_error) {
                    error_message = tr("Error: SER File creation failed");
                } else if (file_write_error) {
                    error_message = tr("Error: SER file writing failed");
                }

                QMessageBox::critical(
                    this,
                    tr("Save Frames As SER File Failed"),
                    error_message);
            }
        }
    }

    // Restart playing if it was playing to start with
    if (restart_playing == true) {
        mp_playback_controls_widget->start_playback();
    }

    return;
}


void c_ser_player::save_frames_as_avi_slot()
{
    // Pause playback if currently playing
    bool restart_playing = false;
    if (mp_playback_controls_widget->is_playing()) {
        // Pause playing while frame is saved
        restart_playing = true;
        mp_playback_controls_widget->pause_payback();
    }

    // Use save_frames dialog to get range of frames to be saved
    double ser_framerate = 0.0;
    if (mp_ser_file->get_fps_rate() > 0 && mp_ser_file->get_fps_scale()) {
        ser_framerate = (double)(mp_ser_file->get_fps_rate()) / (double)(mp_ser_file->get_fps_scale());
    }

    if (mp_save_frames_as_avi_Dialog == nullptr) {
        mp_save_frames_as_avi_Dialog = new c_save_frames_dialog(this,
                                                                c_save_frames_dialog::SAVE_AVI,
                                                                mp_ser_file->get_width(),
                                                                mp_ser_file->get_height(),
                                                                m_total_frames,
                                                                mp_ser_file->has_timestamps(),
                                                                ser_framerate);
    }

    mp_save_frames_as_avi_Dialog->set_markers(mp_playback_controls_widget->get_start_frame(),
                                              mp_playback_controls_widget->get_end_frame(),
                                              mp_playback_controls_widget->get_markers_enable());

    if (m_crop_enable) {
        mp_save_frames_as_avi_Dialog->set_processed_frame_size(m_crop_width, m_crop_height);
    } else {
        mp_save_frames_as_avi_Dialog->set_processed_frame_size(mp_ser_file->get_width(), mp_ser_file->get_height());
    }

    int ret = mp_save_frames_as_avi_Dialog->exec();

    if (ret != QDialog::Rejected &&
        m_ser_file_loaded &&
        !mp_playback_controls_widget->is_playing()) {

        int min_frame = mp_save_frames_as_avi_Dialog->get_start_frame();
        int max_frame = mp_save_frames_as_avi_Dialog->get_end_frame();

        QString last_save_directory = mp_save_frames_as_avi_Dialog->get_last_save_directory();
        QString default_filename = QString::fromStdString(mp_ser_file->get_filename());
        if (last_save_directory.length() > 0) {
            default_filename =  QFileInfo(default_filename).fileName();
            default_filename = QDir(last_save_directory).filePath(default_filename);
        }

        int required_digits_for_number = mp_save_frames_as_avi_Dialog->get_required_digits_for_number();

        if (default_filename.endsWith(".avi", Qt::CaseInsensitive)) {
            default_filename.insert(default_filename.length()-4,
                                    QString("_F%1-%2")
                                    .arg(min_frame, required_digits_for_number, 10, QChar('0'))
                                    .arg(max_frame, required_digits_for_number, 10, QChar('0')));
        } else {
            default_filename.append(QString("_F%1-%2")
                                    .arg(min_frame, required_digits_for_number, 10, QChar('0'))
                                    .arg(max_frame, required_digits_for_number, 10, QChar('0')));
            default_filename.append(".avi");
        }

        QString selected_filter;
        QFileDialog::Options save_dialog_options = 0;
        #ifdef __APPLE__
        // The native save file dialog on OS X does not fill out a default filename
        // so we use QT's save file dialog instead
        save_dialog_options |= QFileDialog::DontUseNativeDialog;
        #endif

        QString filename = QFileDialog::getSaveFileName(this, tr("Save Frames As AVI File"),
                                   default_filename,
                                   tr("AVI Files (*.avi)", "Filetype filter"),
                                   &selected_filter,
                                   save_dialog_options);

        if (!filename.isEmpty()) {
            // Handle the case on Linux where an extension is not added by the save file dialog
            if (!filename.endsWith(".avi", Qt::CaseInsensitive)) {
                filename = filename + ".avi";
            }

            mp_save_frames_as_avi_Dialog->set_last_save_directory(QFileInfo(filename).absolutePath());

            int frame_active_width = mp_save_frames_as_avi_Dialog->get_active_width();
            int frame_active_height = mp_save_frames_as_avi_Dialog->get_active_height();
            int frame_total_width = mp_save_frames_as_avi_Dialog->get_total_width();
            int frame_total_height = mp_save_frames_as_avi_Dialog->get_total_height();
            int decimate_value = mp_save_frames_as_avi_Dialog->get_frame_decimation();
            int sequence_direction = mp_save_frames_as_avi_Dialog->get_sequence_direction();
            int frames_to_be_saved = mp_save_frames_as_avi_Dialog->get_frames_to_be_saved();
            bool do_frame_processing = mp_save_frames_as_avi_Dialog->get_processing_enable();
            double avi_framerate = mp_save_frames_as_avi_Dialog->get_avi_framerate();

            int32_t old_format = 0;
            if (mp_save_frames_as_avi_Dialog->get_avi_old_format()) {
                old_format = mp_save_frames_as_avi_Dialog->get_avi_max_size();
            }

            int32_t fps_rate = avi_framerate * 1000;
            int32_t fps_scale = 1000;
            while (fps_scale > 1) {
                if (fps_rate % 10 == 0) {
                    fps_rate /= 10;
                    fps_scale /= 10;
                } else {
                    break;
                }
            }

            c_pipp_video_write *p_avi_write_file = new c_pipp_avi_write_dib();

            // Keep list of last saved folders up to date
            add_string_to_stringlist(c_persistent_data::m_recent_save_folders, QFileInfo(filename).absolutePath());

            // Update Save Folders Menu
            update_recent_save_folders_menu();

            // Setup progress dialog
            c_save_frames_progress_dialog save_progress_dialog(this, 1, frames_to_be_saved);
            save_progress_dialog.setWindowTitle(tr("Save Frames As AVI File"));
            save_progress_dialog.show();

            int saved_frames = 0;
            bool file_create_error = false;
            bool file_write_error = false;

            // Direction loop
            int start_dir = (sequence_direction == 1) ? 1 : 0;
            int end_dir = (sequence_direction == 0) ? 0 : 1;
            bool loop_break = false;
            for(int current_dir = start_dir; current_dir <= end_dir && !loop_break; current_dir++) {
                int start_frame = min_frame;
                int end_frame = max_frame;
                if (current_dir == 1) {  // Reverse direction - count backwards
                    // Use negative numbers so for loop works counting up or down
                    start_frame = -max_frame;
                    end_frame = -min_frame;
                }

                for (int frame_number = start_frame; frame_number <= end_frame && !loop_break; frame_number += decimate_value) {
                    // Update progress bar
                    saved_frames++;
                    save_progress_dialog.set_value(saved_frames);

                    // Get frame from SER file
                    bool valid_frame = get_and_process_frame(abs(frame_number),  // frame_number
                                                             false,  // conv_to_8_bit
                                                             do_frame_processing);  // do_processing

                    mp_frame_image->resize_image(frame_active_width, frame_active_height);
                    mp_frame_image->add_bars(frame_total_width, frame_total_height);

                    if (valid_frame) {
                        if (!p_avi_write_file->get_open()) {
                            // Create AVI file - only done once
                            file_create_error |= p_avi_write_file->create(
                                filename.toUtf8().constData(),  // const char *filename
                                mp_frame_image->get_width(),  // int32_t m_width
                                mp_frame_image->get_height(),  // int32_t m_height
                                mp_frame_image->get_colour(),  // bool m_colour
                                fps_rate,  // int32_t fps_rate
                                fps_scale, // int32_t fps_scale
                                old_format,  // int32_t m_old_avi_format
                                0);  // int32_t quality
                        }


                        // Write frame to AVI file
                        if (!file_write_error) {
                            file_write_error |= p_avi_write_file->write_frame(
                                mp_frame_image->get_p_buffer(),  // uint8_t *data
                                0,  // int32_t m_colour
                                mp_frame_image->get_byte_depth());  // uint32_t bpp
                        }
                    }

                    if (save_progress_dialog.was_cancelled() || !valid_frame || file_write_error || file_create_error) {
                        // Abort frame saving
                        loop_break = true;
                    }
                }
            }

            // Write header and close SER file
            file_write_error |= p_avi_write_file->close();

            if (!file_write_error && !file_create_error) {
                // Processing has completed with no file error
                save_progress_dialog.set_complete();
                while (!save_progress_dialog.was_cancelled()) {
                      // Wait
                }
            } else {
                // There was a file error
                save_progress_dialog.set_complete();
                save_progress_dialog.hide();
                QString error_message;
                if (file_create_error) {
                    error_message = tr("Error: AVI file creation failed");
                } else if (file_write_error) {
                    error_message = tr("Error: AVI file writing failed");
                }

                QMessageBox::critical(
                    this,
                    tr("Save Frames As AVI File Failed"),
                    error_message);
            }

            delete p_avi_write_file;
        }
    }

    // Restart playing if it was playing to start with
    if (restart_playing == true) {
        mp_playback_controls_widget->start_playback();
    }
}


void c_ser_player::save_frames_as_gif_slot()
{
    // Pause playback if currently playing
    bool restart_playing = false;
    if (mp_playback_controls_widget->is_playing()) {
        // Pause playing while frame is saved
        restart_playing = true;
        mp_playback_controls_widget->pause_payback();
    }

    // Use save_frames dialog to get range of frames to be saved
    if (mp_save_frames_as_gif_Dialog == nullptr) {
        mp_save_frames_as_gif_Dialog = new c_save_frames_dialog(this,
                                                                c_save_frames_dialog::SAVE_GIF,
                                                                mp_ser_file->get_width(),
                                                                mp_ser_file->get_height(),
                                                                m_total_frames,
                                                                mp_ser_file->has_timestamps());

        double gif_frame_time;
        if (mp_ser_file->get_fps_rate() > 0) {
            gif_frame_time = (double)mp_ser_file->get_fps_scale() / mp_ser_file->get_fps_rate();
        } else {
            gif_frame_time = 0.2;
        }

        mp_save_frames_as_gif_Dialog->set_gif_frametime(gif_frame_time);
    }

    mp_save_frames_as_gif_Dialog->set_colour_details(m_is_colour,
                                                     mp_processing_options_Dialog->get_processed_data_is_colour());

    if (m_crop_enable) {
        mp_save_frames_as_gif_Dialog->set_processed_frame_size(m_crop_width, m_crop_height);
    } else {
        mp_save_frames_as_gif_Dialog->set_processed_frame_size(mp_ser_file->get_width(), mp_ser_file->get_height());
    }

    mp_save_frames_as_gif_Dialog->set_markers(mp_playback_controls_widget->get_start_frame(),
                                              mp_playback_controls_widget->get_end_frame(),
                                              mp_playback_controls_widget->get_markers_enable());

    bool is_test_run = false;
    int save_frames_dialog_ret = QDialog::Rejected;
    QTemporaryFile temp_html_file(QDir::tempPath() + "//XXXXXX.html");
    QString temp_gif_filename = QDir::tempPath() + "//ser_player_review.gif";
    uint64_t filesize_after_first_frame = 0;
    uint64_t filesize_after_last_frame = 0;
    uint64_t final_filesize = 0;
    int written_framecount = 0;
    do {  // Loop for doing GIF animation test runs
        save_frames_dialog_ret = mp_save_frames_as_gif_Dialog->exec();  // Show dialog

        int unchanged_border_tolerance;
        bool transparent_pixel_enable;
        int transparent_pixel_tolerence;
        int lossy_compression_level;
        int pixel_depth;

        if (save_frames_dialog_ret != QDialog::Rejected &&
            m_ser_file_loaded &&
            !mp_playback_controls_widget->is_playing()) {

            is_test_run = mp_save_frames_as_gif_Dialog->get_gif_test_run();  // Is this just a test run?

            int min_frame = mp_save_frames_as_gif_Dialog->get_start_frame();
            int max_frame = mp_save_frames_as_gif_Dialog->get_end_frame();

            QString last_save_directory = mp_save_frames_as_gif_Dialog->get_last_save_directory();
            QString default_filename = QString::fromStdString(mp_ser_file->get_filename());
            if (last_save_directory.length() > 0) {
                default_filename =  QFileInfo(default_filename).fileName();
                default_filename = QDir(last_save_directory).filePath(default_filename);
            }

            int required_digits_for_number = mp_save_frames_as_gif_Dialog->get_required_digits_for_number();

            if (default_filename.endsWith(".ser", Qt::CaseInsensitive)) {
                // Remove .ser extension
                default_filename.chop(4);
            }

            // Add frame details
            default_filename.append(QString("_F%1-%2")
                                    .arg(min_frame, required_digits_for_number, 10, QChar('0'))
                                    .arg(max_frame, required_digits_for_number, 10, QChar('0')));

            // Add file extension
            default_filename.append(".gif");

            QString gif_filename;
            if (is_test_run) {
                gif_filename = temp_gif_filename;
            } else {
                // This is a real save, not a test run
                QString selected_filter;
                QFileDialog::Options save_dialog_options = 0;
                #ifdef __APPLE__
                // The native save file dialog on OS X does not fill out a default filename
                // so we use QT's save file dialog instead
                save_dialog_options |= QFileDialog::DontUseNativeDialog;
                #endif

                gif_filename = QFileDialog::getSaveFileName(this, tr("Save Frames As Animated GIF"),
                                           default_filename,
                                           tr("GIF Files (*.gif)", "Filetype filter"),
                                           &selected_filter,
                                           save_dialog_options);
            }

            if (!gif_filename.isEmpty()) {
                c_gif_write gif_write_file;
                // Handle the case on Linux where an extension is not added by the save file dialog
                if (!gif_filename.endsWith(".gif", Qt::CaseInsensitive)) {
                    gif_filename += ".gif";
                }

                if (!is_test_run) {
                    mp_save_frames_as_gif_Dialog->set_last_save_directory(QFileInfo(gif_filename).absolutePath());
                }

                bool do_frame_processing = mp_save_frames_as_gif_Dialog->get_processing_enable();
                int frames_to_be_saved = mp_save_frames_as_gif_Dialog->get_frames_to_be_saved();
                int frame_active_width = mp_save_frames_as_gif_Dialog->get_active_width();
                int frame_active_height = mp_save_frames_as_gif_Dialog->get_active_height();
                int frame_total_width = mp_save_frames_as_gif_Dialog->get_total_width();
                int frame_total_height = mp_save_frames_as_gif_Dialog->get_total_height();
                int sequence_direction = mp_save_frames_as_gif_Dialog->get_sequence_direction();
                int decimate_value = mp_save_frames_as_gif_Dialog->get_frame_decimation();
                int frametime = 100 * mp_save_frames_as_gif_Dialog->get_gif_frametime();
                int final_frametime = 100 * mp_save_frames_as_gif_Dialog->get_gif_final_frametime();
                unchanged_border_tolerance = mp_save_frames_as_gif_Dialog->get_gif_unchanged_border_tolerance();
                transparent_pixel_enable = mp_save_frames_as_gif_Dialog->get_gif_transparent_pixel_enable();
                int colour_quantisation_type = mp_save_frames_as_gif_Dialog->get_gif_colour_quantisation_type();
                transparent_pixel_tolerence = mp_save_frames_as_gif_Dialog->get_gif_transparent_pixel_tolerance();
                lossy_compression_level = mp_save_frames_as_gif_Dialog->get_gif_lossy_compression_level();
                pixel_depth = mp_save_frames_as_gif_Dialog->get_gif_pixel_bit_depth();

                if (!is_test_run) {
                    // Keep list of last saved folders up to date if this is not a test run
                    add_string_to_stringlist(c_persistent_data::m_recent_save_folders, QFileInfo(gif_filename).absolutePath());

                    // Update Save Folders Menu
                    update_recent_save_folders_menu();
                }

                // Setup progress dialog
                c_save_frames_progress_dialog save_progress_dialog(this, 1, frames_to_be_saved);
                save_progress_dialog.setWindowTitle(tr("Save Frames As Animated GIF"));
                if (is_test_run) {
                    // Change button label from 'Abort' to 'Truncate' when doing a test run
                    save_progress_dialog.set_button_label(tr("Truncate"));
                }

                save_progress_dialog.show();

                int saved_frames = 0;
                bool file_create_error = false;
                bool file_write_error = false;

                // Direction loop
                int start_dir = (sequence_direction == 1) ? 1 : 0;
                int end_dir = (sequence_direction == 0) ? 0 : 1;
                bool loop_break = false;
                for (int current_dir = start_dir; current_dir <= end_dir && !loop_break; current_dir++) {
                    int start_frame = min_frame;
                    int end_frame = max_frame;
                    if (current_dir == 1) {  // Reverse direction - count backwards
                        // Use negative numbers so for loop works counting up or down
                        start_frame = -max_frame;
                        end_frame = -min_frame;
                    }

                    for (int frame_number = start_frame; frame_number <= end_frame && !loop_break; frame_number += decimate_value) {
                        // Update progress bar
                        saved_frames++;
                        save_progress_dialog.set_value(saved_frames);

                        // Get frame from SER file
                        bool valid_frame = get_and_process_frame(abs(frame_number),  // frame_number
                                                                 false,  // conv_to_8_bit
                                                                 do_frame_processing);  // do_processing

                        if (valid_frame) {
                            mp_frame_image->resize_image(frame_active_width, frame_active_height);
                            mp_frame_image->add_bars(frame_total_width, frame_total_height);
                            mp_frame_image->conv_data_ready_for_gif();

                            if (!gif_write_file.is_open()) {
                                file_create_error |= gif_write_file.create(
                                        gif_filename,  // const QString &filename
                                        frame_total_width,  // int width
                                        frame_total_height,  // int height
                                        mp_frame_image->get_byte_depth(), // int byte_depth
                                        mp_frame_image->get_colour(), // bool colour
                                        0,  // int repeat_count
                                        (c_gif_write::e_colour_quant_type)colour_quantisation_type,
                                        unchanged_border_tolerance, // int unchanged_border_tolerance
                                        transparent_pixel_enable,  // bool use_transparent_pixels
                                        transparent_pixel_tolerence, // int transparent_tolerence
                                        lossy_compression_level,  // int lossy_compression_level
                                        pixel_depth);  // int bit_depth

                                filesize_after_first_frame = 0;
                                written_framecount = 0;
                            }

                            if (saved_frames == frames_to_be_saved) {
                                // Use final frame time for last frame
                                frametime = final_frametime;
                            }

                            if (!file_write_error && !file_create_error) {
                                written_framecount++;
                                file_write_error |= gif_write_file.write_frame(
                                          mp_frame_image->get_p_buffer(),  // uint8_t  *p_data
                                          frametime);  // uint16_t display_time
                            }

                            if (filesize_after_first_frame == 0) {
                                filesize_after_first_frame = gif_write_file.get_current_filesize();
                            }
                        }

                        if (save_progress_dialog.was_cancelled() || !valid_frame || file_write_error || file_create_error) {
                            // Abort frame saving
                            loop_break = true;
                        }
                    }
                }

                // Close file
                filesize_after_last_frame = gif_write_file.get_current_filesize();
                final_filesize = gif_write_file.close();

                // Processing has completed
                save_progress_dialog.set_complete();


                if (!file_create_error && !file_write_error) {
                    // Wait until close buttin is pressed for non-test runs
                    if (!is_test_run) {
                        // Processing has completed with no file error
                        while (!save_progress_dialog.was_cancelled()) {
                              // Wait
                        }
                    }
                } else {
                    // There was a file error
                    save_progress_dialog.hide();
                    QString error_message;
                    if (file_create_error) {
                        error_message = tr("Error: Animated GIF file creation failed");
                    } else if (file_write_error) {
                        error_message = tr("Error: Animated GIF file writing failed");
                    }

                    QMessageBox::critical(
                        this,
                        tr("Save Frames As Animated GIF Failed"),
                        error_message);
                }

                if (is_test_run && !file_create_error && !file_write_error) {
                    // Create an HTML file with animated GIF and options details
                    if (temp_html_file.open()) {
                        temp_html_file.resize(0);  // Clear any file contents
                        QTextStream stream(&temp_html_file);
                        stream << "<!DOCTYPE html>" << endl;
                        stream << "<html>" << endl;
                        stream << "<body>" << endl;
                        stream << "<p>" << endl;
                        stream << "<table><tr><td>" << endl;
                        stream << "<b><big>" << tr("SER Player Animated GIF Review") << "</big></b><br>" << endl;
                        stream << "<b>" << tr("(Close browser when reviewing is complete)") << " </b><br>" << endl;

                        stream << tr("Frame Delay: ") << mp_save_frames_as_gif_Dialog->get_gif_frametime() << " s<br>" << endl;
                        stream << tr("Final Frame Delay: ") << mp_save_frames_as_gif_Dialog->get_gif_final_frametime() << " s<br>" << endl;

                        stream << tr("Colour Quantisation: ") << mp_save_frames_as_gif_Dialog->get_gif_colour_quantisation_name() << "<br>" << endl;

                        stream << tr("Unchanged Border Tolerance: ") << unchanged_border_tolerance << "<br>" << endl;

                        if (transparent_pixel_enable) {
                            stream << tr("Transperant Pixel Tolerance: ") << transparent_pixel_tolerence << "<br>" << endl;
                        } else {
                            stream << tr("Transperant Pixel Tolerance: Disabled") << "<br>" << endl;
                        }

                        if (pixel_depth < 8) {
                            stream << tr("Reduced Pixel Depth: ") << pixel_depth << "<br>" << endl;
                        } else {
                            stream << tr("Reduced Pixel Depth: Disabled") << "<br>" << endl;
                        }

                        if (lossy_compression_level > 0) {
                            stream << tr("Lossy Compression Level: ") << lossy_compression_level << "<br>" << endl;
                        } else {
                            stream << tr("Lossy Compression Level: Disabled") << "<br>" << endl;
                        }

                        stream << "<hr>" << endl;
                        stream << "<b>" << tr("Frames Saved: %1 of %2").arg(written_framecount).arg(frames_to_be_saved) << "</b><br>" << endl;

                        uint32_t filesize;
                        if (written_framecount < frames_to_be_saved) {
                            uint32_t size_of_average_frame = (uint32_t)(filesize_after_last_frame - filesize_after_first_frame);
                            size_of_average_frame /= (written_framecount - 1);
                            uint32_t size_after_last_frame = (uint32_t)(final_filesize - filesize_after_last_frame);
                            filesize = filesize_after_first_frame +
                                       size_of_average_frame  * (frames_to_be_saved - 1) +
                                       size_after_last_frame;
                        } else {
                            filesize = (uint32_t)final_filesize;
                        }

                        if (filesize > 1024 * 1024) {
                            double filesize_mb = (double)filesize / (1024 * 1024);
                            filesize_mb = (floor(filesize_mb * 100)) / 100;  // Round to 2 decimal places
                            if (written_framecount < frames_to_be_saved) {
                                stream << "<b>" << tr("Estimated Filesize: %1 MB (%2 Bytes)").arg(filesize_mb).arg(filesize) << "</b><br>" << endl;
                            } else {

                                stream << "<b>" << tr("Filesize: %1 MB (%2 Bytes)").arg(filesize_mb).arg(filesize) << "</b><br>" << endl;
                            }
                        } else if (filesize > 1024) {
                           double filesize_kb = (double)filesize / 1024;
                           filesize_kb = (floor(filesize_kb * 100)) / 100;  // Round to 2 decimal places
                           if (written_framecount < frames_to_be_saved) {
                               stream << "<b>" << tr("Estimated Filesize: %1 KB (%2 Bytes)").arg(filesize_kb).arg(filesize) << "</b><br>" << endl;
                           } else {
                               stream << "<b>" << tr("Filesize: %1 KB (%2 Bytes)").arg(filesize_kb).arg(filesize) << "</b><br>" << endl;
                           }
                        } else {
                            if (written_framecount < frames_to_be_saved) {
                                stream << "<b>" << tr("Estimated Filesize: %1 Bytes").arg(filesize) << "</b><br>" << endl;
                            } else {
                                stream << "<b>" << tr("Filesize: %1 Bytes").arg(filesize) << "</b><br>" << endl;
                            }
                        }

                        stream << "</td></tr></table>" << endl;
                        stream << "</p>" << endl;
                        stream << "<img src=\"file:///" << temp_gif_filename << "\">" << endl;

                        stream << "</body>" << endl;
                        stream << "</html>" << endl;
                        temp_html_file.close();
                    }

                    QDesktopServices::openUrl(QUrl::fromLocalFile(temp_html_file.fileName()));
                }
            }
        }
    } while (is_test_run && save_frames_dialog_ret != QDialog::Rejected);

    // Remove temp GIF file if it exists
    if (QFileInfo (temp_gif_filename).exists() && QFileInfo (temp_gif_filename).isFile()) {
        QFile::remove(temp_gif_filename);
    }

    // Restart playing if it was playing to start with
    if (restart_playing == true) {
        mp_playback_controls_widget->start_playback();
    }
}


void c_ser_player::save_frames_as_images_slot()
{
    // Pause playback if currently playing
    bool restart_playing = false;
    if (mp_playback_controls_widget->is_playing()) {
        // Pause playing while frame is saved
        restart_playing = true;
        mp_playback_controls_widget->pause_payback();
    }

    // Use save_frames dialog to get range of frames to be saved
    if (mp_save_frames_as_images_Dialog == nullptr) {
        mp_save_frames_as_images_Dialog = new c_save_frames_dialog(this,
                                                                   c_save_frames_dialog::SAVE_IMAGES,
                                                                   mp_ser_file->get_width(),
                                                                   mp_ser_file->get_height(),
                                                                   m_total_frames,
                                                                   mp_ser_file->has_timestamps());
    }

    mp_save_frames_as_images_Dialog->set_markers(mp_playback_controls_widget->get_start_frame(),
                                              mp_playback_controls_widget->get_end_frame(),
                                              mp_playback_controls_widget->get_markers_enable());

    if (m_crop_enable) {
        mp_save_frames_as_images_Dialog->set_processed_frame_size(m_crop_width, m_crop_height);
    } else {
        mp_save_frames_as_images_Dialog->set_processed_frame_size(mp_ser_file->get_width(), mp_ser_file->get_height());
    }

    int ret = mp_save_frames_as_images_Dialog->exec();

    if (ret != QDialog::Rejected &&
        m_ser_file_loaded &&
        !mp_playback_controls_widget->is_playing()) {

        // Get image filename and type to use
        QString save_directory = mp_save_frames_as_images_Dialog->get_last_save_directory();
        if (save_directory.length() == 0)
        {
            save_directory = m_ser_directory;
        }

        const QString jpg_ext = QString(tr(".jpg"));
        const QString jpg_filter = QString(tr("Joint Picture Expert Group Image (*.jpg)", "Filetype filter"));
        const QString bmp_ext = QString(tr(".bmp"));
        const QString bmp_filter = QString(tr("Windows Bitmap Image (*.bmp)", "Filetype filter"));
        const QString png_ext = QString(tr(".png"));
        const QString png_filter = QString(tr("Portable Network Graphics Image (*.png)", "Filetype filter"));
        const QString tif_ext = QString(tr(".tif"));
        const QString tif_filter = QString(tr("Tagged Image File Format (*.tif)", "Filetype filter"));
        QString selected_filter;
        QString selected_ext;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Frames As Images"),
                                   save_directory,
                                   jpg_filter + ";; " + bmp_filter + ";; " + png_filter + ";; " + tif_filter,
                                   &selected_filter);
        const char *p_format = nullptr;
        if (!filename.isEmpty() && !selected_filter.isEmpty()) {
            if (selected_filter == jpg_filter) {
                p_format = "JPG";
                selected_ext = jpg_ext;
            }

            if (selected_filter == bmp_filter) {
                p_format = "BMP";
                selected_ext = bmp_ext;
            }

            if (selected_filter == png_filter) {
                p_format = "PNG";
                selected_ext = png_ext;
            }

            if (selected_filter == tif_filter) {
                p_format = "TIFF";
                selected_ext = tif_ext;
            }

            // Handle the case on Linux where an extension is not added by the save file dialog
            if (!filename.endsWith(selected_ext, Qt::CaseInsensitive)) {
                filename = filename + selected_ext;
            }

            mp_save_frames_as_images_Dialog->set_last_save_directory(QFileInfo(filename).absolutePath());

            int frame_active_width = mp_save_frames_as_images_Dialog->get_active_width();
            int frame_active_height = mp_save_frames_as_images_Dialog->get_active_height();
            int frame_total_width = mp_save_frames_as_images_Dialog->get_total_width();
            int frame_total_height = mp_save_frames_as_images_Dialog->get_total_height();
            int min_frame = mp_save_frames_as_images_Dialog->get_start_frame();
            int max_frame = mp_save_frames_as_images_Dialog->get_end_frame();
            int decimate_value = mp_save_frames_as_images_Dialog->get_frame_decimation();
            int sequence_direction = mp_save_frames_as_images_Dialog->get_sequence_direction();
            int frames_to_be_saved = mp_save_frames_as_images_Dialog->get_frames_to_be_saved();
            bool use_framenumber_in_name = mp_save_frames_as_images_Dialog->get_use_framenumber_in_name();
            bool append_timestamp_to_filename = mp_save_frames_as_images_Dialog->get_append_timestamp_to_filename();
            int required_digits_for_number = mp_save_frames_as_images_Dialog->get_required_digits_for_number();
            bool do_frame_processing = mp_save_frames_as_images_Dialog->get_processing_enable();

            // Keep list of last saved folders up to date
            add_string_to_stringlist(c_persistent_data::m_recent_save_folders, QFileInfo(filename).absolutePath());

            // Update Save Folders Menu
            update_recent_save_folders_menu();

            QString save_folder = QFileInfo(filename).absolutePath();

            if (min_frame == -1) {
                // Save current frame only
                // Get frame from ser file
                bool valid_frame = get_and_process_frame(mp_playback_controls_widget->slider_value(),  // frame_number
                                                       true,  // conv_to_8_bit
                                                       do_frame_processing);  // do_processing
                if (valid_frame) {
                    mp_frame_image->resize_image(frame_active_width, frame_active_height);
                    mp_frame_image->add_bars(frame_total_width, frame_total_height);
                    mp_frame_image->conv_data_ready_for_qimage();

                    QImage save_qimage = QImage(mp_frame_image->get_p_buffer(),
                                                mp_frame_image->get_width(),
                                                mp_frame_image->get_height(),
                                                QImage::Format_RGB888);

                    QFile file(filename);
                    file.open(QIODevice::WriteOnly);
                    QPixmap::fromImage(save_qimage).save(&file, p_format);
                    file.close();
                }
            } else {
                // Save the range of frames specified by min_frame and max_frame
                QString filename_without_extension = QFileInfo(filename).completeBaseName();
                QString filename_extension = QFileInfo(filename).suffix();

                // Setup progress dialog
                c_save_frames_progress_dialog save_progress_dialog(this, 1, frames_to_be_saved);
                save_progress_dialog.show();

                int saved_frames = 0;
                QString timestamp_string = "";

                // Direction loop
                int start_dir = (sequence_direction == 1) ? 1 : 0;
                int end_dir = (sequence_direction == 0) ? 0 : 1;
                for(int current_dir = start_dir; current_dir <= end_dir; current_dir++) {
                    int start_frame = min_frame;
                    int end_frame = max_frame;
                    if (current_dir == 1) {  // Reverse direction - count backwards
                        // Use negative numbers so for loop works counting up or down
                        start_frame = -max_frame;
                        end_frame = -min_frame;
                    }

                    for (int frame_number = start_frame; frame_number <= end_frame; frame_number += decimate_value) {
                        // Update progress bar
                        saved_frames++;
                        save_progress_dialog.set_value(saved_frames);

                        // Get frame from SER file
                        bool valid_frame = get_and_process_frame(abs(frame_number),  // frame_number
                                                               true,  // conv_to_8_bit
                                                               do_frame_processing);  // do_processing
                        if (valid_frame) {
                            mp_frame_image->resize_image(frame_active_width, frame_active_height);
                            mp_frame_image->add_bars(frame_total_width, frame_total_height);
                            mp_frame_image->conv_data_ready_for_qimage();

                            QImage save_qimage = QImage(mp_frame_image->get_p_buffer(),
                                                        mp_frame_image->get_width(),
                                                        mp_frame_image->get_height(),
                                                        QImage::Format_RGB888);

                            // Get timestamp for frame if required
                            if (append_timestamp_to_filename) {
                                uint64_t ts = mp_ser_file->get_timestamp();
                                timestamp_string = "_" + QString::number(ts);
                                if (ts > 0) {
                                    int32_t ts_year, ts_month, ts_day, ts_hour, ts_minute, ts_second, ts_microsec;
                                    c_pipp_timestamp::timestamp_to_date(
                                        ts,
                                        &ts_year,
                                        &ts_month,
                                        &ts_day,
                                        &ts_hour,
                                        &ts_minute,
                                        &ts_second,
                                        &ts_microsec);
                                    int32_t ts_millisec = ts_microsec / 1000;
                                    timestamp_string = QString("_%1%2%3_%4%5%6.%7_UT")
                                                       .arg(ts_year, 4, 10, QLatin1Char( '0' ))
                                                       .arg(ts_month, 2, 10, QLatin1Char( '0' ))
                                                       .arg(ts_day, 2, 10, QLatin1Char( '0' ))
                                                       .arg(ts_hour, 2, 10, QLatin1Char( '0' ))
                                                       .arg(ts_minute, 2, 10, QLatin1Char( '0' ))
                                                       .arg(ts_second, 2, 10, QLatin1Char( '0' ))
                                                       .arg(ts_millisec, 3, 10, QLatin1Char( '0' ));
                                } else {
                                    timestamp_string = tr("_no_timestamp", "Appended to save filename when no timestamp is available");
                                }
                            }

                            // Insert frame number into filename
                            int number_for_filename = (use_framenumber_in_name) ? abs(frame_number) : saved_frames;
                            QString frame_number_string = QString("%1").arg(number_for_filename, required_digits_for_number, 10, QChar('0'));
                            QString new_filename = save_folder +
                                                   QDir::separator() +
                                                   filename_without_extension +
                                                   QString("_") +
                                                   frame_number_string +
                                                   timestamp_string +
                                                   "." +
                                                   filename_extension;

                            // Open file for writing
                            QFile file(new_filename);
                            file.open(QIODevice::WriteOnly);

                            // Save the frame and close image file
                            QPixmap::fromImage(save_qimage).save(&file, p_format);
                            file.close();
                        }

                        if (save_progress_dialog.was_cancelled() || !valid_frame) {
                            // Abort frame saving
                            break;
                        }
                    }
                }

                // Processing has completed
                save_progress_dialog.set_complete();
                while (!save_progress_dialog.was_cancelled()) {
                      // Wait
                }

            }
        }
    }

    // Restart playing if it was playing to start with
    if (restart_playing == true) {
        mp_playback_controls_widget->start_playback();
    }
}


void c_ser_player::open_save_folder_slot(QAction *action)
{
    if (action != nullptr) {
        // Open folder
        QString dir = action->data().toString();
        if (dir.isEmpty()) {
            // Clear all entries from the list
            c_persistent_data::m_recent_save_folders.clear();
            update_recent_save_folders_menu();
        } else if (QDir(dir).exists()) {
            // Open
            QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
        } else {
            // Folder does not exist
            QMessageBox::warning(nullptr,
                                 tr("Cannot Open Last Save Folder", "Message box title for canot open last save folder"),
                                 tr("Folder Not Found:", "Message box title for cannot open last save folder") + "\n" + dir);
            c_persistent_data::m_recent_save_folders.removeAll(dir);
            update_recent_save_folders_menu();
        }
    }
}


void c_ser_player::invert_changed_slot(bool invert)
{
    mp_frame_image->set_invert_image(invert);
    frame_slider_changed_slot();
}


void c_ser_player::gain_changed_slot(double gain)
{
    mp_frame_image->set_gain(gain);
    frame_slider_changed_slot();
}


void c_ser_player::gamma_changed_slot(double gamma)
{
    mp_frame_image->set_gamma(gamma);
    frame_slider_changed_slot();
}


void c_ser_player::colour_balance_changed_slot(double red, double green, double blue)
{
    mp_frame_image->set_colour_balance(red, green, blue);
    frame_slider_changed_slot();
}


void c_ser_player::colour_align_changed_slot(
        int red_align_x,
        int red_align_y,
        int blue_align_x,
        int blue_align_y)
{
    mp_frame_image->set_colour_align(red_align_x, red_align_y, blue_align_x, blue_align_y);
    frame_slider_changed_slot();
}


void c_ser_player::estimate_colour_balance()
{
    if (m_ser_file_loaded) {
        // Get frame from SER file
        bool is_colour = false;
        if (mp_ser_file->get_colour_id() == COLOURID_RGB || mp_ser_file->get_colour_id() == COLOURID_BGR) {
            is_colour = true;
        }

        mp_frame_image->set_image_details(
                    mp_ser_file->get_width(),  // width
                    mp_ser_file->get_height(),  // height
                    mp_ser_file->get_byte_depth(),  // byte_depth
                    mp_ser_file->get_colour_id(),  // colour_id
                    is_colour);  // colour

        int32_t ret = mp_ser_file->get_frame(mp_playback_controls_widget->slider_value(), mp_frame_image->get_p_buffer());

        mp_frame_image->convert_image_to_8bit();

        if (ret >= 0) {
            // Debayer frame if required
            if (mp_processing_options_Dialog->get_debayer_enable()) {
                int colour_id = mp_processing_options_Dialog->get_debayer_pattern();
                if (colour_id < 0) {
                    // No colour_id specified, use value from SER file
                    colour_id = mp_ser_file->get_colour_id();
                }

                mp_frame_image->debayer_image_bilinear(colour_id);
            }
        }

        if (ret >= 0) {
            double red_gain = 1.0;
            double green_gain = 1.0;
            double blue_gain = 1.0;

            mp_frame_image->estimate_colour_balance(red_gain, green_gain, blue_gain);
            mp_processing_options_Dialog->set_colour_balance(red_gain, green_gain, blue_gain);
        }
    }
}


void c_ser_player::zoom_changed_slot(QAction *action)
{
    if (action != nullptr) {
        resize_window_with_zoom(action->data().toInt());
    }
}


void c_ser_player::language_changed_slot(QAction *action)
{
    if (action != nullptr) {
        c_persistent_data::m_selected_language = action->data().toString();
    }
}


void c_ser_player::about_qt()
{
    QMessageBox::aboutQt(nullptr, tr("About Qt", "Message box title"));
}


void c_ser_player::open_ser_file_slot()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open SER File", "Open file dialog title"),
                                                    c_persistent_data::m_ser_directory,
                                                    tr("SER Files (*.ser)", "Filetype filter"));

    if (!filename.isEmpty()) {
        open_ser_file(filename);
    }
}


void c_ser_player::open_ser_file_slot(QAction *action)
{
    if (action != nullptr) {
        QString filename = action->data().toString();
        if (filename.isEmpty()) {
            // Clear list
            c_persistent_data::m_recent_ser_files.clear();
            update_recent_ser_files_menu();
        } else if (QFile(filename).exists()) {
            // Open SER File
            open_ser_file(filename);
        } else {
            // File does not exist
            QMessageBox::warning(nullptr,
                                 tr("Cannot Open SER File"),
                                 tr("File Not Found:", "Message box title for cannot open SER file") + "\n" + filename);
            c_persistent_data::m_recent_ser_files.removeAll(filename);
            update_recent_ser_files_menu();
        }
    }
}


void c_ser_player::open_ser_file(const QString &filename)
{
    // Reset options before opening a new file
    mp_processing_options_Dialog->reset_all_slot();

    // Disable area selection
    mp_frame_image_Widget->disable_area_selection();

    mp_playback_controls_widget->reset_all_markers_slot();  // Ensure start marker is reset
    mp_playback_controls_widget->stop_playback();  // Stop and reset and currently playing frame

    mp_ser_file->close();
    m_ser_file_loaded = false;
    m_total_frames = mp_ser_file->open(filename.toUtf8().constData(), 0, 0);

    // Check if SER file is broken but fixable - fix it if possible
    bool ser_file_broken = false;
    if (m_total_frames == mp_ser_file->ERROR_ZERO_FRAME_COUNT) {
        // Zero frame count is often the result of the capture software crashing and failing to
        // finish writing the file.  However, the file can be updated with a best guess framecount
        // and the usable data recovered
        ser_file_broken = true;

        // Offer to fix the fix
        QMessageBox::StandardButton fix_ser_file;

        QString error_message = tr("Error: File '%1' has an invalid frame count.  SER Player may be able to fix this file.", "SER File error message")
                                .arg(filename);
        error_message += "\n\n";
        error_message += tr("Fix this SER file?");

        fix_ser_file = QMessageBox::question(nullptr,
                                             tr("Invalid SER File", "Message box title for invalid SER file"),
                                             error_message,
                                             QMessageBox::Yes|QMessageBox::No);
        if (fix_ser_file == QMessageBox::Yes) {
            mp_ser_file->fix_broken_ser_file(filename.toUtf8().constData());

            // Now try and open the file again
            m_total_frames = mp_ser_file->open(filename.toUtf8().constData(), 0, 0);

            if (m_total_frames > 0) {
                ser_file_broken = false;  // File is no longer broken!
                QMessageBox::information(nullptr,
                                         tr("Invalid SER File", "Message box title for invalid SER file"),
                                         tr("The SER file has succesfully been fixed"));
                ser_file_broken = false;  // No longer broken
            } else {
                QMessageBox::warning(nullptr,
                                   tr("Invalid SER File", "Message box title for invalid SER file"),
                                   tr("The SER file could not be fixed"));
            }
        }
    }

    // Check if SER file is broken and not fixable
    if (m_total_frames <= 0 && !ser_file_broken) {
        // Invalid SER file that cannot be fixed
        if (mp_ser_file->get_error_string().length() > 0) {
            QMessageBox::warning(nullptr,
                                 tr("Invalid SER File", "Message box title for invalid SER file"),
                                 mp_ser_file->get_error_string().c_str());
            ser_file_broken = true;
        }
    }

    if (!ser_file_broken) {
        // This is a valid SER file

        // Delete previous save frames dialogs to remove remembered settings
        delete mp_save_frames_as_ser_Dialog;
        mp_save_frames_as_ser_Dialog = nullptr;
        delete mp_save_frames_as_avi_Dialog;
        mp_save_frames_as_avi_Dialog = nullptr;
        delete mp_save_frames_as_gif_Dialog;
        mp_save_frames_as_gif_Dialog = nullptr;
        delete mp_save_frames_as_images_Dialog;
        mp_save_frames_as_images_Dialog = nullptr;

        // Set SER file header details in header details dialog
        mp_header_details_dialog->set_details(
                filename,
                QFileInfo(filename).size(),  // int filesize,
                QString::fromStdString(mp_ser_file->get_file_id()), // QString file_id,
                mp_ser_file->get_lu_id(),  // int lu_id,
                mp_ser_file->get_colour_id(),  // int colour_id,
                mp_ser_file->get_little_endian(),  // int little_endian,
                mp_ser_file->get_width(),  // int image_width,
                mp_ser_file->get_height(),  // int image_height,
                mp_ser_file->get_pixel_depth(),  // int pixel_depth,
                m_total_frames,  // int frame_count,
                QString::fromStdString(mp_ser_file->get_observer_string()),  // QString observer,
                QString::fromStdString(mp_ser_file->get_instrument_string()),  // QString instrument,
                QString::fromStdString(mp_ser_file->get_telescope_string()),  // QString telescope,
                mp_ser_file->get_data_time(),  // uint64_t date_time,
                mp_ser_file->get_data_time_utc(),  // uint64_t date_time_utc)
                QString::fromStdString(mp_ser_file->get_timestamp_info()));  // QString timestamp_info

        // Keep list of opened SER files up to date
        add_string_to_stringlist(c_persistent_data::m_recent_ser_files, QFileInfo(filename).absoluteFilePath());
        c_persistent_data::m_ser_directory = QFileInfo(filename).absolutePath();

        // Update Recent SER Files menu
        update_recent_ser_files_menu();

        // Ensure we are in the stopped state
        mp_playback_controls_widget->stop_playback();

        // Update window title with SER filename
        QString ser_filename = pipp_get_filename_from_filepath(filename.toStdString());
        setWindowTitle(ser_filename + " - " + C_WINDOW_TITLE_QSTRING);

        // Remember SER file directory
        m_ser_directory = QFileInfo(filename).canonicalPath();

        // Set up frame slider widget
        mp_playback_controls_widget->set_maximum_frame(m_total_frames);
        mp_playback_controls_widget->reset_all_markers_slot();  // Reset markers to new frame range
        mp_playback_controls_widget->set_markers_show(true);  // Un-hide markers
        mp_playback_controls_widget->goto_first_frame();

        // Update frame size label
        mp_playback_controls_widget->update_frame_size_label(mp_ser_file->get_width(), mp_ser_file->get_height());

        // Update pixel depth label
        mp_playback_controls_widget->update_pixel_depth_label(mp_ser_file->get_pixel_depth());
        m_ser_file_loaded = true;
        m_is_colour = false;
        m_has_bayer_pattern = false;

        // Update colour ID label
        switch (mp_ser_file->get_colour_id()) {
        case COLOURID_MONO:
            mp_playback_controls_widget->update_colour_id_label(tr("MONO", "Colour ID label"));
            break;
        case COLOURID_BAYER_RGGB:
            mp_playback_controls_widget->update_colour_id_label(tr("RGGB", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_GRBG:
            mp_playback_controls_widget->update_colour_id_label(tr("GRBG", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_GBRG:
            mp_playback_controls_widget->update_colour_id_label(tr("GBRG", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_BGGR:
            mp_playback_controls_widget->update_colour_id_label(tr("BGGR", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_CYYM:
            mp_playback_controls_widget->update_colour_id_label(tr("CYYM", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_YCMY:
            mp_playback_controls_widget->update_colour_id_label(tr("YCMY", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_YMCY:
            mp_playback_controls_widget->update_colour_id_label(tr("YMCY", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_MYYC:
            mp_playback_controls_widget->update_colour_id_label(tr("MYYC", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_RGB:
            mp_playback_controls_widget->update_colour_id_label(tr("RGB", "Colour ID label"));
            m_is_colour = true;
            break;
        case COLOURID_BGR:
            mp_playback_controls_widget->update_colour_id_label(tr("BGR", "Colour ID label"));
            m_is_colour = true;
            break;
        default:
            mp_playback_controls_widget->update_colour_id_label(tr("????", "Colour ID label for unknown ID"));
        }

        // Inform processing dialog whether this data has a bayer pattern or not
        mp_processing_options_Dialog->set_frame_size(mp_ser_file->get_width(), mp_ser_file->get_height());
        mp_processing_options_Dialog->set_data_has_bayer_pattern(m_has_bayer_pattern);
        mp_processing_options_Dialog->set_data_is_colour(m_is_colour);

        // Enable menu items that are only enabled when a SER file is open
        mp_save_frames_as_ser_Act->setEnabled(true);
        mp_save_frames_as_avi_Act->setEnabled(true);
        mp_save_frames_as_gif_Act->setEnabled(true);
        mp_save_frames_as_images_Act->setEnabled(true);
        mp_framerate_Menu->setEnabled(true);
        mp_header_details_Act->setEnabled(true);
        mp_histogram_viewer_Act->setEnabled(true);
        if (mp_histogram_viewer_Act->isChecked()) {
            mp_histogram_dialog->show();
        }

        mp_processing_options_Act->setEnabled(true);
        mp_markers_dialog_Act->setEnabled(true);

        // Calculate frame rate, update framerate label an use value for playback timer
        calculate_display_framerate();

        // Force first frame to be played and update timestamp label
        frame_slider_changed_slot();
        resize_window_100_percent_slot();

        // Move the application to the middle of the screen
        int x_offset = QApplication::desktop()->availableGeometry().size().width() - frameSize().width();
        x_offset /= 2;
        int y_offset = QApplication::desktop()->availableGeometry().size().height() - frameSize().height();
        y_offset /= 2;
        QPoint new_window_pos = QApplication::desktop()->availableGeometry().topLeft();
        new_window_pos.setX(new_window_pos.x() + x_offset);
        new_window_pos.setY(new_window_pos.y() + y_offset);
        move(new_window_pos);

        // Move the histogram to the top-right(ish) of the application window
        mp_histogram_dialog->move_to_default_position();
        mp_playback_controls_dialog->move_to_default_position();

        // Start playback
        mp_playback_controls_widget->start_playback();
    }
}


void c_ser_player::set_defaut_histogram_position()
{
    // Move the histogram to the top-right(ish) of the application window
    QDesktopWidget widget;
    int available_width = widget.availableGeometry().size().width();
    QPoint histogram_pos = geometry().topRight();
    if ((histogram_pos.x() + mp_histogram_dialog->frameGeometry().width()) > available_width) {
        int new_x = available_width - mp_histogram_dialog->frameGeometry().width();
        histogram_pos.setX(new_x);
    }

    mp_histogram_dialog->move(histogram_pos);
}


void c_ser_player::markers_dialog_closed_slot()
{
    mp_markers_dialog_Act->setChecked(false);
}


void c_ser_player::frame_slider_changed_slot()
{
    // Update image to new frame
    if (!m_ser_file_loaded) {
        mp_playback_controls_widget->stop_playback();
    } else {
        bool valid_frame = get_and_process_frame(mp_playback_controls_widget->slider_value(),  // frame_number
                                               true,  // conv_to_8_bit
                                               true);  // do_processing

        if (valid_frame) {
            // Start histogram generation if one is not already being generated
            if (mp_histogram_dialog->isVisible() && !mp_histogram_thread->is_running()) {
                mp_histogram_thread->generate_histogram(mp_frame_image, mp_playback_controls_widget->slider_value());
            }

            mp_frame_image->conv_data_ready_for_qimage();

            QImage frame_qimage = QImage(mp_frame_image->get_p_buffer(),
                                         mp_frame_image->get_width(),
                                         mp_frame_image->get_height(),
                                         QImage::Format_RGB888);

            // Upate image in player
            mp_frame_image_Widget->setPixmap(QPixmap::fromImage(frame_qimage));

            // Update timestamp label
            mp_playback_controls_widget->update_timestamp_label(mp_ser_file->get_timestamp());

            // Ensure displayed histogram matches displayed frame
            if (!mp_playback_controls_widget->is_playing()) {
                if (mp_histogram_dialog->isVisible() && mp_playback_controls_widget->slider_value() != mp_histogram_thread->get_frame_number()) {
                    // We have stopped playing but the histogram is not the histogram for the last frame
                    // Display the last frame again to regenerate the histogram
                    QTimer::singleShot(5, this, SLOT(frame_slider_changed_slot()));
                }
            }
        } else {
            // Should never get here unless something has gone very wrong
            // Stop playing as a last resort
            mp_playback_controls_widget->stop_playback();
        }
    }
}


void c_ser_player::frame_timer_timeout_slot()
{
    if (!m_ser_file_loaded) {
        mp_playback_controls_widget->stop_playback();
    } else {
        if (!mp_playback_controls_widget->goto_next_frame()) {
            // End of playback
            if (mp_histogram_dialog->isVisible()) {
                // A slightly messy way to ensure the displayed histogram matches the displayed frame
                // when playback has stopped
                QTimer::singleShot(5, this, SLOT(frame_slider_changed_slot()));
            }
        }

        if (!mp_playback_controls_widget->is_playing()) {
            mp_playback_controls_widget->stop_playback();
        }
    }
}


void c_ser_player::resize_window_100_percent_slot()
{
    resize_window_with_zoom(100);
}


void c_ser_player::resize_window_with_zoom(int zoom)
{
    m_requested_zoom = zoom;
    QSize frame_border_and_title_size = frameSize() - size();
    QDesktopWidget widget;
    QSize available_desktop_size = widget.availableGeometry().size() - frame_border_and_title_size;
    QSize widget_outside_image_size = size() - mp_frame_image_Widget->size();

    // Calculate size of zoomed image
    QSize zoomed_image_size = (mp_frame_image_Widget->get_image_size() * zoom) / 100;

    // Calculate application size without frame border and title
    QSize new_widget_size = zoomed_image_size + widget_outside_image_size;

    // Check that the application size is not larger than the available desktop size
    QSize oversize_error = QSize(0, 0);
    if (new_widget_size.width() > available_desktop_size.width()) {
        oversize_error.setWidth(new_widget_size.width() - available_desktop_size.width());
    }

    if (new_widget_size.height() > available_desktop_size.height()) {
        oversize_error.setHeight(new_widget_size.height() - available_desktop_size.height());
    }

    // Recalculate size of zoomed image to fit on available desktop size
    zoomed_image_size.scale(zoomed_image_size - oversize_error, Qt::KeepAspectRatio);

    // Recalculate application size without frame border and title
    new_widget_size = zoomed_image_size + widget_outside_image_size;

    showNormal();  // Ensure window is not maximised
    resize(new_widget_size);  // Resize the application
}


void c_ser_player::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}


void c_ser_player::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString &fileName = url.toLocalFile();
        if (fileName.endsWith(".ser", Qt::CaseInsensitive)) {
            open_ser_file(fileName);
            break;
        }
    }
}


void c_ser_player::check_for_updates_slot(bool enabled)
{
    c_persistent_data::m_check_for_updates = enabled;
}


void c_ser_player::debayer_enable_slot()
{
    frame_slider_changed_slot();
}


void c_ser_player::new_version_available_slot(QString version)
{
    c_persistent_data::m_new_version = version;
}


void c_ser_player::histogram_done_slot()
{
    QPixmap histogram_Pixmap;
    mp_histogram_thread->draw_histogram_pixmap(histogram_Pixmap);
    mp_histogram_dialog->set_pixmap(histogram_Pixmap);
}


void c_ser_player::start_playing_slot()
{
    mp_frame_Timer->start(m_display_frame_time);
}


void c_ser_player::stop_playing_slot()
{
    mp_frame_Timer->stop();
}


void c_ser_player::about_ser_player()
{
    QPixmap ser_player_logoPixmap(":/res/resources/ser_player_logo_150x150.png");
    QMessageBox msgBox;
#ifdef BUILD_64_BIT
    QString build_type_string = "x64";
#else
    QString build_type_string = "x86";
#endif

    msgBox.setText("<b><big>" + tr("SER Player") + "</big> " VERSION_STRING " (" + build_type_string + ")</b>");
    QString informative_text = tr("A video player and processing utility for SER files.");
    informative_text += "<qt><a href=\"http://sites.google.com/site/astropipp/\">http://sites.google.com/site/astropipp/</a><br>";
    informative_text += "Copyright (c) 2015-2017 Chris Garry";

    QString translator_credit = tr("English language translation by Chris Garry",
                                   "Translator credit - Replace language and translator names when translating");
    if (translator_credit != "English language translation by Chris Garry") {
        // Add a credit for the translator if the translator credit string has been translated
        informative_text += ("<qt>");
        informative_text += translator_credit;
    }

    informative_text += "<qt>&nbsp;<br>";

    informative_text += "<b>" + tr("3rd Party Components:") + "</b>";
    informative_text += "<br>";
    informative_text += tr("NeuQuant (c) 1994 Anthony Dekker") + "<br>";
    informative_text += tr("Neural-Net image quantization algorithm");
    informative_text += "<qt>&nbsp;<br>";

    informative_text += "<b>" + tr("Credits:") + "</b>";
    informative_text += "<br>";
    informative_text += tr("Sylvain Weiller for all his suggestions, testing and feedback.");

    informative_text += "<qt>&nbsp;<br>";
    informative_text += "<b>" + tr("License:") + "</b><br>";
    informative_text += "This program is free software: you can redistribute it and/or modify "
                           "it under the terms of the GNU General Public License as published by "
                           "the Free Software Foundation, either version 3 of the License, or "
                           "(at your option) any later version.";
    informative_text += "<qt>&nbsp;<br>";
    informative_text += "This program is distributed in the hope that it will be useful, "
                        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                        "GNU General Public License for more details.";
    informative_text += "<qt>&nbsp;<br>";
    informative_text += tr("You should have received a copy of the GNU General Public License "
                           "along with this program.  If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>", "About box text");
    informative_text += "<qt>&nbsp;";

    msgBox.setInformativeText(informative_text);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.setWindowTitle(tr("About SER Player", "About box title"));
    msgBox.setIconPixmap(ser_player_logoPixmap);
    msgBox.exec();
}


void c_ser_player::create_no_file_open_image()
{
    QString no_file_open_string = QString(tr("No SER File Open", "No SER file message on inital image"));
    QImage frame_QImage = QImage(":/res/resources/ser_player_logo.png");
    m_no_file_open_Pixmap = QPixmap::fromImage(frame_QImage);
    int pic_width = m_no_file_open_Pixmap.width();
    int pic_height = m_no_file_open_Pixmap.height();

    QFont font;// = QFont("Arial");
    int pixel_size = 1;
    int last_pixel_size = pixel_size;

    // Select font size
    while(true) {
        font.setPixelSize(pixel_size);
        QFontMetrics fm(font);
        int text_width = fm.width(no_file_open_string);
        if (text_width > (pic_width * 9) / 10) {
            font.setPixelSize(last_pixel_size);
            break;
        }

        last_pixel_size = pixel_size;
        pixel_size++;
    }

    // Draw text on image
    QFontMetrics fm(font);
    int no_ser_text_width = fm.width(no_file_open_string);

    QPainter painter(&m_no_file_open_Pixmap);
    painter.setPen(Qt::white);
    painter.setFont(font);
    painter.drawText(QPoint((pic_width - no_ser_text_width) / 2, (pic_height + 0) / 2), no_file_open_string);

    bool is_newer = false;
#ifndef DISABLE_NEW_VERSION_CHECK
    is_newer = c_new_version_checker::compare_version_strings(VERSION_STRING, c_persistent_data::m_new_version);
#endif

    if (is_newer && c_persistent_data::m_check_for_updates) {
        //: New version notification message
        QString new_version_text = tr("New version of SER Player available: %1").arg(c_persistent_data::m_new_version);
        QString download_from_text = QString("https://sites.google.com/site/astropipp/ser-player");

        // Select font size
        pixel_size = 1;
        int new_ver_pixel_size = pixel_size;
        while(true) {
            font.setPixelSize(pixel_size);
            QFontMetrics fm(font);
            int text_width = fm.width(new_version_text);
            if (text_width > (pic_width * 98) / 100) {
                font.setPixelSize(last_pixel_size);
                break;
            }

            new_ver_pixel_size = pixel_size;
            pixel_size++;
        }

        // Draw text on image
        font.setPixelSize(new_ver_pixel_size);
        QFontMetrics fm(font);
        int new_ver_text_width = fm.width(new_version_text);
        int new_ver_text_height = fm.height();
        painter.setPen(Qt::yellow);
        painter.setFont(font);
        painter.drawText(QPoint((pic_width - new_ver_text_width) / 2, (pic_height + 0) / 2 + new_ver_text_height + 5), new_version_text);


        pixel_size = 1;
        int download_from_pixel_size = pixel_size;
        while(true) {
            font.setPixelSize(pixel_size);
            QFontMetrics fm(font);
            int text_width = fm.width(download_from_text);
            if (text_width > (pic_width * 98) / 100) {
                font.setPixelSize(last_pixel_size);
                break;
            }

            download_from_pixel_size = pixel_size;
            pixel_size++;
        }

        // Draw text on image
        font.setPixelSize(download_from_pixel_size);
        QFontMetrics fm2(font);
        int download_from_text_width = fm2.width(download_from_text);
        int download_from_text_height = fm2.height();
        painter.setPen(Qt::yellow);
        painter.setFont(font);
        painter.drawText(QPoint((pic_width - download_from_text_width) / 2, (pic_height + 0) / 2 + new_ver_text_height + 5 + download_from_text_height + 5), download_from_text);
    }
}


void c_ser_player::changeEvent (QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        mp_histogram_dialog->move_to_default_position();
        mp_playback_controls_dialog->move_to_default_position();
    }
}


/*
void c_ser_player::resizeEvent(QResizeEvent *e)
{
    //QMainWindow::resizeEvent(e);
    //mp_resize_Timer->start(1);
}
*/



/*
void c_ser_player::resize_timer_timeout_slot()
{
    int zoom_level = mp_frame_image_Widget->get_zoom_level();
    mp_playback_controls_widget->update_zoom_label_slot(zoom_level);
}
*/


void c_ser_player::calculate_display_framerate()
{
    double fps;
    if (m_display_framerate < 0) {
        // Display framerate is from SER file timestamps
        if (mp_ser_file->get_fps_rate() > 0) {
            double frame_time = (1000.0 * mp_ser_file->get_fps_scale()) / mp_ser_file->get_fps_rate();
            frame_time += 0.5;
            frame_time = floor(frame_time);
            m_display_frame_time = (int)frame_time;
            fps = (double)mp_ser_file->get_fps_rate() / (double)mp_ser_file->get_fps_scale();
            fps = floor(fps * 100.0 + 0.5) / 100.0;  // Round to 2dp
        } else {
            // No frame rate in file - use default
            m_display_frame_time = 20;  // 20ms ~ 50 FPS
            fps = 1000.0 / m_display_frame_time;
        }
    } else {
        // Display framerate has been specified
        double frame_time = 1000.0 / m_display_framerate;
        frame_time += 0.5;
        frame_time = floor(frame_time);
        m_display_frame_time = (int)frame_time;
        fps = (double)m_display_framerate;
    }

    mp_playback_controls_widget->update_fps_label(fps);
    mp_frame_Timer->setInterval(m_display_frame_time);
}


bool c_ser_player::get_and_process_frame(int frame_number, bool conv_to_8_bit, bool do_processing)
{
    bool is_colour = false;
    if (mp_ser_file->get_colour_id() == COLOURID_RGB || mp_ser_file->get_colour_id() == COLOURID_BGR) {
        is_colour = true;
    }

    mp_frame_image->set_image_details(
                mp_ser_file->get_width(),  // width
                mp_ser_file->get_height(),  // height
                mp_ser_file->get_byte_depth(),  // byte_depth
                mp_ser_file->get_colour_id(),  // colour_id
                is_colour);  // colour

    int32_t ret = mp_ser_file->get_frame(frame_number, mp_frame_image->get_p_buffer());

    if (ret >= 0) {
        if (conv_to_8_bit) {
            mp_frame_image->convert_image_to_8bit();
        }

        if (do_processing) {
            // Debayer frame if required
            if (mp_processing_options_Dialog->get_debayer_enable()) {
                int colour_id = mp_processing_options_Dialog->get_debayer_pattern();
                if (colour_id < 0) {
                    // No colour_id specified, use value from SER file
                    colour_id = mp_ser_file->get_colour_id();
                }

                mp_frame_image->debayer_image_bilinear(colour_id);
            }

            //
            if (m_crop_enable) {
                mp_frame_image->crop_image(
                        m_crop_x_pos,
                        m_crop_y_pos,
                        m_crop_width,
                        m_crop_height);
            }

            mp_frame_image->align_colour_channels();

            if (m_monochrome_conversion_enable) {
                mp_frame_image->monochrome_conversion(m_monochrome_conversion_type);
            }

            mp_frame_image->do_lut_based_processing();

            // Adjust colour saturation if required
            mp_frame_image->change_colour_saturation(mp_processing_options_Dialog->get_colour_saturation());
        }
    }

    return (ret >= 0);
}
