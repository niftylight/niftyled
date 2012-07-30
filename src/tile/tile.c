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
 * @file tile.c
 *
 * @todo hyperlinks for tiles (load tile from local-file/web address)
 * @todo "documentation", "homepage", ... nodes for tiles (including getters in API)
 */

/**
 * @addtogroup tile
 * @{
 */

#include <math.h>
//#include <niftyled.h>
#include "niftyled-chain.h"
#include "_chain.h"





/** helper macro */
#define MIN(a,b) (((a)<(b))?(a):(b))
/** helper macro */
#define MAX(a,b) (((a)>(b))?(a):(b))





/** 
  * model of one LedTile - every tile 
  * can hold one chain and multiple child- 
  * and sibling tiles 
  */
struct _LedTile
{
        /** rotation around pivot + x/y offset */
        double matrix[3][3];

        /** private userdata */
        void *privdata;

        /** geometrical attributes of a tile */
        struct
        {
                /** X,Y offset of this tile (in pixels) */
                LedFrameCord x,y;
                /** rotation angle of  this tile (in radians) */
                double  rotation;
                /** rotation center of this tile (in pixels) */
                double pivot_x, pivot_y;    
        }geometry;

        /** relationships of this tile */
        struct
        {
                /* the LedChain belonging to this tile (or NULL) */
                LedChain *chain;
                /** previous sibling */
                LedTile *prev;
                /** next sibling */
                LedTile *next;
                /** parent */
                LedTile *parent;
                /** set if parent of tile is a <hardware> */
                LedHardware *parent_hw;
                /** first child */
                LedTile *child;
        }relation;
};





/******************************************************************************
 **************************** STATIC FUNCTIONS ********************************
 ******************************************************************************/

/** 
 * multiply two 3x3 matrices 
 * @todo this could be done better
 */
static void _matrix_mul_3(double dst[3][3], double src[3][3])
{
        double tmp[3][3];
        memset(&tmp, 0, sizeof(tmp));
        
        int i, j, k;
        for(i=0; i<3; i++)
        {
                for(j=0; j<3; j++)
                {
                        for(k=0; k<3; k++)
                        {
                                tmp[i][j] += dst[i][k] * src[k][j];
                        }
                }
        }
        
        memcpy(dst, tmp, sizeof(tmp));
}


/** multiply 1x3 matrix to 3x3 matrix @todo this could be done better */
static void _matrix_mul_1(double dst[3], double src[3][3])
{
        double tmp[3] = { 0, 0, 0};
        
        int i, j;
        for(i=0; i<3; i++)
        {
                for(j=0; j<3; j++)
                {
                        tmp[i] += src[j][i] * dst[j];      
                }
        }
        
        memcpy(dst, tmp, sizeof(tmp));
}


/** fill array with identity matrix */
static void _identity_matrix(double matrix[3][3])
{
        /* initialize matrix */
        matrix[0][0] = 1; matrix[0][1] = 0; matrix[0][2] = 0;
        matrix[1][0] = 0; matrix[1][1] = 1; matrix[1][2] = 0;
        matrix[2][0] = 0; matrix[2][1] = 0; matrix[2][2] = 1;
}


/** translate matrix */
static void _translate(double matrix[3][3], double x, double y)
{
        matrix[2][0] += x;
        matrix[2][1] += y;
}


/** calc rotation matrix and multiply to given matrix */
static void _rotate(double matrix[3][3], double angle)
{
        /* tmp matrix for rotation */
        double tmp[3][3] =
        {
                { cos(-angle), -sin(-angle), 0 },
                { sin(-angle),  cos(-angle), 0 },
                {          0,           0, 1 },
        };
        _matrix_mul_3(matrix, tmp);
}


/** get transformed version of pivot */
static void _transformed_pivot(double angle, double *x, double *y)
{
        if(*x == *y)
                return;
        
        double matrix[3][3];
        _identity_matrix(matrix);
        _rotate(matrix, angle);
        
        double vector[3] = { *x, *y, 1 };
        _matrix_mul_1(vector, matrix);
        *x = fabs(round(vector[0]));
        *y = fabs(round(vector[1]));
}

