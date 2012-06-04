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

#include <niftyled.h>
#include "config.h"


/**
 * check the compiled lib-version against the include one.
 * This can throw a warning or immediately kill the application.
 * @note don't call this directly. Use the LED_CHECK_VERSION
 * macro
 *
 * @param version the include version number
 */
void led_check_version(int version)
{
        /* check version of dependencies */
        NFT_LOG_CHECK_VERSION
        NFT_SETTINGS_CHECK_VERSION
                
                
        /* check our own version */
        int myversion = (int) GENERIC_API_VERSION;

        if((myversion / 10000) != (version / 10000))
        {
                NFT_LOG(L_ERROR, "Program compiled against %s API-Version %d but currently installed is %d. Please upgrade.",
                        PACKAGE_NAME, version, myversion);
                exit(1);
        }

        if(myversion != version)
        {
                NFT_LOG(L_WARNING, "Program compiled against %s API-Version %d but currently installed is %d.",
                        PACKAGE_NAME, version, myversion);
        }

        
        NFT_LOG(L_INFO, "%s v%d.%d.%d (API %d)",
                PACKAGE_NAME, 
                GENERIC_MAJOR_VERSION,
                GENERIC_MINOR_VERSION,
                GENERIC_MICRO_VERSION,
                GENERIC_API_VERSION);
}
