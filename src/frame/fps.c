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
 * @file fps.c
 */


/**
 * @addtogroup fps
 * @{
 */

#include <unistd.h>
#include <sys/time.h>
#include <niftyled.h>



/** space to hold the time */
static struct timeval _last;
/** last fps calculated */
static int _current_fps;


/**
 * sample current time
 */
NftResult led_fps_sample()
{
        
        if(gettimeofday(&_last, NULL) != 0)
        {
                NFT_LOG_PERROR("gettimeofday");
                return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


/**
 * delay until next frame is due
 */
NftResult led_fps_delay(int fps)
{
        /* get current-time */
        struct timeval current;
        if(gettimeofday(&current, NULL) != 0)
        {
                NFT_LOG_PERROR("gettimeofday");
                return NFT_FAILURE;
        }

        /* calc delay from fps */
        long fps_sec =  (1/fps);
        long fps_usec = (1000000/fps);
                
        /* delay if frame isn't due yet */
        if(((_last.tv_sec+fps_sec) >= current.tv_sec) &&
           ((_last.tv_usec+fps_usec) > current.tv_usec))
        {
                /* calc time to delay */
                unsigned long _usec = (_last.tv_usec+fps_usec)-current.tv_usec;
                unsigned long _sec = (_last.tv_sec+fps_sec)-current.tv_sec;
                usleep(_usec + (_sec*1000000));
                
                /* calculate current fps */
                _current_fps = 1000000/
                        (
                                (current.tv_usec-_last.tv_usec + (current.tv_sec-_last.tv_sec)*1000000)+
                                (_usec + (_sec*1000000))
                         );
        }
        else
        {
                /* calculate current fps */
                _current_fps = 1000000/(current.tv_usec-_last.tv_usec + (current.tv_sec-_last.tv_sec)*1000000);
        }
        
        return NFT_SUCCESS;
}


/**
 * return current fps
 */
int led_fps_get()
{
        return _current_fps;
}


/**
 * @}
 */
