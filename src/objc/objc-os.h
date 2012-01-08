/*
 * Copyright (C) 2011 Dmitry Skiba
 * Copyright (c) 2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
* objc-os.h
* OS portability layer.
**********************************************************************/

#ifndef _OBJC_OS_H
#define _OBJC_OS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "android/dyld.h"

#include <malloc/malloc.h>
#include <libkern/OSAtomic.h>

#include <objc/objc.h>
#include <objc/objc-api.h>

__BEGIN_DECLS

extern void _objc_fatal(const char *fmt, ...) __attribute__((noreturn, format (printf, 1, 2)));

#define require_action_string(cond, dest, act, msg) do { if (!(cond)) { { act; } goto dest; } } while (0)
#define require_noerr_string(err, dest, msg) do { if (err) goto dest; } while (0)
#define require_string(cond, dest, msg) do { if (!(cond)) goto dest; } while (0)

#define INIT_ONCE_PTR(var, create, delete)                              \
    do {                                                                \
        if (var) break;                                                 \
        typeof(var) v = create;                                         \
        while (!var) {                                                  \
            if (OSAtomicCompareAndSwapPtrBarrier(0, v, (void**)&var)) { \
                goto done;                                              \
            }                                                           \
        }                                                               \
        delete;                                                         \
    done:;                                                              \
    } while (0)

#define INIT_ONCE_32(var, create, delete)                               \
    do {                                                                \
        if (var) break;                                                 \
        typeof(var) v = create;                                         \
        while (!var) {                                                  \
            if (OSAtomicCompareAndSwap32Barrier(0, v, (volatile int32_t *)&var)) { \
                goto done;                                              \
            }                                                           \
        }                                                               \
        delete;                                                         \
    done:;                                                              \
    } while (0)


// Internal data types

typedef pthread_key_t tls_key_t;

static inline void tls_create(tls_key_t *k, void (*dtor)(void*)) { 
    pthread_key_create(k, dtor); 
}
static inline void *tls_get(tls_key_t k) { 
    return pthread_getspecific(k); 
}
static inline void tls_set(tls_key_t k, void *value) { 
    pthread_setspecific(k, value); 
}

typedef pthread_mutex_t mutex_t;
#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER;

extern int DebuggerMode;
extern void gdb_objc_debuggerModeFailure(void);
extern BOOL isManagedDuringDebugger(void *lock);
extern BOOL isLockedDuringDebugger(mutex_t *lock);

static inline int _mutex_lock_nodebug(mutex_t *m) { 
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger(m)) {
            gdb_objc_debuggerModeFailure();
        }
        return 0;
    }
    return pthread_mutex_lock(m); 
}
static inline int _mutex_try_lock_nodebug(mutex_t *m) { 
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger(m)) {
            gdb_objc_debuggerModeFailure();
        }
        return 1;
    }
    return !pthread_mutex_trylock(m); 
}
static inline int _mutex_unlock_nodebug(mutex_t *m) { 
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        return 0;
    }
    return pthread_mutex_unlock(m); 
}


typedef struct { 
    pthread_mutex_t *mutex; 
} recursive_mutex_t;
#define RECURSIVE_MUTEX_INITIALIZER {0};
#define RECURSIVE_MUTEX_NOT_LOCKED EPERM
extern void recursive_mutex_init(recursive_mutex_t *m);

static inline int _recursive_mutex_lock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger((mutex_t *)m)) {
            gdb_objc_debuggerModeFailure();
        }
        return 0;
    }
    return pthread_mutex_lock(m->mutex); 
}
static inline int _recursive_mutex_try_lock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger((mutex_t *)m)) {
            gdb_objc_debuggerModeFailure();
        }
        return 1;
    }
    return !pthread_mutex_trylock(m->mutex); 
}
static inline int _recursive_mutex_unlock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        return 0;
    }
    return pthread_mutex_unlock(m->mutex); 
}


typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} monitor_t;
#define MONITOR_INITIALIZER { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER }
#define MONITOR_NOT_ENTERED EPERM

static inline int monitor_init(monitor_t *c) {
    int err = pthread_mutex_init(&c->mutex, NULL);
    if (err) return err;
    err = pthread_cond_init(&c->cond, NULL);
    if (err) {
        pthread_mutex_destroy(&c->mutex);
        return err;
    }
    return 0;
}
static inline int _monitor_enter_nodebug(monitor_t *c) {
    assert(!isManagedDuringDebugger(c));
    return pthread_mutex_lock(&c->mutex);
}
static inline int _monitor_exit_nodebug(monitor_t *c) {
    return pthread_mutex_unlock(&c->mutex);
}
static inline int _monitor_wait_nodebug(monitor_t *c) { 
    return pthread_cond_wait(&c->cond, &c->mutex);
}
static inline int monitor_notify(monitor_t *c) { 
    return pthread_cond_signal(&c->cond);
}
static inline int monitor_notifyAll(monitor_t *c) { 
    return pthread_cond_broadcast(&c->cond);
}


//TODO implement rwlock_t as pthread_rwlock_t

typedef struct { 
    pthread_mutex_t mutex; 
} rwlock_t;

static inline void rwlock_init(rwlock_t *l)
{
    int error=pthread_mutex_init(&l->mutex,NULL);
    if (error) {
        _objc_fatal("Failed to create rwlock, error: %d.",error);
    }
}

static inline void _rwlock_read_nodebug(rwlock_t *l)
{
    pthread_mutex_lock(&l->mutex);
}

static inline void _rwlock_unlock_read_nodebug(rwlock_t *l)
{
    pthread_mutex_unlock(&l->mutex);
}

static inline int _rwlock_try_read_nodebug(rwlock_t *l)
{
    return !pthread_mutex_trylock(&l->mutex);
}

static inline void _rwlock_write_nodebug(rwlock_t *l)
{
    pthread_mutex_lock(&l->mutex);
}

static inline void _rwlock_unlock_write_nodebug(rwlock_t *l)
{
    pthread_mutex_unlock(&l->mutex);
}

static inline int _rwlock_try_write_nodebug(rwlock_t *l)
{
    return !pthread_mutex_trylock(&l->mutex);
}


#ifndef __LP64__
typedef struct mach_header headerType;
typedef struct segment_command segmentType;
typedef struct section sectionType;
#else
typedef struct mach_header_64 headerType;
typedef struct segment_command_64 segmentType;
typedef struct section_64 sectionType;
#endif
#define headerIsBundle(hi) (hi->mhdr->filetype == MH_BUNDLE)
#define libobjc_header ((headerType *)&_mh_dylib_header)

typedef struct {
    Dl_info             dl_info;
    const segmentType * objcSegmentHeader;
    const segmentType * dataSegmentHeader;
    ptrdiff_t           image_slide;
} os_header_info;

// Prototypes

#if defined(MESSAGE_LOGGING)
/* Secure /tmp usage */
extern int secure_open(const char *filename, int flags, uid_t euid);
#endif

__END_DECLS

#endif
