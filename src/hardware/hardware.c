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
 *sibling
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
#include <malloc.h>

#if HAVE_DLFCN_H
#include <dlfcn.h>
#else
#error We need dlfcn.h (-ldl) to use runtime loadable plugins
#endif

#include "niftyled-version.h"
#include "niftyled-hardware.h"
#include "niftyled-setup.h"
#include "_tile.h"
#include "_chain.h"
#include "_relation.h"


/** casting macro @todo add type validty check */
#define HARDWARE(h) ((LedHardware *) h)
/** macro to get next hardware */
#define HARDWARE_NEXT(h) (HARDWARE(relation_next(RELATION(h))))
/** macro to get previous hardware */
#define HARDWARE_PREV(h) (HARDWARE(relation_next(RELATION(h))))
/** macro to unlink hardware from any relations */
#define HARDWARE_UNLINK(h) (relation_unlink(RELATION(h)))
/** macro to run a function on each hardware */
#define HARDWARE_FOREACH(h,f, u) (relation_foreach(RELATION(h), f, u))
/** macro to append hardware at end of sibling list */
#define HARDWARE_APPEND(h, s) (relation_append(RELATION(h), RELATION(s)))
/** macro to get nth sibling of a hardware */
#define HARDWARE_NTH(h, n) (HARDWARE(relation_nth(RELATION(h), n)))
/** macro to get total amount of siblings of a hardware */
#define HARDWARE_COUNT(h) (relation_sibling_count(RELATION(h)))

/** casting macro @todo add type validty check */
#define PLUGIN_PROP(p) ((LedPluginCustomProp *) p)
/** macro to get next plugin property */
#define PLUGIN_PROP_NEXT(p) (PLUGIN_PROP(relation_next(RELATION(p))))
/** macro to get previous plugin property */
#define PLUGIN_PROP_PREV(p) (PLUGIN_PROP(relation_prev(RELATION(p))))
/** macro to unlink plugin property from any relations */
#define PLUGIN_PROP_UNLINK(p) (relation_unlink(RELATION(p)))
/** macro to append plugin property at end of sibling list */
#define PLUGIN_PROP_APPEND(p, s) (relation_append(RELATION(p), RELATION(s)))
/** macro to get nth sibling of a plugin property */
#define PLUGIN_PROP_NTH(p, n) (PLUGIN_PROP(relation_nth(RELATION(p), n)))
/** macro to get total amount of siblings of a plugin property */
#define PLUGIN_PROP_COUNT(p) (relation_sibling_count(RELATION(p)))




/** dynamic runtime plugin property */
struct _LedPluginCustomProp
{
		/** relations of this property */
		Relation relation;
        /** name of this property */
        char name[64];
        /** type of this property */
        LedPluginCustomPropType type;
};


/** model of LED-hardware to interface with LEDs */
struct _LedHardware
{
		/** relations of this hardware (must stay first entry in struct)*/
		Relation relation;
		/** chain of this hardware-plugin (holds all currently configured
			LEDs this plugin can control) */
		LedChain *chain;
		/** first LedTile registered to this hardware */
		LedTile *first_tile;
		/** setup of this hardware */
		LedSetup *setup;
        /** plugin handle as returned by dlopen() */
        void *libhandle;
        /** descriptor that has been provided by plugin */
        LedHardwarePlugin *plugin;
        /** first runtime registered dynamic plugin property */
        LedPluginCustomProp *first_prop;
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
            /** if != false device has been initialized successfully */
                char initialized;
            /** amount of LEDs controlled by this hardware (set while hw init) */
                LedCount ledcount;
            /** pixelformat name (set while hw init) */
                char pixelformat[1024];
            /** advance this many LEDs to reach the next LED when sending */
                LedCount stride;
        } params;
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
#ifdef WIN32
#define LED_HARDWARE_FILE_EXTENSION "dll"
#else
#define LED_HARDWARE_FILE_EXTENSION "so"
#endif
/** suffix one hardware plugin has to have (e.g. foobar-hardware.so) */
#define LED_HARDWARE_FILE_SUFFIX "-hardware." LED_HARDWARE_FILE_EXTENSION
/** maximum size of hardware-plugin filename */
#define LED_HARDWARE_LIBNAME_MAXSIZE     1024

        char *suffix;
        if(!(suffix = strstr(filename, LED_HARDWARE_FILE_SUFFIX)))
        {
                /* NFT_LOG(L_WARNING, "\"%s\" has no \"%s\" suffix", filename,
                 * LED_HARDWARE_FILE_SUFFIX); */
                return NULL;
        }

        /* copy filename */
        static char name[LED_HARDWARE_LIBNAME_MAXSIZE];
        strncpy(name, filename, suffix - filename);

        /* terminate string */
        name[suffix - filename] = '\0';

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
/** symbol name of "hardware_descriptor" (LedHardwarePlugin) structure that a hardware-plugin has to provide */
#define LED_HARDWARE_DESCRIPTOR  "hardware_descriptor"



        /* build library-name from hardware-family */
        char *libname;
        if(!(libname = alloca(LED_HARDWARE_LIBNAME_MAXSIZE)))
        {
                NFT_LOG_PERROR("alloca");
                return NULL;
        }

        if(snprintf
           (libname, LED_HARDWARE_LIBNAME_MAXSIZE, "%s/%s-hardware.so",
            PLUGINDIR, family) < 0)
        {
                NFT_LOG_PERROR("snprintf");
                return NULL;
        }


        /* search all prefixes for plugin library */
        void *handle;
        unsigned int i;
        for(i = 0; i < sizeof(_prefixes) / sizeof(char *); i++)
        {
                NFT_LOG(L_NOISY, "\tTrying to load \"%s\"",
                        _lib_path(_prefixes[i], libname));
                if((handle =
                    dlopen(_lib_path(_prefixes[i], libname), RTLD_LAZY)))
                        break;
        }

        /* check for LD_LIBRARY_PATH override */
        if(!handle)
        {
                snprintf(libname, LED_HARDWARE_LIBNAME_MAXSIZE,
                         "%s-hardware.so", family);
                NFT_LOG(L_NOISY, "\tTrying to load \"%s\"", libname);
                if(!(handle = dlopen(libname, RTLD_LAZY)))
                {
                        NFT_LOG(L_ERROR, "Failed to find libfile \"%s\"",
                                libname);
                        return NULL;
                }
        }


        /* get plugin descriptor from newly loaded library */
        LedHardwarePlugin *plugin;
        if(!(plugin = dlsym(handle, LED_HARDWARE_DESCRIPTOR)))
        {
                NFT_LOG(L_ERROR,
                        "Plugin doesn't provide descriptor symbol: \"%s\"",
                        LED_HARDWARE_DESCRIPTOR);
                dlclose(handle);
                return NULL;
        }


        /* check plugin API major-version */
        if(plugin->api_major != HW_PLUGIN_API_MAJOR_VERSION)
        {
                NFT_LOG(L_ERROR,
                        "Plugin has been compiled against major version %d of %s, we are version %d. Not loading plugin.",
                        plugin->api_major, PACKAGE_NAME,
                        HW_PLUGIN_API_MAJOR_VERSION);
                dlclose(handle);
                return NULL;
        }

        /* check plugin API minor-version */
        if(plugin->api_minor != HW_PLUGIN_API_MINOR_VERSION)
        {
                NFT_LOG(L_WARNING,
                        "Plugin compiled against %d of %s, we are version %d. Continue at own risk.",
                        plugin->api_minor, PACKAGE_NAME,
                        HW_PLUGIN_API_MINOR_VERSION);
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
        strncpy(a->params.name, name, sizeof(a->params.name));

        return a;
}