/** rotation around pivot */
static void _rotate_pivot(double matrix[3][3], double angle, double x, double y)
{
        
        double x2 = x, y2 = y;
        _transformed_pivot(angle, &x2, &y2);
        
        
        /* translate to pivot */
        _translate(matrix, -x, -y);
        
        /* rotate */
        _rotate(matrix, angle);

        /* translate back to rotated pivot */
        _translate(matrix, x2, y2);
}


/** get dimensions of transformed tile */
static void _transformed_dimensions(LedTile *m,
                                    LedFrameCord *width, LedFrameCord *height)
{
        if(*width == *height)
                return;

        double corners[4][3] =
        {
                { 0.0,             0.0,              1 },
                { (double) *width, 0.0,              1 },
                { (double) *width, (double) *height, 1 },
                { 0.0,             (double) *height, 1 },
        };
        
        double matrix[3][3];
        _identity_matrix(matrix);
        _rotate_pivot(matrix, m->geometry.rotation, m->geometry.pivot_x, m->geometry.pivot_y);

        double w_min = 0, h_min = 0, w_max = 0, h_max = 0;

	/* four corners */
        int i;
        for(i = 0; i<4; i++)
        {
                _matrix_mul_1(corners[i], matrix);
                w_max = MAX(w_max, corners[i][0]);
                h_max = MAX(h_max, corners[i][1]);
                w_min = MIN(w_min, corners[i][0]);
                h_min = MIN(h_min, corners[i][1]);
        }
        
        *width = (LedFrameCord)  round(w_max-w_min);
        *height = (LedFrameCord) round(h_max-h_min);

        
        //printf("%.0f/%.0f ---> %d/%d %.0f°\n", w, h, *width, *height, m->geometry.rotation*180/M_PI);
}

/** recalc total dimension of tile */
static void _dimensions(LedTile *m, 
                        LedFrameCord *width, LedFrameCord *height)
{
        
        *width = 0;
        *height = 0;

        
        /* walk children */
        LedTile *child;
        for(child = m->relation.child; child; child = child->relation.next)
        {
                /* recalc dimensions of this child */
                LedFrameCord w, h;
                _dimensions(child, &w, &h);
                _transformed_dimensions(child, &w, &h);
                
                *width  = MAX(*width,  w + child->geometry.x);
                *height = MAX(*height, h + child->geometry.y);

                /* walk siblings of child... */                        
                LedTile *sibling;
                for(sibling = child->relation.next; sibling; sibling = sibling->relation.next)
                {
                        LedFrameCord w, h;
                        _dimensions(sibling, &w, &h);
                        _transformed_dimensions(sibling, &w, &h);
                        
                        *width =  MAX(*width,  w + sibling->geometry.x);
                        *height = MAX(*height, h + sibling->geometry.y);
                }
        
        }

        
        
        /* if we have a chain, find dimensions of our own chain */
        if(m->relation.chain)
        {
                *width = MAX(*width, led_chain_get_max_x(m->relation.chain)+1);
                *height = MAX(*height, led_chain_get_max_y(m->relation.chain)+1);
        }
	
}


/** calculate mapping matrix for one tile */
static void _map_matrix(LedTile *m)
{
        /* recalc matrix for this tile */
        _identity_matrix(m->matrix);
        _rotate_pivot(m->matrix, m->geometry.rotation, m->geometry.pivot_x, m->geometry.pivot_y);
        _translate(m->matrix, m->geometry.x, m->geometry.y);
}


/**************************** INTERNAL FUNCTIONS ******************************/
/**
 * set parent hardware of this tile
 *
 * @param t LedTile descriptor
 * @param h LedHardware descriptor of parent hardware
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult tile_set_parent_hardware(LedTile *t, LedHardware *h)
{
        if(!t)
                NFT_LOG_NULL(NFT_FAILURE);

        if(t->relation.parent)
        {
                NFT_LOG(L_ERROR, "Attempt to attach tile to a hardware but it's already attached to a tile");
                return NFT_FAILURE;
        }
        
        t->relation.parent_hw = h;
        
        return NFT_SUCCESS;
}


/******************************************************************************
 ****************************** API FUNCTIONS *********************************
 ******************************************************************************/

/**
 * create new (empty) LedTile object
 *
 * @result newly created LedTile descriptor
 */
