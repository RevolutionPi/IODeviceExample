# IODeviceExample
This repository contains the code for a RevPi DIO module. It can be used as origin for own IO modules.

Use the following link to download the gnu compiler to build the project for several operating system.
https://developer.arm.com/open-source/gnu-toolchain/gnu-rm

Please change the definition of the variable TOOLCHAIN_PATH in the makefile to the location of
your compiler.

Use 'make' to build the binary file. When you call make the first time you will see some warnings like this:
makefile:105: bsp/sw/bsp/bspError.d: Datei oder Verzeichnis nicht gefunden
You can ignore this. It is displayed because the .d-files cannot be found but they are generated immediately.

We use a 'Power Debug Interface USB 3' from Lauterbach (lauterbach.com) to download the elf-file to the
arm processor. But other debuggers can be used also.

