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
 * @file chain.c
 *
 * @todo add LED radius & shape properties
 * @todo larger round/rectangular LEDs take mean-value of area from frame as value
 */

/**
 * @addtogroup chain
 * @{
 */

#include <math.h>
#include <stdint.h>
#include "niftyled-chain.h"
#include "niftyled-frame.h"



/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))


/** model of one single LED  */
struct _Led
{
        /** position of LED inside map */
        LedFrameCord x, y;
        /** component-number this LED has in a pixel
		    (red, green, blue, cyan, ...) For example, in a RGB system, a red
			LED would have component number 0, a green one has 1 and a blue one
			has 2 */
        LedFrameComponent component;
        /** 32 bit gain value of this LED - use this to define brightness 
		    for LED hardware that supports it. The hardware plugin has to 
		    scale the 32 bit value so it can be used by the hardware.
		    0 should be lowest brightness, UINT32_MAX should be maximum brightness */
        LedGain gain;
        /** private userdata */
        void *privdata;
};

/** model of one serial chain of LEDs */
struct _LedChain
{
        /** amount of LEDs in this chain */
        LedCount ledcount;
        /** Pixel format how LED-values are stored in this chain */
        LedPixelFormat *format;
        /** Pixel format for conversions when greyscale-values
            are written to chain (NULL for no conversion) */
        LedPixelFormat *src_format;
        /** our converter to do format conversions */
        LedPixelFormatConverter *converter;
        /** temporary frame in src_format, internally used for conversions if formats differ */
        LedFrame *tmpframe;
        /** array of leds holding "ledcount" Led descriptors */
        Led *leds;
        /** buffersize in bytes */
        size_t buffersize;
        /** buffer that holds LEDs' greyscale-values */
        void *ledbuffer;
                /** if this chain belongs to a tile, this contains the pointer of the tile */
        LedTile *parent_tile;
                /** if this chain belongs to a hardware, this will be set */
        LedHardware *parent_hw;
        /**
         * temporary mapping-buffer. holds one offset per led in chain.
         * Offset points to coresponding location in LedFrame of same LedPixelFormat
         */
        int *mapoffsets;
        /** private userdata */
        void *privdata;
};


/******************************************************************************
 **************************** STATIC FUNCTIONS ********************************
 ******************************************************************************/


/**************************** INTERNAL FUNCTIONS ******************************/

/**
 * set parent hardware of this chain
 *
 * @param c LedChain descriptor
 * @param h LedHardware descriptor of parent hardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult chain_set_parent_hardware(LedChain * c, LedHardware * h)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        c->parent_hw = h;

        return NFT_SUCCESS;
}

/**
 * set parent tile of this chain
 *
 * @param c LedChain descriptor
 * @param t LedTile descriptor of parent tile
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult chain_set_parent_tile(LedChain * c, LedTile * t)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        c->parent_tile = t;

        return NFT_SUCCESS;
}


/**
 * quit usage of LED-chain and free its resources
 *
 * @param c the chain which should be freed, the pointer will be invalid after this function returns
 */
void chain_destroy(LedChain * c)
{
        if(!c)
                return;

        NFT_LOG(L_DEBUG, "Destroying chain with %d LEDs", c->ledcount);


        /* unlink from parent tile */
        if(c->parent_tile)
        {
                /* clear chain in tile */
                led_tile_set_chain(c->parent_tile, NULL);
        }

        /* unregister from config context */
        // ~ led_settings_chain_unregister(c);


        /* free LED descriptor array */
        free(c->leds);

        /* free LED-buffer */
        free(c->ledbuffer);

        /* free mapbuffer */
        free(c->mapoffsets);

        /* free temporary frame */
        led_frame_destroy(c->tmpframe);

        /* deinitialize this conversion instance */
        led_pixel_format_destroy();

        /* free our very own space */
        free(c);
}


