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
 * @file relation.c
 */

#include <stdbool.h>
#include <niftylog.h>
#include "_relation.h"




/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/


/******************************************************************************/
/************************ "private" API FUNCTIONS *****************************/
/******************************************************************************/

/**
 * get next sibling
 *
 * @param r a relation
 * @result next sibling of r or NULL
 */
Relation *_relation_next(Relation * r)
{
        if(!r)
                NFT_LOG_NULL(NULL);

        return r->next;
}


/**
 * get previous sibling
 *
 * @param r a relation
 * @result previous sibling of r or NULL
 */
Relation *_relation_prev(Relation * r)
{
        if(!r)
                NFT_LOG_NULL(NULL);

        return r->prev;
}


/**
 * get child of object
 *
 * @param r a relation
 * @result (first) child of relation or NULL
 */
Relation *_relation_child(Relation * r)
{
        if(!r)
                NFT_LOG_NULL(NULL);

        return r->child;
}


/**
 * get parent of object
 *
 * @param r a relation
 * @result parent of relation or NULL
 */
Relation *_relation_parent(Relation * r)
{
        if(!r)
                NFT_LOG_NULL(NULL);

        return r->parent;
}


/**
 * get head of (first sibling) list this relation is part of
 *
 * @param r a relation
 * @param first sibling of this relation
 */
Relation *_relation_first(Relation * r)
{
        if(!r)
                NFT_LOG_NULL(NULL);

        /* take the shortcut? */
        if(r->parent)
                /* first child of parent == head of this list */
                return r->parent->child;

        Relation *first;
        for(first = r; first->prev; first = first->prev);

        return first;
}


/**
 * return last sibling in list of objects
 *
 * @param object relation
 * @result last sibling of r or NULL. If r is the last sibling, r is returned.
 */
Relation *_relation_last(Relation * r)
{
        if(!r)
                NFT_LOG_NULL(NULL);

        Relation *last;
        for(last = r; last->next; last = last->next);

        return last;
}


/**
 * get nth sibling of this object
 *
 * @param h a LedHardware (probably the head of a linked list ;)
 * @param n position in list (starting from 0)
 * @result object at position n in list starting from r or NULL upon error
 */
Relation *_relation_nth(Relation * r, int n)
{
        if(!r)
                return NULL;

        if(n == 0)
                return r;

        return _relation_nth(r, n - 1);
}


/**
 * append object to end of sibling list
 *
 * @param p object relation
 * @param s object to append to last sibling of p
 * @result NFT_SUCCESS or NFT_FAILURE 
 */
NftResult _relation_append(Relation * p, Relation * s)
{
        if(!p)
                NFT_LOG_NULL(NFT_FAILURE);

        Relation *last;
        if(!(last = _relation_last(p)))
                return NFT_FAILURE;

        if(last == s)
        {
                NFT_LOG(L_ERROR, "Attempt to make us our own child");
                return NFT_FAILURE;
        }

        /* register next */
        last->next = s;

        /* register previous */
        if(s)
        {
                s->prev = last;
                s->parent = last->parent;
        }

        return NFT_SUCCESS;
}


/**
 * append object to last sibling of child object
 *
 * @param p parent
 * @param c child
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult _relation_append_child(Relation * p, Relation * c)
{
        if(!p)
                NFT_LOG_NULL(NFT_FAILURE);

        /* do we have a child, yet? */
        Relation *child;
        if(!(child = p->child))
        {
                /* make this first child */
                p->child = c;
        }
        /* append to last sibling of child */
        else
        {
                if(!_relation_append(child, c))
                        return NFT_FAILURE;
        }

        /* set parent */
        c->parent = p;

        return NFT_SUCCESS;
}


/**
 * clear relation structure of an object
 *
 * @param r relation to clear
 */
void _relation_clear(Relation * r)
{
        if(!r)
                NFT_LOG_NULL();

        memset(r, 0, sizeof(Relation));
}


/**
 * unlink an object from all relations before destruction
 *
 * @param r relation to unlink 
 */
void _relation_unlink(Relation * r)
{
        if(!r)
                NFT_LOG_NULL();

        /* unlink from linked-list of siblings */
        if(r->next)
                r->next->prev = r->prev;
        if(r->prev)
                r->prev->next = r->next;

        /* unlink from parent tile */
        if(r->parent)
        {
                /* if we are the first child of our parent, make our next
                 * sibling the new head */
                if(r->parent->child == r)
                        r->parent->child = r->next;
        }

        /* clear structure */
        _relation_clear(r);

}


/**
 * get amount of siblings this object has
 *
 * @param r object relation
 * @result amount of siblings r has
 */
int _relation_sibling_count(Relation * r)
{
        Relation *t = r;
        int i;
        for(i = 0; t; i++)
                t = t->next;

        return i - 1;
}


/**
 * do something for this and all sibling objects
 *
 * @param r object
 * @param func foreach function
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult _relation_foreach(Relation * r,
                            NftResult(*func) (Relation * r, void *userptr),
                            void *userptr)
{
        if(!r || !func)
                return NFT_FAILURE;

        Relation *t, *tmp;
        for(t = r; t; t = tmp)
        {
                tmp = _relation_next(t);

                if(!func(t, userptr))
                        return NFT_FAILURE;
        }

        return NFT_SUCCESS;
}


NftResult _relation_foreach_recursive(Relation * r,
                                      NftResult(*func) (Relation * r,
                                                        void *userptr));


/******************************************************************************/
/**************************** API FUNCTIONS ***********************************/
/******************************************************************************/

