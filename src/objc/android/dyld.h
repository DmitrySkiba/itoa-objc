/*
 * Copyright (C) 2011 Dmitry Skiba
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DYLD_INCLUDED_
#define _DYLD_INCLUDED_

#include <stdint.h>
#include <stdbool.h>

/* Stuff necessary to compile without errors.
 */

#define MH_MAGIC 0
#define MH_CIGAM 1
#define MH_MAGIC_64 2
#define MH_CIGAM_64 3

#define MH_DYLIB 0
#define MH_BUNDLE 1

#define SEG_DATA "SEG_DATA"
#define SECT_OBJC_MODULES "SECT_OBJC_MODULES"

#define LC_SEGMENT 0

#define RTLD_NOLOAD 0
#define RTLD_FIRST 0

struct mach_header {
    uint32_t magic;
    uint32_t ncmds;
    uint32_t filetype;
    void* handle;
};

struct segment_command {
    uint32_t cmd;
    uint32_t cmdsize;
    char segname[16];
    uint32_t vmaddr;
    uint32_t fileoff;
    uint32_t filesize;
    uint32_t nsects;
};

struct section {
    char sectname[16];
    char segname[16];
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
};

enum dyld_image_states {
    dyld_image_state_bound,
    dyld_image_state_dependents_initialized,
    DYLD_IMAGE_STATE_COUNT
};

struct dyld_image_info {
    const struct mach_header* imageLoadAddress;
};

typedef const char* (*dyld_image_state_change_handler)(
    enum dyld_image_states state,
    uint32_t infoCount,
    const struct dyld_image_info info[]);

char *getsectdatafromheader(
    const struct mach_header *mhp,
    const char *segname,
    const char *sectname,
    uint32_t *size);

void _dyld_register_func_for_add_image(
    void (*func)(const struct mach_header* mh,intptr_t vmaddr_slide));

void _dyld_register_func_for_remove_image(
    void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));

void dyld_register_image_state_change_handler(
    enum dyld_image_states state,
    bool batch,
    dyld_image_state_change_handler handler);

const char* dyld_image_path_containing_address(
    const void* addr);


#endif // _DYLD_INCLUDED_

