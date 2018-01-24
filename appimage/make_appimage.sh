#!/bin/bash

# Helper file for generating a Linux AppImage
# From ser-player.pro folder enter this on the command line:
# source appimage/make_appimage.sh

rm -rf bin build appdir
rm *.AppImage

export QT_SELECT=5
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
ls -l bin/
ldd bin/ser-player
make INSTALL_ROOT=appdir -j$(nproc) install ; find appdir/
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
source export_app_version.sh
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -no-translations
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage
