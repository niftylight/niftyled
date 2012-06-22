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
#include "niftyled-prefs_tile.h"



/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))



#define        LED_HARDWARE_PROP_NAME                  "name"
#define        LED_HARDWARE_PROP_PLUGIN                "plugin"
#define        LED_HARDWARE_PROP_ID                    "id"
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
 * It's used by nft_prefs_obj_to_node() etc. 
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
 * @note you shouldn't call this function directly. 
 * It's used by nft_prefs_obj_from_node() etc.
 */
static NftResult _prefs_to_hardware(LedPrefs *p, void **newObj, NftPrefsNode *n, void *userptr)
{
        if(!p || !newObj || !n)
                NFT_LOG_NULL(NFT_FAILURE);

            /** initial result */
            NftResult r = NFT_FAILURE;
    
                
        char *name = NULL;
        char *id = NULL;
        char *plugin_name = NULL;

        /* get hardware name */
        if(!(name = nft_prefs_node_prop_string_get(n, LED_HARDWARE_PROP_NAME)))
        {
                NFT_LOG(L_ERROR, "\"hardware\" has no name");
                goto _pth_end;
        }

        /* get plugin-name */
        if(!(plugin_name = nft_prefs_node_prop_string_get(n, LED_HARDWARE_PROP_PLUGIN)))
        {
                NFT_LOG(L_ERROR, "\"hardware\" has no \"plugin\" type");
                goto _pth_end;
        }

        /* get plugin-id */
        if(!(id = nft_prefs_node_prop_string_get(n, LED_HARDWARE_PROP_ID)))
        {
                NFT_LOG(L_ERROR, "\"hardware\" has no \"id\" type");
                goto _pth_end;
        }

        /* get stride */
        LedCount stride;
        if(!(nft_prefs_node_prop_int_get(n, LED_HARDWARE_PROP_STRIDE, (int *) &stride)))
        {
                NFT_LOG(L_WARNING, "\"hardware\" has no \"stride\". Using 0 as default.");
                stride = 0;
        }
        
        /* create new hardware object */
            LedHardware *h;
        if(!(h = led_hardware_new(name, plugin_name)))
        {
                    NFT_LOG(L_ERROR, "Failed to initialize \"%s\" from \"%s\" plugin.",
                                    name, plugin_name);
                goto _pth_end;
        }
        
        /* set stride */
        if(!(led_hardware_set_stride(h, stride)))
            {
                NFT_LOG(L_ERROR, "Failed to set stride (%d) of hardware \"%s\"",
                                stride, name);
                goto _pth_end;
        }
    
        /* set id */
        if(!(led_hardware_set_id(h, id)))
            {
                NFT_LOG(L_ERROR, "Failed to set ID \"%s\" of hardware \"%s\"",
                                id, name);
                goto _pth_end;
        }

    
        /* process child nodes */
        LedPrefsNode *child;
        for(child = nft_prefs_node_get_first_child(n);
            child;
            child = nft_prefs_node_get_next(child))
        {
                /* is child a tile node? */
                    if(led_prefs_is_tile_node(child))
                    {
                        if(!led_hardware_append_tile(h, led_prefs_tile_from_node(p, child)))
                        {
                                NFT_LOG(L_ERROR, "Failed to add \"tile\" to \"%s\". Aborting.",
                                                name);
                                goto _pth_end;
                        }
                        continue;
                }
                    /* is child a chain node? */
                    else if(led_prefs_is_chain_node(child))
                    {
                        LedChain *c;
                        if(!(c = led_prefs_chain_from_node(p, child)))
                        {
                                NFT_LOG(L_ERROR, "Failed to create \"chain\" node of hardware %s",
                                                name);
                                    goto _pth_end;
                        }
                        
                        if(!led_hardware_init(h, id, led_chain_get_ledcount(c), led_pixel_format_to_string(led_chain_get_format(c))))
                        {
                                NFT_LOG(L_WARNING, "Failed to initialize hardware \"%s\"", name);
                        }

                        led_chain_destroy(c);
                }
                else
                    {
                        NFT_LOG(L_WARNING, "Attempt to add \"%s\" node to tile. Only \"tile\" allowed. (Ignoring node)",
                                nft_prefs_node_get_name(child));
                        continue;
                }
        }
        
            /* everything pico bello */
        r = NFT_SUCCESS;
    
_pth_end:
        /* free strings */
        nft_prefs_free(id);
        nft_prefs_free(plugin_name);
        nft_prefs_free(name);

            /* newly created hardware object */
            *newObj = h;
    
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
