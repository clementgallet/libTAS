#ifndef INPUTS_H_INCLUDED
#define INPUTS_H_INCLUDED

#define ALLINPUTS_MAXKEY 16

#include <X11/Xlib.h> // For the KeySym type
#include <X11/keysym.h>

class AllInputs {
    public:
        KeySym keyboard[ALLINPUTS_MAXKEY];
        short controller_axes[4][6];
        unsigned short controller_buttons[4];

        void emptyInputs();
};


#endif // INPUTS_H_INCLUDED
