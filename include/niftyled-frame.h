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
 * @file niftyled-frame.h
 * @brief LedFrame API to organize raster pixmaps
 */

/**
 * @addtogroup setup 
 * @{ 
 * @defgroup frame LedFrame
 * @brief abstract handling of RAW pixel-data
 * @{
 */

#ifndef _LED_FRAME_H
#define _LED_FRAME_H

#include <niftylog.h>
#include "nifty-primitives.h"
#include "niftyled-pixel_format.h"


/** type used to address a LedFrameComponent */
#define LED_T_COMPONENT		short
/** type used for frame coordinates (x/y) */
#define LED_T_COORDINATE 	int


/** model of one pixelframe */
typedef struct _LedFrame        LedFrame;

/** type to define coordinates (x,y positions, width & height) */
typedef LED_T_COORDINATE        LedFrameCord;

/** type to define the channelnumber for one portion of a pixel */
typedef LED_T_COMPONENT         LedFrameComponent;





LedFrame                       *led_frame_new(LedFrameCord width, LedFrameCord height, LedPixelFormat * format);
void                            led_frame_destroy(LedFrame * f);
void                            led_frame_print(LedFrame * f, NftLoglevel l);
void                            led_frame_print_buffer(LedFrame * f);
void                            led_frame_convert_endianess(LedFrame * f);

void                            led_frame_set_big_endian(LedFrame * f, bool is_big_endian);
NftResult                       led_frame_set_buffer(LedFrame * f, void *buffer, size_t buffersize, void (*freebuf) (void *));

bool                            led_frame_get_big_endian(LedFrame * f);
LedFrameCord                    led_frame_get_width(LedFrame * f);
LedFrameCord                    led_frame_get_height(LedFrame * f);
LedPixelFormat                 *led_frame_get_format(LedFrame * f);
void                           *led_frame_get_buffer(LedFrame * f);
size_t                          led_frame_get_buffersize(LedFrame * f);




#endif /* _LED_FRAME_H */

/**
 * @}
 * @}
 */
