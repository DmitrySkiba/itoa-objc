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

/*
  For some reason @catch(id) doesn't work without the following hack.

  Include this file to one (and only one) of your source files if you 
   use @catch(id) anywhere in the program.
  
  Catching other types works fine without this hack.
  
  ***

  In Android 1.6 linker simply couldn't find OBJC_EHTYPE_id, 
   and refused to load images which used @catch(id), so this hack was
   necessary.
  In 2.3 linker loads images, but program crashes on @catch(id). So
   hack is here to stay.
  
  Crash occurs in the __cxa_type_match function which is being called
   from __gxx_personality_v0 function which resides in 
   libstdc++-v3\libsupc++\eh_personality.cc (search for macro get_adjusted_ptr
   which wraps __cxa_type_match). Crash occurs because bogus catch_info
   is being passed to __cxa_type_match.
*/


/* We can't specify vtable directly, since it is not static.
   Hence the 'fix' constructor function. */
__attribute__((weak)) struct _objc_ehtype_t OBJC_EHTYPE_id={
    0,
    (char*)"id",
    0
};
__attribute__((weak, constructor)) void fix_OBJC_EHTYPE_id_vtable() {
    extern const void** _objc_ehtype_vtable;
    OBJC_EHTYPE_id._objc_ehtype_vtable=_objc_ehtype_vtable+2;
}

