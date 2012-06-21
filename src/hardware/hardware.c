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
 * @file hardware.c
 */


/**
 * @addtogroup hardware
 * @{
 * @todo (optionally) run plugin-callbacks in threads
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

//#include <niftyled.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <alloca.h>

#if HAVE_DLFCN_H
#include <dlfcn.h>
#else
#error We need dlfcn.h (-ldl) to use runtime loadable plugins
#endif

#include "niftyled-version.h"
#include "niftyled-hardware.h"
#include "_tile.h"
#include "_chain.h"




/** model of LED-hardware to interface with LEDs */
struct _LedHardware
{
        /** plugin handle as returned by dlopen() */
        void *libhandle;
        /** descriptor that has been provided by plugin */
        LedHardwarePlugin *plugin;
        /** 
         * space for private data used by the plugin internally
         * (this is optional and thus may be NULL) 
         */
        void *plugin_privdata;
        /** space for private user data */
        void *privdata;
        /** properties of this hardware */
        struct
        {
            /** instance name of this hardware */
            char name[1024];
            /** unique id that defines one out of multiple hardwares of the 
             * same family */
            char id[4096];
            /** if != FALSE device has been initialized successfully */
            char initialized;
            /** amount of LEDs controlled by this hardware (set while hw init) */
            LedCount ledcount;
            /** pixelformat name (set while hw init) */
            char pixelformat[1024];
            /** advance this many LEDs to reach the next LED when sending */
            LedCount stride;
        }props;
	/** relations of this hardware */
	struct
	{
            /** chain of this hardware-plugin (holds all currently configured 
                LEDs this plugin can control) */
            LedChain *chain;
            /** first LedTile registered to this hardware */
            LedTile *first_tile;
            /** previous sibling */
            LedHardware *prev;
            /** next sibling */
            LedHardware *next;
	}relation;
};


/* search these paths to try loading the plugin */
static const char *_prefixes[] = { "/lib", "/usr/lib", "/usr/local/lib" };


/******************************************************************************
 **************************** STATIC FUNCTIONS ********************************
 ******************************************************************************/

/** extract plugin-name from filename */
static const char *_familyname_from_filename(const char *filename)
{
/** file extension for shared libraries */
#define LED_HARDWARE_FILE_EXTENSION ".so"
/** suffix one hardware plugin has to have (e.g. foobar-hardware.so) */
#define LED_HARDWARE_FILE_SUFFIX "-hardware." LED_HARDWARE_FILE_EXTENSION
/** maximum size of hardware-plugin filename */
#define LED_HARDWARE_LIBNAME_MAXSIZE     1024
        
        char *suffix;
        if(!(suffix = strstr(filename, LED_HARDWARE_FILE_SUFFIX)))
        {       
                /*NFT_LOG(L_WARNING, "\"%s\" has no \"%s\" suffix", 
                        filename, LED_HARDWARE_FILE_SUFFIX);*/
                return NULL;
        }

        /* copy filename */
        static char name[LED_HARDWARE_LIBNAME_MAXSIZE];
        strncpy(name, filename, suffix-filename);

        /* terminate string */
        name[suffix-filename] = '\0';
        
        return name;
}


/** build full library path from prefix + filename */
static const char *_lib_path(const char *prefix, const char *filename)
{
        static char path[4096];

        if(prefix)
                snprintf(path, sizeof(path), "%s/%s", prefix, filename);
        else
                return filename;
        
        return path;
}


