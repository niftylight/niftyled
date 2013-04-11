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
 * @file niftyled-hardware.h
 * @brief LedHardware API to organize hardware adapters that interface to LEDs
 */

/**         
 * @addtogroup setup
 * @{
 * @defgroup hardware LedHardware
 * @brief Used to interface with LED-controlling hardware. 
 * A LedHardware is used to send pixels to a device that then sets 
 * brightness-values on a physical lighting-device (LED)
 * Basically this is an API to runtime loadable plugins and helper functions.
 * All code that actually interfaces the hardware is located in the "plugin"
 * library. 
 * There's one "dummy" plugin family for testing purposes.
 *
 * Every LedHardware has:
 * - one or more @ref LedTile defining the physical location of each LED.
 * - a @ref LedChain representing all LEDs controlled by this hardware instance 
 * - an ID unique to the actual hardware device (e.g. /dev/ttyS0)
 * - a unique name for this instance
 * - a plugin (shared-library) to actually control the hardware device
 * - a stride value (s. http://wiki.niftylight.de/index.php/LED-setup_XML#stride )
 *
 * <h3>Using one or more LedHardware adapters</h3>
 * - use @ref led_hardware_new() to create a new @ref LedHardware.
 * - after that @ref led_hardware_init() will open/initialize the hardware and
 * - @ref led_hardware_deinit() will deinitialize it again. (can be re-initialized again)
 * - @ref led_hardware_destroy() will completly free all resources.
 * - use other functions from this module to interact with the hardware-model.
 * - use led_hardware_list_*() functions to operate on a hardware and all its siblings
 * @{
 */

#ifndef _LED_HARDWARE_H
#define _LED_HARDWARE_H



/** current API version - s. version.sh */
#define LED_HARDWARE_API        HW_PLUGIN_API_VERSION



/** macro to check if plugin provides function */
#define LED_HARDWARE_PLUGIN_HAS_FUNC(h, func) ((h) && \
        (led_hardware_get_plugin(h)) && \
        (led_hardware_get_plugin(h)->func))



/** hardware-model to interface with LEDs */
typedef struct _LedHardware     LedHardware;


#include <niftylog.h>
#include "niftyled-tile.h"




/** dynamic runtime plugin property */
typedef struct _LedPluginCustomProp LedPluginCustomProp;

/** 
 * type IDs for custom plugin properties 
 * @note also edit led_hardware_plugin_prop_type_from_string()
 * if you edit this)
 */
typedef enum
{
        /** always first entry */
        LED_HW_CUSTOM_PROP_MIN,

        /** string property */
        LED_HW_CUSTOM_PROP_STRING,
        /** integer property */
        LED_HW_CUSTOM_PROP_INT,
        /** float property */
        LED_HW_CUSTOM_PROP_FLOAT,

        /** always last entry */
        LED_HW_CUSTOM_PROP_MAX
} LedPluginCustomPropType;


/** 
 * IDs of plugin "parameters" to exchange specific data or settings with the plugin
 * (used for getter/setter) 
 * Parameters are properties common to all plugins. While LedPluginProperties
 * can be dynamically registered specifically by one plugin family, these
 * are available to all plugins. 
 */
typedef enum
{
        /** always first entry */
        LED_HW_MIN,

        /** led-gain */
        LED_HW_GAIN,
        /** amount of LEDs controlled by plugin */
        LED_HW_LEDCOUNT,
        /** hardware-id */
        LED_HW_ID,
        /** custom property */
        LED_HW_CUSTOM_PROP,
        /* add new parameter-types above this line (don't forget to define name in hardware.c: led_hardware_get_plugin_param_name() LedPluginParamNames */

        /** always last entry */
        LED_HW_MAX
} LedPluginParam;



/**
 * @brief plugin-parameter specific data passed to getter/setter 
 * (also s. @ref LedPluginParam)
 * - when used with the setter, you store the value(s) for the LedPluginParam
 *   you want to set in the coresponding field of this union
 * - when used with the getter, you can define this union - pass it to the 
 *   getter - and read out the value(s) of the LedPluginParam from the 
 *   corresponding field afterwards (if getter doesn't fail)
 *
 */
typedef union
{
        /** LED_HW_GAIN: to set/get gain-value of one LED */
        struct
        {
                /** position of LED in chain */
                LedCount                        pos;
                /** gain-value of LED */
                LedGain                         value;
        } gain;
        /** LED_HW_LEDCOUNT: to set/get amount of LEDs the plugin controls */
        LedCount                        ledcount;
        /** LED_HW_ID: hardware id of plugin instance */
        const char                     *id;
        /** LED_HW_CUSTOM_PROP: */
        struct
        {
                /** name of this property */
                const char                     *name;
                /** type of this property */
                LedPluginCustomPropType         type;
                /** size of value in bytes (mainly for strings) */
                size_t                          valuesize;
                /** value of this property */
                union
                {
                        /** LED_HW_CUSTOM_PROP_STRING */
                        char                           *s;
                        /** LED_HW_CUSTOM_PROP_INT */
                        int                             i;
                        /** LED_HW_CUSTOM_PROP_FLOAT */
                        float                           f;
                } value;
        } custom;
} LedPluginParamData;