/** unload hardware plugin */
static void _unload_plugin(LedHardware * h)
{
        /* close library */
        NFT_LOG(L_DEBUG, "Unloading plugin instance \"%s\" (%s)",
                h->params.name, h->params.id);
        if(h->libhandle)
                dlclose(h->libhandle);
}


/** try to re-initialize forcefully disconnected hardware */
static void _reinitialize(LedHardware * h)
{
        /**
         * @todo make this dependant on some "try_reconnect"
         * config-property
         */

        /* got a pixelformat? */
        if(h->params.pixelformat[0] == '\0')
                return;

        /** try to initialize */
        if(!led_hardware_init(h,
                              h->params.id,
                              h->params.ledcount, h->params.pixelformat))
        {
                NFT_LOG(L_WARNING, "Attempt to re-initialize %s failed",
                        h->params.name);
        }
}



/**************************** INTERNAL FUNCTIONS ******************************/

/** set parent setup of this hardware */
void hardware_set_parent_setup(LedHardware * h, LedSetup * s)
{
        if(!h)
                NFT_LOG_NULL();

        h->setup = s;
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
        NFT_LOG(L_DEBUG,
                "Trying to create new hardware \"%s\" (plugin: \"%s\")", name,
                plugin_name);

        LedHardware *h;
        if(!(h = _load_plugin(name, plugin_name)))
                return NULL;

        NFT_LOG(L_DEBUG, "Loaded new plugin instance \"%s\"", h->params.name);

        /* print info about loaded plugin */
        led_hardware_plugin_print(h->plugin, L_INFO);

        /* initialize plugin */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, plugin_init))
        {
                if(!(h->plugin->plugin_init(&(h->plugin_privdata), h)))
                {
                        NFT_LOG(L_ERROR,
                                "Plugin initialization function failed");
                        _unload_plugin(h);
                        return NULL;
                }
        }

        /* register to current LedConfCtxt */
        // ~ if(!led_settings_hardware_register(h))
        // ~ {
        // ~ _unload_plugin(h);
        // ~ return NULL;
        // ~ }

        return h;
}



/**
 * quit usage of hardware and free all its resources
 * @note This will also call @ref led_hardware_deinit()
 * @param h @ref LedHardware descriptor
 */
void led_hardware_destroy(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL();

        NFT_LOG(L_DEBUG,
                "Destroying hardware \"%s\" (family: \"%s\" id: \"%s\")",
                h->params.name ? h->params.name : "<undefined>",
                h->plugin->family ? h->plugin->family : "<undefined>",
                h->params.id ? h->params.id : "<undefined>");

		/* unlink from any relations */
		HARDWARE_UNLINK(h);
		
        
        /* is this the first hardware in setup? */
        if(h == led_setup_get_hardware(h->setup))
        {
                /* set next sibling as head of setup */
                led_setup_set_hardware(h->setup, HARDWARE(relation_next(RELATION(h))));
        }

        /* deinitialize hardware */
        led_hardware_deinit(h);

        /* destroy tile(s) */
        led_tile_list_destroy(led_hardware_get_tile(h));

        /* destroy chain */
        chain_destroy(h->chain);

        /* plugin deinitialize */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, plugin_deinit))
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
void led_hardware_list_destroy(LedHardware * first)
{
        if(!first)
                return;

        if(HARDWARE_NEXT(first))
                led_hardware_list_destroy(HARDWARE_NEXT(first));

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
NftResult led_hardware_init(LedHardware * h, const char *id,
                            LedCount ledcount, const char *pixelformat)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* hardware already initialized? */
        if(h->params.initialized)
        {
                NFT_LOG(L_WARNING,
                        "Attempt to initialize already initialized \"%s\" (%s)",
                        h->params.name, id);
                return NFT_SUCCESS;
        }

        /* if we are re-initializing, we already have a chain, otherwise we'll
         * initialize a new one */
        if(!h->chain)
        {
                /** initialize LedChain of this hardware */
                if(!
                   (h->chain = led_chain_new(ledcount, pixelformat)))
                {
                        NFT_LOG(L_ERROR,
                                "Failed to create chain. Initialization failed");
                        return NFT_FAILURE;
                }

                /* register hardware with chain */
                chain_set_parent_hardware(h->chain, h);
        }

        /* save ledcount */
        h->params.ledcount = ledcount;

        /* save pixelformat */
        strncpy(h->params.pixelformat, pixelformat,
                sizeof(h->params.pixelformat));

        /* initialize hardware */
        NFT_LOG(L_DEBUG, "Initializing \"%s\" (%s)...", h->params.name, id);

        /* no init function */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, hw_init))
        {
                /* initialize */
                if(!(h->plugin->hw_init(h->plugin_privdata, id)))
                {
                        NFT_LOG(L_ERROR, "Failed to initialize hardware");
                        return NFT_FAILURE;
                }
        }

        /* mark hardware as "initialized" */
        h->params.initialized = true;

        /* ID might have changed after initializing (when using a wildcard id) */
        NFT_LOG(L_INFO, "\t\033[1mHardware ID:\033[0m \"%s\"\n",
                led_hardware_get_id(h));

        /* set id in model */
        if(!led_hardware_set_id(h, led_hardware_get_id(h)))
        {
                NFT_LOG(L_ERROR, "Failed to set hardware id");
                return NFT_FAILURE;
        }

        /* set ledcount */
        if(!led_hardware_set_ledcount(h, ledcount))
        {
                NFT_LOG(L_WARNING,
                        "Hardware \"%s\" (%s) didn't accept our ledcount (%d). Trying to adapt.",
                        led_hardware_get_name(h), led_hardware_get_id(h),
                        ledcount);

                if(!led_chain_set_ledcount(h->chain, ledcount))
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
void led_hardware_deinit(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL();

        if(!h->params.initialized)
        {
                NFT_LOG(L_DEBUG,
                        "Attempt to deinitialize plugin %s - \"%s\" (%s) that wasn't initialized.",
                        h->plugin->family, h->params.name, h->params.id);
                return;
        }

        /* deinitialize hardware */
        NFT_LOG(L_DEBUG, "Deinitializing \"%s\" (%s)", h->params.name,
                h->params.id);
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, hw_deinit))
                h->plugin->hw_deinit(h->plugin_privdata);

        /* mark hardware as "deinitialized" */
        h->params.initialized = false;
}


