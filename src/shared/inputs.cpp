#include "inputs.h"

void AllInputs::emptyInputs() {
    int i,j;
    for (i=0; i<ALLINPUTS_MAXKEY; i++)
        keyboard[i] = XK_VoidSymbol;
    for (i=0; i<4; i++) {
        for (j=0; j<6; j++)
            controller_axes[i][j] = 0;
        controller_buttons[i] = 0;
    }
}