/** load hardware plugin */
static LedHardware *_load_plugin(const char *name, const char *family)
{
        
/** maximum size of hardware-plugin filename */
#define LED_HARDWARE_LIBNAME_MAXSIZE     1024
/** symbol name of "hardware_descriptor" (LedHardwarePlugin) structure that an hardware-plugin has to provide */
#define LED_HARDWARE_DESCRIPTOR  "hardware_descriptor"

        
     
        /* build library-name from hardware-family */
        char *libname;
        if(!(libname = alloca(LED_HARDWARE_LIBNAME_MAXSIZE)))
        {
                NFT_LOG_PERROR("alloca");
                return NULL;
        }  
        
        if(snprintf(libname, LED_HARDWARE_LIBNAME_MAXSIZE, "%s/%s-hardware.so", PLUGINDIR, family) < 0)
        {
                NFT_LOG_PERROR("snprintf");
                return NULL;
        }

        
        /* search all prefixes for plugin library */
        void *handle;
        int i;
        for(i=0; i < sizeof(_prefixes)/sizeof(char *); i++)
        {
                NFT_LOG(L_NOISY, "\tTrying to load \"%s\"", _lib_path(_prefixes[i], libname));
                if((handle = dlopen(_lib_path(_prefixes[i], libname), RTLD_LAZY)))
                        break;
        }

        /* check for LD_LIBRARY_PATH override */
        if(!handle)
        {
                snprintf(libname, LED_HARDWARE_LIBNAME_MAXSIZE,  "%s-hardware.so", family);
                NFT_LOG(L_NOISY, "\tTrying to load \"%s\"", libname);
                if(!(handle = dlopen(libname, RTLD_LAZY)))
                {
                        NFT_LOG(L_ERROR, "Failed to find libfile \"%s\"", libname);
                        return NULL;
                }
        }

        
        /* get plugin descriptor from newly loaded library */
        LedHardwarePlugin *plugin;
        if(!(plugin = dlsym(handle, LED_HARDWARE_DESCRIPTOR)))
        {
                NFT_LOG(L_ERROR, "Plugin doesn't provide descriptor symbol: \"%s\"", LED_HARDWARE_DESCRIPTOR);
                dlclose(handle);
                return NULL;
        }

        
        /* check plugin API major-version */
	if(plugin->api_major != HW_PLUGIN_API_MAJOR_VERSION)
	{
		NFT_LOG(L_ERROR, "Plugin has been compiled against major version %d of %s, we are version %d. Not loading plugin.", 
		        plugin->api_major, PACKAGE_NAME, HW_PLUGIN_API_MAJOR_VERSION);
		dlclose(handle);
		return NULL;
	}

	/* check plugin API minor-version */
        if(plugin->api_minor != HW_PLUGIN_API_MINOR_VERSION)
        {
                NFT_LOG(L_WARNING, "Plugin compiled against %d of %s, we are version %d. Continue at own risk.", 
                        plugin->api_minor, PACKAGE_NAME, HW_PLUGIN_API_MINOR_VERSION);
        }

                
        /* prepare hardware descriptor */
        LedHardware *a;
        if(!(a = calloc(1, sizeof(LedHardware))))
        {
                NFT_LOG_PERROR("calloc");
                dlclose(handle);
                return NULL;
        }

        /* save plugin descriptor in hardware descriptor */
        a->plugin = plugin;
        /* save libhandle */
        a->libhandle = handle;
        /* copy instance-name */
        strncpy(a->props.name, name, sizeof(a->props.name));
	
        return a;
}



/** unload hardware plugin */
static void _unload_plugin(LedHardware *h)
{                                
        /* close library */
        NFT_LOG(L_DEBUG, "Unloading plugin instance \"%s\" (%s)", h->props.name, h->props.id);
        if(h->libhandle)
                dlclose(h->libhandle);
}


/** try to re-initialize forcefully disconnected hardware */
static void _reinitialize(LedHardware *h)
{
        /** 
         * @todo make this dependant on some "try_reconnect" 
         * config-property 
         */

        /* got a pixelformat? */
        if(h->props.pixelformat[0] == '\0')
                return;
        
        /** try to initialize */
        if(!led_hardware_init(
                h, 
                h->props.id, 
                h->props.ledcount, 
                h->props.pixelformat))
        {
                NFT_LOG(L_WARNING, "Attempt to re-initialize %s failed", h->props.name);
        }
}



/******************************************************************************
 ****************************** API FUNCTIONS *********************************
 ******************************************************************************/

/**
 * create new hardware 
 * 
 * The lifecycle of a LedHardware usually looks like this:
 *
 * led_hardware_new()  // create new hardware <br>
 * [ led_hardware_*_set() ]  // optionally set hardware properties <br>
 * [...] <br>
 * led_hardware_init()  // initialize hardware <br>
 * [...] <br>
 * [ led_hardware_*_set/get() ]  // optionally set/get hardware properties <br>
 * [...] <br>
 * do <br>
 * { <br>
 *      led_hardware_buffer_send()  // send greyscale-values to hardware <br>
 *      [...] <br>
 *      led_hardware_buffer_show()  // latch buffer <br>
 * } <br>
 * [...] <br>
 * [ led_hardware_deinit() ]  // deinitialize hardware <br>
 * [...] <br>
 * led_hardware_destroy()  // free resources <br>
 *
 * @param name unique instance name
 * @param plugin_name name of installed & working hardware plugin
 * @result @ref LedHardware descriptor or NULL
 */
LedHardware *led_hardware_new(const char *name, const char *plugin_name)
{                
        /* try to load hardware-plugin */
        NFT_LOG(L_DEBUG, "Trying to create new hardware \"%s\" (plugin: \"%s\")", name, plugin_name);
        
        LedHardware *h;
        if(!(h = _load_plugin(name, plugin_name)))
                return NULL;
        
        NFT_LOG(L_DEBUG, "Loaded new plugin instance \"%s\"", h->props.name); 

        /* print info about loaded plugin */
        led_hardware_plugin_print(h->plugin, L_INFO);
	
        /* initialize plugin */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h,plugin_init))
        {
                if(!(h->plugin->plugin_init(&(h->plugin_privdata), h)))
                {
                        NFT_LOG(L_ERROR, "Plugin initialization function failed");
                        _unload_plugin(h);
                        return NULL;
                }
        }
        
        /* register to current LedConfCtxt */
        //~ if(!led_settings_hardware_register(h))
        //~ {
                //~ _unload_plugin(h);
                //~ return NULL;
        //~ }
        
        return h;
}



/**
 * quit usage of hardware and free all its resources 
 * @note This will also call @ref led_hardware_deinit()
 * @param h @ref LedHardware descriptor
 */
