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

#import "dyld.h"
#import "sofcn.h"
#import "objc-private.h"

dyld_image_state_change_handler dyld_handlers[DYLD_IMAGE_STATE_COUNT]={0};

__private_extern__
void dyld_register_image_state_change_handler(
    enum dyld_image_states state,
    bool batch,
    dyld_image_state_change_handler handler)
{
    if (dyld_handlers[state]) {
        _objc_fatal("dyld handler for event %d is already set.",state);
    }
    dyld_handlers[state]=handler;
}

__private_extern__
char *getsectdatafromheader(
    const struct mach_header *mhp,
    const char *segname,
    const char *sectname,
    uint32_t *size)
{
    if (!strcmp(sectname,"__objc_imageinfo")) {
        static objc_image_info image_info={
            version: 0,
            flags: 0
        };
        if (!dlsym(mhp->handle,"__start___objc_classlist") &&
            !dlsym(mhp->handle,"__start___objc_protolist") &&
            !dlsym(mhp->handle,"__start___objc_catlist"))
        {
            *size=0;
            return NULL;
        }
        *size=sizeof(image_info);
        return (char*)&image_info;
    }
    void* start;
    void* stop;
    {
        static const char* const start_tag="__start_";
        static const char* const stop_tag="__stop_";
        char symbol_name[strlen(start_tag)+strlen(sectname)+1];
        strcpy(symbol_name,start_tag);
        strcpy(symbol_name+strlen(start_tag),sectname);
        start=dlsym(mhp->handle,symbol_name);
        strcpy(symbol_name,stop_tag);
        strcpy(symbol_name+strlen(stop_tag),sectname);
        stop=dlsym(mhp->handle,symbol_name);
    }
    if (!start || !stop) {
        *size=0;
        return NULL;
    }
    *size=(int)stop-(int)start;
    return start;
}

__private_extern__
void _dyld_register_func_for_add_image(
    void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide))
{
}

__private_extern__
void _dyld_register_func_for_remove_image(
    void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide))
{
}

__private_extern__ 
const char* dyld_image_path_containing_address(const void* addr) {
    return "";
}

static
void call_dyld_handlers_on(soinfo_t* so) {
    struct image_t {
        struct mach_header header;
        struct segment_command segment;
    };
    
    struct image_t* image=(struct image_t*)malloc(sizeof(struct image_t));
    struct dyld_image_info image_info={&image->header};
    unsigned i;

    image->header.magic=MH_MAGIC;
    image->header.filetype=MH_BUNDLE;
    image->header.ncmds=1;
    image->header.handle=so;
    image->segment.cmd=LC_SEGMENT;
    image->segment.cmdsize=sizeof(struct segment_command);
    strcpy(image->segment.segname,SEG_DATA);
    image->segment.vmaddr=(uint32_t)&image->header;
    image->segment.fileoff=0;
    image->segment.filesize=so->size;
    image->segment.nsects=0;
    
    for (i=0;i!=DYLD_IMAGE_STATE_COUNT;++i) {
        if (dyld_handlers[i]) {
            dyld_handlers[i]((enum dyld_image_states)i,1,&image_info);
        }
    }
    
    // Leak is intentional - we protect from getting the same memory
    // region twice. Objc compares addresses of headers, and hence
    // will not load second library.
    //free(image);
}

extern void call_dyld_handlers() {
    soinfo_t* i=get_first_so();
    i=i->next; // skip libdl.so
    for (;i;i=i->next) {
        call_dyld_handlers_on(i);
    }
}
