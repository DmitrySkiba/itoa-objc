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

#include "sofcn.h"
#include <linux/elf.h>
#include <string.h>

///////////////////////////////////////////////// soinfo_t

soinfo_t* get_first_so() {
    static soinfo_t* first_so=0;
    if (!first_so) {
        first_so=(soinfo_t*)dlopen("libdl.so",RTLD_NOW | RTLD_GLOBAL);
    }
    return first_so;
}

soinfo_t* find_so(const char* name) {
    soinfo_t* i=get_first_so();
    for (;i;i=i->next) {
        if (!strcmp(i->name,name)) {
            return i;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////