void led_hardware_destroy(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL();
        
        NFT_LOG(L_DEBUG, "Destroying hardware \"%s\" (family: \"%s\" id: \"%s\")", h->props.name, h->plugin->family, h->props.id);

        /* unregister from config context */
        //~ led_settings_hardware_unregister(h);       

        /* unlink from linked-list of siblings */
	if(h->relation.next)
		h->relation.next->relation.prev = h->relation.prev;
	if(h->relation.prev)
		h->relation.prev->relation.next = h->relation.next;
        
        /* deinitialize hardware */
        led_hardware_deinit(h);

        /* destroy tile(s) */
        led_tile_list_destroy(led_hardware_get_tile(h));
        
        /* destroy chain */
        chain_destroy(h->relation.chain);
        
        /* plugin deinitialize */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h,plugin_deinit))
        {
                h->plugin->plugin_deinit(h->plugin_privdata);
        }
        
        /* unload plugin */
        _unload_plugin(h);

        
        /* free descriptor */
        free(h);
}


/**
 * destroy hardware and all it's siblings sequentially
 *
 * @param first first LedHardware
 */
void led_hardware_destroy_list(LedHardware *first)
{       
         if(!first)
                return;

        if(first->relation.next)
                led_hardware_destroy_list(first->relation.next);

        led_hardware_destroy(first);
        
        return;
}


/**
 * initialize this piece of hardware
 *
 * @note remember to call @ref led_hardware_deinit() and @ref led_hardware_destroy().
 * You can use @ref led_hardware_init() to re-initialize the hardware unless it has
 * not been destroyed, yet.
 *
 * @param h @ref LedHardware descriptor
 * @param id Hardware ID of this plugin (e.g. /dev/ttyS0 or something)
 * @param ledcount amount of leds currently connected to this hardware
 * @param pixelformat printable name of the requested pixelformat
 * (s. http://www.gegl.org/babl/ for valid formats)
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_init(LedHardware *h, const char *id, LedCount ledcount, const char *pixelformat)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* hardware already initialized? */
        if(h->props.initialized)
        {
                NFT_LOG(L_WARNING, "Attempt to initialize already initialized \"%s\" (%s)", h->props.name, id);
                return NFT_SUCCESS;
        }

        /** if we are re-initializing, we already have a chain */
        if(!h->relation.chain)
        {
                /** initialize LedChain of this hardware */
                if(!(h->relation.chain = led_chain_new(ledcount, pixelformat)))
                {
                        NFT_LOG(L_ERROR, "Failed to create chain. Initialization failed");
                        return NFT_FAILURE;
                }

                /* register hardware with chain */
                chain_set_parent_hardware(h->relation.chain, h);
        }
        
        /* save ledcount */
        h->props.ledcount = ledcount;

        /* save pixelformat */
        strncpy(h->props.pixelformat, pixelformat, sizeof(h->props.pixelformat));
        
        /* initialize hardware */
        NFT_LOG(L_DEBUG, "Initializing \"%s\" (%s)...", h->props.name, id);
        
        /* no init function */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h,hw_init))
        {
                /* initialize */
                if(!(h->plugin->hw_init(h->plugin_privdata, id)))
                {
                        NFT_LOG(L_ERROR, "Failed to initialize hardware");
                        return NFT_FAILURE;
                }   
        }

        /* mark hardware as "initialized" */
        h->props.initialized = TRUE;

        /* ID might have changed after initializing (when using a wildcard id) */
        NFT_LOG(L_INFO, "\t\033[1mHardware ID:\033[0m \"%s\"\n", led_hardware_get_id(h));

        /* set id in model */
        if(!led_hardware_set_id(h, led_hardware_get_id(h)))
        {
                NFT_LOG(L_ERROR, "Failed to set hardware id");
                return NFT_FAILURE;
        }
        
        /* set ledcount */
        if(!led_hardware_set_ledcount(h, ledcount))
        {
                NFT_LOG(L_WARNING, "Hardware \"%s\" (%s) didn't accept our ledcount (%d). Trying to adapt.", 
                        led_hardware_get_name(h), led_hardware_get_id(h), ledcount);

                if(!led_chain_set_ledcount(h->relation.chain, ledcount))
                {
                        NFT_LOG(L_ERROR, "Failed to change chain-length");
                        return NFT_FAILURE;
                }
        }
        
        return NFT_SUCCESS;
}


/**
 * Deinitialize this piece of hardware
 *
 * @param h @ref LedHardware descriptor
 */
void led_hardware_deinit(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL();

        if(!h->props.initialized)
        {
                NFT_LOG(L_DEBUG, "Attempt to deinitialize plugin %s - \"%s\" (%s) that wasn't initialized.", h->plugin->family, h->props.name, h->props.id);
                return;
        }
        
        /* deinitialize hardware */
        NFT_LOG(L_DEBUG, "Deinitializing \"%s\" (%s)", h->props.name, h->props.id);
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h,hw_deinit))
                h->plugin->hw_deinit(h->plugin_privdata);
       
        /* mark hardware as "deinitialized" */
        h->props.initialized = FALSE;
}


