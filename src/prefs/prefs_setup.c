/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2013 Daniel Hiepler <daniel@niftylight.de>
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
 * @file prefs_setup.c
 */


/**
 * @addtogroup prefs_setup 
 * @{
 */

#include "niftyled-prefs_setup.h"
#include "niftyled-prefs_hardware.h"




/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/


/**
 * Object-to-Config function. 
 * Creates a config-node (and subnodes) from a LedSetup model
 * @param c the current preferences context
 * @param n a freshly created prefs node that this function should fill with properties of obj
 * @param obj object of this class where preferences should be generated from
 * @result NFT_SUCCESS if everything went fine, NFT_FAILURE otherwise
 * @note you shouldn't call this function directly
 * It's used by nft_prefs_obj_to_node() etc. 
 */
static NftResult _prefs_from_setup(NftPrefs * p, NftPrefsNode * n, void *obj,
                                   void *userptr)
{
        if(!p || !n || !obj)
                NFT_LOG_NULL(NFT_FAILURE);

        /* "Setup" object */
        LedSetup *s = obj;

        /* process all "hardware" objects */
        LedHardware *h;
        for(h = led_setup_get_hardware(s); h;
            h = led_hardware_list_get_next(h))
        {
                /* generate prefs for each hardware node */
                NftPrefsNode *node;
                if(!
                   (node =
                    nft_prefs_obj_to_node(p, LED_HARDWARE_NAME, h, NULL)))
                        return NFT_FAILURE;

                /* add hardware to this setup */
                if(!(nft_prefs_node_add_child(n, node)))
                        return NFT_FAILURE;

        }

        return NFT_SUCCESS;
}


/**
 * Config-to-Object function.
 * Creates a LedSetup model from a prefs node
 * @note you shouldn't call this function directly
 * It's used by nft_prefs_obj_from_node() etc. 
 */
static NftResult _prefs_to_setup(LedPrefs * p, void **newObj,
                                 NftPrefsNode * n, void *userptr)
{
        if(!p || !newObj)
                NFT_LOG_NULL(NFT_FAILURE);


        /* new setup */
        LedSetup *s;

        /* new setup */
        if(!(s = led_setup_new()))
        {
                NFT_LOG(L_ERROR, "Failed to create new LedSetup object");
                return NFT_FAILURE;
        }

        /* walk all child nodes and process them */
        NftPrefsNode *child;
        for(child = nft_prefs_node_get_first_child(n);
            child; child = nft_prefs_node_get_next(child))
        {
                /* check if node describes a LedHardware object */
                if(!led_prefs_is_hardware_node(child))
                {
                        NFT_LOG(L_ERROR,
                                "\"%s\" object may only contain \"%s\" children but got \"%s\"",
                                LED_SETUP_NAME, LED_HARDWARE_NAME,
                                nft_prefs_node_get_name(child));
                        goto _pts_error;
                }

                /* call toObj function of child node */
                LedHardware *hw;
                if(!(hw = nft_prefs_obj_from_node(p, child, userptr)))
                {
                        NFT_LOG(L_ERROR,
                                "Failed to create object from preference node");
                        return NFT_FAILURE;
                }

                /* register first hardware */
                if(!led_setup_get_hardware(s))
                {
                        led_setup_set_hardware(s, hw);
                }
                /* attach hardware to list */
                else
                {
                        if(!led_hardware_list_append_head
                           (led_setup_get_hardware(s), hw))
                        {
                                NFT_LOG(L_ERROR,
                                        "Failed to append LedHardware as sibling");
                                return NFT_FAILURE;
                        }
                }
        }

        /* save new setup-object to "newObj" pointer */
        *newObj = s;

        return NFT_SUCCESS;


_pts_error:
        led_setup_destroy(s);
        return NFT_FAILURE;
}



/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/

/**
 * register "setup" prefs class (called once for initialization)
 */
NftResult _prefs_setup_class_register(NftPrefs * p)
{
        return nft_prefs_class_register(p, LED_SETUP_NAME, &_prefs_to_setup,
                                        &_prefs_from_setup);
}


/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/


/**
 * generate LedSetup from LedPrefsNode
 *
 * @param p LedPrefs context
 * @param n LedPrefsNode 
 * @result newly created LedSetup
 */
LedSetup *led_prefs_setup_from_node(LedPrefs * p, LedPrefsNode * n)
{
        if(!p || !n)
                NFT_LOG_NULL(NULL);

        /* check if node is of expected class */
        if(strcmp(nft_prefs_node_get_name(n), LED_SETUP_NAME) != 0)
        {
                NFT_LOG(L_ERROR,
                        "got wrong LedPrefsNode class. Expected \"%s\" but got \"%s\"",
                        LED_SETUP_NAME, nft_prefs_node_get_name(n));
                return NULL;
        }

        return nft_prefs_obj_from_node(p, n, NULL);
}


/**
 * generate LedPrefsNode from LedSetup object
 *
 * @param p LedPrefs context
 * @param s LedSetup object 
 * @result newly created LedPrefsNode 
 */
LedPrefsNode *led_prefs_setup_to_node(LedPrefs * p, LedSetup * s)
{
        if(!p || !s)
                NFT_LOG_NULL(NULL);

        return nft_prefs_obj_to_node(p, LED_SETUP_NAME, s, NULL);
}


/**
 * @}
 */
