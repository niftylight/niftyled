/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2014 Daniel Hiepler <daniel@niftylight.de>
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
static NftResult _prefs_from_chain(NftPrefs * p, NftPrefsNode * n, void *obj,
                                   void *userptr)
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
                                           (char *) led_pixel_format_to_string
                                           (led_chain_get_format(c))))
                return NFT_FAILURE;


        /* add all LEDs in this chain */
        LedCount i;
        for(i = 0; i < led_chain_get_ledcount(c); i++)
        {
                /* generate prefs node from LED */
                NftPrefsNode *node;
                if(!
                   (node = led_prefs_led_to_node(p, led_chain_get_nth(c, i))))
                        return NFT_FAILURE;

                /* add node as child of this node */
                if(!nft_prefs_node_add_child(n, node))
                        return NFT_FAILURE;
        }


        return NFT_SUCCESS;
}


/**
 * Config-to-Object function.
 * Creates a LedHardware model from a prefs node
 * @note you shouldn't call this function directly
 * It's used by nft_prefs_obj_from_node() etc. 
 */
static NftResult _prefs_to_chain(LedPrefs * p, void **newObj,
                                 NftPrefsNode * n, void *userptr)
{
        if(!p || !newObj)
                NFT_LOG_NULL(NFT_FAILURE);




        /* LedCount of chain */
        int ledcount = 0;
        if(!nft_prefs_node_prop_int_get
           (n, LED_CHAIN_PROP_LEDCOUNT, &ledcount))
        {
                NFT_LOG(L_WARNING,
                        "chain has no \"%s\" property. Using %d as default.",
                        LED_CHAIN_PROP_LEDCOUNT, ledcount);
        }

        /* new chain */
        LedChain *c;

        /* get format of this chain */
        char *format;
        if(!(format = nft_prefs_node_prop_string_get(n,
                                                     LED_CHAIN_PROP_FORMAT)))
        {

#define LED_CHAIN_DEFAULT_FORMAT "RGB u8"

                NFT_LOG(L_WARNING,
                        "chain has no \"%s\" property. Using \"%s\" as default.",
                        LED_CHAIN_PROP_FORMAT, LED_CHAIN_DEFAULT_FORMAT);

                /* new chain (with default format) */
                if(!(c = led_chain_new((LedCount) ledcount, LED_CHAIN_DEFAULT_FORMAT)))
                {
                        NFT_LOG(L_ERROR,
                                "Failed to create new LedSetup object");
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
                        NFT_LOG(L_ERROR,
                                "Failed to create new LedSetup object");
                        return NFT_FAILURE;
                }
        }

        /* free string */
        nft_prefs_free(format);

        /* process child nodes (LEDs) */
        NftPrefsNode *child;
        LedCount i = 0;
        for(child = nft_prefs_node_get_first_child(n);
            child; child = nft_prefs_node_get_next(child))
        {
                /* check if node describes a Led object */
                if(!led_prefs_is_led_node(child))
                {
                        NFT_LOG(L_ERROR,
                                "\"chain\" may only contain \"%s\" children. Skipping \"%s\".",
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





/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/


/**
 * register "chain" prefs class (called once for initialization)
 */
NftResult _prefs_chain_class_register(NftPrefs * p)
{
        if(!p)
                NFT_LOG_NULL(NFT_FAILURE);

        return nft_prefs_class_register(p, LED_CHAIN_NAME, &_prefs_to_chain,
                                        &_prefs_from_chain);
}






/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

/**
 * check if NftPrefsNode represents a chain object
 *
 * @param n LedPrefsNode
 * @result true if node represents a chain object, false otherwise
 */
bool led_prefs_is_chain_node(LedPrefsNode * n)
{
        return (strcmp(nft_prefs_node_get_name(n), LED_CHAIN_NAME) == 0);
}


/**
 * generate LedChain from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param n LedPrefsNode 
 * @result newly created LedChain
 */
LedChain *led_prefs_chain_from_node(LedPrefs * p, LedPrefsNode * n)
{
        if(!p || !n)
                NFT_LOG_NULL(NULL);

        /* check if node is of expected class */
        if(!led_prefs_is_chain_node(n))
        {
                NFT_LOG(L_ERROR,
                        "got wrong LedPrefsNode class. Expected \"%s\" but got \"%s\"",
                        LED_CHAIN_NAME, nft_prefs_node_get_name(n));
                return NULL;
        }

        return nft_prefs_obj_from_node(p, n, NULL);
}


/**
 * generate LedPrefsNode from LedChain object
 *
 * @param p LedPrefs context
 * @param c LedChain object 
 * @result newly created LedPrefsNode 
 */
LedPrefsNode *led_prefs_chain_to_node(LedPrefs * p, LedChain * c)
{
        if(!p || !c)
                NFT_LOG_NULL(NULL);

        return nft_prefs_obj_to_node(p, LED_CHAIN_NAME, c, NULL);
}




/**
 * @}
 */
