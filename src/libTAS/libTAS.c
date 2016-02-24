#include "libTAS.h"

void* SDL_handle;
int video_opengl = 0;

/* Socket to communicate to the program */
int socket_fd = 0;

/* Frame counter */
unsigned long frame_counter = 0;

/* 
 * Store the game window pointer
 * We assume the game never handle multiple windows at a time
 */
void* gameWindow = NULL;

/* Has the game window pointer be sent to the program? */
int gw_sent = 0;

char* dumpfile = NULL;

KeySym xkeyboard[16] = {0}; // TODO: Remove this magic number
KeySym old_xkeyboard[16] = {0};

void __attribute__((constructor)) init(void)
{
    
    // SMB uses its own version of SDL.
    SDL_handle = dlopen("/home/clement/supermeatboy/amd64/libSDL2-2.0.so.0",
                        RTLD_LAZY);
    if (!SDL_handle)
    {
        debuglog(LCF_ERROR | LCF_HOOK, "Could not load SDL.");
        exit(-1);
    }

    if (!hook_functions(SDL_handle))
        exit(-1);

    if (!unlink(SOCKET_FILENAME))
        debuglog(LCF_SOCKET, "Removed stall socket.");

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        debuglog(LCF_ERROR | LCF_SOCKET, "Couldn't bind client socket.");
        exit(-1);
    }

    if (listen(tmp_fd, 1))
    {
        debuglog(LCF_ERROR | LCF_SOCKET, "Couldn't listen on client socket.");
        exit(1);
    }

    debuglog(LCF_SOCKET, "Loading complete, awaiting client connection...");

    if ((socket_fd = accept(tmp_fd, NULL, NULL)) < 0)
    {
        debuglog(LCF_ERROR | LCF_SOCKET, "Couldn't accept client connection.");
        exit(1);
    }

    debuglog(LCF_SOCKET, "Client connected.");

    close(tmp_fd);
    unlink(SOCKET_FILENAME);

    int message = MSGB_PID;
    /* Send information to the program */

    /* Send game process pid */
    debuglog(LCF_SOCKET, "Send pid to program");
    send(socket_fd, &message, sizeof(int), 0);
    pid_t mypid = getpid();
    send(socket_fd, &mypid, sizeof(pid_t), 0);

    /* End message */
    message = MSGB_END_INIT;
    send(socket_fd, &message, sizeof(int), 0);

    /* Receive information from the program */
    recv(socket_fd, &message, sizeof(int), 0);
    while (message != MSGN_END_INIT) {
        switch (message) {
            case MSGN_TASFLAGS:
                debuglog(LCF_SOCKET, "Receiving tas flags");
                recv(socket_fd, &tasflags, sizeof(struct TasFlags), 0);
                break;
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET, "Receiving dump filename");
                size_t str_len;
                recv(socket_fd, &str_len, sizeof(size_t), 0);
                dumpfile = malloc(str_len * sizeof(char) + 1);
                recv(socket_fd, dumpfile, str_len * sizeof(char), 0);
                dumpfile[str_len] = '\0';
                break;
            default:
                debuglog(LCF_ERROR | LCF_SOCKET, "Unknown socket message %d.", message);
                exit(1);
        }
        recv(socket_fd, &message, sizeof(int), 0);
    }
        

    int i;
    for (i=0; i<16; i++)
        old_xkeyboard[i] = XK_VoidSymbol;

    X11_InitKeymap();

    //proceed_commands();
}

void __attribute__((destructor)) term(void)
{
    dlclose(SDL_handle);
    close(socket_fd);

    debuglog(LCF_SOCKET, "Exiting.");
}

