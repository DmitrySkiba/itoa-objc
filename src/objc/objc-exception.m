/*
 * Copyright (C) 2011 Dmitry Skiba
 * Copyright (c) 2002-2007 Apple Inc. All rights reserved.
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

#include "objc-private.h"
#include <objc/objc-exception.h>


// unwind library types and functions
// Mostly adapted from Itanium C++ ABI: Exception Handling
//   http://www.codesourcery.com/cxx-abi/abi-eh.html

struct _Unwind_Exception;
struct _Unwind_Context;

typedef enum {
    _URC_NO_REASON = 0,
    _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
    _URC_FATAL_PHASE2_ERROR = 2,
    _URC_FATAL_PHASE1_ERROR = 3,
    _URC_NORMAL_STOP = 4,
    _URC_END_OF_STACK = 5,
    _URC_HANDLER_FOUND = 6,
    _URC_INSTALL_CONTEXT = 7,
    _URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

#ifdef __arm__

typedef enum {
    _US_VIRTUAL_UNWIND_FRAME = 0,
    _US_UNWIND_FRAME_STARTING = 1,
    _US_UNWIND_FRAME_RESUME = 2,
    _US_ACTION_MASK = 3,
    _US_FORCE_UNWIND = 8,
    _US_END_OF_STACK = 16
} _Unwind_State;

#else // !__arm__

typedef int _Unwind_Action;
static const _Unwind_Action _UA_SEARCH_PHASE = 1;
static const _Unwind_Action _UA_CLEANUP_PHASE = 2;
static const _Unwind_Action _UA_HANDLER_FRAME = 4;
static const _Unwind_Action _UA_FORCE_UNWIND = 8;

extern uintptr_t _Unwind_GetIP (struct _Unwind_Context *);
extern uintptr_t _Unwind_GetCFA (struct _Unwind_Context *);
extern uintptr_t _Unwind_GetLanguageSpecificData(struct _Unwind_Context *);

#endif // !__arm__

typedef void (*terminate_handler) ();

// mangled std::set_terminate()
extern terminate_handler _ZSt13set_terminatePFvvE(terminate_handler);
extern void *__cxa_allocate_exception(size_t thrown_size);
extern void __cxa_throw(void *exc, void *typeinfo, void (*destructor)(void *)) __attribute__((noreturn));
extern void *__cxa_begin_catch(void *exc);
extern void __cxa_end_catch(void);
extern void __cxa_rethrow(void);
extern void *__cxa_current_exception_type(void);

#define CXX_PERSONALITY __gxx_personality_v0
#define OBJC_PERSONALITY __objc_personality_v0

#ifdef __arm__

extern _Unwind_Reason_Code
CXX_PERSONALITY(_Unwind_State state,
                struct _Unwind_Exception *exceptionObject,
                struct _Unwind_Context *context);

#else // !__arm__

extern _Unwind_Reason_Code 
CXX_PERSONALITY(int version,
                _Unwind_Action actions,
                uint64_t exceptionClass,
                struct _Unwind_Exception *exceptionObject,
                struct _Unwind_Context *context);

#endif // __arm__

// objc's internal exception types and data

extern const void *objc_ehtype_vtable[];

struct objc_typeinfo {
    // Position of vtable and name fields must match C++ typeinfo object
    const void **vtable;  // always objc_ehtype_vtable+2
    const char *name;     // c++ typeinfo string

    Class cls;
};

struct objc_exception {
    id obj;
    struct objc_typeinfo tinfo;
};


static void _objc_exception_noop(void) { } 
static char _objc_exception_false(void) { return 0; } 
static char _objc_exception_true(void) { return 1; } 
static char _objc_exception_do_catch(struct objc_typeinfo *catch_tinfo, 
                                     struct objc_typeinfo *throw_tinfo, 
                                     void **throw_obj_p, 
                                     unsigned outer);

const void *objc_ehtype_vtable[] = {
    NULL,  // typeinfo's vtable? - fixme 
    NULL,  // typeinfo's typeinfo, initialized in _objc_init_ehtype_vtable 
    _objc_exception_noop,      // in-place destructor?
    _objc_exception_noop,      // destructor?
    _objc_exception_do_catch,  // __do_catch
    _objc_exception_false,     // __do_upcast
    _objc_exception_true,      // __is_pointer_p
    _objc_exception_false,     // __is_function_p
};

struct objc_typeinfo OBJC_EHTYPE_id = {
    objc_ehtype_vtable+2, 
    "id", 
    NULL
};

/* This variable provides access to the objc_ehtype_vtable.
   Used in objc-exception-init.mm and objc/fix_catch_id.h. */
