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
 * @file prefs_hardware.c
 */


/**
 * @addtogroup prefs_hardware 
 * @{
 */

#include "niftyled-prefs_hardware.h"
#include "niftyled-prefs_chain.h"



/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))


#define	LED_HARDWARE_PROP_NAME                  "name"
#define	LED_HARDWARE_PROP_PLUGIN                "plugin"
#define	LED_HARDWARE_PROP_ID                    "id"
#define LED_HARDWARE_PROP_STRIDE                "stride"




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
static NftResult _prefs_from_hardware(NftPrefs *p, NftPrefsNode *n, void *obj, void *userptr)
{
	if(!p || !n | !obj)
		NFT_LOG_NULL(NFT_FAILURE);

    	/* hardware "object" */
    	LedHardware *h = obj;
    
	
	
	/* name of hardware */
	if(!nft_prefs_node_prop_string_set(n, LED_HARDWARE_PROP_NAME,
	                                  (char *) led_hardware_get_name(h)))
		return NFT_FAILURE;
	

	/* plugin family of hardware */
	if(!nft_prefs_node_prop_string_set(n, LED_HARDWARE_PROP_PLUGIN,
	                                  (char *) led_hardware_get_name(h)))
		return NFT_FAILURE;
    
	
	/* id of hardware */
	if(!nft_prefs_node_prop_string_set(n, LED_HARDWARE_PROP_ID,
	                                  (char *) led_hardware_get_id(h)))
		return NFT_FAILURE;
	
      
	/* LED stride */
        if(!nft_prefs_node_prop_int_set(n, LED_HARDWARE_PROP_STRIDE,
                                        led_hardware_get_stride(h)))
                return NFT_FAILURE;

        
        /* chain of this hardware */
    	LedChain *c;
    	if((c = led_hardware_get_chain(h)))
	{
	    	/* generate prefs node from chain */
		NftPrefsNode *node;
		if(!(node = led_prefs_chain_to_node(p, c)))
			return NFT_FAILURE;

		/* add node as child of this node */
		nft_prefs_node_add_child(n, node);
    	}
    
        
	/* all OK */
	return NFT_SUCCESS;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 */
static NftResult _prefs_to_hardware(LedPrefs *c, void **newObj, NftPrefsNode *n, void *userptr)
{
	if(!c || !newObj || !n)
		NFT_LOG_NULL(NFT_FAILURE);

    	/** initial result */
    	NftResult r = NFT_FAILURE;
    
        LedHardware *h = *newObj;
        //~ char *name = NULL;
	//~ char *plugin_name = NULL;
        //~ char *id = NULL;
        

	//~ /* get hardware name */
	//~ if(!(name = nft_settings_node_prop_string_get(n, LED_HARDWARE_SETTING_NAME)))
	//~ {
		//~ NFT_LOG(L_ERROR, "<hardware> config-node has no name");
		//~ goto _cth_end;
	//~ }

	//~ /* get plugin-name */
	//~ if(!(plugin_name = nft_settings_node_prop_string_get(n, LED_HARDWARE_SETTING_PLUGIN)))
	//~ {
		//~ NFT_LOG(L_ERROR, "<hardware> config-node has no \"plugin\" type");
		//~ goto _cth_end;
	//~ }

	//~ /* get plugin-id */
	//~ if(!(id = nft_settings_node_prop_string_get(n, LED_HARDWARE_SETTING_ID)))
	//~ {
		//~ NFT_LOG(L_ERROR, "<hardware> config-node has no \"id\" type");
		//~ goto _cth_end;
	//~ }

        //~ /* get stride */
        //~ LedCount stride;
        //~ if(!(nft_settings_node_prop_int_get(n, LED_HARDWARE_SETTING_STRIDE, (int *) &stride)))
        //~ {
                //~ NFT_LOG(L_WARNING, "<hardware> config-node has no \"stride\". Using 0 as default.");
                //~ stride = 0;
        //~ }
        
	//~ /* create new hardware object */
	//~ LedHardware *h;
	//~ if(!(h = led_hardware_new(name, plugin_name)))
	//~ {
		//~ goto _cth_end;
	//~ }
        
        //~ /* set stride */
        //~ if(!(led_hardware_set_stride (h, stride)))
                //~ goto _cth_end;

        //~ /* set id */
        //~ if(!(led_hardware_set_id(h, id)))
                //~ goto _cth_end;

	//~ /* initialize hardware */
	//~ /*if(!(led_hardware_init(h, ledcount, (LedGreyscaleFormat) format)))
	//~ {
		//~ NFT_LOG(L_WARNING, "<hardware> successfully loaded but failed to initialize. Continuing anyway.");
	//~ }*/


        //~ /* is this a sibling of another hardware? (a previous node == hardware?) */
	//~ NftSettingsNode *prev;
        //~ for(prev = nft_settings_node_prev(n); prev; prev = nft_settings_node_prev(prev))
        //~ {
                //~ if(strcmp(nft_settings_node_name(prev), LED_HARDWARE_NAME) == 0)
                //~ {
                        //~ /* register this hardware to previous sibling */
                        //~ LedHardware *prev_h = nft_settings_node_obj_get(prev);
                        //~ if(!(led_hardware_set_sibling(prev_h, h)))
                                //~ goto _cth_end;
                        //~ break;
                //~ }
        //~ }
        
	//~ /* return new hardware object */
	//~ r = h;
	
//~ _cth_end:
	//~ /* free strings */
	//~ nft_settings_node_prop_string_free(id);
	//~ nft_settings_node_prop_string_free(plugin_name);
	//~ nft_settings_node_prop_string_free(name);
	
	return r;
}




/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/

/**
 * register "hardware" prefs class (called once for initialization)
 */
NftResult _prefs_hardware_class_register(NftPrefs *p)
{
    	if(!p)
		NFT_LOG_NULL(NFT_FAILURE);
    
	return nft_prefs_class_register(p, LED_HARDWARE_NAME, &_prefs_to_hardware, &_prefs_from_hardware);
}





/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/


/**
 * check if NftPrefsNode represents a hardware object
 *
 * @param n LedPrefsNode
 * @result TRUE if node represents a hardware object, FALSE otherwise
 */ 
bool led_prefs_is_hardware_node(LedPrefsNode *n)
{
	return (strcmp(nft_prefs_node_get_name(n), LED_HARDWARE_NAME) == 0);
}


/**
 * generate LedHardware from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param n LedPrefsNode 
 * @result newly created LedHardware
 */
LedHardware *led_prefs_hardware_from_node(LedPrefs *p, LedPrefsNode *n)
{
    	if(!p || !n)
		NFT_LOG_NULL(NULL);
    
    	/* check if node is of expected class */
    	if(strcmp(nft_prefs_node_get_name(n), LED_HARDWARE_NAME) != 0)
    	{
		NFT_LOG(L_ERROR, "got wrong LedPrefsNode class. Expected \"%s\" but got \"%s\"",
		        	LED_HARDWARE_NAME, nft_prefs_node_get_name(n));
		return NULL;
	}
    
	return  nft_prefs_obj_from_node(p, n, NULL);
}


/**
 * generate LedPrefsNode from LedHardware object
 *
 * @param p LedPrefs context
 * @param h LedHardware object 
 */
LedPrefsNode *led_prefs_hardware_to_node(LedPrefs *p, LedHardware *h)
{
	return nft_prefs_obj_to_node(p, LED_HARDWARE_NAME, h, NULL);
}


/**
 * @}
 */