void SDL_GL_SwapWindow(void)
{
    /* Apparently, we must not put any code here before the SwapWindow call */
    SDL_GL_SwapWindow_real();

    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, "%s call.", __func__);

    /* Dumping audio and video if needed */
    static int dump_inited = 0;
    if (tasflags.av_dumping) {
        if (! dump_inited) {
            /* Initializing the video dump */
            int av = openVideoDump(gameWindow, video_opengl, dumpfile);
            dump_inited = 1;
            if (av != 0)
                /* Init failed, disable AV dumping */
                tasflags.av_dumping = 0;
        }
    }

    if (tasflags.av_dumping) {
        /* Write the current frame */
        int enc = encodeOneFrame(frame_counter, gameWindow);
        if (enc != 0)
            /* Encode failed, disable AV dumping */
            tasflags.av_dumping = 0;

    }

    /* Advance deterministic time by one frame */
    advanceFrame();

    int message;

    /* 
     * We need to pass the game window identifier to the program
     * so that it can capture inputs
     */
    if (gameWindow != NULL) {
        if (!gw_sent) {

            /* Access the X Window identifier from the SDL_Window struct */
            SDL_SysWMinfo info;
            SDL_GetVersion_real(&info.version);
            if (SDL_GetWindowWMInfo_real(gameWindow, &info) == SDL_FALSE) {
                debuglog(LCF_SDL | LCF_ERROR, "Could not get the X11 window identifier");
                return;
            }
            if (info.subsystem != SDL_SYSWM_X11) {
                debuglog(LCF_SDL | LCF_ERROR, "SDL says we are not running on X11");
                return;
            }
            Window xgw = info.info.x11.window;

            /* Send the X Window identifier to the program */
            message = MSGB_WINDOW_ID;
            send(socket_fd, &message, sizeof(int), 0);
            send(socket_fd, &xgw, sizeof(Window), 0);
            gw_sent = 1;
        }
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Window pointer is empty but the game wants to draw something.");
        return;
    }

    message = MSGB_START_FRAMEBOUNDARY;
    send(socket_fd, &message, sizeof(int), 0);
    send(socket_fd, &frame_counter, sizeof(unsigned long), 0);

    if (tasflags.running && tasflags.speed_divisor > 1)
        usleep_real(1000000 * (tasflags.speed_divisor - 1) / 60);

    struct TasFlags oldflags = tasflags;
    proceed_commands();

    /* Do the rest of the stuff before finishing the frame */
    if (oldflags.fastforward != tasflags.fastforward)
        SDL_GL_SetSwapInterval_real(!tasflags.fastforward);


    ++frame_counter;
}

int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL, "%s call - setting to %d.", __func__, interval);
    return SDL_GL_SetSwapInterval_real(interval);
}
    

void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL, "%s call - title: %s, pos: (%d,%d), size: (%d,%d), flags: %d.", __func__, title, x, y, w, h, flags);
    gameWindow = SDL_CreateWindow_real(title, x, y, w, h, flags); // Save the game window
    if (flags & /* SDL_WINDOW_OPENGL */ 0x00000002)
        video_opengl = 1;

    /* A new window was created. It needs to be passed to the program */
    gw_sent = 0;
    return gameWindow;
}

void SDL_DestroyWindow(void* window){
    debuglog(LCF_SDL, "%s call.", __func__);
    SDL_DestroyWindow_real(window);
    if (gameWindow == window)
        gameWindow = NULL;
}

Uint32 SDL_GetWindowFlags(void* window){
    debuglog(LCF_SDL, "%s call.", __func__);
    return SDL_GetWindowFlags_real(window);
}

void SDL_Quit(){
    debuglog(LCF_SDL, "%s call.", __func__);
    int message = MSGB_QUIT;
    send(socket_fd, &message, sizeof(int), 0);
    if (tasflags.av_dumping)
        closeVideoDump();
}

const Uint8* SDL_GetKeyboardState(__attribute__ ((unused)) int* numkeys)
{
    debuglog(LCF_SDL | LCF_KEYBOARD, "%s call.", __func__);
    xkeyboardToSDLkeyboard(xkeyboard, keyboard_state);
    //*numkeys = 512;
    return keyboard_state;
}

