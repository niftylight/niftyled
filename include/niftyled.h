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


#ifndef _NIFTYLED
#define _NIFTYLED

/**
 * @mainpage libniftyled API documentation
 *
 * <h2>A library designed to provide an abstract interface to use LED hardware.</h2>
 *
 * For hardware-plugin developers - Use your LED device with libniftyled:
 * - check the dummy plugin as example (lib/niftyled-plugins/example)
 * - check documentation of a @ref LedHardwarePlugin
 *
 * For LED-controlling application developers:
 * - check @ref LedChain to define a chain of serially arranged @ref Led's
 * - check @ref LedTile to see how to associate a @ref LedChain to a tile and build larger tiles from groups of tiles
 * - check @ref LedHardware for interfacing with hardware
 * - check @ref LedSettings for loading, saving & handling LED-Setup configurations
 *
 * @todo check if all loglevels are appropriate to message
 */

/**
 * @file niftyled.h
 * @brief niftyled API toplevel include file
 */



#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif


/* integer representation of niftyled elements */
typedef enum
{
        T_LED_HARDWARE = 1,
        T_LED_TILE,
        T_LED_CHAIN,
        T_LED,
        T_LED_INVALID,
}NIFTYLED_TYPE;




#include <niftylog.h>
#include <niftyprefs.h>
#include "niftyled_version.h"
#include "pixel_format.h"
#include "frame.h"
#include "chain.h"
#include "hardware.h"
#include "tile.h"
#include "fps.h"
/*#include "settings.h"
#include "settings_chain.h"
#include "settings_hardware.h"
#include "settings_tile.h"*/




#endif /* _NIFTYLED */

/**
 * @}
 */
