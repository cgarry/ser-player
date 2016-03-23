#!/bin/bash

debian_version="1"
arch="amd64"

this_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# Get the version the of SER Player we are packaging - thsi is set manually
#Note this sets the ${ser_player_version} variable
source $this_dir/ser_player_version.sh

release_dir=$this_dir/ser-player_${ser_player_version}-${debian_version}_${arch}

# Get Qt path from script generated during build
source $this_dir/get_qmakedir_x64.sh 
QTDIR="$QMAKEDIR/../"

# Create emtpy directory structure
sudo rm -rf $release_dir
mkdir -p $release_dir/DEBIAN/
mkdir -p $release_dir/usr/bin/
mkdir -p $release_dir/usr/share/app-install/icons/
mkdir -p $release_dir/usr/share/applications/
mkdir -p $release_dir/usr/share/doc/ser-player/
mkdir -p $release_dir/usr/share/mime/packages/
mkdir -p $release_dir/usr/lib/ser-player/libs/
mkdir -p $release_dir/usr/lib/ser-player/platforms/
mkdir -p $release_dir/usr/lib/ser-player/plugins/imageformats/

# Change the file permissions of the directories
chmod -R 0755 $release_dir

# Copy start-up script to directory 
cp $this_dir/debian_files/ser-player $release_dir/usr/bin/
chmod 0755 $release_dir/usr/bin/ser-player

# Copy executable to directory and strip it
cp ../bin/ser-player $release_dir/usr/lib/ser-player/
strip -s $release_dir/usr/lib/ser-player/ser-player
chmod 0755 $release_dir/usr/lib/ser-player/ser-player
chrpath -d $release_dir/usr/lib/ser-player/ser-player

# Copy libs to directories
cp $QTDIR/lib/libQt5Widgets.so.5 $release_dir/usr/lib/ser-player/libs/
chmod 0644 $release_dir/usr/lib/ser-player/libs/libQt5Widgets.so.5
cp $QTDIR/lib/libQt5Gui.so.5 $release_dir/usr/lib/ser-player/libs/
chmod 0644 $release_dir/usr/lib/ser-player/libs/libQt5Gui.so.5
cp $QTDIR/lib/libQt5Network.so.5 $release_dir/usr/lib/ser-player/libs/
chmod 0644 $release_dir/usr/lib/ser-player/libs/libQt5Network.so.5
cp $QTDIR/lib/libQt5Core.so.5 $release_dir/usr/lib/ser-player/libs/
chmod 0644 $release_dir/usr/lib/ser-player/libs/libQt5Core.so.5
cp $QTDIR/lib/libQt5DBus.so.5 $release_dir/usr/lib/ser-player/libs/
chmod 0644 $release_dir/usr/lib/ser-player/libs/libQt5DBus.so.5
cp $QTDIR/lib/libicui18n.so.54 $release_dir/usr/lib/ser-player/libs/
strip -s $release_dir/usr/lib/ser-player/libs/libicui18n.so.54
chmod 0644 $release_dir/usr/lib/ser-player/libs/libicui18n.so.54
cp $QTDIR/lib/libicuuc.so.54 $release_dir/usr/lib/ser-player/libs/
strip -s $release_dir/usr/lib/ser-player/libs/libicuuc.so.54
chmod 0644 $release_dir/usr/lib/ser-player/libs/libicuuc.so.54
cp $QTDIR/lib/libicudata.so.54 $release_dir/usr/lib/ser-player/libs/
strip -s $release_dir/usr/lib/ser-player/libs/libicudata.so.54
chmod 0644 $release_dir/usr/lib/ser-player/libs/libicudata.so.54


# Copy plugins to directories
cp $QTDIR/plugins/platforms/libqxcb.so $release_dir/usr/lib/ser-player/platforms/
cp $QTDIR/plugins/imageformats/libqtiff.so $release_dir/usr/lib/ser-player/plugins/imageformats/
cp $QTDIR/plugins/imageformats/libqjpeg.so $release_dir/usr/lib/ser-player/plugins/imageformats/

# Copy other files into their directories
cp $this_dir/debian_files/control $release_dir/DEBIAN/control
sed -i s/_arch_/$arch/g $release_dir/DEBIAN/control
sed -i s/_ser_player_version_/$ser_player_version/g $release_dir/DEBIAN/control
sed -i s/_debian_version_/$debian_version/g $release_dir/DEBIAN/control
chmod 0644 $release_dir/DEBIAN/control
cp $this_dir/debian_files/ser-player.png $release_dir/usr/share/app-install/icons/
chmod 0644 $release_dir/usr/share/app-install/icons/ser-player.png
cp $this_dir/debian_files/ser-player.desktop $release_dir/usr/share/applications/
chmod 0644 $release_dir/usr/share/applications/ser-player.desktop
cp $this_dir/debian_files/copyright $release_dir/usr/share/doc/ser-player/
chmod 0644 $release_dir/usr/share/doc/ser-player/copyright
cp $this_dir/debian_files/changelog $release_dir/usr/share/doc/ser-player/
gzip --best $release_dir/usr/share/doc/ser-player/changelog
chmod 0644 $release_dir/usr/share/doc/ser-player/changelog.gz
cp $this_dir/debian_files/ser-player.xml $release_dir/usr/share/mime/packages/
chmod 0644 $release_dir/usr/share/mime/packages/ser-player.xml

# Get total size of package, rounded up to nearest KB
# Set the install size field in the control file with this value
total_size=$(du -sb $release_dir | cut -f1)
let "total_size += 1023"
let "total_size /= 1024"
sed -i s/_installed_size_/$total_size/g $release_dir/DEBIAN/control

# Change owner of release directory and files
sudo chown -R root: $release_dir

# Make debian package
sudo dpkg --build $release_dir

# Clean up
sudo rm -rf $release_dir

