#-------------------------------------------------
#
# Project created by QtCreator 2015-05-15T15:03:40
#
#-------------------------------------------------

QT += core gui
QT += concurrent
!DISABLE_NEW_VERSION_CHECK: QT += network

CONFIG += c++11
CONFIG += warn_on
unix:!macx:QMAKE_CXXFLAGS += -std=gnu++0x

# Internationalisation
TRANSLATIONS = translations/ser_player_da.ts \
               translations/ser_player_fr.ts \
               translations/ser_player_de.ts

# Make Mac version as backwards compatible with old OS versions as possible
macx:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

# Remove standard function warnings on Windows
win32:DEFINES += _CRT_SECURE_NO_WARNINGS

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



TARGET = "ser-player"
TEMPLATE = app

SOURCES += src/main.cpp\
    src/ser_player.cpp \
    src/pipp_ser.cpp \
    src/pipp_buffer.cpp \
    src/persistent_data.cpp \
    src/pipp_timestamp.cpp \
    src/image_widget.cpp \
    src/application.cpp \
    src/frame_slider.cpp \
    src/save_frames_dialog.cpp \
    src/save_frames_progress_dialog.cpp \
    src/markers_dialog.cpp \
    src/image.cpp \
    src/histogram_thread.cpp \
    src/histogram_dialog.cpp \
    src/pipp_ser_write.cpp \
    src/header_details_dialog.cpp \
    src/utf8_validator.cpp \
    src/processing_options_dialog.cpp \
    src/icon_groupbox.cpp \
    src/gif_write.cpp \
    src/lzw_compressor.cpp \
    src/pipp_avi_write.cpp \
    src/pipp_avi_write_dib.cpp \
    src/selection_box_dialog.cpp

!DISABLE_NEW_VERSION_CHECK: SOURCES += src/new_version_checker.cpp

macx:SOURCES += src/pipp_utf8_osx.cpp
unix:!macx:SOURCES += src/pipp_utf8_linux.cpp
win32:SOURCES += src/pipp_utf8.cpp

HEADERS  += src/ser_player.h \
    src/pipp_ser.h \
    src/pipp_buffer.h \
    src/pipp_utf8.h \
    src/persistent_data.h \
    src/pipp_timestamp.h \
    src/image_widget.h \
    src/application.h \
    src/new_version_checker.h \
    src/frame_slider.h \
    src/save_frames_dialog.h \
    src/save_frames_progress_dialog.h \
    src/markers_dialog.h \
    src/image.h \
    src/histogram_thread.h \
    src/histogram_dialog.h \
    src/pipp_ser_write.h \
    src/header_details_dialog.h \
    src/utf8_validator.h \
    src/processing_options_dialog.h \
    src/icon_groupbox.h \
    src/gif_write.h \
    src/lzw_compressor.h \
    src/pipp_video_write.h \
    src/pipp_avi_write.h \
    src/pipp_avi_write_dib.h \
    src/selection_box_dialog.h

# Build directories
contains(QT_ARCH, i386) {
    DEFINES += BUILD_32_BIT
    win32:OBJECTS_DIR = $$PWD/build/o/win32
    win32:DESTDIR = $$PWD/bin32
} else {
    DEFINES += BUILD_64_BIT
    win32:OBJECTS_DIR = $$PWD/build/o/win64
    win32:DESTDIR = $$PWD/bin64
}

MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui
!win32:DESTDIR = $$PWD/bin
unix:!macx:OBJECTS_DIR = $$PWD/build/o/unix
macx:OBJECTS_DIR = $$PWD/build/o/mac
macx:PRO_FILE_DIR = $$PWD

# Icons
macx:ICON = platform-specific/os-x/ser_player_icon.icns
win32:RC_ICONS = platform-specific/windows/ser_player_icon.ico

RESOURCES += \
    images.qrc

# Modify generated OS X package to fix shortcomings
macx:release:QMAKE_POST_LINK = $$PWD/platform-specific/os-x/post_compile.sh

# Call windeployqt.exe to budle all DLLs and so on required to run
win32:release:QMAKE_POST_LINK = $$quote(windeployqt --force --no-translations \"$$DESTDIR/SER-Player.exe\"$$escape_expand(\n\t))

# SSL DLLs
win32 {
contains(QT_ARCH, i386) {
    EXTRA_BINFILES += $$PWD/platform-specific/windows/openssl/win32/libeay32.dll \
                      $$PWD/platform-specific/windows/openssl/win32/ssleay32.dll \
                      $$PWD/platform-specific/windows/openssl/win32/OpenSSL_License.txt
    EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
    EXTRA_BINFILES_WIN ~= s,/,\\,g
        DESTDIR_WIN = $${DESTDIR}
    DESTDIR_WIN ~= s,/,\\,g
    for(FILE,EXTRA_BINFILES_WIN){
                QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }
} else {
    EXTRA_BINFILES += $$PWD/platform-specific/windows/openssl/win64/libeay32.dll \
                      $$PWD/platform-specific/windows/openssl/win64/ssleay32.dll \
                      $$PWD/platform-specific/windows/openssl/win64/OpenSSL_License.txt
    EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
    EXTRA_BINFILES_WIN ~= s,/,\\,g
        DESTDIR_WIN = $${DESTDIR}
    DESTDIR_WIN ~= s,/,\\,g
    for(FILE,EXTRA_BINFILES_WIN){
                QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }
}
}


