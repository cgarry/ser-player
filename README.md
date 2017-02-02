##SER Player
A simple video player for playing SER files used for solar, lunar and planetary astronomy-imaging.

SER Player is a C++ application using the Qt cross-platform framework so that it can be built for Windows, OS X and Linux.

SER File Format Specification: [http://www.grischa-hahn.homepage.t-online.de/astro/ser/SER%20Doc%20V3b.pdf](http://www.grischa-hahn.homepage.t-online.de/astro/ser/SER%20Doc%20V3b.pdf)

###Project Goals
Some simple goals that hopefully get accross my intentions for this project.
* Produce a lightweight and intuative video player application for .ser files.
* ~~Avoid feature creep and over complication in order to keep SER Player distinct from [PIPP](https://sites.google.com/site/astropipp/).  New features may be added if they are not duplicating PIPP's functionality.~~
* SER Player must remain cross-platform and support at least Windows, OS X and Linux.

##Building SER Player for Windows
This section has some basic notes on building the application for Windows.  These notes assume a Windows PC is being used to build the application.
* Install Qt v5.4.1 or later and Qt Creator
* Open Qt project file **ser-player\ser_player.pro** with Qt Creator
* Select **Release** instead of **Debug**.
* Do **Build Menu->Run qmake** to run qmake.
* Do **Build Menu->Build All** to build the application.
* The directory **ser-player\bin\** will contain the application executable and many DLL files required for it to run.

With the application built we next need to build an installer.  This is done using the [NSIS](http://nsis.sourceforge.net/Download) Windows installer creation program.
* Download and install NSIS 2.46.
* Start NSIS and click on **Compile NSI scripts**.
* This will open a **MakeNSISW** window.
* Open file **ser-player\ser_player_nsis\ser_player.nsi** with the **MakeNSISW** window.  The simplest way is to drag the file and drop it in the window.
* This will run the NSIS installer creator and generate the installer file **ser-player\ser_player_nsis\ser_player_install.exe**

This installer file can now be distributed to other Windows users.


##Building SER Player for OS X
This section has some basic notes on building the application for OS X.  These notes assume a Mac running OS X is being used to build the application.
* Install Qt v5.4.1 or later and Qt Creator
* Open Qt project file **ser-player\ser_player.pro** with Qt Creator
* Select **Release** instead of **Debug**.
* Do **Build Menu->Run qmake** to run qmake.
* Do **Build Menu->Build All** to build the application.
* This Apple disk image (.dmg) file **ser_player\bin\SER-Player.dmg** will created.

This disk image file contains the SER-Player application and can be distributed to other Mac users.


##Building SER Player for Linux
###Building using the Terminal
This section has some basic notes on building the application for Linux using the terminal.  This has only been tested with variations of Debian linux but should work with other linux distributions.  The qtbase5-dev package must be installed.

* Terminal $ **cd ser-player**
* Terminal $ **qmake CONFIG+=release** (Or **qmake DEFINES+=DISABLE_NEW_VERSION_CHECK CONFIG+=release**)
* Terminal $ **make**

The directory **ser-player/bin/** will now contain the application executable only.

* Terminal $ **cd linux_appimage/**
* Terminal $ **./make_appimage.sh**

The directory **ser-player/linux_appimage/** will now contain the the SER Player Appimage.


###Building Using Qt Creator
This section has some basic notes on building the application for Linux using Qt Creator.  This has only been tested with variations of Debian linux but should work with other linux distributions.
* Install Qt v5.4.1 or later and Qt Creator
* Open Qt project file **ser-player/ser_player/ser_player.pro** with Qt Creator
* Select **Release** instead of **Debug**.
* Do **Build Menu->Run qmake** to run qmake.
* Do **Build Menu->Build All** to build the application.

The directory **ser-player/bin/** will contain the application executable only.

* Terminal $ **cd ser-player/linux_appimage/**
* Terminal $ **./make_appimage.sh**

The directory **ser-player/linux_appimage/** will now contain the the SER Player Appimage.