/**
 * Get initialization-state of hardware
 *
 * @param h @ref LedHardware descriptor
 * @result NFT_SUCCESS if hardware was initialized, NFT_FAILURE otherwise
 */
NftResult led_hardware_is_initialized(LedHardware * h)
{
        if(!h || !h->params.initialized)
                return NFT_FAILURE;

        return NFT_SUCCESS;
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
const char *led_hardware_get_id(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        /* don't get id from plugin if we aren't initialized */
        if(!led_hardware_is_initialized(h))
                return h->params.id;

        /* does plugin provide get-operation? */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
                return h->params.id;

        /* get operation */
        LedPluginParamData get_id;
        if(h->plugin->get(h->plugin_privdata, LED_HW_ID, &get_id))
        {
                /* buffer id from hardware */
                strncpy(h->params.id, get_id.id, sizeof(h->params.id));
                NFT_LOG(L_DEBUG, "Got id \"%s\" from %s", h->params.id,
                        h->params.name);
        }
        else
                NFT_LOG(L_WARNING, "Plugin failed to deliver an id.");


        return h->params.id;
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
NftResult led_hardware_set_id(LedHardware * h, const char *id)
{
        if(!h || !id)
                NFT_LOG_NULL(NFT_FAILURE);

        NFT_LOG(L_DEBUG, "Setting id of %s to %s", h->params.name, id);

        /* just save id in model if we aren't initialized */
        if(led_hardware_is_initialized(h))
        {
                /* does plugin provide operations? */
                if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
                {
                        /* set id operation */
                        LedPluginParamData set_id = {.id = id };
                        if(!h->plugin->set(h->plugin_privdata, LED_HW_ID,
                                           &set_id))
                        {
                                NFT_LOG(L_ERROR,
                                        "Setting ID to plugin failed.");
                                return NFT_FAILURE;
                        }
                }
                else
                {
                        NFT_LOG(L_WARNING,
                                "Plugin family %s has no set-handler.",
                                h->params.name);
                }
        }


        /** save id in model */
        if(h->params.id != id)
                strncpy(h->params.id, id, sizeof(h->params.id));

        return NFT_SUCCESS;
}


/**
 * set led-stride of hardware
 *
 * @param h hardware to set stride
 * @param stride the amount of LEDs to skip
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_stride(LedHardware * h, LedCount stride)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* does hardware have a chain? */
        if(h->chain)
        {
                /* is stride <= length of chain? */
                LedCount ledcount = led_chain_get_ledcount(h->chain);
                if(stride > ledcount)
                {
                        NFT_LOG(L_ERROR,
                                "Attempt to set stride to %d LEDs but chain of hardware only has %d LEDs.",
                                stride, ledcount);

                        return NFT_FAILURE;
                }
        }

        h->params.stride = stride;

        return NFT_SUCCESS;
}


/**
 * get stride of hardware
 *
 * @param h hardware to get stride from
 * @result stride or 0 upon error
 */
LedCount led_hardware_get_stride(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(0);

        return h->params.stride;
}


/**
 * get currently registered chain of this hardware
 *
 * @param h LedHardware descriptor
 * @result LedChain descriptor
 */
LedChain *led_hardware_get_chain(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        if(!h->chain)
                NFT_LOG(L_DEBUG,
                        "Hardware has no chain. Initialize to create chain.");

        return h->chain;
}


/**
 * set LedTile descriptor registered to this hardware
 *
 * @param h LedHardware descriptor
 * @param t LedTile descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_tile(LedHardware * h, LedTile * t)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* register tile with hardware */
        h->first_tile = t;

        if(t)
        {
                /* register hardware with tile */
                return tile_set_parent_hardware(t, h);
        }

        return NFT_SUCCESS;
}


/**
 * append LedTile to list of registered tile to this hardware
 *
 * @param h LedHardware descriptor
 * @param t LedTile descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_append_tile(LedHardware * h, LedTile * t)
{
        if(!h->first_tile)
                return led_hardware_set_tile(h, t);

        if(!(led_tile_list_append_head(h->first_tile, t)))
        {
                NFT_LOG(L_ERROR,
                        "Failed to append tile %p to hardware \"%s\"", t,
                        led_hardware_get_name(h));
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
LedTile *led_hardware_get_tile(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->first_tile;
}


/**
 * get name of this hardware
 *
 * @param h the LedHardware to get the name from
 * @result const string containing name (or maybe NULL)
 */
const char *led_hardware_get_name(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->params.name;
}


