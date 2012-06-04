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
 * @addtogroup settings_chain
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
 * Creates a config-node (and subnodes) from a LedChain model
 * @note you shouldn't call this function directly
 */
static NftSettingsNode *_settings_from_chain(LedSettings *c, LedChain *chain)
{
	if(!c || !chain)
		NFT_LOG_NULL(NULL);

	/* create new config node */
	NftSettingsNode *n;
	if(!(n = nft_settings_node_new(LED_CHAIN_NAME)))
		goto _cfc_error;
	
	
	/* amount of LEDs in this chain - ledcount */
	if(!nft_settings_node_prop_int_set(n, LED_CHAIN_SETTING_LEDCOUNT,
	                                  led_chain_get_ledcount(chain)))
		goto _cfc_error;

        /* pixel-format of this chain */
        if(!nft_settings_node_prop_string_set(n, LED_CHAIN_SETTING_FORMAT,
                                          (char *) led_pixel_format_to_string(led_chain_get_format(chain))))
                goto _cfc_error;

        
	/* add all LEDs in this chain */
	int i;
	for(i = 0; i < led_chain_get_ledcount(chain); i++)
	{
		/* create node */
		NftSettingsNode *led;
		if(!(led = nft_settings_node_new(LED_LED_NAME)))
			goto _cfc_error;

		/* set node as child of this chain-node */
		if(!nft_settings_node_child_append(led, n))
			goto _cfc_error;

		/* x-position */
		if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_X,
	                                  led_get_x(led_chain_get_nth(chain, i))))
			goto _cfc_error;

		/* y-position */
		if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_Y,
	                                  led_get_y(led_chain_get_nth(chain, i))))
			goto _cfc_error;

		/* gain */
		if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_GAIN,
	                                  led_get_gain(led_chain_get_nth(chain, i))))
			goto _cfc_error;

		/* component */
		if(!nft_settings_node_prop_int_set(led, LED_LED_SETTING_COMPONENT,
	                                  led_get_component(led_chain_get_nth(chain, i))))
			goto _cfc_error;

	}

		
	/* return newly created node */
	return n;

_cfc_error:
	nft_settings_node_destroy(n);
	return NULL;
}



/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/

/**
 * Config-to-Object function.
 * Creates an Led object inside the parent chain from a config node
 * @note you shouldn't call this function directly
 */
void *_settings_to_led(LedSettings *c, NftSettingsNode *n)
{
	/* parent-node == chain? */
	NftSettingsNode *parent = nft_settings_node_parent(n);
	if(!parent ||
	   (strcmp(nft_settings_node_name(parent), LED_CHAIN_NAME) != 0))
	{
		NFT_LOG(L_ERROR, "<led> is only allowed inside a <chain>");
		return NULL;
	}

	/* get chain */
	LedChain *chain;
	if(!(chain = nft_settings_node_obj_get(parent)))
		return NULL;

	/* dummy (everything happens in _settings_to_chain) */
	return chain;
}


/**
 * Config-to-Object function.
 * Creates an LedChain model from a config node
 * @note you shouldn't call this function directly
 */
