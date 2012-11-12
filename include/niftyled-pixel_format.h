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
 * @file niftyled-pixel_format.h
 * @brief LedPixelFormat API to organize various colorspaces
 */

/**
 * @addtogroup setup
 * @{
 * @defgroup pixel_format LedPixelFormat
 * @brief pixel-format related functionality (libbabl wrapper)
 * @{
 */

#ifndef _LED_PIXEL_FORMAT_H
#define _LED_PIXEL_FORMAT_H

#include <stdbool.h>
#include <stdlib.h>
#include <babl/babl.h>


/** wrapper type to define the pixel-format of a frame */
typedef Babl                    LedPixelFormat;

/** wrapper type to define a babl-fish that converts one bufferful from one colorspace to another */
typedef Babl                    LedPixelFormatConverter;



void                            led_pixel_format_new();
void                            led_pixel_format_destroy();
bool                            led_pixel_format_is_big_endian();
const char                     *led_pixel_format_to_string(LedPixelFormat * f);
LedPixelFormat                 *led_pixel_format_from_string(const char *s);
bool                            led_pixel_format_is_equal(LedPixelFormat * a, LedPixelFormat * b);
const char                     *led_pixel_format_colorspace_to_string(LedPixelFormat * f);
void                            led_pixel_format_convert(LedPixelFormatConverter * converter, void *src, void *dst, size_t n);

size_t                          led_pixel_format_get_bytes_per_pixel(LedPixelFormat * f);
size_t                          led_pixel_format_get_buffer_size(LedPixelFormat * f, int n);
LedPixelFormatConverter        *led_pixel_format_get_converter(LedPixelFormat * src, LedPixelFormat * dst);
size_t                          led_pixel_format_get_n_components(LedPixelFormat * f);
const char                     *led_pixel_format_get_component_type(LedPixelFormat * f, unsigned int component);
size_t                          led_pixel_format_get_component_offset(LedPixelFormat * f, size_t n);
size_t                          led_pixel_format_get_n_formats();
LedPixelFormat                 *led_pixel_format_get_nth(size_t n);

/*size_t                          led_pixel_format_get_pixel_offset(LedPixelFormat *f, size_t n);*/

#endif /* _LED_PIXEL_FORMAT_H */

/**
 * @}
 * @}
 */