LedTile *led_tile_new()
{
        LedTile *m;
        if(!(m = calloc(1, sizeof(LedTile))))
        {
                NFT_LOG_PERROR("calloc");
                return NULL;
        }

        /* register to current LedConfCtxt */
        //~ if(!led_settings_tile_register(m))
        //~ {
                //~ free(m);
                //~ return NULL;
        //~ }
        
        return m;
}


/**
 * destroy LedTile & all child tiles
 *
 * @param m LedTile descriptor
 */
void led_tile_destroy(LedTile *m)
{
        if(!m)
                return;

        /* free children recursively */
        if(m->relation.child)
                led_tile_list_destroy(m->relation.child);

        /* unlink from linked-list of siblings */
        if(m->relation.next)
                m->relation.next->relation.prev = m->relation.prev;
                
        if(m->relation.prev)
                m->relation.prev->relation.next = m->relation.next;

        
        /* unlink from parent tile */
        if(m->relation.parent)
        {
                /* if we are the first child of our parent, 
                   make our next-sibling the new head */
                if(m->relation.parent->relation.child == m)
                        m->relation.parent->relation.child = m->relation.next;
        }

        /* unlink from parent hardware */
        if(m->relation.parent_hw)
        {
                /* are we the first tile of this hardware? */
                if(led_hardware_get_tile(m->relation.parent_hw) == m)
                {
                        /* register next tile with hardware */
                        led_hardware_set_tile(m->relation.parent_hw, m->relation.next);
                }
        }
        

        
        /* unregister from config context */
        //~ led_settings_tile_unregister(m);
        
        /* free chain of this tile */
        if(m->relation.chain)
                led_chain_destroy(m->relation.chain);

        /* clear old pointers */
        m->relation.next = NULL;
        m->relation.prev = NULL;
        m->relation.child = NULL;
        m->relation.parent = NULL;
        m->relation.parent_hw = NULL;
        m->relation.chain = NULL;
        m->privdata = NULL;
        
        /* free descriptor */
        free(m);
}


/**
 * destroy tile and all siblings recursively
 *
 * @param first First LedTile to destroy
 */
void led_tile_list_destroy(LedTile *first)
{
        if(!first)
                return;

        if(first->relation.next)
                led_tile_list_destroy(first->relation.next);

        
        led_tile_destroy(first);
}


/**
 * create a new LED tile as copy of m and its children
 *
 * @note new tile's parent and siblings will be NULL, only children are copied
 *
 * @param m the tile to create a copy of
 * @result newly allocated tile that replicates m or NULL on error
 */
LedTile *led_tile_dup(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(NULL);

        LedTile *r;
        if(!(r = led_tile_new()))
                return NULL;

        /* copy contents of structure */
        memcpy(r, m, sizeof(LedTile));

        /* clear fields we don't want to duplicate */
        r->relation.child = NULL;
        r->relation.next = NULL;
        r->relation.prev = NULL;
        r->relation.parent = NULL;
        r->relation.parent_hw = NULL;
        
        /* copy chain */
        LedChain *c;
        if((c = led_tile_get_chain(m)))
        {
                if(!led_tile_set_chain(r, led_chain_dup(c)))
                        goto _lmd_error;
        }

        /* copy children */
        LedTile *child;
        for(child = m->relation.child; child; child = child->relation.next)
        {
                if(!led_tile_append_child(r, led_tile_dup(child)))
                        goto _lmd_error;
        }

        
        return r;

        
_lmd_error:
        led_tile_destroy(r);
        return NULL;
}


/**
 * print debug-info for this tile
 *
 * @param t a tile
 * @param l minimum current loglevel so tile gets printed
 */
void led_tile_print(LedTile *t, NftLoglevel l)
{
        if(!t)
                NFT_LOG_NULL();

        NFT_LOG(l,
                "Tile: %p (%d/%d:%dx%d %.2f° [%d/%d]) parent: %s",
                t,
                led_tile_get_width(t), led_tile_get_height(t),
                t->geometry.x, t->geometry.y,
                t->geometry.rotation, t->geometry.pivot_x, t->geometry.pivot_y,
                t->relation.parent ? "tile" : "hardware");
}


