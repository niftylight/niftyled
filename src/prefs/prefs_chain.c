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
 * @file prefs_chain.c
 */


/**
 * @addtogroup prefs_chain
 * @{
 */

#include <niftyled.h>



#define LED_CHAIN_PROP_LEDCOUNT "ledcount"
#define LED_CHAIN_PROP_FORMAT   "pixel_format"
#define LED_LED_PROP_X          "x"
#define LED_LED_PROP_Y          "y"
#define LED_LED_PROP_GAIN       "gain"
#define LED_LED_PROP_COMPONENT  "component"




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
 * It's used by nft_prefs_obj_to_node() etc. 
 */
static NftResult _prefs_from_chain(NftPrefs *p, NftPrefsNode *n, void *obj, void *userptr)
{
    	if(!p || !n || !obj)
		NFT_LOG_NULL(NFT_FAILURE);

    
    	/* chain to generate preferences from */
    	LedChain *c = obj;


    	/* amount of LEDs in this chain - ledcount */
	if(!nft_prefs_node_prop_int_set(n, LED_CHAIN_PROP_LEDCOUNT,
	                                  led_chain_get_ledcount(c)))
		return NFT_FAILURE;

        /* pixel-format of this chain */
        if(!nft_prefs_node_prop_string_set(n, LED_CHAIN_PROP_FORMAT,
                         (char *) led_pixel_format_to_string(
                                          led_chain_get_format(c))))
                return NFT_FAILURE;

    
    	/* add all LEDs in this chain */
	LedCount i;
	for(i = 0; i < led_chain_get_ledcount(c); i++)
	{
		/* generate prefs node from LED */
	    	NftPrefsNode *node;
	    	if(!(node = led_prefs_led_to_node(p, led_chain_get_nth(c, i))))
			return NFT_FAILURE;
	    
		/* add node as child of this node */
		if(!nft_prefs_node_add_child(n, node))
			return NFT_FAILURE;
	}

    
	return NFT_SUCCESS;
}


/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedHardware model
 * @param p the current preferences context
 * @param n a freshly created prefs node that this function should fill with properties of obj
 * @param obj object of this class where preferences should be generated from
 * @result NFT_SUCCESS if everything went fine, NFT_FAILURE otherwise
 * @note you shouldn't call this function directly
 * It's used by nft_prefs_obj_to_node() etc. 
 */
