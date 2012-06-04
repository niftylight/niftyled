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
 * @addtogroup settings_hardware 
 * @{
 */

#include <niftyled.h>


/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))


#define	LED_HARDWARE_SETTING_NAME                  "name"
#define	LED_HARDWARE_SETTING_PLUGIN                "plugin"
#define	LED_HARDWARE_SETTING_ID                    "id"
#define LED_HARDWARE_SETTING_STRIDE                "stride"




/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/

/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedHardware model
 * @note you shouldn't call this function directly
 */
static NftSettingsNode *_settings_from_hardware(LedSettings *c, LedHardware *h)
{
	if(!c || !h)
		NFT_LOG_NULL(NULL);

	/* create new config node for this hardware */
	NftSettingsNode *n;
	if(!(n = nft_settings_node_new(LED_HARDWARE_NAME)))
		goto _cfh_error;
	

	
	
	/* name of hardware */
	if(!nft_settings_node_prop_string_set(n, LED_HARDWARE_SETTING_NAME,
	                                  (char *) led_hardware_get_name(h)))
		goto _cfh_error;
	

	/* plugin family of hardware */
	if(!nft_settings_node_prop_string_set(n, LED_HARDWARE_SETTING_PLUGIN, 
					(char *) led_hardware_get_plugin_family(h)))
		goto _cfh_error;
	
	
	/* id of hardware */
	if(!nft_settings_node_prop_string_set(n, LED_HARDWARE_SETTING_ID,
	                                  (char *) led_hardware_get_id(h)))
		goto _cfh_error;
	
      
	/* LED stride */
        if(!nft_settings_node_prop_int_set(n, LED_HARDWARE_SETTING_STRIDE,
                                       led_hardware_get_stride(h)))
                goto _cfh_error;

        
        /* chain of this hardware */
        LedChain *chain;
        if((chain = led_hardware_get_chain(h)))
        {
                if(!nft_settings_func_from_obj_call(c, chain, n, NULL))
                        goto _cfh_error;
        }
        
	/* return newly created node */
	return n;

        
_cfh_error:
	nft_settings_node_destroy(n);
	return NULL;
}






/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/

/**
 * Config-to-Object function.
 * Creates an LedHardware model from a config node
 * @note you shouldn't call this function directly
 */
LedHardware *_settings_to_hardware(LedSettings *c, NftSettingsNode *n)
{
	if(!n)
		NFT_LOG_NULL(NULL);

        
        char *name = NULL;
	char *plugin_name = NULL;
        char *id = NULL;
        LedHardware *r = NULL;

	/* get hardware name */
	if(!(name = nft_settings_node_prop_string_get(n, LED_HARDWARE_SETTING_NAME)))
	{
		NFT_LOG(L_ERROR, "<hardware> config-node has no name");
		goto _cth_end;
	}

	/* get plugin-name */
	if(!(plugin_name = nft_settings_node_prop_string_get(n, LED_HARDWARE_SETTING_PLUGIN)))
	{
		NFT_LOG(L_ERROR, "<hardware> config-node has no \"plugin\" type");
		goto _cth_end;
	}

	/* get plugin-id */
	if(!(id = nft_settings_node_prop_string_get(n, LED_HARDWARE_SETTING_ID)))
	{
		NFT_LOG(L_ERROR, "<hardware> config-node has no \"id\" type");
		goto _cth_end;
	}

        /* get stride */
        LedCount stride;
        if(!(nft_settings_node_prop_int_get(n, LED_HARDWARE_SETTING_STRIDE, (int *) &stride)))
        {
                NFT_LOG(L_WARNING, "<hardware> config-node has no \"stride\". Using 0 as default.");
                stride = 0;
        }
        
	/* create new hardware object */
	LedHardware *h;
	if(!(h = led_hardware_new(name, plugin_name)))
	{
		goto _cth_end;
	}
        
        /* set stride */
        if(!(led_hardware_set_stride (h, stride)))
                goto _cth_end;

        /* set id */
        if(!(led_hardware_set_id(h, id)))
                goto _cth_end;

	/* initialize hardware */
	//~ if(!(led_hardware_init(h, ledcount, (LedGreyscaleFormat) format)))
	//~ {
		//~ NFT_LOG(L_WARNING, "<hardware> successfully loaded but failed to initialize. Continuing anyway.");
	//~ }


        /* is this a sibling of another hardware? (a previous node == hardware?) */
	NftSettingsNode *prev;
        for(prev = nft_settings_node_prev(n); prev; prev = nft_settings_node_prev(prev))
        {
                if(strcmp(nft_settings_node_name(prev), LED_HARDWARE_NAME) == 0)
                {
                        /* register this hardware to previous sibling */
                        LedHardware *prev_h = nft_settings_node_obj_get(prev);
                        if(!(led_hardware_set_sibling(prev_h, h)))
                                goto _cth_end;
                        break;
                }
        }
        
	/* return new hardware object */
	r = h;
	
_cth_end:
	/* free strings */
	nft_settings_node_prop_string_free(id);
	nft_settings_node_prop_string_free(plugin_name);
	nft_settings_node_prop_string_free(name);
	
	return r;
}



