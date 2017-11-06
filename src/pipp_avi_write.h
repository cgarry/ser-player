#ifndef PIPP_ODML_WRITE_H
#define PIPP_ODML_WRITE_H

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "pipp_video_write.h"
#include "pipp_buffer.h"

#define DEBUGF //printf


// AVI Flags
#define AVIF_HASINDEX 0x00000010
#define AVIF_MUSTUSEINDEX 0x00000020
#define AVIF_ISINTERLEAVED 0x00000100
#define AVIF_TRUSTCKTYPE 0x00000800
#define AVIF_WASCAPTUREFILE 0x00010000
#define AVIF_COPYRIGHTED 0x00020000

// AVI Index Flags
#define AVIIF_LIST      0x00000001  // Points to LIST not a CHUNK
#define AVIIF_KEYFRAME  0x00000010  // This frame is a key frame
#define AVIIF_FIRSTPART 0x00000020
#define AVIIF_LASTPART  0x00000040
#define AVIIF_NOTIME    0x00000100  // This frame takes no time 

// FCC codes
#define FCC_RIFF 0x46464952
#define FCC_AVI  0x20495641
#define FCC_AVIX 0x58495641
#define FCC_LIST 0x5453494C
#define FCC_JUNK 0x4b4e554a
#define FCC_AVIX 0x58495641
#define FCC_hdrl 0x6C726468
#define FCC_avih 0x68697661
#define FCC_vids 0x73646976
#define FCC_7Fxx 0x78784637
#define FCC_idx1 0x31786469
#define FCC_movi 0x69766f6d
#define FCC_00db 0x62643030
#define FCC_odml 0x6C6D646F
#define FCC_indx 0x78646E69
#define FCC_strl 0x6C727473
#define FCC_strh 0x68727473
#define FCC_strf 0x66727473
#define FCC_strn 0x6E727473
#define FCC_dmlh 0x686C6D64
#define FCC_ix00 0x30307869

#define FCC_BI_RGB 0x00000000
#define FCC_RGB    0x20424752
#define FCC_Y800   0x30303859
#define FCC_DIB    0x20424944
#define FCC_UYVY   0x59565955
#define FCC_YUY2   0x32595559
#define FCC_YUYV   0x56595559
#define FCC_I420   0x30323449
#define FCC_IYUV   0x56555949
#define FCC_YV12   0x32315659
#define FCC_BY8    0x20385942

#define NUMBER_SUPERINDEX_ENTRIES 62


class c_pipp_avi_write: public c_pipp_video_write {
    // ------------------------------------------
    // Protected definitions
    // ------------------------------------------
    protected:
        // Member structures
        struct s_list_header {
            union {
                char chr[4];
                uint32_t u32;
            } list;
            uint32_t size;
            union {
                char chr[4];
                uint32_t u32;
            } four_cc;
        };

        static_assert (sizeof(s_list_header) == 4 + 4 + 4, "Unexpected size for structure s_list_header");

        struct s_chunk_header {
            union {
                char chr[4];
                uint32_t u32;
            } four_cc;
            uint32_t size;
        };

        static_assert (sizeof(s_chunk_header) == 4 + 4, "Unexpected size for structure s_chunk_header");

        struct s_main_avi_header {
            uint32_t micro_sec_per_frame; // frame display rate (or 0)
            uint32_t max_bytes_per_sec; // max. transfer rate
            uint32_t padding_granularity; // pad to multiples of this size;
            uint32_t flags; // the ever-present flags
            uint32_t total_frames; // # frames in file
            uint32_t initial_frames;
            uint32_t streams;
            uint32_t suggested_buffer_size;
            int32_t width;
            int32_t height;
            uint32_t reserved[4];
        };

        static_assert (sizeof(s_main_avi_header) == 10 * 4 + 4 * 4, "Unexpected size for structure s_main_avi_header");

