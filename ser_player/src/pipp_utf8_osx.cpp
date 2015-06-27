// ---------------------------------------------------------------------
// Copyright (C) 2015 Chris Garry
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
// ---------------------------------------------------------------------


#include <cstdio>
#include <cwchar>
#include <cstring>
#include <fcntl.h>
#include <stdlib.h>
#include <copyfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "pipp_utf8.h"


// ------------------------------------------
// fopen_utf8
// ------------------------------------------
FILE *fopen_utf8(
    const std::string &filename,
    const std::string &mode)
{
    // Open file, Linux supports UTF-8 filenames by default
    FILE *ret = fopen(filename.c_str(), mode.c_str());
    return ret;
}


// ------------------------------------------
// rename_utf8
// ------------------------------------------
int rename_utf8( 
   const std::string &oldname,
   const std::string &newname)
{
    // Do the rename, Linux supports UTF-8 filenames by default
    int ret = rename (oldname.c_str(), newname.c_str());
    return ret;
}


// ------------------------------------------
// remove_utf8
// ------------------------------------------
int remove_utf8( 
   const std::string &path)
{
    // Remove file
    int ret = remove(path.c_str());
    return ret;
}


// ------------------------------------------
// copy_file_utf8
// ------------------------------------------
bool copy_file_utf8(
   const std::string &oldname,
   const std::string &newname)
{
    copyfile_flags_t flags = COPYFILE_MOVE | COPYFILE_DATA;
    int ret = copyfile(oldname.c_str(), newname.c_str(), NULL, flags);
    return ret >= 0;

}


// ------------------------------------------
// is_dirctory_utf8
// ------------------------------------------
bool is_directory_utf8(
    const std::string &path)
{
    struct stat myStat;
    if ((stat(path.c_str(), &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR)) {
        // path exists and is a directory
        return true;
    }

    return false;
}


// ------------------------------------------
// create_directories_utf8
// ------------------------------------------
bool create_directories_utf8(
    const std::string &path)
{
    int ret = mkdir(path.c_str(), 0777);
    return ret == 0;
}


// ------------------------------------------
// pipp_get_filename_from_filepath
// ------------------------------------------
const char *pipp_get_filename_from_filepath(
    const std::string &path)
{
    // Find last occurance of '/'
    const char *name = strrchr(path.c_str(), '/');

    if (name == NULL) {
        // '/' was not found, look for '\\' instead
        name = strrchr(path.c_str(), '\\');
    }

    if (name == NULL) {
        // Neither '/' or '\\' was found - there is no path to remove
        name = path.c_str();
    }

    // Filepath has been removed, move past character
    name++;

    return name;
}