/**
 * set name of this hardware
 *
 * @param h the LedHardware to set name from
 * @param name new name of this hardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_name(LedHardware * h, const char *name)
{
        if(!h || !name)
                NFT_LOG_NULL(NFT_FAILURE);

        strncpy(h->params.name, name, sizeof(h->params.name));

        return NFT_SUCCESS;
}


/**
 * set amount of LEDs connected to hardware
 *
 * @param h @ref LedHardware descriptor
 * @param leds amount of LEDs
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_set_ledcount(LedHardware * h, LedCount leds)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        NFT_LOG(L_DEBUG, "Setting ledcount of %s (%s) to %d", h->params.name,
                h->params.id, leds);

        /* just save ledcount in model if we aren't initialized */
        if(led_hardware_is_initialized(h))
        {
                /* set operation */
                if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
                {
                        LedPluginParamData set_ledcount = {.ledcount = leds };

                        if(!
                           (h->
                            plugin->set(h->plugin_privdata, LED_HW_LEDCOUNT,
                                        &set_ledcount)))
                        {
                                NFT_LOG(L_ERROR,
                                        "Plugin %s (\"%s\") failed ledcount (%d) event",
                                        h->params.name, h->params.id, leds);
                                return NFT_FAILURE;
                        }
                }
                else
                {
                        NFT_LOG(L_WARNING,
                                "Plugin family %s has no set-handler.",
                                h->params.name);
                }
        }

        /* save in model */
        if(!(chain_set_ledcount(h->chain, leds)))
        {
                NFT_LOG(L_ERROR,
                        "Failed to set chain of hardware to new ledcount (%d)",
                        leds);
                return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


/**
 * get amount of LEDs connected to hardware
 *
 * @param h @ref LedHardware descriptor
 * @result amount of LEDs currently configured (or 0 upon error)
 */
LedCount led_hardware_get_ledcount(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(0);

        LedCount ledcount = led_chain_get_ledcount(h->chain);

        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no get-handler.",
                        h->params.name);
                return ledcount;
        }

        LedPluginParamData get_ledcount;

        if(!
           (h->
            plugin->get(h->plugin_privdata, LED_HW_LEDCOUNT, &get_ledcount)))
        {
                NFT_LOG(L_WARNING,
                        "Plugin %s (\"%s\") failed to deliver ledcount. Continuing with current chainlength: %d",
                        h->params.name, h->params.id, ledcount);
                return ledcount;
        }

        if(get_ledcount.ledcount != ledcount)
        {
                /* if hardware is initialized already, we might have a problem
                 * now */
                if(h->params.initialized)
                {
                        NFT_LOG(L_WARNING,
                                "Plugin silently changed ledcount! I'm confused... Continuing with 0 LEDs");
                        return 0;
                }
                else
                {
                        NFT_LOG(L_WARNING,
                                "Plugin is not initialized. Continuing with current hardware chain ledcount.");
                }

                return ledcount;
        }



        /* return value from model if there's no plugin-handler */
        return ledcount;
}


/** helper to sum up LEDs of all related hardware elements */
NftResult _count_leds(Relation *r, void *u)
{
		LedCount *count = u;

		*count += led_hardware_get_ledcount(HARDWARE(r));
		NFT_LOG(L_INFO, "Hardware \"%s\" has %d LEDs",
				led_hardware_get_name(HARDWARE(r)),
				led_hardware_get_ledcount(HARDWARE(r)));

		return NFT_SUCCESS;
}


/**
 * count LEDs connected to hardware and it's siblings
 *
 * @param h First LedHardware
 * @result sum of LEDs registered to all sibling hardware-interfaces in total or < 0 upon error
 */
LedCount led_hardware_list_get_ledcount(LedHardware * h)
{

        if(!h)
                NFT_LOG_NULL(-1);


        /* count total LEDs on hardware adapters */
        LedCount res = 0;
		HARDWARE_FOREACH(h, _count_leds, &res);
        
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
NftResult led_hardware_set_gain(LedHardware * h, LedCount pos, LedGain gain)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        NFT_LOG(L_NOISY, "Setting gain of LED %d from %s (%s) to %hu", pos,
                h->params.name, h->params.id, gain);


        /* set operation plugin */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no set-handler.",
                        h->params.name);
                return NFT_SUCCESS;
        }

        LedPluginParamData set_gain = {.gain.pos = pos,.gain.value = gain };

        if(!(h->plugin->set(h->plugin_privdata, LED_HW_GAIN, &set_gain)))
        {
                NFT_LOG(L_ERROR,
                        "Plugin %s (\"%s\") failed to set gain (%hu) for LED %d",
                        h->params.name, h->params.id, gain, pos);
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
LedGain led_hardware_get_gain(LedHardware * h, LedCount pos)
{
        if(!h)
                NFT_LOG_NULL(0);

        NFT_LOG(L_NOISY, "Getting gain of LED %d from %s (%s)", pos,
                h->params.name, h->params.id);


        /* set operation plugin */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
        {
                NFT_LOG(L_WARNING, "Plugin family %s has no get-handler.",
                        h->params.name);
                return 0;
        }

        LedPluginParamData get_gain = {.gain.pos = pos };

        if(!(h->plugin->get(h->plugin_privdata, LED_HW_GAIN, &get_gain)))
        {
                NFT_LOG(L_ERROR,
                        "Plugin %s (\"%s\") failed to get gain at %d",
                        h->params.name, h->params.id, pos);
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
void *led_hardware_get_privdata(LedHardware * h)
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
NftResult led_hardware_set_privdata(LedHardware * h, void *privdata)
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
void led_hardware_print(LedHardware * h, NftLoglevel l)
{
        if(!h)
                NFT_LOG_NULL();

        NFT_LOG(l,
                "Hardware: %p (\"%s\" id:%s [%s] stride:%d siblings:%d",
                h,
                h->params.name,
                h->params.id,
                h->params.initialized ? "initialized" : "not initialized",
                h->params.stride, led_hardware_list_get_length(h));
}


/** foreach helper to register setup */
static NftResult _register_setup(Relation *r, void *u)
{
		LedHardware *h = HARDWARE(r);
		LedSetup *s = u;

		h->setup = s;

		return NFT_SUCCESS;
}


/**
 * append hardware to last sibling of head
 * @param h a LedHardware
 * @param sibling another LedHardware that should be set as sibling of the last
 * LedHardware in the list, starting from head
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_list_append_head(LedHardware * h,
                                        LedHardware * sibling)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        HARDWARE_APPEND(h, sibling);
		
		/* register setup for this & all siblings */
		return HARDWARE_FOREACH(sibling, _register_setup, h->setup);
}


/**
 * get nth sibling of this hardware
 *
 * @param h a LedHardware (probably the head of a linked list ;)
 * @param n position in list (starting from 0)
 * @result LedHardware at position n in list starting from h or NULL upon error
 */
LedHardware *led_hardware_list_get_nth(LedHardware * h, int n)
{
		return HARDWARE_NTH(h, n);
}


/**
 * get next sibling
 *
 * @param h a LedHardware
 * @result next sibling of h or NULL upon error
 */
LedHardware *led_hardware_list_get_next(LedHardware * h)
{
		return HARDWARE_NEXT(h);
}


/**
 * get previous sibling
 *
 * @param h a LedHardware
 * @result previous sibling of h or NULL upon error
 */
LedHardware *led_hardware_list_get_prev(LedHardware * h)
{
        return HARDWARE_PREV(h);
}


/**
 * get amount of siblings this hardware has left
 *
 * @param h LedHardware to take as head of a linked list
 * @result amount of siblings h has
 */
int led_hardware_list_get_length(LedHardware * h)
{
		return HARDWARE_COUNT(h);
}


/**
 * print debug-info of hardware-plugin
 *
 * @param p a LedHardwarePlugin
 * @param l minimum current loglevel so tile gets printed
 */
void led_hardware_plugin_print(LedHardwarePlugin * p, NftLoglevel l)
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
                (p->url ? p->url : "-"));
}


