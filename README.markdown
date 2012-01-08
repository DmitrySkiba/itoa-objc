This is an Apple's objc4 runtime library ported to Android.

Changes:

* Non OBJC2 code removed
* GC-related code removed
* Exception handling code modified to use DWARF model (instead of sjlj)
* Assembly code adapted to standard GCC assembler (non Apple's)
* Dyld facilities emulated at a minimum level necessary to load images
* etc.

This project is a part of [Itoa](itoaproject.com).
