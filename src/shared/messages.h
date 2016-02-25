#ifndef MESSAGES_H_INCLUDED
#define MESSAGES_H_INCLUDED

enum {
    /* 
     * The game notices the program that he reaches a frame boundary.
     * Then he sends the frame number
     * Argument: unsigned long
     */
    MSGB_START_FRAMEBOUNDARY,

    /* 
     * Send all inputs to the game
     * Argument: struct AllInputs
     */
    MSGN_ALL_INPUTS,

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
     * Argument: pid_t
     */
    MSGB_PID,

    /*
     * Notice the program of the end of initialization messages
     * Argument: none
     */
    MSGB_END_INIT,

    /*
     * Notice the game of the end of initialization messages
     * Argument: none
     */
    MSGN_END_INIT,

    /*
     * Send the dump file to the game
     * Arguments: size_t (string length) then char[len]
     */
    MSGN_DUMP_FILE,

    /*
     * Send the X11 window identifier of the game to the program
     * Argument: int
     */
    MSGB_WINDOW_ID,
};

#endif // MESSAGES_H_INCLUDED
