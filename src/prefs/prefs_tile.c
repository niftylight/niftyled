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
 * @addtogroup prefs_tile
 * @{
 */

#include <math.h>
#include <niftyled.h>


#define LED_TILE_PROP_X              "x"
#define LED_TILE_PROP_Y              "y"
#define LED_TILE_PROP_WIDTH          "width"
#define LED_TILE_PROP_HEIGHT         "height"
#define LED_TILE_PROP_ROTATION       "rotation"
#define LED_TILE_PROP_ROT_X          "pivot_x"
#define LED_TILE_PROP_ROT_Y          "pivot_y"





/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/


/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedHardware model
 * @param c the current preferences context
 * @param n a freshly created prefs node that this function should fill with properties of obj
 * @param obj object of this class where preferences should be generated from
 * @result NFT_SUCCESS if everything went fine, NFT_FAILURE otherwise
 * @note you shouldn't call this function directly
 */
static NftResult _prefs_from_tile(NftPrefs *p, NftPrefsNode *n, void *obj, void *userptr)
{
	if(!p || !n || !obj)
		NFT_LOG_NULL(NFT_FAILURE);

    	/* tile to generate preferences from */
    	LedTile *t = obj;


    	/* x offset */
    	if(!nft_prefs_node_prop_int_set(n, LED_TILE_PROP_X, led_tile_get_x(t)))
                return NFT_FAILURE;
           
	/* y offset */
    	if(!nft_prefs_node_prop_int_set(n, LED_TILE_PROP_Y, led_tile_get_y(t)))
                return NFT_FAILURE;
    
        /* mapping width */
    	if(!nft_prefs_node_prop_int_set(n, LED_TILE_PROP_WIDTH, led_tile_get_width(t)))
                return NFT_FAILURE;

	/* mapping height */
    	if(!nft_prefs_node_prop_int_set(n, LED_TILE_PROP_HEIGHT, led_tile_get_height(t)))
                return NFT_FAILURE;    	

        /* rotation center x */
    	if(!nft_prefs_node_prop_double_set(n, LED_TILE_PROP_ROT_X, led_tile_get_pivot_x(t)))
                return NFT_FAILURE;  

        /* rotation center y */
    	if(!nft_prefs_node_prop_double_set(n, LED_TILE_PROP_ROT_Y, led_tile_get_pivot_y(t)))
                return NFT_FAILURE;  
       
	/* rotation angle (radians -> degrees) */
    	if(!nft_prefs_node_prop_double_set(n, LED_TILE_PROP_ROTATION, (led_tile_get_rotation(t)*180)/M_PI))
                return NFT_FAILURE; 

    
        /* chain of this tile */
    	LedChain *c;
    	if((c = led_tile_get_chain(t)))
	{
	    	/* generate prefs node from chain */
		NftPrefsNode *node;
		if(!(node = led_prefs_chain_to_node(p, c)))
			return NFT_FAILURE;

		/* add node as child of this node */
		nft_prefs_node_add_child(n, node);
    	}
    
	
	/* child tiles of this tile */
        LedTile *child;
	for(child = led_tile_get_child(t); child; child = led_tile_get_next_sibling(child))
	{
	    	/* generate prefs node from tile */
	    	NftPrefsNode *node;
	    	if(!(node = led_prefs_tile_to_node(p, child)))
			return NFT_FAILURE;
	    
		/* add node as child of this node */
		if(!nft_prefs_node_add_child(n, node))
			return NFT_FAILURE;
	}
	
	/* all went fine */
	return NFT_SUCCESS;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 */
static NftResult _prefs_to_tile(LedPrefs *p, void **newObj, NftPrefsNode *n, void *userptr)
{
	if(!p || !n)
		NFT_LOG_NULL(NFT_FAILURE);

	
	/* get x offset */
	LedFrameCord x;
    	if(!nft_prefs_node_prop_int_get(n, LED_TILE_PROP_X, &x))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_PROP_X);
		x = 0;
	}

	/* get y offset */
	LedFrameCord y;
    	if(!nft_prefs_node_prop_int_get(n, LED_TILE_PROP_Y, &y))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_PROP_Y);
		y = 0;
	}
	       
	
        /* get rotation center x */
	double rot_x;
	if(!nft_prefs_node_prop_double_get(n, LED_TILE_PROP_ROT_X, &rot_x))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_PROP_ROT_X);
		rot_x = 0;
	}
		       
        /* get rotation center y */
	double rot_y;
	if(!nft_prefs_node_prop_double_get(n, LED_TILE_PROP_ROT_Y, &rot_y))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_PROP_ROT_Y);
		rot_y = 0;
	}
		           
	/* get rotation angle */
	double rotation;
	if(!nft_prefs_node_prop_double_get(n, LED_TILE_PROP_ROTATION, &rotation))
	{
		NFT_LOG(L_WARNING, "<tile> config-node has no \"%s\" offset. Using 0 as default.", LED_TILE_PROP_ROTATION);
		rotation = 0;
	}
		           
	/* convert degrees to radians */
        rotation = (rotation*M_PI)/180;


	/* create new tile */
	LedTile *t;
	if(!(t = led_tile_new()))
		return NFT_FAILURE;

        /* set correct attributes */
	led_tile_set_x(t, x);
	led_tile_set_y(t, y);
        led_tile_set_pivot_x(t, rot_x);
        led_tile_set_pivot_y(t, rot_y);
	led_tile_set_rotation(t, rotation);

	   
	/* process child nodes */
	LedPrefsNode *child;
	for(child = nft_prefs_node_get_first_child(n);
	    child;
	    child = nft_prefs_node_get_next(child))
	{
		/* is child a chain node? */
	    	if(led_prefs_is_chain_node(child))
	    	{
			/* only one chain for every tile */
			if(led_tile_get_chain(t))
			{
				NFT_LOG(L_WARNING, "preferences contain more than one \"chain\" for \"tile\" node (only one allowed -> ignoring node)");
			    	continue;
			}
			
			/* generate chain & add to tile */
			if(!(led_tile_set_chain(t, led_prefs_chain_from_node(p, child))))
			{
				NFT_LOG(L_ERROR, "Failed to add \"chain\" to \"tile\". Aborting.");
				goto _ptt_error;
			}
		}
	    	/* do we have a child-tile node? */
	    	else if(led_prefs_is_tile_node(child))
	    	{
			if(!led_tile_append_child(t, led_prefs_tile_from_node(p, child)))
			{
				NFT_LOG(L_ERROR, "Failed to add \"tile\" to \"tile\". Aborting.");
			    	goto _ptt_error;
			}
		}
	    	else
		/* invalid node? */
	    	{
			NFT_LOG(L_WARNING, "Attempt to add \"%s\" node to tile. Only \"chain\" and \"tile\" allowed. (Ignoring node)",
			        nft_prefs_node_get_name(child));
			continue;
		}
	}
	
    	/* save new chain-object to "newObj" pointer */
    	*newObj = t;
        
	return NFT_SUCCESS;

        
