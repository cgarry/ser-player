// ---------------------------------------------------------------------
// Copyright (C) 2017 Chris Garry
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

#ifndef TIFF_WRITE_H
#define TIFF_WRITE_H


#include <cstdint>
#include <memory>


extern int32_t save_tiff_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample,
    bool is_colour);

    
#endif  // TIFF_WRITE_H