/**
 * get amount of available plugins, use this to iterate through all
 * installed plugins (e.g. led_hardware_plugin_name())
 *
 * @result amount of installed hardware plugins
 */
int led_hardware_plugin_total_count()
{
        int amount = 0;


        /* count plugins in all possible dirs */
        unsigned int i;
        for(i = 0; i < sizeof(_prefixes) / sizeof(char *); i++)
        {
                DIR *dir;
                if(!(dir = opendir(_lib_path(_prefixes[i], PLUGINDIR))))
                {
                        NFT_LOG(L_DEBUG, "Failed to open dir \"%s\" (%s)",
                                _lib_path(_prefixes[i], PLUGINDIR),
                                strerror(errno));
                        continue;
                }

                struct dirent *entry;
                while((entry = readdir(dir)))
                {
                        /* extract pluginname from filename */
                        const char *familyname;
                        if(!
                           (familyname =
                            _familyname_from_filename(entry->d_name)))
                                continue;

                        NFT_LOG(L_DEBUG, "Found \"%s\"", familyname);
                        amount++;
                }

                closedir(dir);
        }

        /* return amount of found files */
        return amount;
}


/**
 * get plugin descriptor of this hardware
 *
 * @param h @ref LedHardware descriptor
 * @result LedHardwarePlugin descriptor or NULL upon error
 */
LedHardwarePlugin *led_hardware_get_plugin(LedHardware * h)
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
void *led_hardware_plugin_get_privdata(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NULL);

        return h->plugin_privdata;
}


/**
 * getter
 *
 * @param h a LedHardware
 * @result plugin-family of this hardware or NULL
 */
const char *led_hardware_plugin_get_family(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);

        return h->plugin->family;
}


/**
 * get family-name of a certain installed plugin
 *
 * @param num the index of the plugin (0 to led_hardware_plugin_total_count()-1).
 *        A value of 0 will always bring the "dummy" hardware-plugin
 * @result string holding the name of this plugin or NULL
 */
const char *led_hardware_plugin_get_family_by_n(unsigned int num)
{
        int index = num;


        /* search all possible dirs */
        unsigned int i, amount = 0;
        for(i = 0; i < sizeof(_prefixes) / sizeof(char *); i++)
        {
                DIR *dir;
                if(!(dir = opendir(_lib_path(_prefixes[i], PLUGINDIR))))
                {
                        NFT_LOG(L_DEBUG, "Failed to open dir \"%s\" (%s)",
                                _lib_path(_prefixes[i], PLUGINDIR),
                                strerror(errno));
                        continue;
                }

                struct dirent *entry;
                while((entry = readdir(dir)))
                {
                        /* extract pluginname from filename */
                        /** @todo maybe load plugin and try to get hardware_descriptor->family */
                        const char *familyname;
                        if(!
                           (familyname =
                            _familyname_from_filename(entry->d_name)))
                                continue;

                        if(num == amount++)
                                return familyname;
                }

                closedir(dir);
        }

        NFT_LOG(L_WARNING,
                "invalid index %d. Only %d installed hardware-plugins found.",
                index, led_hardware_plugin_total_count());

        return NULL;
}



/**
 * return license of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result license-string
 */
const char *led_hardware_plugin_get_license(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);

        return h->plugin->license ?
                h->plugin->license : "Please check url/documentation.";
}


/**
 * return author of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result author-string
 */
const char *led_hardware_plugin_get_author(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);

        return h->plugin->author ?
                h->plugin->author : "Please check url/documentation.";
}


/**
 * return description of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result description-string
 */
const char *led_hardware_plugin_get_description(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);

        return h->plugin->description ?
                h->plugin->description : "Please check url/documentation.";
}


/**
 * return url of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result url-string
 */
const char *led_hardware_plugin_get_url(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);

        return h->plugin->url ?
                h->plugin->url : "Please check documentation.";
}


/**
 * return id-example of this plugin
 *
 * @param h @ref LedHardware descriptor
 * @result id-example-string
 */
const char *led_hardware_plugin_get_id_example(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(NULL);


        return h->plugin->id_example ?
                h->plugin->id_example : "Please check url/documentation.";

}


/**
 * return plugin major version
 *
 * @param h @ref LedHardware descriptor
 * @result major version number
 */
int led_hardware_plugin_get_version_major(LedHardware * h)
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
int led_hardware_plugin_get_version_minor(LedHardware * h)
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
int led_hardware_plugin_get_version_micro(LedHardware * h)
{
        if(!h || !h->plugin)
                NFT_LOG_NULL(-1);

        return h->plugin->micro_version;
}


/**
 * get printable name of LedPluginParam
 *
 * @param p a valid LedPluginParam type
 * @result pointer to printable name or "undefined"
 */
const char *led_hardware_plugin_get_param_name(LedPluginParam p)
{
        /** printable names of LedPluginParam definitions */
        static const char *LedPluginParamNames[] = {
                "GAIN",
                "LEDCOUNT",
                "HW_ID",
                "CUSTOM_PROP",
        };

        if(p <= LED_HW_MIN || p >= LED_HW_MAX)
                return "undefined";

        return LedPluginParamNames[p - 1];
}



/**
 * refresh temporary-chain to reflect mapping of currently registered tiles
 *
 * @param h LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_refresh_mapping(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        if(!h->chain)
        {
                NFT_LOG(L_WARNING,
                        "Hardware has no chain, yet. (initialize hardware first). Not refreshing mapping.");
                return NFT_FAILURE;
        }

        /* map tiles to chain */
        LedTile *t;
        LedCount mapped = 0;
        for(t = h->first_tile; t; t = led_tile_list_get_next(t))
        {
                LedCount res;
                if((res =
                    led_tile_to_chain(t, h->chain, mapped)) == 0)
                {
                        NFT_LOG(L_WARNING,
                                "Failed to map hardware-tile(s) to hardware-chain");
                        return NFT_SUCCESS;
                }

                mapped += res;
        }

        if(mapped != led_hardware_get_ledcount(h))
        {
                NFT_LOG(L_WARNING,
                        "Amount of LEDs mapped from tiles (%d) differs from hardware ledcount (%d)",
                        mapped, led_hardware_get_ledcount(h));
        }

        /* apply hardware-stride */
        led_chain_stride_map(led_hardware_get_chain(h), 
                             led_hardware_get_stride(h),
                             0);

        /* output mapped raw chain (for debugging) */
        led_chain_print(h->chain, L_NOISY);

        return NFT_SUCCESS;
}


