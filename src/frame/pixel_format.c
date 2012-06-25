/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2010 Daniel Hiepler <daniel@niftylight.de>
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
 * @file pixel_format.c
 */

/**
 * @addtogroup pixel_format
 * @{
 */

#include <stdlib.h>
#include <stdarg.h>
#include <babl/babl.h>
#include <niftylog.h>
#include "niftyled-pixel_format.h"
#include "niftyled-frame.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif





/**
 * initialize babl instance
 */
void led_pixel_format_new()
{
        babl_init();
}


/**
 * deinitialize babl instance
 */
void led_pixel_format_destroy()
{
        babl_exit();
}


/**
 * return data-type used by this format as string
 *
 * @param f LedPixelFormat
 * @param component the component-number starting from 0 (e.g. 1 to get "G" type in "RGB")
 * @result printable name of format (e.g. "u8") or NULL upon error
 */
const char *led_pixel_format_get_component_type(LedPixelFormat *f, int component)
{
        if(component >= led_pixel_format_get_n_components(f))
        {
                NFT_LOG(L_ERROR, "Format-type of component %d requested. But only have %d components in format %s",
                        component, led_pixel_format_get_n_components(f), babl_get_name(f));
                return NULL;
        }
        
        return babl_get_name(babl_format_get_type(f, component));
}


/**
 * return printable name string of LedPixelFormat
 *
 * @param f LedPixelFormat
 * @result string or NULL
 */
const char *led_pixel_format_to_string(LedPixelFormat *f)
{
        if(!f)
                NFT_LOG_NULL(NULL);

        return babl_get_name(f);
}


/**
 * return printable name of colorspace (e.g. "RGB")
 *
 * @param f LedPixelFormat
 * @result string or NULL
 */
const char *led_pixel_format_colorspace_to_string(LedPixelFormat *f)
{
        if(!f)
                NFT_LOG_NULL(NULL);

        /* copy format string */
        static char colorspace[64];
        strncpy(colorspace, led_pixel_format_to_string(f), sizeof(colorspace));

        /* find space */
        int i;
        for(i = 0; i < sizeof(colorspace); i++)
        {
                if(colorspace[i] == ' ')
                {
                        colorspace[i] = '\0';
                        break;
                }
        }

        return colorspace;
}


/**
 * get LedPixelFormat object from name string
 *
 * @param s format name as seen on http://gegl.org/babl/
 * @result LedPixelFormat object or NULL upon error
 */
LedPixelFormat *led_pixel_format_from_string(const char *s)
{       
        if(!s)
                NFT_LOG_NULL(NULL);


        /* register some extra formats to libbabl */
        if(strcmp(s, "BGR u8") == 0)
        {
                return babl_format_new(babl_model("RGB"), babl_type ("u8"), babl_component ("B"), babl_component ("G"), babl_component ("R"), NULL);
        }
        if(strcmp(s, "BGR u16") == 0)
        {
                return babl_format_new(babl_model("RGB"), babl_type ("u16"), babl_component ("B"), babl_component ("G"), babl_component ("R"), NULL);
        }
        if(strcmp(s, "BGR u32") == 0)
        {
                return babl_format_new(babl_model("RGB"), babl_type ("u32"), babl_component ("B"), babl_component ("G"), babl_component ("R"), NULL);
        }
        else if(strcmp(s, "ARGB u8") == 0)
        {       
                return babl_format_new(babl_model("RGBA"), babl_type ("u8"), babl_component ("A"), babl_component ("R"), babl_component ("G"), babl_component ("B"), NULL);
        }
        else if(strcmp(s, "ARGB u16") == 0)
        {       
                return babl_format_new(babl_model("RGBA"), babl_type ("u16"), babl_component ("A"), babl_component ("R"), babl_component ("G"), babl_component ("B"), NULL);
        }
        else if(strcmp(s, "ARGB u32") == 0)
        {       
                return babl_format_new(babl_model("RGBA"), babl_type ("u32"), babl_component ("A"), babl_component ("R"), babl_component ("G"), babl_component ("B"), NULL);
        }
        else if(strcmp(s, "ABGR u8") == 0)
        {
                return babl_format_new(babl_model("RGBA"), babl_type ("u8"), babl_component ("A"), babl_component ("B"), babl_component ("G"), babl_component ("R"), NULL);
        }
        else if(strcmp(s, "ABGR u16") == 0)
        {
                return babl_format_new(babl_model("RGBA"), babl_type ("u16"), babl_component ("A"), babl_component ("B"), babl_component ("G"), babl_component ("R"), NULL);
        }
        else if(strcmp(s, "ABGR u32") == 0)
        {
                return babl_format_new(babl_model("RGBA"), babl_type ("u32"), babl_component ("A"), babl_component ("B"), babl_component ("G"), babl_component ("R"), NULL);
        }
        
        return babl_format(s);
}


