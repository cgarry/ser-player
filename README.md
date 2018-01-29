# SER Player
A simple video player for playing SER files used for solar, lunar and planetary astronomy-imaging.

SER Player is a C++ application using the Qt cross-platform framework so that it can be built for Windows, macOS and Linux.

SER File Format Specification: [http://www.grischa-hahn.homepage.t-online.de/astro/ser/SER%20Doc%20V3b.pdf](http://www.grischa-hahn.homepage.t-online.de/astro/ser/SER%20Doc%20V3b.pdf)

### Project Goals
Some simple goals that hopefully get accross my intentions for this project.
* Produce a lightweight and intuative video player application for .ser files.
* ~~Avoid feature creep and over complication in order to keep SER Player distinct from [PIPP](https://sites.google.com/site/astropipp/).  New features may be added if they are not duplicating PIPP's functionality.~~
* SER Player must remain cross-platform and support at least Windows, macOS and Linux.

## Building SER Player for Linux

### Building using the Terminal

This section has some basic notes on building the application for Linux using the terminal.  The instructions below are for variations of Debian Linux and will need to be modified for other Linux distributions.  On Debian these packages need to be installed: qt5-qmake, qtbase5-dev and libpng-dev.

- Terminal $ **cd ser-player**
- Terminal $ **export QT_SELECT=5** (This selects the QT5 version of qmake over the QT4 version)
- Terminal $ **qmake CONFIG+=release** (Or **qmake DEFINES+=DISABLE_NEW_VERSION_CHECK CONFIG+=release**)
- Terminal $ **make**

The directory **ser-player/bin/** will now contain the application executable.  To install the application use the following:

- Terminal $ **sudo make install**

After this SER Player should be fully installed to the desktop and associated with .ser files so they can be double-clicked to view them in SER Player.  To uninstall SER Player use the following:

Terminal $ **sudo make uninstall**

## Building SER Player for Windows

This section has some basic notes on building the application for Windows.  These notes assume a Windows PC is being used to build the application.
* Install Qt v5.4.1 or later and Qt Creator
* Open Qt project file **ser-player\ser_player.pro** with Qt Creator
* Select **Release** instead of **Debug**.
* Do **Build Menu->Run qmake** to run qmake.
* Do **Build Menu->Build All** to build the application.
* The directory **ser-player\bin\** will contain the application executable and many DLL files required for it to run.

With the application built we next need to build an installer.  This is done using the [NSIS](http://nsis.sourceforge.net/Download) Windows installer creation program.
* Download and install NSIS 3.01
* Start NSIS and click on **Compile NSI scripts**.
* This will open a **MakeNSISW** window.
* Open file **ser-player\ser_player_nsis\ser_player64.nsi** with the **MakeNSISW** window.  The simplest way is to drag the file and drop it in the window.  This is for the 64-bit version of SER Player.  Use the ser_player32.nsi file for the 32-bit version.
* This will run the NSIS installer creator and generate the installer file of this form: **ser-player\ser_player_nsis\SER_Player_Windows_x64_v1.7.1.79.exe**

This installer file can now be use to install SER Player on your system.


## Building SER Player for macOS
This section has some basic notes on building the application for macOS.  These notes assume a Mac running macOS is being used to build the application.
* Install Qt v5.4.1 or later and Qt Creator
* Open Qt project file **ser-player\ser_player.pro** with Qt Creator
* Select **Release** instead of **Debug**.
* Do **Build Menu->Run qmake** to run qmake.
* Do **Build Menu->Build All** to build the application.
* This Apple disk image (.dmg) file **ser_player\bin\SER-Player.dmg** will created.

This disk image file contains the SER-Player application and can install in the usual way.

