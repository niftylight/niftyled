/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2013 Daniel Hiepler <daniel@niftylight.de>
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
 * @file setup.c
 */


/**
 * @addtogroup setup
 * @{
 */

#include "niftyled-setup.h"
#include "_hardware.h"


/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))



/**
 * model to describe one setup of LED hardware
 */
struct _LedSetup
{
            /** first hardware in setup or NULL */
        LedHardware *firstHw;
};








/**
 * allocate new LedSetup model descriptor
 *
 * @result new empty LedSetup model descriptor or NULL
 */
LedSetup *led_setup_new()
{
        LedSetup *r;
        if(!(r = calloc(1, sizeof(LedSetup))))
        {
                NFT_LOG_PERROR("calloc");
                return NULL;
        }

        return r;
}


/**
 * free all resources allocated by a LedSetup object
 *
 * @param s valid LedSetup
 */
void led_setup_destroy(LedSetup * s)
{
        if(!s)
                return;

        /* destroy all LedHardware objects */
        if(s->firstHw)
        {
                led_hardware_list_destroy(s->firstHw);
        }

        /* be really tidy :) */
        s->firstHw = NULL;

        /* free descriptor */
        free(s);
}


/**
 * set head of hardware list in this setup
 *
 * @param s valid LedSetup
 * @param h LedHardware to set as head in this setup
 */
void led_setup_set_hardware(LedSetup * s, LedHardware * h)
{
        if(!s)
                NFT_LOG_NULL();

        s->firstHw = h;

        if(h)
        {
                hardware_set_parent_setup(h, s);
        }
}


/**
 * get head of hardware list from this setup
 *
 * @param s valid LedSetup
 * @result head of LedHardware list of this setup
 */
LedHardware *led_setup_get_hardware(LedSetup * s)
{
        if(!s)
                NFT_LOG_NULL(NULL);

        return s->firstHw;
}


/**
 * get total dimensions of the current setup in pixels
 *
 * @param[in] s LedSetup descriptor
 * @param[out] total width of setup in pixels or NULL
 * @param[out] total height of setup in pixels or NULL		
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_setup_get_dim(LedSetup * s, LedFrameCord * width,
                            LedFrameCord * height)
{
        if(!s)
                NFT_LOG_NULL(NFT_FAILURE);

        /* start at 0 */
        if(width)
                *width = 0;
        if(height)
                *height = 0;

        /* no hardware registered? */
        if(!s->firstHw)
                return NFT_SUCCESS;

        /* walk all registered Hardware descriptors */
        LedHardware *h;
        for(h = s->firstHw; h; h = led_hardware_list_get_next(h))
        {
                /* walk all tiles registered to this hardware */
                LedTile *t;
                for(t = led_hardware_get_tile(h); t;
                    t = led_tile_list_get_next(t))
                {
                        /* get position of tile */
                        LedFrameCord x, y;
                        if(!led_tile_get_pos(t, &x, &y))
                                return NFT_FAILURE;

                        /* get dimensions of tile */
                        LedFrameCord w, h;
                        if(!led_tile_get_transformed_dim(t, &w, &h))
                                return NFT_FAILURE;

                        /* if this tile is wider/higher than the current
                         * dimensions, take as new width/height */
                        if(width)
                                *width = MAX(*width, w + x);
                        if(height)
                                *height = MAX(*height, h + y);
                }
        }

        NFT_LOG(L_NOISY, "%dx%d", *width, *height);

        return NFT_SUCCESS;
}




/**
 * @}
 */
