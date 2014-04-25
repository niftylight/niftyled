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
 * @file niftyled-prefs.h
 * @brief LedPrefs API to organize all preferences of an LED setup
 */

/**
 * @defgroup prefs LedPrefs
 * @brief XML configuration
 *
 * @{
 */

#ifndef _LED_PREFS_H
#define _LED_PREFS_H

#include <niftyprefs.h>
#include "niftyled-frame.h"



/** model to hold LedPrefs */
typedef NftPrefs                LedPrefs;

/** wrapper model for niftyprefs */
typedef NftPrefsNode            LedPrefsNode;


/* integer representation of niftyled elements */
typedef enum
{
        LED_SETUP_T = 1,
        LED_HARDWARE_T,
        LED_TILE_T,
        LED_CHAIN_T,
        LED_T,
        LED_INVALID_T,
} NIFTYLED_TYPE;




NftResult                       led_prefs_default_filename(char *dst, size_t size, const char *filename);
const char                     *led_prefs_node_get_uri(LedPrefsNode * n);

LedPrefs                       *led_prefs_init();
void                            led_prefs_deinit(LedPrefs * p);

char                           *led_prefs_node_to_buffer_minimal(LedPrefs *p, LedPrefsNode * n);
char                           *led_prefs_node_to_buffer(LedPrefs *p, LedPrefsNode * n);
NftResult                       led_prefs_node_to_file_minimal(LedPrefs *p, LedPrefsNode * n, const char *filename, bool overwrite);
NftResult                       led_prefs_node_to_file(LedPrefs *p, LedPrefsNode * n, const char *filename, bool overwrite);
LedPrefsNode                   *led_prefs_node_from_buffer(LedPrefs *p, char *buffer, size_t bufsize);
LedPrefsNode                   *led_prefs_node_from_file(LedPrefs *p, const char *filename);
NIFTYLED_TYPE                   led_prefs_node_get_type(LedPrefsNode * n);

NIFTYLED_TYPE                   led_prefs_type_from_string(const char *name);
const char                     *led_prefs_type_to_string(NIFTYLED_TYPE type);

void                            led_prefs_node_free(LedPrefsNode * n);



#endif /* _LED_PREFS_H */

/**
 * @}
 */
