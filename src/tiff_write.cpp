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


#include <cstdint>
#include <cstring>
#include <memory>
#include <cstdio>
#include <iostream>
#include "pipp_utf8.h"


using namespace std;


// Baseline tags
#define TAG_NEWSUBFILETYPE 254  // A general indication of the kind of data contained in this subfile.
#define TAG_SUBFILETYPE 255  // A general indication of the kind of data contained in this subfile.
#define TAG_IMAGEWIDTH 256  // The number of columns in the image, i.e., the number of pixels per row.
#define TAG_IMAGELENGTH 257  // The number of rows of pixels in the image.
#define TAG_BITSPERSAMPLE 258  // Number of bits per component.
#define TAG_COMPRESSION 259  // Compression scheme used on the image data.
#define TAG_PHOTOMETRICINTERPRETATION 262  // PhotometricInterpretation The color space of the image data.
#define TAG_THRESHHOLDING 263  // Threshholding For black and white TIFF files that represent shades of gray, the technique used to convert from gray to black and white pixels.
#define TAG_CELLWIDTH 264  // CellWidth The width of the dithering or halftoning matrix used to create a dithered or halftoned bilevel file.
#define TAG_CELLLENGTH 265  // CellLength The length of the dithering or halftoning matrix used to create a dithered or halftoned bilevel file.
#define TAG_FILLORDER 266  // FillOrder The logical order of bits within a byte.
#define TAG_IMAGEDESCRIPTION 270  // ImageDescription A string that describes the subject of the image.
#define TAG_MAKE 271  // Make The scanner manufacturer.
#define TAG_MODEL 272  // Model The scanner model name or number.
#define TAG_STRIPOFFSETS 273  // StripOffsets For each strip, the byte offset of that strip.
#define TAG_ORIENTATION 274  // Orientation The orientation of the image with respect to the rows and columns.
#define TAG_SAMPLESPERPIXEL 277  // SamplesPerPixel The number of components per pixel.
#define TAG_ROWSPERSTRIP 278  // RowsPerStrip The number of rows per strip.
#define TAG_STRIPBYTECOUNTS 279  // StripByteCounts For each strip, the number of bytes in the strip after compression.
#define TAG_MINSAMPLEVALUE 280  // MinSampleValue The minimum component value used.
#define TAG_MAXSAMPLEVALUE 281  // MaxSampleValue The maximum component value used.
#define TAG_XRESOLUTION 282  // XResolution The number of pixels per ResolutionUnit in the ImageWidth direction.
#define TAG_YRESOLUTION 283  // YResolution The number of pixels per ResolutionUnit in the ImageLength direction.
#define TAG_PLANARCONFIGURATION 284  // PlanarConfiguration How the components of each pixel are stored.
#define TAG_FREEOFFSETS 288  // FreeOffsets For each string of contiguous unused bytes in a TIFF file, the byte offset of the string.
#define TAG_FREEBYTECOUNTS 289  // FreeByteCounts For each string of contiguous unused bytes in a TIFF file, the number of bytes in the string.
#define TAG_GRAYRESPONSEUNIT 290  // GrayResponseUnit The precision of the information contained in the GrayResponseCurve.
#define TAG_GRAYRESPONSECURVE 291  // GrayResponseCurve For grayscale data, the optical density of each possible pixel value.
#define TAG_RESOLUTIONUNIT 296  // ResolutionUnit The unit of measurement for XResolution and YResolution.
#define TAG_SOFTWARE 305  // Software Name and version number of the software package(s) used to create the image.
#define TAG_DATETIME 306  // DateTime Date and time of image creation.
#define TAG_ARTIST 315  // Artist Person who created the image.
#define TAG_HOSTCOMPUTER 316  // HostComputer The computer and/or operating system in use at the time of image creation.
#define TAG_COLORMAP 320  // ColorMap A color map for palette color images.
#define TAG_EXTRASAMPLES 338  // ExtraSamples Description of extra components.
#define TAG_COPYRIGHT 33432  // Copyright Copyright notice.

// Extension tags
#define TAG_SAMPLEFORMAT 339 //  Specifies how to interpret each data sample in a pixel.


