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
 * @file settings.c
 */


/**
 * @addtogroup settings
 * @{
 */

#include <niftyled.h>
#include <niftysettings.h>
#include <errno.h>
#include "config.h"




/* static declarations */
static NftSettingsCtxt *_cctxt;

/* declare external functions */
extern LedHardware *_settings_to_hardware(LedSettings *c, NftSettingsNode *node);
extern LedChain *_settings_to_chain(LedSettings *c, NftSettingsNode *node);
extern void *_settings_to_led(LedSettings *c, NftSettingsNode *node);
extern LedTile *_settings_to_tile(LedSettings *c, NftSettingsNode *n);


/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/

/**
 * deinitialize config-module
 *
 */
static void _settings_deinit()
{
	nft_settings_deinit(_cctxt);
        _cctxt = NULL;
}


/**
 * initialize config-module
 */
static NftResult _settings_init()
{
	if(!(_cctxt = nft_settings_init(GENERIC_SETTINGS_FILE_VERSION)))
                return NFT_FAILURE;

        atexit(_settings_deinit);
        return NFT_SUCCESS;
}


/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/



/**
 * create new LedSettings model
 * @note this corresponds to one config-file
 *
 * @result LedSettings descriptor or NULL upon error
 */
LedSettings *led_settings_new()
{
	LedSettings *c;

	/* create fresh config model */
	if(!(c = nft_settings_new(led_settings_context(), "niftyled")))
		return NULL;

	/* register toObj functions for all our LED-config objects */
	if(!(nft_settings_func_to_obj_set(led_settings_context(), LED_HARDWARE_NAME, (NftSettingsToObjFunc *) _settings_to_hardware, TRUE)))
		return NULL;
	if(!(nft_settings_func_to_obj_set(led_settings_context(), LED_CHAIN_NAME, (NftSettingsToObjFunc *) _settings_to_chain, TRUE)))
		return NULL;
	if(!(nft_settings_func_to_obj_set(led_settings_context(), LED_LED_NAME, (NftSettingsToObjFunc *) _settings_to_led, TRUE)))
		return NULL;
	if(!(nft_settings_func_to_obj_set(led_settings_context(), LED_TILE_NAME, (NftSettingsToObjFunc *) _settings_to_tile, TRUE)))
                return NULL;
        
	return c;
}


/**
 * destroy setup and all it's resources
 *
 * @param c LedSettings descriptor
 */
void led_settings_destroy(LedSettings *c)
{        
        nft_settings_destroy(c);
}


/**
 * get current config context
 *
 * @result current LedSettingsCtxt
 */
LedSettingsCtxt *led_settings_context()
{
        if(!_cctxt)
        {
                if(!_settings_init())
                        return NULL;
        }
        
        return _cctxt;
}


/**
 * load & parse config from file
 *
 * @param filename full path to XML config file
 * @result LedSettings -structure or NULL upon error
 */
LedSettings *led_settings_load(const char *filename)
{
	LedSettings *c;

	/* create new config */
	if(!(c = led_settings_new()))
		return NULL;

        /* load config */
	if(!nft_settings_load(c, filename))
	{
		led_settings_destroy(c);
		return NULL;
	}

        /* check version */
        if((GENERIC_SETTINGS_FILE_VERSION / 10000) != (nft_settings_get_version(c) / 10000))
        {
                NFT_LOG(L_ERROR, "Config file has version %d but we need at least %d. Please upgrade config.",
                        nft_settings_get_version(c), GENERIC_SETTINGS_FILE_VERSION);
                return NULL;
        }

        if(GENERIC_SETTINGS_FILE_VERSION != nft_settings_get_version(c))
        {
                NFT_LOG(L_WARNING, "Config file has version %d but most recent version is %d. Please upgrade config.",
                        nft_settings_get_version(c), GENERIC_SETTINGS_FILE_VERSION);
        }
        
	/* create & initialize all top-level objects */
	NftSettingsNode *n;
	for(n = nft_settings_node_child(nft_settings_root(c));
	    n;
	    n = nft_settings_node_next(n))
	{
		/* create new object */
		if(!(nft_settings_func_to_obj_call(c, n, NULL, NULL)))
		{
			NFT_LOG(L_WARNING, "Failed to create object \"%s\"",
			        nft_settings_node_name(n));
			continue;
		}
	}

        /* debug output currently parsed & regenerated config */
        /*LedSettings *t = led_settings_new();
        led_settings_from_hardware(t, led_settings_hardware_first(c));
        led_settings_from_tile(t, led_settings_tile_first(c));
        led_settings_save(t, "-");
        led_settings_destroy(t);*/
        
	return c;
}


