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
 * @file prefs.c
 */


/**
 * @addtogroup prefs
 * @{
 */

#include <errno.h>
#include "niftyled-hardware.h"
#include "niftyled-chain.h"
#include "niftyled-tile.h"
#include "niftyled-prefs.h"
#include "_prefs_setup.h"
#include "_prefs_hardware.h"
#include "_prefs_tile.h"
#include "_prefs_chain.h"
#include "config.h"





//~ /* declare external functions */
//~ extern LedHardware *_prefs_to_hardware(LedPrefs *c, NftPrefsNode *node);
//~ extern LedChain *_prefs_to_chain(LedPrefs *c, NftPrefsNode *node);
//~ extern void *_prefs_to_led(LedPrefs *c, NftPrefsNode *node);
//~ extern LedTile *_prefs_to_tile(LedPrefs *c, NftPrefsNode *n);


/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/


/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/



/**
 * create new LedPrefs model
 * @note this corresponds to one config-file
 *
 * @result LedPrefs descriptor or NULL upon error
 */
LedPrefs *led_prefs_init()
{
    	/* initialize libniftyprefs */
	LedPrefs *p;
	if(!(p = nft_prefs_init()))
		return NULL;


    	/* register overall setup class */
    	if(!(_prefs_setup_class_register(p)))
		goto _lpi_error;
	         
	/* register hardware class */
    	if(!(_prefs_hardware_class_register(p)))
		goto _lpi_error;

	/* register tile class */
	if(!(_prefs_tile_class_register(p)))
		goto _lpi_error;
    
	/* register chain class */
    	if(!(_prefs_chain_class_register(p)))
		goto _lpi_error;

	/* register LED class */
	if(!(_prefs_led_class_register(p)))
		goto _lpi_error;
        
	return p;
    
_lpi_error:
    	nft_prefs_deinit(p);
    	return NULL;
}


/**
 * destroy setup and all it's resources
 *
 * @param p LedPrefs descriptor
 */
void led_prefs_deinit(LedPrefs *p)
{        
        nft_prefs_deinit(p);
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
NftResult led_prefs_default_filename(char *dst, size_t size, const char *filename)
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


/** wrapper for nft_prefs_node_from_buffer */
LedPrefsNode *led_prefs_node_from_buffer(LedPrefs *p, char *buffer, size_t bufsize)
{
	return nft_prefs_node_from_buffer(p, buffer, bufsize);
}


/** wrapper for nft_prefs_node_from_buffer */
LedPrefsNode *led_prefs_node_from_file(LedPrefs *p, const char *filename)
{
	return nft_prefs_node_from_file(p, filename);
}


/** wrapper for nft_prefs_node_from_buffer */
char *led_prefs_node_to_buffer(LedPrefs *p, LedPrefsNode *n)
{
	return nft_prefs_node_to_buffer(p, n);
}


/** wrapper for nft_prefs_node_from_buffer */
NftResult led_prefs_node_to_file(LedPrefs *p, LedPrefsNode *n, const char *filename)
{
	return nft_prefs_node_to_file(p, n, filename);
}


/** wrapper for nft_prefs_node_from_buffer */
void led_prefs_node_free(LedPrefsNode *n)
{
	return nft_prefs_node_free(n);
}



/**
 * @}
 */
