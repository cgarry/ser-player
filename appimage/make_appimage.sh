#!/bin/bash

# Helper file for generating a SER Player Linux AppImage
# ------------------------------------------------------
# This file is designed for to be used on Ubuntu 14.04
# Either 64-bit or 32-bit depending on the version of
# AppImage required.
#
# From ser-player.pro folder enter this on the command line:
# source appimage/make_appimage.sh
#

MACHINE_ARCH=`uname -m`
if [ ${MACHINE_ARCH} != 'x86_64' ]; then
  # 32-bit
  MACHINE_ARCH='i686'
fi

# Clean up any previous attempts to build an AppImage
rm -rf bin build appdir
rm *.AppImage

# Ensure the system is up to date
sudo apt-get update -qq
sudo apt-get upgrade

# Install some required packages
sudo apt-get -y install build-essential libgl1-mesa-dev libpng-dev

# Get the correct version of Qt
sudo add-apt-repository ppa:beineri/opt-qt593-trusty -y
sudo apt-get update -qq
sudo apt-get -y install qt59base
source /opt/qt*/bin/qt*-env.sh

# Build the SER Player binary
qmake CONFIG+=release BUILD_FOR_APPIMAGE=
make -j$(nproc)
ls -l bin/
ldd bin/ser-player
make INSTALL_ROOT=appdir -j$(nproc) install ; find appdir/

# Checkout and build linuxdeployqt if required
type linuxdeployqt >/dev/null 2>&1 || {
  git clone https://github.com/probonopd/linuxdeployqt.git
  ( cd linuxdeployqt/ && qmake && make && sudo make install )
  
  # Clean up
  rm -rf linuxdeployqt
}

# Get and build patchelf if required
type patchelf >/dev/null 2>&1 || {
  wget https://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.bz2
  tar xf patchelf-0.9.tar.bz2
  ( cd patchelf-0.9/ && ./configure  && make && sudo make install )
  
  # Clean up
  rm -rf patchelf-0.9
  rm patchelf-0.9.*
}

# Get appimagetool AppImage
wget -c -nv "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-$MACHINE_ARCH.AppImage"
chmod a+x "appimagetool-$MACHINE_ARCH.AppImage"

# Use linuxdeployqt to generate a populated appdir
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
source export_app_version.sh

linuxdeployqt appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -no-translations

# Remove AppRun symbolic link created by appimagetool in appdir
rm -f appdir/AppRun

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
    type gtk-update-icon-cache >/dev/null 2>&1 && gtk-update-icon-cache "\$HOME/.local/share/icons/hicolor/" -t
    # Update mime to filetype database
    type update-mime-database >/dev/null 2>&1 && update-mime-database "\$HOME/.local/share/mime/"
    # Update mime to application database
    type update-desktop-database >/dev/null 2>&1 && update-desktop-database "\$HOME/.local/share/applications/"

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
    type gtk-update-icon-cache >/dev/null 2>&1 && gtk-update-icon-cache "\$HOME/.local/share/icons/hicolor/" -t
    # Update mime to filetype database
    type update-mime-database >/dev/null 2>&1 && update-mime-database "\$HOME/.local/share/mime/"
    # Update mime to application database
    type update-desktop-database >/dev/null 2>&1 && update-desktop-database "\$HOME/.local/share/applications/"

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
./appimagetool-$MACHINE_ARCH.AppImage -v appdir