/** foreach helper to refresh mapping of a hardware */
static NftResult _refresh_mapping(Relation *r, void *u)
{
		if(!led_hardware_refresh_mapping(HARDWARE(r)))
                        return NFT_FAILURE;

		return NFT_SUCCESS;
}


/**
 * wrapper to apply led_hardware_refresh_mapping() to a list of tiles
 *
 * @param first first LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_list_refresh_mapping(LedHardware * first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

		return HARDWARE_FOREACH(first, _refresh_mapping, NULL);
}


/**
 * set hardware LED gain according to values in chain
 *
 * @param h LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_refresh_gain(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);



        /* walk all LEDs of hardware */
        LedCount r;
        for(r = 0; r < led_chain_get_ledcount(h->chain); r++)
        {
                Led *led = led_chain_get_nth(h->chain, r);
                if(!led_hardware_set_gain(h, r, led_get_gain(led)))
                        return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


/** foreach helper to refresh gain of a hardware */
static NftResult _refresh_gain(Relation *r, void *u)
{
		return led_hardware_refresh_gain(HARDWARE(r));
}


/**
 * set LED gain according to values in a chain to hardware and all siblings
 * plugins
 *
 * @param first first LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_list_refresh_gain(LedHardware * first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

		return HARDWARE_FOREACH(first, _refresh_gain, NULL);
}


/**
 * wrapper for plugin-function: show current data-buffer
 *
 * @param h @ref LedHardware descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_show(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* don't show on non-initialized plugin */
        if(!led_hardware_is_initialized(h))
        {
                NFT_LOG(L_ERROR,
                        "Attempt to latch on non-initialized hardware (\"%s - %s\")",
                        h->params.name, h->params.id);

                /** try to re-initialize hardware so next call succeeds */
                _reinitialize(h);
                return NFT_FAILURE;
        }


        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, show))
        {
                NFT_LOG(L_WARNING,
                        "Plugin \"%s\" doesn't provide show-function",
                        h->params.name);
                return NFT_SUCCESS;
        }

        if(!h->plugin->show(h->plugin_privdata))
        {
                NFT_LOG(L_ERROR, "Error while latching %s", h->params.name);

                /* deinitialize hardware */
                led_hardware_deinit(h);
                return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


/** foreach helper to show hardware */
static NftResult _show(Relation *r, void *u)
{
		if(!led_hardware_show(HARDWARE(r)))
		{
				NFT_LOG(L_ERROR, "Failed to latch \"%s\"", HARDWARE(r)->params.name);
		}

		return NFT_SUCCESS;
}


/**
 * latch a hardware and all siblings sequentially
 *
 * @param first (first) LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_list_show(LedHardware * first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

		return HARDWARE_FOREACH(first, _show, NULL);
}


/**
 * send values of current chain to hardware-plugin
 *
 * @param h LedHardwareDescriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_send(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(NFT_FAILURE);

        /* don't send anything to non-initialized plugin */
        if(!led_hardware_is_initialized(h))
        {
                NFT_LOG(L_ERROR,
                        "Attempt to send to non-initialized hardware (\"%s - %s\")",
                        h->params.name, h->params.id);
                return NFT_FAILURE;
        }

        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, send))
        {
                NFT_LOG(L_WARNING,
                        "Plugin \"%s\" doesn't provide send-function",
                        h->params.name);
                return NFT_SUCCESS;
        }

        NFT_LOG(L_DEBUG, "Sending %d LEDs to %s",
                led_chain_get_ledcount(h->chain), h->params.name);

        if(!h->plugin->send(h->plugin_privdata, h->chain,
                            led_chain_get_ledcount(h->chain), 0))
        {
                NFT_LOG(L_ERROR, "Error while sending to %s", h->params.name);

                return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


/** foreach helper to send data of hardware */
static NftResult _send(Relation *r, void *u)
{
		return led_hardware_send(HARDWARE(r));
}


/**
 * send chain-values to a hardware and all siblings
 * @param first first LedHardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_list_send(LedHardware * first)
{
        if(!first)
                NFT_LOG_NULL(NFT_FAILURE);

		return HARDWARE_FOREACH(first, _send, NULL);
}











/**
 * register dynamic runtime plugin property
 *
 * @param h LedHardware
 * @param propname printable name of dynamic property
 * @param type one of LedPluginParam
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_register(LedHardware * h,
                                            const char *propname,
                                            LedPluginCustomPropType type)
{
        if(!h || !propname)
                NFT_LOG_NULL(NFT_FAILURE);

        /* validate type */
        if(type >= LED_HW_CUSTOM_PROP_MAX || type <= LED_HW_CUSTOM_PROP_MIN)
        {
                NFT_LOG(L_ERROR,
                        "Invalid type of custom property: %d (valid: %d to %d)",
                        type, LED_HW_CUSTOM_PROP_MIN, LED_HW_CUSTOM_PROP_MAX);
                return NFT_FAILURE;
        }

        /* allocate new descriptor */
        LedPluginCustomProp *p;
        if(!(p = calloc(1, sizeof(LedPluginCustomProp))))
        {
                NFT_LOG_PERROR("calloc");
                return NFT_FAILURE;
        }

        /* copy name */
        strncpy(p->name, propname, sizeof(p->name) - 1);
        if(p->name[sizeof(p->name) - 1] != 0)
        {
                NFT_LOG(L_ERROR,
                        "Property name got truncated. Maximum length is %d bytes",
                        sizeof(p->name));
                free(p);
                return NFT_FAILURE;
        }

        /* set type */
        switch (type)
        {
                case LED_HW_CUSTOM_PROP_INT:
                {
                        p->type = LED_HW_CUSTOM_PROP_INT;
                        break;
                }

                case LED_HW_CUSTOM_PROP_FLOAT:
                {
                        p->type = LED_HW_CUSTOM_PROP_FLOAT;
                }

                case LED_HW_CUSTOM_PROP_STRING:
                {
                        p->type = LED_HW_CUSTOM_PROP_STRING;
                }

                default:
                        break;
        }

        /* first property? */
        if(!h->first_prop)
        {
                h->first_prop = p;
                return NFT_SUCCESS;
        }
		
		return PLUGIN_PROP_APPEND(h->first_prop, p);
}


/**
 * free resources of a registered dynamic runtime plugin property
 *
 * @param h LedHardware
 * @param propname name of property
 */
