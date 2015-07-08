echo Running Mac post compile script
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cp $SCRIPT_DIR/info.plist $SCRIPT_DIR/../../bin/ser-player.app/Contents/
$QTDIR/bin/macdeployqt $SCRIPT_DIR/../../bin/SER-Player.app -dmg
