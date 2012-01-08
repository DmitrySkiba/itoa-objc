#
# Copyright (C) 2011 Dmitry Skiba
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

MODULE_PATH := $(call my-dir)

include $(CLEAR_VARS)

MODULE_NAME := objc

$(call itoa-sysroot-copy-files,\
    $(MODULE_PATH)/include/objc,usr/include/objc)

$(eval $(call itoa-copy-file,\
    $(MODULE_PATH)/APPLE_LICENSE,lib$(MODULE_NAME).APPLE_LICENSE))
$(eval $(call itoa-copy-file,\
    $(MODULE_PATH)/APACHE_LICENSE,lib$(MODULE_NAME).APACHE_LICENSE))
$(eval $(call itoa-copy-file,\
    $(MODULE_PATH)/LICENSE.txt,lib$(MODULE_NAME).LICENSE.txt))

MODULE_CFLAGS += \
    -I$(MODULE_PATH)/include \
    -I$(MODULE_PATH)/src/objc \
    \
    -DNO_BUILTINS \
    -DNO_DEBUGGER_MODE \
    \
    -fdollars-in-identifiers \

MODULE_SRC_FILES += \
    src/objc/hashtable2.m \
    src/objc/maptable.m \
    src/objc/objc-cache.m \
    src/objc/objc-class.m \
    src/objc/objc-errors.m \
    src/objc/objc-os.m \
    src/objc/objc-file.m \
    src/objc/objc-initialize.m \
    src/objc/objc-auto.m \
    src/objc/objc-sync.m \
    src/objc/objc-layout.m \
    src/objc/objc-runtime.m \
    src/objc/objc-runtime-new.m \
    src/objc/objc-typeencoding.m \
    src/objc/objc-exception.m \
    src/objc/objc-exception-init.mm \
    src/objc/objc-loadmethod.m \
    src/objc/objc-sel.mm \
    src/objc/objc-sel-set.m \
    src/objc/objc-references.mm \
    src/objc/objc-msg-arm.S \
    src/objc/objc-accessors.m \
    src/objc/Object.m \
    src/objc/Protocol.m \
    \
    src/objc/android/objc-syslog.m \
    src/objc/android/dyld.m \
    src/objc/android/sofcn.c \
    
MODULE_LDLIBS := -llog -ldl
MODULE_SHARED_LIBRARIES := macemu
    
include $(BUILD_SHARED_LIBRARY)