/**
 * set x offset of this tile
 *
 * @param m LedTile
 * @param x x-offset
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_x(LedTile *m, LedFrameCord x)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);
        
        /* save x-offset */
        m->geometry.x = x;

        _map_matrix(m);
                
        return NFT_SUCCESS;
}


/**
 * get x offset of this tile
 *
 * @param m LedTile
 * @result x offset in pixels
 */
LedFrameCord led_tile_get_x(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        return m->geometry.x;
}


/**
 * set y offset of this tile
 *
 * @param m LedTile
 * @param y y-offset
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_y(LedTile *m, LedFrameCord y)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        
        /* save x-offset */
        m->geometry.y = y;

        _map_matrix(m);
        
        return NFT_SUCCESS;
}


/**
 * get y offset of this tile
 *
 * @param m LedTile
 * @result y offset in pixels
 */
LedFrameCord led_tile_get_y(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        return m->geometry.y;
}


/**
 * set current rotation angle of this tile (in radians)
 *
 * @param m LedTile descriptor
 * @param angle rotation angle in radians
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_rotation(LedTile *m, double angle)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        m->geometry.rotation = angle - (double) ((int) (angle)/360)*360;

        _map_matrix(m);
                
        return NFT_SUCCESS;
}


/**
 * get current rotation angle of this tile (in radians)
 *
 * @param m LedTile descriptor
 * @result rotation angle in radians
 */
double led_tile_get_rotation(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        return m->geometry.rotation;
}


/**
 * set x-coordinate of rotation center
 *
 * @param m LedTile descriptor
 * @param x coordinate in pixels
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_pivot_x(LedTile *m, double x)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        m->geometry.pivot_x = x;

        _map_matrix(m);
        
        return NFT_SUCCESS;
}


/**
 * get x-coordinate of rotation center
 *
 * @param m LedTile descriptor
 * @result x coordinate in pixels
 */
double led_tile_get_pivot_x(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        return m->geometry.pivot_x;
}


/**
 * set y-coordinate of rotation center
 *
 * @param m LedTile descriptor
 * @param y coordinate in pixels
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_pivot_y(LedTile *m, double y)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        m->geometry.pivot_y = y;

        _map_matrix(m);
        
        return NFT_SUCCESS;
}


/**
 * get y-coordinate of rotation center
 *
 * @param m LedTile descriptor
 * @result y coordinate in pixels
 */
double led_tile_get_pivot_y(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);
        
        return m->geometry.pivot_y ;
}


/**
 * get pivot_x of tile after it has been transformed
 *
 * @param t LedTile descriptor
 * @result x coordinate in pixels
 */
double led_tile_get_transformed_pivot_x(LedTile *t)
{
        if(!t)
                NFT_LOG_NULL(0);

        double x = t->geometry.pivot_x, y = t->geometry.pivot_y;
        _transformed_pivot(t->geometry.rotation, &x, &y);
        
        return x;
}


/**
 * get pivot_y of tile after it has been transformed
 *
 * @param t LedTile descriptor
 * @result y coordinate in pixels
 */
double led_tile_get_transformed_pivot_y(LedTile *t)
{
        if(!t)
                NFT_LOG_NULL(0);

        double x = t->geometry.pivot_x, y = t->geometry.pivot_y;
        _transformed_pivot(t->geometry.rotation, &x, &y);
        
        return y;
}


/**
 * get total mapping width of this tile
 *
 * @param m LedTile
 * @result width in pixels or -1 upon error
 */
LedFrameCord led_tile_get_width(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        LedFrameCord w, h;
        _dimensions(m, &w, &h);

        return w;
}


/**
 * get total mapping width of transformed tile
 * @param t LedTile
 * @result width in pixels or -1 upon error
 */
LedFrameCord led_tile_get_transformed_width(LedTile *t)
{
        if(!t)
                NFT_LOG_NULL(-1);
        
        LedFrameCord w, h;
        _dimensions(t, &w, &h);
        _transformed_dimensions(t, &w, &h);

        return w;
}

/**
 * get total mapping height of this tile
 *
 * @param m LedTile
 * @result height in pixels or -1 upon error
 */
LedFrameCord led_tile_get_height(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        LedFrameCord w, h;
        _dimensions(m, &w, &h);
        
        return h;
}