/**
 * internal function to change amount of ledcount of a chain (API wrapper contains some checks)
 *
 * @param c to change ledcount
 * @param ledcount new amount of LEDs in this chain
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult chain_set_ledcount(LedChain * c, LedCount ledcount)
{
        /* calc old and new bufsize */
        int components = led_pixel_format_get_n_components(c->format);
        size_t nbufsize = led_pixel_format_get_buffer_size(c->format,
                                                           ledcount /
                                                           components);
        size_t obufsize = led_pixel_format_get_buffer_size(c->format,
                                                           c->ledcount /
                                                           components);


        /* allocate new ledbuffer */
        void *newbuf;
        if(!(newbuf = malloc(nbufsize)))
        {
                NFT_LOG_PERROR("malloc");
                return NFT_FAILURE;
        }

        /** copy old ledbuffer into new buffer */
        memcpy(newbuf, c->ledbuffer, MIN(nbufsize, obufsize));


        /* allocate new mapping buffer */
        void **mapoffsets;
        if(!(mapoffsets = calloc(ledcount, sizeof(int))))
        {
                NFT_LOG_PERROR("malloc");
                return NFT_FAILURE;
        }

        /* copy old mapbuffer into new buffer */
        memcpy(mapoffsets, c->mapoffsets,
               MIN((ledcount * sizeof(int)), (c->ledcount * sizeof(int))));


        /** allocate buffer for LED-descriptors */
        Led *newleds;
        if(!(newleds = calloc(ledcount, sizeof(Led))))
        {
                NFT_LOG_PERROR("calloc");
                free(newbuf);
                return NFT_FAILURE;
        }

        /** copy old LEDs to new buffer */
        LedCount i;
        for(i = 0; i < MIN(ledcount, c->ledcount); i++)
        {
                led_copy(&newleds[i], &c->leds[i]);
        }

        /* free resources */
        free(c->ledbuffer);
        free(c->leds);

        /* replace with resources that were just created */
        c->buffersize = nbufsize;
        c->ledbuffer = newbuf;
        c->leds = newleds;
        c->ledcount = ledcount;

        return NFT_SUCCESS;
}


/******************************************************************************
 ****************************** API FUNCTIONS *********************************
 ******************************************************************************/

/**
 * create new LED-chain
 *
 * the typical life-cycle of a @ref LedChain looks like this:
 *
 * led_chain_new() <br>
 * [...] <br>
 * led_*_get/set() <br>
 * [...] <br>
 * for every frame <br>
 * { <br>
 *     led_chain_greyscale_from_frame() <br>
 * } <br>
 * [...] <br>
 * led_chain_destroy() <br>
 *
 * @param ledcount chainlength - amount of LEDs in this chain
 * @param pixelformat the pixelformat of this LedChain
 *    - s. @ref pixel_format for supported formats
 * @result LedChain descriptor or NULL; use @ref led_chain_destroy() to free
 */
LedChain *led_chain_new(LedCount ledcount, const char *pixelformat)
{
        /* allocate space for descriptor */
        LedChain *c;
        if(!(c = calloc(1, sizeof(LedChain))))
        {
                NFT_LOG_PERROR("calloc");
                return NULL;
        }

        /* initialize pixel conversion */
        led_pixel_format_new();

        /* set pixelformat */
        if(!(c->format = led_pixel_format_from_string(pixelformat)))
        {
                NFT_LOG(L_ERROR, "Invalid pixel-format: \"%s\"", pixelformat);
                goto _lcn_error;
        }

        /* calculate amount of pixels in this chain */
        int components = led_pixel_format_get_n_components(c->format);

        /* do we have incomplete pixels? */
        if(ledcount % components != 0)
        {
                NFT_LOG(L_WARNING,
                        "We have an incomplete pixel. %d LEDs defined but %d needed for complete %s pixel (%d components)",
                        ledcount, ((ledcount / components) + 1) * components,
                        led_pixel_format_to_string(c->format), components);
        }


        /* amount of pixels */
        int pixels = ledcount / components;
        if(pixels == 0)
        {
                NFT_LOG(L_ERROR,
                        "You didn't define enough LEDs to form at least one %s pixel.",
                        led_pixel_format_to_string(c->format));
                goto _lcn_error;
        }

        /* remember our ledcount */
        c->ledcount = ledcount;

        /** get size of LED greyscale buffer */
        c->buffersize = led_pixel_format_get_buffer_size(c->format, pixels);

        /** allocate buffer to store LED greyscale values */
        if(!(c->ledbuffer = calloc(1, c->buffersize)))
                goto _lcn_error;


        /* allocate LED descriptors */
        if(!(c->leds = calloc(ledcount, sizeof(Led))))
                goto _lcn_error;

        /* allocate space for pointer-offsets */
        if(!(c->mapoffsets = calloc(ledcount, sizeof(int))))
                goto _lcn_error;

        /* register to current LedConfCtxt to create an XML config of this
         * chain */
        // ~ if(!led_settings_chain_register(c))
        // ~ goto _lcn_error;

        led_chain_print(c, L_DEBUG);

        return c;

_lcn_error:
        led_chain_destroy(c);
        return NULL;
}



/**
 * quit usage of LED-chain and free its resources
 *
 * @param c the chain which should be freed, the pointer will be invalid after this function returns
 */