// Field Size values
#define FIELDSIZE_BYTE 1
#define FIELDSIZE_ASCII 2
#define FIELDSIZE_SHORT 3
#define FIELDSIZE_LONG 4
#define FIELDSIZE_RATIONAL 5
#define FIELDSIZE_SBYTE 6
#define FIELDSIZE_UNDEFINED 7
#define FIELDSIZE_SSHORT 8
#define FIELDSIZE_SLONG 9
#define FIELDSIZE_SRATIONAL 10
#define FIELDSIZE_FLOAT 11
#define FIELDSIZE_DOUBLE 12

// Compression type values
#define COMP_NONE 1
#define COMP_HUFFMANRLE 2
#define COMP_PACKBITS 32773
#define COMP_LZW 5
#define COMP_OLDJPEG 6
#define COMP_NEWJPEG 7
#define COMP_DEFALTE 8

// Photometric values
#define PHOTOMETRIC_MINISWHITE 0
#define PHOTOMETRIC_MINISBLACK 1
#define PHOTOMETRIC_RGB 2
#define PHOTOMETRIC_PALETTE 3
#define PHOTOMETRIC_MASK 4

// FillOrder values
#define FILLORDER_MSB2LSB 1
#define FILLORDER_LSB2MSB 2

// Orientation - Many readers ignore this
#define ORIENTATION_TOPLEFT 1
#define ORIENTATION_TOPRIGHT 2
#define ORIENTATION_BOTRIGHT 3
#define ORIENTATION_BOTLEFT 4
#define ORIENTATION_LEFTTOP 5
#define ORIENTATION_RIGHTTOP 6
#define ORIENTATION_RIGHTBOT 7
#define ORIENTATION_LEFTBOT 8

// Planarconfiguration values - Baseline readers can assume PLANARCONFIG_CONTIG
#define PLANARCONFIG_CONTIG 1
#define PLANARCONFIG_SEPARATE 2

// Resolution Units
#define RESUNIT_NONE 1
#define RESUNIT_INCH 2
#define RESUNIT_CENTIMETER 3


// ------------------------------------------
// IFD Structure
// ------------------------------------------
struct s_ifd {
    uint16_t tag;
    uint16_t type;
    uint32_t count;
    uint32_t value_offset;
};


struct s_tiff_header {
    uint16_t byte_order;
    uint16_t magic_42;
    uint32_t ifd_offset;
    uint16_t pad1;
    uint16_t ifd_count;
    s_ifd ifd[12];
    uint32_t end_of_ifd;
    uint16_t bit_per_sample[3];
    uint16_t pad2;
    uint32_t resolution1;
    uint32_t resolution2;
};