        struct s_avi_stream_header {
            union {
                char chr[4];
                uint32_t u32;
            } type;
            union {
                char chr[4];
                uint32_t u32;
            } handler;
            uint32_t flags;
            uint16_t priority;
            uint16_t language;
            uint32_t initial_frames;
            uint32_t scale;
            uint32_t rate; /* dwRate / dwScale == samples/second */
            uint32_t start;
            uint32_t length; /* In units above... */
            uint32_t suggested_buffer_size;
            uint32_t quality;
            uint32_t sample_size;
            struct {
                short int left;
                short int top;
                short int right;
                short int bottom;
            }  frame;
        };

        static_assert (sizeof(s_avi_stream_header) == 3 * 4 + 2 * 2 + 8 * 4 + 4 * 2, "Unexpected size for structure s_avi_stream_header");

        struct s_bitmap_info_header {
            uint32_t size;
            int32_t width;
            int32_t height;
            uint16_t planes;
            uint16_t bit_count;
            union {
                char chr[4];
                uint32_t u32;
            } compression;
            uint32_t size_image;
            uint32_t x_pels_per_meter;
            uint32_t y_pels_per_meter;
            uint32_t clr_used;
            uint32_t clr_important;
        };

        static_assert (sizeof(s_bitmap_info_header) == 3 * 4 + 2 * 2 + 6 * 4, "Unexpected size for structure s_bitmap_info_header");

        struct s_avi_old_index_entry {
            union {
                char chr[4];
                uint32_t u32;
            } chunk_id;
            uint32_t flags;
            uint32_t offset;
            uint32_t size;
        };

        static_assert (sizeof(s_avi_old_index_entry) == 4 * 4, "Unexpected size for structure s_avi_old_index_entry");

        struct s_avi_superindex_header {
            int16_t longs_per_entry;
            int8_t index_sub_type;
            int8_t index_type;
            int32_t entries_in_use;
            union {
                char chr[4];
                uint32_t u32;
            } chunk_id;
            uint32_t reserved[3];
        };

        static_assert (sizeof(s_avi_superindex_header) == 2 + 2 * 1 + 2 * 4 + 3 * 4, "Unexpected size for structure s_avi_superindex_header");

        struct s_avi_superindex_entry {
            int64_t offset;
            int32_t size;
            int32_t duration;
        };

        static_assert (sizeof(s_avi_superindex_entry) == 1 * 8 + 2 * 4, "Unexpected size for structure s_avi_superindex_entry");

        struct s_extended_avi_header {
            int32_t total_frames;
            int32_t reserved[61];  // Hardly documented!
        };

        static_assert (sizeof(s_extended_avi_header) == 1 * 4 + 61 * 4, "Unexpected size for structure s_extended_avi_header");

        struct s_avi_stdindex_header {
            int16_t longs_per_entry;
            int8_t index_sub_type;
            int8_t index_type;
            int32_t entries_in_use;
            union {
                char chr[4];
                uint32_t u32;
            } chunk_id;
            int32_t base_offset[2];
            int32_t reserved3;
        };

        static_assert (sizeof(s_avi_stdindex_header) == 1 * 2 + 2 * 1 + 5 * 4, "Unexpected size for structure s_avi_stdindex_header");

        struct s_avi_stdindex_entry {
            int32_t offset;
            int32_t size;
        };

        static_assert (sizeof(s_avi_stdindex_entry) == 2 * 4, "Unexpected size for structure s_avi_stdindex_entry");

        // Member variables
        std::unique_ptr<char[]> mp_filename;
        std::unique_ptr<char[]> mp_extension;
        FILE *mp_avi_file;
        bool m_open;
        int32_t m_split_count;
        int32_t m_old_avi_format;
        int32_t m_width;
        int32_t m_height;
        int32_t m_frame_size;
        bool m_colour;
        int32_t m_max_frames_in_first_riff;
        int32_t m_max_frames_in_other_riffs;
        bool m_write_colour_table;
        int32_t m_total_frame_count;
        int32_t m_current_frame_count;
        int32_t m_riff_count;
        int32_t m_bytes_per_pixel;
        int64_t m_last_frame_pos;
        int64_t m_riff_start_position;
        bool m_file_write_error;
        bool m_big_endian_processor;

