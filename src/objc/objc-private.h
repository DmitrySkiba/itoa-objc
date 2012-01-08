/*
 * Copyright (C) 2011 Dmitry Skiba
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
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
/*
 *    objc-private.h
 *    Copyright 1988-1996, NeXT Software, Inc.
 */

#ifndef _OBJC_PRIVATE_H_
#define _OBJC_PRIVATE_H_

#include "objc-os.h"

#include <objc/objc.h>
#include <objc/objc-api.h>
#include <objc/runtime.h>
#include <objc/objc-auto.h>
#include <objc/objc-internal.h>
#include "maptable.h"
#include "hashtable2.h"
#include "objc-config.h"
#include "objc-references.h"
#include "objc-initialize.h"
#include "objc-loadmethod.h"


__BEGIN_DECLS

struct old_category;
struct old_method_list;
typedef struct {
    IMP imp;
    SEL sel;
} message_ref;

typedef struct objc_module *Module;
typedef struct objc_cache *Cache;


#if OLD

struct old_class {
    struct old_class *isa;
    struct old_class *super_class;
    const char *name;
    long version;
    long info;
    long instance_size;
    struct old_ivar_list *ivars;
    struct old_method_list **methodLists;
    Cache cache;
    struct old_protocol_list *protocols;
    // CLS_EXT only
    const char *ivar_layout;
    struct old_class_ext *ext;
};

struct old_class_ext {
    uint32_t size;
    const char *weak_ivar_layout;
    struct objc_property_list **propertyLists;
};

struct old_category {
    char *category_name;
    char *class_name;
    struct old_method_list *instance_methods;
    struct old_method_list *class_methods;
    struct old_protocol_list *protocols;
    uint32_t size;
    struct objc_property_list *instance_properties;
};

struct old_ivar {
    char *ivar_name;
    char *ivar_type;
    int ivar_offset;
#ifdef __LP64__
    int space;
#endif
};

struct old_ivar_list {
    int ivar_count;
#ifdef __LP64__
    int space;
#endif
    /* variable length structure */
    struct old_ivar ivar_list[1];
};


struct old_method {
    SEL method_name;
    char *method_types;
    IMP method_imp;
};

struct old_method_list {
    struct old_method_list *obsolete;

    int method_count;
#ifdef __LP64__
    int space;
#endif
    /* variable length structure */
    struct old_method method_list[1];
};

struct old_protocol {
    Class isa;
    const char *protocol_name;
    struct old_protocol_list *protocol_list;
    struct objc_method_description_list *instance_methods;
    struct objc_method_description_list *class_methods;
};

struct old_protocol_list {
    struct old_protocol_list *next;
    long count;
    struct old_protocol *list[1];
};

struct old_protocol_ext {
    uint32_t size;
    struct objc_method_description_list *optional_instance_methods;
    struct objc_method_description_list *optional_class_methods;
    struct objc_property_list *instance_properties;
};

#endif /* OLD */

typedef objc_property_t Property;

struct objc_property {
    const char *name;
    const char *attributes;
};

struct objc_property_list {
    uint32_t entsize;
    uint32_t count;
    struct objc_property first;
};

typedef struct {
    uint32_t version; // currently 0
    uint32_t flags;
} objc_image_info;

// masks for objc_image_info.flags
#define OBJC_IMAGE_IS_REPLACEMENT (1<<0)
#define OBJC_IMAGE_SUPPORTS_GC (1<<1)
#define OBJC_IMAGE_REQUIRES_GC (1<<2)
#define OBJC_IMAGE_OPTIMIZED_BY_DYLD (1<<3)


#define _objcHeaderIsReplacement(h)  ((h)->info  &&  ((h)->info->flags & OBJC_IMAGE_IS_REPLACEMENT))

/* OBJC_IMAGE_IS_REPLACEMENT:
   Don't load any classes
   Don't load any categories
   Do fix up selector refs (@selector points to them)
   Do fix up class refs (@class and objc_msgSend points to them)
   Do fix up protocols (@protocol points to them)
   Do fix up super_class pointers in classes ([super ...] points to them)
   Future: do load new classes?
   Future: do load new categories?
   Future: do insert new methods on existing classes?
   Future: do insert new methods on existing categories?
*/