/**
 * Get initialization-state of hardware
 *
 * @param h @ref LedHardware descriptor
 * @result NFT_SUCCESS if hardware was initialized, NFT_FAILURE otherwise
 */
NftResult led_hardware_is_initialized(LedHardware *h)
{
        if(!h || !h->props.initialized)
                return NFT_FAILURE;

        return NFT_SUCCESS;
}


/**
 * get plugin descriptor of this hardware
 *
 * @param h @ref LedHardware descriptor
 * @result LedHardwarePlugin descriptor or NULL upon error
 */
LedHardwarePlugin *led_hardware_get_plugin(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->plugin;
}


/**
 * get pointer that plugin registered as private-data
 *
 * @param h @ref LedHardware descriptor 
 * @result pointer to private data
 */
void *led_hardware_get_plugin_privdata(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->plugin_privdata;
}


/**
 * get ID of this hardware
 *
 * @note if the plugin provides a getter, the result of this is returned.
 * otherwise, the value from the model (e.g. read by config-file) is returned.
 * Thus the ID can be read directly from the hardware if it can provide it.
 *
 * @param h the LedHardware to get id from as string or NULL
 */
const char *led_hardware_get_id(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        /* don't get id from plugin if we aren't initialized */
        if(!led_hardware_is_initialized(h))
                return h->props.id;
        
        /* does plugin provide get-operation? */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
                return h->props.id;

        /* get operation */
        LedPluginObjData get_id;
        if(h->plugin->get(h->plugin_privdata, LED_HW_ID, &get_id))
        {
                /* buffer id from hardware */
                strncpy(h->props.id, get_id.id, sizeof(h->props.id));
                NFT_LOG(L_DEBUG, "Got id \"%s\" from %s", h->props.id, h->props.name);
        }
        else
                NFT_LOG(L_WARNING, "Plugin failed to deliver an id.");
        
        
        return h->props.id;
}


/**
 * set ID of this hardware (before calling @ref led_hardware_init )
 *
 * @note if the plugin provides a setter, the result of this is returned.
 * otherwise, the value is just set in the model.
 *
 * @param h the LedHardware to set id from
 * @param id ID-string defining this piece of physical hardware uniquely
 */
NftResult led_hardware_set_id(LedHardware *h, const char *id)
{
        if(!h || !id)
                NFT_LOG_NULL(NFT_FAILURE);

        NFT_LOG(L_DEBUG, "Setting id of %s to %s", h->props.name, id);

        /* just save id in model if we aren't initialized */
        if(led_hardware_is_initialized(h))
        {
                /* does plugin provide operations? */
                if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
                {
                        /* set id operation */
                        LedPluginObjData set_id = { .id = id };
                        if(!h->plugin->set(h->plugin_privdata, LED_HW_ID, &set_id))
                        {      
                                NFT_LOG(L_ERROR, "Setting ID to plugin failed.");
                                return NFT_FAILURE;
                        }  
                }
                else
                {
                        NFT_LOG(L_WARNING, "Plugin family %s has no set-handler.", h->props.name);
                }
        }
        
        
        /** save id in model */
        if(h->props.id != id)
                strncpy(h->props.id, id, sizeof(h->props.id));
        
        return NFT_SUCCESS;
}


/**
 * set led-stride of hardware
 *
 * @param h hardware to set stride
 * @param stride the amount of LEDs to skip
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_stride(LedHardware *h, LedCount stride)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        h->props.stride = stride;
        
        return NFT_SUCCESS;
}


/**
 * get stride of hardware
 *
 * @param h hardware to get stride from
 * @result stride or 0 upon error
 */
LedCount led_hardware_get_stride(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(0);

        return h->props.stride;
}


/**
 * get currently registered chain of this hardware
 *
 * @param h LedHardware descriptor
 * @result LedChain descriptor
 */
LedChain *led_hardware_get_chain(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        if(!h->relation.chain)
                NFT_LOG(L_DEBUG, "Hardware has no chain. Initialize to create chain.");
        
        return h->relation.chain;
}


/**
 * set LedTile descriptor registered to this hardware
 *
 * @param h LedHardware descriptor
 * @param t LedTile descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_tile(LedHardware *h, LedTile *t)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* register tile with hardware */
        h->relation.first_tile = t;
        
        /* register hardware with tile */
        return tile_set_parent_hardware(t, h);
}


/**
 * append LedTile to list of registered tile to this hardware
 *
 * @param h LedHardware descriptor
 * @param t LedTile descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */ 
NftResult led_hardware_append_tile(LedHardware *h, LedTile *t)
{
	if(!h->relation.first_tile)
		return led_hardware_set_tile(h, t);
	
	if(!(led_tile_append_sibling(h->relation.first_tile, t)))
    	{
		NFT_LOG(L_ERROR, "Failed to append tile %p to hardware \"%s\"", 
		        	t, led_hardware_get_name(h));
		return NFT_FAILURE;
	}

    	return tile_set_parent_hardware(t, h);
}

/**
 * get LedTile descriptor registered to this hardware
 *
 * @param h LedHardware descriptor
 * @result LedTile descriptor
 */
