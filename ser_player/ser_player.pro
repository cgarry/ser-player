#-------------------------------------------------
#
# Project created by QtCreator 2015-05-15T15:03:40
#
#-------------------------------------------------

QT += core gui
QT += network
CONFIG += c++11
CONFIG += warn_on

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
    src/application.cpp \
    src/new_version_checker.cpp

    macx:SOURCES += src/pipp_utf8_osx.cpp
    linux:SOURCES += src/pipp_utf8_linux.cpp
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
DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../
linix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac


# Icons
macx:ICON = os_x/ser_player_icon.icns
win32:RC_ICONS = windows/ser_player_icon.ico

RESOURCES += \
    images.qrc

macx:release:QMAKE_POST_LINK = ../ser_player/os_x/post_compile.sh
win32:release:QMAKE_POST_LINK = %QTDIR%\\bin\\windeployqt --force --no-translations \"$$DESTDIR/SER-Player.exe\"
#win32:release:QMAKE_POST_LINK = %QTDIR%\\bin\\windeployqt --force \"$$DESTDIR/SER_Player.exe\"
#linux:release:rm ../linux_release/get_qtdir.sh ; echo "QTDIR=$QTDIR"
linux:release:QMAKE_POST_LINK = rm -f ../linux_release/get_qtdir.sh && echo QTDIR=${QTDIR} > ../linux_release/get_qtdir.sh
