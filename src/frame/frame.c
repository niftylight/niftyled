/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2014 Daniel Hiepler <daniel@niftylight.de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/**
 * @file frame.c
 */

/**
 * @addtogroup frame
 * @{
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#endif

#include "niftyled-frame.h"



/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))


/** one frame */
struct _LedFrame
{
        /** width of frame in pixels */
        LedFrameCord width;
        /** height of frame in pixels */
        LedFrameCord height;
        /* format of this frame */
        LedPixelFormat *format;
        /** size of buffer in bytes */
        size_t bufsize;
        /** data buffer */
        void *buffer;
        /** 
         * true if framebuffer is big-endian, false otherwise
         * (set after buffer-contents change because flag might 
         * be changed in frame_convert_endianness()) 
         */
        bool is_big_endian;
        /** buffer free-func */
        void (*freebuf) (void *buf);
};






/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/


/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/


/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/**
 * create new frame(buffer)
 *
 * @param width width of frame in pixels
 * @param height height of frame in pixels
 * @param format the pixelformat of the source buffer
 *    - check @ref pixel_format for supported formats
 * @result NFT_SUCCESS or NFT_FAILURE
 */
LedFrame *led_frame_new(LedFrameCord width, LedFrameCord height,
                        LedPixelFormat * format)
{
        if(!format)
                NFT_LOG_NULL(NULL);

        /* initialize conversion instance */
        led_pixel_format_new();


        /* size of pixelbuffer */
        size_t bufsize =
                led_pixel_format_get_buffer_size(format, width * height);

        /* allocate buffer */
        void *framebuffer;
        if(!(framebuffer = malloc(bufsize)))
        {
                NFT_LOG_PERROR("malloc()");
                return NULL;
        }

        /* clear buffer */
        memset(framebuffer, 0, bufsize);

        /* allocate descriptor */
        LedFrame *n;
        if(!(n = calloc(1, sizeof(LedFrame))))
        {
                NFT_LOG_PERROR("calloc()");
                free(framebuffer);
                return NULL;
        }


        /* fill descriptor with data */
        n->format = format;
        n->freebuf = free;
        n->width = width;
        n->height = height;
        n->buffer = framebuffer;
        n->bufsize = bufsize;


        /* voilà */
        return n;
}


/**
 * free resources of one Frame
 *
 * @param f an LedFrame
 */
void led_frame_destroy(LedFrame * f)
{
        if(!f)
                return;

        if(f->freebuf)
        {
                f->freebuf(f->buffer);
        }

        free(f);

        /* deinitialize conversion instance */
        led_pixel_format_destroy();
}


/**
 * get dimensions of frame in pixels
 *
 * @param[in] f an LedFrame
 * @param[out] width pointer to width of frame in pixels or NULL
 * @param[out] height pointer to height of frame in pixels or NULL
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_frame_get_dim(LedFrame * f, LedFrameCord * width,
                            LedFrameCord * height)
{
        if(!f)
                NFT_LOG_NULL(NFT_FAILURE);

        if(width)
                *width = f->width;
        if(height)
                *height = f->height;

        return NFT_SUCCESS;
}


/**
 * get height of frame in pixels
 *
 * @param f an LedFrame
 * @result height of frame or 0 upon error
 */
LedFrameCord led_frame_get_height(LedFrame * f)
{
        if(!f)
                NFT_LOG_NULL(0);

        return f->height;
}


/**
 * get buffer of this frame
 *
 * @param f LedFrame descriptor
 * @result pointer to raw buffer or NULL upon error
 */
void *led_frame_get_buffer(LedFrame * f)
{
        if(!f)
                NFT_LOG_NULL(NULL);

        return f->buffer;
}


/**
 * free current buffer and set custom memory as buffer
 * @note !! Must be at least led_frame_get_buffersize() large
 * 
 * @param f LedFrame
 * @param buffer the new buffer
 * @param buffersize size of buffer in bytes
 * @param freebuf function to free buffer or NULL
 */
