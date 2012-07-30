/*
 * ledset - CLI tool to send a greyscale value to a single LED using libniftyled
 * Copyright (C) 2006-2010 Daniel Hiepler <daniel@niftylight.de>
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

#include <unistd.h>
#include <getopt.h>
#include <niftyled.h>
#include "config.h"


/** we support 2 different modes */
typedef enum
{
	/* normal running-mode. Just set one LED to a defined brightness-value */
	MODE_NORMAL,
	/* interactively step through a chain to map LED->X/Y */
	MODE_INTERACTIVE,
}RunMode;



/** local structure to hold various information */
static struct
{
	/** name of config-file */
	char configfile[1024];
	/** name ouf output file */
	char outputfile[1024];
	/** amount of total LEDs controlled by this instance */
	LedCount ledcount;
	/** chain that represents the whole current setup */
	LedChain *chain;
	/** position of LED to light in setup */
	LedCount ledpos;
	/** brightness value for this LED */
	long long int ledval;
	/** MODE_INTERACTIVE or MODE_NORMAL */
	RunMode mode;
}_c;


/** print a line with all valid logleves */
void _print_loglevels()
{
	/* print loglevels */
	printf("Valid loglevels:\n\t");
	NftLoglevel i;
	for(i = L_MAX+1; i<L_MIN-1; i++)
		printf("%s ", nft_log_level_to_string(i));
	printf("\n");
}


/** print commandline help */
static void _print_help(char *name)
{
	printf("Set brightness of one LED using libniftyled - %s\n"
	       "Usage: %s [options]\n\n"
	       "Valid options:\n"
	       "\t--help\t\t\t-h\t\tThis help text\n"
	       "\t--plugin-help\t\t-p\t\tList of installed plugins + information\n"
	       "\t--config <file>\t\t-c <file>\tLoad this config file [~/.ledset.xml]\n"
	       "\t--pos <pos>\t\t-P <pos>\tPosition of LED in chain [0]\n"
	       "\t--value <value>\t\t-V <value>\tBrightness value [255] (0 = lowest brightness, maximum brightness depends on pixelformat of chain)\n"
	       "\t--loglevel <level>\t-l <level>\tOnly show messages with loglevel <level> [notice]\n"
	       "\t--interactive\t\t-i\t\tInteractive tile-mapper\n"
	       "\t--output <file>\t\t-o <file>\tName of file to write XML config to when in \"interactive\" mode [stdout]\n\n",
	       PACKAGE_URL, name);

	/* print loglevels */
	printf("\n");
	_print_loglevels();

}


/** print list of installed plugins + information they provide */
static void _print_plugin_help()
{

	/* save current loglevel */
	NftLoglevel ll_current = nft_log_level_get();
	nft_log_level_set(L_NOTICE);


	int i;
	for(i = 0; i < led_hardware_plugin_total_count(); i++)
	{
		const char *name;
		if(!(name = led_hardware_plugin_get_family_by_n(i)))
			continue;

		printf("======================================\n\n");

		LedHardware *h;
		if(!(h = led_hardware_new("tmp01", name)))
			continue;

		printf("\tID Example: %s\n",
		       led_hardware_plugin_get_id_example(h));


		led_hardware_destroy(h);

	}

	/* restore logolevel */
	nft_log_level_set(ll_current);
}