void led_chain_destroy(LedChain * c)
{
        if(!c)
                return;

        /* don't destroy chain that belongs to a hardware */
        if(c->parent_hw)
        {
                NFT_LOG(L_ERROR,
                        "Chain belongs to a LedHardware. Destroy it by destroying the parent");
                return;
        }

        chain_destroy(c);
}


/**
 * create a new LED chain as exact copy of c
 *
 * @param c chain to create a copy of
 * @result newly allocated chain that replicates c or NULL on error
 * @note if you set a private pointer using led_chain_set_privdata(), it will NOT be copied to the duplicate
 */
LedChain *led_chain_dup(LedChain * c)
{
        if(!c)
                NFT_LOG_NULL(NULL);

        LedChain *r;
        if(!
           (r =
            led_chain_new(c->ledcount,
                          led_pixel_format_to_string(c->format))))
                return NULL;

        /* copy LEDs */
        if(!c->leds || !r->leds)
                NFT_LOG_NULL(NFT_FAILURE);

        /* copy LED descriptors */
        memcpy(r->leds, c->leds, r->ledcount * sizeof(Led));

        /* copy LED buffer */
        memcpy(r->ledbuffer, c->ledbuffer, r->buffersize);

        /* copy mapping-buffer */
        memcpy(r->mapoffsets, r->mapoffsets, r->ledcount * sizeof(int));

        return r;
}


/**
 * get ledcount from ledchain
 *
 * @param c @ref LedChain descriptor
 * @result length of chain
 */
LedCount led_chain_get_ledcount(LedChain * c)
{
        if(!c)
                NFT_LOG_NULL(0);

        return c->ledcount;
}


/**
 * change the amount of LEDs this LedChain can hold
 *
 * @param c LedChain descriptor
 * @param ledcount new amount of LEDs
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_chain_set_ledcount(LedChain * c, LedCount ledcount)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        /* silently skip if old_ledcount == new_ledcount */
        if(c->ledcount == ledcount)
                return NFT_SUCCESS;

        /* if this chain is a mapped hardware chain,
         * led_hardware_set_ledcount() must be used */
        if(c->parent_hw)
        {
                NFT_LOG(L_ERROR,
                        "This is a hardware chain. You must use led_hardware_set_ledcount()!");
                return NFT_FAILURE;
        }

        return chain_set_ledcount(c, ledcount);
}


/**
 * get private userdata previously set by led_chain_set_privdata()
 *
 * @param c LedChain descriptor
 * @result private userdata
 */
void *led_chain_get_privdata(LedChain * c)
{
        if(!c)
                NFT_LOG_NULL(NULL);

        return c->privdata;
}


/**
 * associate private userdata pointer with chain
 *
 * @param c LedChain descriptor
 * @param privdata pointer to private userdata
 * @result NFT_SUCCESS or NFT_FAILURE upon error
 */
NftResult led_chain_set_privdata(LedChain * c, void *privdata)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        c->privdata = privdata;

        return NFT_SUCCESS;
}


/**
 * get current data format from LED greyscale values of this led-chain
 *
 * @param c @ref LedChain descriptor
 * @result LedPixelFormat of LedChain
 */
LedPixelFormat *led_chain_get_format(LedChain * c)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        return c->format;
}


/**
 * print debug-info of a chain
 *
 * @param c a LedChain
 * @param l minimum current loglevel so tile gets printed
 */
void led_chain_print(LedChain * c, NftLoglevel l)
{
        if(!c)
                NFT_LOG_NULL();

        NFT_LOG(l,
                "Chain: %p\n"
                "\tLenght: %d\n"
                "\tFormat: %s\n"
                "\tSource format: %s\n"
                "\tBuffersize: %d",
                c,
                c->ledcount,
                c->format ? led_pixel_format_to_string(c->format) : "none",
                c->src_format ? led_pixel_format_to_string(c->src_format) :
                "none", c->buffersize);

        if(nft_log_level_get() <= L_NOISY)
        {

                int bpc = led_pixel_format_get_bytes_per_pixel(c->format) /
                        led_pixel_format_get_n_components(c->format);

                LedCount i;
                for(i = 0; i < c->ledcount; i++)
                {
                        /* get greyscale value */
                        long long int value;
                        led_chain_get_greyscale(c, i, &value);

                        switch (bpc)
                        {
                                case 1:
                                {
                                        NFT_LOG(l,
                                                "Pos: %d\tX: %d\tY: %d\tComponent: %d\tGain: %hu\tGreyscale: %hhu",
                                                i, c->leds[i].x, c->leds[i].y,
                                                c->leds[i].component,
                                                c->leds[i].gain,
                                                (unsigned char) value);
                                        break;
                                }

                                case 2:
                                {
                                        NFT_LOG(l,
                                                "Pos: %d\tX: %d\tY: %d\tComponent: %d\tGain: %hu\tGreyscale: %hu",
                                                i, c->leds[i].x, c->leds[i].y,
                                                c->leds[i].component,
                                                c->leds[i].gain,
                                                (unsigned short) value);
                                        break;
                                }

                                default:
                                {
                                        NFT_LOG(L_ERROR,
                                                "Unsupported bytes-per-component: %d",
                                                bpc);
                                        break;
                                }
                        }
                }
        }

}