// ------------------------------------------
// Save TIFF image (colour)
// ------------------------------------------
static int32_t save_colour_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample)
{
    // Image attributes
    uint32_t data_size = width * height * bytes_per_sample * 3;
    
    // Buffer for TIFF header
    std::unique_ptr<s_tiff_header> p_tiff_header(new s_tiff_header);
    
    // Detect endianess of the processor
    bool big_endian_processor = (*(uint16_t *)"\0\xff" < 0x100);

    // Fill in TIFF header
    if (big_endian_processor) {
        p_tiff_header->byte_order = 0x4D4D;  // Big endian
    } else {
        p_tiff_header->byte_order = 0x4949;  // Little endian
    }

    p_tiff_header->magic_42 = 42;  // Magic number
    p_tiff_header->ifd_offset = (uint32_t)((uint8_t *)&p_tiff_header->ifd_count - (uint8_t *)p_tiff_header.get());
    p_tiff_header->pad1 = 0;
    p_tiff_header->ifd_count = 12;

    p_tiff_header->ifd[0].tag          = TAG_IMAGEWIDTH;
    p_tiff_header->ifd[0].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[0].count        = 1;
    p_tiff_header->ifd[0].value_offset = width;

    p_tiff_header->ifd[1].tag          = TAG_IMAGELENGTH;
    p_tiff_header->ifd[1].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[1].count        = 1;
    p_tiff_header->ifd[1].value_offset = height;

    p_tiff_header->ifd[2].tag          = TAG_BITSPERSAMPLE;
    p_tiff_header->ifd[2].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[2].count        = 3;
    p_tiff_header->ifd[2].value_offset = (uint32_t)((uint8_t *)&(p_tiff_header->bit_per_sample[0]) - (uint8_t *)p_tiff_header.get());

    p_tiff_header->ifd[3].tag          = TAG_COMPRESSION;
    p_tiff_header->ifd[3].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[3].count        = 1;
    p_tiff_header->ifd[3].value_offset = COMP_NONE;

    p_tiff_header->ifd[4].tag          = TAG_PHOTOMETRICINTERPRETATION;
    p_tiff_header->ifd[4].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[4].count        = 1;
    p_tiff_header->ifd[4].value_offset = PHOTOMETRIC_RGB;

    p_tiff_header->ifd[5].tag          = TAG_STRIPOFFSETS;
    p_tiff_header->ifd[5].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[5].count        = 1;
    p_tiff_header->ifd[5].value_offset = sizeof(s_tiff_header);

    p_tiff_header->ifd[6].tag          = TAG_SAMPLESPERPIXEL;
    p_tiff_header->ifd[6].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[6].count        = 1;
    p_tiff_header->ifd[6].value_offset = 3;  // R, G and B

    p_tiff_header->ifd[7].tag          = TAG_ROWSPERSTRIP;
    p_tiff_header->ifd[7].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[7].count        = 1;
    p_tiff_header->ifd[7].value_offset = height;

    p_tiff_header->ifd[8].tag          = TAG_STRIPBYTECOUNTS;
    p_tiff_header->ifd[8].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[8].count        = 1;
    p_tiff_header->ifd[8].value_offset = data_size;

    p_tiff_header->ifd[9].tag          = TAG_XRESOLUTION;
    p_tiff_header->ifd[9].type         = FIELDSIZE_RATIONAL;
    p_tiff_header->ifd[9].count        = 1;
    p_tiff_header->ifd[9].value_offset = (uint32_t)((uint8_t *)&p_tiff_header->resolution1 - (uint8_t *)p_tiff_header.get());

    p_tiff_header->ifd[10].tag          = TAG_YRESOLUTION;
    p_tiff_header->ifd[10].type         = FIELDSIZE_RATIONAL;
    p_tiff_header->ifd[10].count        = 1;
    p_tiff_header->ifd[10].value_offset = (uint32_t)((uint8_t *)&p_tiff_header->resolution1 - (uint8_t *)p_tiff_header.get());

    p_tiff_header->ifd[11].tag          = TAG_RESOLUTIONUNIT;
    p_tiff_header->ifd[11].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[11].count        = 1;
    p_tiff_header->ifd[11].value_offset = RESUNIT_INCH;

    p_tiff_header->end_of_ifd = 0;  // Indicate end of IFDs

    // Bits Per Sample data
    p_tiff_header->bit_per_sample[0] = 8 * bytes_per_sample;
    p_tiff_header->bit_per_sample[1] = p_tiff_header->bit_per_sample[0];
    p_tiff_header->bit_per_sample[2] = p_tiff_header->bit_per_sample[0];
    p_tiff_header->pad2 = 0;

    // Resolution
    p_tiff_header->resolution1 = 720000;
    p_tiff_header->resolution2 = 10000;
    
    // Buffer for image data
    std::unique_ptr<uint8_t[]> p_temp_buffer(new uint8_t[data_size]);

    // Generate image data to write
    if (bytes_per_sample == 1) {
        uint8_t *write_ptr = p_temp_buffer.get();
        const uint8_t *read_buffer = p_image_data;
        const uint8_t *read_ptr;

        for (int y = height-1; y >= 0; y--) {
            read_ptr = read_buffer + y * width * 3;
            for (uint32_t x = 0; x < width; x++) {
                *write_ptr++ = *(read_ptr + 2);
                *write_ptr++ = *(read_ptr + 1);
                *write_ptr++ = *read_ptr;
                read_ptr += 3;
            }
        }
    } else if (bytes_per_sample == 2) {
        uint16_t *write_ptr = (uint16_t *)p_temp_buffer.get();
        uint16_t *read_buffer = (uint16_t *)p_image_data;
        uint16_t *read_ptr;

        for (int y = height-1; y >= 0; y--) {
            read_ptr = read_buffer + y * width * 3;
            for (uint32_t x = 0; x < width; x++) {
                *write_ptr++ = *(read_ptr + 2);
                *write_ptr++ = *(read_ptr + 1);
                *write_ptr++ = *read_ptr;
                read_ptr += 3;
            }
        }
    }

    // Open file for writing - Using the good old C way because it is faster
    FILE *rgb_fp;
    rgb_fp = fopen_utf8(filename, "wb");
    if (!rgb_fp) {
        cerr << "Error: Unable to open file for writing: '";
        cerr.write(filename, strlen(filename));
        cerr << "'" << endl;
        exit(-1);
    }
    
    // Write Header and Image data to file
    fwrite(p_tiff_header.get(), 1, sizeof(s_tiff_header), rgb_fp);  // Write TIFF header to file
    fwrite(p_temp_buffer.get(), 1, data_size, rgb_fp);  // Write image data to file
    fclose(rgb_fp);  // Close file

    return 0;
}


