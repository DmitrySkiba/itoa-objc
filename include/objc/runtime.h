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

#ifndef _OBJC_RUNTIME_H
#define _OBJC_RUNTIME_H

#include <objc/objc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>


/* Types */

typedef struct objc_method *Method;
typedef struct objc_ivar *Ivar;
typedef struct objc_category *Category;
typedef struct objc_property *objc_property_t;

struct objc_class {
    Class isa;
} OBJC2_UNAVAILABLE;
/* Use `Class` instead of `struct objc_class *` */

#ifdef __OBJC__
@class Protocol;
#else
typedef struct objc_object Protocol;
#endif

struct objc_method_description {
    SEL name;
    char *types;
};


/* Functions */

OBJC_EXPORT id object_copy(id obj, size_t size);
OBJC_EXPORT id object_dispose(id obj);

OBJC_EXPORT Class object_getClass(id obj);
OBJC_EXPORT Class object_setClass(id obj, Class cls);

OBJC_EXPORT const char *object_getClassName(id obj);
OBJC_EXPORT void *object_getIndexedIvars(id obj);

OBJC_EXPORT id object_getIvar(id obj, Ivar ivar);
OBJC_EXPORT void object_setIvar(id obj, Ivar ivar, id value);

OBJC_EXPORT Ivar object_setInstanceVariable(id obj, const char *name, void *value);
OBJC_EXPORT Ivar object_getInstanceVariable(id obj, const char *name, void **outValue);

OBJC_EXPORT id objc_getClass(const char *name);
OBJC_EXPORT id objc_getMetaClass(const char *name);
OBJC_EXPORT id objc_lookUpClass(const char *name);
OBJC_EXPORT id objc_getRequiredClass(const char *name);
OBJC_EXPORT Class objc_getFutureClass(const char *name);
OBJC_EXPORT void objc_setFutureClass(Class cls, const char *name);
OBJC_EXPORT int objc_getClassList(Class *buffer, int bufferCount);

OBJC_EXPORT Protocol *objc_getProtocol(const char *name);
OBJC_EXPORT Protocol **objc_copyProtocolList(unsigned int *outCount);

OBJC_EXPORT const char *class_getName(Class cls);
OBJC_EXPORT BOOL class_isMetaClass(Class cls);
OBJC_EXPORT Class class_getSuperclass(Class cls);
OBJC_EXPORT Class class_setSuperclass(Class cls, Class newSuper);

OBJC_EXPORT int class_getVersion(Class cls);
OBJC_EXPORT void class_setVersion(Class cls, int version);

OBJC_EXPORT size_t class_getInstanceSize(Class cls);

OBJC_EXPORT Ivar class_getInstanceVariable(Class cls, const char *name);
OBJC_EXPORT Ivar class_getClassVariable(Class cls, const char *name);
OBJC_EXPORT Ivar *class_copyIvarList(Class cls, unsigned int *outCount);

OBJC_EXPORT Method class_getInstanceMethod(Class cls, SEL name);
OBJC_EXPORT Method class_getClassMethod(Class cls, SEL name);
OBJC_EXPORT IMP class_getMethodImplementation(Class cls, SEL name);
OBJC_EXPORT IMP class_getMethodImplementation_stret(Class cls, SEL name);
OBJC_EXPORT BOOL class_respondsToSelector(Class cls, SEL sel);
OBJC_EXPORT Method *class_copyMethodList(Class cls, unsigned int *outCount);

OBJC_EXPORT BOOL class_conformsToProtocol(Class cls, Protocol *protocol);
OBJC_EXPORT Protocol **class_copyProtocolList(Class cls, unsigned int *outCount);

OBJC_EXPORT objc_property_t class_getProperty(Class cls, const char *name);
OBJC_EXPORT objc_property_t *class_copyPropertyList(Class cls, unsigned int *outCount);

OBJC_EXPORT const char *class_getIvarLayout(Class cls);
OBJC_EXPORT const char *class_getWeakIvarLayout(Class cls);

OBJC_EXPORT id class_createInstance(Class cls, size_t extraBytes);

OBJC_EXPORT Class objc_allocateClassPair(Class superclass, const char *name, 
                                         size_t extraBytes);
OBJC_EXPORT void objc_registerClassPair(Class cls);
OBJC_EXPORT Class objc_duplicateClass(Class original, const char *name, 
                                      size_t extraBytes);
OBJC_EXPORT void objc_disposeClassPair(Class cls);

OBJC_EXPORT BOOL class_addMethod(Class cls, SEL name, IMP imp, 
                                 const char *types);
