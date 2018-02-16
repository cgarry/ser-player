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
cat > appdir/AppRun <<EOL
#!/bin/bash

if [[ \$1 == --install ]]; then
    echo "Installing SER Player ($VERSION) to desktop"
    # Copy desktop file, icon files and mime file to the system
    cp -r "\$APPDIR/usr/share/" "\$HOME/.local/"
    
    # Modify desktop file to point to AppImage
    rm -f "\$HOME/.local/share/applications/com.google.sites.ser-player.desktop"
    cat "\$APPDIR/usr/share/applications/com.google.sites.ser-player.desktop" | sed -e "s:Exec=ser-player \%F:Exec=\$HOME/.local/bin/ser-player \%F:" > "\$HOME/.local/share/applications/com.google.sites.ser-player.desktop"

    # Copy the actual AppImage into $HOME/.local/bin/
    mkdir -p "$HOME/.local/bin/"
    cp \$APPIMAGE "\$HOME/.local/bin/ser-player"

    # Update icon cache
    which gtk-update-icon-cache && gtk-update-icon-cache "\$HOME/.local/share/icons/hicolor/" -t
    # Update mime to filetype database
    which update-mime-database && update-mime-database "\$HOME/.local/share/mime/"
    # Update mime to application database
    which update-desktop-database && update-desktop-database "\$HOME/.local/share/applications/"

elif [[ \$1 == --uninstall ]]; then
    echo "Uninstalling SER Player from desktop"
    # Delete icon files
    find "\$HOME/.local/share/icons/hicolor/" -name "ser-player.*" -exec rm -f {} \;
    # Delete mime file
    rm -f "\$HOME/.local/share/mime/packages/ser-player.xml"
    # Delete desktop file
    rm -f "\$HOME/.local/share/applications/com.google.sites.ser-player.desktop"
    # Delete the application binary
    rm -f "\$HOME/.local/bin/ser-player"

    # Update icon cache
    which gtk-update-icon-cache && gtk-update-icon-cache "\$HOME/.local/share/icons/hicolor/" -t
    # Update mime to filetype database
    which update-mime-database && update-mime-database "\$HOME/.local/share/mime/"
    # Update mime to application database
    which update-desktop-database && update-desktop-database "\$HOME/.local/share/applications/"

elif [[ \$1 == --help ]]; then
    echo "SER Player ($VERSION) AppImage Command Line Arguments"
    echo "  --help       Display this help."
    echo "  --install    Install SER Player to desktop and associate with .ser files."
    echo "  --uninstall  Uninstall SER Player from Desktop and remove associations."
    echo

else
    echo APPDIR: \$APPDIR
    ls -l "\$APPDIR"
    exec "\$APPDIR/usr/bin/ser-player" "\$@"
fi
EOL
chmod a+x appdir/AppRun

# Use appimagetool to create the final AppImage from the appdir
./appimagetool-x86_64.AppImage appdir