NftResult led_frame_set_buffer(LedFrame * f, void *buffer, size_t buffersize,
                               void (*freebuf) (void *))
{
        if(!f)
                NFT_LOG_NULL(NFT_FAILURE);

        /** buffer large enough? */
        if(f->bufsize > buffersize)
        {
                NFT_LOG(L_ERROR,
                        "Frame buffersize (%d) differs from new buffersize (%d)",
                        f->bufsize, buffersize);
                return NFT_FAILURE;
        }

        /** free previous buffer? */
        if(f->buffer && f->freebuf)
                f->freebuf(f->buffer);

        f->buffer = buffer;
        f->freebuf = freebuf;

        return NFT_SUCCESS;
}


/**
 * get size of buffer in bytes
 *
 * @param f LedFrame descriptor
 * @result size of led_frame_get_buffer() in bytes
 */
size_t led_frame_get_buffersize(LedFrame * f)
{
        return led_pixel_format_get_buffer_size(f->format,
                                                f->width * f->height);
}


/**
 * get pixel-format of a frame
 *
 * @param f LedFrame
 * @result pixelformat of frame or NULL upon error
 */
LedPixelFormat *led_frame_get_format(LedFrame * f)
{
        if(!f)
                NFT_LOG_NULL(NULL);

        return f->format;
}


/**
 * log a whole frame at L_NOISY loglevel (for debugging)
 * @todo make it work with large framebuffers
 *
 * @param f a LedFrame
 */
void led_frame_print_buffer(LedFrame * f)
{
/* loglevel to use for printout */
#define P_LOGLEVEL L_NOISY
/* total buffer */
#define P_BUFSIZE 4096
/* buffersize to use for one number */
#define P_NUMSIZE 16
/* maximum amount of columns to print */
#define P_PRINT_COL 16
/* maximum amount of rows to print */
#define P_PRINT_ROW 32

        if(!f)
                NFT_LOG_NULL();

        if(nft_log_level_is_noisier_than(P_LOGLEVEL, nft_log_level_get()))
                return;


        char num[P_NUMSIZE];
        char buf[P_BUFSIZE];
        buf[0] = '\0';


        int h, hm = MIN(P_PRINT_ROW, f->height);
        for(h = 0; h < hm; h++)
        {
                strncat(buf, "\n ", P_BUFSIZE - strlen(buf));

                if(hm < f->height && h == P_PRINT_ROW - 1)
                        strncat(buf, "...\n ", P_BUFSIZE - strlen(buf));

                int w, wm = MIN(P_PRINT_COL, f->width);
                for(w = 0; w < wm; w++)
                {
                        /* print ... if wm = 64 */
                        if(wm < f->width && w == P_PRINT_COL - 1)
                                strncat(buf, "... ", P_BUFSIZE - strlen(buf));

                        // strncat(tmp, "0x", P_BUFSIZE-strlen(tmp));
                        unsigned int c;
                        for(c = 0;
                            c < led_pixel_format_get_n_components(f->format);
                            c++)
                        {

                                /* calculate amount of components to this pixel 
                                 */
                                size_t n =
                                        (f->width * h +
                                         w) *
                                        led_pixel_format_get_n_components
                                        (f->format);
                                /* byte-offset of this component */
                                size_t offset =
                                        led_pixel_format_get_component_offset
                                        (f->format, n + c);

                                if(babl_format_get_type(f->format, c) ==
                                   babl_type("u8"))
                                {
                                        unsigned char *val;
                                        val = (unsigned char *) f->buffer +
                                                offset;
#ifdef WIN32
                                        snprintf(num, P_NUMSIZE - 1, "%.2X",
                                                 *val);
#else
                                        snprintf(num, P_NUMSIZE - 1, "%.2hhX",
                                                 *val);
#endif
                                }
                                else if(babl_format_get_type(f->format, c) ==
                                        babl_type("u16"))
                                {
                                        unsigned short *val;
                                        val = (unsigned short *) f->buffer +
                                                offset;
                                        snprintf(num, P_NUMSIZE - 1, "%.4hX",
                                                 *val);
                                }
                                else if(babl_format_get_type(f->format, c) ==
                                        babl_type("u32"))
                                {
                                        unsigned int *val;
                                        val = (unsigned int *) f->buffer +
                                                offset;
                                        snprintf(num, P_NUMSIZE - 1, "%.8X",
                                                 *val);
                                }
                                else if(babl_format_get_type(f->format, c) ==
                                        babl_type("double"))
                                {
                                        double *val;
                                        val = (double *) f->buffer + offset;
                                        snprintf(num, P_NUMSIZE - 1, "%f",
                                                 *val);
                                }
                                else if(babl_format_get_type(f->format, c) ==
                                        babl_type("float"))
                                {
                                        float *val;
                                        val = (float *) f->buffer + offset;
                                        snprintf(num, P_NUMSIZE - 1, "%f",
                                                 *val);
                                }
                                else
                                {
                                        NFT_LOG(L_ERROR, "Unhandled type");
                                        return;
                                }

                                strncat(buf, num, P_BUFSIZE - strlen(buf));
                        }
                        strncat(buf, " ", P_BUFSIZE - strlen(buf));
                }

        }


        NFT_LOG(P_LOGLEVEL, "%s", buf);

}