/**
 * check whether two pixel-formats are the same
 *
 * @param a one LedPixelFormat
 * @param b another LedPixelFormat
 * @result TRUE if formats match, FALSE otherwise
 */
bool led_pixel_format_is_equal(LedPixelFormat *a, LedPixelFormat *b)
{
        if(a == b)
                return TRUE;

        return FALSE;
}


/**
 * get amount of bytes per pixel of a format
 *
 * @param f LedPixelFormat descriptor
 * @result amount of bytes per pixel
 */
size_t led_pixel_format_get_bytes_per_pixel(LedPixelFormat *f)
{
        return (size_t) babl_format_get_bytes_per_pixel(f);
}


/**
 * get amount of components per pixel of a format
 *
 * @param f LedPixelFormat descriptor
 * @result amount of components per pixel
 */
size_t led_pixel_format_get_n_components(LedPixelFormat *f)
{
        return (size_t) babl_format_get_n_components(f);
}


/** 
 * calculate raw buffer size 
 * @param f LedPixelFormat descriptor
 * @param n amount of pixels
 * @result amount of bytes needed to store n pixels
 */
size_t led_pixel_format_get_buffer_size(LedPixelFormat *f, LedFrameCord n)
{
        return (size_t) (int) n * babl_format_get_bytes_per_pixel(f);
}


/**
 * get format converter (babl fish)
 *
 * @param src source LedPixelFormat
 * @param dst destination LedPixelFormat
 * @result a LedPixelFormatConverter or NULL upon error
 */
LedPixelFormatConverter *led_pixel_format_get_converter(LedPixelFormat *src, LedPixelFormat *dst)
{                          
        return babl_fish(src, dst);
}


/**
 * apply conversion to buffer
 *
 * @param converter the previously allocated converter
 * @param src source buffer
 * @param dst destination buffer
 * @param n amount of pixels to convert
 *
 */
void led_pixel_format_convert(LedPixelFormatConverter *converter,
                              void *src, void *dst, size_t n)
{        
        NFT_LOG(L_NOISY, "Converting %d pixels", n);
        babl_process(converter, src, dst, (long) n);                             
}


/**
 * check for endianess
 *
 * @result TRUE if we run on a big-endian or universal system, FALSE otherwise
 */
bool led_pixel_format_is_big_endian()
{
#ifdef __BIG_ENDIAN__
        return TRUE;
#else
        return FALSE;
#endif
}


/**
 * return byte-offset to pixel n relative to the first pixel in a buffer
 * e.g. to seek to the 4th pixel in an RGB u8 buffer,
 * n = 4 will result in 4*3 = 12
 *
 * @param f LedPixelFormat of buf
 * @param n amount of pixels to seek
 * @result byte offset
 */
//~ size_t led_pixel_format_get_pixel_offset(LedPixelFormat *f, size_t n)
//~ {
        //~ if(!f)
                //~ NFT_LOG_NULL(0);

        //~ return n*led_pixel_format_get_bytes_per_pixel(f);
//~ }


/**
 * return byte-offset to component n relative to the first component in a buffer
 *
 * @param f LedPixelFormat of buf
 * @param n amount of components to seek
 * @result byte-offset
 */
size_t led_pixel_format_get_component_offset(LedPixelFormat *f, size_t n)
{
        if(!f)
                NFT_LOG_NULL(0);

        /* calculate bytes-per-component */
        int bpc = led_pixel_format_get_bytes_per_pixel(f)/led_pixel_format_get_n_components(f);
        
        return bpc*n;
}





/**
 * @}
 */