LedTile *led_hardware_get_tile(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->relation.first_tile;
}


/**
 * get name of this hardware
 *
 * @param h the LedHardware to get the name from
 * @result const string containing name (or maybe NULL)
 */
const char *led_hardware_get_name(LedHardware *h)
{
	if(!h)
		NFT_LOG_NULL(NULL);

	return h->props.name;
}


/**
 * set name of this hardware
 *
 * @param h the LedHardware to set name from
 * @param name new name of this hardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_name(LedHardware *h, const char *name)
{
	if(!h || !name)
		NFT_LOG_NULL(NFT_FAILURE);

	strncpy(h->props.name, name, sizeof(h->props.name));
	
	return NFT_SUCCESS;
}


/**
 * set amount of LEDs connected to hardware
 *
 * @param h @ref LedHardware descriptor
 * @param leds amount of LEDs
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_ledcount(LedHardware *h, LedCount leds)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        NFT_LOG(L_DEBUG, "Setting ledcount of %s (%s) to %d", h->props.name, h->props.id, leds);


        /* set operation */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h,set))
        {
                LedPluginObjData set_ledcount = { .ledcount = leds };
                
                if(!(h->plugin->set(h->plugin_privdata, LED_HW_LEDCOUNT, &set_ledcount)))
                {
                        NFT_LOG(L_ERROR, "Plugin %s (\"%s\") failed ledcount (%d) event", h->props.name, h->props.id, leds);
                        return NFT_FAILURE;
                }
        }
        else
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no set-handler.", h->props.name);
        }
        
        /* save in model */
        if(!(led_chain_set_ledcount(h->relation.chain, leds)))
                return NFT_FAILURE;
                                
        return NFT_SUCCESS;
}


/**
 * get amount of LEDs connected to hardware
 *
 * @param h @ref LedHardware descriptor
 * @result amount of LEDs currently configured (or 0 upon error)
 */
LedCount led_hardware_get_ledcount(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(0);

        LedCount ledcount = led_chain_get_ledcount(h->relation.chain);
        
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h,get))
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no get-handler.", h->props.name);
                return ledcount;
        }
        
        LedPluginObjData get_ledcount;
        
        if(!(h->plugin->get(h->plugin_privdata, LED_HW_LEDCOUNT, &get_ledcount)))
        {
                NFT_LOG(L_WARNING, "Plugin %s (\"%s\") failed to deliver ledcount. Continuing with current chainlength: %d", h->props.name, h->props.id, ledcount);
                return ledcount;
        }

        if(get_ledcount.ledcount != ledcount)
        {
                NFT_LOG(L_WARNING, "Plugin silently changed ledcount! I'm confused, this is a bug!");
                return 0;
        }
        

        
        /* return value from model if there's no plugin-handler */
        return ledcount;
}


/**
 * count LEDs connected to hardware and it's siblings
 *
 * @param h First LedHardware
 * @result sum of LEDs registered to all sibling hardware-interfaces in total or < 0 upon error
 */
LedCount led_hardware_get_list_ledcount(LedHardware *h)
{

        if(!h)
                NFT_LOG_NULL(-1);

        
	/* count total LEDs on hardware adapters */
        LedCount res = 0;
	LedHardware *t;
        for(t = h; t; t=t->relation.next)
        {
                res += led_hardware_get_ledcount(t);
                NFT_LOG(L_INFO, "Hardware \"%s\" has %d LEDs", led_hardware_get_name(t), led_hardware_get_ledcount(t));
        }

        return res;
}


/**
 * set gain of a LED connected to hardware
 *
 * @param h @ref LedHardware descriptor
 * @param pos position of LED in chain (if value is negative, process all LEDs in chain)
 * @param gain gain setting (ranging from 0 for lowest gain to 2^32 for highest gain)
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_gain(LedHardware *h, LedCount pos, LedGain gain)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        NFT_LOG(L_NOISY, "Setting gain of LED %d from %s (%s) to %hu", pos, h->props.name, h->props.id, gain);


        /* set operation plugin */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h,set))
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no set-handler.", h->props.name);
                return NFT_SUCCESS;
        }
        
        LedPluginObjData set_gain = { .gain.pos = pos, .gain.value = gain };
        
        if(!(h->plugin->set(h->plugin_privdata, LED_HW_GAIN, &set_gain)))
        {
                NFT_LOG(L_ERROR, "Plugin %s (\"%s\") failed to set gain (%hu) for LED %d", h->props.name, h->props.id, gain, pos);
                return NFT_FAILURE;
        }               
        
        return NFT_SUCCESS;
}


/**
 * get gain of a LED connected to hardware
 *
 * @param h @ref LedHardware descriptor
 * @param pos position of LED in chain (if value is negative, process all LEDs in chain)
 * @result LedGain or 0 on failure.
 */
