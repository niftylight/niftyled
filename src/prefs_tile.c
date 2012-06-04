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
 * @file prefs_tile.c
 */


/**
 * @addtogroup settings_tile
 * @{
 */

#include <math.h>
#include <niftyled.h>


#define LED_TILE_SETTING_X              "x"
#define LED_TILE_SETTING_Y              "y"
#define LED_TILE_SETTING_WIDTH          "width"
#define LED_TILE_SETTING_HEIGHT         "height"
#define LED_TILE_SETTING_ROTATION       "rotation"
#define LED_TILE_SETTING_ROT_X          "pivot_x"
#define LED_TILE_SETTING_ROT_Y          "pivot_y"
#define LED_TILE_SETTING_CHILDCOUNT     "children"



/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedTile model
 * @note you shouldn't call this function directly
 */
static NftSettingsNode *_settings_from_tile(LedSettings *c, LedTile *m)
{
	if(!c || !m)
		NFT_LOG_NULL(NULL);

	/* create new settings node */
	NftSettingsNode *n;
	if(!(n = nft_settings_node_new(LED_TILE_NAME)))
		goto _cfm_error;
	

        /* x offset */
	if(!nft_settings_node_prop_int_set(n, LED_TILE_SETTING_X,
					led_tile_get_x(m)))
		goto _cfm_error;

	/* y offset */
	if(!nft_settings_node_prop_int_set(n, LED_TILE_SETTING_Y,
					led_tile_get_y(m)))
		goto _cfm_error;

        /* mapping width */
	if(!nft_settings_node_prop_int_set(n, LED_TILE_SETTING_WIDTH,
					led_tile_get_width(m)))
		goto _cfm_error;

	/* mapping height */
	if(!nft_settings_node_prop_int_set(n, LED_TILE_SETTING_HEIGHT,
					led_tile_get_height(m)))
		goto _cfm_error;

        /* rotation center x */
        if(!nft_settings_node_prop_double_set(n, LED_TILE_SETTING_ROT_X, 
                                       led_tile_get_pivot_x(m)))
                goto _cfm_error;

        /* rotation center y */
        if(!nft_settings_node_prop_double_set(n, LED_TILE_SETTING_ROT_Y, 
                                       led_tile_get_pivot_y(m)))
                goto _cfm_error;
        
	/* rotation angle (radians -> degrees) */
	if(!nft_settings_node_prop_double_set(n, LED_TILE_SETTING_ROTATION,
	                                  (led_tile_get_rotation(m)*180)/M_PI))
		goto _cfm_error;
        
        /* chain of this tile */
        if(led_tile_get_chain(m))
        {
                if(!nft_settings_func_from_obj_call(c, led_tile_get_chain(m), n, NULL))
                        goto _cfm_error;
        }

	/* child tiles of this tile */
        LedTile *child;
	for(child = led_tile_get_child(m); child; child = led_tile_get_next_sibling(child))
	{
		/* add child node to this node */
		if(!nft_settings_func_from_obj_call(c, child, n, NULL))
		{
			goto _cfm_error;
		}
	}
	
	/* return newly created node */
	return n;

_cfm_error:
	nft_settings_node_destroy(n);
	return NULL;
}


/**
 * Config-to-Object function.
 * Creates an LedTile model from a config node
 * @note you shouldn't call this function directly
 */