/**
 * print debug-info of a frame
 *
 * @param f a LedFrame
 * @param l minimum current loglevel so tile gets printed
 */
void led_frame_print(LedFrame * f, NftLoglevel l)
{
        if(!f)
                NFT_LOG_NULL();

        NFT_LOG(l,
                "Frame 0x%p\n"
                "\tDimensions: %dx%d\n"
                "\tFormat: %s (%s)\n"
                "\tBufsize: %d bytes",
                f,
                f->width, f->height,
                f->format ? led_pixel_format_to_string(f->format) : "none",
                f->is_big_endian ? "big endian" : "little endian",
                f->bufsize);
}


/**
 * set endianness-flag of a frame
 *
 * @param f a LedFrame
 * @param is_big_endian true if buffer of frame is big-endian ordered, false otherwise
 */
void led_frame_set_big_endian(LedFrame * f, bool is_big_endian)
{
        if(!f)
                NFT_LOG_NULL();

        f->is_big_endian = is_big_endian;
}




/**
 * get endianness-flag of a frame
 *
 * @param f an LedFrame
 * @result is_big_endian true if buffer of frame is big-endian ordered, false otherwise
 */
bool led_frame_get_big_endian(LedFrame * f)
{
        if(!f)
                /** prefer wrong result over crash (we'll fail later) */
                NFT_LOG_NULL(false);

        return f->is_big_endian;
}




/**
 * convert frame-buffer from little- to big-endian or vice-versa
 *
 * @param f an LedFrame
 */
void led_frame_convert_endianness(LedFrame * f)
{
        switch (led_pixel_format_get_bytes_per_pixel(f->format))
        {
#if HAVE_BYTESWAP_H
                case 2:
                {
                        short *t = f->buffer;
                        size_t n;
                        for(n = 0;
                            n < led_pixel_format_get_buffer_size(f->format,
                                                                 f->width *
                                                                 f->height) /
                            2; n++)
                        {
                                short a = t[n];
                                t[n] = bswap_16(a);
                        }
                        break;
                }

                case 4:
                {
                        int *t = f->buffer;
                        size_t n;
                        for(n = 0;
                            n < led_pixel_format_get_buffer_size(f->format,
                                                                 f->width *
                                                                 f->height) /
                            4; n++)
                        {
                                int a = t[n];

                                t[n] = bswap_32(a);
                        }
                        break;
                }

#else
                case 4:
                {
                        int *t = f->buffer;
                        size_t n;
                        for(n = 0;
                            n < led_pixel_format_get_buffer_size(f->format,
                                                                 f->width *
                                                                 f->height) /
                            4; n++)
                        {
                                int a = t[n];

                                t[n] = ((a & 0xff) << 24) +
                                        ((a & 0xff00) << 8) +
                                        ((a & 0xff0000) >> 8) +
                                        ((a & 0xff000000) >> 24);
                        }
                        break;
                }
#endif

                default:
                {
                        NFT_LOG(L_WARNING,
                                "Change endianness of %d bytes-per-pixel not supported.",
                                led_pixel_format_get_bytes_per_pixel
                                (f->format));
                        NFT_TODO();
                        return;
                }
        }

}


/**
 * @}
 */