#define _objcInfoSupportsGC(info) (((info)->flags & OBJC_IMAGE_SUPPORTS_GC) ? 1 : 0)
#define _objcInfoRequiresGC(info) (((info)->flags & OBJC_IMAGE_REQUIRES_GC) ? 1 : 0)
#define _objcHeaderSupportsGC(h) ((h)->info && _objcInfoSupportsGC((h)->info))
#define _objcHeaderRequiresGC(h) ((h)->info && _objcInfoRequiresGC((h)->info))

/* OBJC_IMAGE_SUPPORTS_GC:
    was compiled with -fobjc-gc flag, regardless of whether write-barriers were issued
    if executable image compiled this way, then all subsequent libraries etc. must also be this way
*/

#define _objcHeaderOptimizedByDyld(h)  ((h)->info  &&  ((h)->info->flags & OBJC_IMAGE_OPTIMIZED_BY_DYLD))

/* OBJC_IMAGE_OPTIMIZED_BY_DYLD:
   Assorted metadata precooked in the dyld shared cache.
   Never set for images outside the shared cache file itself.
*/
   

typedef struct _header_info {
    struct _header_info *next;
    const headerType *  mhdr;
    struct objc_module *mod_ptr;
    size_t              mod_count;
    const objc_image_info *info;
    BOOL                allClassesRealized;
    os_header_info      os;
} header_info;

extern header_info *FirstHeader;
extern header_info *LastHeader;
extern int HeaderCount;

extern void _objc_appendHeader(header_info *hi);
extern void _objc_removeHeader(header_info *hi);
extern const char *_nameForHeader(const headerType*);

extern objc_image_info *_getObjcImageInfo(const headerType *head, ptrdiff_t slide, size_t *size);
extern const char *_getObjcHeaderName(const headerType *head);
extern ptrdiff_t _getImageSlide(const headerType *header);


extern Module _getObjcModules(const header_info *hi, size_t *count);
extern SEL *_getObjcSelectorRefs(const header_info *hi, size_t *count);
extern BOOL _hasObjcContents(const header_info *hi);

extern SEL *_getObjc2SelectorRefs(const header_info *hi, size_t *count);
extern message_ref *_getObjc2MessageRefs(const header_info *hi, size_t *count);extern struct class_t **_getObjc2ClassRefs(const header_info *hi, size_t *count);
extern struct class_t **_getObjc2SuperRefs(const header_info *hi, size_t *count);
extern struct class_t **_getObjc2ClassList(const header_info *hi, size_t *count);
extern struct class_t **_getObjc2NonlazyClassList(const header_info *hi, size_t *count);
extern struct category_t **_getObjc2CategoryList(const header_info *hi, size_t *count);
extern struct category_t **_getObjc2NonlazyCategoryList(const header_info *hi, size_t *count);
extern struct protocol_t **_getObjc2ProtocolList(const header_info *head, size_t *count);
extern struct protocol_t **_getObjc2ProtocolRefs(const header_info *head, size_t *count);

#define END_OF_METHODS_LIST ((struct old_method_list*)-1)


/* selectors */
extern void sel_init(BOOL gc);
extern SEL sel_registerNameNoLock(const char *str, BOOL copy);
extern void sel_lock(void);
extern void sel_unlock(void);
extern BOOL sel_preoptimizationValid(const header_info *hi);
extern void disableSelectorPreoptimization(void);

extern SEL SEL_load;
extern SEL SEL_initialize;
extern SEL SEL_resolveClassMethod;
extern SEL SEL_resolveInstanceMethod;
extern SEL SEL_cxx_construct;
extern SEL SEL_cxx_destruct;
extern SEL SEL_retain;
extern SEL SEL_release;
extern SEL SEL_autorelease;
extern SEL SEL_copy;
extern SEL SEL_finalize;


/* optional malloc zone for runtime data */
extern malloc_zone_t *_objc_internal_zone(void);
extern void *_malloc_internal(size_t size);
extern void *_calloc_internal(size_t count, size_t size);
extern void *_realloc_internal(void *ptr, size_t size);
extern char *_strdup_internal(const char *str);
extern char *_strdupcat_internal(const char *s1, const char *s2);
extern void *_memdup_internal(const void *mem, size_t size);
extern void _free_internal(void *ptr);