        c_pipp_buffer m_temp_buffer;

        // Various list and chunk structures in main file
        s_list_header m_avi_riff_header;
        s_list_header m_avix_riff_header;
        s_list_header m_hdrl_list_header;
        s_chunk_header m_avih_chunk_header;
        s_main_avi_header m_main_avih_header;
        s_list_header m_strl_list_header;
        s_chunk_header m_strh_chunk_header;
        s_avi_stream_header m_vids_stream_header;
        s_chunk_header m_strf_chunk_header;
        s_bitmap_info_header m_bitmap_info_header;
        s_chunk_header m_junk_chunk_header;

        // New odml stuff start
        s_chunk_header m_indx_chunk_header;
        s_avi_superindex_header m_avi_superindex_header;
        s_avi_superindex_entry m_avi_superindex_entries[NUMBER_SUPERINDEX_ENTRIES];
        s_list_header m_odml_list_header;
        s_chunk_header m_dmlh_chunk_header;
        s_extended_avi_header m_extended_avi_header;
        // New odml stuff end

        s_list_header m_movi_list_header;
        s_list_header m_movi_avix_list_header;
        s_chunk_header m_00db_chunk_header;
        s_chunk_header m_idx1_chunk_header;
        s_avi_old_index_entry m_avi_index_entry;

        s_chunk_header m_ix00_chunk_header;
        s_avi_stdindex_header m_avi_stdindex_header;
        s_avi_stdindex_entry m_avi_stdindex_entry;


    // ------------------------------------------
    // Public definitions
    // ------------------------------------------
    public:
    
        // ------------------------------------------
        // Constructor
        // ------------------------------------------
        c_pipp_avi_write();


        // ------------------------------------------
        // Destructor
        // ------------------------------------------
        virtual ~c_pipp_avi_write() {
        }


        // ------------------------------------------
        // Return the open state of the AVI file
        // ------------------------------------------
        bool get_open() {
            return m_open;
        }


        // ------------------------------------------
        // Create a new AVI file
        // ------------------------------------------
        bool create(
            const char *filename,
            int32_t  m_width,
            int32_t  m_height,
            bool  m_colour,
            int32_t  fps_rate,
            int32_t  fps_scale,
            int32_t  m_old_avi_format,
            int32_t  quality,
            void *extra_data = NULL);
            
            
        // ------------------------------------------
        // Write frame to AVI file
        // ------------------------------------------
        virtual bool write_frame(
            uint8_t *data,
            int32_t m_colour,
            uint32_t bpp,
            void *extra_data = NULL) = 0;
        

        // ------------------------------------------
        // Write header and close AVI file
        // ------------------------------------------
        bool close();


    protected:
        // ------------------------------------------
        // Write headers to file
        // ------------------------------------------
        void fwrite_error_check(
                const void *ptr,
                size_t size,
                size_t count,
                FILE *p_stream);


        // ------------------------------------------
        // Write headers to file
        // ------------------------------------------
        void write_headers();


        // ------------------------------------------
        // A new frame has been added
        // ------------------------------------------
        void frame_added();


        // ------------------------------------------
        // Write debug output
        // ------------------------------------------
        int32_t debug_headers();


        // ------------------------------------------
        // Set codec specific values
        // ------------------------------------------
        virtual int32_t set_codec_values() = 0;

    private:
        // ------------------------------------------
        // Finish the current RIFF
        // ------------------------------------------
        void finish_riff();
        void split_create();

        // ------------------------------------------
        // Write header and close AVI file
        // ------------------------------------------
        void split_close();

        template <typename T>
        static T swap_endianess(T data)
        {
            int32_t ret;
            uint8_t *p_read = (uint8_t *)&data;
            p_read += sizeof(data) - 1;
            uint8_t *p_write = (uint8_t *)&ret;
            for (size_t x = 0; x < sizeof(data); x++) {
                *p_write++ = *p_read--;
            }

            return ret;
        }

