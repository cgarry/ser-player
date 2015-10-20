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


#define VERSION_STRING "v1.3.9"

#include <Qt>
#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QImage>
#include <QImageWriter>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QMutex>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QWidgetAction>

#include <cmath>

#include "frame_slider.h"
#include "ser_player.h"
#include "persistent_data.h"
#include "image_functions.h"
#include "pipp_ser.h"
#include "pipp_timestamp.h"
#include "pipp_utf8.h"
#include "image_widget.h"
#include "colour_dialog.h"
#include "markers_dialog.h"
#include "save_frames_dialog.h"
#include "save_frames_progress_dialog.h"


#ifndef DISABLE_NEW_VERSION_CHECK
#include "new_version_checker.h"
#endif

const QString c_ser_player::C_WINDOW_TITLE_QSTRING = QString("SER Player");


c_ser_player::c_ser_player(QWidget *parent)
    : QMainWindow(parent)
{
    m_current_state = STATE_NO_FILE;
    m_is_colour = false;
    m_has_bayer_pattern = false;

    m_frame_details.width = 100;
    m_frame_details.height = 100;
    m_frame_details.colour_id = COLOURID_MONO;
    m_frame_details.p_buffer = NULL;

    // Menu Items
    m_ser_directory = "";
    m_display_framerate = -1;
    m_colour_saturation = 1.0;


    //
    // File menu
    //
    QMenu *file_menu = menuBar()->addMenu(tr("File", "Menu title"));
    QAction *fileopen_Act = new QAction(tr("Open SER File", "Menu title"), this);
    file_menu->addAction(fileopen_Act);
    connect(fileopen_Act, SIGNAL(triggered()), this, SLOT(open_ser_file_slot()));

    file_menu->addSeparator();

    m_save_frames_Act = new QAction(tr("Save Frames As Images...", "Menu title"), this);
    m_save_frames_Act->setEnabled(false);
    file_menu->addAction(m_save_frames_Act);
    connect(m_save_frames_Act, SIGNAL(triggered()), this, SLOT(save_frames_slot()));

    file_menu->addSeparator();

    QAction *quit_Act = new QAction(tr("Quit", "Menu title"), this);
    file_menu->addAction(quit_Act);
    connect(quit_Act, SIGNAL(triggered()), this, SLOT(close()));


    //
    // Playback menu
    //
    QMenu *playback_menu = menuBar()->addMenu(tr("Playback", "Menu title"));

    const int zoom_levels[] = {25, 50, 75, 100, 125, 150, 200, 250, 300};
    QMenu *zoom_Menu = playback_menu->addMenu(tr("Change Zoom", "Zoom menu"));
    QActionGroup *zoom_ActGroup = new QActionGroup(zoom_Menu);
    QAction *zoom_action;
    for (int x = 0; x <  (int)(sizeof(zoom_levels) / sizeof(zoom_levels[0])); x++) {
        zoom_action = new QAction(tr("%1%", "Zoom menu").arg(zoom_levels[x]), this);
        zoom_action->setData(zoom_levels[x]);
        zoom_Menu->addAction(zoom_action);
        zoom_ActGroup->addAction(zoom_action);
    }

    connect(zoom_ActGroup, SIGNAL (triggered(QAction *)), this, SLOT (zoom_changed_slot(QAction *)));
    playback_menu->addSeparator();

    mp_framerate_Menu = playback_menu->addMenu(tr("Display Framerate"));
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

    fps_action = new QAction(tr("1 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(1);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("5 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(5);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("10 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(10);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("25 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(25);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("50 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(50);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("75 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(75);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("100 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(100);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("150 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(150);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);

    fps_action = new QAction(tr("200 fps", "Framerate menu"), this);
    fps_action->setCheckable(true);
    fps_action->setData(200);
    mp_framerate_Menu->addAction(fps_action);
    fps_ActGroup->addAction(fps_action);
    connect(fps_ActGroup, SIGNAL (triggered(QAction *)), this, SLOT (fps_changed_slot(QAction *)));


    mp_direction_Menu = playback_menu->addMenu(tr("Playback Direction"));
    mp_direction_Menu->setEnabled(false);
    QActionGroup *direction_ActGroup = new QActionGroup(mp_direction_Menu);
    direction_ActGroup->setExclusive(true);
    QAction *direction_Action;

    direction_Action = new QAction(tr("Forward", "Playback direction menu"), this);
    direction_Action->setCheckable(true);
    direction_Action->setChecked(true);
    direction_Action->setData(0);
    mp_direction_Menu->addAction(direction_Action);
    direction_ActGroup->addAction(direction_Action);

    direction_Action = new QAction(tr("Reverse", "Playback direction menu"), this);
    direction_Action->setCheckable(true);
    direction_Action->setData(1);
    mp_direction_Menu->addAction(direction_Action);
    direction_ActGroup->addAction(direction_Action);

    direction_Action = new QAction(tr("Forward + Reverse","Playback direction menu"), this);
    direction_Action->setCheckable(true);
    direction_Action->setData(2);
    mp_direction_Menu->addAction(direction_Action);
    direction_ActGroup->addAction(direction_Action);
    connect(direction_ActGroup, SIGNAL(triggered(QAction *)), this, SLOT (direction_changed_slot(QAction *)));


    // Enable Debayer menu action
    m_debayer_Act = new QAction(tr("Enable Debayering"), this);
    m_debayer_Act->setCheckable(true);
    m_debayer_Act->setChecked(c_persistent_data::m_enable_debayering);
    playback_menu->addAction(m_debayer_Act);
    connect(m_debayer_Act, SIGNAL(triggered(bool)), this, SLOT(debayer_enable_slot(bool)));
    m_debayer_Act->setEnabled(false);


    //
    // Windows menu
    //
    QMenu *window_menu = menuBar()->addMenu(tr("Window", "Menu title"));

    // Colour Setting menu action
    mp_colour_settings_action = window_menu->addAction(tr("Colour Settings"));
    mp_colour_settings_action->setEnabled(false);
    connect(mp_colour_settings_action, SIGNAL(triggered()), this, SLOT(colour_settings_slot()));
    mp_colour_settings_Dialog = new c_colour_dialog(this);
    mp_colour_settings_Dialog->hide();
    connect(mp_colour_settings_Dialog, SIGNAL(colour_saturation_changed(double)), this, SLOT(colour_saturation_changed_slot(double)));
    connect(mp_colour_settings_Dialog, SIGNAL(colour_balance_changed(double,double,double)), this, SLOT(colour_balance_changed_slot(double,double,double)));
    connect(mp_colour_settings_Dialog, SIGNAL(estimate_colour_balance()), this, SLOT(estimate_colour_balance()));

    // Markers Dialog action
    mp_markers_dialog_action = window_menu->addAction(tr("Start/End Markers"));
    mp_markers_dialog_action->setEnabled(true);


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

        action = new QAction(lang, this);
        action->setCheckable(true);
        action->setData(locale);
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

    mp_frame_Image = new QImage(":/res/resources/ser_player_logo.png");
    create_no_file_open_image();  // Create m_no_file_open_Pixmap

    mp_ser_file_Mutex = new QMutex;
    mp_frame_slider_changed_Mutex = new QMutex;

    mp_frame_image_Widget = new c_image_Widget(this);
    mp_frame_image_Widget->setPixmap(m_no_file_open_Pixmap);

    mp_frame_Slider = new c_frame_slider(this);
    mp_frame_Slider->set_maximum_frame(99);
    mp_frame_Slider->set_direction(0);
    mp_frame_Slider->set_repeat(c_persistent_data::m_repeat);
    connect(mp_markers_dialog_action, SIGNAL(triggered()), mp_frame_Slider, SLOT(show_markers_dialog()));


    m_play_Pixmap = QPixmap(":/res/resources/play_button.png");
    m_pause_Pixmap = QPixmap(":/res/resources/pause_button.png");

    mp_forward_PushButton = new QPushButton;
    QPixmap forward_Pixmap = QPixmap(":/res/resources/forward_button.png");
    mp_forward_PushButton->setIcon(forward_Pixmap);
    mp_forward_PushButton->setIconSize(forward_Pixmap.size());
    mp_forward_PushButton->setFixedSize(forward_Pixmap.size() + QSize(10, 10));  // Nice and small
    mp_forward_PushButton->setToolTip(tr("Click to advance 1 frame\nShift-Click to advance multiple frames", "Button Tool tip"));  // Nice and small

    mp_back_PushButton = new QPushButton;
    QPixmap back_Pixmap = QPixmap(":/res/resources/back_button.png");
    mp_back_PushButton->setIcon(back_Pixmap);
    mp_back_PushButton->setIconSize(back_Pixmap.size());
    mp_back_PushButton->setFixedSize(back_Pixmap.size() + QSize(10, 10));
    mp_back_PushButton->setToolTip(tr("Click to go back 1 frame\nShift-Click to go back multiple frames", "Button Tool tip"));

    mp_play_PushButton = new QPushButton;
    mp_play_PushButton->setIcon(m_play_Pixmap);
    mp_play_PushButton->setIconSize(m_play_Pixmap.size());
    mp_play_PushButton->setFixedSize(m_play_Pixmap.size() + QSize(10, 10));
    mp_play_PushButton->setToolTip(tr("Play/Pause", "Button Tool tip"));

    mp_stop_PushButton = new QPushButton;
    QPixmap stop_Pixmap = QPixmap(":/res/resources/stop_button.png");
    mp_stop_PushButton->setIcon(stop_Pixmap);
    mp_stop_PushButton->setIconSize(stop_Pixmap.size());
    mp_stop_PushButton->setFixedSize(stop_Pixmap.size() + QSize(10, 10));
    mp_stop_PushButton->setToolTip(tr("Stop", "Button Tool tip"));

    mp_repeat_PushButton = new QPushButton;
    QPixmap repeat_Pixmap = QPixmap(":/res/resources/repeat_button.png");
    mp_repeat_PushButton->setIcon(repeat_Pixmap);
    mp_repeat_PushButton->setIconSize(repeat_Pixmap.size());
    mp_repeat_PushButton->setFixedSize(repeat_Pixmap.size() + QSize(10, 10));
    mp_repeat_PushButton->setCheckable(true);
    mp_repeat_PushButton->setChecked(c_persistent_data::m_repeat);
    mp_repeat_PushButton->setToolTip(tr("Repeat On/Off", "Button Tool tip"));

    m_framecount_label_String = tr("%1/%2", "Frame number/Frame count label");
    mp_framecount_Label = new QLabel;
    mp_framecount_Label->setText(m_framecount_label_String.arg("-").arg("----"));
    mp_framecount_Label->setToolTip(tr("Frame number/Total Frames", "Tool tip"));

    mp_fps_Label = new QLabel;
    m_fps_label_String = tr("%1 FPS", "Framerate label");
    mp_fps_Label->setText(m_fps_label_String.arg("--"));
    mp_fps_Label->setToolTip(tr("Display Frame rate", "Tool tip"));

    mp_colour_id_Label = new QLabel;
    mp_colour_id_Label->setText("----");
    mp_colour_id_Label->setToolTip(tr("Colour ID", "Tool tip"));

    m_zoom_label_String = tr("%1%", "Zoom level label");
    mp_zoom_Label =  new QLabel;
    mp_zoom_Label->setText(m_zoom_label_String.arg(100));
    mp_zoom_Label->setToolTip(tr("Display Zoom Level", "Tool tip"));

    m_frame_size_label_String = tr("%1x%2", "Frame size label");
    mp_frame_size_Label = new QLabel;
    mp_frame_size_Label->setText(m_frame_size_label_String.arg("---").arg("---"));
    mp_frame_size_Label->setToolTip(tr("Frame size", "Tool tip"));

    m_pixel_depth_label_String = tr("%1-Bit", "Pixel depth label");
    mp_pixel_depth_Label = new QLabel;
    mp_pixel_depth_Label->setText(m_pixel_depth_label_String.arg("-"));
    mp_pixel_depth_Label->setToolTip(tr("Pixel bit depth", "Tool tip"));

    m_timestamp_label_String = tr("%3/%2/%1 %4:%5:%6.%7 UT", "Timestamp label");
    mp_timestamp_Label = new QLabel;
    mp_timestamp_Label->setText(tr("No Timestamp", "Timestamp label for no timestamp"));
    mp_timestamp_Label->setToolTip(tr("Frame timestamp", "Tool tip"));

    QHBoxLayout *slider_h_layout = new QHBoxLayout;
    slider_h_layout->setSpacing(0);
    slider_h_layout->setMargin(0);
    slider_h_layout->addWidget(mp_back_PushButton);
#ifdef __APPLE__
    slider_h_layout->addSpacing(15);
#else
    slider_h_layout->addSpacing(4);
#endif
    slider_h_layout->addWidget(mp_forward_PushButton);
#ifdef __APPLE__
    slider_h_layout->addSpacing(15);
#else
    slider_h_layout->addSpacing(4);
#endif
    slider_h_layout->addWidget(mp_frame_Slider);

    QHBoxLayout *controls_h_layout1 = new QHBoxLayout;
    controls_h_layout1->setSpacing(8);
    controls_h_layout1->setMargin(0);
    controls_h_layout1->addStretch();
    controls_h_layout1->addWidget(mp_zoom_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_frame_size_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_pixel_depth_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_colour_id_Label, 0, Qt::AlignTop | Qt::AlignRight);
    controls_h_layout1->addWidget(mp_framecount_Label, 0, Qt::AlignTop | Qt::AlignRight);

    QHBoxLayout *controls_h_layout2 = new QHBoxLayout;
    controls_h_layout2->setMargin(0);
    controls_h_layout2->setSpacing(0);
    controls_h_layout2->setMargin(0);
    controls_h_layout2->addStretch();
    controls_h_layout2->addWidget(mp_fps_Label, 0, Qt::AlignRight);
    controls_h_layout2->addSpacing(8);
    controls_h_layout2->addWidget(mp_timestamp_Label, 0, Qt::AlignRight);

    QVBoxLayout *controls_v_layout1 = new QVBoxLayout;
    controls_v_layout1->setSpacing(0);
    controls_v_layout1->setMargin(0);
    controls_v_layout1->addLayout(controls_h_layout1);
    controls_v_layout1->addLayout(controls_h_layout2);

    QHBoxLayout *controls_h_layout = new QHBoxLayout;
#ifdef __APPLE__
    controls_h_layout->setSpacing(20);
#else
    controls_h_layout->setSpacing(4);
#endif
    controls_h_layout->setMargin(0);
    controls_h_layout->addWidget(mp_play_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addWidget(mp_stop_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addWidget(mp_repeat_PushButton, 0, Qt::AlignTop);
    controls_h_layout->addStretch();
    controls_h_layout->addLayout(controls_v_layout1);

    QVBoxLayout *controls_v_layout = new QVBoxLayout;
    controls_v_layout->setSpacing(5);
    controls_v_layout->setMargin(5);
    controls_v_layout->addLayout(slider_h_layout);
    controls_v_layout->addLayout(controls_h_layout);

    QVBoxLayout *main_vlayout = new QVBoxLayout;
    main_vlayout->setSpacing(0);
    main_vlayout->setMargin(0);
    main_vlayout->addWidget(mp_frame_image_Widget, 2);
    main_vlayout->addLayout(controls_v_layout);

    // Set layout in QWidget
    QWidget *main_widget = new QWidget;
    main_widget->setLayout(main_vlayout);

    setCentralWidget(main_widget);
    setWindowTitle(C_WINDOW_TITLE_QSTRING);

    mp_ser_file = new c_pipp_ser;

    mp_frame_Timer = new QTimer(this);
    connect(mp_frame_Timer, SIGNAL(timeout()), this, SLOT(frame_timer_timeout_slot()));

    mp_resize_Timer = new QTimer(this);
    mp_resize_Timer->setSingleShot(true);
    connect(mp_resize_Timer, SIGNAL(timeout()), this, SLOT(resize_timer_timeout_slot()));

    connect(mp_forward_PushButton, SIGNAL(pressed()),
            this, SLOT(forward_button_pressed_slot()));

    connect(mp_forward_PushButton, SIGNAL(released()),
            this, SLOT(forward_button_released_slot()));

    connect(mp_back_PushButton, SIGNAL(pressed()),
            this, SLOT(back_button_pressed_slot()));

    connect(mp_back_PushButton, SIGNAL(released()),
            this, SLOT(back_button_released_slot()));

    connect(mp_play_PushButton, SIGNAL(pressed()),
            this, SLOT(play_button_pressed_slot()));

    connect(mp_stop_PushButton, SIGNAL(pressed()),
            this, SLOT(stop_button_pressed_slot()));

    connect(mp_repeat_PushButton, SIGNAL(toggled(bool)),
            this, SLOT(repeat_button_toggled_slot(bool)));

    connect(mp_frame_Slider, SIGNAL(valueChanged(int)),
            this, SLOT(frame_slider_changed_slot()));

    connect(mp_frame_image_Widget, SIGNAL(double_click_signal()),
            this, SLOT(resize_window_100_percent_slot()));

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
#endif
}


c_ser_player::~c_ser_player()
{

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
}


void c_ser_player::fps_changed_slot(QAction *action)
{
    if (action != NULL) {
        m_display_framerate = action->data().toInt();
        mp_framerate_Menu->setTitle(tr("Display Framerate"));
        calculate_display_framerate();
    }
}


void c_ser_player::direction_changed_slot(QAction *action)
{
    if (action != NULL) {
        mp_frame_Slider->set_direction(action->data().toInt());
    }
}


// Colour settings menu QAction has been clicked
void c_ser_player::colour_settings_slot()
{
    mp_colour_settings_Dialog->show();
}


// Colour settings menu QAction has been clicked
void c_ser_player::save_frames_slot()
{
    // Pause playback if currently playing
    bool restart_playing = false;
    if (m_current_state == STATE_PLAYING) {
        // Pause playing while frame is saved
        restart_playing = true;
        play_button_pressed_slot();
    }

    // Use save_frames dialog to get range of frames to be saved
    c_save_frames_dialog *save_frames_Dialog = new c_save_frames_dialog(this,
                                                                        m_total_frames,
                                                                        mp_frame_Slider->get_start_frame(),
                                                                        mp_frame_Slider->get_end_frame(),
                                                                        mp_frame_Slider->get_markers_active());
    int ret = save_frames_Dialog->exec();

    if (ret != QDialog::Rejected &&
        m_current_state != STATE_NO_FILE &&
        m_current_state != STATE_PLAYING) {

        // Get image filename and type to use
        const QString jpg_ext = QString(tr(".jpg"));
        const QString jpg_filter = QString(tr("Joint Picture Expert Group Image (*.jpg)", "Filetype filter"));
        const QString bmp_ext = QString(tr(".bmp"));
        const QString bmp_filter = QString(tr("Windows Bitmap Image (*.bmp)", "Filetype filter"));
        const QString png_ext = QString(tr(".png"));
        const QString png_filter = QString(tr("Portable Network Graphics Image (*.png)", "Filetype filter"));
        const QString tif_ext = QString(tr(".tif"));
        const QString tif_filter = QString(tr("Tagged Image File Format (*.tif)", "Filetype filter"));
        QString selected_filter;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Frames As Images"),
                                   m_ser_directory,
                                   jpg_filter + ";; " + bmp_filter + ";; " + png_filter + ";; " + tif_filter,
                                   &selected_filter);
        const char *p_format = NULL;
        if (!filename.isEmpty() && !selected_filter.isEmpty()) {
            if (selected_filter == jpg_filter) {
                p_format = "JPG";
            }

            if (selected_filter == bmp_filter) {
                p_format = "BMP";
            }

            if (selected_filter == png_filter) {
                p_format = "PNG";
            }

            if (selected_filter == tif_filter) {
                p_format = "TIFF";
            }

            int min_frame = save_frames_Dialog->get_start_frame();
            int max_frame = save_frames_Dialog->get_end_frame();

            if (min_frame == -1) {
                // Save current frame only
                QFile file(filename);
                file.open(QIODevice::WriteOnly);

                // Get frame from ser file
                QImage *frame_as_qimage = get_frame_as_qimage(mp_frame_Slider->value());
                QPixmap::fromImage(*frame_as_qimage).save(&file, p_format);
                file.close();
                delete frame_as_qimage;
            } else {
                // Save the range of frames specified by min_frame and max_frame
                QString filepath = QFileInfo(filename).absolutePath();
                QString filename_without_extension = QFileInfo(filename).completeBaseName();
                QString filename_extension = QFileInfo(filename).suffix();

                // Setup progress dialog
                c_save_frames_progress_dialog my_progress_dialog(this, min_frame, max_frame);
                my_progress_dialog.show();
                QString progress_title = tr("Saving %1 frames").arg(max_frame - min_frame + 1);
                int required_digits_for_number = (int)ceil(log10((double)max_frame));

                for (int frame = min_frame; frame <= max_frame; frame++) {
                    // Update progress bar
                    my_progress_dialog.set_value(frame);

                    // Insert frame number into filename
                    QString frame_number_string = QString("%1").arg(frame, required_digits_for_number, 10, QChar('0'));
                    QString new_filename = filepath +
                                           QDir::separator() +
                                           filename_without_extension +
                                           QString("_") +
                                           frame_number_string +
                                           "." +
                                           filename_extension;

                    // Open file for writing
                    QFile file(new_filename);
                    file.open(QIODevice::WriteOnly);

                    // Get frame from ser file
                    QImage *frame_as_qimage = get_frame_as_qimage(frame);

                    // Save the frame and close image file
                    QPixmap::fromImage(*frame_as_qimage).save(&file, p_format);
                    file.close();
                    delete frame_as_qimage;

                    if (my_progress_dialog.was_cancelled()) {
                        // Abort frame saving
                        break;
                    }
                }

                // Processing has completed
                my_progress_dialog.set_complete();
                while (!my_progress_dialog.was_cancelled()) {
                      // Wait
                }

            }
        }
    }

    // Restart playing if it was playing to start with
    if (restart_playing == true) {
        play_button_pressed_slot();
    }
}


void c_ser_player::colour_saturation_changed_slot(double value)
{
    m_colour_saturation = value;
    frame_slider_changed_slot();
}


void c_ser_player::colour_balance_changed_slot(double red, double green, double blue)
{
    image_functions::set_colour_balance_luts(red, green, blue);
    frame_slider_changed_slot();
}


void c_ser_player::estimate_colour_balance()
{
    if (m_current_state != STATE_NO_FILE) {
        mp_frame_slider_changed_Mutex->lock();

        // Get frame from SER file
        mp_ser_file_Mutex->lock();
        int32_t frame_size = m_frame_details.width * m_frame_details.height * mp_ser_file->get_bytes_per_sample() * 3;
        delete[] m_frame_details.p_buffer;
        m_frame_details.p_buffer = new uint8_t[frame_size];
        int32_t ret = mp_ser_file->get_frame(mp_frame_Slider->value(), m_frame_details.p_buffer);
        mp_ser_file_Mutex->unlock();

        if (mp_ser_file->get_bytes_per_sample() == 2) {
            image_functions::convert_image_to_8bit(m_frame_details);
        }

        if (ret >= 0) {
            // Debayer frame if required
            bool image_debayered = false;
            if (c_persistent_data::m_enable_debayering) {
                image_debayered = image_functions::debayer_image_bilinear(m_frame_details);
            }
        }

        mp_frame_slider_changed_Mutex->unlock();

        if (ret >= 0) {
            double red_gain = 1.0;
            double green_gain = 1.0;
            double blue_gain = 1.0;
            image_functions::estimate_colour_balance(red_gain, green_gain, blue_gain, m_frame_details);
            mp_colour_settings_Dialog->set_colour_balance(red_gain, green_gain, blue_gain);
        }
    }
}


void c_ser_player::zoom_changed_slot(QAction *action)
{
    if (action != NULL) {
        resize_window_with_zoom(action->data().toInt());
    }
}


void c_ser_player::language_changed_slot(QAction *action)
{
    if (action != NULL) {
        c_persistent_data::m_selected_language = action->data().toString();
    }
}


void c_ser_player::about_qt()
{
    QMessageBox::aboutQt(NULL, tr("About Qt", "Message box title"));
}


void c_ser_player::open_ser_file_slot()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open SER File", "Open file dialog title"),
                                                    c_persistent_data::m_ser_directory,
                                                    tr("SER Files (*.ser)", "Filetype filter"));

    if (!filename.isEmpty()) {
        c_persistent_data::m_ser_directory = QFileInfo(filename).absolutePath();
        open_ser_file(filename);
    }
}


void c_ser_player::open_ser_file(const QString &filename)
{
    stop_button_pressed_slot();  // Stop and reset and currently playing frame
    mp_ser_file_Mutex->lock();
    mp_ser_file->close();
    m_total_frames = mp_ser_file->open(filename.toStdString().c_str(), 0, 0);

    if (m_total_frames <= 0) {
        // Invalid SER file
        if (mp_ser_file->get_error_string().length() > 0) {
            QMessageBox::warning(NULL,
                                 tr("Invalid SER File", "Message box title for invalid SER file"),
                                 mp_ser_file->get_error_string());
        }

        mp_ser_file_Mutex->unlock();

    } else {
        // This is a valid SER file
        mp_frame_Slider->set_maximum_frame(m_total_frames);
        mp_frame_Slider->reset_all_markers_slot();

        m_save_frames_Act->setEnabled(true);
        QString ser_filename = pipp_get_filename_from_filepath(filename.toStdString());
        m_ser_directory = QFileInfo(filename).canonicalPath();  // Remember SER file directory
        setWindowTitle(ser_filename + " - " + C_WINDOW_TITLE_QSTRING);

        m_frame_details.width = mp_ser_file->get_width();
        m_frame_details.height = mp_ser_file->get_height();
        m_frame_details.colour_id = mp_ser_file->get_colour_id();
        m_frame_details.p_buffer = new uint8_t[m_frame_details.width * m_frame_details.height * mp_ser_file->get_bytes_per_sample() * 3];
        mp_ser_file->get_frame(m_frame_details.p_buffer);

        if (mp_ser_file->get_bytes_per_sample() == 2) {
            image_functions::convert_image_to_8bit(m_frame_details);
        }

        // Debayer frame if required
        bool image_debayered = false;
        if (c_persistent_data::m_enable_debayering) {
            image_debayered = image_functions::debayer_image_bilinear(m_frame_details);
        }

        // Adjust colour saturation if required
        if (image_debayered || m_frame_details.colour_id == COLOURID_RGB || m_frame_details.colour_id == COLOURID_BGR) {
            image_functions::change_colour_balance(
                m_frame_details); // struct s_image_details &image_details

            image_functions::change_colour_saturation(
                m_colour_saturation,  // double saturation
                m_frame_details); // struct s_image_details &image_details
        }

        image_functions::conv_data_ready_for_qimage(
            image_debayered,  // bool image_debayered
            m_frame_details); // struct s_image_details &image_details

        delete mp_frame_Image;
        mp_frame_Image = new QImage(m_frame_details.p_buffer, m_frame_details.width, m_frame_details.height, QImage::Format_RGB888);
        mp_frame_image_Widget->setPixmap(QPixmap::fromImage(*mp_frame_Image));

        m_current_state = STATE_STOPPED;
        mp_frame_Slider->goto_first_frame();
        mp_frame_size_Label->setText(m_frame_size_label_String
                                  .arg(m_frame_details.width)
                                  .arg(m_frame_details.height));
        mp_pixel_depth_Label->setText(m_pixel_depth_label_String.arg(mp_ser_file->get_pixel_depth()));
        m_is_colour = false;

        switch (m_frame_details.colour_id) {
        case COLOURID_MONO:
            mp_colour_id_Label->setText(tr("MONO", "Colour ID label"));
            break;
        case COLOURID_BAYER_RGGB:
            mp_colour_id_Label->setText(tr("RGGB", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_GRBG:
            mp_colour_id_Label->setText(tr("GRBG", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_GBRG:
            mp_colour_id_Label->setText(tr("GBRG", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_BGGR:
            mp_colour_id_Label->setText(tr("BGGR", "Colour ID label"));
            m_has_bayer_pattern = true;
            break;
        case COLOURID_BAYER_CYYM:
            mp_colour_id_Label->setText(tr("CYYM", "Colour ID label"));
            break;
        case COLOURID_BAYER_YCMY:
            mp_colour_id_Label->setText(tr("YCMY", "Colour ID label"));
            break;
        case COLOURID_BAYER_YMCY:
            mp_colour_id_Label->setText(tr("YMCY", "Colour ID label"));
            break;
        case COLOURID_BAYER_MYYC:
            mp_colour_id_Label->setText(tr("MYYC", "Colour ID label"));
            break;
        case COLOURID_RGB:
            mp_colour_id_Label->setText(tr("RGB", "Colour ID label"));
            m_is_colour = true;
            break;
        case COLOURID_BGR:
            mp_colour_id_Label->setText(tr("BGR", "Colour ID label"));
            m_is_colour = true;
            break;
        default:
            mp_colour_id_Label->setText(tr("????", "Colour ID label for unknown ID"));
        }

        if (m_is_colour || (m_has_bayer_pattern && c_persistent_data::m_enable_debayering)) {
            // This is now a colour image, enable colour saturation menu
            mp_colour_settings_action->setEnabled(true);
        } else {
            mp_colour_settings_action->setEnabled(false);
            mp_colour_settings_Dialog->hide();
        }

        m_debayer_Act->setEnabled(m_has_bayer_pattern);
        mp_framerate_Menu->setEnabled(true);
        mp_direction_Menu->setEnabled(true);
        calculate_display_framerate();

        uint64_t ts = mp_ser_file->get_timestamp();
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

            mp_timestamp_Label->setText(m_timestamp_label_String
                                     .arg(ts_year, 4, 10, QLatin1Char( '0' ))
                                     .arg(ts_month, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_day, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_hour, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_minute, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_second, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_microsec, 6, 10, QLatin1Char( '0' )));
        } else {
            mp_timestamp_Label->setText(tr("No Timestamp", "Timestamp label for no timestamp"));
        }

        mp_ser_file_Mutex->unlock();
        frame_slider_changed_slot();
        resize_window_100_percent_slot();
        play_button_pressed_slot();  // Start playing SER file
    }
}


void c_ser_player::frame_slider_changed_slot()
{
    // Update image to new frame
    if (m_current_state == STATE_NO_FILE) {
        mp_frame_Slider->setValue(1);
    } else {
        mp_frame_slider_changed_Mutex->lock();
        mp_framecount_Label->setText(m_framecount_label_String.arg(mp_frame_Slider->value()).arg(m_total_frames));

        mp_ser_file_Mutex->lock();
        int32_t frame_size = m_frame_details.width * m_frame_details.height * mp_ser_file->get_bytes_per_sample() * 3;
        delete[] m_frame_details.p_buffer;
        m_frame_details.p_buffer = new uint8_t[frame_size];
        int32_t ret = mp_ser_file->get_frame(mp_frame_Slider->value(), m_frame_details.p_buffer);
        uint64_t ts = mp_ser_file->get_timestamp();
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

            mp_timestamp_Label->setText(m_timestamp_label_String
                                     .arg(ts_year, 4, 10, QLatin1Char( '0' ))
                                     .arg(ts_month, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_day, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_hour, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_minute, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_second, 2, 10, QLatin1Char( '0' ))
                                     .arg(ts_microsec, 6, 10, QLatin1Char( '0' )));
        }
        mp_ser_file_Mutex->unlock();

        if (ret >= 0) {
            if (mp_ser_file->get_bytes_per_sample() == 2) {
                image_functions::convert_image_to_8bit(m_frame_details);
            }

            // Debayer frame if required
            bool image_debayered = false;
            if (c_persistent_data::m_enable_debayering) {
                image_debayered = image_functions::debayer_image_bilinear(m_frame_details);
            }

            if (image_debayered || m_frame_details.colour_id == COLOURID_RGB || m_frame_details.colour_id == COLOURID_BGR) {
                // Adjust colour saturation and balance if required
                image_functions::change_colour_balance(
                    m_frame_details); // struct s_image_details &image_details

                image_functions::change_colour_saturation(
                    m_colour_saturation,  // double saturation
                    m_frame_details); // struct s_image_details &image_details
            }

            image_functions::conv_data_ready_for_qimage(
                image_debayered,  // bool image_debayered
                m_frame_details); // struct s_image_details &image_details

            delete mp_frame_Image;
            mp_frame_Image = new QImage(m_frame_details.p_buffer, m_frame_details.width, m_frame_details.height, QImage::Format_RGB888);
            mp_frame_image_Widget->setPixmap(QPixmap::fromImage(*mp_frame_Image));
            //m_frame_image_label->setMinimumSize(QSize(1, 1));
        } else {
            mp_frame_Timer->stop();
            mp_play_PushButton->setIcon(m_play_Pixmap);
        }

        mp_frame_slider_changed_Mutex->unlock();
    }
}


void c_ser_player::frame_timer_timeout_slot()
{
    if (m_current_state == STATE_NO_FILE) {
        mp_frame_Timer->stop();
        mp_play_PushButton->setIcon(m_play_Pixmap);
    } else {
        if (!mp_frame_Slider->goto_next_frame()) {
            // End of playback
            m_current_state = STATE_FINISHED;
        }

        if (m_current_state != STATE_PLAYING) {
            mp_frame_Timer->stop();
            mp_play_PushButton->setIcon(m_play_Pixmap);
        }
    }
}


void c_ser_player::forward_button_pressed_slot()
{
    if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
        bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
        int value = mp_frame_Slider->value();

        if (shift_key) {
            value += 50;
        } else {
            value++;
        }

        if (value > mp_frame_Slider->maximum()) {
            value = mp_frame_Slider->maximum();
        }

        mp_frame_Slider->setValue(value);

        // Start repeat timer
        m_forward_button_held = true;
        QTimer::singleShot(500, this, SLOT(forward_button_held_slot()));
    }
}


void c_ser_player::forward_button_released_slot()
{
    m_forward_button_held = false;
}


void c_ser_player::forward_button_held_slot()
{
    if (m_forward_button_held &&
        mp_forward_PushButton->rect().contains(mp_forward_PushButton->mapFromGlobal(QCursor::pos())) &&
        QApplication::mouseButtons() & Qt::LeftButton) {
        // Forward button is still being held
        if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
            bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
            int value = mp_frame_Slider->value();

            if (shift_key) {
                value += 50;
            } else {
                value++;
            }

            if (value > mp_frame_Slider->maximum()) {
                value = mp_frame_Slider->maximum();
            }

            mp_frame_Slider->setValue(value);
        }

        // Re-set repeat timer
        QTimer::singleShot(100, this, SLOT(forward_button_held_slot()));
    }
}


void c_ser_player::back_button_pressed_slot()
{
    if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
        bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
        int value = mp_frame_Slider->value();

        if (shift_key) {
            value -= 50;
        } else {
            value--;
        }

        if (value < mp_frame_Slider->minimum()) {
            value = mp_frame_Slider->minimum();
        }

        mp_frame_Slider->setValue(value);

        // Start repeat timer
        m_back_button_held = true;
        QTimer::singleShot(500, this, SLOT(back_button_held_slot()));
    }
}


void c_ser_player::back_button_released_slot()
{
    m_back_button_held = false;
}



void c_ser_player::back_button_held_slot()
{
    if (m_back_button_held &&
        mp_back_PushButton->rect().contains(mp_back_PushButton->mapFromGlobal(QCursor::pos())) &&
        QApplication::mouseButtons() & Qt::LeftButton) {
        // Forward button is still being held
        if (m_current_state != STATE_NO_FILE && m_current_state != STATE_PLAYING) {
            bool shift_key = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
            int value = mp_frame_Slider->value();

            if (shift_key) {
                value -= 50;
            } else {
                value--;
            }

            if (value < mp_frame_Slider->minimum()) {
                value = mp_frame_Slider->minimum();
            }

            mp_frame_Slider->setValue(value);
        }

        // Re-set repeat timer
        QTimer::singleShot(100, this, SLOT(back_button_held_slot()));
    }
}


void c_ser_player::play_button_pressed_slot()
{
    if (m_current_state == STATE_NO_FILE) {
        open_ser_file_slot();
    } else {
        if (m_current_state == STATE_PLAYING) {
            // Stop playing
            m_current_state = STATE_PAUSED;
            mp_play_PushButton->setIcon(m_play_Pixmap);
        } else if (m_current_state == STATE_FINISHED) {
            // Start playing from start
            m_current_state = STATE_PLAYING;
            mp_play_PushButton->setIcon(m_pause_Pixmap);
            mp_frame_Timer->start(m_display_frame_time);
            mp_frame_Slider->goto_first_frame();
        } else {
            // Start playing from current position if not before start marker
            m_current_state = STATE_PLAYING;
            mp_play_PushButton->setIcon(m_pause_Pixmap);
            mp_frame_Slider->goto_next_frame();
            mp_frame_Timer->start(m_display_frame_time);
        }
    }
}


void c_ser_player::stop_button_pressed_slot()
{
    if (m_current_state != STATE_NO_FILE) {
        m_current_state = STATE_STOPPED;
        mp_play_PushButton->setIcon(m_play_Pixmap);
        mp_frame_Timer->stop();
        mp_frame_Slider->goto_first_frame();
    }
}


void c_ser_player::repeat_button_toggled_slot(bool checked) {
    c_persistent_data::m_repeat = checked;
    mp_frame_Slider->set_repeat(checked);
}



void c_ser_player::resize_window_100_percent_slot()
{
    resize_window_with_zoom(100);
}


void c_ser_player::resize_window_with_zoom(int zoom)
{
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


void c_ser_player::debayer_enable_slot(bool enabled)
{
    c_persistent_data::m_enable_debayering = enabled;
    if (m_is_colour || (m_has_bayer_pattern && c_persistent_data::m_enable_debayering)) {
        // This is now a colour image, enable colour saturation menu
        mp_colour_settings_action->setEnabled(true);
    } else {
        mp_colour_settings_action->setEnabled(false);
        mp_colour_settings_Dialog->hide();
    }

    frame_slider_changed_slot();
}


void c_ser_player::new_version_available_slot(QString version)
{
    c_persistent_data::m_new_version = version;
}



void c_ser_player::about_ser_player()
{
    QPixmap ser_player_logoPixmap(":/res/resources/ser_player_logo_150x150.png");
    QMessageBox msgBox;
    msgBox.setText("<b><big>" + tr("SER Player") + "</big> (" VERSION_STRING ")</b>");
    QString informative_text = tr("A simple video player for SER files.");
    informative_text += "<qt><a href=\"http://sites.google.com/site/astropipp/\">http://sites.google.com/site/astropipp/</a><br>";
    informative_text += "Copyright (c) 2015 Chris Garry";

    QString translator_credit = tr("English language translation by Chris Garry",
                                   "Translator credit - Replace language and translator names when translating");
    if (translator_credit != "English language translation by Chris Garry") {
        // Add a credit for the translator if the translator credit string has been translated
        informative_text += ("<qt>");
        informative_text += translator_credit;
    }

    informative_text += "<qt>&nbsp;<br>";
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

    msgBox.setInformativeText(informative_text);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.setWindowTitle(tr("About SER Player", "About box title"));
    msgBox.setIconPixmap(ser_player_logoPixmap);
    msgBox.exec();
}


void c_ser_player::create_no_file_open_image()
{
    QString no_file_open_string = QString(tr("No SER File Open", "No SER file message on inital image"));
    m_no_file_open_Pixmap = QPixmap::fromImage(*mp_frame_Image);
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


void c_ser_player::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    mp_resize_Timer->start(1);
}


void c_ser_player::resize_timer_timeout_slot()
{
    int zoom_level = mp_frame_image_Widget->get_zoom_level();
    mp_zoom_Label->setText(m_zoom_label_String.arg(zoom_level));
}


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

    mp_fps_Label->setText(m_fps_label_String.arg(fps));
    mp_frame_Timer->setInterval(m_display_frame_time);
}


QImage *c_ser_player::get_frame_as_qimage(int frame_number)
{
    mp_ser_file_Mutex->lock();
    delete[] m_frame_details.p_buffer;
    m_frame_details.p_buffer = new uint8_t[m_frame_details.width * m_frame_details.height * mp_ser_file->get_bytes_per_sample() * 3];
    int32_t ret = mp_ser_file->get_frame(frame_number, m_frame_details.p_buffer);
    mp_ser_file_Mutex->unlock();

    QImage *p_frame_as_qimage = NULL;
    if (ret >= 0) {
        if (mp_ser_file->get_bytes_per_sample() == 2) {
            image_functions::convert_image_to_8bit(m_frame_details);
        }

        // Debayer frame if required
        bool image_debayered = false;
        if (c_persistent_data::m_enable_debayering) {
            image_debayered = image_functions::debayer_image_bilinear(m_frame_details);
        }

        if (image_debayered || m_frame_details.colour_id == COLOURID_RGB || m_frame_details.colour_id == COLOURID_BGR) {
            // Adjust colour saturation and balance if required
            image_functions::change_colour_balance(
                m_frame_details); // struct s_image_details &image_details

            image_functions::change_colour_saturation(
                m_colour_saturation,  // double saturation
                m_frame_details); // struct s_image_details &image_details
        }

        image_functions::conv_data_ready_for_qimage(
            image_debayered,  // bool image_debayered
            m_frame_details); // struct s_image_details &image_details


        p_frame_as_qimage = new QImage(m_frame_details.p_buffer, m_frame_details.width, m_frame_details.height, QImage::Format_RGB888);
    }

    return p_frame_as_qimage;
}