/** 
 * @brief Descriptor of runtime-loadable plugins to access & control LED-hardware adapters
 * (every plugin must provide this)
 * @note descriptor delivered by plugin should be exported with symbol <b>"hardware_descriptor"</b>
 *
 * <h3>Developing a hardware-plugin:</h3>
 * Every hardware-plugin must provide a symbol called <b>"hardware_descriptor"</b>
 * which holds a @ref LedHardwarePlugin structure describing the plugin. Besides
 * some mandatory information like hardware-family, plugin- or api version
 * the descriptor holds pointers to various mandatory or optional functions. See their
 * description to learn if they're optional or mandatory. The functions are then called by 
 * the library as necessary.
 */
typedef struct LedHardwarePlugin
{
        /** family name of the plugin (lib{family}-hardware.so) */
        const char                     *family;
        /** api major version */
        int                             api_major;
        /** api minor version */
        int                             api_minor;
        /** api micro version */
        int                             api_micro;
        /** plugin major version */
        int                             major_version;
        /** plugin minor version */
        int                             minor_version;
        /** plugin micro version */
        int                             micro_version;
        /** license string or NULL */
        char                           *license;
        /** author(s) string or NULL */
        char                           *author;
        /** short plugin description string or NULL */
        char                           *description;
        /** plugin URL string or NULL */
        char                           *url;
        /** example ID string or NULL */
        char                           *id_example;
        /** 
         * gets called first after loading a hardware_plugin. You may fill 
         * *privdata with a pointer to some own descriptor to differ multiple
         * pieces of hardware supported by your plugin connected at the same 
         * time. You should also register custom properties here.
         * @note mandatory - plugin must provide this function
         * @param privdata The plugins private data-descriptor from LedHardware->privdata 
         * @param h - the LedHardware this plugin belongs to
         * @result NFT_SUCCESS or NFT_FAILURE
         */
        NftResult                       (*plugin_init) (void **privdata, LedHardware * h);
        /** 
         * deinitialize the plugin when it's unloaded - free all resources 
         * @note function is optional - may be NULL
         * @param privdata The plugins private data-descriptor from LedHardware->privdata 
         */
        void                            (*plugin_deinit) (void *privdata);
        /** 
         * initialize hardware (if you want to do something before this, use the plugin_init() handler)
         * @note function is optional - may be NULL
         * @param privdata The plugins private data-descriptor from LedHardware->privdata 
         * @result NFT_SUCCESS or NFT_FAILURE
         */
        NftResult                       (*hw_init) (void *privdata, const char *id);
        /** 
         * deinitialize hardware 
         * @note function is optional - may be NULL
         * @param privdata The plugins private data-descriptor from LedHardware->privdata 
         */
        void                            (*hw_deinit) (void *privdata);
        /**
         * get properties or data from plugin
         *
         * @note function is optional - may be NULL (you'll really want it, tho)
         * @p privdata The plugins private data-descriptor from LedHardware->privdata 
         * @p param plugin parameter
         * @p data union where plugin should store the data
         * @result NFT_SUCCESS or NFT_FAILURE
         */
        NftResult                       (*get) (void *privdata, LedPluginParam param, LedPluginParamData * data);
        /**
         * set properties or data to plugin
         *
         * @note function is optional - may be NULL (you'll really want it, tho)
         * @p privdata The plugins private data-descriptor from LedHardware->privdata 
         * @p param plugin parameter
         * @p data union where plugin reads parameter-specific data from
         * @result NFT_SUCCESS or NFT_FAILURE if value isn't accepted
         */
        NftResult                       (*set) (void *privdata, LedPluginParam param, LedPluginParamData * data);
        /**
         * send data from chain to hardware
         *
         * @note function is optional - may be NULL (but everything will stay pretty dark without it)
         * @p privdata The plugins private data-descriptor from LedHardware->privdata 
         * @p chain the chain holding the data to send
         * @p count amount of LEDs to send
         * @p offset start with data at this LED in chain
         */
        NftResult                       (*send) (void *privdata, LedChain * chain, LedCount count, LedCount offset);
        /**
         * show data sent to chain
         *
         * @note function is optional - may be NULL (but everything will stay pretty dark without it)
         * @p privdata The plugins private data-descriptor from LedHardware->privdata 
         */
        NftResult                       (*show) (void *privdata);
} LedHardwarePlugin;








/* LedHardware functions */
LedHardware                    *led_hardware_new(const char *name, const char *plugin_name);
void                            led_hardware_destroy(LedHardware * h);
NftResult                       led_hardware_init(LedHardware * h, const char *id, LedCount ledcount, const char *pixelformat);
void                            led_hardware_deinit(LedHardware * h);
bool                            led_hardware_is_initialized(LedHardware * h);

