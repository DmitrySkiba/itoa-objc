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

#include <typeinfo>

extern "C" void _objc_init_ehtype_vtable() {
    extern const void** _objc_ehtype_vtable;
    /* Not sure &typeid(id) is right, but it really doesn't matter:
        result of comparasion in __cxa_type_match affects return type
        to be either ctm_succeeded_with_ptr_to_base or ctm_succeeded,
        and they both are good since the result is checked against
        ctm_failed. So any value that doesn't crash the program is
        good enough. */        
    _objc_ehtype_vtable[1]=&typeid(id);
}