OBJC_EXPORT IMP class_replaceMethod(Class cls, SEL name, IMP imp, 
                                    const char *types);
OBJC_EXPORT BOOL class_addIvar(Class cls, const char *name, size_t size, 
                               uint8_t alignment, const char *types);
OBJC_EXPORT BOOL class_addProtocol(Class cls, Protocol *protocol);
OBJC_EXPORT void class_setIvarLayout(Class cls, const char *layout);
OBJC_EXPORT void class_setWeakIvarLayout(Class cls, const char *layout);


OBJC_EXPORT SEL method_getName(Method m);
OBJC_EXPORT IMP method_getImplementation(Method m);
OBJC_EXPORT const char *method_getTypeEncoding(Method m);

OBJC_EXPORT unsigned int method_getNumberOfArguments(Method m);
OBJC_EXPORT char *method_copyReturnType(Method m);
OBJC_EXPORT char *method_copyArgumentType(Method m, unsigned int index);
OBJC_EXPORT void method_getReturnType(Method m, char *dst, size_t dst_len);
OBJC_EXPORT void method_getArgumentType(Method m, unsigned int index, 
                                        char *dst, size_t dst_len);
OBJC_EXPORT struct objc_method_description *method_getDescription(Method m);

OBJC_EXPORT IMP method_setImplementation(Method m, IMP imp);
OBJC_EXPORT void method_exchangeImplementations(Method m1, Method m2);

OBJC_EXPORT const char *ivar_getName(Ivar v);
OBJC_EXPORT const char *ivar_getTypeEncoding(Ivar v);
OBJC_EXPORT ptrdiff_t ivar_getOffset(Ivar v);

OBJC_EXPORT const char *property_getName(objc_property_t property);
OBJC_EXPORT const char *property_getAttributes(objc_property_t property);

OBJC_EXPORT BOOL protocol_conformsToProtocol(Protocol *proto, Protocol *other);
OBJC_EXPORT BOOL protocol_isEqual(Protocol *proto, Protocol *other);
OBJC_EXPORT const char *protocol_getName(Protocol *p);
OBJC_EXPORT struct objc_method_description protocol_getMethodDescription(Protocol *p, SEL aSel, BOOL isRequiredMethod, BOOL isInstanceMethod);
OBJC_EXPORT struct objc_method_description *protocol_copyMethodDescriptionList(Protocol *p, BOOL isRequiredMethod, BOOL isInstanceMethod, unsigned int *outCount);
OBJC_EXPORT objc_property_t protocol_getProperty(Protocol *proto, const char *name, BOOL isRequiredProperty, BOOL isInstanceProperty);
OBJC_EXPORT objc_property_t *protocol_copyPropertyList(Protocol *proto, unsigned int *outCount);
OBJC_EXPORT Protocol **protocol_copyProtocolList(Protocol *proto, unsigned int *outCount);

OBJC_EXPORT const char **objc_copyImageNames(unsigned int *outCount);
OBJC_EXPORT const char *class_getImageName(Class cls);
OBJC_EXPORT const char **objc_copyClassNamesForImage(const char *image, 
                                                     unsigned int *outCount);

OBJC_EXPORT const char *sel_getName(SEL sel);
OBJC_EXPORT SEL sel_getUid(const char *str);
OBJC_EXPORT SEL sel_registerName(const char *str);
OBJC_EXPORT BOOL sel_isEqual(SEL lhs, SEL rhs);

OBJC_EXPORT void objc_enumerationMutation(id);
OBJC_EXPORT void objc_setEnumerationMutationHandler(void (*handler)(id));

OBJC_EXPORT void objc_setForwardHandler(void *fwd, void *fwd_stret);


/* Associated Object support. */

/* objc_setAssociatedObject() options */
enum {
    OBJC_ASSOCIATION_ASSIGN = 0,
    OBJC_ASSOCIATION_RETAIN_NONATOMIC = 1,
    OBJC_ASSOCIATION_COPY_NONATOMIC = 3,
    OBJC_ASSOCIATION_RETAIN = 01401,
    OBJC_ASSOCIATION_COPY = 01403
};
typedef uintptr_t objc_AssociationPolicy;

OBJC_EXPORT void objc_setAssociatedObject(id object, void *key, id value, objc_AssociationPolicy policy);
OBJC_EXPORT id objc_getAssociatedObject(id object, void *key);
OBJC_EXPORT void objc_removeAssociatedObjects(id object);