/**
 * get smallest x-coordinate of all LEDs in chain
 *
 * @param c LedChain descriptor
 * @result x coordinate in pixels or -1 upon error
 */
LedFrameCord led_chain_get_min_x(LedChain * c)
{
        LedFrameCord r = 0;
        LedCount i;

        if(!c)
                NFT_LOG_NULL(-1);

        /* empty chain? */
        if(c->ledcount == 0)
                return -1;

        for(i = 0; i < c->ledcount; i++)
                r = MIN(r, led_get_x(led_chain_get_nth(c, i)));

        return r;
}


/**
 * get smallest y-coordinate of all LEDs in chain
 *
 * @param c LedChain descriptor
 * @result y coordinate in pixels or -1 upon error
 */
LedFrameCord led_chain_get_min_y(LedChain * c)
{
        LedFrameCord r = 0;
        LedCount i;

        if(!c)
                NFT_LOG_NULL(-1);

        /* empty chain? */
        if(c->ledcount == 0)
                return -1;

        for(i = 0; i < c->ledcount; i++)
                r = MIN(r, led_get_y(led_chain_get_nth(c, i)));

        return r;
}


/**
 * get largest x-coordinate of all LEDs in chain
 *
 * @param c LedChain descriptor
 * @result x coordinate in pixels or -1 upon error
 */
LedFrameCord led_chain_get_max_x(LedChain * c)
{
        LedFrameCord r = 0;
        LedCount i;

        if(!c)
                NFT_LOG_NULL(-1);

        /* empty chain? */
        if(c->ledcount == 0)
                return -1;

        for(i = 0; i < c->ledcount; i++)
                r = MAX(r, led_get_x(led_chain_get_nth(c, i)));

        return r;
}


/**
 * get largest y-coordinate of all LEDs in chain
 *
 * @param c LedChain descriptor
 * @result y coordinate in pixels or -1 upon error
 */
LedFrameCord led_chain_get_max_y(LedChain * c)
{
        LedFrameCord r = 0;
        LedCount i;


        if(!c)
                NFT_LOG_NULL(-1);

        /* empty chain? */
        if(c->ledcount == 0)
                return -1;

        for(i = 0; i < c->ledcount; i++)
                r = MAX(r, led_get_y(led_chain_get_nth(c, i)));

        return r;
}


/**
 * get largest component-value inside chain
 *
 * @param chain LedChain descriptor
 * @result maxmium component-value of all Leds in chain
 */
LedFrameComponent led_chain_get_max_component(LedChain * chain)
{
        LedFrameComponent r = 0;
        LedCount i;
        for(i = 0; i < chain->ledcount; i++)
        {
                LedFrameComponent c;
                if((c = led_get_component(led_chain_get_nth(chain, i))) > r)
                        r = c;
        }

        return r;
}


/**
 * get largest gain-value inside chain
 *
 * @param chain LedChain descriptor
 * @result maxmium gain-value of all Leds in chain
 */
LedGain led_chain_get_max_gain(LedChain * chain)
{
        LedGain r = 0;
        LedCount i;
        for(i = 0; i < chain->ledcount; i++)
        {
                LedGain g;
                if((g = led_get_gain(led_chain_get_nth(chain, i))) > r)
                        r = g;
        }

        return r;
}


/**
 * get parent of this chain
 *
 * @param chain LedChain descriptor
 * @result parent LedHardware of this chain or NULL
 */
LedHardware *led_chain_get_parent_hardware(LedChain * chain)
{
        if(!chain)
                NFT_LOG_NULL(NULL);

        if(!chain->parent_hw && chain->parent_tile)
        {
                NFT_LOG(L_NOISY,
                        "Requested parent hardware but this chain is child of a tile.");
                return NULL;
        }

        return chain->parent_hw;
}


/**
 * get parent of this chain
 *
 * @param chain LedChain descriptor
 * @result parent LedTile of this chain or NULL
 */
LedTile *led_chain_get_parent_tile(LedChain * chain)
{
        if(!chain)
                NFT_LOG_NULL(NULL);

        if(!chain->parent_tile && chain->parent_hw)
        {
                NFT_LOG(L_NOISY,
                        "Requested parent tile but this chain is child of a hardware.");
                return NULL;
        }

        return chain->parent_tile;
}


