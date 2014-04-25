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
 * @file led.c
 *
 */

/**
 * @addtogroup led
 * @{
 */

#include "niftyled-led.h"
#include "niftyled-frame.h"
#include "_led.h"




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
 * set position of a LED inside a pixel-frame for mapping
 *
 * @param[in] l @ref Led descriptor
 * @param[in] x new X coordinate of LED
 * @param[in] y new Y coordinate of LED		 
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_set_pos(Led * l, LedFrameCord x, LedFrameCord y)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        l->x = x;
        l->y = y;

        return NFT_SUCCESS;
}


/**
 * get position of a LED inside a pixel-frame for mapping
 *
 * @param[in] l @ref Led descriptor
 * @param[out] x pointer to X coordinate of LED or NULL
 * @param[out] y pointer to Y coordinate of LED or NULL
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_get_pos(Led * l, LedFrameCord * x, LedFrameCord * y)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        if(x)
                *x = l->x;
        if(y)
                *y = l->y;

        return NFT_SUCCESS;
}


/**
 * set pixel component (e.g. Red, Green, Blue, Cyan, ...) of a LED for mapping
 * this defines the LEDs color
 *
 * @param l @ref Led descriptor
 * @param component The component-number according to the definition of the pixel-frame this LED belongs to
 */
NftResult led_set_component(Led * l, LedFrameComponent component)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        l->component = component;

        return NFT_SUCCESS;
}


/**
 * get component number of a LED
 *
 * @param l @ref Led descriptor
 * @result The component-number according to the definition of the pixel-frame this LED belongs to
 */
LedFrameComponent led_get_component(Led * l)
{
        if(!l)
                NFT_LOG_NULL(0);

        return l->component;
}


/**
 * set driver-hardware gain of this LED
 *
 * @param l @ref Led descriptor
 * @param gain Gain setting of this LED (only has effect if hardware-plugin supports it)
 */
NftResult led_set_gain(Led * l, LedGain gain)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        l->gain = gain;

        return NFT_SUCCESS;
}


/**
 * get driver-hardware gain of this LED
 *
 * @param l @ref Led descriptor
 * @result current gain setting of this LED
 */
LedGain led_get_gain(Led * l)
{
        if(!l)
                NFT_LOG_NULL(0);

        return l->gain;
}


/**
 * get private userdata previously set by led_set_privdata()
 *
 * @param l Led descriptor
 * @result private userdata
 */
void *led_get_privdata(Led * l)
{
        if(!l)
                NFT_LOG_NULL(NULL);

        return l->privdata;
}


/**
 * associate private userdata pointer with LED
 *
 * @param l Led descriptor
 * @param privdata pointer to private userdata
 * @result NFT_SUCCESS or NFT_FAILURE upon error
 */
NftResult led_set_privdata(Led * l, void *privdata)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        l->privdata = privdata;

        return NFT_SUCCESS;
}


/**
 * copy a single LED of one chain to another
 *
 * @param dst - destination chain
 * @param src - source chain
 * @result NFT_SUCCESS or NFT_FAILURE
 * @note if you set a private pointer using led_set_privdata(), it will NOT be copied
 */
NftResult led_copy(Led * dst, Led * src)
{
        if(!dst || !src)
                NFT_LOG_NULL(NFT_FAILURE);

        /* save private pointer */
        void *ptr = dst->privdata;

        /* copy structure */
        memcpy(dst, src, sizeof(Led));

        /* copy private pointer back so it will be kept */
        dst->privdata = ptr;

        return NFT_SUCCESS;
}



/**
 * @}
 */