LedHardwarePlugin              *led_hardware_get_plugin(LedHardware * h);
const char                     *led_hardware_get_id(LedHardware * h);
LedCount                        led_hardware_get_stride(LedHardware * h);
LedChain                       *led_hardware_get_chain(LedHardware * h);
LedTile                        *led_hardware_get_tile(LedHardware * h);
const char                     *led_hardware_get_name(LedHardware * h);
LedCount                        led_hardware_get_ledcount(LedHardware * h);
LedGain                         led_hardware_get_gain(LedHardware * h, LedCount pos);
void                           *led_hardware_get_privdata(LedHardware * h);

//const char *            led_hardware_get_propname(LedHardware *h, const char *propname);

NftResult                       led_hardware_set_tile(LedHardware * h, LedTile * t);
NftResult                       led_hardware_set_id(LedHardware * h, const char *id);
NftResult                       led_hardware_set_stride(LedHardware * h, LedCount stride);
NftResult                       led_hardware_set_name(LedHardware * h, const char *name);
NftResult                       led_hardware_set_ledcount(LedHardware * h, LedCount leds);
NftResult                       led_hardware_set_gain(LedHardware * h, LedCount pos, LedGain gain);
NftResult                       led_hardware_set_privdata(LedHardware * h, void *privdata);

NftResult                       led_hardware_append_tile(LedHardware * h, LedTile * t);
void                            led_hardware_print(LedHardware * h, NftLoglevel l);
NftResult                       led_hardware_send(LedHardware * h);
NftResult                       led_hardware_show(LedHardware * h);
NftResult                       led_hardware_refresh_gain(LedHardware * h);
NftResult                       led_hardware_refresh_mapping(LedHardware * h);

/* LedHardware linked list functions */
void                            led_hardware_list_destroy(LedHardware * first);
NftResult                       led_hardware_list_append_head(LedHardware * head, LedHardware * sibling);
NftResult                       led_hardware_list_refresh_gain(LedHardware * first);
NftResult                       led_hardware_list_refresh_mapping(LedHardware * first);
NftResult                       led_hardware_list_send(LedHardware * first);
NftResult                       led_hardware_list_show(LedHardware * first);

int                             led_hardware_list_get_length(LedHardware * h);
LedHardware                    *led_hardware_list_get_nth(LedHardware * h, int n);
LedHardware                    *led_hardware_list_get_next(LedHardware * h);
LedHardware                    *led_hardware_list_get_prev(LedHardware * h);
LedCount                        led_hardware_list_get_ledcount(LedHardware * first);

/* LedHardwarePlugin functions */
int                             led_hardware_plugin_total_count();
void                            led_hardware_plugin_print(LedHardwarePlugin * p, NftLoglevel l);
void                           *led_hardware_plugin_get_privdata(LedHardware * h);
const char                     *led_hardware_plugin_get_family(LedHardware * h);
const char                     *led_hardware_plugin_get_family_by_n(unsigned int num);
const char                     *led_hardware_plugin_get_license(LedHardware * h);
const char                     *led_hardware_plugin_get_author(LedHardware * h);
const char                     *led_hardware_plugin_get_description(LedHardware * h);
const char                     *led_hardware_plugin_get_url(LedHardware * h);
const char                     *led_hardware_plugin_get_id_example(LedHardware * h);
int                             led_hardware_plugin_get_version_major(LedHardware * h);
int                             led_hardware_plugin_get_version_minor(LedHardware * h);
int                             led_hardware_plugin_get_version_micro(LedHardware * h);
const char                     *led_hardware_plugin_get_param_name(LedPluginParam p);

/* LedPluginParam functions */
NftResult                       led_hardware_plugin_prop_register(LedHardware * h, const char *propname, LedPluginCustomPropType type);
void                            led_hardware_plugin_prop_unregister(LedHardware * h, const char *propname);
LedPluginCustomPropType         led_hardware_plugin_prop_type_from_string(const char *type);
LedPluginCustomProp            *led_hardware_plugin_prop_find(LedHardware * h, const char *propname);

NftResult                       led_hardware_plugin_prop_set_string(LedHardware * h, const char *propname, const char *s);
NftResult                       led_hardware_plugin_prop_set_int(LedHardware * h, const char *propname, int i);
NftResult                       led_hardware_plugin_prop_set_float(LedHardware * h, const char *propname, float f);

const char                     *led_hardware_plugin_prop_get_name(LedPluginCustomProp * p);
LedPluginCustomPropType         led_hardware_plugin_prop_get_type(LedPluginCustomProp * p);
int                             led_hardware_plugin_prop_get_count(LedHardware * h);
LedPluginCustomProp            *led_hardware_plugin_prop_get_nth(LedHardware * h, int n);
LedPluginCustomProp            *led_hardware_plugin_prop_get_next(LedPluginCustomProp * p);
NftResult                       led_hardware_plugin_prop_get_string(LedHardware * h, const char *propname, char **v);
NftResult                       led_hardware_plugin_prop_get_int(LedHardware * h, const char *propname, int *v);
NftResult                       led_hardware_plugin_prop_get_float(LedHardware * h, const char *propname, float *v);

#endif /* _LED_HARDWARE_H */


/**
 * @}
 * @}
 */