extern const void **_objc_ehtype_vtable;
const void **_objc_ehtype_vtable=objc_ehtype_vtable;

/* Initializes typeinfo in objc_ehtype_vtable. */
extern void _objc_init_ehtype_vtable();


/***********************************************************************
* Foundation customization
**********************************************************************/

/***********************************************************************
* _objc_default_exception_preprocessor
* Default exception preprocessor. Expected to be overridden by Foundation.
**********************************************************************/
static id _objc_default_exception_preprocessor(id exception)
{
    return exception;
}
static objc_exception_preprocessor exception_preprocessor = _objc_default_exception_preprocessor;


/***********************************************************************
* _objc_default_exception_matcher
* Default exception matcher. Expected to be overridden by Foundation.
**********************************************************************/
static int _objc_default_exception_matcher(Class catch_cls, id exception)
{
    Class cls;
    for (cls = exception->isa;
         cls != NULL; 
         cls = class_getSuperclass(cls))
    {
        if (cls == catch_cls) return 1;
    }

    return 0;
}
static objc_exception_matcher exception_matcher = _objc_default_exception_matcher;


/***********************************************************************
* _objc_default_uncaught_exception_handler
* Default uncaught exception handler. Expected to be overridden by Foundation.
**********************************************************************/
static void _objc_default_uncaught_exception_handler(id exception)
{
}
static objc_uncaught_exception_handler uncaught_handler = _objc_default_uncaught_exception_handler;


/***********************************************************************
* objc_setExceptionPreprocessor
* Set a handler for preprocessing Objective-C exceptions. 
* Returns the previous handler. 
**********************************************************************/
objc_exception_preprocessor
objc_setExceptionPreprocessor(objc_exception_preprocessor fn)
{
    objc_exception_preprocessor result = exception_preprocessor;
    exception_preprocessor = fn;
    return result;
}


/***********************************************************************
* objc_setExceptionMatcher
* Set a handler for matching Objective-C exceptions. 
* Returns the previous handler. 
**********************************************************************/
objc_exception_matcher
objc_setExceptionMatcher(objc_exception_matcher fn)
{
    objc_exception_matcher result = exception_matcher;
    exception_matcher = fn;
    return result;
}


/***********************************************************************
* objc_setUncaughtExceptionHandler
* Set a handler for uncaught Objective-C exceptions. 
* Returns the previous handler. 
**********************************************************************/
objc_uncaught_exception_handler 
objc_setUncaughtExceptionHandler(objc_uncaught_exception_handler fn)
{
    objc_uncaught_exception_handler result = uncaught_handler;
    uncaught_handler = fn;
    return result;
}


/***********************************************************************
* Exception personality
**********************************************************************/

#ifdef __arm__

_Unwind_Reason_Code
OBJC_PERSONALITY(_Unwind_State state,
                 struct _Unwind_Exception *exceptionObject,
                 struct _Unwind_Context *context)
{
    // Let C++ handle the unwind itself.
    return CXX_PERSONALITY(state, exceptionObject, context);
}

#else // !__arm__

_Unwind_Reason_Code 
OBJC_PERSONALITY(int version,
                 _Unwind_Action actions,
                 uint64_t exceptionClass,
                 struct _Unwind_Exception *exceptionObject,
                 struct _Unwind_Context *context)
{
    BOOL unwinding = ((actions & _UA_CLEANUP_PHASE)  ||  
                      (actions & _UA_FORCE_UNWIND));

    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: %s through frame [ip=%p sp=%p] "
                     "for exception %p", 
                     unwinding ? "unwinding" : "searching", 
                     (void*)(_Unwind_GetIP(context)-1),
                     (void*)_Unwind_GetCFA(context), exceptionObject);
    }

    // Let C++ handle the unwind itself.
    return CXX_PERSONALITY(version, actions, exceptionClass, 
                           exceptionObject, context);
}