LedGain led_hardware_get_gain(LedHardware *h, LedCount pos)
{
        if(!h)
                NFT_LOG_NULL(0);

        NFT_LOG(L_NOISY, "Getting gain of LED %d from %s (%s)", pos, h->props.name, h->props.id);

                
        /* set operation plugin */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h,get))
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no get-handler.", h->props.name);
                return 0;
        }
        
        LedPluginObjData get_gain = { .gain.pos = pos };
        
        if(!(h->plugin->get(h->plugin_privdata, LED_HW_GAIN, &get_gain)))
        {
                NFT_LOG(L_ERROR, "Plugin %s (\"%s\") failed to get gain at %d", h->props.name, h->props.id, pos);
                return NFT_FAILURE;
        }

        return get_gain.gain.value;
}


/**
 * get private userdata previously set by led_hardware_set_privdata()
 *
 * @param h a LedHardware
 * @result pointer to private userdata
 */
void *led_hardware_get_privdata(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->privdata;
}


/**
 * associate private userdata pointer with hardware
 *
 * @param h a LedHardware
 * @param privdata pointer to private userdata
 * @result NFT_SUCCESS or NFT_FAILURE upon failure
 */
NftResult led_hardware_set_privdata(LedHardware *h, void *privdata)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        h->privdata = privdata;

        return NFT_SUCCESS;
}


/**
 * print debug-info for hardware
 *
 * @param h a LedHardware
 * @param l minimum current loglevel so tile gets printed
 */
void led_hardware_print(LedHardware *h, NftLoglevel l)
{
        if(!h)
                NFT_LOG_NULL();
       
        NFT_LOG(l,
                "Hardware: %p (\"%s\" id:%s [%s] stride:%d siblings:%d",
                h,
                h->props.name,
                h->props.id,
                h->props.initialized ? "initialized" : "not initialized",
                h->props.stride,
                led_hardware_get_sibling_count(h));
}


/**
 * set sibling hardware to this hardware
 */
NftResult led_hardware_set_sibling(LedHardware *h, LedHardware *sibling)
{
	if(!h)
		NFT_LOG_NULL(NFT_FAILURE);


        /* don't attach to ourself */
        if(h == sibling)
        {
                NFT_LOG(L_ERROR, "Attempt to make us our own sibling");
                return NFT_FAILURE;
        }
        
        /* don't overwrite existing sibling */
        if(h->relation.next && sibling != NULL)
        {
                NFT_LOG(L_ERROR, "Hardware already has sibling. Must be removed before setting new one.");
                return NFT_FAILURE;
        }
        

        /* register next */
	h->relation.next = sibling;

        /* register previous */
	if(sibling)
		sibling->relation.prev = h;
	
	return NFT_SUCCESS;
}


/**
 * append hardware to last sibling of head
 */
NftResult led_hardware_append_sibling(LedHardware *head, LedHardware *sibling)
{
        if(!head)
                NFT_LOG_NULL(NFT_FAILURE);

        /* don't try to append a sibling twice */
        if(head->relation.next == sibling)
                return TRUE;
        
        LedHardware *last;
        for(last = head; 
            last->relation.next;
            last = last->relation.next);
        
        return led_hardware_set_sibling(last, sibling);
        
}


/**
 * get nth sibling of this hardware
 */
LedHardware *led_hardware_get_nth_sibling(LedHardware *h, int n)
{
        if(!h)
                return NULL;

        if(n == 0)
                return h;

        return led_hardware_get_nth_sibling(h->relation.next, n-1);
}


/**
 * get next sibling
 */
LedHardware *led_hardware_get_next_sibling(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->relation.next;
}


/**
 * get previous sibling
 */
LedHardware *led_hardware_get_prev_sibling(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->relation.prev;
}


/**
 * get amount of siblings this hardware has left
 */
int led_hardware_get_sibling_count(LedHardware *h)
{
        LedHardware *t = h;
        int i;
        for(i = 0; t; i++)
                t = t->relation.next;

        return i-1;
}


/**
 * print debug-info of hardware-plugin
 *
 * @param p a LedHardwarePlugin
 * @param l minimum current loglevel so tile gets printed
 */
void led_hardware_plugin_print(LedHardwarePlugin *p, NftLoglevel l)
{
        if(!p)
                NFT_LOG_NULL();
       
        NFT_LOG(l,
                "Hardware %p\n"
                "\t\033[1mPlugin family:\033[0m %s\n"
                "\t\033[1mAPI version:\033[0m %d.%d.%d\n"
                "\t\033[1mPlugin version:\033[0m %d.%d.%d\n"
                "\t\033[1mLicense:\033[0m %s\n"
                "\t\033[1mAuthor:\033[0m %s\n"
                "\t\033[1mDescription:\033[0m %s\n"
                "\t\033[1mURL:\033[0m %s",
                p,
                p->family,
                p->api_major, p->api_minor, p->api_micro, 
                p->major_version, p->minor_version, p->micro_version, 
                (p->license ? p->license : 
	                 "check documentation or sourcecode"),
                (p->author ? p->author : "-"), 
                (p->description ? p->description : "-"),
                (p->url ? p->url : "-")
                );
}


/**
 * get amount of available plugins, use this to iterate through all
 * installed plugins (e.g. led_hardware_plugin_name())
 *
 * @result amount of installed hardware plugins
 */