static NftResult _prefs_from_led(NftPrefs *p, NftPrefsNode *n, void *obj, void *userptr)
{
    	if(!p || !n || !obj)
		NFT_LOG_NULL(NFT_FAILURE);

    
    	/* Led to generate preferences from */
    	Led *led = obj;

    	/* x-position */
	if(!nft_prefs_node_prop_int_set(n, LED_LED_PROP_X, 
	                                   	led_get_x(led)))
		return NFT_FAILURE;

	/* y-position */
	if(!nft_prefs_node_prop_int_set(n, LED_LED_PROP_Y, 
	                                   	led_get_y(led)))
		return NFT_FAILURE;

	/* gain */
	if(!nft_prefs_node_prop_int_set(n, LED_LED_PROP_GAIN, 
	                                   	led_get_gain(led)))
		return NFT_FAILURE;

	/* component */
	if(!nft_prefs_node_prop_int_set(n, LED_LED_PROP_COMPONENT,
				  		led_get_component(led)))
		return NFT_FAILURE;

    
	return NFT_SUCCESS;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 * It's used by nft_prefs_obj_from_node() etc. 
 */
static NftResult _prefs_to_chain(LedPrefs *p, void **newObj, NftPrefsNode *n, void *userptr)
{
    	if(!p || ! newObj)
		NFT_LOG_NULL(NFT_FAILURE);




    	/* LedCount of chain */
    	LedCount count;
    	if(!nft_prefs_node_prop_int_get(n, LED_CHAIN_PROP_LEDCOUNT, (int *) &count))
    	{
		count = 0;
		
		NFT_LOG(L_WARNING, "chain has no \"%s\" property. Using %d as default.",
		        	LED_CHAIN_PROP_LEDCOUNT, count);
	}

      	/* new chain */
    	LedChain *c;
    
    	/* get format of this chain */
    	char *format;
	if(!(format = nft_prefs_node_prop_string_get(n, 
	                                            LED_CHAIN_PROP_FORMAT)))
    	{	

#define LED_CHAIN_DEFAULT_FORMAT "RGB u8"
		
		NFT_LOG(L_WARNING, "chain has no \"%s\" property. Using \"%s\" as default.",
		        LED_CHAIN_PROP_FORMAT, LED_CHAIN_DEFAULT_FORMAT);

		/* new chain (with default format) */
		if(!(c = led_chain_new(count, LED_CHAIN_DEFAULT_FORMAT)))
		{
			NFT_LOG(L_ERROR, "Failed to create new LedSetup object");
			return NFT_FAILURE;
		}
		
		/* free string */
    		nft_prefs_free(format);
	}
    	/* create new chain with given format */
    	else
    	{
	    /* new chain */
	    if(!(c = led_chain_new(count, format)))
	    {
		    NFT_LOG(L_ERROR, "Failed to create new LedSetup object");
		    return NFT_FAILURE;
	    }
	}

    
    	/* process child nodes (LEDs) */
    	NftPrefsNode *child;
    	LedCount i = 0;
    	for(child = nft_prefs_node_get_first_child(n);
	    child;
	    child = nft_prefs_node_get_next(child))
    	{
		/* check if node describes a Led object */
		if(!led_prefs_is_led_node(child))
		{
			NFT_LOG(L_ERROR, "\"chain\" may only contain \"%s\" children. Skipping \"%s\".", 
			        LED_LED_NAME, nft_prefs_node_get_name(child));
		    	continue;
		}

		/* create Led from node */
		Led *l = led_chain_get_nth(c, i++);
		if(!led_prefs_led_from_node(p, child, l))
			goto _ptc_error;

		
	}

    	/* save new chain-object to "newObj" pointer */
    	*newObj = c;
    
	return NFT_SUCCESS;

    
_ptc_error:
    	led_chain_destroy(c);
    	return NFT_FAILURE;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 * It's used by nft_prefs_obj_from_node() etc.  
 */
static NftResult _prefs_to_led(LedPrefs *c, void **newObj, NftPrefsNode *n, void *userptr)
{
    	if(!c || !n || !userptr)
		NFT_LOG_NULL(NFT_FAILURE);

    	/* we don't need to allocate a Led descriptor here, we'll always get it
           passed as userptr */
    	Led *led = userptr;

    	/* led x-pos */
	int t;
	if(!(nft_prefs_node_prop_int_get(n, LED_LED_PROP_X, &t)))
	{
	    	t = 0;
		NFT_LOG(L_WARNING, "\"led\" has no \"%s\" prop. Using %d as default.",
		        	LED_LED_PROP_X, t);	
	}
	led_set_x(led, (LedFrameCord) t);

	
	/* led y-pos */
	if(!(nft_prefs_node_prop_int_get(n, LED_LED_PROP_Y, &t)))
	{
	    	t = 0;
		NFT_LOG(L_WARNING, "\"led\" has no \"%s\" prop. Using %d as default.",
		        	LED_LED_PROP_Y, t);	
	}
	led_set_y(led, (LedFrameCord) t);

	
	/* led gain */
	if(!(nft_prefs_node_prop_int_get(n, LED_LED_PROP_GAIN, &t)))
    	{
	    	t = 0;
		NFT_LOG(L_WARNING, "\"led\" has no \"%s\" prop. Using %d as default.",
		        	LED_LED_PROP_GAIN, t);	
	}
	if(t < LED_GAIN_MIN || t > LED_GAIN_MAX)
	{
		NFT_LOG(L_WARNING, "<led> config has invalid gain: %d Using 0 instead.", t);
		t = 0;
	}
	led_set_gain(led, (LedGain) t);
	
	
	/* led component */
	if(!(nft_prefs_node_prop_int_get(n, LED_LED_PROP_COMPONENT, &t)))
	{
	    	t = 0;
		NFT_LOG(L_WARNING, "\"led\" has no \"%s\" prop. Using %d as default.",
		        	LED_LED_PROP_COMPONENT, t);	
	}
	led_set_component(led, (LedFrameComponent) t);


      	/* save new chain-object to "newObj" pointer 
     	   (just a dummy in this case to show everything is ok) */
    	*newObj = led;
    
	return NFT_SUCCESS;
}



/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/


/**
 * register "chain" prefs class (called once for initialization)
 */
NftResult _prefs_chain_class_register(NftPrefs *p)
{
    	if(!p)
	    	NFT_LOG_NULL(NFT_FAILURE);
    
	return nft_prefs_class_register(p, LED_CHAIN_NAME, &_prefs_to_chain, &_prefs_from_chain);
}


/**
 * register "led" prefs class (called once for initialization)
 */
NftResult _prefs_led_class_register(NftPrefs *p)
{
    	if(!p)
		NFT_LOG_NULL(NFT_FAILURE);
    
	return nft_prefs_class_register(p, LED_LED_NAME, &_prefs_to_led, &_prefs_from_led);
}




/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/**
 * check if NftPrefsNode represents a chain object
 *
 * @param n LedPrefsNode
 * @result TRUE if node represents a chain object, FALSE otherwise
 */ 
bool led_prefs_is_chain_node(LedPrefsNode *n)
{
	return (strcmp(nft_prefs_node_get_name(n), LED_CHAIN_NAME) == 0);
}


/**
 * check if NftPrefsNode represents a led object
 *
 * @param n LedPrefsNode
 * @result TRUE if node represents a led object, FALSE otherwise
 */ 
bool led_prefs_is_led_node(LedPrefsNode *n)
{
	return (strcmp(nft_prefs_node_get_name(n), LED_LED_NAME) == 0);
}


/**
 * generate LedChain from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param n LedPrefsNode 
 * @result newly created LedChain
 */
LedChain *led_prefs_chain_from_node(LedPrefs *p, LedPrefsNode *n)
{
    	if(!p || !n)
		NFT_LOG_NULL(NULL);

    	/* check if node is of expected class */
    	if(!led_prefs_is_chain_node(n))
    	{
		NFT_LOG(L_ERROR, "got wrong LedPrefsNode class. Expected \"%s\" but got \"%s\"",
		        	LED_CHAIN_NAME, nft_prefs_node_get_name(n));
		return NULL;
	}
    
	return  nft_prefs_obj_from_node(p, n, NULL);
}


/**
 * generate LedPrefsNode from LedChain object
 *
 * @param p LedPrefs context
 * @param c LedChain object 
 * @result newly created LedPrefsNode 
 */
LedPrefsNode *led_prefs_chain_to_node(LedPrefs *p, LedChain *c)
{
    	if(!p || !c)
		NFT_LOG_NULL(NULL);
    
	return nft_prefs_obj_to_node(p, LED_CHAIN_NAME, c, NULL);
}


/**
 * generate Led from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param n LedPrefsNode 
 * @param led Led descriptor that will be filled according to preferences 
 * @result NFT_SUCCESS or NFT_FAILURE upon error
 */
NftResult led_prefs_led_from_node(LedPrefs *p, LedPrefsNode *n, Led *led)
{
	if(!p || !n || !led)
		NFT_LOG_NULL(NFT_FAILURE);

    	/* check if node is of expected class */
       	if(!led_prefs_is_led_node(n))
    	{
		NFT_LOG(L_ERROR, "got wrong LedPrefsNode class. Expected \"%s\" but got \"%s\"",
		        	LED_LED_NAME, nft_prefs_node_get_name(n));
		return NFT_FAILURE;
	}
    
    	/* we'll pass the destination pointer for new object as userpointer */
    	if(!nft_prefs_obj_from_node(p, n, led))
		return NFT_FAILURE;
    
    	return NFT_SUCCESS;
}


/**
 * generate LedPrefsNode from LedChain object
 *
 * @param p LedPrefs context
 * @param l LedChain object 
 * @result newly created LedPrefsNode 
 */
LedPrefsNode *led_prefs_led_to_node(LedPrefs *p, Led *l)
{
	if(!p || !l)
		NFT_LOG_NULL(NULL);
    
	return nft_prefs_obj_to_node(p, LED_LED_NAME, l, NULL);
}


/**
 * @}
 */