void led_hardware_plugin_prop_unregister(LedHardware * h,
                                         const char *propname)
{
        if(!h || !propname)
                NFT_LOG_NULL();

        LedPluginCustomProp *p;
        if(!(p = led_hardware_plugin_prop_find(h, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return;
        }

		/* this property is first property of hardware? */
        if(p == h->first_prop)
        {
                h->first_prop = PLUGIN_PROP_NEXT(p);
        }

		/* unlink */
		PLUGIN_PROP_UNLINK(p);
		
      
        free(p);
}


/**
 * count amount of properties a hardware has
 *
 * @param h LedHardware
 * @result amount or properties currently registered to hardware
 */
int led_hardware_plugin_prop_get_count(LedHardware * h)
{
        if(!h)
                NFT_LOG_NULL(0);

        return PLUGIN_PROP_COUNT(h->first_prop);
}


/**
 * get next property of current property
 *
 * @param p current property
 * @result next sibling property or NULL
 */
LedPluginCustomProp *led_hardware_plugin_prop_get_next(LedPluginCustomProp * p)
{
        return PLUGIN_PROP_NEXT(p);
}


/**
 * get nth plugin property of a hardware
 *
 * @param h LedHardware
 * @param n get the nth param (minimum: 0, maximum: led_hardware_plugin_prop_count()-1)
 * @result nth LedPluginCustomProp or NULL
 */
LedPluginCustomProp *led_hardware_plugin_prop_get_nth(LedHardware * h, int n)
{
        if(!h)
                NFT_LOG_NULL(NULL);

		return PLUGIN_PROP_NTH(h->first_prop, n);
}


/** find plugin custom property by its name */
static LedPluginCustomProp *_prop_get_by_name(LedPluginCustomProp * p,
                                              const char *name)
{
        if(!name)
                NFT_LOG_NULL(NULL);

        if(!p)
                return NULL;

        if(strcmp(p->name, name) == 0)
                return p;

        return _prop_get_by_name(PLUGIN_PROP_NEXT(p), name);
}


/**
 * find property by name
 */
LedPluginCustomProp *led_hardware_plugin_prop_find(LedHardware *h, const char *propname)
{
	if(!h || !propname)
			NFT_LOG_NULL(NULL);
		
	return _prop_get_by_name(h->first_prop, propname);
}


/**
 * convert custom property type name to value
 *
 * @param type printable name string of type
 * @result LedPluginCustomPropType or -1 on error
 */
LedPluginCustomPropType led_hardware_plugin_prop_type_from_string(const char
                                                                  *type)
{
        if(!type)
                NFT_LOG_NULL(-1);

        if(strcmp(type, "string") == 0)
                return LED_HW_CUSTOM_PROP_STRING;
        else if(strcmp(type, "int") == 0)
                return LED_HW_CUSTOM_PROP_INT;
        else if(strcmp(type, "float") == 0)
                return LED_HW_CUSTOM_PROP_INT;
        else
                return -1;
}


/**
 * get name of plugin property
 *
 * @param p LedPluginCustomProp
 * @result name of property or NULL
 */
const char *led_hardware_plugin_prop_get_name(LedPluginCustomProp * p)
{
        if(!p)
                NFT_LOG_NULL(NULL);

        return p->name;
}


/**
 * get type of plugin property
 *
 * @param p LedPluginCustomProp
 * @result type of property or -1
 */
LedPluginCustomPropType led_hardware_plugin_prop_get_type(LedPluginCustomProp
                                                          * p)
{
        if(!p)
                NFT_LOG_NULL(-1);

        return p->type;
}


/**
 * set property
 *
 * @param h LedHardware
 * @param propname name of property to set
 * @param s value to set property to
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_set_string(LedHardware * h,
                                              const char *propname,
                                              const char *s)
{
        if(!h || !propname || !s)
                NFT_LOG_NULL(NFT_FAILURE);


        LedPluginCustomProp *p;
        if(!(p = _prop_get_by_name(h->first_prop, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return NFT_FAILURE;
        }

        /* validate type */
        if(p->type != LED_HW_CUSTOM_PROP_STRING)
        {
                NFT_LOG(L_ERROR, "Property \"%s\" is not of type STRING",
                        propname);
                return NFT_FAILURE;
        }

        /* set value of property */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
        {
                LedPluginParamData set_custom = {
                        .custom.name = propname,
                        .custom.type = LED_HW_CUSTOM_PROP_STRING,
                        .custom.valuesize = strlen(s),
                        .custom.value.s = (char *) s
                };

                if(!
                   (h->plugin->set(h->plugin_privdata, LED_HW_CUSTOM_PROP,
                                   &set_custom)))
                {
                        NFT_LOG(L_ERROR,
                                "Plugin %s (\"%s\") failed to set \"%s\"=\"%s\"",
                                h->params.name, h->params.id, propname, s);
                        return NFT_FAILURE;
                }
        }
        else
        {
                NFT_LOG(L_WARNING,
                        "Plugin family %s has no set-handler. Ignoring.",
                        h->params.name);
        }

        return NFT_SUCCESS;
}


/**
 * set property
 *
 * @param h LedHardware
 * @param propname name of property to set
 * @param i value to set property to
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_set_int(LedHardware * h,
                                           const char *propname, int i)
{
        if(!h || !propname)
                NFT_LOG_NULL(NFT_FAILURE);

        LedPluginCustomProp *p;
        if(!(p = _prop_get_by_name(h->first_prop, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return NFT_FAILURE;
        }

        /* validate type */
        if(p->type != LED_HW_CUSTOM_PROP_INT)
        {
                NFT_LOG(L_ERROR, "Property \"%s\" is not of type INT",
                        propname);
                return NFT_FAILURE;
        }

        /* set value of property */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
        {
                LedPluginParamData set_custom = {
                        .custom.name = propname,
                        .custom.type = LED_HW_CUSTOM_PROP_INT,
                        .custom.valuesize = sizeof(int),
                        .custom.value.i = i
                };

                if(!
                   (h->plugin->set(h->plugin_privdata, LED_HW_CUSTOM_PROP,
                                   &set_custom)))
                {
                        NFT_LOG(L_ERROR,
                                "Plugin %s (\"%s\") failed to set \"%s\"=\"%d\"",
                                h->params.name, h->params.id, propname, i);
                        return NFT_FAILURE;
                }
        }
        else
        {
                NFT_LOG(L_WARNING,
                        "Plugin family %s has no set-handler. Ignoring.",
                        h->params.name);
        }

        return NFT_SUCCESS;
}