/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/** 
 * create config from LedHardware object
 */
NftResult led_settings_create_from_hardware(LedSettings *c, LedHardware *h)
{
        NftSettingsNode *n;
	if(!(n = nft_settings_func_from_obj_call(c, h, NULL, NULL)))
		return NFT_FAILURE;

        /* call handlers for plugin-specific property */
        nft_settings_func_from_obj_call(c, led_hardware_get_plugin(h), n, NULL);
        
	return NFT_SUCCESS;
}


/**
 * register one LedHardware object to config-module
 */
NftResult led_settings_hardware_register(LedHardware *h)
{
	if(!h)
		NFT_LOG_NULL(NFT_FAILURE);

	return nft_settings_func_from_obj_set(led_settings_context(), h, (NftSettingsFromObjFunc *) _settings_from_hardware);
}


/**
 * unregister one LedHardware object from config-module
 */
NftResult led_settings_hardware_unregister(LedHardware *h)
{
	if(!h)
		NFT_LOG_NULL(NFT_FAILURE);
	
	/* unregister our config-creator */
	nft_settings_func_from_obj_unset(led_settings_context(), h);        
                          
        return NFT_SUCCESS;
}


/**
 * unlink hardware from settings tree
 */
NftResult led_settings_hardware_unlink(LedSettings *s, LedHardware *h)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_find_rec_by_obj(nft_settings_root((LedSettings *) s), h)))
        {
                NFT_LOG(L_ERROR, "no setup node for hardware %p found", h);
                return NFT_FAILURE;
        }
        
        nft_settings_node_destroy(n);
        
        return NFT_SUCCESS;
}


/**
 * get first toplevel hardware from config
 *
 * @param c LedSettings
 * @result first LedHardware or NULL upon error
 */
LedHardware *led_settings_hardware_get_first(LedSettings *c)
{
        if(!c)
                NFT_LOG_NULL(NULL);
        
        LedHardware *first = NULL;
        NftSettingsNodeIter *i;
        if(!(i = nft_settings_node_iter_ref(c, LED_HARDWARE_NAME, NULL)))
                return NULL;

        first = nft_settings_node_iter_obj_get (i);

        nft_settings_node_iter_unref(i);

        return first;
}


/**
 * get XML description of hardware as string
 *
 * @param c LedSettings
 * @param h LedHardware
 * @result string with XML dump
 * @note free() result when it's not needed anymore
 */
const char *led_settings_hardware_dump_xml(LedSettings *c, LedHardware *h)
{
        NftSettingsNode *n;
        if(!(n = nft_settings_node_find_rec_by_obj(nft_settings_root(c), h)))
        {
                NFT_LOG(L_WARNING, "Request to dump XML data from hardware without calling led_settings_hardware_register() first");
                return NULL;
        }

        return nft_settings_node_dump(n);
}


/**
 * create a new LedHardware from an XML hardware description
 *
 * @param xml string containing a complete & valid XML node
 * @result newly allocated LedHardware descriptor
 */
LedHardware *led_settings_hardware_parse_xml(const char *xml)
{
        if(!xml)
                NFT_LOG_NULL(NULL);
        
        NftSettingsNode *n;
        if(!(n = nft_settings_node_parse(xml)))
        {
                NFT_LOG(L_ERROR, "failed to parse <hardare> XML node");
                return NULL;
        }

        LedHardware *result;
        result = _settings_to_hardware(NULL, n);

        nft_settings_node_destroy(n);
        
        return result;
}


/**
 * @}
 */
