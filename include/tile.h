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
 * @file tile.h
 * @brief LedTile API to define arrangements of chains or child tiles
 */

/**      
 * @defgroup tile LedTile
 * @brief Model to describe a LED arrangement. 
 * - can contain a @ref LedChain or groups of LedTiles. 
 * - every Tile can have a certain position and a rotation relative to it's 
 *   parent or to 0,0
 * - rotation is defined by a rotation angle and the location of the pivot 
 *   (center of rotation)
 *
 * @{
 */

#ifndef _LED_TILE_H
#define _LED_TILE_H


/** name of the LedTile object for NftSettings */
#define LED_TILE_NAME "tile"


/** model of one LedTile */
typedef struct _LedTile LedTile;


#include "chain.h"
#include "hardware.h"


LedTile *       led_tile_new();
void            led_tile_destroy(LedTile *t);

LedTile *       led_tile_dup(LedTile *t);
LedCount        led_tile_to_chain(LedTile *t, LedChain *dst, LedCount offset);
void            led_tile_print(LedTile *t, NftLoglevel l);

NftResult	led_tile_set_x(LedTile *t, LedFrameCord x);
NftResult	led_tile_set_y(LedTile *t, LedFrameCord y);
NftResult	led_tile_set_rotation(LedTile *t, double angle);
NftResult       led_tile_set_pivot_x(LedTile *t, double x);
NftResult       led_tile_set_pivot_y(LedTile *t, double y);
NftResult       led_tile_set_chain(LedTile *t, LedChain *c);
NftResult       led_tile_set_privdata(LedTile *t, void *privdata);


LedFrameCord    led_tile_get_x(LedTile *t);
LedFrameCord 	led_tile_get_y(LedTile *t);
double 		led_tile_get_rotation(LedTile *t);
double          led_tile_get_pivot_x(LedTile *t);
double          led_tile_get_pivot_y(LedTile *t);
double          led_tile_get_transformed_pivot_x(LedTile *t);
double          led_tile_get_transformed_pivot_y(LedTile *t);
LedFrameCord    led_tile_get_transformed_width(LedTile *t);
LedFrameCord    led_tile_get_transformed_height(LedTile *t);
LedChain *      led_tile_get_chain(LedTile *t);
void *          led_tile_get_privdata(LedTile *t);
LedCount        led_tile_get_ledcount(LedTile *t);
LedFrameCord	led_tile_get_width(LedTile *t);
LedFrameCord	led_tile_get_height(LedTile *t);


NftResult       led_tile_append_sibling(LedTile *head, LedTile *sibling);
NftResult	led_tile_set_sibling(LedTile *t, LedTile *sibling);
LedTile *       led_tile_get_nth_sibling(LedTile *c, int n);
LedTile *       led_tile_get_prev_sibling(LedTile *t);
LedTile *       led_tile_get_next_sibling(LedTile *t);
int	        led_tile_get_sibling_count(LedTile *c);

NftResult 	led_tile_append_child(LedTile *t, LedTile *child);
LedTile *       led_tile_get_child(LedTile *t);

void            led_tile_list_destroy(LedTile *first);



#endif  /* _LED_TILE_H */

/**
 * @}
 */

