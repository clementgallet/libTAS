#ifndef MESSAGES_H_INCLUDED
#define MESSAGES_H_INCLUDED

enum {
    /* 
     * The game notices the program that he reaches a frame boundary.
     * The he sends the frame number
     * Argument: unsigned long)
     */
    MSGB_START_FRAMEBOUNDARY,

    /* 
     * Send keyboard inputs
     * Argument: char[32]
     */
    MSGN_KEYBOARD_INPUT,

    /*
     * Send tasflags
     * Argument: struct TasFlags
     */
    MSGN_TASFLAGS,

    /*
     * The programs tells the game to end the frame boundary
     * Argument: none
     */
    MSGN_END_FRAMEBOUNDARY,

    /*
     * The game tells the program that he has quit
     * Argument: none
     */
    MSGB_QUIT,

    /*
     * Send the game pid so that the program can attach to it
     */
    MSGB_PID,
};

#endif // MESSAGES_H_INCLUDED