int led_hardware_get_plugin_count()
{       
        int amount = 0;
        
        
        /* count plugins in all possible dirs */
        int i;
        for(i=0; i < sizeof(_prefixes)/sizeof(char *); i++)
        {
                DIR *dir;
                if(!(dir = opendir(_lib_path(_prefixes[i], PLUGINDIR))))
                {
                        NFT_LOG(L_DEBUG, "Failed to open dir \"%s\" (%s)",
                                _lib_path(_prefixes[i], PLUGINDIR), strerror(errno));
                        continue;
                }

                struct dirent *entry;
                while((entry = readdir(dir)))
                {
                        /* extract pluginname from filename */
                        const char *familyname;
                        if(!(familyname = _familyname_from_filename(entry->d_name)))
                                continue;
                        
                        NFT_LOG(L_DEBUG, "Found \"%s\"", familyname);
                        amount++;
                }
                
                closedir(dir);
        }

        /* return amount of found files + dummy plugin */
        return amount+1;
}


/**
 * getter
 */
const char *led_hardware_get_plugin_family(LedHardware *h)
{
	if(!h || !h->plugin)
		NFT_LOG_NULL(NULL);

	return h->plugin->family;
}


/**
 * get family-name of a certain installed plugin
 *
 * @param num the index of the plugin (0 to led_hardware_plugin_count()-1).
 *        A value of 0 will always bring the "dummy" hardware-plugin
 * @result string holding the name of this plugin or NULL
 */
const char *led_hardware_get_plugin_family_by_n(int num)
{
        int index = num;
        
        if(num-- == 0)
                return "dummy";

        /* search all possible dirs */
        int i, amount = 0;
        for(i=0; i < sizeof(_prefixes)/sizeof(char *); i++)
        {
                DIR *dir;
                if(!(dir = opendir(_lib_path(_prefixes[i], PLUGINDIR))))
                {
                        NFT_LOG(L_DEBUG, "Failed to open dir \"%s\" (%s)",
                                _lib_path(_prefixes[i], PLUGINDIR), strerror(errno));
                        continue;
                }

                struct dirent *entry;
                while((entry = readdir(dir)))
                {
                        /* extract pluginname from filename */
                        /** @todo maybe load plugin and try to get hardware_descriptor->family */
                        const char *familyname;
                        if(!(familyname = _familyname_from_filename(entry->d_name)))
                                continue;
                        
                        if(num == amount++)
                                return familyname;
                }
                
                closedir(dir);
        }

        NFT_LOG(L_WARNING, "invalid index %d. Only %d installed hardware-plugins found.", 
                index, led_hardware_get_plugin_count());
        
        return NULL;
}



/**
 * return license of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result license-string
 */
const char *led_hardware_get_plugin_license(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);
        
        if(h->plugin->license)
                return h->plugin->license;
        else
                return "Please check url/documentation.";
}


/**
 * return author of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result author-string
 */
const char *led_hardware_get_plugin_author(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);
        
        if(h->plugin->author)
                return h->plugin->author;
        else
                return "Please check url/documentation.";
}


/**
 * return description of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result description-string
 */
const char *led_hardware_get_plugin_description(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);
        
        if(h->plugin->description)
                return h->plugin->description;
        else
                return "Please check url/documentation.";
}


/**
 * return url of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result url-string
 */
const char *led_hardware_get_plugin_url(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);
        
        if(h->plugin->url)
                return h->plugin->url;
        else
                return "Please check documentation.";
}


/**
 * return id-example of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result id-example-string
 */
const char *led_hardware_get_plugin_id_example(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);
        
        if(h->plugin->id_example)
                return h->plugin->id_example;
        else
                return "Please check url/documentation.";
}


/**
 * return plugin major version
 *
 * @param h @ref LedHardware descriptor
 * @result major version number
 */
int led_hardware_get_plugin_version_major(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(-1);

        return h->plugin->major_version;
}


/**
 * return plugin minor version
 *
 * @param h @ref LedHardware descriptor
 * @result minor version number
 */
int led_hardware_get_plugin_version_minor(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(-1);

        return h->plugin->minor_version;
}


/**
 * return plugin micro version
 *
 * @param h @ref LedHardware descriptor
 * @result micro version number
 */
int led_hardware_get_plugin_version_micro(LedHardware *h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(-1);

        return h->plugin->micro_version;
}


/**
 * get printable name of LedPluginObj
 *
 * @param o a valid LedPluginObj type
 * @result pointer to printable name or "undefined"
 */
const char *led_hardware_get_plugin_obj_name(LedPluginObj o)
{
        /** printable names of LedPluginObj definitions */
        static const char *LedPluginObjNames[] =
        {
                "GAIN",
                "LEDCOUNT",
                "HW_ID",
        };

        if(o <= LED_HW_MIN || o >= LED_HW_MAX)
                return "undefined";
        
        return LedPluginObjNames[o-1];
}


