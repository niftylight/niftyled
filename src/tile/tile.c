/*
 * libniftyled - Interface library for LED interfaces
 * Copyright (C) 2006-2013 Daniel Hiepler <daniel@niftylight.de>
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
#include "_relation.h"


/** casting macro @todo add type validty check */
#define TILE(t) ((LedTile *) t)
/** macro to get next tile */
#define TILE_NEXT(t) (TILE(_relation_next(RELATION(t))))
/** macro to get previous tile */
#define TILE_PREV(t) (TILE(_relation_next(RELATION(t))))
/** macro to unlink tile from any relations */
#define TILE_UNLINK(t) (_relation_unlink(RELATION(t)))
/** macro to run a function on each tile */
#define TILE_FOREACH(t,f, u) (_relation_foreach(RELATION(t), f, u))
/** macro to append tile at end of sibling list */
#define TILE_APPEND(t, s) (_relation_append(RELATION(t), RELATION(s)))
/** macro to append tile to last sibling of a tile's child */
#define TILE_APPEND_CHILD(t, s) (_relation_append_child(RELATION(t), RELATION(s)))
/** macro to get nth sibling of a tile */
#define TILE_NTH(t, n) (TILE(_relation_nth(RELATION(t), n)))
/** macro to get total amount of siblings of a tile */
#define TILE_COUNT(t) (_relation_sibling_count(RELATION(t)))
/** macro to get first child of this tile */
#define TILE_CHILD(t) (TILE(_relation_child(RELATION(t))))
/** macro to get parent of this tile */
#define TILE_PARENT(t) (TILE(_relation_parent(RELATION(t))))




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
                /** relations of this tile (must stay first entry in struct)*/
        Relation relation;
        /* the LedChain belonging to this tile (or NULL) */
        LedChain *chain;
                /** set if parent of tile is a <hardware> */
        LedHardware *parent_hw;
        /** rotation around pivot + x/y offset */
        double matrix[3][3];
        /** private userdata */
        void *privdata;
        /** geometrical attributes of a tile */
        struct
        {
                /** X,Y offset of this tile (in pixels) */
                LedFrameCord x, y;
                /** rotation angle of  this tile (in radians) */
                double rotation;
                /** rotation center of this tile (in pixels) */
                double pivot_x, pivot_y;
        } geometry;
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
        for(i = 0; i < 3; i++)
        {
                for(j = 0; j < 3; j++)
                {
                        for(k = 0; k < 3; k++)
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
        double tmp[3] = { 0, 0, 0 };

        int i, j;
        for(i = 0; i < 3; i++)
        {
                for(j = 0; j < 3; j++)
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
        matrix[0][0] = 1;
        matrix[0][1] = 0;
        matrix[0][2] = 0;
        matrix[1][0] = 0;
        matrix[1][1] = 1;
        matrix[1][2] = 0;
        matrix[2][0] = 0;
        matrix[2][1] = 0;
        matrix[2][2] = 1;
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
        double tmp[3][3] = {
                { cos(-angle), -sin(-angle), 0 },
                { sin(-angle),  cos(-angle), 0 },
                { 0,            0,           1 },
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
        *x = fabs(vector[0]);
        *y = fabs(vector[1]);
}

/** rotation around pivot */
static void _rotate_pivot(double matrix[3][3], double angle, double x,
                          double y)
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
static void _transformed_dimensions(LedTile * m,
                                    LedFrameCord * width,
                                    LedFrameCord * height)
{
        // if(*width == *height)
        // return;

        double corners[4][3] = 
		{
                { 0.0,             0.0,              1 },
                { (double) *width, 0.0,              1 },
                { (double) *width, (double) *height, 1 },
                { 0.0,             (double) *height, 1 },
        };

        double matrix[3][3];
        _identity_matrix(matrix);
		
        _rotate_pivot(matrix, 
                      m->geometry.rotation, 
                      m->geometry.pivot_x,
                      m->geometry.pivot_y);

        double w_min = 0, h_min = 0, w_max = 0, h_max = 0;

        /* four corners */
        int i;
        for(i = 0; i < 4; i++)
        {
                _matrix_mul_1(corners[i], matrix);
                w_max = MAX(w_max, corners[i][0]);
                h_max = MAX(h_max, corners[i][1]);
                w_min = MIN(w_min, corners[i][0]);
                h_min = MIN(h_min, corners[i][1]);
        }

        *width = (LedFrameCord) round(w_max - w_min);
        *height = (LedFrameCord) round(h_max - h_min);
}


/** foreach helper to calc dimensions of a tile */
static void _dimensions(LedTile * m, LedFrameCord * width,
                        LedFrameCord * height);
static NftResult _calc_dimensions(Relation * r, void *u)
{
        LedTile *t = TILE(r);
        LedFrameCord **dim = u;
        LedFrameCord *width = dim[0];
        LedFrameCord *height = dim[1];

        /* recalc dimensions of this child */
        LedFrameCord w, h;
        _dimensions(t, &w, &h);
        _transformed_dimensions(t, &w, &h);

        *width = MAX(*width, w + t->geometry.x);
        *height = MAX(*height, h + t->geometry.y);

        /* walk siblings of child... */
        // ~ LedTile *sibling;
        // ~ for(sibling = child->relation.next; sibling;
        // ~ sibling = sibling->relation.next)
        // ~ {
        // ~ LedFrameCord w, h;
        // ~ _dimensions(sibling, &w, &h);
        // ~ _transformed_dimensions(sibling, &w, &h);

        // ~ *width = MAX(*width, w + sibling->geometry.x);
        // ~ *height = MAX(*height, h + sibling->geometry.y);
        // ~ }

        return NFT_SUCCESS;
}


/** recalc total dimension of tile */
static void _dimensions(LedTile * m,
                        LedFrameCord * width, LedFrameCord * height)
{
        *width = 0;
        *height = 0;


        /* walk children */
        LedFrameCord *dim[2] = { width, height };
        TILE_FOREACH(TILE_CHILD(m), _calc_dimensions, dim);

        /* if we have a chain, find dimensions of our own chain */
        if(m->chain)
        {
                *width = MAX(*width, led_chain_get_max_x(m->chain) + 1);
                *height = MAX(*height, led_chain_get_max_y(m->chain) + 1);
        }

		NFT_LOG(L_NOISY, "%d/%d", *width, *height);
}


/** calculate mapping matrix for one tile */
static void _map_matrix(LedTile * m)
{
        /* recalc matrix for this tile */
        _identity_matrix(m->matrix);
        _rotate_pivot(m->matrix, m->geometry.rotation, m->geometry.pivot_x,
                      m->geometry.pivot_y);
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
NftResult _tile_set_parent_hardware(LedTile * t, LedHardware * h)
{
        if(!t)
                NFT_LOG_NULL(NFT_FAILURE);

        if(TILE_PARENT(t))
        {
                NFT_LOG(L_ERROR,
                        "Attempt to attach tile to a hardware but it's already attached to a tile");
                return NFT_FAILURE;
        }

        t->parent_hw = h;

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
        // ~ if(!led_settings_tile_register(m))
        // ~ {
        // ~ free(m);
        // ~ return NULL;
        // ~ }

        return m;
}


/** foreach helper to destroy tiles */
static NftResult _destroy(Relation * r, void *u)
{
        led_tile_destroy(TILE(r));

        return NFT_SUCCESS;
}

/**
 * destroy LedTile & all child tiles
 *
 * @param m LedTile descriptor
 */
void led_tile_destroy(LedTile * m)
{
        if(!m)
                return;

        /* free children recursively */
        TILE_FOREACH(TILE_CHILD(m), _destroy, NULL);

        /* unlink from parent hardware */
        if(m->parent_hw)
        {
                /* are we the first tile of this hardware? */
                if(led_hardware_get_tile(m->parent_hw) == m)
                {
                        /* register next tile with hardware */
                        led_hardware_set_tile(m->parent_hw, TILE_NEXT(m));
                }
        }

        /* unlink from linked-list of siblings */
        TILE_UNLINK(m);


        /* unregister from config context */
        // ~ led_settings_tile_unregister(m);

        /* free chain of this tile */
        if(m->chain)
                led_chain_destroy(m->chain);

        /* clear old pointers */
        m->parent_hw = NULL;
        m->chain = NULL;
        m->privdata = NULL;

        /* free descriptor */
        free(m);
}


/** foreach helper to destroy tile */
static NftResult _destroy_list(Relation * r, void *u)
{
        led_tile_destroy(TILE(r));
        return NFT_SUCCESS;
}


/**
 * destroy tile and all siblings recursively
 *
 * @param first First LedTile to destroy
 */
void led_tile_list_destroy(LedTile * first)
{
        if(!first)
                return;

        TILE_FOREACH(first, _destroy_list, NULL);
}


/** foreach helper to append child */
static NftResult _append_child(Relation * r, void *u)
{
        return led_tile_list_append_child(TILE(u), led_tile_dup(TILE(r)));
}


/**
 * create a new LED tile as copy of m and its children
 *
 * @note new tile's parent and siblings will be NULL, only children are copied
 *
 * @param m the tile to create a copy of
 * @result newly allocated tile that replicates m or NULL on error
 * @note if you set a private pointer using led_tile_set_privdata(), it will NOT be copied to the duplicate
 */
LedTile *led_tile_dup(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(NULL);

        LedTile *r;
        if(!(r = led_tile_new()))
                return NULL;

        /* copy contents of structure */
        memcpy(r, m, sizeof(LedTile));

        /* clear fields we don't want to duplicate */
        _relation_clear(RELATION(r));
        r->parent_hw = NULL;

        /* copy chain */
        LedChain *c;
        if((c = led_tile_get_chain(m)))
        {
                if(!led_tile_set_chain(r, led_chain_dup(c)))
                        goto _lmd_error;
        }

        /* copy children */
        LedTile *child;
        if((child = TILE_CHILD(m)))
        {
                if(!TILE_FOREACH(child, _append_child, r))
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
void led_tile_print(LedTile * t, NftLoglevel l)
{
        if(!t)
                NFT_LOG_NULL();

        NFT_LOG(l,
                "Tile: %p (%d/%d:%dx%d %.2f° [%d/%d]) parent: %s",
                t,
                led_tile_get_width(t), led_tile_get_height(t),
                t->geometry.x, t->geometry.y,
                t->geometry.rotation, t->geometry.pivot_x,
                t->geometry.pivot_y,
                t->relation.parent ? "tile" : "hardware");
}


/**
 * set x offset of this tile
 *
 * @param m LedTile
 * @param x x-offset
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_x(LedTile * m, LedFrameCord x)
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
LedFrameCord led_tile_get_x(LedTile * m)
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
NftResult led_tile_set_y(LedTile * m, LedFrameCord y)
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
LedFrameCord led_tile_get_y(LedTile * m)
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
NftResult led_tile_set_rotation(LedTile * m, double angle)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        m->geometry.rotation = angle - (double) ((int) (angle) / 360) * 360;

        _map_matrix(m);

        return NFT_SUCCESS;
}


/**
 * get current rotation angle of this tile (in radians)
 *
 * @param m LedTile descriptor
 * @result rotation angle in radians
 */
double led_tile_get_rotation(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(-1);

		NFT_LOG(L_NOISY, "%.2f°", m->geometry.rotation*180/M_PI);
				
        return m->geometry.rotation;
}


/**
 * set x-coordinate of rotation center
 *
 * @param m LedTile descriptor
 * @param x coordinate in pixels
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_pivot_x(LedTile * m, double x)
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
double led_tile_get_pivot_x(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(-1);

		NFT_LOG(L_NOISY, "%.2f", m->geometry.pivot_x);
		
        return m->geometry.pivot_x;
}


/**
 * set y-coordinate of rotation center
 *
 * @param m LedTile descriptor
 * @param y coordinate in pixels
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_pivot_y(LedTile * m, double y)
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
double led_tile_get_pivot_y(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(-1);

		NFT_LOG(L_NOISY, "%.2f", m->geometry.pivot_y);
		
        return m->geometry.pivot_y;
}


/**
 * get pivot_x of tile after it has been transformed
 *
 * @param t LedTile descriptor
 * @result x coordinate in pixels
 */
double led_tile_get_transformed_pivot_x(LedTile * t)
{
        if(!t)
                NFT_LOG_NULL(0);

        double x = t->geometry.pivot_x, y = t->geometry.pivot_y;
        _transformed_pivot(t->geometry.rotation, &x, &y);

		NFT_LOG(L_NOISY, "%.2f", x);
		
        return x;
}


/**
 * get pivot_y of tile after it has been transformed
 *
 * @param t LedTile descriptor
 * @result y coordinate in pixels
 */
double led_tile_get_transformed_pivot_y(LedTile * t)
{
        if(!t)
                NFT_LOG_NULL(0);

        double x = t->geometry.pivot_x, y = t->geometry.pivot_y;
        _transformed_pivot(t->geometry.rotation, &x, &y);

		NFT_LOG(L_NOISY, "%.2f", y);
		
        return y;
}


/**
 * get total mapping width of this tile
 *
 * @param m LedTile
 * @result width in pixels or -1 upon error
 */
LedFrameCord led_tile_get_width(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        LedFrameCord w, h;
        _dimensions(m, &w, &h);

		NFT_LOG(L_NOISY, "%d", w);
		
        return w;
}


/**
 * get total mapping width of transformed tile
 * @param t LedTile
 * @result width in pixels or -1 upon error
 */
LedFrameCord led_tile_get_transformed_width(LedTile * t)
{
        if(!t)
                NFT_LOG_NULL(-1);

        LedFrameCord w, h;
        _dimensions(t, &w, &h);
        _transformed_dimensions(t, &w, &h);

		NFT_LOG(L_NOISY, "%d", w);

        return w;
}

/**
 * get total mapping height of this tile
 *
 * @param m LedTile
 * @result height in pixels or -1 upon error
 */
LedFrameCord led_tile_get_height(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(-1);

        LedFrameCord w, h;
        _dimensions(m, &w, &h);

		NFT_LOG(L_NOISY, "%d", h);
		
        return h;
}

/**
 * get total mapping height of transformed tile
 * @param t LedTile
 * @result width in pixels or -1 upon error
 */
LedFrameCord led_tile_get_transformed_height(LedTile * t)
{
        if(!t)
                NFT_LOG_NULL(-1);

        LedFrameCord w, h;
        _dimensions(t, &w, &h);
        _transformed_dimensions(t, &w, &h);

		NFT_LOG(L_NOISY, "%d", h);
		
        return h;
}

/**
 * get the chain belonging to this tile
 *
 * @param m LedTile descriptor
 * @result LedChain descriptor
 */
LedChain *led_tile_get_chain(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(NULL);

        return m->chain;
}


/**
 * set chain belonging to this tile
 *
 * @param m LedTile descriptor
 * @param c LedChain descriptor to associate with this chain
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_set_chain(LedTile * m, LedChain * c)
{
        if(!m)
                NFT_LOG_NULL(NFT_FAILURE);

        /* register chain to tile */
        m->chain = c;

        /* register tile to chain */
        if(c)
                _chain_set_parent_tile(c, m);

        return NFT_SUCCESS;
}


/**
 * get private userdata previously set by led_tile_set_privdata()
 *
 * @param t LedTile descriptor
 * @result private userdata
 */
void *led_tile_get_privdata(LedTile * t)
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
NftResult led_tile_set_privdata(LedTile * t, void *privdata)
{
        if(!t)
                NFT_LOG_NULL(NFT_FAILURE);

        t->privdata = privdata;

        return NFT_SUCCESS;
}


/** foreach helper to count LEDs of tile */
static NftResult _ledcount(Relation * r, void *u)
{
        LedCount *c = u;
        LedCount t;
        t = led_tile_get_ledcount(TILE(r));
        *c += t;

        return NFT_SUCCESS;
}


/**
 * return the total amount of LEDs registered in a tile and it's children
 *
 * @param m LedTile descriptor
 * @result amount in LEDs (components)
 */
LedCount led_tile_get_ledcount(LedTile * m)
{
        if(!m)
                NFT_LOG_NULL(0);


        LedCount r = 0;

        TILE_FOREACH(TILE_CHILD(m), _ledcount, &r);

        if((m->chain))
        {
                r += led_chain_get_ledcount(m->chain);
        }

        return r;
}


/** foreach helper to set parent hardware */
static NftResult _set_parent_hw(Relation * r, void *u)
{
        TILE(r)->parent_hw = u;
        return NFT_SUCCESS;
}


/**
 * append tile to last sibling of head
 *
 * @param head LedTile descriptor
 * @param sibling LedTile descriptor of sibling to be attached
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_list_append_head(LedTile * head, LedTile * sibling)
{
        if(!head)
                NFT_LOG_NULL(NFT_FAILURE);

        if(!TILE_APPEND(head, sibling))
                return NFT_FAILURE;

        return TILE_FOREACH(sibling, _set_parent_hw, head->parent_hw);
}


/**
 * append tile to last child of tile
 *
 * @param m parent LedTile descriptor
 * @param child child LedTile descriptor (will be appended to last child of m)
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult led_tile_list_append_child(LedTile * m, LedTile * child)
{
        if(!m || !child)
                NFT_LOG_NULL(NFT_FAILURE);


        return TILE_APPEND_CHILD(m, child);

}


/**
 * get nth sibling of this tile
 *
 * @param m LedTile descriptor
 * @param n position of sibling to get
 * @result LedTile descriptor of nth sibling
 */
LedTile *led_tile_list_get_nth(LedTile * m, int n)
{
        if(!m)
                return NULL;

        if(n == 0)
                return m;

        return TILE_NTH(m, n);
}


/**
 * get next sibling
 *
 * @param m LedTile descriptor
 * @result LedTile descriptor of next sibling
 */
LedTile *led_tile_list_get_next(LedTile * m)
{
        return TILE_NEXT(m);
}


/**
 * get previous sibling
 *
 * @param m LedTile descriptor
 * @result LedTile descriptor of previous sibling
 */
LedTile *led_tile_list_get_prev(LedTile * m)
{
        return TILE_PREV(m);
}


/**
 * get first child-tile of a parent tile
 *
 * @param t LedTile descriptor
 * @result LedTile descriptor of first child
 */
LedTile *led_tile_get_child(LedTile * t)
{
        return TILE_CHILD(t);
}


/**
 * get parent tile of a tile
 *
 * @param t LedTile descriptor
 * @result LedTile descriptor of parent or NULL
 */
LedTile *led_tile_get_parent_tile(LedTile * t)
{
        return TILE_PARENT(t);
}


/**
 * get parent hardware of a tile
 *
 * @param t LedTile descriptor
 * @result LedHardware descriptor of parent or NULL
 */
LedHardware *led_tile_get_parent_hardware(LedTile * t)
{
        if(!t)
                NFT_LOG_NULL(NULL);

        return t->parent_hw;
}


/** foreach helper to translate children of a tile */
NftResult _to_chain(Relation * r, void *u)
{
        struct
        {
                LedCount *total;
                LedChain *dst;
                LedCount *offset;
        } *desc = u;

        *desc->total +=
                led_tile_to_chain(TILE(r), desc->dst,
                                  *desc->offset + *desc->total);

        return NFT_SUCCESS;
}


/**
 * translate the chain of a tile (or subtile(s)) to a
 * LedChain with respect to the offset, rotation and pivot of
 * tile(s) 
 *
 * @param m a LedTile
 * @param dst The destination LedChain
 * @param offset Start writing LEDs at this
 *      position in dst-chain (to map multiple
 *      tiles to the same destination-chain)
 * @result The amount of LEDs written to dst (or 0 upon error)
 */
LedCount led_tile_to_chain(LedTile * m, LedChain * dst, LedCount offset)
{
        if(!m || !dst)
                NFT_LOG_NULL(0);


        /* result will be the amount of total LEDs processed */
        LedCount leds_total = 0;

        /* process children recursively (if there are any) */
        struct
        {
                LedCount *total;
                LedChain *dst;
                LedCount *offset;
        }
        desc =
        {
        .total = &leds_total,.dst = dst,.offset = &offset};

        TILE_FOREACH(TILE_CHILD(m), _to_chain, &desc);

        /* if there's a chain in this tile, process it */
        if(m->chain)
        {
                /* calculate complex transformation matrix for this tile */
                double matrix[3][3];
                _identity_matrix(matrix);
                LedTile *p;
                for(p = m; p; p = TILE_PARENT(p))
                {
                        _matrix_mul_3(matrix, p->matrix);
                }

                /* copy all LEDs of this tile to dst-chain one by one & shift
                 * according to offset */
                LedCount i;
                for(i = 0; i < led_chain_get_ledcount(m->chain); i++)
                {
                        if(i + offset >= led_chain_get_ledcount(dst))
                        {
                                NFT_LOG(L_WARNING,
                                        "Destination chain is not large enough to map all LEDs of all tiles");
                                break;
                        }

                        /* get destination LED */
                        Led *led = led_chain_get_nth(dst, offset + i);

                        /* copy current LED of this chain to dest */
                        led_copy(led, led_chain_get_nth(m->chain, i));

                        /* copy greyscale value */
                        long long int greyscale = 0;
                        led_chain_get_greyscale(m->chain, i, &greyscale);
                        led_chain_set_greyscale(dst, offset + i, greyscale);

                        /* transform position according to complex transform
                         * matrix */
                        double vector[3] = { ((double) led_get_x(led)) + 0.5,
                                ((double) led_get_y(led)) + 0.5, 1
                        };
                        _matrix_mul_1(vector, matrix);

                        led_set_x(led,
                                  (LedFrameCord) (round(vector[0] - 0.5)));
                        led_set_y(led,
                                  (LedFrameCord) (round(vector[1] - 0.5)));

                        leds_total++;
                }

                NFT_LOG(L_VERBOSE,
                        "Copied %d LEDs from tile to to dest chain (%d LEDs) with offset %d",
                        leds_total, led_chain_get_ledcount(dst), offset);
        }


        return leds_total;
}




/**
 * @}
 */
