#include "inputs.h"

void emptyInputs(struct AllInputs* ai) {
    int i,j;
    for (i=0; i<ALLINPUTS_MAXKEY; i++)
        ai->keyboard[i] = XK_VoidSymbol;
    for (i=0; i<4; i++) {
        for (j=0; j<6; j++)
            ai->controller_axes[i][j] = 0;
        ai->controller_buttons[i] = 0;
    }
}
