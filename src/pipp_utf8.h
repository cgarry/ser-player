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


#ifndef PIPP_UTF8_H
#define PIPP_UTF8_H

#include <cstdio>
#include <string>

FILE *fopen_utf8(
    const std::string &filename,
	const std::string &mode);


int rename_utf8( 
   const std::string &oldname, 
   const std::string &newname);


int remove_utf8( 
   const std::string &path);


bool copy_file_utf8(
   const std::string &oldname, 
   const std::string &newname);


bool is_directory_utf8(
    const std::string &path);

bool create_directories_utf8(
    const std::string &path);
    
const char *pipp_get_filename_from_filepath(
    const std::string &path);


// 64-bit fseek for various platforms
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>        // define or not BSD macro
#endif

#ifdef __linux__
#define fseek64 fseeko64  // Linux
#define ftell64 ftello64  // Linux
#elif defined (__APPLE__) && defined(__MACH__)
#define fseek64 fseeko  // OS X
#define ftell64 ftello  // OS X
#elif defined(BSD)
#define fseek64 fseeko  // DragonFly BSD, FreeBSD, OpenBSD, NetBSD
#define ftell64 ftello  // DragonFly BSD, FreeBSD, OpenBSD, NetBSD
#elif defined (__FreeBSD_kernel__) && defined (__GLIBC__)
#define fseek64 fseeko64  // kFreeBSD
#define ftell64 ftello64  // kFreeBSD
#elif defined (__gnu_hurd__)
#define fseek64 fseeko64  // GNU/Hurd
#define ftell64 ftello64  // GNU/Hurd
#else
#define fseek64 _fseeki64  // Windows
#define ftell64 _ftelli64  // Windows
#endif


#endif  // PIPP_UTF8_H
