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


#include "pipp_utf8.h"

extern "C" {
    #include "png.h"
}

#include <cstdint>
#include <cstring>
#include <memory>

// Support as many libpng versions as required
#if PNG_LIBPNG_VER_MAJOR==2
    #error "libpng 2.x.x is not supported"
#elif PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR==7
    #error "libpng 1.7.x is not supported"
#elif PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR==6

// ------------------------------------------
// Save PNG image (colour)
// ------------------------------------------
static int32_t save_colour_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample)
{
    int32_t ret = -1;
    png_image image; // The control structure used by libpng

    // Initialize the 'png_image' structure.
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;
    image.format = PNG_FORMAT_BGR;
    if (bytes_per_sample == 2) {
        image.format |= PNG_FORMAT_FLAG_LINEAR;
    }

    FILE *p_png_file = fopen_utf8(filename, "wb");

    if (p_png_file != nullptr) {
        png_image_write_to_stdio(
            &image,
            p_png_file,
            0,  // convert_to_8bit
            (png_bytep)(p_image_data),
            image.width * -3,  // row_stride
            NULL);  // colormap
        ret = 0;
        fclose(p_png_file);
    }

    return ret;
}


// ------------------------------------------
// Save PNG image (mono B, G or R)
// ------------------------------------------
static int32_t save_mono_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample)
{
    int32_t ret = -1;
    png_image image; // The control structure used by libpng

    // Initialize the 'png_image' structure.
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;

    if (bytes_per_sample == 1) {
        image.format = PNG_FORMAT_GRAY;  // 8-bit data
    } else {
        image.format = PNG_FORMAT_LINEAR_Y;  // 16-bit data
    }

    FILE *p_png_file = fopen_utf8(filename, "wb");

    if (p_png_file != nullptr) {
        png_image_write_to_stdio(
            &image,
            p_png_file,
            0,  // convert_to_8bit
            (png_bytep)p_image_data,
            image.width * -1,  // row_stride
            NULL);  // colormap
        ret = 0;
        fclose(p_png_file);
    }

    return ret;
}

#elif PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR==5
    #error "libpng 1.5.x is not supported"
#elif PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR==4
    #error "libpng 1.4.x is not supported"
#elif PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR==3
    #error "libpng 1.3.x is not supported"
#elif PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR==2

// ------------------------------------------
// Save PNG image (colour)
// ------------------------------------------
static int32_t save_colour_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample)
{
    int32_t ret = -1;


    return ret;
}

// ------------------------------------------
// Save PNG image (mono B, G or R)
// ------------------------------------------
static int32_t save_mono_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample)
{
    int32_t ret = -1;


    return ret;
}

#else
    #error "Unsuported libpng version"
#endif


int32_t save_png_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample,
    bool is_colour)
{
    int ret = -1;
    if (is_colour) {
        // Create colour PNG file
        ret = save_colour_file(filename,
                               p_image_data,
                               width,
                               height,
                               bytes_per_sample);
    } else {
        // Create monochrome PNG file
        ret = save_mono_file(filename,
                             p_image_data,
                             width,
                             height,
                             bytes_per_sample);
    }

    return ret;
}

