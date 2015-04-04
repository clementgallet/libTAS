#!/bin/sh

LIB_SOURCE_FILES="libTAS.c"
C_WARNINGS="-std=c99 -pedantic -Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow -Wno-unused-parameter -shared -fpic"
C_OPTIMISATIONS="-g -O1"

[ -d ../../bin ] || mkdir ../../bin
gcc $LIB_SOURCE_FILES $C_WARNINGS $C_OPTIMISATIONS -o ../../bin/libTAS.so
