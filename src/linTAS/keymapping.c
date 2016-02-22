#include "keymapping.h"

void default_hotkeys(KeySym *hotkeys){

    hotkeys[HOTKEY_PLAYPAUSE] = XK_Pause;
    hotkeys[HOTKEY_FRAMEADVANCE] = XK_v;
    hotkeys[HOTKEY_FASTFORWARD] = XK_Tab;
    hotkeys[HOTKEY_READWRITE] = XK_o;
    hotkeys[HOTKEY_SAVESTATE] = XK_s;
    hotkeys[HOTKEY_LOADSTATE] = XK_l;
}

/* 
 * Format the keyboard state to be saved and passed to the game
 * It consists on converting keycode to keysyms, removing pressed hotkeys,
 * and listing the remaining pressed keys.
 */
void format_keyboard(struct AllInputs* ai, Display *display, char keyboard_state[], KeySym hotkeys[]){
    int i,j,k;
    int keysym_i = 0;

    /* Initialize keyboard array */
    for (i=0; i<ALLINPUTS_MAXKEY; i++)
        ai->keyboard[i] = XK_VoidSymbol;

    for (i=0; i<32; i++) {
        if (keyboard_state[i] == 0)
            continue;
        for (j=0; j<8; j++) {
            if ((keyboard_state[i] >> j) & 0x1) {

                /* We got a pressed keycode */
                KeyCode kc = (i << 3) | j;
                /* Translating to keysym */
                KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);

                for (k=0; k<HOTKEY_LEN; k++) {
                    if (hotkeys[k] == ks) {
                        /* The pressed key was in fact a hotkey. */
                        break;
                    }
                }

                if (k == HOTKEY_LEN) {

                    /* This is not a hotkey */
                    /* Checking the current number of keys */
                    if (keysym_i >= ALLINPUTS_MAXKEY) {
                        fprintf(stderr, "We reached the maximum number of inputs (%d), skipping the rest", ALLINPUTS_MAXKEY);
                        return;
                    }

                    /* Saving the key */
                    ai->keyboard[keysym_i++] = ks;
                }
            }
        }
    }
}