LedChain *_settings_to_chain(LedSettings *s, NftSettingsNode *n)
{
	if(!s || !n)
		NFT_LOG_NULL(NULL);

        
	/* get ledcount */
	LedCount ledcount;
	if(!(nft_settings_node_prop_int_get(n, LED_CHAIN_SETTING_LEDCOUNT, (int *) &ledcount)))
	{
		NFT_LOG(L_WARNING, "<chain> config-node has no \"ledcount\". Using 0 as default.");
		ledcount = 0;
	}

        /* get greyscale format */
        char *format_string;
        if(!(format_string = nft_settings_node_prop_string_get(n, LED_CHAIN_SETTING_FORMAT)))
        {
                NFT_LOG(L_WARNING, "<chain> config-node has no \"format\". Using \"RGB u8\" as default.");
                format_string = (char *) xmlStrdup((xmlChar *) "RGB u8");
        }
        
        
	/* our resulting chain */
	LedChain *r = NULL;
        
        
           
        /** parent node existing? */
        NftSettingsNode *parent;
	if((parent = nft_settings_node_parent(n)))
        {
                /** parent-node == hardware? */
                if(strcmp(nft_settings_node_name(parent), LED_HARDWARE_NAME) == 0)
                {
                        /* create chain by initializing hardware */
                        LedHardware *h = nft_settings_node_obj_get(parent);
                        if(!led_hardware_init(h, led_hardware_get_id(h), ledcount, format_string))
                        {
                                NFT_LOG(L_ERROR, "Failed to initialize hardware: %s (%s)", 
                                        led_hardware_get_name(h), led_hardware_get_id(h));
                                goto _stc_exit;
                        }

                        r = led_hardware_get_chain(h);
                }
                /** parent-node == tile? */
                else if(strcmp(nft_settings_node_name(parent), LED_TILE_NAME) == 0)
                {                        
                        LedTile *t = nft_settings_node_obj_get(parent);
                        r = led_chain_new(ledcount, format_string);
                        led_tile_set_chain(t, r);
                }
                /* invalid parent node */
                else
                {
                        NFT_LOG(L_ERROR, "<chain> node must either be child of a <hardware> or a <tile> node");
                        goto _stc_exit;
                }
        }

        
	/* walk all led-nodes in this chain. Therefore we get an iterator */
	NftSettingsNodeIter *i;
	if(!(i = nft_settings_node_iter_ref(s, LED_LED_NAME, n)))
		goto _stc_exit;

	
	
	/* walk all objects */
        LedCount a = 0;
	do
	{
		/* get config-node from iter */
		NftSettingsNode *led;
		if(!(led = nft_settings_node_iter_node_get(i)))
			break;

		
		/* led x-pos */
                int t;
		if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_X, &t)))
		{
			NFT_LOG(L_WARNING, "<led> config-node has no \"x\" prop. Using 0 as default.");
			t = 0;
		}
		led_set_x(led_chain_get_nth(r, a), (LedFrameCord) t);

		
		/* led y-pos */
		if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_Y, &t)))
		{
			NFT_LOG(L_WARNING, "<led> config-node has no \"y\" prop. Using 0 as default.");
			t = 0;
		}
		led_set_y(led_chain_get_nth(r, a), (LedFrameCord) t);

		
		/* led gain */
		if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_GAIN, &t)))
		{
			NFT_LOG(L_WARNING, "<led> config-node has no \"gain\". Using 0 as default.");
			t = 0;
		}
                if(t < LED_GAIN_MIN || t > LED_GAIN_MAX)
                {
                        NFT_LOG(L_WARNING, "<led> config has invalid gain: %d Using 0 instead.", t);
                        t = 0;
                }
                led_set_gain(led_chain_get_nth(r, a), (LedGain) t);
                
                
		/* led component */
		if(!(nft_settings_node_prop_int_get(led, LED_LED_SETTING_COMPONENT, &t)))
		{
			NFT_LOG(L_WARNING, "<led> config-node has no \"component\". Using 0 as default.");
			t = 0;
		}
		led_set_component(led_chain_get_nth(r, a), (LedFrameComponent) t);
                
		/* next led */
		a++;
	}
	while(nft_settings_node_iter_next(i));

	/* unref iterator */
	nft_settings_node_iter_unref(i);

        
_stc_exit:
        /* free resources */
        nft_settings_node_prop_string_free(format_string);
        
	return r;
}



/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/** 
 * create config from LedChain object
 */
NftResult led_settings_create_from_chain(LedSettings *c, LedChain *chain)
{
	if(!c || !chain)
		NFT_LOG_NULL(NFT_FAILURE);
	
	if(!nft_settings_func_from_obj_call(c, chain, NULL, NULL))
		return NFT_FAILURE;

	return NFT_SUCCESS;
}


/**
 * register one LedChain object to config-module
 */
NftResult led_settings_chain_register(LedChain *chain)
{
	if(!chain)
		NFT_LOG_NULL(NFT_FAILURE);
	
	return nft_settings_func_from_obj_set(led_settings_context(), chain, (NftSettingsFromObjFunc *) _settings_from_chain);
}


/**
 * unregister one LedChain object from config-module
 */
NftResult led_settings_chain_unregister(LedChain *chain)
{
	if(!chain)
		NFT_LOG_NULL(NFT_FAILURE);
	
	/* unregister our config-creator */
	nft_settings_func_from_obj_unset(led_settings_context(), chain);

        return NFT_SUCCESS;
}


/**
 * unlink chain from settings tree
 */
NftResult led_settings_chain_unlink(LedSettings *s, LedChain *c)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_find_rec_by_obj(nft_settings_root((LedSettings *) s), c)))
        {
                NFT_LOG(L_ERROR, "no setup node for chain %p found", c);
                return NFT_FAILURE;
        }
        
        nft_settings_node_destroy(n);
        
        return NFT_SUCCESS;
}


/**
 * get XML description of chain as string
 *
 * @param s LedSettings
 * @param c LedChain
 * @result string with XML dump
 * @note free() result when it's not needed anymore
 */
const char *led_settings_chain_dump_xml(LedSettings *s, LedChain *c)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_find_rec_by_obj(nft_settings_root(s), c)))
        {
                NFT_LOG(L_WARNING, "Request to dump XML data from chain without calling led_settings_chain_register() first");
                return NULL;
        }

        return nft_settings_node_dump(n);
}


/**
 * create a new LedChain from an XML tile description
 *
 * @param xml string containing a complete & valid XML node
 * @result newly allocated LedChain descriptor
 */
LedChain *led_settings_chain_parse_xml(const char *xml)
{
        if(!xml)
                NFT_LOG_NULL(NULL);
        
        NftSettingsNode *n;
        if(!(n = nft_settings_node_parse(xml)))
        {
                NFT_LOG(L_ERROR, "failed to parse <chain> XML node");
                return NULL;
        }

        LedChain *result;
        result = _settings_to_chain(NULL, n);

        nft_settings_node_destroy(n);
        
        return result;
}


/**
 * @}
 */
