echo Running Mac post compile script
cp ../ser_player/os_x/info.plist ../bin/ser-player.app/Contents/
$QTDIR/bin/macdeployqt ../bin/SER-Player.app -dmg
