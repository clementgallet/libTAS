#ifndef INPUTS_H_INCLUDED
#define INPUTS_H_INCLUDED

#define ALLINPUTS_MAXKEY 16

#include <X11/Xlib.h> // For the KeySym type

struct AllInputs {
    KeySym keyboard[ALLINPUTS_MAXKEY];
};

#endif // INPUTS_H_INCLUDED