// ------------------------------------------
// Save TIFF image (mono B, G or R)
// ------------------------------------------
static int32_t save_mono_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample)
{
    // Image attributes
    uint32_t data_size = width * height * bytes_per_sample;
    
    // Buffer for TIFF header
    std::unique_ptr<s_tiff_header> p_tiff_header(new s_tiff_header);

    // Detect endianess of the processor
    bool big_endian_processor = (*(uint16_t *)"\0\xff" < 0x100);
    
    // Fill in TIFF header
    if (big_endian_processor) {
        p_tiff_header->byte_order = 0x4D4D;  // Big endian
    } else {
        p_tiff_header->byte_order = 0x4949;  // Little endian
    }

    p_tiff_header->magic_42 = 42;  // Magic number
    p_tiff_header->ifd_offset = (uint32_t)((uint8_t *)&p_tiff_header->ifd_count - (uint8_t *)p_tiff_header.get());
    p_tiff_header->pad1 = 0;
    p_tiff_header->ifd_count = 12;

    p_tiff_header->ifd[0].tag          = TAG_IMAGEWIDTH;
    p_tiff_header->ifd[0].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[0].count        = 1;
    p_tiff_header->ifd[0].value_offset = width;

    p_tiff_header->ifd[1].tag          = TAG_IMAGELENGTH;
    p_tiff_header->ifd[1].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[1].count        = 1;
    p_tiff_header->ifd[1].value_offset = height;

    p_tiff_header->ifd[2].tag          = TAG_BITSPERSAMPLE;
    p_tiff_header->ifd[2].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[2].count        = 1;
    p_tiff_header->ifd[2].value_offset = 8 * bytes_per_sample;

    p_tiff_header->ifd[3].tag          = TAG_COMPRESSION;
    p_tiff_header->ifd[3].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[3].count        = 1;
    p_tiff_header->ifd[3].value_offset = COMP_NONE;

    p_tiff_header->ifd[4].tag          = TAG_PHOTOMETRICINTERPRETATION;
    p_tiff_header->ifd[4].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[4].count        = 1;
    p_tiff_header->ifd[4].value_offset = PHOTOMETRIC_MINISBLACK;

    p_tiff_header->ifd[5].tag          = TAG_STRIPOFFSETS;
    p_tiff_header->ifd[5].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[5].count        = 1;
    p_tiff_header->ifd[5].value_offset = sizeof(s_tiff_header);

    p_tiff_header->ifd[6].tag          = TAG_SAMPLESPERPIXEL;
    p_tiff_header->ifd[6].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[6].count        = 1;
    p_tiff_header->ifd[6].value_offset = 1;  // Mono

    p_tiff_header->ifd[7].tag          = TAG_ROWSPERSTRIP;
    p_tiff_header->ifd[7].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[7].count        = 1;
    p_tiff_header->ifd[7].value_offset = height;

    p_tiff_header->ifd[8].tag          = TAG_STRIPBYTECOUNTS;
    p_tiff_header->ifd[8].type         = FIELDSIZE_LONG;
    p_tiff_header->ifd[8].count        = 1;
    p_tiff_header->ifd[8].value_offset = data_size;

    p_tiff_header->ifd[9].tag          = TAG_XRESOLUTION;
    p_tiff_header->ifd[9].type         = FIELDSIZE_RATIONAL;
    p_tiff_header->ifd[9].count        = 1;
    p_tiff_header->ifd[9].value_offset = (uint32_t)((uint8_t *)&p_tiff_header->resolution1 - (uint8_t *)p_tiff_header.get());

    p_tiff_header->ifd[10].tag          = TAG_YRESOLUTION;
    p_tiff_header->ifd[10].type         = FIELDSIZE_RATIONAL;
    p_tiff_header->ifd[10].count        = 1;
    p_tiff_header->ifd[10].value_offset = (uint32_t)((uint8_t *)&p_tiff_header->resolution1 - (uint8_t *)p_tiff_header.get());

    p_tiff_header->ifd[11].tag          = TAG_RESOLUTIONUNIT;
    p_tiff_header->ifd[11].type         = FIELDSIZE_SHORT;
    p_tiff_header->ifd[11].count        = 1;
    p_tiff_header->ifd[11].value_offset = RESUNIT_INCH;

    p_tiff_header->end_of_ifd = 0;  // Indicate end of IFDs

    // Bits Per Sample data
    p_tiff_header->bit_per_sample[0] = 8 * bytes_per_sample;
    p_tiff_header->bit_per_sample[1] = p_tiff_header->bit_per_sample[0];
    p_tiff_header->bit_per_sample[2] = p_tiff_header->bit_per_sample[0];
    p_tiff_header->pad2 = 0;

    // Resolution
    p_tiff_header->resolution1 = 720000;
    p_tiff_header->resolution2 = 10000;

    // Buffer for image data
    std::unique_ptr<uint8_t[]> p_temp_buffer(new uint8_t[data_size]);
    
    // Generate image data to write
    if (bytes_per_sample == 1) {
        uint8_t *write_ptr = p_temp_buffer.get();
        const uint8_t *read_buffer = p_image_data;
        const uint8_t *read_ptr;

        for (int y = height-1; y >= 0; y--) {
            //read_ptr = read_buffer + y * width * 3 + colour;
            read_ptr = read_buffer + y * width;
            for (uint32_t x = 0; x < width; x++) {
                *write_ptr++ = *read_ptr;
                //read_ptr += 3;
                read_ptr++;
            }
        }
    } else if (bytes_per_sample == 2) {
        uint16_t *write_ptr = (uint16_t *)p_temp_buffer.get();
        uint16_t *read_buffer = (uint16_t *)p_image_data;
        uint16_t *read_ptr;

        for (int y = height-1; y >= 0; y--) {
            //read_ptr = read_buffer + y * width * 3 + colour;
            read_ptr = read_buffer + y * width;
            for (uint32_t x = 0; x < width; x++) {
                *write_ptr++ = *read_ptr;
                //read_ptr += 3;
                read_ptr++;
            }
        }
    }

    // Open file for writing - Using the good old C way because it is faster
    FILE *mono_fp;
    mono_fp = fopen_utf8(filename, "wb");
    if (!mono_fp) {
        cerr << "Error: Unable to open file for writing: '";
        cerr.write(filename, strlen(filename));
        cerr << "'" << endl;
        return -1;
    }
    
    // Write Header and Image data to file
    fwrite(p_tiff_header.get(), 1, sizeof(s_tiff_header), mono_fp);  // Write TIFF header to file
    fwrite(p_temp_buffer.get(), 1, data_size, mono_fp);  // Write image data to file
    fclose(mono_fp);  // Close file

    return 0;
}


int32_t save_tiff_file(
    const char *filename,
    const uint8_t *p_image_data,
    uint32_t width,
    uint32_t height,
    uint32_t bytes_per_sample,
    bool is_colour)
{
    int ret = -1;
    if (is_colour) {
        // Create colour TIFF file
        ret = save_colour_file(filename,
                               p_image_data,
                               width,
                               height,
                               bytes_per_sample);
    } else {
        // Create monochrome TIFF file
        ret = save_mono_file(filename,
                             p_image_data,
                             width,
                             height,
                             bytes_per_sample);
    }

    return ret;
}

