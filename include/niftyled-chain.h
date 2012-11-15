/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2011 Daniel Hiepler <daniel@niftylight.de>
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
 * @file niftyled-chain.h
 * @brief LedChain API to organize a serial chain of LEDs
 */

/**
 * @addtogroup setup
 * @{
 * @defgroup chain LedChain
 * @brief Model for chains of serially aligned LEDs
 * - can contain one or multiple LEDs of a certain @ref LedPixelFormat (e.g. RGB)
 * @{
 */

#ifndef _LED_CHAIN_H
#define _LED_CHAIN_H

#include <stdlib.h>
#include "nifty-primitives.h"
#include "niftyled-frame.h"
#include "niftyled-pixel_format.h"





/** type used for LedGain */
#define LED_T_GAIN short


/** model of one single LED */
typedef struct _Led             Led;

/** model of one chain of @ref Led's */
typedef struct _LedChain        LedChain;

/** type to count LEDs */
typedef long int                LedCount;

/** type to define the gain-setting of an LED-driver (0 = turned off, 65535 = full brightness) */
typedef LED_T_GAIN              LedGain;


/** minimum value for LedGain type (LED is turned off) */
#define LED_GAIN_MIN    (0)
/** maximum value for LedGain type (LED at full brightness) */
#define LED_GAIN_MAX    ((unsigned LED_T_GAIN)((1<<(sizeof(LedGain)*8))-1))


#include "niftyled-tile.h"
#include "niftyled-hardware.h"





/* LedChain API */
LedChain                       *led_chain_new(LedCount ledcount, const char *pixelformat);
void                            led_chain_destroy(LedChain * c);
LedChain                       *led_chain_dup(LedChain * c);
NftResult                       led_chain_map_from_frame(LedChain * c, LedFrame * f);

//~ LedCount                                            led_chain_fill_from_tile(LedChain * c, LedTile * t, LedCount offset);
NftResult                       led_chain_fill_from_frame(LedChain * c, LedFrame * f);
void                            led_chain_print(LedChain * f, NftLoglevel l);
LedCount                        led_chain_stride_map(LedChain * c, LedCount stride, LedCount offset);
LedCount                        led_chain_stride_unmap(LedChain * c, LedCount stride, LedCount offset);
bool                            led_chain_parent_is_hardware(LedChain * c);
bool                            led_chain_parent_is_tile(LedChain * c);

NftResult                       led_chain_set_greyscale(LedChain * c, LedCount pos, long long int value);
NftResult                       led_chain_set_ledcount(LedChain * c, LedCount ledcount);
NftResult                       led_chain_set_privdata(LedChain * c, void *privdata);

NftResult                       led_chain_get_greyscale(LedChain * c, LedCount pos, long long int *value);
LedCount                        led_chain_get_ledcount(LedChain * c);
void                           *led_chain_get_privdata(LedChain * c);
Led                            *led_chain_get_nth(LedChain * c, LedCount n);
LedPixelFormat                 *led_chain_get_format(LedChain * c);
void                           *led_chain_get_buffer(LedChain * chain);
size_t                          led_chain_get_buffer_size(LedChain * chain);
LedFrameCord                    led_chain_get_max_x(LedChain * chain);
LedFrameCord                    led_chain_get_max_y(LedChain * chain);
LedFrameComponent               led_chain_get_max_component(LedChain * chain);
LedGain                         led_chain_get_max_gain(LedChain * chain);
LedHardware                    *led_chain_get_parent_hardware(LedChain * chain);
LedTile                        *led_chain_get_parent_tile(LedChain * chain);


/* Led API */
LedFrameCord                    led_get_x(Led * l);
LedFrameCord                    led_get_y(Led * l);
LedFrameComponent               led_get_component(Led * l);
LedGain                         led_get_gain(Led * l);
void                           *led_get_privdata(Led * l);

NftResult                       led_set_x(Led * l, LedFrameCord x);
NftResult                       led_set_y(Led * l, LedFrameCord y);
NftResult                       led_set_component(Led * l, LedFrameComponent component);
NftResult                       led_set_gain(Led * l, LedGain gain);
NftResult                       led_set_privdata(Led * l, void *privdata);

NftResult                       led_copy(Led * dst, Led * src);


#endif /* _LED_CHAIN_H */

/**
 * @}
 * @}
 */