/**
 * get pointer to ledbuffer of this chain
 *
 * @param chain LedChain descriptor
 * @result raw greyscale buffer of chain or NULL upon error
 */
void *led_chain_get_buffer(LedChain * chain)
{
        if(!chain)
                NFT_LOG_NULL(NULL);

        return chain->ledbuffer;
}


/**
 * get size of current raw-buffer of this chain
 *
 * @param chain LedChain descriptor
 * @result size of chain-buffer in bytes or 0 upon error
 */
size_t led_chain_get_buffer_size(LedChain * chain)
{
        if(!chain)
                NFT_LOG_NULL(0);

        return chain->buffersize;
}


/**
 * get pointer to n-th LED descriptor in LedChain
 *
 * @param c LedChain
 * @param n position of Led to get
 * @result Led descriptor
 */
Led *led_chain_get_nth(LedChain * c, LedCount n)
{
        if(!c)
                NFT_LOG_NULL(NULL);

        if(n >= c->ledcount)
        {
                NFT_LOG(L_ERROR, "n > chain->ledcount (%d > %d)", n,
                        c->ledcount);
                return NULL;
        }

        return &c->leds[n];
}


/**
 * set x-coordinate of a LED inside a pixel-frame for mapping
 *
 * @param l @ref Led descriptor
 * @param x new X coordinate of LED
 */
NftResult led_set_x(Led * l, LedFrameCord x)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        l->x = x;

        return NFT_SUCCESS;
}


/**
 * get x-coordinate of a LED inside a pixel-frame for mapping
 *
 * @param l @ref Led descriptor
 * @result X coordinate of LED
 */
LedFrameCord led_get_x(Led * l)
{
        if(!l)
                NFT_LOG_NULL(0);

        return l->x;
}


/**
 * set y-coordinate of a LED inside a pixel-frame for mapping
 *
 * @param l @ref Led descriptor
 * @param y new Y coordinate of LED
 */
NftResult led_set_y(Led * l, LedFrameCord y)
{
        if(!l)
                NFT_LOG_NULL(NFT_FAILURE);

        l->y = y;

        return NFT_SUCCESS;
}


/**
 * get y-coordinate of a LED inside a pixel-frame for mapping
 *
 * @param l @ref Led descriptor
 * @result Y coordinate of LED
 */
