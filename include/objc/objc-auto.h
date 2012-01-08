/*
 * Copyright (C) 2011 Dmitry Skiba
 * Copyright (c) 2004-2007 Apple Inc. All rights reserved.
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

#ifndef _OBJC_AUTO_H_
#define _OBJC_AUTO_H_

#include <objc/objc.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>


#define OBJC_NO_GC 1

/* objc_collect() options */
enum {
    // choose one
    OBJC_RATIO_COLLECTION        = (0 << 0),  // run "ratio" generational collections, then a full
    OBJC_GENERATIONAL_COLLECTION = (1 << 0),  // run fast incremental collection
    OBJC_FULL_COLLECTION         = (2 << 0),  // run full collection.
    OBJC_EXHAUSTIVE_COLLECTION   = (3 << 0),  // run full collections until memory available stops improving
    
    OBJC_COLLECT_IF_NEEDED       = (1 << 3), // run collection only if needed (allocation threshold exceeded)
    OBJC_WAIT_UNTIL_DONE         = (1 << 4), // wait (when possible) for collection to end before returning (when collector is running on dedicated thread)
};

/* objc_clear_stack() options */
enum {
    OBJC_CLEAR_RESIDENT_STACK = (1 << 0)
};

static OBJC_INLINE void objc_collect(unsigned long options) { }
static OBJC_INLINE BOOL objc_collectingEnabled(void) { return NO; }
static OBJC_INLINE void objc_setCollectionThreshold(size_t threshold) { }
static OBJC_INLINE void objc_setCollectionRatio(size_t ratio) { }
static OBJC_INLINE void objc_startCollectorThread(void) { }

OBJC_EXPORT BOOL objc_atomicCompareAndSwapPtr(id predicate, id replacement, volatile id *objectLocation);
OBJC_EXPORT BOOL objc_atomicCompareAndSwapPtrBarrier(id predicate, id replacement, volatile id *objectLocation);

static OBJC_INLINE BOOL objc_atomicCompareAndSwapGlobal(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation); }

static OBJC_INLINE BOOL objc_atomicCompareAndSwapGlobalBarrier(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtrBarrier(predicate, replacement, objectLocation); }

static OBJC_INLINE BOOL objc_atomicCompareAndSwapInstanceVariable(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtr(predicate, replacement, objectLocation); }

static OBJC_INLINE BOOL objc_atomicCompareAndSwapInstanceVariableBarrier(id predicate, id replacement, volatile id *objectLocation) 
    { return objc_atomicCompareAndSwapPtrBarrier(predicate, replacement, objectLocation); }


static OBJC_INLINE id objc_assign_strongCast(id val, id *dest) 
    { return (*dest = val); }

static OBJC_INLINE id objc_assign_global(id val, id *dest) 
    { return (*dest = val); }

static OBJC_INLINE id objc_assign_ivar(id val, id dest, ptrdiff_t offset) 
    { return (*(id*)((char *)dest+offset) = val); }

static OBJC_INLINE void *objc_memmove_collectable(void *dst, const void *src, size_t size) 
    { return memmove(dst, src, size); }

static OBJC_INLINE id objc_read_weak(id *location) 
    { return *location; }

static OBJC_INLINE id objc_assign_weak(id value, id *location) 
    { return (*location = value); }


static OBJC_INLINE void objc_finalizeOnMainThread(Class cls) { }
static OBJC_INLINE BOOL objc_is_finalized(void *ptr) { return NO; }
static OBJC_INLINE void objc_clear_stack(unsigned long options) { }

static OBJC_INLINE BOOL objc_collecting_enabled(void) { return NO; }
static OBJC_INLINE void objc_set_collection_threshold(size_t threshold) { } 
static OBJC_INLINE void objc_set_collection_ratio(size_t ratio) { } 
static OBJC_INLINE void objc_start_collector_thread(void) { }

OBJC_EXPORT id class_createInstance(Class cls, size_t extraBytes);
static OBJC_INLINE id objc_allocate_object(Class cls, int extra) 
    { return class_createInstance(cls, extra); }

static OBJC_INLINE void objc_registerThreadWithCollector() { }
static OBJC_INLINE void objc_unregisterThreadWithCollector() { }
static OBJC_INLINE void objc_assertRegisteredThreadWithCollector() { }

#endif /* _OBJC_AUTO_H_ */
