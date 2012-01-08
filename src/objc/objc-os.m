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
* objc-os.m
* OS portability layer.
**********************************************************************/

#define OLD 1
#include "objc-os.h"
#include "objc-private.h"
#include "objc-loadmethod.h"

#if TARGET_OS_WIN32

#include "objcrt.h"

malloc_zone_t *_objc_internal_zone(void) 
{ 
    return NULL; 
}

int monitor_init(monitor_t *c) 
{
    // fixme error checking
    HANDLE mutex = CreateMutex(NULL, TRUE, NULL);
    while (!c->mutex) {
        // fixme memory barrier here?
        if (0 == InterlockedCompareExchangePointer(&c->mutex, mutex, 0)) {
            // we win - finish construction
            c->waiters = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
            c->waitersDone = CreateEvent(NULL, FALSE, FALSE, NULL);
            InitializeCriticalSection(&c->waitCountLock);
            c->waitCount = 0;
            c->didBroadcast = 0;
            ReleaseMutex(c->mutex);    
            return 0;
        }
    }

    // someone else allocated the mutex and constructed the monitor
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    return 0;
}

void mutex_init(mutex_t *m)
{
    while (!m->lock) {
        CRITICAL_SECTION *newlock = malloc(sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(newlock);
        // fixme memory barrier here?
        if (0 == InterlockedCompareExchangePointer(&m->lock, newlock, 0)) {
            return;
        }
        // someone else installed their lock first
        DeleteCriticalSection(newlock);
        free(newlock);
    }
}


void recursive_mutex_init(recursive_mutex_t *m)
{
    // fixme error checking
    HANDLE newmutex = CreateMutex(NULL, FALSE, NULL);
    while (!m->mutex) {
        // fixme memory barrier here?
        if (0 == InterlockedCompareExchangePointer(&m->mutex, newmutex, 0)) {
            // we win
            return;
        }
    }
    
    // someone else installed their lock first
    CloseHandle(newmutex);
}


WINBOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        environ_init();
        tls_init();
        lock_init();
        sel_init(NO);
        exception_init();        
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

OBJC_EXPORT void *_objc_init_image(HMODULE image, const objc_sections *sects)
{
    header_info *hi = _malloc_internal(sizeof(header_info));
    size_t count, i;

    hi->mhdr = (const headerType *)image;
    hi->info = sects->iiStart;
    hi->allClassesRealized = NO;
    hi->os.modules = sects->modStart ? (Module *)((void **)sects->modStart+1) : 0;
    hi->os.moduleCount = (Module *)sects->modEnd - hi->os.modules;
    hi->os.protocols = sects->protoStart ? (struct old_protocol **)((void **)sects->protoStart+1) : 0;
    hi->os.protocolCount = (struct old_protocol **)sects->protoEnd - hi->os.protocols;
    hi->os.imageinfo = NULL;
    hi->os.imageinfoBytes = 0;
    // hi->os.imageinfo = sects->iiStart ? (uint8_t *)((void **)sects->iiStart+1) : 0;;
//     hi->os.imageinfoBytes = (uint8_t *)sects->iiEnd - hi->os.imageinfo;
    hi->os.selrefs = sects->selrefsStart ? (SEL *)((void **)sects->selrefsStart+1) : 0;
    hi->os.selrefCount = (SEL *)sects->selrefsEnd - hi->os.selrefs;
    hi->os.clsrefs = sects->clsrefsStart ? (Class *)((void **)sects->clsrefsStart+1) : 0;
    hi->os.clsrefCount = (Class *)sects->clsrefsEnd - hi->os.clsrefs;

    count = 0;
    for (i = 0; i < hi->os.moduleCount; i++) {
        if (hi->os.modules[i]) count++;
    }
    hi->mod_count = 0;
    hi->mod_ptr = 0;
    if (count > 0) {
        hi->mod_ptr = malloc(count * sizeof(struct objc_module));
        for (i = 0; i < hi->os.moduleCount; i++) {
            if (hi->os.modules[i]) memcpy(&hi->mod_ptr[hi->mod_count++], hi->os.modules[i], sizeof(struct objc_module));
        }
    }

    _objc_appendHeader(hi);

    if (PrintImages) {
        _objc_inform("IMAGES: loading image for %s%s%s\n", 
            _nameForHeader(hi->mhdr), 
            headerIsBundle(hi) ? " (bundle)" : "", 
            _objcHeaderIsReplacement(hi) ? " (replacement)":"");
    }

    _read_images(&hi, 1);

    return hi;
}

OBJC_EXPORT void _objc_load_image(HMODULE image, header_info *hinfo)
{
    prepare_load_methods(hinfo);
    call_load_methods();
}

OBJC_EXPORT void _objc_unload_image(HMODULE image, header_info *hinfo)
{
    _objc_fatal("image unload not supported");
}


// TARGET_OS_WIN32
#elif TARGET_OS_MAC

__private_extern__ void mutex_init(mutex_t *m)
{
    pthread_mutex_init(m, NULL);
}


__private_extern__ void recursive_mutex_init(recursive_mutex_t *m)
{
    // fixme error checking
    pthread_mutex_t *newmutex;

    // Build recursive mutex attributes, if needed
    static pthread_mutexattr_t *attr;
    if (!attr) {
        pthread_mutexattr_t *newattr = 
            _malloc_internal(sizeof(pthread_mutexattr_t));
        pthread_mutexattr_init(newattr);
        pthread_mutexattr_settype(newattr, PTHREAD_MUTEX_RECURSIVE);
        while (!attr) {
            if (OSAtomicCompareAndSwapPtrBarrier(0, newattr, (void**)&attr)) {
                // we win
                goto attr_done;
            }
        }
        // someone else built the attr first
        _free_internal(newattr);
    }
 attr_done:

    // Build the mutex itself
    newmutex = _malloc_internal(sizeof(pthread_mutex_t));
    pthread_mutex_init(newmutex, attr);
    while (!m->mutex) {
        if (OSAtomicCompareAndSwapPtrBarrier(0, newmutex, (void**)&m->mutex)) {
            // we win
            return;
        }
    }
    
    // someone else installed their mutex first
    pthread_mutex_destroy(newmutex);
}


/***********************************************************************
* bad_magic.
* Return YES if the header has invalid Mach-o magic.
**********************************************************************/
__private_extern__ BOOL bad_magic(const headerType *mhdr)
{
    return (mhdr->magic != MH_MAGIC  &&  mhdr->magic != MH_MAGIC_64  &&  
            mhdr->magic != MH_CIGAM  &&  mhdr->magic != MH_CIGAM_64);
}


static const segmentType *
getsegbynamefromheader(const headerType *head, const char *segname)
{
#ifndef __LP64__
#define SEGMENT_CMD LC_SEGMENT
#else
#define SEGMENT_CMD LC_SEGMENT_64
#endif
    const segmentType *sgp;
    unsigned long i;
    
    sgp = (const segmentType *) (head + 1);
    for (i = 0; i < head->ncmds; i++){
        if (sgp->cmd == SEGMENT_CMD) {
            if (strncmp(sgp->segname, segname, sizeof(sgp->segname)) == 0) {
                return sgp;
            }
        }
        sgp = (const segmentType *)((char *)sgp + sgp->cmdsize);
    }
    return NULL;
#undef SEGMENT_CMD
}


static header_info * _objc_addHeader(const headerType *mhdr)
{
    size_t info_size = 0;
    const segmentType *objc_segment;
    const objc_image_info *image_info;
    const segmentType *data_segment;
    header_info *result;
    ptrdiff_t image_slide;

    if (bad_magic(mhdr)) return NULL;

    // Weed out duplicates
    for (result = FirstHeader; result; result = result->next) {
        if (mhdr == result->mhdr) return NULL;
    }

    // Locate the __OBJC segment
    image_slide = _getImageSlide(mhdr);
    image_info = _getObjcImageInfo(mhdr, image_slide, &info_size);
    objc_segment = getsegbynamefromheader(mhdr, SEG_OBJC);
    data_segment = getsegbynamefromheader(mhdr, SEG_DATA);

    if (!objc_segment  &&  !image_info) return NULL;

    // Allocate a header_info entry.
    result = _calloc_internal(sizeof(header_info), 1);

    // Set up the new header_info entry.
    result->mhdr = mhdr;
    result->os.image_slide = image_slide;
    result->os.objcSegmentHeader = objc_segment;
    result->os.dataSegmentHeader = data_segment;
    result->info = image_info;
    dladdr((void*)result->mhdr, &result->os.dl_info);
    result->allClassesRealized = NO;

    // dylibs are not allowed to unload
    // ...except those with image_info and nothing else (5359412)
    if (result->mhdr->filetype == MH_DYLIB  &&  _hasObjcContents(result)) {
        dlopen(result->os.dl_info.dli_fname, RTLD_NOLOAD);
    }

    // Make sure every copy of objc_image_info in this image is the same.
    // This means same version and same bitwise contents.
    if (result->info) {
        const objc_image_info *start = result->info;
        const objc_image_info *end = 
            (objc_image_info *)(info_size + (uint8_t *)start);
        const objc_image_info *info = start;
        while (info < end) {
            // version is byte size, except for version 0
            size_t struct_size = info->version;
            if (struct_size == 0) struct_size = 2 * sizeof(uint32_t);
            if (info->version != start->version  ||  
                0 != memcmp(info, start, struct_size))
            {
                _objc_inform("'%s' has inconsistently-compiled Objective-C "
                            "code. Please recompile all code in it.", 
                            _nameForHeader(mhdr));
            }
            info = (objc_image_info *)(struct_size + (uint8_t *)info);
        }
    }
    

    _objc_appendHeader(result);
    
    return result;
}


static const char *_gcForHInfo2(const header_info *hinfo)
{
    return "";
}


/***********************************************************************
* map_images_nolock
* Process the given images which are being mapped in by dyld.
* All class registration and fixups are performed (or deferred pending
* discovery of missing superclasses etc), and +load methods are called.
*
* info[] is in bottom-up order i.e. libobjc will be earlier in the 
* array than any library that links to libobjc.
*
* Locking: loadMethodLock(old) or runtimeLock(new) acquired by map_images.
**********************************************************************/
__private_extern__ const char *
map_images_nolock(enum dyld_image_states state, uint32_t infoCount,
                  const struct dyld_image_info infoList[])
{
    static BOOL firstTime = YES;
    static BOOL wantsGC NOBSS = NO;
    uint32_t i;
    header_info *hi;
    header_info *hList[infoCount];
    uint32_t hCount;

    // Perform first-time initialization if necessary.
    // This function is called before ordinary library initializers. 
    // fixme defer initialization until an objc-using image is found?
    if (firstTime) {
    }

    if (PrintImages) {
        _objc_inform("IMAGES: processing %u newly-mapped images...\n", infoCount);
    }


    // Find all images with Objective-C metadata.
    hCount = 0;
    i = infoCount;
    while (i--) {
        const headerType *mhdr = (headerType *)infoList[i].imageLoadAddress;

        hi = _objc_addHeader(mhdr);
        if (!hi) {
            // no objc data in this entry
            continue;
        }

        hList[hCount++] = hi;
        

        if (PrintImages) {
            _objc_inform("IMAGES: loading image for %s%s%s%s%s\n", 
                         _nameForHeader(mhdr), 
                         mhdr->filetype == MH_BUNDLE ? " (bundle)" : "", 
                         _objcHeaderIsReplacement(hi) ? " (replacement)" : "",
                         _objcHeaderOptimizedByDyld(hi)?" (preoptimized)" : "",
                         _gcForHInfo2(hi));
        }
    }
    
    if (firstTime) {
        extern SEL FwdSel;  // in objc-msg-*.s
        sel_init(wantsGC);
        FwdSel = sel_registerName("forward::");
    }

    _read_images(hList, hCount);

    firstTime = NO;

    return NULL;
}


/***********************************************************************
* load_images_nolock
* Prepares +load in the given images which are being mapped in by dyld.
* Returns YES if there are now +load methods to be called by call_load_methods.
*
* Locking: loadMethodLock(both) and runtimeLock(new) acquired by load_images
**********************************************************************/
__private_extern__ BOOL 
load_images_nolock(enum dyld_image_states state,uint32_t infoCount,
                   const struct dyld_image_info infoList[])
{
    BOOL found = NO;
    uint32_t i;

    i = infoCount;
    while (i--) {
        header_info *hi;
        for (hi = FirstHeader; hi != NULL; hi = hi->next) {
            const headerType *mhdr = (headerType*)infoList[i].imageLoadAddress;
            if (hi->mhdr == mhdr) {
                prepare_load_methods(hi);
                found = YES;
            }
        }
    }

    return found;
}


/***********************************************************************
* unmap_image_nolock
* Process the given image which is about to be unmapped by dyld.
* mh is mach_header instead of headerType because that's what 
*   dyld_priv.h says even for 64-bit.
* 
* Locking: loadMethodLock(both) and runtimeLock(new) acquired by unmap_image.
**********************************************************************/
__private_extern__ void 
unmap_image_nolock(const struct mach_header *mh, intptr_t vmaddr_slide)
{
    if (PrintImages) {
        _objc_inform("IMAGES: processing 1 newly-unmapped image...\n");
    }

    header_info *hi;
    
    // Find the runtime's header_info struct for the image
    for (hi = FirstHeader; hi != NULL; hi = hi->next) {
        if (hi->mhdr == (const headerType *)mh) {
            break;
        }
    }

    if (!hi) return;

    if (PrintImages) { 
        _objc_inform("IMAGES: unloading image for %s%s%s%s\n", 
                     _nameForHeader(hi->mhdr), 
                     hi->mhdr->filetype == MH_BUNDLE ? " (bundle)" : "", 
                     _objcHeaderIsReplacement(hi) ? " (replacement)" : "", 
                     _gcForHInfo2(hi));
    }

    _unload_image(hi);

    // Remove header_info from header list
    _objc_removeHeader(hi);
    _free_internal(hi);
}


/***********************************************************************
* _objc_init
* Static initializer. Registers our image notifier with dyld.
**********************************************************************/
static __attribute__((constructor))
void _objc_init(void)
{
    // fixme defer initialization until an objc-using image is found?
    objc_environ_init();
    tls_init();
    lock_init();
    exception_init();

    // Register for unmap first, in case some +load unmaps something
    _dyld_register_func_for_remove_image(&unmap_image);
    dyld_register_image_state_change_handler(dyld_image_state_bound,
                                             1/*batch*/, &map_images);
    dyld_register_image_state_change_handler(dyld_image_state_dependents_initialized, 0/*not batch*/, &load_images);
}


/***********************************************************************
* _headerForAddress.
* addr can be a class or a category
**********************************************************************/
static const header_info *_headerForAddress(void *addr)
{
    unsigned long            size;
    unsigned long            seg;
    header_info *        hi;

    // Check all headers in the vector
    for (hi = FirstHeader; hi != NULL; hi = hi->next)
    {
        // Locate header data, if any
        const segmentType *segHeader;
        segHeader = hi->os.dataSegmentHeader;
        if (!segHeader) continue;
        seg = segHeader->vmaddr + hi->os.image_slide;
        size = segHeader->filesize;

        // Is the class in this header?
        if ((seg <= (unsigned long) addr) &&
            ((unsigned long) addr < (seg + size)))
            return hi;
    }

    // Not found
    return 0;
}


/***********************************************************************
* _headerForClass
* Return the image header containing this class, or NULL.
* Returns NULL on runtime-constructed classes, and the NSCF classes.
**********************************************************************/
__private_extern__ const header_info *_headerForClass(Class cls)
{
    return _headerForAddress(cls);
}


#if defined(MESSAGE_LOGGING)
/**********************************************************************
* secure_open
* Securely open a file from a world-writable directory (like /tmp)
* If the file does not exist, it will be atomically created with mode 0600
* If the file exists, it must be, and remain after opening: 
*   1. a regular file (in particular, not a symlink)
*   2. owned by euid
*   3. permissions 0600
*   4. link count == 1
* Returns a file descriptor or -1. Errno may or may not be set on error.
**********************************************************************/
__private_extern__ int secure_open(const char *filename, int flags, uid_t euid)
{
    struct stat fs, ls;
    int fd = -1;
    BOOL truncate = NO;
    BOOL create = NO;

    if (flags & O_TRUNC) {
        // Don't truncate the file until after it is open and verified.
        truncate = YES;
        flags &= ~O_TRUNC;
    }
    if (flags & O_CREAT) {
        // Don't create except when we're ready for it
        create = YES;
        flags &= ~O_CREAT;
        flags &= ~O_EXCL;
    }

    if (lstat(filename, &ls) < 0) {
        if (errno == ENOENT  &&  create) {
            // No such file - create it
            fd = open(filename, flags | O_CREAT | O_EXCL, 0600);
            if (fd >= 0) {
                // File was created successfully.
                // New file does not need to be truncated.
                return fd;
            } else {
                // File creation failed.
                return -1;
            }
        } else {
            // lstat failed, or user doesn't want to create the file
            return -1;
        }
    } else {
        // lstat succeeded - verify attributes and open
        if (S_ISREG(ls.st_mode)  &&  // regular file?
            ls.st_nlink == 1  &&     // link count == 1?
            ls.st_uid == euid  &&    // owned by euid?
            (ls.st_mode & ALLPERMS) == (S_IRUSR | S_IWUSR))  // mode 0600?
        {
            // Attributes look ok - open it and check attributes again
            fd = open(filename, flags, 0000);
            if (fd >= 0) {
                // File is open - double-check attributes
                if (0 == fstat(fd, &fs)  &&  
                    fs.st_nlink == ls.st_nlink  &&  // link count == 1?
                    fs.st_uid == ls.st_uid  &&      // owned by euid?
                    fs.st_mode == ls.st_mode  &&    // regular file, 0600?
                    fs.st_ino == ls.st_ino  &&      // same inode as before?
                    fs.st_dev == ls.st_dev)         // same device as before?
                {
                    // File is open and OK
                    if (truncate) ftruncate(fd, 0);
                    return fd;
                } else {
                    // Opened file looks funny - close it
                    close(fd);
                    return -1;
                }
            } else {
                // File didn't open
                return -1;
            }
        } else {
            // Unopened file looks funny - don't open it
            return -1;
        }
    }
}
#endif

/***********************************************************************
* _objc_internal_zone.
* Malloc zone for internal runtime data.
* By default this is the default malloc zone, but a dedicated zone is 
* used if environment variable OBJC_USE_INTERNAL_ZONE is set.
**********************************************************************/
__private_extern__ malloc_zone_t *_objc_internal_zone(void)
{
    static malloc_zone_t *z = (malloc_zone_t *)-1;
    if (z == (malloc_zone_t *)-1) {
        if (UseInternalZone) {
            z = malloc_create_zone(getpagesize(), 0);
            malloc_set_zone_name(z, "ObjC");
        } else {
            z = malloc_default_zone();
        }
    }
    return z;
}


// TARGET_OS_MAC
#else


#error unknown OS


#endif