LedFrameCord led_get_y(Led * l)
{
        if(!l)
                NFT_LOG_NULL(0);

        return l->y;
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
 * rearrange chain according to stride
 *
 * @param c LedChain to rearrange
 * @param stride mapping stride
 * @param offset begin mapping of LEDs at this position in LedChain
 * @result amount of mapped LEDs
 */
LedCount led_chain_stride_map(LedChain * c, LedCount stride, LedCount offset)
{
        if(!c)
                NFT_LOG_NULL(-1);

        if(offset >= led_chain_get_ledcount(c))
        {
                NFT_LOG(L_ERROR, "offset (%d) >= chain ledcount (%d)", offset,
                        led_chain_get_ledcount(c));
                return -1;
        }


        /* calculate amount of LEDs to process */
        LedCount count = led_chain_get_ledcount(c) - offset;

        /* do nothing if stride == 0 */
        if(stride == 0)
                return count;

        NFT_LOG(L_DEBUG,
                "Striding %d LEDs of chain (%d LEDs) with stride %d and offset %d",
                count, led_chain_get_ledcount(c), stride, offset);

        /* create duplicate of chain */
        LedChain *tmp;
        if(!(tmp = led_chain_dup(c)))
                return -1;

        /* rearrange LEDs */
        LedCount i = 0;
        LedCount off = 0;
        LedCount pos = 0;
        // Led *dst = c->leds;
        // Led *src = tmp->leds;
        LedChain *dst = c;
        LedChain *src = tmp;
        for(i = 0; i < count; i++)
        {
                /* copy LED */
                Led *led = led_chain_get_nth(src, offset + pos);
                if(!led_copy(led_chain_get_nth(dst, offset + i), led))
                        goto _lhs_exit;

                /* copy greyscale value */
                long long int greyscale = 0;
                led_chain_get_greyscale(src, offset + pos, &greyscale);
                led_chain_set_greyscale(dst, offset + i, greyscale);

                if((pos += stride) >= c->ledcount)
                {
                        off++;
                        pos = off;
                        continue;
                }
        }

_lhs_exit:
        /* free temporary chain */
        led_chain_destroy(tmp);
        return i;
}


/**
 * undo what led_chain_stride_map() did
 *
 * @param c LedChain to unmap
 * @param stride mapping stride
 * @param offset begin unmapping of LEDs at this position in LedChain
 * @result amount of unmapped LEDs
 */
LedCount led_chain_stride_unmap(LedChain * c, LedCount stride,
                                LedCount offset)
{
        if(!c)
                NFT_LOG_NULL(-1);

        if(offset >= led_chain_get_ledcount(c))
        {
                NFT_LOG(L_ERROR, "offset (%d) >= chain ledcount (%d)", offset,
                        led_chain_get_ledcount(c));
                return -1;
        }


        /* calculate amount of LEDs to process */
        LedCount count = led_chain_get_ledcount(c) - offset;

        /* do nothing if stride == 0 */
        if(stride == 0)
                return count;

        NFT_LOG(L_DEBUG,
                "Unstriding %d LEDs of chain (%d LEDs) with stride %d and offset %d",
                count, led_chain_get_ledcount(c), stride, offset);

        /* create duplicate of chain */
        LedChain *tmp;
        if(!(tmp = led_chain_dup(c)))
                return -1;

        /* rearrange LEDs */
        LedCount i = 0;
        LedCount off = 0;
        LedCount pos = 0;
        // Led *dst = c->leds;
        // Led *src = tmp->leds;
        LedChain *dst = c;
        LedChain *src = tmp;
        for(i = 0; i < count; i++)
        {
                /* copy LED */
                Led *led = led_chain_get_nth(src, offset + i);
                if(!led_copy(led_chain_get_nth(dst, offset + pos), led))
                        goto _lhs_exit;

                /* copy greyscale value */
                long long int greyscale = 0;
                led_chain_get_greyscale(src, offset + i, &greyscale);
                led_chain_set_greyscale(dst, offset + pos, greyscale);

                if((pos += stride) >= c->ledcount)
                {
                        off++;
                        pos = off;
                        continue;
                }
        }

_lhs_exit:
        /* free temporary chain */
        led_chain_destroy(tmp);
        return i;
}


/**
 * copy one greyscale value from one buffer to another
 */
static inline void _set_greyscale_value(size_t bpc, void *srcbuf,
                                        void *dstbuf)
{
        /* handle different bytes-per-component for different LedPixelFormats */
        switch (bpc)
        {
                case 1:
                {
                        uint8_t *s = srcbuf;
                        uint8_t *d = dstbuf;
                        *d = *s;
                        break;
                }

                case 2:
                {
                        uint16_t *s = srcbuf;
                        uint16_t *d = dstbuf;
                        *d = *s;
                        break;
                }

                case 3:
                {
                        char *s = srcbuf;
                        char *d = dstbuf;
                        *d = s[0];
                        *d = s[1];
                        *d = s[2];
                        break;
                }

                case 4:
                {
                        int *s = srcbuf;
                        int *d = dstbuf;
                        *d = *s;
                        break;
                }

                case 8:
                {
                        double *s = srcbuf;
                        double *d = dstbuf;
                        *d = *s;
                        break;
                }

                default:
                {
                        NFT_LOG(L_ERROR, "Unsupported component-size: %d",
                                bpc);
                }
        }
}

/**
 * fill chain with greyscale values from another chain
 *
 * @param dst The LED chain whose brightness values should be set 
 * @param src chain to take pixels from
 * @param offset start writing dst at this offset
 * @result amount of LEDs copied to dst 
 */
//~ LedCount led_chain_fill_from_chain(LedChain * dst, LedChain * src, LedCount offset)
//~ {
                // ~ if(!src || !dst)
                                // ~ NFT_LOG_NULL(-1);

                // ~ if(offset >= dst->ledcount)
                // ~ {
                                // ~ NFT_LOG(L_ERROR, "offset (%d) >= ledcount
                                // (%d) of chain", offset, dst->ledcount);
                                // ~ return -1;
                // ~ }

                // ~ /* equal pixelformats? */
        // ~ if(led_pixel_format_is_equal(src->format, dst->format))
                // ~ {
                                // ~ /* amount of components to seek for this
                                // pixel */
                                // ~ char *srcbuf = src->ledbuffer;
                                // ~ char *dstbuf =
                                // dst->ledbuffer+led_pixel_format_get_component_offset(dst->format, 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // 
                                // offset);
                                // ~ LedCount amount =
                                // MIN(dst->ledcount-offset, src->ledcount);
                                // ~ memcpy(dstbuf, srcbuf, amount);

                                // ~ return amount;
                // ~ }

                // ~ /* differing pixelformats */
                // ~ LedPixelFormatConverter *conv;
                // ~ if(!(conv = led_pixel_format_get_converter(src->format,
                // dst->format)))
                // ~ {
                                // ~ NFT_LOG(L_ERROR, "Failed to get converter
                                // for \"%s\"->\"%s\"",
                                        // ~
                                        // led_pixel_format_to_string(src->format),
                                        // ~
                                        // led_pixel_format_to_string(dst->format));
                                // ~ return -1;
                // ~ }

                // ~ /* convert */
                // ~ LedCount amount =
                // src->ledcount/led_pixel_format_get_n_components(src->format);
        // ~ led_pixel_format_convert(conv,
                                                 // ~ src->ledbuffer,
                                                 // dst->ledbuffer,
                                                 // ~ amount);
                // ~ return amount;
//~ }


/**
 * fill chain with pixels from a tile (and sub tiles/chains)
 *
 * @param c The LED chain whose brightness values should be set
 * @param tile LedTile to take pixels from 
 * @result amount of LEDs processed 
 */
//~ LedCount led_chain_fill_from_tile(LedChain * c, LedTile * t, LedCount offset)
//~ {
                // ~ if(!c || !t)
                                // ~ NFT_LOG_NULL(-1);

                // ~ LedCount offs = offset;
                // ~ LedChain *tc;
                // ~ if((tc = led_tile_get_chain(t)))
                // ~ {
                                // ~ if((offs += led_chain_fill_from_chain(c,
                                // tc, offs)) < 0)
                                                // ~ return -1;
                // ~ }

                // ~ LedTile *ct;
                // ~ for(ct = led_tile_get_child(t); ct; ct =
                // led_tile_list_get_next(ct))
                // ~ {
                                // ~ offs += led_chain_fill_from_tile(c, ct,
                                // offs);
                // ~ }

                // ~ return offs;
//~ }


/**
 * fill chain with pixels from a frame
 *
 * @param c - The LED chain whose brightness values should be set
 * @param f - A frame of pixels
 */
NftResult led_chain_fill_from_frame(LedChain * c, LedFrame * f)
{
        LedFrame *srcframe = f;

        if(!c || !f)
                NFT_LOG_NULL(NFT_FAILURE);


#ifdef WORDS_BIGENDIAN
        /* convert little to big endian? */
        if(!led_frame_get_big_endian(f))
        {
                led_frame_convert_endianess(f);

                /* mark frame as converted */
                led_frame_set_big_endian(f, true);
        }
#else
        /* convert big to little endian? */
        if(led_frame_get_big_endian(f))
        {
                led_frame_convert_endianess(f);

                /* mark frame as converted */
                led_frame_set_big_endian(f, false);
        }
#endif

        /* frame format != chain format? */
        if(!led_pixel_format_is_equal(c->format, led_frame_get_format(f)))
        {
                /* do we have a tmpframe already but dimensions differ? */
                if(c->tmpframe &&
                   (led_frame_get_width(c->tmpframe) !=
                    led_frame_get_height(f) ||
                    led_frame_get_height(c->tmpframe) !=
                    led_frame_get_height(f)))
                {
                        /* free tmpframe for we will allocate a new one below */
                        led_frame_destroy(c->tmpframe);
                        c->tmpframe = NULL;
                }

                /* do we need a new temporary frame? */
                if(!c->tmpframe)
                {
                        /* create new temp-frame with dimensions of src-frame
                         * and format of this chain */
                        if(!
                           (c->tmpframe =
                            led_frame_new(led_frame_get_width(f),
                                          led_frame_get_height(f),
                                          c->format)))
                        {
                                return NFT_FAILURE;
                        }

                        /* copy endianess of our frame to tmpframe */
                        led_frame_set_big_endian(c->tmpframe,
                                                 led_frame_get_big_endian(f));
                }

                /* do we need a converter? */
                LedPixelFormat *format = led_frame_get_format(f);
                if(!c->converter ||
                   !led_pixel_format_is_equal(c->src_format, format))
                {
                        /* get new converter */
                        if(!
                           (c->converter =
                            led_pixel_format_get_converter(format,
                                                           c->format)))
                        {
                                NFT_LOG(L_ERROR,
                                        "Failed to create converter for color-conversion");
                                return NFT_FAILURE;
                        }

                        /* save src-format */
                        c->src_format = format;
                }

                /* convert frame */
                led_pixel_format_convert(c->converter,
                                         led_frame_get_buffer(f),
                                         led_frame_get_buffer(c->tmpframe),
                                         led_frame_get_width(f) *
                                         led_frame_get_height(f));

                /* use our tmpframe as src */
                srcframe = c->tmpframe;
        }


        /* map frame src-buffer to chain dest-buffer */
        char *srcbuf = led_frame_get_buffer(srcframe);
        char *dstbuf = c->ledbuffer;
        /* amount of bytes to add for moving from one component to the next
         * component */
        const size_t offset =
                led_pixel_format_get_component_offset(c->format, 1);
        LedCount i;

        /* handle different bytes-per-component */
        const size_t bpc =
                led_pixel_format_get_bytes_per_pixel(c->format) /
                led_pixel_format_get_n_components(c->format);

        /* get every single LED in chain from frame-buffer and write to chain
         * buffer */
        for(i = 0; i < c->ledcount; i++)
        {
                /* copy one greyscale value */
                _set_greyscale_value(bpc, srcbuf + c->mapoffsets[i], dstbuf);

                /* move pointer to destbuffer to next component */
                dstbuf += offset;
        }


        return NFT_SUCCESS;
}


/**
 * initialize the mapping of a frame to this chain
 *
 * @param c LedChain
 * @param f LedFrame
 */
NftResult led_chain_map_from_frame(LedChain * c, LedFrame * f)
{
        if(!c || !f)
                NFT_LOG_NULL(NFT_FAILURE);

        /* first LED in chain */
        Led *l = c->leds;

        /* walk all LEDs */
        LedCount i;
        for(i = 0; i < c->ledcount; i++)
        {
                /* validate coordinates */
                if(l->x < 0 ||
                   l->x > led_frame_get_width(f) ||
                   l->y < 0 || l->y > led_frame_get_height(f))
                {
                        NFT_LOG(L_ERROR, "Illegal coordinates (%d/%d)", l->x,
                                l->y);
                        continue;
                }


                /* amount of components to seek for this pixel */
                size_t n =
                        (led_frame_get_width(f) * l->y +
                         l->x) * led_pixel_format_get_n_components(c->format);
                /* get offset of specific component */
                c->mapoffsets[i] =
                        led_pixel_format_get_component_offset(c->format,
                                                              n +
                                                              l->component);

                /* get next LED - led_chain_get_nth(i) */
                l++;
        }

        return NFT_SUCCESS;
}

/**
 * set greyscale value of a specific LED in a chain
 * @todo merge with frame.c:component_setter
 *
 * @param c LedChain descriptor
 * @param pos LED number
 * @param value new greyscale-value cast to long long int
 */
NftResult led_chain_set_greyscale(LedChain * c, LedCount pos,
                                  long long int value)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        if(pos >= c->ledcount)
        {
                NFT_LOG(L_ERROR,
                        "Invalid LED position: %d (Chainlength is: %d)", pos,
                        c->ledcount);
                return NFT_FAILURE;
        }

        char *src = (char *) &value;
        char *dst = c->ledbuffer;
        dst += led_pixel_format_get_component_offset(c->format, (size_t) pos);

        /* handle different bytes-per-component */
        const size_t bpc =
                led_pixel_format_get_bytes_per_pixel(c->format) /
                led_pixel_format_get_n_components(c->format);

        /* copy one greyscale value */
        _set_greyscale_value(bpc, src, dst);

        return NFT_SUCCESS;
}