extern Class _calloc_class(size_t size);

extern IMP lookUpMethod(Class, SEL, BOOL initialize, BOOL cache);
extern void lockForMethodLookup(void);
extern void unlockForMethodLookup(void);
extern IMP prepareForMethodLookup(Class cls, SEL sel, BOOL initialize);

extern IMP _cache_getImp(Class cls, SEL sel);
extern Method _cache_getMethod(Class cls, SEL sel, IMP objc_msgForward_internal_imp);

/* message dispatcher */
extern IMP _class_lookupMethodAndLoadCache(Class, SEL);
extern id _objc_msgForward_internal(id self, SEL sel, ...);
extern id _objc_ignored_method(id self, SEL _cmd);

/* errors */
extern void __objc_error(id, const char *, ...) __attribute__((format (printf, 2, 3)));
extern void _objc_inform(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
extern void _objc_inform_on_crash(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
extern void _objc_inform_now_and_on_crash(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
extern void _objc_warn_deprecated(const char *oldname, const char *newname) __attribute__((noinline));
extern void _objc_error(id, const char *, va_list);
extern void inform_duplicate(const char *name, Class oldCls, Class cls);
extern void _objc_syslog(const char *msg);

/* magic */
extern Class _objc_getFreedObjectClass (void);
#ifndef OBJC_INSTRUMENTED
extern const struct objc_cache _objc_empty_cache;
#else
extern struct objc_cache _objc_empty_cache;
#endif
extern IMP _objc_empty_vtable[128];

/* map table additions */
extern void *NXMapKeyCopyingInsert(NXMapTable *table, const void *key, const void *value);
extern void *NXMapKeyFreeingRemove(NXMapTable *table, const void *key);

/* locking */
/* Every lock used anywhere must be declared here. 
 * Locks not declared here may cause gdb deadlocks. */
extern void lock_init(void);
extern rwlock_t selLock;
extern mutex_t cacheUpdateLock;
extern recursive_mutex_t loadMethodLock;
extern rwlock_t runtimeLock;
extern mutex_t classLock;
extern mutex_t methodListLock;

/* Debugger mode for gdb */
#define DEBUGGER_OFF 0
#define DEBUGGER_PARTIAL 1
#define DEBUGGER_FULL 2
extern int startDebuggerMode(void);
extern void endDebuggerMode(void);

#if defined(NDEBUG)  ||  TARGET_OS_WIN32

#define _destroyLockList(x)           do { } while (0)

#define mutex_lock(m)             _mutex_lock_nodebug(m)
#define mutex_try_lock(m)         _mutex_try_lock_nodebug(m)
#define mutex_unlock(m)           _mutex_unlock_nodebug(m)
#define mutex_assert_locked(m)    do { } while (0)
#define mutex_assert_unlocked(m)  do { } while (0)

#define recursive_mutex_lock(m)             _recursive_mutex_lock_nodebug(m)
#define recursive_mutex_try_lock(m)         _recursive_mutex_try_lock_nodebug(m)
#define recursive_mutex_unlock(m)           _recursive_mutex_unlock_nodebug(m)
#define recursive_mutex_assert_locked(m)    do { } while (0)
#define recursive_mutex_assert_unlocked(m)  do { } while (0)

#define monitor_enter(m)            _monitor_enter_nodebug(m)
#define monitor_exit(m)             _monitor_exit_nodebug(m)
#define monitor_wait(m)             _monitor_wait_nodebug(m)
#define monitor_assert_locked(m)    do { } while (0)
#define monitor_assert_unlocked(m)  do { } while (0)

#define rwlock_read(m)              _rwlock_read_nodebug(m)
#define rwlock_write(m)             _rwlock_write_nodebug(m)
#define rwlock_try_read(m)          _rwlock_try_read_nodebug(m)
#define rwlock_try_write(m)         _rwlock_try_write_nodebug(m)
#define rwlock_unlock_read(m)       _rwlock_unlock_read_nodebug(m)
#define rwlock_unlock_write(m)      _rwlock_unlock_write_nodebug(m)
#define rwlock_assert_reading(m)    do { } while (0)
#define rwlock_assert_writing(m)    do { } while (0)
#define rwlock_assert_locked(m)     do { } while (0)
#define rwlock_assert_unlocked(m)   do { } while (0)

#else

struct _objc_lock_list;
extern void _destroyLockList(struct _objc_lock_list *locks);

extern int _mutex_lock_debug(mutex_t *lock, const char *name);
extern int _mutex_try_lock_debug(mutex_t *lock, const char *name);
extern int _mutex_unlock_debug(mutex_t *lock, const char *name);
extern void _mutex_assert_locked_debug(mutex_t *lock, const char *name);
extern void _mutex_assert_unlocked_debug(mutex_t *lock, const char *name);

extern int _recursive_mutex_lock_debug(recursive_mutex_t *lock, const char *name);
extern int _recursive_mutex_try_lock_debug(recursive_mutex_t *lock, const char *name);
extern int _recursive_mutex_unlock_debug(recursive_mutex_t *lock, const char *name);
extern void _recursive_mutex_assert_locked_debug(recursive_mutex_t *lock, const char *name);
extern void _recursive_mutex_assert_unlocked_debug(recursive_mutex_t *lock, const char *name);

extern int _monitor_enter_debug(monitor_t *lock, const char *name);
extern int _monitor_exit_debug(monitor_t *lock, const char *name);
extern int _monitor_wait_debug(monitor_t *lock, const char *name);
extern void _monitor_assert_locked_debug(monitor_t *lock, const char *name);
extern void _monitor_assert_unlocked_debug(monitor_t *lock, const char *name);

extern void _rwlock_read_debug(rwlock_t *l, const char *name);
extern void _rwlock_write_debug(rwlock_t *l, const char *name);
extern int  _rwlock_try_read_debug(rwlock_t *l, const char *name);
extern int  _rwlock_try_write_debug(rwlock_t *l, const char *name);
extern void _rwlock_unlock_read_debug(rwlock_t *l, const char *name);
extern void _rwlock_unlock_write_debug(rwlock_t *l, const char *name);
extern void _rwlock_assert_reading_debug(rwlock_t *l, const char *name);
extern void _rwlock_assert_writing_debug(rwlock_t *l, const char *name);
extern void _rwlock_assert_locked_debug(rwlock_t *l, const char *name);
extern void _rwlock_assert_unlocked_debug(rwlock_t *l, const char *name);

#define mutex_lock(m)             _mutex_lock_debug (m, #m)
#define mutex_try_lock(m)         _mutex_try_lock_debug (m, #m)
#define mutex_unlock(m)           _mutex_unlock_debug (m, #m)
#define mutex_assert_locked(m)    _mutex_assert_locked_debug (m, #m)
#define mutex_assert_unlocked(m)  _mutex_assert_unlocked_debug (m, #m)

#define recursive_mutex_lock(m)             _recursive_mutex_lock_debug (m, #m)
#define recursive_mutex_try_lock(m)         _recursive_mutex_try_lock_debug (m, #m)
#define recursive_mutex_unlock(m)           _recursive_mutex_unlock_debug (m, #m)
#define recursive_mutex_assert_locked(m)    _recursive_mutex_assert_locked_debug (m, #m)
#define recursive_mutex_assert_unlocked(m)  _recursive_mutex_assert_unlocked_debug (m, #m)

#define monitor_enter(m)            _monitor_enter_debug(m, #m)
#define monitor_exit(m)             _monitor_exit_debug(m, #m)
#define monitor_wait(m)             _monitor_wait_debug(m, #m)
#define monitor_assert_locked(m)    _monitor_assert_locked_debug(m, #m)
#define monitor_assert_unlocked(m)  _monitor_assert_unlocked_debug(m, #m)

#define rwlock_read(m)              _rwlock_read_debug(m, #m)
#define rwlock_write(m)             _rwlock_write_debug(m, #m)
#define rwlock_try_read(m)          _rwlock_try_read_debug(m, #m)
#define rwlock_try_write(m)         _rwlock_try_write_debug(m, #m)
#define rwlock_unlock_read(m)       _rwlock_unlock_read_debug(m, #m)
#define rwlock_unlock_write(m)      _rwlock_unlock_write_debug(m, #m)
#define rwlock_assert_reading(m)    _rwlock_assert_reading_debug(m, #m)
#define rwlock_assert_writing(m)    _rwlock_assert_writing_debug(m, #m)
#define rwlock_assert_locked(m)     _rwlock_assert_locked_debug(m, #m)
#define rwlock_assert_unlocked(m)   _rwlock_assert_unlocked_debug(m, #m)

#endif

#define rwlock_unlock(m, s)                           \
    do {                                              \
        if ((s) == RDONLY) rwlock_unlock_read(m);     \
        else if ((s) == RDWR) rwlock_unlock_write(m); \
    } while (0)


extern NXHashTable *class_hash;

/* forward handler functions */
extern void *_objc_forward_handler;
extern void *_objc_forward_stret_handler;

/* GC and RTP startup */
extern void gc_init(BOOL on);
extern void rtp_init(void);

/* Class change notifications (gdb only for now) */
#define OBJC_CLASS_ADDED (1<<0)
#define OBJC_CLASS_REMOVED (1<<1)
#define OBJC_CLASS_IVARS_CHANGED (1<<2)
#define OBJC_CLASS_METHODS_CHANGED (1<<3)
extern void gdb_objc_class_changed(Class cls, unsigned long changes, const char *classname)
    __attribute__((noinline));

/* Code modification */
//TODO remove
extern size_t objc_branch_size(void *entry, void *target);
extern size_t objc_write_branch(void *entry, void *target);
extern size_t objc_cond_branch_size(void *entry, void *target, unsigned cond);
extern size_t objc_write_cond_branch(void *entry, void *target, unsigned cond);
#if defined(__ppc__)  ||  defined(__ppc64__)
#define COND_ALWAYS 0x02800000  /* BO=10100, BI=00000 */
#define COND_NE     0x00820000  /* BO=00100, BI=00010 */
#elif defined(__i386__) || defined(__x86_64__)
#define COND_ALWAYS 0xE9  /* JMP rel32 */
#define COND_NE     0x85  /* JNE rel32  (0F 85) */
#endif



#if !defined(SEG_OBJC)
#define SEG_OBJC        "__OBJC"        /* objective-C runtime segment */
#endif


// Settings from environment variables
#if NO_ENVIRON
#   define ENV(x) enum { x = 0 }
#else
#   define ENV(x) extern int x
#endif
ENV(PrintImages);               // env OBJC_PRINT_IMAGES
ENV(PrintLoading);              // env OBJC_PRINT_LOAD_METHODS
ENV(PrintInitializing);         // env OBJC_PRINT_INITIALIZE_METHODS
ENV(PrintResolving);            // env OBJC_PRINT_RESOLVED_METHODS
ENV(PrintConnecting);           // env OBJC_PRINT_CLASS_SETUP
ENV(PrintProtocols);            // env OBJC_PRINT_PROTOCOL_SETUP
ENV(PrintIvars);                // env OBJC_PRINT_IVAR_SETUP
ENV(PrintVtables);              // env OBJC_PRINT_VTABLE_SETUP
ENV(PrintVtableImages);         // env OBJC_PRINT_VTABLE_IMAGES
ENV(PrintFuture);               // env OBJC_PRINT_FUTURE_CLASSES
ENV(PrintRTP);                  // env OBJC_PRINT_RTP
ENV(PrintGC);                   // env OBJC_PRINT_GC
ENV(PrintPreopt);               // env OBJC_PRINT_PREOPTIMIZATION
ENV(PrintCxxCtors);             // env OBJC_PRINT_CXX_CTORS
ENV(PrintExceptions);           // env OBJC_PRINT_EXCEPTIONS
ENV(PrintDeprecation);          // env OBJC_PRINT_DEPRECATION_WARNINGS
ENV(PrintReplacedMethods);      // env OBJC_PRINT_REPLACED_METHODS
ENV(PrintCaches);               // env OBJC_PRINT_CACHE_SETUP
ENV(UseInternalZone);           // env OBJC_USE_INTERNAL_ZONE

ENV(DebugUnload);               // env OBJC_DEBUG_UNLOAD
ENV(DebugFragileSuperclasses);  // env OBJC_DEBUG_FRAGILE_SUPERCLASSES
ENV(DebugFinalizers);           // env OBJC_DEBUG_FINALIZERS
ENV(DebugNilSync);              // env OBJC_DEBUG_NIL_SYNC

ENV(DisableVtables);            // env OBJC_DISABLE_VTABLES
ENV(DisablePreopt);             // env OBJC_DISABLE_PREOPTIMIZATION
#undef ENV

extern void logReplacedMethod(const char *className, SEL s, BOOL isMeta, const char *catName, IMP oldImp, IMP newImp);


static __inline int _objc_strcmp(const char *s1, const char *s2) {
    char c1, c2;
    for ( ; (c1 = *s1) == (c2 = *s2); s1++, s2++)
        if (c1 == '\0')
            return 0;
    return (c1 - c2);
}       

static __inline uint32_t _objc_strhash(const char *s) {
    uint32_t hash = 0;
    for (;;) {
    int a = *s++;
    if (0 == a) break;
    hash += (hash << 8) + a;
    }
    return hash;
}


// objc per-thread storage
typedef struct {
    struct _objc_initializing_classes *initializingClasses; // for +initialize
    struct _objc_lock_list *lockList;  // for lock debugging
    struct SyncCache *syncCache;  // for @synchronize
    struct alt_handler_list *handlerList;  // for exception alt handlers

    // If you add new fields here, don't forget to update 
    // _objc_pthread_destroyspecific()

} _objc_pthread_data;

extern _objc_pthread_data *_objc_fetch_pthread_data(BOOL create);
extern void tls_init(void);



// Attribute for global variables to keep them out of bss storage
// To save one page per non-Objective-C process, variables used in 
// the "Objective-C not used" case should not be in bss storage.
// On Tiger, this reduces the number of touched pages for each 
// CoreFoundation-only process from three to two. See #3857126 and #3857136.
#define NOBSS //__attribute__((section("__DATA,__data")))

// cache.h
static inline int isPowerOf2(unsigned long l) { return 1 == __builtin_popcountl(l); }
extern void flush_caches(Class cls, BOOL flush_meta);
extern void flush_cache(Class cls);
extern BOOL _cache_fill(Class cls, Method smt, SEL sel);
extern void _cache_addForwardEntry(Class cls, SEL sel);
extern void _cache_free(Cache cache);

extern mutex_t cacheUpdateLock;

// encoding.h
extern unsigned int encoding_getNumberOfArguments(const char *typedesc);
extern unsigned int encoding_getSizeOfArguments(const char *typedesc);
extern unsigned int encoding_getArgumentInfo(const char *typedesc, int arg, const char **type, int *offset);
extern void encoding_getReturnType(const char *t, char *dst, size_t dst_len);
extern char * encoding_copyReturnType(const char *t);
extern void encoding_getArgumentType(const char *t, unsigned int index, char *dst, size_t dst_len);
extern char *encoding_copyArgumentType(const char *t, unsigned int index);

// sync.h
extern void _destroySyncCache(struct SyncCache *cache);

// layout.h
typedef struct {
    uint8_t *bits;
    size_t bitCount;
    size_t bitsAllocated;
    BOOL weak;
} layout_bitmap;
extern layout_bitmap layout_bitmap_create(const unsigned char *layout_string, size_t layoutStringInstanceSize, size_t instanceSize, BOOL weak);
extern void layout_bitmap_free(layout_bitmap bits);
extern const unsigned char *layout_string_create(layout_bitmap bits);
extern void layout_bitmap_set_ivar(layout_bitmap bits, const char *type, size_t offset);
extern void layout_bitmap_grow(layout_bitmap *bits, size_t newCount);
extern void layout_bitmap_slide(layout_bitmap *bits, size_t oldPos, size_t newPos);
extern BOOL layout_bitmap_splat(layout_bitmap dst, layout_bitmap src, 
                                size_t oldSrcInstanceSize);
extern BOOL layout_bitmap_or(layout_bitmap dst, layout_bitmap src, const char *msg);
extern BOOL layout_bitmap_clear(layout_bitmap dst, layout_bitmap src, const char *msg);
extern void layout_bitmap_print(layout_bitmap bits);


// fixme runtime
extern id look_up_class(const char *aClassName, BOOL includeUnconnected, BOOL includeClassHandler);
extern const char *map_images(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info infoList[]);
extern const char *map_images_nolock(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info infoList[]);
extern const char * load_images(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info infoList[]);
extern BOOL load_images_nolock(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info infoList[]);
extern void unmap_image(const struct mach_header *mh, intptr_t vmaddr_slide);
extern void unmap_image_nolock(const struct mach_header *mh, intptr_t vmaddr_slide);
extern void _read_images(header_info **hList, uint32_t hCount);
extern void prepare_load_methods(header_info *hi);
extern void _unload_image(header_info *hi);
extern const char ** _objc_copyClassNamesForImage(header_info *hi, unsigned int *outCount);

extern Class _objc_allocateFutureClass(const char *name);


extern Property *copyPropertyList(struct objc_property_list *plist, unsigned int *outCount);


extern const header_info *_headerForClass(Class cls);

// fixme class
extern Property property_list_nth(const struct objc_property_list *plist, uint32_t i);
extern size_t property_list_size(const struct objc_property_list *plist);

extern Class _class_getSuperclass(Class cls);
extern BOOL _class_getInfo(Class cls, int info);
extern const char *_class_getName(Class cls);
extern size_t _class_getInstanceSize(Class cls);
extern Class _class_getMeta(Class cls);
extern BOOL _class_isMetaClass(Class cls);
extern Cache _class_getCache(Class cls);
extern void _class_setCache(Class cls, Cache cache);
extern BOOL _class_isInitializing(Class cls);
extern BOOL _class_isInitialized(Class cls);
extern void _class_setInitializing(Class cls);
extern void _class_setInitialized(Class cls);
extern Class _class_getNonMetaClass(Class cls);
extern Method _class_getMethod(Class cls, SEL sel);
extern Method _class_getMethodNoSuper(Class cls, SEL sel);
extern Method _class_getMethodNoSuper_nolock(Class cls, SEL sel);
extern BOOL _class_isLoadable(Class cls);
extern IMP _class_getLoadMethod(Class cls);
extern BOOL _class_hasLoadMethod(Class cls);
extern BOOL _class_hasCxxStructorsNoSuper(Class cls);
extern BOOL _class_shouldFinalizeOnMainThread(Class cls);
extern void _class_setFinalizeOnMainThread(Class cls);
extern BOOL _class_instancesHaveAssociatedObjects(Class cls);
extern void _class_assertInstancesHaveAssociatedObjects(Class cls);
extern BOOL _class_shouldGrowCache(Class cls);
extern void _class_setGrowCache(Class cls, BOOL grow);
extern Ivar _class_getVariable(Class cls, const char *name);

extern id _internal_class_createInstanceFromZone(Class cls, size_t extraBytes,
                                                 void *zone);
extern id _internal_object_dispose(id anObject);

extern Class gdb_class_getClass(Class cls);
extern BOOL class_instancesHaveAssociatedObjects(Class cls);
OBJC_EXPORT BOOL gdb_objc_isRuntimeLocked();

extern const char *_category_getName(Category cat);
extern const char *_category_getClassName(Category cat);
extern Class _category_getClass(Category cat);
extern IMP _category_getLoadMethod(Category cat);

extern BOOL object_cxxConstruct(id obj);
extern void object_cxxDestruct(id obj);

extern Method _class_resolveMethod(Class cls, SEL sel);
extern void log_and_fill_cache(Class cls, Class implementer, Method meth, SEL sel);

#define OBJC_WARN_DEPRECATED \
    do { \
        static int warned = 0; \
        if (!warned) { \
            warned = 1; \
            _objc_warn_deprecated(__FUNCTION__, NULL); \
        } \
    } while (0) \

__END_DECLS

#endif /* _OBJC_PRIVATE_H_ */

