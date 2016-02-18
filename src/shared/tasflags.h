#ifndef TASFLAGS_H_INCLUDED
#define TASFLAGS_H_INCLUDED

struct TasFlags {
    int running; // is the game running or on pause
    int speed_divisor; // by how much is the speed reduced
    int recording; // are the input recorded or read
};

const struct TasFlags DEFAULTFLAGS = {0, 1, 0};

#endif // TAS_FLAGS_H_INCLUDED