/**
 * get greyscale value of a specific LED in a chain
 * @todo merge with frame.c:component_setter
 *
 * @param c LedChain descriptor
 * @param pos LED number
 * @param value greyscale-value cast to long long int
 */
NftResult led_chain_get_greyscale(LedChain * c, LedCount pos,
                                  long long int *value)
{
        if(!c)
                NFT_LOG_NULL(NFT_FAILURE);

        if(pos >= c->ledcount)
        {
                NFT_LOG(L_ERROR,
                        "Invalid LED position: %d (Chainlength is: %d)", pos,
                        c->ledcount);
                return NFT_FAILURE;
        }

        char *src = c->ledbuffer;
        char *dst = (char *) value;
        src += led_pixel_format_get_component_offset(c->format, (size_t) pos);

        /* handle different bytes-per-component */
        const size_t bpc =
                led_pixel_format_get_bytes_per_pixel(c->format) /
                led_pixel_format_get_n_components(c->format);

        /* copy one greyscale value */
        _set_greyscale_value(bpc, src, dst);

        return NFT_SUCCESS;
}


/**
 * return true if this LedChain belongs to a LedHardware
 *
 * @param c a LedChain descriptor
 * @result true or false
 */
bool led_chain_parent_is_hardware(LedChain * c)
{
        return (c->parent_hw ? true : false);
}


/**
 * return true if this LedChain belongs to a LedTile
 *
 * @param c a LedChain descriptor
 * @result true or false
 */
bool led_chain_parent_is_tile(LedChain * c)
{
        return (c->parent_tile ? true : false);
}

/**
 * @}
 */