#define _C_ID       '@'
#define _C_CLASS    '#'
#define _C_SEL      ':'
#define _C_CHR      'c'
#define _C_UCHR     'C'
#define _C_SHT      's'
#define _C_USHT     'S'
#define _C_INT      'i'
#define _C_UINT     'I'
#define _C_LNG      'l'
#define _C_ULNG     'L'
#define _C_LNG_LNG  'q'
#define _C_ULNG_LNG 'Q'
#define _C_FLT      'f'
#define _C_DBL      'd'
#define _C_BFLD     'b'
#define _C_BOOL     'B'
#define _C_VOID     'v'
#define _C_UNDEF    '?'
#define _C_PTR      '^'
#define _C_CHARPTR  '*'
#define _C_ATOM     '%'
#define _C_ARY_B    '['
#define _C_ARY_E    ']'
#define _C_UNION_B  '('
#define _C_UNION_E  ')'
#define _C_STRUCT_B '{'
#define _C_STRUCT_E '}'
#define _C_VECTOR   '!'
#define _C_CONST    'r'


/* Obsolete types */

struct objc_method_list;


/* Obsolete functions */

OBJC_EXPORT BOOL sel_isMapped(SEL sel)                       OBJC2_UNAVAILABLE;

OBJC_EXPORT id object_copyFromZone(id anObject, size_t nBytes, void *z);
OBJC_EXPORT id object_realloc(id anObject, size_t nBytes)    OBJC2_UNAVAILABLE;
OBJC_EXPORT id object_reallocFromZone(id anObject, size_t nBytes, void *z) OBJC2_UNAVAILABLE;

#define OBSOLETE_OBJC_GETCLASSES 1
OBJC_EXPORT void *objc_getClasses(void)                      OBJC2_UNAVAILABLE;
OBJC_EXPORT void objc_addClass(Class myClass)                OBJC2_UNAVAILABLE;
OBJC_EXPORT void objc_setClassHandler(int (*)(const char *)) OBJC2_UNAVAILABLE;
OBJC_EXPORT void objc_setMultithreaded (BOOL flag)           OBJC2_UNAVAILABLE;

OBJC_EXPORT id class_createInstanceFromZone(Class, size_t idxIvars, void *z);

OBJC_EXPORT void class_addMethods(Class, struct objc_method_list *) OBJC2_UNAVAILABLE;
OBJC_EXPORT void class_removeMethods(Class, struct objc_method_list *) OBJC2_UNAVAILABLE;

OBJC_EXPORT Class class_poseAs(Class imposter, Class original) OBJC2_UNAVAILABLE;

OBJC_EXPORT unsigned int method_getSizeOfArguments(Method m) OBJC2_UNAVAILABLE;
OBJC_EXPORT unsigned method_getArgumentInfo(struct objc_method *m, int arg, const char **type, int *offset) OBJC2_UNAVAILABLE;

OBJC_EXPORT BOOL class_respondsToMethod(Class, SEL)          OBJC2_UNAVAILABLE;
OBJC_EXPORT IMP class_lookupMethod(Class, SEL)               OBJC2_UNAVAILABLE;
OBJC_EXPORT Class objc_getOrigClass(const char *name)        OBJC2_UNAVAILABLE;
#define OBJC_NEXT_METHOD_LIST 1
OBJC_EXPORT struct objc_method_list *class_nextMethodList(Class, void **) OBJC2_UNAVAILABLE;
// usage for nextMethodList
//
// void *iterator = 0;
// struct objc_method_list *mlist;
// while ( mlist = class_nextMethodList( cls, &iterator ) )
//    ;

OBJC_EXPORT id (*_alloc)(Class, size_t)                      OBJC2_UNAVAILABLE;
OBJC_EXPORT id (*_copy)(id, size_t)                          OBJC2_UNAVAILABLE;
OBJC_EXPORT id (*_realloc)(id, size_t)                       OBJC2_UNAVAILABLE;
OBJC_EXPORT id (*_dealloc)(id)                               OBJC2_UNAVAILABLE;
OBJC_EXPORT id (*_zoneAlloc)(Class, size_t, void *)          OBJC2_UNAVAILABLE;
OBJC_EXPORT id (*_zoneRealloc)(id, size_t, void *)           OBJC2_UNAVAILABLE;
OBJC_EXPORT id (*_zoneCopy)(id, size_t, void *)              OBJC2_UNAVAILABLE;
OBJC_EXPORT void (*_error)(id, const char *, va_list)        OBJC2_UNAVAILABLE;

#endif
