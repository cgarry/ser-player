#!/bin/bash

# Helper file for generating a Linux AppImage
# From ser-player.pro folder enter this on the command line:
# source appimage/make_appimage.sh

# Clean up any previous attempts to build an AppImage
rm -rf bin build appdir
rm *.AppImage

# Build the SER Player binary
export QT_SELECT=5
qmake CONFIG+=release APPIMAGE=
make -j$(nproc)
ls -l bin/
ldd bin/ser-player
make INSTALL_ROOT=appdir -j$(nproc) install ; find appdir/

# Get linuxdeployqt AppImage
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage

# Get appimagetool AppImage
wget -c -nv "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod a+x appimagetool-x86_64.AppImage

# Use linuxdeployqt to generate a populated appdir
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
source export_app_version.sh
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -no-transla1
tions

# Remove AppRun symbolic link created by appimagetool
rm appdir/AppRun

# Create a script to replace the deleted AppRun link
echo '#!/bin/bash' > appdir/AppRun
echo 'echo from AppRun!' >> appdir/AppRun
echo 'pwd' >> appdir/AppRun
echo 'ls -l' >> appdir/AppRun
echo 'echo APPDIR: "$APPDIR"' >> appdir/AppRun
echo 'ls -l "$APPDIR"' >> appdir/AppRun
echo 'exec "$APPDIR/usr/bin/ser-player" "$@"' >> appdir/AppRun
chmod a+x appdir/AppRun

# Use appimagetool to create the final AppImage from the appdir
./appimagetool-x86_64.AppImage appdir