LedTile *_settings_to_tile(LedSettings *c, NftSettingsNode *n)
{
	if(!c || !n)
		NFT_LOG_NULL(NULL);

	
	/* get x offset */
	LedFrameCord x;
	if(!(nft_settings_node_prop_int_get(n, LED_TILE_SETTING_X, &x)))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_SETTING_X);
		x = 0;
	}

	/* get y offset */
	LedFrameCord y;
	if(!(nft_settings_node_prop_int_get(n, LED_TILE_SETTING_Y, &y)))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_SETTING_Y);
		y = 0;
	}

        /* get rotation center x */
	double rot_x;
	if(!(nft_settings_node_prop_double_get(n, LED_TILE_SETTING_ROT_X, &rot_x)))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\". Using 0 as default.", LED_TILE_SETTING_ROT_X);
		rot_x = 0;
	}

        /* get rotation center y */
	double rot_y;
	if(!(nft_settings_node_prop_double_get(n, LED_TILE_SETTING_ROT_Y, &rot_y)))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\". Using 0 as default.", LED_TILE_SETTING_ROT_Y);
		rot_y = 0;
	}
        
	/* get rotation angle */
	double rotation;
	if(!(nft_settings_node_prop_double_get(n, LED_TILE_SETTING_ROTATION, &rotation)))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\". Using 0 as default.", LED_TILE_SETTING_ROTATION);
		rotation = 0;
	}
        
	/* convert degrees to radians */
        rotation = (rotation*M_PI)/180;
        
	/* create new tile */
	LedTile *r = NULL;
	if(!(r = led_tile_new()))
		return NULL;

        /* set correct attributes */
	led_tile_set_x(r, x);
	led_tile_set_y(r, y);
        led_tile_set_pivot_x(r, rot_x);
        led_tile_set_pivot_y(r, rot_y);
	led_tile_set_rotation(r, rotation);


        /* is this a child of another tile? (parent node == tile?) */
        NftSettingsNode *parent;
        if((parent = nft_settings_node_parent(n)))
           
        {
                /* parent is <tile> */
                if(strcmp(nft_settings_node_name(parent), LED_TILE_NAME) == 0)
                {
                        /* append this chain to parent */
                        LedTile *p_tile = nft_settings_node_obj_get(parent);
                        if(!(led_tile_append_child(p_tile, r)))
                                goto _ctm_error;
                }
                /* parent is <hardware> */
                else if(strcmp(nft_settings_node_name(parent), LED_HARDWARE_NAME) == 0)
                {
                        /* does this hardware already have a "first-child"? */
                        LedHardware *hw = nft_settings_node_obj_get(parent);
                        if(led_hardware_get_tile(hw))
                        {
                                NFT_LOG(L_ERROR,"Tile is child of a hardware that already has a child.");
                                goto _ctm_error;
                        }

                        /* hw already has tile(s) set? */
                        LedTile *tile;
                        if((tile = led_hardware_get_tile(hw)))
                        {
                                if(!(led_tile_append_sibling(tile, r)))
                                        goto _ctm_error;
                        }
                        else
                        {
                                /* append this chain to parent */
                                if(!(led_hardware_set_tile(hw, r)))
                                        goto _ctm_error;
                        }
                }
                /* invalid or no parent */
                else
                {
                        NFT_LOG(L_ERROR, "<tile> node must either be child of a <hardware> or another <tile> node");
                        goto _ctm_error;
                }
        }
        else
        {
                NFT_LOG(L_ERROR, "<tile> has no parent node. Must either <hardware> or another <tile> node");
                        goto _ctm_error;
        }
        
	return r;

        
_ctm_error:
	led_tile_destroy(r);
	return NULL;
}


/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/** 
 * create config from LedTile object
 */
NftResult led_settings_create_from_tile(LedSettings *c, LedTile *t)
{
	if(!c || !t)
		NFT_LOG_NULL(NFT_FAILURE);
	
	if(!nft_settings_func_from_obj_call(c, t, NULL, NULL))
		return NFT_FAILURE;

	return NFT_SUCCESS;
}


/**
 * register one LedTile object to config-module
 *
 * @note this is called by the *_new() function, only use to register something manually
 * @param t the tile to register
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_settings_tile_register(LedTile *t)
{
	if(!t)
		NFT_LOG_NULL(NFT_FAILURE);
	
	return nft_settings_func_from_obj_set(led_settings_context(), t, (NftSettingsFromObjFunc *) _settings_from_tile);
}


/**
 * unregister one LedTile object from config-module
 *
 * @note this is called by the *_destroy(), only use to unregister something manually
 * @param t the tile to unregister
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_settings_tile_unregister(LedTile *t)
{
	if(!t)
		NFT_LOG_NULL(NFT_FAILURE);
	
	/* unregister our config-creator */
	nft_settings_func_from_obj_unset(led_settings_context(), t);       

        return NFT_SUCCESS;
}


/**
 * unlink tile from settings tree
 */
NftResult led_settings_tile_unlink(LedSettings *s, LedTile *t)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_find_rec_by_obj(nft_settings_root((LedSettings *) s), t)))
        {
                NFT_LOG(L_ERROR, "no setup node for tile %p found", t);
                return NFT_FAILURE;
        }
        
        nft_settings_node_destroy(n);
        
        return NFT_SUCCESS;
}


/**
 * get XML description of tile as string
 *
 * @param c LedSettings
 * @param t LedTile
 * @result string with XML dump
 * @note free() result when it's not needed anymore
 */
const char *led_settings_tile_dump_xml(LedSettings *c, LedTile *t)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_find_rec_by_obj(nft_settings_root(c), t)))
        {
                NFT_LOG(L_WARNING, "Request to dump XML data from tile without calling led_settings_tile_register() first");
                return NULL;
        }

        return nft_settings_node_dump(n);
}


/**
 * create a new LedTile from an XML tile description
 *
 * @param xml string containing a complete & valid XML node
 * @result newly allocated LedTile descriptor
 */
LedTile *led_settings_tile_parse_xml(const char *xml)
{
        if(!xml)
                NFT_LOG_NULL(NULL);
        
        NftSettingsNode *n;
        if(!(n = nft_settings_node_parse(xml)))
        {
                NFT_LOG(L_ERROR, "failed to parse <tile> XML node");
                return NULL;
        }

        LedTile *result;
        result = _settings_to_tile(NULL, n);

        nft_settings_node_destroy(n);
        
        return result;
}


/**
 * @}
 */
 
