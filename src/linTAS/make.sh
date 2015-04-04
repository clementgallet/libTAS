#!/bin/sh

SOURCE_FILES="main.c"
C_WARNINGS="-std=c99 -pedantic -Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow -Wno-unused-parameter"
C_OPTIMISATIONS="-g -O1"

[ -d ../../bin ] || mkdir ../../bin
gcc $SOURCE_FILES $C_WARNINGS $C_OPTIMISATIONS -o ../../bin/linTAS