#endif // !__arm__

/***********************************************************************
* Compiler ABI
**********************************************************************/

static void _objc_exception_destructor(void *exc_gen) {
    struct objc_exception *exc = (struct objc_exception *)exc_gen;
}


void objc_exception_throw(id obj)
{
    struct objc_exception *exc = 
        __cxa_allocate_exception(sizeof(struct objc_exception));

    exc->obj = (*exception_preprocessor)(obj);

    exc->tinfo.vtable = objc_ehtype_vtable+2;
    exc->tinfo.name = object_getClassName(obj);
    exc->tinfo.cls = obj ? obj->isa : Nil;

    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: throwing %p (object %p, a %s)", 
                     exc, obj, object_getClassName(obj));
    }
    
    __cxa_throw(exc, &exc->tinfo, &_objc_exception_destructor);
    __builtin_trap();
}


void objc_exception_rethrow(void)
{
    // exception_preprocessor doesn't get another bite of the apple
    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: rethrowing current exception");
    }
    
    __cxa_rethrow();
    __builtin_trap();
}


id objc_begin_catch(void *exc_gen)
{
    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: handling exception %p at %p", 
                     exc_gen, __builtin_return_address(0));
    }
    // NOT actually an id in the catch(...) case!
    return (id)__cxa_begin_catch(exc_gen);
}


void objc_end_catch(void)
{
    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: finishing handler");
    }
    __cxa_end_catch();
}


static char _objc_exception_do_catch(struct objc_typeinfo *catch_tinfo, 
                                     struct objc_typeinfo *throw_tinfo, 
                                     void **throw_obj_p, 
                                     unsigned outer)
{
    id exception;

    if (throw_tinfo->vtable != objc_ehtype_vtable+2) {
        // Only objc types can be caught here.
        if (PrintExceptions) _objc_inform("EXCEPTIONS: skipping catch(?)");
        return 0;
    }

    // `catch (id)` always catches objc types.
    /*
    if (catch_tinfo == &OBJC_EHTYPE_id) {
    Can't compare addresses since we add copies of OBJC_EHTYPE_id
     to the images to fix @catch(id), see objc/fix_catch_id.h
    */
    if (!strcmp(catch_tinfo->name,"id")) {
        if (PrintExceptions) _objc_inform("EXCEPTIONS: catch(id)");
        return 1;
    }

    exception = *(id *)throw_obj_p;
    // fixme remapped catch_tinfo->cls
    if ((*exception_matcher)(catch_tinfo->cls, exception)) {
        if (PrintExceptions) _objc_inform("EXCEPTIONS: catch(%s)", 
                                          class_getName(catch_tinfo->cls));
        return 1;
    }

    if (PrintExceptions) _objc_inform("EXCEPTIONS: skipping catch(%s)", 
                                      class_getName(catch_tinfo->cls));

    return 0;
}


/***********************************************************************
* _objc_terminate
* Custom std::terminate handler.
*
* The uncaught exception callback is implemented as a std::terminate handler. 
* 1. Check if there's an active exception
* 2. If so, check if it's an Objective-C exception
* 3. If so, call our registered callback with the object.
* 4. Finally, call the previous terminate handler.
**********************************************************************/
static terminate_handler old_terminate = NULL;
static void _objc_terminate(void)
{
    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: terminating");
    }

    if (! __cxa_current_exception_type()) {
        // No current exception.
        (*old_terminate)();
    }
    else {
        // There is a current exception. Check if it's an objc exception.
        @try {
            __cxa_rethrow();
        } @catch (id e) {
            // It's an objc object. Call Foundation's handler, if any.
            (*uncaught_handler)(e);
            (*old_terminate)();
        } @catch (...) {
            // It's not an objc object. Continue to C++ terminate.
            (*old_terminate)();
        }
    }
}

/***********************************************************************
* exception_init
* Initialize libobjc's exception handling system.
* Called by map_images().
**********************************************************************/
__private_extern__ void exception_init(void)
{
    // call std::set_terminate
    old_terminate = _ZSt13set_terminatePFvvE(&_objc_terminate);
 
    // initialize objc_ehtype_vtable
    _objc_init_ehtype_vtable();   
}
