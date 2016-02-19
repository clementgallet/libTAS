#!/bin/sh

SOURCE_FILES="main.c keymapping.c recording.c"
C_WARNINGS="-std=gnu99 -pedantic -Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow"
C_OPTIMISATIONS="-g -O1 -lX11"

[ -d ../../bin ] || mkdir ../../bin
gcc $SOURCE_FILES $C_WARNINGS $C_OPTIMISATIONS -o ../../bin/linTAS
