/* This code generates a C header that contains the list of keysym descriptions,
 * so we don't depend on XKeysymToString.
 * It first write a single string containing all key descriptions,
 * and an array that maps a keysym to an offset in the big string.
 * This is done for the two keysym groups: LATIN1 (0x00ab) and MISC (0xffab)
 * 
 * Can be compiled with: gcc -o genkeydesc genkeydesc.c -lxcb -lX11 -lxcb-keysyms
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <string.h>
#include <stdio.h>

int main()
{
    xcb_connection_t *conn = xcb_connect(NULL,NULL);
    xcb_key_symbols_t *keysyms = xcb_key_symbols_alloc(conn);

    /* LATIN1 */
    int str_offset_latin[256];
    int offset = 0;
    
    for (int ks = 0; ks < 256; ks++) {
        char* str = XKeysymToString(ks);
        if (str) {
            printf("\t\"%s\\0\"\n", str);
            str_offset_latin[ks] = offset;
            offset += strlen(str) + 1;
        }
        else
            str_offset_latin[ks] = -1;
    }

    /* MISC */
    printf("\n\n");
    int str_offset_misc[256];
    offset = 0;
    for (int ks = 0; ks < 256; ks++) {
        char* str = XKeysymToString(0xff00 | ks);
        if (str) {
            printf("\t\"%s\\0\"\n", str);
            str_offset_misc[ks] = offset;
            offset += strlen(str) + 1;
        }
        else
            str_offset_misc[ks] = -1;
    }

    /* Print offset tables */
    printf("\n\n");
    for (int ks = 0; ks < 256; ks++) {
        printf("\t%d,\n", str_offset_latin[ks]);
    }
    printf("\n\n");
    for (int ks = 0; ks < 256; ks++) {
        printf("\t%d,\n", str_offset_misc[ks]);
    }

    return 0;
}