_ptt_error:
	led_tile_destroy(t);
	return NFT_FAILURE;
}


/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/

/**
 * register "hardware" prefs class (called once for initialization)
 */
NftResult _prefs_tile_class_register(NftPrefs *p)
{
    	if(!p)
		NFT_LOG_NULL(NFT_FAILURE);
    
	return nft_prefs_class_register(p, LED_TILE_NAME, &_prefs_to_tile, &_prefs_from_tile);
}



/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/**
 * check if NftPrefsNode represents a tile object
 *
 * @param n LedPrefsNode
 * @result TRUE if node represents a tile object, FALSE otherwise
 */ 
bool led_prefs_is_tile_node(LedPrefsNode *n)
{
	return (strcmp(nft_prefs_node_get_name(n), LED_TILE_NAME) == 0);
}


/**
 * generate LedTile from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param n LedPrefsNode 
 * @result newly created LedTile
 */
LedTile *led_prefs_tile_from_node(LedPrefs *p, LedPrefsNode *n)
{
    	if(!p || !n)
		NFT_LOG_NULL(NULL);
    
    	/* check if node is of expected class */
    	if(!led_prefs_is_tile_node(n))
    	{
		NFT_LOG(L_ERROR, "got wrong LedPrefsNode class. Expected \"%s\" but got \"%s\"",
		        	LED_TILE_NAME, nft_prefs_node_get_name(n));
		return NULL;
	}
    
	return  nft_prefs_obj_from_node(p, n, NULL);
}


/**
 * generate LedPrefsNode from LedTile object
 *
 * @param p LedPrefs context
 * @param t LedTile object 
 * @result newly created LedPrefsNode 
 */
LedPrefsNode *led_prefs_tile_to_node(LedPrefs *p, LedTile *t)
{
	return nft_prefs_obj_to_node(p, LED_TILE_NAME, t, NULL);
}


/**
 * @}
 */