/**
 * save config to a file
 *
 * @param c LedSettings descriptor of config to save
 * @param filename full path of file to save
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_settings_save(LedSettings *c, const char *filename)
{
	/* write config */
	return nft_settings_save(c, filename);
}


/**
 * get or build default config-filename
 * if NIFTY_CONFIG environment variable is set, that is returned.
 * Other wise the path to the supplied filename in the HOME dir is returned.
 * If filename is NULL, the default-filename is returned
 *
 * @param dst destination buffer to write path
 * @param size length of that buffer in bytes
 * @param filename the filename that should be used to build path if nothing more appropriate is found
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_settings_default_filename(char *dst, size_t size, const char *filename)
{
        /* try to get environment variable */
        char *env;
        if((env = getenv("NIFTY_SETTINGS")))
        {
                strncpy(dst, env, size);
                return NFT_SUCCESS;
        }
        
        /* build our own default path */
        if(filename)
        {
                if(snprintf(dst, size, "%s/%s", getenv("HOME"), filename) < 0)
                {
                        NFT_LOG_PERROR("snprintf");
                        return NFT_FAILURE;
                }
        }
        /* use our own default filename */
        else
        {
                if(snprintf(dst, size, "%s/.niftyled.xml", getenv("HOME")) < 0)
                {
                        NFT_LOG_PERROR("snprintf");
                        return NFT_FAILURE;
                }
        }
        
        
        return NFT_SUCCESS;
}


/**
 * get width of complete setup
 *
 * @param s LedSettings descriptor
 * @result total width of current setup in pixels
 */
LedFrameCord led_settings_get_width(LedSettings *s)
{
        if(!s)
                NFT_LOG_NULL(0);

        LedHardware *first;
        if(!(first = led_settings_hardware_get_first(s)))
        {
                NFT_LOG(L_DEBUG, "Couldn't get width. No hardware node in setup.");
                return 0;
        }

        LedHardware *h;
        LedFrameCord width = 0;
        for(h = first; h; h = led_hardware_get_next_sibling(h))
        {
                LedTile *tile = led_hardware_get_tile(h);
                LedFrameCord t = led_tile_get_width(tile)+led_tile_get_x(tile);
                if(t > width)
                        width = t;
        }

        return width;

}


/**
 * get height of complete setup
 *
 * @param s LedSettings descriptor
 * @result total height of current setup in pixels
 */
LedFrameCord led_settings_get_height(LedSettings *s)
{
        if(!s)
                NFT_LOG_NULL(0);

        LedHardware *first;
        if(!(first = led_settings_hardware_get_first(s)))
        {
                NFT_LOG(L_DEBUG, "Couldn't get height. No hardware node in setup.");
                return 0;
        }

        LedHardware *h;
        LedFrameCord height = 0;
        for(h = first; h; h = led_hardware_get_next_sibling(h))
        {
                LedTile *tile = led_hardware_get_tile(h);
                LedFrameCord t = led_tile_get_height(tile)+led_tile_get_y(tile);
                if(t > height)
                        height = t;
        }

        return height;
}


/**
 * return type of first XML node found
 *
 * @param xml string containing one niftyled XML node
 * @result NIFTYLED_TYPE
 */
NIFTYLED_TYPE led_settings_node_get_type(const char *xml)
{
        /* parse node */
        NftSettingsNode *n;
        if(!(n = nft_settings_node_parse(xml)))
        {
                NFT_LOG(L_ERROR, "Failed to parse XML node");
                return T_LED_INVALID;
        }


        NIFTYLED_TYPE t;

        if(strcmp(nft_settings_node_name(n), LED_HARDWARE_NAME) == 0)
                t = T_LED_HARDWARE;
        else if(strcmp(nft_settings_node_name(n), LED_TILE_NAME) == 0)
                t = T_LED_TILE;
        else if(strcmp(nft_settings_node_name(n), LED_CHAIN_NAME) == 0)
                t = T_LED_CHAIN;
        else if(strcmp(nft_settings_node_name(n), LED_LED_NAME) == 0)
                t = T_LED;
        else
                t = T_LED_INVALID;
        
        nft_settings_node_destroy(n);

        return t;
}

/**
 * @}
 */