/**
 * refresh temporary-chain to reflect mapping of currently registered tiles
 * 
 * @param h LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_refresh_mapping(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* map tiles to chain */
        LedTile *t;
        int mapped = 0;
        for(t = h->relation.first_tile; t; t = led_tile_get_next_sibling(t))
        {
                if((mapped += led_tile_to_chain(t, h->relation.chain, mapped)) == -1)
                {
                        NFT_LOG(L_WARNING, "Failed to map hardware-tile(s) to hardware-chain");
                        return NFT_SUCCESS;
                }
        }
        
        if(mapped != led_hardware_get_ledcount(h))
        {
                NFT_LOG(L_WARNING, "Amount of LEDs mapped from tiles (%d) differs from hardware ledcount (%d)",
                        mapped, led_hardware_get_ledcount(h));
        }

        /* apply hardware-stride */
        led_chain_stride_map(h->relation.chain, led_hardware_get_stride(h), 0);
        
        /* output mapped raw chain (for debugging) */
        led_chain_print(h->relation.chain, L_NOISY);
        
        return NFT_SUCCESS;
}


/**
 * wrapper to apply led_hardware_refresh_mapping() to a list of tiles
 * 
 * @param first first LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_refresh_mapping_list(LedHardware *first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

        if(first->relation.next)
                if(!led_hardware_refresh_mapping_list(first->relation.next))
                        return NFT_FAILURE;
        
        return led_hardware_refresh_mapping(first);
}


/**
 * set hardware LED gain according to values in chain
 *
 * @param h LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_refresh_gain(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        
        
        /* walk all LEDs of hardware */
        int r;
        for(r = 0; r < led_chain_get_ledcount(h->relation.chain); r++)
        {
                Led *led = led_chain_get_nth(h->relation.chain, r);
                if(!led_hardware_set_gain(h, r, led_get_gain(led)))
                        return NFT_FAILURE;
        }
        
        return NFT_SUCCESS;
}


/**
 * set LED gain according to values in a chain to hardware and all siblings
 * plugins
 *
 * @param first first LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_refresh_gain_list(LedHardware *first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

        
        /* walk hardware siblings */
        LedHardware *t;
        for(t=first; t; t = t->relation.next)
                led_hardware_refresh_gain(t);
        
        
        return NFT_SUCCESS;
}


/**
 * wrapper for plugin-function: show current data-buffer
 *
 * @param h @ref LedHardware descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_show(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* don't show on non-initialized plugin */
        if(!led_hardware_is_initialized(h))
        {
                NFT_LOG(L_ERROR, "Attempt to latch on non-initialized hardware (\"%s - %s\")",
                        h->props.name, h->props.id);

                /** try to re-initialize hardware so next call succeeds */
                _reinitialize(h);
                return NFT_FAILURE;
        }

        
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, show))
        {
                NFT_LOG(L_WARNING, "Plugin \"%s\" doesn't provide show-function", h->props.name);
                return NFT_SUCCESS;
        }

        if(!h->plugin->show(h->plugin_privdata))
        {
                NFT_LOG(L_ERROR, "Error while latching %s", h->props.name);

                /* deinitialize hardware */
                led_hardware_deinit(h);
                return NFT_FAILURE;
        }
        
        return NFT_SUCCESS;
}


/**
 * latch a hardware and all siblings sequentially
 *
 * @param first (first) LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_show_list(LedHardware *first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

        NftResult res = NFT_SUCCESS;
        if(first->relation.next)
                res = led_hardware_show_list(first->relation.next);
                        
        
        if(!led_hardware_show(first))
                return NFT_FAILURE;

        return res;
}


/**
 * send values of current chain to hardware-plugin
 *
 * @param h LedHardwareDescriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_send(LedHardware *h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* don't send anything to non-initialized plugin */
        if(!led_hardware_is_initialized(h))
        {
                NFT_LOG(L_ERROR, "Attempt to send to non-initialized hardware (\"%s - %s\")",
                        h->props.name, h->props.id);
                return NFT_FAILURE;
        }

        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, send))
        {
                NFT_LOG(L_WARNING, "Plugin \"%s\" doesn't provide send-function", h->props.name);
                return NFT_SUCCESS;
        }

        NFT_LOG(L_DEBUG, "Sending %d LEDs to %s", led_chain_get_ledcount(h->relation.chain), h->props.name);
        
        if(!h->plugin->send(h->plugin_privdata, h->relation.chain, led_chain_get_ledcount(h->relation.chain), 0))
        {
                NFT_LOG(L_ERROR, "Error while sending to %s", h->props.name);
                
                return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


/**
 * send chain-values to a hardware and all siblings
 * @param first first LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_send_list(LedHardware *first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

        NftResult res = NFT_SUCCESS;
        if(first->relation.next)
                res = led_hardware_send_list(first->relation.next);
                        
        
        if(!led_hardware_send(first))
                return NFT_FAILURE;

        return res;
}

/**
 * helper to combine plugin-name with property-name
 *
 * @param h LedHardware descriptor
 * @param propname name of property
 * @result string of form <hardware-instance-name>-<property-name>
 */
char *led_hardware_get_propname(LedHardware *h, const char *propname)
{

        static char tmpname[256];
        snprintf(tmpname, sizeof(tmpname), "%s-%s", h->props.name, propname);

        return tmpname;
}





/**
 * @}
 */
