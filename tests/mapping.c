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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <niftyled.h>


/**
 * thise test is just a playground, it doesn't serve as reference or example.
 * best thing would be, if you don't look at it in the first place.
 */



static LedTile *_create_subsubmodule(LedPrefs *c, LedFrameCord x, LedFrameCord y, double angle)
{
		/* create new LED-Chain */
		LedChain *chain;
		if(!(chain = led_chain_new(4, "RGB u8")))
				return NULL;

		/* set LED positions */
		led_set_x(led_chain_get_nth(chain, 1), 1);
		led_set_y(led_chain_get_nth(chain, 3), 1);
		led_set_x(led_chain_get_nth(chain, 2), 1);
		led_set_y(led_chain_get_nth(chain, 2), 1);

		/* create new module */
		LedTile *m;
		if(!(m = led_tile_new()))
		{
				led_chain_destroy(chain);
				goto _cs_error;
		}

		/* set module  attributes */
		led_tile_set_chain(m, chain);
		led_tile_set_x(m, x);
		led_tile_set_y(m, y);
		led_tile_set_rotation(m, angle);
		led_tile_set_pivot_x(m, 1);
		led_tile_set_pivot_y(m, 1);
		return m;

		_cs_error:
				led_chain_destroy(chain);
				led_tile_destroy(m);
				return NULL;
}


int main(int argc, char *argv[])
{
		int result = -1;
		LedPrefs *conf = NULL;
		LedTile *m = NULL;
		LedTile *msub1 = NULL;
		LedTile *msub2 = NULL;
		LedChain *cm = NULL;


		/* check library version */
		NFT_LED_CHECK_VERSION

				/* set maximum verbosity */
				nft_log_level_set(L_NOISY);


		/* create new config */
		if(!(conf = led_prefs_init()))
				goto m_deinit;



		/***** BEGIN module creation *****/



		/* create parent module */
		if(!(m = led_tile_new()))
				goto m_deinit;


		/* create submodule */
		if(!(msub1 = led_tile_new()))
				goto m_deinit;

		/* append children to submodule */
		if(!led_tile_list_append_child(msub1, _create_subsubmodule(conf, 0, 0, 0)))
				goto m_deinit;
		if(!led_tile_list_append_child(msub1, _create_subsubmodule(conf, 2, 0, (90*M_PI)/180)))
				goto m_deinit;
		if(!led_tile_list_append_child(msub1, _create_subsubmodule(conf, 2,2, (180*M_PI)/180)))
				goto m_deinit;
		if(!led_tile_list_append_child(msub1, _create_subsubmodule(conf, 0, 2, (270*M_PI)/180)))
				goto m_deinit;

		/* duplicate sub-module */
		if(!(msub2 = led_tile_dup(msub1)))
				goto m_deinit;

		/* adjust rotation & offset */
		if(!led_tile_set_x(msub2, 4))
				goto m_deinit;
		if(!led_tile_set_y(msub2, 0))
				goto m_deinit;
		if(!led_tile_set_rotation (msub2, (90*M_PI)/180))
				goto m_deinit;
		if(!led_tile_set_pivot_x(msub2, 2))
				goto m_deinit;
		if(!led_tile_set_pivot_y(msub2, 2))
				goto m_deinit;

		/* append submodules to parent */
		if(!led_tile_list_append_child(m, msub1))
				goto m_deinit;
		if(!led_tile_list_append_child(m, msub2))
				goto m_deinit;


		/***** END module creation *****/


		/* mapping chain */
		if(!(cm = led_chain_new(led_tile_get_ledcount(m), "RGB u8")))
				goto m_deinit;

		/* map */
		if(led_tile_to_chain(m, cm, 0) != led_tile_get_ledcount(m))
				goto m_deinit;



		/* dump config of tile to PrefsNode */
		NftPrefsNode *n;
		if(!(n = led_prefs_tile_to_node(conf, m)))
				goto m_deinit;

		/* dump node to file */
		if(!(led_prefs_node_to_file(n, "-", false)))
				goto m_deinit;

		/* free node */
		//led_prefs_node_free(n);



		/* dump config of chain to PrefsNode */
		if(!(n = led_prefs_chain_to_node(conf, cm)))
				goto m_deinit;

		/* dump node to file */
		if(!(led_prefs_node_to_file(n, "-", false)))
				goto m_deinit;

		/* free node */
		//led_prefs_node_free(n);


		result = 0;



m_deinit:

		/* destroy module (and children + chains) */
		led_tile_destroy(m);
		led_chain_destroy(cm);

		/* cleanup config */
		led_prefs_deinit(conf);

		return result;
}
