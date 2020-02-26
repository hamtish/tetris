#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw( void ) {
    int getsw = 0;
    getsw = PORTD & 0x0f00;
    getsw = getsw >> 8;
    return getsw;
}

int getbtns( void ) {
    int getbtns = 0;
    getbtns = PORTD & 0x0e0;
    getbtns = getbtns >> 5;
    return getbtns;
}