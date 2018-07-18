#include "hooklib2.h"
#include "hooklib3.h"

int libtasTestFunc2()
{
    return 1;
}

int libtasTestCallingFunc3()
{
    return libtasTestFunc3();
}
