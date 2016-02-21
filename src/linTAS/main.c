#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/tasflags.h"
#include "../shared/messages.h"
#include "keymapping.h"
#include "recording.h"
#include "savestates.h"

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

struct State savestate;
int didSave = 0;

unsigned long int frame_counter = 0;

char keyboard_state[32];
KeySym hotkeys[HOTKEY_LEN];

char *moviefile = NULL;
FILE* fp;

pid_t game_pid;

static int MyErrorHandler(Display *display, XErrorEvent *theEvent)
{
    (void) fprintf(stderr,
		   "Ignoring Xlib error: error code %d request code %d\n",
		   theEvent->error_code,
		   theEvent->request_code);

    return 0;
}

int main(int argc, char **argv)
{
    int message;

    /* Parsing arguments */
    int c;
    while ((c = getopt (argc, argv, "r:w:")) != -1)
        switch (c) {
            case 'r':
                tasflags.recording = 0;
                moviefile = optarg;
                break;
            case 'w':
                tasflags.recording = 1;
                moviefile = optarg;
                break;
            case '?':
                fprintf (stderr, "Unknown option character");
                break;
            default:
                return 1;
        }

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    Display *display;
    XEvent event;
    // Find the window which has the current keyboard focus
    Window win_focus;
    int revert;
    struct timespec tim;

    XSetErrorHandler(MyErrorHandler);

    /* open connection with the server */
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }


    printf("Connecting to libTAS...\n");

    if (connect(socket_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        printf("Couldn’t connect to socket.\n");
        return 1;
    }

    printf("Connected.\n");

    /* Share informations */

    /* Get the game process pid */
    recv(socket_fd, &message, sizeof(int), 0);
    if (message == MSGB_PID) {
        recv(socket_fd, &game_pid, sizeof(pid_t), 0);
    }
    else {
        fprintf(stderr, "Error in msg socket, waiting for game pid\n");
        exit(1);
    }

    tim.tv_sec  = 1;
    tim.tv_nsec = 0L;

    nanosleep(&tim, NULL);

    XGetInputFocus(display, &win_focus, &revert);
    XSelectInput(display, win_focus, KeyPressMask);

    default_hotkeys(hotkeys);

    if (tasflags.recording >= 0){
        fp = openRecording(moviefile, tasflags.recording);
    }

    while (1)
    {
        
        /* Wait for frame boundary */
        recv(socket_fd, &message, sizeof(int), 0);

        if (message == MSGB_QUIT) {
            printf("Game has quit. Exiting\n");
            break;
        }

        if (message != MSGB_START_FRAMEBOUNDARY) {
            printf("Error in msg socket, waiting for frame boundary\n");
            exit(1);
        }
                   
        recv(socket_fd, &frame_counter, sizeof(unsigned long), 0);


        int isidle = !tasflags.running;
        int tasflagsmod = 0; // register if tasflags have been modified on this frame
            
        /* We are at a frame boundary */
        do {

            XGetInputFocus(display, &win_focus, &revert);
            XSelectInput(display, win_focus, KeyPressMask | KeyReleaseMask);

            XQueryKeymap(display, keyboard_state);

            while( XPending( display ) > 0 ) {

                XNextEvent(display, &event);

                if (event.type == KeyPress)
                {
                    KeyCode kc = ((XKeyPressedEvent*)&event)->keycode;
                    KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
                    //s = XKeysymToString(ks);

                    if (ks == hotkeys[HOTKEY_FRAMEADVANCE]){
                        isidle = 0;
                        tasflags.running = 0;
                        tasflagsmod = 1;
                    }
                    if (ks == hotkeys[HOTKEY_PLAYPAUSE]){
                        tasflags.running = !tasflags.running;
                        tasflagsmod = 1;
                        isidle = !tasflags.running;
                    }
                    if (ks == hotkeys[HOTKEY_FASTFORWARD]){
                        tasflags.fastforward = 1;
                        tasflagsmod = 1;
                    }
                    if (ks == hotkeys[HOTKEY_SAVESTATE]){
                        saveState(game_pid, &savestate);
                        didSave = 1;
                    }
                    if (ks == hotkeys[HOTKEY_LOADSTATE]){
                        if (didSave)
                            loadState(game_pid, &savestate);
                    }
                    if (ks == hotkeys[HOTKEY_READWRITE]){
                        /* TODO: Use enum instead of values */
                        if (tasflags.recording >= 0)
                            tasflags.recording = !tasflags.recording;
                        if (tasflags.recording == 1)
                            truncateRecording(fp);
                        tasflagsmod = 1;
                    }
                }
                if (event.type == KeyRelease)
                {
                    KeyCode kc = ((XKeyPressedEvent*)&event)->keycode;
                    KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
                    if (ks == hotkeys[HOTKEY_FASTFORWARD]){
                        tasflags.fastforward = 0;
                        tasflagsmod = 1;
                    }
                }
            }

            /* Sleep a bit to not surcharge the processor */
            if (isidle) {
                tim.tv_sec  = 0;
                tim.tv_nsec = 10000000L;
                nanosleep(&tim, NULL);
            }

        } while (isidle);

        struct AllInputs ai;

        if (tasflags.recording == -1) {
            /* Grab keyboard inputs */
            XQueryKeymap(display, keyboard_state);

            /* Remove hotkeys from the keyboard state array */
            remove_hotkeys(display, keyboard_state, hotkeys);

            /* Build input struct */
            memmove(ai.keyboard, keyboard_state, 32*sizeof(char));
        }

        if (tasflags.recording == 1) {
            /* Grab keyboard inputs */
            XQueryKeymap(display, keyboard_state);

            /* Remove hotkeys from the keyboard state array */
            remove_hotkeys(display, keyboard_state, hotkeys);

            /* Build input struct */
            memmove(ai.keyboard, keyboard_state, 32*sizeof(char));

            /* Save inputs to file */
            if (!writeFrame(fp, frame_counter, ai))
                tasflags.recording = -1;
        }

        if (tasflags.recording == 0) {
            /* Save inputs to file */
            if (!readFrame(fp, frame_counter, &ai))
                tasflags.recording = -1;
        }

        /* Send tasflags if modified */
        if (tasflagsmod) {
            message = MSGN_TASFLAGS;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &tasflags, sizeof(struct TasFlags), 0);
        }

        /* Send inputs and end of frame */
        message = MSGN_KEYBOARD_INPUT;
        send(socket_fd, &message, sizeof(int), 0);
        send(socket_fd, ai.keyboard, 32 * sizeof(char), 0);

        message = MSGN_END_FRAMEBOUNDARY; 
        send(socket_fd, &message, sizeof(int), 0);

    }

    if (didSave)
        deallocState(&savestate);
    if (tasflags.recording >= 0){
        closeRecording(fp);
    }
    close(socket_fd);
    return 0;
}