/**
 * get total mapping height of transformed tile
 * @param t LedTile
 * @result width in pixels or -1 upon error
 */
LedFrameCord led_tile_get_transformed_height(LedTile *t)
{
        if(!t)
                NFT_LOG_NULL(-1);
        
        LedFrameCord w, h;
        _dimensions(t, &w, &h);
        _transformed_dimensions(t, &w, &h);

        return h;
}

/**
 * get the chain belonging to this tile
 *
 * @param m LedTile descriptor
 * @result LedChain descriptor
 */
LedChain *led_tile_get_chain(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(NULL);

        return m->relation.chain;
}


/**
 * set chain belonging to this tile
 *
 * @param m LedTile descriptor
 * @param c LedChain descriptor to associate with this chain
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_chain(LedTile *m, LedChain *c)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        /* register chain to tile */
        m->relation.chain = c;

        /* register tile to chain */
        if(c)
                chain_set_parent_tile(c, m);
                
        return NFT_SUCCESS;
}


/**
 * get private userdata previously set by led_tile_set_privdata()
 *
 * @param t LedTile descriptor
 * @result private userdata
 */
void *led_tile_get_privdata(LedTile *t)
{
        if(!t)
                NFT_LOG_NULL(NULL);

        return t->privdata;
}


/**
 * associate private userdata pointer with tile
 *
 * @param t LedTile descriptor
 * @param privdata pointer to private userdata
 * @result NFT_SUCCESS or NFT_FAILURE upon error
 */
NftResult led_tile_set_privdata(LedTile *t, void *privdata)
{
        if(!t)
                NFT_LOG_NULL(NFT_FAILURE);

        t->privdata = privdata;

        return NFT_SUCCESS;
}


/**
 * return the total amount of LEDs registered in a tile and it's children
 *
 * @param m LedTile descriptor
 * @result amount in LEDs (components)
 */
LedCount led_tile_get_ledcount(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(0);

        
        LedCount r = 0;

        LedTile *child;
        for(child = m->relation.child; child; child = child->relation.next)
        {
                LedCount t;
                if((t = led_tile_get_ledcount(child)) == 0)
                        return 0;
                r += t;
        }

        if((m->relation.chain))
        {
                r += led_chain_get_ledcount(m->relation.chain);
        }
        
        return r;
}


/**
 * append tile to last sibling of head
 *
 * @param head LedTile descriptor
 * @param sibling LedTile descriptor of sibling to be attached
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_list_append_head(LedTile *head, LedTile *sibling)
{
        if(!head)
                NFT_LOG_NULL(NFT_FAILURE);

        /* don't try to append a sibling twice */
        if(head->relation.next == sibling)
                return TRUE;
        
        LedTile *last;
        for(last = head; 
            last->relation.next;
            last = last->relation.next);

        return led_tile_list_append(last, sibling);
}


/**
 * set sibling tile to this tile
 *
 * @param t the older sibling that gets "sibling" registered as "next tile"
 * @param sibling the younger tile that gets "t" registered as "previous tile"
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_list_append(LedTile *t, LedTile *sibling)
{
        if(!t)
                NFT_LOG_NULL(NFT_FAILURE);

        if(t->relation.next && sibling != NULL)
        {
                NFT_LOG(L_ERROR, "Tile already has sibling. Must be removed before setting new one.");
                return NFT_FAILURE;
        }
        
        if(t == sibling)
        {
                NFT_LOG(L_ERROR, "Attempt to make us our own sibling");
                return NFT_FAILURE;
        }
        
        t->relation.next = sibling;

        if(sibling)
                sibling->relation.prev = t;
        
        return NFT_SUCCESS;
}


/**
 * get nth sibling of this tile
 *
 * @param m LedTile descriptor
 * @param n position of sibling to get
 * @result LedTile descriptor of nth sibling
 */
LedTile *led_tile_list_get_nth(LedTile *m, int n)
{
        if(!m)
                return NULL;

        if(n == 0)
                return m;

        return led_tile_list_get_nth(m->relation.next, n-1);
}


/**
 * get next sibling
 *
 * @param m LedTile descriptor
 * @result LedTile descriptor of next sibling
 */
