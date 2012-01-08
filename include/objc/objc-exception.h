/*
 * Copyright (C) 2011 Dmitry Skiba
 * Copyright (c) 2002-2003, 2006-2007 Apple Inc.  All Rights Reserved.
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

#ifndef __OBJC_EXCEPTION_H_
#define __OBJC_EXCEPTION_H_

#include <objc/objc.h>
#include <stdint.h>

typedef id (*objc_exception_preprocessor)(id exception);
typedef int (*objc_exception_matcher)(Class catch_type, id exception);
typedef void (*objc_uncaught_exception_handler)(id exception);
typedef void (*objc_exception_handler)(id unused, void *context);

OBJC_EXPORT void objc_exception_throw(id exception);
OBJC_EXPORT void objc_exception_rethrow(void);
OBJC_EXPORT id objc_begin_catch(void *exc_buf);
OBJC_EXPORT void objc_end_catch(void);

OBJC_EXPORT objc_exception_preprocessor objc_setExceptionPreprocessor(objc_exception_preprocessor fn);
OBJC_EXPORT objc_exception_matcher objc_setExceptionMatcher(objc_exception_matcher fn);
OBJC_EXPORT objc_uncaught_exception_handler objc_setUncaughtExceptionHandler(objc_uncaught_exception_handler fn);

#endif  // __OBJC_EXCEPTION_H_
