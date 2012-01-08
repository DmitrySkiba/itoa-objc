/*
 * Copyright (C) 2011 Dmitry Skiba
 * Copyright (c) 1999-2001, 2005-2007 Apple Inc.  All Rights Reserved.
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
    Protocol.h
    Copyright 1991-1996 NeXT Software, Inc.
*/


#include <stdlib.h>
#include <string.h>

#define OLD 1
#import <objc/Protocol.h>
#import "objc-private.h"


/* some forward declarations */

__private_extern__ Method _protocol_getMethod(Protocol *p, SEL sel, BOOL isRequiredMethod, BOOL isInstanceMethod);

@implementation Protocol 

// fixme hack - make Protocol a non-lazy class
+ (void) load { } 

typedef struct {
    uintptr_t count;
    Protocol *list[0];
} protocol_list_t;

- (BOOL) conformsTo: (Protocol *)aProtocolObj
{
    return protocol_conformsToProtocol(self, aProtocolObj);
}

- (struct objc_method_description *) descriptionForInstanceMethod:(SEL)aSel
{
    return method_getDescription(_protocol_getMethod(self, aSel, YES, YES));
}

- (struct objc_method_description *) descriptionForClassMethod:(SEL)aSel
{
    return method_getDescription(_protocol_getMethod(self, aSel, YES, NO));
}

- (const char *)name
{
    return protocol_getName(self);
}

- (BOOL)isEqual:other
{
    // check isKindOf:
    Class cls;
    Class protoClass = objc_getClass("Protocol");
    for (cls = other->isa; cls; cls = class_getSuperclass(cls)) {
        if (cls == protoClass) break;
    }
    if (!cls) return NO;
    // check equality
    return protocol_isEqual(self, other);
}

- (unsigned int)hash
{
    return 23;
}

@end
