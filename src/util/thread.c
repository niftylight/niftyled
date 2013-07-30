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
 * @file thread.c
 */


#include <stdlib.h>
#include <stdbool.h>
#include <niftylog.h>
#include "_thread.h"


#define THREAD_MODEL_POSIX 1
#define HAVE_THREADS 1



#ifdef HAVE_THREADS
#ifdef THREAD_MODEL_POSIX
#include <pthread.h>
#elif defined(THREAD_MODEL_GTHREAD2)    /* !THREAD_MODEL_POSIX */
#include <glib/gthread.h>
#else /* !THREAD_MODEL_GTHREAD2 */


#endif
#endif /* HAVE_THREADS */




/**
 * The Thread data structure and the Thread subsystem is a wrapper system for native
 * threading implementations.
 */
struct _Thread
{
#ifdef HAVE_THREADS
#ifdef THREAD_MODEL_POSIX
        pthread_t thread;               /**< Private used for the pthread implementation. */
#elif defined(THREAD_MODEL_WIN32)       /* !THREAD_MODEL_POSIX */
        HANDLE thread;
        DWORD threadId;
#elif defined(THREAD_MODEL_GTHREAD)     /* !THREAD_MODEL_WIN32 */
        GThread *thread;
#endif
#endif /* HAVE_THREADS */
};


/**
 * The Mutex data structure and the Mutex subsystem is a wrapper system for native
 * thread locking implementations.
 */
struct _Mutex
{
#ifdef HAVE_THREADS
#ifdef THREAD_MODEL_POSIX
        pthread_mutex_t mutex;
#elif defined(THREAD_MODEL_WIN32)       /* !THREAD_MODEL_POSIX */

#elif defined(THREAD_MODEL_GTHREAD)     /* !THREAD_MODEL_WIN32 */
        GMutex *mutex;
        GStaticMutex static_mutex;
        int static_mutex_used;
#endif
#endif /* HAVE_THREADS */
};





Thread *thread_create(ThreadFunc func, void *data, bool joinable)
{
        Thread *thread = NULL;

        if(!(thread = calloc(0, sizeof(Thread))))
        {
                NFT_LOG_PERROR("calloc");
                return NULL;
        }

#if defined(THREAD_MODEL_POSIX)
        pthread_attr_t attr;
        int res;

        pthread_attr_init(&attr);

        if(joinable)
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        else
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        res = pthread_create(&thread->thread, &attr, func, data);

        pthread_attr_destroy(&attr);

        if(res != 0)
        {
                NFT_LOG(L_ERROR, "Error while creating thread");
                free(thread);
                return NULL;
        }

#endif

        return thread;
}


void thread_free(Thread * thread)
{
        return free(thread);
}


void *thread_join(Thread * thread)
{
        void *result = NULL;

#if defined(THREAD_MODEL_POSIX)
        if(pthread_join(thread->thread, &result) < 0)
        {
                NFT_LOG(L_ERROR, "Error while joining thread");
                return NULL;
        }
#endif

        return result;
}

void thread_exit(void *retval)
{
#if defined(THREAD_MODEL_POSIX)
        pthread_exit(retval);
#endif
}


/**
 * Creates a new Mutex that is used to do thread locking so data
 * can be synchronized. 
 *
 * @note use thread_mutex_free() to finalize the Mutex 
 * @result A newly allocated Mutex that can be used with the thread_mutex_lock() and
 *	thread_mutex_unlock() functions or NULL on failure.
 */
Mutex *thread_mutex_new(void)
{
        Mutex *r;
        if(!(r = calloc(1, sizeof(Mutex))))
        {
                NFT_LOG_PERROR("calloc");
                return NULL;
        }

#if defined(THREAD_MODEL_POSIX)
        if(pthread_mutex_init(&r->mutex, NULL) != 0)
        {
                NFT_LOG_PERROR("failed to init mutex");
                free(r);
                return NULL;
        }
#endif

        return r;
}


/**
 * A Mutex is allocated to have more flexibility with the actual
 * thread backend. Thus they need to be freed as well.
 *
 * @param mutex Pointer to the Mutex that needs to be freed.
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult thread_mutex_free(Mutex * mutex)
{

        thread_mutex_unlock(mutex);

#if defined(THREAD_MODEL_POSIX)
        pthread_mutex_destroy(&mutex->mutex);
#endif


        free(mutex);

        return NFT_SUCCESS;
}


/**
 * lock mutex
 */
NftResult thread_mutex_lock(Mutex * mutex)
{

#if defined(THREAD_MODEL_POSIX)
        if(pthread_mutex_lock(&mutex->mutex) < 0)
                return NFT_FAILURE;
#endif

        return NFT_SUCCESS;
}


/**
 * unlock mutex
 */
NftResult thread_mutex_unlock(Mutex * mutex)
{

#if defined(THREAD_MODEL_POSIX)
        if(pthread_mutex_unlock(&mutex->mutex) < 0)
                return NFT_FAILURE;
#endif

        return NFT_SUCCESS;
}
