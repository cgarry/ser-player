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


#include <Windows.h>
#include <cstdio>
#include <cwchar>
#include <cstring>
#include <iostream>
#include "pipp_utf8.h"


using namespace std;


// ------------------------------------------
// fopen_utf8
// ------------------------------------------
FILE *fopen_utf8(
    const std::string &filename,
	const std::string &mode)
{
    // Convert filename from utf-8 to wchat_t
    int length = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, 0, 0);
    wchar_t *w_fname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, w_fname, length);

    // Convert mode from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, mode.c_str(), -1, 0, 0);
    wchar_t *w_mode = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, mode.c_str(), -1, w_mode, length);

    // Actually open file
    FILE *ret = _wfopen(w_fname, w_mode);

    // Delete string buffers
    delete [] w_fname;
    delete [] w_mode;

    return ret;
}


// ------------------------------------------
// rename_utf8
// ------------------------------------------
int rename_utf8( 
   const std::string &oldname, 
   const std::string &newname)
{
    int length;

    // Convert oldname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, oldname.c_str(), -1, 0, 0);
    wchar_t *w_oldname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, oldname.c_str(), -1, w_oldname, length);

    // Convert newname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, newname.c_str(), -1, 0, 0);
    wchar_t *w_newname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, newname.c_str(), -1, w_newname, length);

    // Do the rename
    int ret = _wrename(w_oldname, w_newname);

    // Delete string buffers
    delete [] w_oldname;
    delete [] w_newname;

    return ret;
}


// ------------------------------------------
// remove_utf8
// ------------------------------------------
int remove_utf8( 
   const std::string &path)
{
    int length;

    // Convert path from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, 0, 0);
    wchar_t *w_path = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, w_path, length);

    int ret = _wremove(w_path);

    delete [] w_path;

    return ret;
}


// ------------------------------------------
// copy_file_utf8
// ------------------------------------------
bool copy_file_utf8(
   const std::string &oldname, 
   const std::string &newname)
{
    int length;

    // Convert oldname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, oldname.c_str(), -1, 0, 0);
    wchar_t *w_oldname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, oldname.c_str(), -1, w_oldname, length);

    // Convert newname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, newname.c_str(), -1, 0, 0);
    wchar_t *w_newname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, newname.c_str(), -1, w_newname, length);

    int ret = CopyFileW(
        w_oldname,
        w_newname,
        false);

    // Delete string buffers
    delete [] w_oldname;
    delete [] w_newname;

    return ret != 0;
}


// ------------------------------------------
// is_dirctory_utf8
// ------------------------------------------
bool is_directory_utf8(
    const std::string &path)
{
    int length;

    // Convert path from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, 0, 0);
    wchar_t *w_path = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, w_path, length);

    DWORD ret = GetFileAttributesW(w_path);
    delete [] w_path;

    if (ret == INVALID_FILE_ATTRIBUTES) {
        // Nothing exists with this path
        return false;
    }

    if (ret & FILE_ATTRIBUTE_DIRECTORY) {
        // Path exists and it is a directory
        return true;
    }

    return false;// Path is not a directory
}


// ------------------------------------------
// create_directories_utf8
// ------------------------------------------
bool create_directories_utf8(
    const std::string &path)
{
    int length;

    // Convert path from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, 0, 0);
    wchar_t *w_path = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, w_path, length);

    int ret = CreateDirectoryW(
      w_path,
      NULL
    );

    delete [] w_path;

    return ret != 0;
}


// ------------------------------------------
// pipp_get_filename_from_filepath
// ------------------------------------------
const char *pipp_get_filename_from_filepath(
    const std::string &path)
{
    const char *p_name;

    // Find last occurance of '/'
    size_t last_fwd_slash = path.find_last_of('/');

    if (last_fwd_slash == std::string::npos) {
        // '/' was not found, look for '\\' instead
        last_fwd_slash = path.find_last_of('\\');
    }

    if (last_fwd_slash == std::string::npos) {
        // Neither '/' or '\\' was found - there is no path to remove
        p_name = path.c_str();
    } else {
        p_name = path.c_str() + last_fwd_slash + 1;
    }

    return p_name;
}
