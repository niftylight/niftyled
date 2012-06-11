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



#define LED_CHAIN_SETTING_LEDCOUNT "ledcount"
#define LED_CHAIN_SETTING_FORMAT   "pixel_format"
#define LED_LED_SETTING_X          "x"
#define LED_LED_SETTING_Y          "y"
#define LED_LED_SETTING_GAIN       "gain"
#define LED_LED_SETTING_COMPONENT  "component"




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
static NftResult _prefs_from_chain(NftPrefs *c, NftPrefsNode *n, void *obj, void *userptr)
{
	return NFT_FAILURE;
}


/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedHardware model
 * @param c the current preferences context
 * @param n a freshly created prefs node that this function should fill with properties of obj
 * @param obj object of this class where preferences should be generated from
 * @result NFT_SUCCESS if everything went fine, NFT_FAILURE otherwise
 * @note you shouldn't call this function directly
 */
static NftResult _prefs_from_led(NftPrefs *c, NftPrefsNode *n, void *obj, void *userptr)
{
	return NFT_FAILURE;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 */
static NftResult _prefs_to_chain(LedPrefs *c, void **newObj, NftPrefsNode *n, void *userptr)
{
	return NFT_FAILURE;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 */
static NftResult _prefs_to_led(LedPrefs *c, void **newObj, NftPrefsNode *n, void *userptr)
{
	return NFT_FAILURE;
}


/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedChain model
 * @note you shouldn't call this function directly
 */
//~ static NftSettingsNode *_settings_from_chain(LedSettings *c, LedChain *chain)
//~ {
	//~ if(!c || !chain)
		//~ NFT_LOG_NULL(NULL);

	//~ /* create new config node */
	//~ NftSettingsNode *n;
	//~ if(!(n = nft_settings_node_new(LED_CHAIN_NAME)))
		//~ goto _cfc_error;
	
	
	//~ /* amount of LEDs in this chain - ledcount */
	//~ if(!nft_settings_node_prop_int_set(n, LED_CHAIN_SETTING_LEDCOUNT,
	                                  //~ led_chain_get_ledcount(chain)))
		//~ goto _cfc_error;

        //~ /* pixel-format of this chain */
        //~ if(!nft_settings_node_prop_string_set(n, LED_CHAIN_SETTING_FORMAT,
                                          //~ (char *) led_pixel_format_to_string(led_chain_get_format(chain))))
                //~ goto _cfc_error;

        
	//~ /* add all LEDs in this chain */
	//~ int i;
	//~ for(i = 0; i < led_chain_get_ledcount(chain); i++)
	//~ {
		//~ /* create node */
		//~ NftSettingsNode *led;
		//~ if(!(led = nft_settings_node_new(LED_LED_NAME)))
			//~ goto _cfc_error;

		//~ /* set node as child of this chain-node */
		//~ if(!nft_settings_node_child_append(led, n))
			//~ goto _cfc_error;

		//~ /* x-position */
		//~ if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_X,
	                                  //~ led_get_x(led_chain_get_nth(chain, i))))
			//~ goto _cfc_error;

		//~ /* y-position */
		//~ if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_Y,
	                                  //~ led_get_y(led_chain_get_nth(chain, i))))
			//~ goto _cfc_error;

		//~ /* gain */
		//~ if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_GAIN,
	                                  //~ led_get_gain(led_chain_get_nth(chain, i))))
			//~ goto _cfc_error;

		//~ /* component */
		//~ if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_COMPONENT,
	                                  //~ led_get_component(led_chain_get_nth(chain, i))))
			//~ goto _cfc_error;

	//~ }

		
	//~ /* return newly created node */
	//~ return n;

//~ _cfc_error:
	//~ nft_settings_node_destroy(n);
	//~ return NULL;
//~ }


//~ /**
 //~ * Config-to-Object function.
 //~ * Creates an Led object inside the parent chain from a config node
 //~ * @note you shouldn't call this function directly
 //~ */
//~ void *_settings_to_led(LedSettings *c, NftSettingsNode *n)
//~ {
	//~ /* parent-node == chain? */
	//~ NftSettingsNode *parent = nft_settings_node_parent(n);
	//~ if(!parent ||
	   //~ (strcmp(nft_settings_node_name(parent), LED_CHAIN_NAME) != 0))
	//~ {
		//~ NFT_LOG(L_ERROR, "<led> is only allowed inside a <chain>");
		//~ return NULL;
	//~ }

	//~ /* get chain */
	//~ LedChain *chain;
	//~ if(!(chain = nft_settings_node_obj_get(parent)))
		//~ return NULL;

	//~ /* dummy (everything happens in _settings_to_chain) */
	//~ return chain;
//~ }


//~ /**
 //~ * Config-to-Object function.
 //~ * Creates an LedChain model from a config node
 //~ * @note you shouldn't call this function directly
 //~ */
//~ LedChain *_settings_to_chain(LedSettings *s, NftSettingsNode *n)
//~ {
	//~ if(!s || !n)
		//~ NFT_LOG_NULL(NULL);

        
	//~ /* get ledcount */
	//~ LedCount ledcount;
	//~ if(!(nft_settings_node_prop_int_get(n, LED_CHAIN_SETTING_LEDCOUNT, (int *) &ledcount)))
	//~ {
		//~ NFT_LOG(L_WARNING, "<chain> config-node has no \"ledcount\". Using 0 as default.");
		//~ ledcount = 0;
	//~ }

        //~ /* get greyscale format */
        //~ char *format_string;
        //~ if(!(format_string = nft_settings_node_prop_string_get(n, LED_CHAIN_SETTING_FORMAT)))
        //~ {
                //~ NFT_LOG(L_WARNING, "<chain> config-node has no \"format\". Using \"RGB u8\" as default.");
                //~ format_string = (char *) xmlStrdup((xmlChar *) "RGB u8");
        //~ }
        
        
	//~ /* our resulting chain */
	//~ LedChain *r = NULL;
        
        
           
        //~ /** parent node existing? */
        //~ NftSettingsNode *parent;
	//~ if((parent = nft_settings_node_parent(n)))
        //~ {
                //~ /** parent-node == hardware? */
                //~ if(strcmp(nft_settings_node_name(parent), LED_HARDWARE_NAME) == 0)
                //~ {
                        //~ /* create chain by initializing hardware */
                        //~ LedHardware *h = nft_settings_node_obj_get(parent);
                        //~ if(!led_hardware_init(h, led_hardware_get_id(h), ledcount, format_string))
                        //~ {
                                //~ NFT_LOG(L_ERROR, "Failed to initialize hardware: %s (%s)", 
                                        //~ led_hardware_get_name(h), led_hardware_get_id(h));
                                //~ goto _stc_exit;
                        //~ }

                        //~ r = led_hardware_get_chain(h);
                //~ }
                //~ /** parent-node == tile? */
                //~ else if(strcmp(nft_settings_node_name(parent), LED_TILE_NAME) == 0)
                //~ {                        
                        //~ LedTile *t = nft_settings_node_obj_get(parent);
                        //~ r = led_chain_new(ledcount, format_string);
                        //~ led_tile_set_chain(t, r);
                //~ }
                //~ /* invalid parent node */
                //~ else
                //~ {
                        //~ NFT_LOG(L_ERROR, "<chain> node must either be child of a <hardware> or a <tile> node");
                        //~ goto _stc_exit;
                //~ }
        //~ }

        
	//~ /* walk all led-nodes in this chain. Therefore we get an iterator */
	//~ NftSettingsNodeIter *i;
	//~ if(!(i = nft_settings_node_iter_ref(s, LED_LED_NAME, n)))
		//~ goto _stc_exit;

	
	
	//~ /* walk all objects */
        //~ LedCount a = 0;
	//~ do
	//~ {
		//~ /* get config-node from iter */
		//~ NftSettingsNode *led;
		//~ if(!(led = nft_settings_node_iter_node_get(i)))
			//~ break;

		
		//~ /* led x-pos */
                //~ int t;
		//~ if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_X, &t)))
		//~ {
			//~ NFT_LOG(L_WARNING, "<led> config-node has no \"x\" prop. Using 0 as default.");
			//~ t = 0;
		//~ }
		//~ led_set_x(led_chain_get_nth(r, a), (LedFrameCord) t);

		
		//~ /* led y-pos */
		//~ if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_Y, &t)))
		//~ {
			//~ NFT_LOG(L_WARNING, "<led> config-node has no \"y\" prop. Using 0 as default.");
			//~ t = 0;
		//~ }
		//~ led_set_y(led_chain_get_nth(r, a), (LedFrameCord) t);

		
		//~ /* led gain */
		//~ if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_GAIN, &t)))
		//~ {
			//~ NFT_LOG(L_WARNING, "<led> config-node has no \"gain\". Using 0 as default.");
			//~ t = 0;
		//~ }
                //~ if(t < LED_GAIN_MIN || t > LED_GAIN_MAX)
                //~ {
                        //~ NFT_LOG(L_WARNING, "<led> config has invalid gain: %d Using 0 instead.", t);
                        //~ t = 0;
                //~ }
                //~ led_set_gain(led_chain_get_nth(r, a), (LedGain) t);
                
                
		//~ /* led component */
		//~ if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_COMPONENT, &t)))
		//~ {
			//~ NFT_LOG(L_WARNING, "<led> config-node has no \"component\". Using 0 as default.");
			//~ t = 0;
		//~ }
		//~ led_set_component(led_chain_get_nth(r, a), (LedFrameComponent) t);
                
		//~ /* next led */
		//~ a++;
	//~ }
	//~ while(nft_settings_node_iter_next(i));

	//~ /* unref iterator */
	//~ nft_settings_node_iter_unref(i);

        
//~ _stc_exit:
        //~ /* free resources */
        //~ nft_settings_node_prop_string_free(format_string);
        
	//~ return r;
//~ }



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
 * generate LedChain from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param newly created LedChain
 */
LedChain *led_prefs_chain_from_node(LedPrefs *p, LedPrefsNode *n)
{
    	if(!p || !n)
		NFT_LOG_NULL(NULL);

    	/* check if node is of expected class */
    	if(strcmp(nft_prefs_node_get_name(n), LED_CHAIN_NAME) != 0)
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
 * @param h LedChain object 
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
 * @param led Led descriptor that will be filled according to preferences 
 * @param NFT_SUCCESS or NFT_FAILURE upon error
 */
NftResult led_prefs_led_from_node(LedPrefs *p, LedPrefsNode *n, Led *led)
{
	if(!p || !n || !led)
		NFT_LOG_NULL(NFT_FAILURE);

    	/* check if node is of expected class */
       	if(strcmp(nft_prefs_node_get_name(n), LED_LED_NAME) != 0)
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
 * @param h LedChain object 
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
