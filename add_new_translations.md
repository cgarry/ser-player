# Notes on adding a new translation

These are just my notes to remind me of the steps required to add a new language translation.

1. Edit ser-player.pro and add new line to 'TRANSLATIONS = ..' section.7
2. Start Qt Creator application and open ser-player.pro file.
3. Select Tools->External->Linguist->Update Translations (lupdate).  This will create new 'ser_player_XX.ts' file in the \ser-player\translations folder.
4. Select Tools->External->Linguist->Release Translations (lrelease).  This will create new 'ser_player_XX.qm' file in the \ser-player\translations folder.
5. In Qt Creator add new .qm file to Resources->images.qrc->/res->translations
6. Add new .ts and .qm files to git.
7. Once project is rebuilt new language option should appear in app Help->Language menu.
8. Send .ts file to translator for translation.