/**
 * set property
 *
 * @param h LedHardware
 * @param propname name of property to set
 * @param f value to set property to
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_set_float(LedHardware * h,
                                             const char *propname, float f)
{
        if(!h || !propname)
                NFT_LOG_NULL(NFT_FAILURE);

        LedPluginCustomProp *p;
        if(!(p = _prop_get_by_name(h->first_prop, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return NFT_FAILURE;
        }

        /* validate type */
        if(p->type != LED_HW_CUSTOM_PROP_FLOAT)
        {
                NFT_LOG(L_ERROR, "Property \"%s\" is not of type FLOAT",
                        propname);
                return NFT_FAILURE;
        }

        /* set value of property */
        if(LED_HARDWARE_PLUGIN_HAS_FUNC(h, set))
        {
                LedPluginParamData set_custom = {
                        .custom.name = propname,
                        .custom.type = LED_HW_CUSTOM_PROP_FLOAT,
                        .custom.valuesize = sizeof(float),
                        .custom.value.f = f
                };

                if(!
                   (h->plugin->set(h->plugin_privdata, LED_HW_CUSTOM_PROP,
                                   &set_custom)))
                {
                        NFT_LOG(L_ERROR,
                                "Plugin %s (\"%s\") failed to set \"%s\"=\"%f\"",
                                h->params.name, h->params.id, propname, f);
                        return NFT_FAILURE;
                }
        }
        else
        {
                NFT_LOG(L_WARNING,
                        "Plugin family %s has no set-handler. Ignoring.",
                        h->params.name);
        }

        return NFT_SUCCESS;
}


/**
 * get property
 *
 * @param h LedHardware
 * @param propname name of property to set
 * @param v resulting value
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_get_string(LedHardware * h,
                                              const char *propname, char **v)
{
        if(!h || !propname || !v)
                NFT_LOG_NULL(NFT_FAILURE);

        LedPluginCustomProp *p;
        if(!(p = _prop_get_by_name(h->first_prop, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return NFT_FAILURE;
        }

        /* validate type */
        if(p->type != LED_HW_CUSTOM_PROP_STRING)
        {
                NFT_LOG(L_ERROR, "Property \"%s\" is not of type STRING",
                        propname);
                return NFT_FAILURE;
        }

        /* does plugin provide get-operation? */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
        {
                NFT_LOG(L_ERROR, "Plugin \"%s\" doesn't have get() function.",
                        led_hardware_plugin_get_family(h));
                return NFT_FAILURE;
        }

        /* get operation */
        LedPluginParamData get_custom = {
                .custom.name = propname,
                .custom.type = LED_HW_CUSTOM_PROP_STRING,
        };

        if(h->
           plugin->get(h->plugin_privdata, LED_HW_CUSTOM_PROP, &get_custom))
        {
                /* buffer id from hardware */
                NFT_LOG(L_DEBUG, "Got \"%s\"=\"%s\" from %s",
                        propname, get_custom.custom.value.s, h->params.name);
        }
        else
                NFT_LOG(L_WARNING, "Failed to get \"%s\" from %s.",
                        propname, h->params.name);


        *v = get_custom.custom.value.s;

        return NFT_SUCCESS;
}


/**
 * get property
 *
 * @param h LedHardware
 * @param propname name of property to set
 * @param v resulting value
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_get_int(LedHardware * h,
                                           const char *propname, int *v)
{
        if(!h || !propname || !v)
                NFT_LOG_NULL(NFT_FAILURE);

        LedPluginCustomProp *p;
        if(!(p = _prop_get_by_name(h->first_prop, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return NFT_FAILURE;
        }

        /* validate type */
        if(p->type != LED_HW_CUSTOM_PROP_INT)
        {
                NFT_LOG(L_ERROR, "Property \"%s\" is not of type INT",
                        propname);
                return NFT_FAILURE;
        }

        /* does plugin provide get-operation? */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
        {
                NFT_LOG(L_ERROR, "Plugin \"%s\" doesn't have get() function.",
                        led_hardware_plugin_get_family(h));
                return NFT_FAILURE;
        }

        /* get operation */
        LedPluginParamData get_custom = {
                .custom.name = propname,
                .custom.type = LED_HW_CUSTOM_PROP_INT,
        };

        if(h->
           plugin->get(h->plugin_privdata, LED_HW_CUSTOM_PROP, &get_custom))
        {
                /* buffer id from hardware */
                NFT_LOG(L_DEBUG, "Got \"%s\"=\"%d\" from %s",
                        propname, get_custom.custom.value.i, h->params.name);
        }
        else
                NFT_LOG(L_WARNING, "Failed to get \"%s\" from %s.",
                        propname, h->params.name);

        *v = get_custom.custom.value.i;

        return NFT_SUCCESS;
}


/**
 * get property
 *
 * @param h LedHardware
 * @param propname name of property to set
 * @param v resulting value
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_hardware_plugin_prop_get_float(LedHardware * h,
                                             const char *propname, float *v)
{
        if(!h || !propname || !v)
                NFT_LOG_NULL(NFT_FAILURE);

        LedPluginCustomProp *p;
        if(!(p = _prop_get_by_name(h->first_prop, propname)))
        {
                NFT_LOG(L_ERROR, "Failed to find property \"%s\" in \"%s\"",
                        propname, led_hardware_get_name(h));
                return NFT_FAILURE;
        }

        /* validate type */
        if(p->type != LED_HW_CUSTOM_PROP_FLOAT)
        {
                NFT_LOG(L_ERROR, "Property \"%s\" is not of type FLOAT",
                        propname);
                return NFT_FAILURE;
        }

        /* does plugin provide get-operation? */
        if(!LED_HARDWARE_PLUGIN_HAS_FUNC(h, get))
        {
                NFT_LOG(L_ERROR, "Plugin \"%s\" doesn't have get() function.",
                        led_hardware_plugin_get_family(h));
                return NFT_FAILURE;
        }

        /* get operation */
        LedPluginParamData get_custom = {
                .custom.name = propname,
                .custom.type = LED_HW_CUSTOM_PROP_FLOAT,
        };

        if(h->
           plugin->get(h->plugin_privdata, LED_HW_CUSTOM_PROP, &get_custom))
        {
                /* buffer id from hardware */
                NFT_LOG(L_DEBUG, "Got \"%s\"=\"%f\" from %s",
                        propname, get_custom.custom.value.f, h->params.name);
        }
        else
                NFT_LOG(L_WARNING, "Failed to get \"%s\" from %s.",
                        propname, h->params.name);

        *v = get_custom.custom.value.f;

        return NFT_SUCCESS;
}


/**
 * @}
 */
