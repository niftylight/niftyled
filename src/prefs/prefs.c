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

#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include "niftyled-hardware.h"
#include "niftyled-chain.h"
#include "niftyled-tile.h"
#include "niftyled-prefs.h"
#include "_prefs_setup.h"
#include "_prefs_hardware.h"
#include "_prefs_tile.h"
#include "_prefs_chain.h"
#include "config.h"


/** printable type names (must match order of _type_name_types!) */
const char *_type_name_strings[] = {
        LED_SETUP_NAME,
        LED_HARDWARE_NAME,
        LED_TILE_NAME,
        LED_CHAIN_NAME,
        LED_LED_NAME,
        "invalid",
        NULL
};

/** corresponding numeric types (must match order of _type_name_strings!) */
const NIFTYLED_TYPE _type_name_types[] = {
        LED_SETUP_T,
        LED_HARDWARE_T,
        LED_TILE_T,
        LED_CHAIN_T,
        LED_T,
        LED_INVALID_T
};

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
        if(!(prefs_setup_class_register(p)))
                goto _lpi_error;

        /* register hardware class */
        if(!(prefs_hardware_class_register(p)))
                goto _lpi_error;

        /* register tile class */
        if(!(prefs_tile_class_register(p)))
                goto _lpi_error;

        /* register chain class */
        if(!(prefs_chain_class_register(p)))
                goto _lpi_error;

        /* register LED class */
        if(!(prefs_led_class_register(p)))
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
void led_prefs_deinit(LedPrefs * p)
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
NftResult led_prefs_default_filename(char *dst, size_t size,
                                     const char *filename)
{
        /* try to get environment variable */
        char *env;
        if((env = getenv("NFT_SETUP")))
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
                if(snprintf(dst, size, "%s/.niftyled.xml", getenv("HOME")) <
                   0)
                {
                        NFT_LOG_PERROR("snprintf");
                        return NFT_FAILURE;
                }
        }


        return NFT_SUCCESS;
}


/**
 * get URL of the origin of this node or NULL
 *
 * @param n LedPrefsNode node
 * @result filename of current preferences or NULL
 */
const char *led_prefs_node_get_filename(LedPrefsNode * n)
{
        return nft_prefs_node_get_filename(n);
}


/**
 * dump LedPrefsNode and all children to a printable buffer
 *
 * @param n LedPrefsNode to dump
 * @result newly allocated buffer. (use free() to deallocate)
 */
char *led_prefs_node_to_buffer_light(LedPrefsNode * n)
{
        return nft_prefs_node_to_buffer_light(n);
}


/**
 * dump fully encapsulated LedPrefsNode and all children to a printable buffer
 *
 * @param n LedPrefsNode to dump
 * @result newly allocated buffer. (use free() to deallocate)
 */
char *led_prefs_node_to_buffer(LedPrefsNode * n)
{
        return nft_prefs_node_to_buffer(n);
}


/**
 * dump LedPrefsNode and all children to a file
 *
 * @param n LedPrefsNode to dump
 * @param filename full path of file to be written
 * @param overwrite if a file called "filename" already exists, it.
 * will be overwritten if this is "true", otherwise NFT_FAILURE will be returned 
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_prefs_node_to_file_light(LedPrefsNode * n, const char *filename,
                                       bool overwrite)
{
        return nft_prefs_node_to_file_light(n, filename, overwrite);
}


/**
 * dump LedPrefsNode and all children to a file
 * fully encapsulated by the underlying prefs mechanism
 *
 * @param n LedPrefsNode to dump
 * @param filename full path of file to be written
 * @param overwrite if a file called "filename" already exists, it.
 * will be overwritten if this is "true", otherwise NFT_FAILURE will be returned 
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_prefs_node_to_file(LedPrefsNode * n, const char *filename,
                                 bool overwrite)
{
        return nft_prefs_node_to_file(n, filename, overwrite);
}


/**
 * parse buffer and create LedPrefsNode accordingly
 *
 * @param buffer src buffer to parse
 * @param bufsize size of buffer in bytes
 * @result newly created LedPrefsNode (use led_prefs_node_free() to deallocate)
 */
LedPrefsNode *led_prefs_node_from_buffer(char *buffer, size_t bufsize)
{
        return nft_prefs_node_from_buffer(buffer, bufsize);
}


/**
 * parse file and create LedPrefsNode accordingly
 *
 * @param filename full path of file to parse
 * @result newly created LedPrefsNode (use led_prefs_node_free() to deallocate)
 */
LedPrefsNode *led_prefs_node_from_file(const char *filename)
{
        return nft_prefs_node_from_file(filename);
}


/**
 * free resources of a LedPrefsNode + all children
 *
 * @param n LedPrefsNode to deallocate
 */
void led_prefs_node_free(LedPrefsNode * n)
{
        nft_prefs_node_free(n);
}


/**
 * get NIFTYLED_TYPE of a prefs node
 *
 * @param n LedPrefsNode to get type of
 * @result NIFTYLED_TYPE of this node or LED_INVALID_T on unhandled node
 */
NIFTYLED_TYPE led_prefs_node_get_type(LedPrefsNode * n)
{
        if(!n)
                NFT_LOG_NULL(LED_INVALID_T);

        const char *name = nft_prefs_node_get_name(n);

        return led_prefs_type_from_string(name);
}


/**
 * convert type name to NIFTYLED_TYPE
 *
 * @param name printable string of type
 * @result NIFTYLED_TYPE or LED_INVALID_T on unhandled type
 */
NIFTYLED_TYPE led_prefs_type_from_string(const char *name)
{
        /* we don't need much space */
        char *tmp = alloca(256);
        size_t i;
        for(i = 0; i < 255 && name[i]; i++)
        {
                tmp[i] = tolower(name[i]);
        }

        /* terminate string */
        tmp[i] = '\0';

        for(i = 0; _type_name_strings[i]; i++)
        {
                if(strcmp(tmp, _type_name_strings[i]) == 0)
                        return _type_name_types[i];
        }

        return LED_INVALID_T;
}


/**
 * convert NIFTYLED_TYPE to printable string
 *
 * @param type numeric NIFTYLED_TYPE
 * @result printable string
 */
const char *led_prefs_type_to_string(NIFTYLED_TYPE type)
{
        size_t i;
        for(i = 0; _type_name_strings[i]; i++)
        {
                if(_type_name_types[i] == type)
                        return _type_name_strings[i];
        }

        return "invalid";
}


/**
 * @}
 */