        // Change from little-endian to big-endian on big-endian systems
        static void swap_structure_endianess(s_list_header *p_list_header)
        {
            p_list_header->list.u32 = swap_endianess(p_list_header->list.u32);
            p_list_header->size = swap_endianess(p_list_header->size);
            p_list_header->four_cc.u32 = swap_endianess(p_list_header->four_cc.u32);
        }

        static void swap_structure_endianess(s_chunk_header *p_chunk_header)
        {
            p_chunk_header->four_cc.u32 = swap_endianess(p_chunk_header->four_cc.u32);
            p_chunk_header->size = swap_endianess(p_chunk_header->size);
        }

        static void swap_structure_endianess(s_main_avi_header *p_main_avi_header)
        {
            p_main_avi_header->micro_sec_per_frame = swap_endianess(p_main_avi_header->micro_sec_per_frame);
            p_main_avi_header->max_bytes_per_sec = swap_endianess(p_main_avi_header->max_bytes_per_sec);
            p_main_avi_header->padding_granularity = swap_endianess(p_main_avi_header->padding_granularity);
            p_main_avi_header->flags = swap_endianess(p_main_avi_header->flags);
            p_main_avi_header->total_frames = swap_endianess(p_main_avi_header->total_frames);
            p_main_avi_header->initial_frames = swap_endianess(p_main_avi_header->initial_frames);
            p_main_avi_header->streams = swap_endianess(p_main_avi_header->streams);
            p_main_avi_header->suggested_buffer_size = swap_endianess(p_main_avi_header->suggested_buffer_size);
            p_main_avi_header->width = swap_endianess(p_main_avi_header->width);
            p_main_avi_header->height = swap_endianess(p_main_avi_header->height);
        }

        static void swap_structure_endianess(s_avi_stream_header *p_avi_stream_header)
        {
            p_avi_stream_header->type.u32 = swap_endianess(p_avi_stream_header->type.u32);
            p_avi_stream_header->handler.u32 = swap_endianess(p_avi_stream_header->handler.u32);
            p_avi_stream_header->flags = swap_endianess(p_avi_stream_header->flags);
            p_avi_stream_header->priority = swap_endianess(p_avi_stream_header->priority);
            p_avi_stream_header->language = swap_endianess(p_avi_stream_header->language);
            p_avi_stream_header->initial_frames = swap_endianess(p_avi_stream_header->initial_frames);
            p_avi_stream_header->scale = swap_endianess(p_avi_stream_header->scale);
            p_avi_stream_header->rate = swap_endianess(p_avi_stream_header->rate);
            p_avi_stream_header->start = swap_endianess(p_avi_stream_header->start);
            p_avi_stream_header->length = swap_endianess(p_avi_stream_header->length);
            p_avi_stream_header->suggested_buffer_size = swap_endianess(p_avi_stream_header->suggested_buffer_size);
            p_avi_stream_header->quality = swap_endianess(p_avi_stream_header->quality);
            p_avi_stream_header->sample_size = swap_endianess(p_avi_stream_header->sample_size);
            p_avi_stream_header->frame.left = swap_endianess(p_avi_stream_header->frame.left);
            p_avi_stream_header->frame.top = swap_endianess(p_avi_stream_header->frame.top);
            p_avi_stream_header->frame.right = swap_endianess(p_avi_stream_header->frame.right);
            p_avi_stream_header->frame.bottom = swap_endianess(p_avi_stream_header->frame.bottom);
        }

