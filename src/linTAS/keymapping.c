#include "keymapping.h"

void default_hotkeys(KeySym *hotkeys){

    hotkeys[HOTKEY_PLAYPAUSE] = XK_Pause;
    hotkeys[HOTKEY_FRAMEADVANCE] = XK_v;
    hotkeys[HOTKEY_FASTFORWARD] = XK_Tab;
    hotkeys[HOTKEY_READWRITE] = XK_o;
    hotkeys[HOTKEY_SAVESTATE] = XK_s;
}

/* 
 * Remove hotkeys from the keyboard state array before passing to the game
 */
void remove_hotkeys(Display *display, char keyboard_state[], KeySym hotkeys[]){
    int i,j,k;

    for (i=0; i<32; i++) {
        if (keyboard_state[i] == 0)
            continue;
        for (j=0; j<8; j++) {
            if ((keyboard_state[i] >> j) & 0x1) {
                KeyCode kc = (i << 3) | j; // a KeyCode that is being pressed
                for (k=0; k<HOTKEY_LEN; k++) {
                    if (XkbKeycodeToKeysym(display, kc, 0, 0) == hotkeys[k]) {
                        /* We found a hotkey in the keyboard state. Removing it */
                        keyboard_state[i] ^= (1 << j);
                        break;
                    }
                }
            }
        }
    }
}
