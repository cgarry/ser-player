#-------------------------------------------------
#
# Project created by QtCreator 2015-05-15T15:03:40
#
#-------------------------------------------------

QT += core gui
!DISABLE_NEW_VERSION_CHECK: QT += network

CONFIG += c++11
CONFIG += warn_on
unix:!macx:QMAKE_CXXFLAGS += -std=gnu++0x

# Internationalisation
TRANSLATIONS = translations/ser_player_da.ts

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
    src/application.cpp

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
    src/new_version_checker.h

# Build directories
DESTDIR = $$PWD/bin
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui
unix:!macx:OBJECTS_DIR = $$PWD/build/o/unix
win32:OBJECTS_DIR = $$PWD/build/o/win32
macx:OBJECTS_DIR = $$PWD/build/o/mac


# Icons
macx:ICON = platform-specific/os-x/ser_player_icon.icns
win32:RC_ICONS = platform-specific/windows/ser_player_icon.ico

RESOURCES += \
    images.qrc

# Modify generated OS X package to fix shortcomings
macx:release:QMAKE_POST_LINK = ../platform-specific/os-x/post_compile.sh

# Call windeployqt.exe to budle all DLLs and so on required to run
win32:release:QMAKE_POST_LINK = windeployqt --force --no-translations \"$$DESTDIR/SER-Player.exe\"


