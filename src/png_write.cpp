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
// Save PNG image
// ------------------------------------------
int32_t save_png_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample,
    bool is_colour)
{
    int32_t ret = -1;
    png_image image; // The control structure used by libpng

    // Initialize the 'png_image' structure.
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;

    int row_stride;
    if (is_colour) {
        row_stride = image.width * -3;
        image.format = PNG_FORMAT_BGR;
        if (bytes_per_sample == 2) {
            image.format |= PNG_FORMAT_FLAG_LINEAR;
        }
    } else {
        row_stride = image.width * -1;
        if (bytes_per_sample == 1) {
            image.format = PNG_FORMAT_GRAY;  // 8-bit data
        } else {
            image.format = PNG_FORMAT_LINEAR_Y;  // 16-bit data
        }
    }

    FILE *p_png_file = fopen_utf8(filename, "wb");

    if (p_png_file != nullptr) {
        png_image_write_to_stdio(
            &image,
            p_png_file,
            0,  // convert_to_8bit
            (png_bytep)(p_image_data),
            row_stride,
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
// Save PNG image
// ------------------------------------------
int32_t save_png_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample,
    bool is_colour)
{
    int32_t ret = -1;
    png_structp png_ptr;
    png_infop info_ptr;

    FILE *p_png_file = fopen_utf8(filename, "wb");
    if (p_png_file == nullptr) {
        return ret;
    }

    /* Create and initialize the png_struct with the desired error handler
     * functions.  If you want to use the default stderr and longjump method,
     * you can supply NULL for the last three parameters.  We also check that
     * the library version is compatible with the one used at compile time,
     * in case we are using dynamically linked libraries.  REQUIRED.
     */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == nullptr) {
        fclose(p_png_file);
        return ret;
    }

    /* Allocate/initialize the image information data.  REQUIRED */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
       fclose(p_png_file);
       png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
       return ret;
    }


    /* Set error handling.  REQUIRED if you aren't supplying your own
     * error handling functions in the png_create_write_struct() call.
     */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
       /* If we get here, we had a problem writing the file */
       fclose(p_png_file);
       png_destroy_write_struct(&png_ptr, &info_ptr);
       return ret;
    }

    /* Set up the output control if you are using standard C streams */
    png_init_io(png_ptr, p_png_file);

    /* Set the image information here.  Width and height are up to 2^31,
     * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
     * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
     * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
     * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
     * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
     * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
     */
    if (is_colour) {
        png_set_IHDR(png_ptr, info_ptr, width, height, bytes_per_sample*8, PNG_COLOR_TYPE_RGB,
           PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    } else {
        png_set_IHDR(png_ptr, info_ptr, width, height, bytes_per_sample*8, PNG_COLOR_TYPE_GRAY,
           PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    }

    /* Write the file header information.  REQUIRED */
    png_write_info(png_ptr, info_ptr);

    /* Flip BGR pixels to RGB */
    png_set_bgr(png_ptr);

    /* Swap bytes of 16-bit files to most significant byte first */
    png_set_swap(png_ptr);

    // Write out image
    uint8_t *row_pointers[height];
    int samples_per_pixel;
    if (is_colour) {
        samples_per_pixel = 3;
    } else {
        samples_per_pixel = 1;
    }

    for (uint32_t i = 0; i < height; i++) {
        // Yuck!  Casting away the const from the pointer as this version of libpng is sloppyly written!
        row_pointers[i] = (uint8_t *)p_image_data + (height - 1 - i) * width * bytes_per_sample * samples_per_pixel;
    }

    png_write_image(png_ptr, row_pointers);

    /* Clean up after the write, and free any memory allocated */
    png_destroy_write_struct(&png_ptr, &info_ptr);

    /* Close the file */
    fclose(p_png_file);

    ret = 0;
    return ret;
}

#else
    #error "Unsuported libpng version"
#endif