/** parse commandline arguments */
static NftResult _parse_args(int argc, char *argv[])
{
	int index, argument;

	static struct option loptions[] =
	{
		{"help", 0, 0, 'h'},
		{"plugin-help", 0, 0, 'p'},
		{"loglevel", required_argument, 0, 'l'},
		{"config", required_argument, 0, 'c'},
		{"pos", required_argument, 0, 'P'},
		{"value", required_argument, 0, 'V'},
		{"interactive", no_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{0,0,0,0}
	};

	while((argument = getopt_long(argc, argv, "hpl:c:P:V:io:", loptions, &index)) >= 0)
	{

		switch(argument)
		{                        
			/* --help */
			case 'h':
			{
				_print_help(argv[0]);
				return NFT_FAILURE;
			}

				/* --plugin-help */
			case 'p':
			{
				_print_plugin_help();
				return NFT_FAILURE;
			}

				/* --config */
			case 'c':
			{
				/* save filename for later */
				strncpy(_c.configfile, optarg, sizeof(_c.configfile));
				break;
			}

				/* --pos */
			case 'P':
			{
				if(sscanf(optarg, "%d", (int*) &_c.ledpos) != 1)
				{
					NFT_LOG(L_ERROR, "Invalid led position \"%s\" (Use a numerical value)", optarg);
					return NFT_FAILURE;
				}
				break;
			}

				/* --value */
			case 'V':
			{
				if(sscanf(optarg, "%Ld", &_c.ledval) != 1)
				{
					NFT_LOG(L_ERROR, "Invalid greyscale-value \"%s\" (Use a numerical value)", optarg);
					return NFT_FAILURE;
				}
				break;
			}

				/* --loglevel */
			case 'l':
			{
				if(!nft_log_level_set(nft_log_level_from_string(optarg)))
				{
					_print_loglevels();
					return NFT_FAILURE;
				}
				break;
			}

				/* run in interactive-mode */
			case 'i':
			{
				_c.mode = MODE_INTERACTIVE;
				break;
			}

				/* --output */
			case 'o':
			{
				/* save filename for later */
				strncpy(_c.outputfile, optarg, sizeof(_c.outputfile));
				break;
			}

				/* invalid argument */
			case '?':
			{
				NFT_LOG(L_ERROR, "argument %d is invalid", index);
				_print_help(argv[0]);
				return NFT_FAILURE;
			}

				/* unhandled arguments */
			default:
			{
				NFT_LOG(L_ERROR, "argument %d is invalid", index);
				break;
			}
		}
	}


	return NFT_SUCCESS;
}





/** light LED n of a certain hardware adapter */
static NftResult _light_led_n(LedHardware *h, LedCount n, long long int val)
{
	/* get chain */
	LedChain *chain;
	if(!(chain = led_hardware_get_chain(h)))
	{
		NFT_LOG(L_ERROR, "Hardware has no chain.");
		return NFT_FAILURE;
	}

	/** set greyscale value */
	if(!led_chain_set_greyscale(chain, n, val))
	{
		NFT_LOG(L_ERROR, "Failed to set grayscale value");
		return NFT_FAILURE;
	}

	/* send chain to hardware */
	if(!led_hardware_send(h))
	{
		NFT_LOG(L_ERROR, "Failed to send data to hardware.");
		return NFT_FAILURE;
	}

	/* latch hardware */
	if(!led_hardware_show(h))
	{
		NFT_LOG(L_ERROR, "Failed to latch hardware.");
		return NFT_FAILURE;
	}

	return NFT_SUCCESS;
}

/** read string from stdin into buf */
static int _readstr(char *buf, size_t bufsize)
{
	int result = - 1;

	/* display the previous "printf" - 
	 this is the wrong place for doing that ;) */
	fflush(stdout);

	if((result = read(STDIN_FILENO, buf, bufsize)) == -1)
	{
		NFT_LOG_PERROR("read()");
	}

	return result;
}

/** read integer value from stdin */
static NftResult _readint(int *i)
{        
	/* display the previous "printf" - 
	 this is the wrong place for doing that ;) */
	fflush(stdout);

	char tmp[64];
	if(_readstr(tmp, sizeof(tmp)) < 0)
		return NFT_FAILURE;

	if(sscanf(tmp, "%d", i) != 1)
		return NFT_FAILURE;

	return NFT_SUCCESS;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main(int argc, char *argv[])
{    

	/* set default loglevel */
	nft_log_level_set(L_NOTICE);

	/* check binary version compatibility */
	NFT_LED_CHECK_VERSION

	/* for preferences context */
	LedPrefs *p = NULL;
	/* for setup created from input file */
	LedSetup *s = NULL;


	/* default values */
	_c.mode = MODE_NORMAL;
	_c.ledcount = 0;
	_c.ledpos = 0;
	_c.ledval = 255;

	/* default prefs-filename */
	if(!led_prefs_default_filename(_c.configfile, sizeof(_c.configfile), ".ledset.xml"))
		return -1;

	/* default output filename (stdout) */
	strncpy(_c.outputfile, "-", sizeof(_c.outputfile));

	/* parse commandline arguments */
	if(!_parse_args(argc, argv))
		return -1;


	/* print welcome msg */
	NFT_LOG(L_INFO, "%s %s (c) D.Hiepler 2006-2011", PACKAGE_NAME, PACKAGE_VERSION);
	NFT_LOG(L_VERBOSE, "Loglevel: %s", nft_log_level_to_string(nft_log_level_get()));


	/* initialize settings context */
	if(!(p = led_prefs_init()))
		goto m_exit;

	/* parse prefs-file */
	LedPrefsNode *pnode;
	if(!(pnode = nft_prefs_node_from_file(p, _c.configfile)))
	{
		NFT_LOG(L_ERROR, "Failed to open configfile \"%s\"", _c.configfile);
		goto m_exit;
	}

	/* create setup from prefs-node */
	if(!(s = led_prefs_setup_from_node(p, pnode)))
	{
		NFT_LOG(L_ERROR, "No valid setup found in preferences file.");
		nft_prefs_node_free(pnode);
		goto m_exit;
	}

	/* free preferences node */
	nft_prefs_node_free(pnode);


	/* first hardware */
	LedHardware *firstHw;
	if(!(firstHw = led_setup_get_hardware(s)))
	{
		NFT_LOG(L_ERROR, "No hardware registered. Cannot send value.");
		goto m_exit;
	}

	/** decide about the mode we are running in */
	switch(_c.mode)
	{
		case MODE_NORMAL:
		{
			/** seek to hardware */
			LedHardware *h;
			LedCount n = _c.ledpos;
			for(h = firstHw; h; h = led_hardware_list_get_next(h))
			{
				if(n < led_chain_get_ledcount(led_hardware_get_chain(h)))
					break;

				n -= led_chain_get_ledcount(led_hardware_get_chain(h));
			}

			NFT_LOG(L_INFO, 
			        "Setting LED %d on hardware \"%s\" to brightness %lld [%d-%d]",
			        n, led_hardware_get_name(h), _c.ledval, 
			        LED_GAIN_MIN, LED_GAIN_MAX);

			_light_led_n(h, n, _c.ledval);

			break;
		}

		case MODE_INTERACTIVE:
		{

			/* set hardware-stride to 0 while doing this */
			/*led_hardware_set_stride(first, 0);*/

			/* get hardware ledcount */
			if(_c.ledcount == 0)
				_c.ledcount = led_chain_get_ledcount(led_hardware_get_chain(firstHw));
			if(_c.ledcount <= 0)
			{
				NFT_LOG(L_ERROR, "ledcount must be > 0");
				goto m_exit;
			}

			printf("\n=====================================================\n"
			       "Going through all %d LEDs on adapter \"%s\" as defined in \"%s\",\n"
			       "lighting one LED at a time. Please enter attributes of the LED that is currently lit.\n"
			       "=====================================================\n\n",
			       _c.ledcount, led_hardware_get_name(firstHw), _c.configfile);

			/* first run through all LEDs once */
			NFT_LOG(L_INFO, "Turning off all LEDs...");
			int l;
			for(l=0; l < _c.ledcount; l++)
			{
				_light_led_n(firstHw, l, 0);
			}
			NFT_LOG(L_INFO, "Done.");


			/* initialize a new tile */
			LedTile *tile;
			if(!(tile = led_tile_new()))
			{
				NFT_LOG(L_ERROR, "Failed to create new tile");
				goto m_exit;
			}

			/* attach tile to hardware */
			if(!(led_hardware_append_tile(firstHw, tile)))
			{
				NFT_LOG(L_ERROR, "Failed to attach tile to hardware");
				goto m_exit;
			}

			/* initialize new chain for new hardware */
			LedChain *chain;
			if(!(chain = led_chain_new(_c.ledcount, led_pixel_format_to_string(
			                                                                   led_chain_get_format(
			                                                                                        led_hardware_get_chain(firstHw)))))
			   )
			{
				NFT_LOG(L_ERROR, "Failed to create new chain.");
				goto m_exit;
			}

			/* attach chain to tile */
			if(!led_tile_set_chain(tile, chain))
			{
				NFT_LOG(L_ERROR, "Failed to attach chain to tile.");
				goto m_exit;
			}

			/* loop through all LEDs */
			for(l = 0; l < _c.ledcount; l++)
			{
				int x,y,component;

				/* light LED n */
				_light_led_n(firstHw, l, _c.ledval);


				/* ask for channel of LED n (if format uses more than one channel) */
				if(led_pixel_format_get_n_components(led_chain_get_format(led_hardware_get_chain(firstHw))) != 1)
				{
					printf("Enter component of LED %d: ", l);
					if(!_readint(&component))
					{
						NFT_LOG(L_ERROR, "Parsing error. Please enter valid integer.");
						/** start over with same LED */
						l--;
						continue;
					}
				}
				/* if we only have one component, we can choose it for the user */
				else
				{
					component = 0;
				}


				/* ask for X of LED n */
				printf("Enter X for LED %d: ", l);
				if(!_readint(&x))
				{
					NFT_LOG(L_ERROR, "Parsing error. Please enter valid integer.");
					/** start over with same LED */
					l--;
					continue;
				}


				/* ask for Y of LED n */
				printf("Enter Y for LED %d: ", l);
				if(!_readint(&y))
				{
					NFT_LOG(L_ERROR, "Parsing error. Please enter valid integer.");
					/** start over with same LED */
					l--;
					continue;
				}


				/* turn LED off */
				_light_led_n(firstHw, l, 0);


				/* update X/Y-values in config */
				Led *led = led_chain_get_nth(chain, l); 
				led_set_x(led, (LedFrameCord) x);
				led_set_y(led, (LedFrameCord) y);
				led_set_component(led, (LedFrameComponent) component);
			}

			/* apply stride */
			NftResult r;
			if((r = led_chain_stride_unmap(chain, led_hardware_get_stride(firstHw), 0)) != _c.ledcount)
			{
				NFT_LOG(L_ERROR, "Amount of LEDs stride-mapped (%d) != total amount of LEDs (%d)",
				        r, _c.ledcount);
			}

			/* handle users that can't count... */
			bool user_can_count = FALSE;
			for(l = 0; l < _c.ledcount; l++)
			{
				if(led_get_x(led_chain_get_nth(chain, l)) == 0 &&
				   led_get_y(led_chain_get_nth(chain, l)) == 0)
				{
					user_can_count = TRUE;
					break;
				}
			}

			/* subtract -1 from every coordinate & notify user */
			if(user_can_count)
			{
				NFT_LOG(L_NOTICE, "It seems you started counting from 1 instead of 0. Trying to correct that error...");
				for(l=0; l < _c.ledcount; l++)
				{
					led_set_x(led_chain_get_nth(chain, l), led_get_x(led_chain_get_nth(chain, l))-1);
					led_set_y(led_chain_get_nth(chain, l), led_get_y(led_chain_get_nth(chain, l))-1);
				}
				NFT_LOG(L_NOTICE, "corrected... Please doublecheck the result.");
			}
			
			/* create config */
			if(!(pnode = led_prefs_setup_to_node(p, s)))
			{
				NFT_LOG(L_ERROR, "Failed to create prefs-node from setup.");
				break;
			}

			/* write config file */
			nft_prefs_node_to_file(p, pnode, _c.outputfile);

			NFT_LOG(L_NOTICE, "Written config file for %dx%d tile.",
			        led_tile_get_width(tile), led_tile_get_height(tile));
			break;
		}
	}



m_exit:
	/* destroy setup */
	led_setup_destroy(s);

	/* destroy prefs */
	led_prefs_deinit(p);


	return 0;
}