LedTile *led_tile_list_get_next(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(NULL);
        
        return m->relation.next;
}


/**
 * get previous sibling
 *
 * @param m LedTile descriptor
 * @result LedTile descriptor of previous sibling
 */
LedTile *led_tile_list_get_prev(LedTile *m)
{
        if(!m)
                NFT_LOG_NULL(NULL);
        
        return m->relation.prev;
}


/**
 * append tile to last of the parent's children
 *
 * @param m parent LedTile descriptor
 * @param child child LedTile descriptor
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_append_child(LedTile *m, LedTile *child)
{
        if(!m || !child)
                NFT_LOG_NULL(NFT_FAILURE);

        
        
        /* set child-chain to parent */
        if(!(m->relation.child))
        {
        
                if(m == child)
                {
                        NFT_LOG(L_ERROR, "Attempt to make us our own child");
                        return NFT_FAILURE;
                }
                
                /* register child */
                m->relation.child = child;
                
        }
        else
        {
                /* don't try to append a child twice */
                if(m->relation.child == child)
                        return NFT_SUCCESS;

                /* set child at last sibling-tile */
                led_tile_list_append_head(m->relation.child, child);
        }
        
        /* register parent with child */
        if(child)
                child->relation.parent = m;
        

        return NFT_SUCCESS;
}


/**
 * get first child-tile of a parent tile
 *
 * @param t LedTile descriptor
 * @result LedTile descriptor of first child
 */
LedTile *led_tile_get_child(LedTile *t)
{
        if(!t)
                NFT_LOG_NULL(NULL);

        return t->relation.child;
}


/**
 * convert a tile to a flat LedChain
 *
 * @todo improve accuracy (rotation)
 * @param m a LedTile
 * @param dst The destination LedChain
 * @param offset Start writing LEDs at this 
 *      position in dst-chain (to map multiple 
 *      tiles to the same destination-chain)
 * @result The amount of LEDs written to dst (or 0 upon error)
 */
LedCount led_tile_to_chain(LedTile *m, LedChain *dst, LedCount offset)
{       
        if(!m || !dst)
                NFT_LOG_NULL(0);

        
        /* result will be the amount of total LEDs processed */
        LedCount leds_total = 0;
        
        /* process children recursively (if there are any) */
        LedTile *c;
        for(c = m->relation.child; c; c = c->relation.next)
        {
		LedCount res;
                if((res = led_tile_to_chain(c, dst, offset+leds_total)) == 0)
                        return 0;
		
		leds_total += res;
        }

        /* if there's a chain in this tile, process it */
        if(m->relation.chain)
        {
                /* calculate complex transformation matrix for this tile */
                double matrix[3][3];
                _identity_matrix(matrix);
                LedTile *p;
                for(p = m; p; p = p->relation.parent)
                {
                        _matrix_mul_3(matrix, p->matrix);
                }
                                
                /* copy all LEDs of this tile to dst-chain one by one & shift according to offset */
                LedCount i;
                for(i = 0; i < led_chain_get_ledcount(m->relation.chain); i++)
                {
                        if(i+offset >= led_chain_get_ledcount(dst))
                        {
                                NFT_LOG(L_WARNING, "Destination chain is not large enough to map all LEDs of all tiles");
                                break;
                        }

                        /* get destination LED */
                        Led *led = led_chain_get_nth(dst, offset+i);               
                        /* copy current LED of this chain to dest */
                        led_copy(led, led_chain_get_nth(m->relation.chain, i));
                        /* transform position according to complex transform matrix */
                        double vector[3] = { ((double) led_get_x(led))+0.5, ((double) led_get_y(led))+0.5, 1 };
                        _matrix_mul_1(vector, matrix);
                        
                        led_set_x(led, (LedFrameCord) (round(vector[0]-0.5)));
                        led_set_y(led, (LedFrameCord) (round(vector[1]-0.5)));
                        
                        leds_total++;
                }

                NFT_LOG(L_VERBOSE, "Copied %d LEDs from tile to to dest chain (%d LEDs) with offset %d", 
                leds_total, led_chain_get_ledcount(dst), offset);
        }
        
        
        return leds_total;
}




/**
 * @}
 */