int SDL_PeepEvents(SDL_Event*      events,
                   int             numevents,
                   SDL_eventaction action,
                   Uint32          minType,
                   Uint32          maxType)
{
    debuglog(LCF_SDL | LCF_EVENTS, "%s call.", __func__);
    return SDL_PeepEvents_real(events, numevents, action, minType, maxType);
}

int SDL_PollEvent(SDL_Event *event)
{
    //return SDL_PollEvent_real(event);
    debuglog(LCF_SDL | LCF_EVENTS, "%s call.", __func__);

    int isone = SDL_PollEvent_real(event);
    while (isone == 1){
        /* TODO: Make a proper event filtering! */
        if ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)){
            /*
               if (event.type == SDL_KEYDOWN)
               printf("KEYDOWN ");
               if (event.type == SDL_KEYUP)
               printf("KEYUP ");
            //printf("windowID: %d\n", event.key.windowID);
            //printf("timestamp: %d\n", event.key.timestamp);
            printf("sym key: %d ", event.key.keysym.sym);
            printf("scan key: %d\n", event.key.keysym.scancode);
            */
        }
        else if (event->type == SDL_WINDOWEVENT){
            switch (event->window.event) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window %d gained keyboard focus.", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window %d lost keyboard focus.", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window %d closed.", event->window.windowID);
                    break;
                default:
                    break;
            }
            return 1;
        }
        else {
            return 1;
        }
        isone = SDL_PollEvent_real(event);
    }


    /* Generate released keyboard input events */

    int i, j;
    for (i=0; i<16; i++) { // TODO: Another magic number
        if (old_xkeyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<16; j++) {
            if (old_xkeyboard[i] == xkeyboard[j]) {
                /* Key was not released */
                break;
            }
        }
        if (j == 16) {
            /* Key was released. Generate event */
            event->type = SDL_KEYUP;
            event->key.state = SDL_RELEASED;
            event->key.windowID = SDL_GetWindowID_real(gameWindow);
            event->key.timestamp = SDL_GetTicks_real() - 1; // TODO: Should use our deterministic timer instead

            SDL_Keysym keysym;
            xkeysymToSDL(&keysym, old_xkeyboard[i]);
            event->key.keysym = keysym;

            /* Update old keyboard state so that this event won't trigger inifinitely */
            old_xkeyboard[i] = XK_VoidSymbol;
            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key %d.", event->key.keysym.sym);
            return 1;

        }
    }

    /* Generate pressed keyboard input events */

    int k;
    for (i=0; i<16; i++) { // TODO: Another magic number
        if (xkeyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<16; j++) {
            if (xkeyboard[i] == old_xkeyboard[j]) {
                /* Key was not pressed */
                break;
            }
        }
        if (j == 16) {
            /* Key was pressed. Generate event */
            event->type = SDL_KEYDOWN;
            event->key.state = SDL_PRESSED;
            event->key.windowID = SDL_GetWindowID_real(gameWindow);
            event->key.timestamp = SDL_GetTicks_real() - 1; // TODO: Should use our deterministic timer instead

            SDL_Keysym keysym;
            xkeysymToSDL(&keysym, xkeyboard[i]);
            event->key.keysym = keysym;

            /* Update old keyboard state so that this event won't trigger inifinitely */
            for (k=0; k<16; k++)
                if (old_xkeyboard[k] == XK_VoidSymbol) {
                    /* We found an empty space to put our key*/
                    old_xkeyboard[k] = xkeyboard[i];
                    break;
                }

            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key %d.", event->key.keysym.sym);
            return 1;

        }
    }
    return 0;
}

void proceed_commands(void)
{
    while (1)
    {
        int message;

        recv(socket_fd, &message, sizeof(int), 0);
        switch (message)
        {
            case MSGN_TASFLAGS:
                recv(socket_fd, &tasflags, sizeof(struct TasFlags), 0);
                break;

            case MSGN_END_FRAMEBOUNDARY:
                return;

            case MSGN_KEYBOARD_INPUT:
                recv(socket_fd, xkeyboard, 32 * sizeof(char), 0);
                break;

        }
    }
}