        static void swap_structure_endianess(s_bitmap_info_header *p_bitmap_info_header)
        {
            p_bitmap_info_header->size = swap_endianess(p_bitmap_info_header->size);
            p_bitmap_info_header->width = swap_endianess(p_bitmap_info_header->width);
            p_bitmap_info_header->height = swap_endianess(p_bitmap_info_header->height);
            p_bitmap_info_header->planes = swap_endianess(p_bitmap_info_header->planes);
            p_bitmap_info_header->bit_count = swap_endianess(p_bitmap_info_header->bit_count);
            p_bitmap_info_header->compression.u32 = swap_endianess(p_bitmap_info_header->compression.u32);
            p_bitmap_info_header->size_image = swap_endianess(p_bitmap_info_header->size_image);
            p_bitmap_info_header->x_pels_per_meter = swap_endianess(p_bitmap_info_header->x_pels_per_meter);
            p_bitmap_info_header->y_pels_per_meter = swap_endianess(p_bitmap_info_header->y_pels_per_meter);
            p_bitmap_info_header->clr_used = swap_endianess(p_bitmap_info_header->clr_used);
            p_bitmap_info_header->clr_important = swap_endianess(p_bitmap_info_header->clr_important);
        }

        static void swap_structure_endianess(s_avi_old_index_entry *p_avi_old_index_entry)
        {
            p_avi_old_index_entry->chunk_id.u32 = swap_endianess(p_avi_old_index_entry->chunk_id.u32);
            p_avi_old_index_entry->flags = swap_endianess(p_avi_old_index_entry->flags);
            p_avi_old_index_entry->offset = swap_endianess(p_avi_old_index_entry->offset);
            p_avi_old_index_entry->size = swap_endianess(p_avi_old_index_entry->size);
        }

        static void swap_structure_endianess(s_avi_superindex_header *p_avi_superindex_header)
        {
            p_avi_superindex_header->longs_per_entry = swap_endianess(p_avi_superindex_header->longs_per_entry);
            p_avi_superindex_header->index_sub_type = swap_endianess(p_avi_superindex_header->index_sub_type);
            p_avi_superindex_header->index_type = swap_endianess(p_avi_superindex_header->index_type);
            p_avi_superindex_header->entries_in_use = swap_endianess(p_avi_superindex_header->entries_in_use);
            p_avi_superindex_header->chunk_id.u32 = swap_endianess(p_avi_superindex_header->chunk_id.u32);
        }

        static void swap_structure_endianess(s_avi_superindex_entry *p_avi_superindex_entry)
        {
            p_avi_superindex_entry->offset = swap_endianess(p_avi_superindex_entry->offset);
            p_avi_superindex_entry->size = swap_endianess(p_avi_superindex_entry->size);
            p_avi_superindex_entry->duration = swap_endianess(p_avi_superindex_entry->duration);
        }

        static void swap_structure_endianess(s_extended_avi_header *p_extended_avi_header)
        {
            p_extended_avi_header->total_frames = swap_endianess(p_extended_avi_header->total_frames);
        }

        static void swap_structure_endianess(s_avi_stdindex_header *p_avi_stdindex_header)
        {
            p_avi_stdindex_header->longs_per_entry = swap_endianess(p_avi_stdindex_header->longs_per_entry);
            p_avi_stdindex_header->index_sub_type = swap_endianess(p_avi_stdindex_header->index_sub_type);
            p_avi_stdindex_header->index_type = swap_endianess(p_avi_stdindex_header->index_type);
            p_avi_stdindex_header->entries_in_use = swap_endianess(p_avi_stdindex_header->entries_in_use);
            p_avi_stdindex_header->chunk_id.u32 = swap_endianess(p_avi_stdindex_header->chunk_id.u32);
            p_avi_stdindex_header->base_offset[0] = swap_endianess(p_avi_stdindex_header->base_offset[0]);
            p_avi_stdindex_header->base_offset[1] = swap_endianess(p_avi_stdindex_header->base_offset[1]);
        }

        static void swap_structure_endianess(s_avi_stdindex_entry *p_avi_stdindex_entry)
        {
            p_avi_stdindex_entry->offset = swap_endianess(p_avi_stdindex_entry->offset);
            p_avi_stdindex_entry->size = swap_endianess(p_avi_stdindex_entry->size);
        }
};

    
#endif  // PIPP_ODML_WRITE_H
