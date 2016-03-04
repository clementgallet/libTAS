#include "libTAS.h"

/* Handle to the SDL dynamic library that is shipped with the game */
void* SDL_handle;

/* Does SDL use OpenGL for display? */
int video_opengl = 0;

/* Message that identify what is sent in the socket */
int message;

/* Frame counter */
/* TODO: Where should I put this? */
unsigned long frame_counter = 0;

#ifdef LIBTAS_HUD
/* Font used for displaying HUD on top of the game window */
TTF_Font *font = NULL;
#endif

/* 
 * Store the game window pointer
 * (SDL_Window* in fact, but we don't need the SDL_Window struct)
 * We assume the game never open multiple windows at a time
 */
void* gameWindow = NULL;

/* Has the game window pointer be sent to the program? */
int gw_sent = 0;

char* dumpfile = NULL;
char* sdlfile = NULL;

void __attribute__((constructor)) init(void)
{

    initSocket();

#ifdef LIBTAS_HUD
    /* Initialize SDL TTF */
    if(TTF_Init() == -1) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't init SDL TTF.");
        exit(1);
    }

    font = TTF_OpenFont("/home/clement/libTAS/src/external/FreeSans.ttf", 30);
    if (font == NULL) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't load font");
        exit(1);
    }
#endif
    /* Send information to the program */

    /* Send game process pid */
    debuglog(LCF_SOCKET, "Send pid to program");
    sendMessage(MSGB_PID);
    pid_t mypid = getpid();
    sendData(&mypid, sizeof(pid_t));

    /* End message */
    sendMessage(MSGB_END_INIT);

    /* Receive information from the program */
    receiveData(&message, sizeof(int));
    while (message != MSGN_END_INIT) {
        switch (message) {
            case MSGN_TASFLAGS:
                debuglog(LCF_SOCKET, "Receiving tas flags");
                receiveData(&tasflags, sizeof(struct TasFlags));
                break;
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET, "Receiving dump filename");
                size_t str_len;
                receiveData(&str_len, sizeof(size_t));
                dumpfile = malloc(str_len * sizeof(char) + 1);
                receiveData(dumpfile, str_len * sizeof(char));
                dumpfile[str_len] = '\0';
                break;
            case MSGN_SDL_FILE:
                debuglog(LCF_SOCKET, "Receiving sdl filename");
                size_t sdl_len;
                receiveData(&sdl_len, sizeof(size_t));
                sdlfile = malloc(sdl_len * sizeof(char) + 1);
                receiveData(sdlfile, sdl_len * sizeof(char));
                sdlfile[sdl_len] = '\0';
                break;
            default:
                debuglog(LCF_ERROR | LCF_SOCKET, "Unknown socket message %d.", message);
                exit(1);
        }
        receiveData(&message, sizeof(int));
    }
        
    // SMB uses its own version of SDL.
    SDL_handle = dlopen(sdlfile, RTLD_LAZY);
    if (!SDL_handle)
    {
        debuglog(LCF_ERROR | LCF_HOOK, "Could not load SDL.");
        exit(-1);
    }

    if (!hook_functions(SDL_handle))
        exit(-1);

    emptyInputs(&ai);
    emptyInputs(&old_ai);

}

void __attribute__((destructor)) term(void)
{
#ifdef LIBTAS_HUD
    TTF_CloseFont(font);
    TTF_Quit();
#endif
    dlclose(SDL_handle);
    closeSocket();

    debuglog(LCF_SOCKET, "Exiting.");
}


/* Override */ void SDL_GL_SwapWindow(void* window)
{
    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, "%s call.", __func__);

#ifdef LIBTAS_HUD
    SDL_Color color = {255, 0, 0, 0};
    RenderText(font, "Test test", 640, 480, color, 2, 2);
#endif

    SDL_GL_SwapWindow_real(window);

#ifdef LIBTAS_DUMP
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
#endif

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
            sendMessage(MSGB_WINDOW_ID);
            sendData(&xgw, sizeof(Window));
            gw_sent = 1;
        }
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Window pointer is empty but the game wants to draw something.");
        return;
    }

    enterFrameBoundary();

}

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL, "%s call - setting to %d.", __func__, interval);
    return SDL_GL_SetSwapInterval_real(interval);
    //return SDL_GL_SetSwapInterval_real(0);
}
    

/* Override */ void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL, "%s call - title: %s, pos: (%d,%d), size: (%d,%d), flags: %d.", __func__, title, x, y, w, h, flags);
    gameWindow = SDL_CreateWindow_real(title, x, y, w, h, flags); // Save the game window
    if (flags & /* SDL_WINDOW_OPENGL */ 0x00000002)
        video_opengl = 1;

    /* A new window was created. It needs to be passed to the program */
    gw_sent = 0;
    return gameWindow;
}

/* Override */ void SDL_DestroyWindow(void* window){
    debuglog(LCF_SDL, "%s call.", __func__);
    SDL_DestroyWindow_real(window);
    if (gameWindow == window)
        gameWindow = NULL;
}

/* Override */ Uint32 SDL_GetWindowFlags(void* window){
    debuglog(LCF_SDL, "%s call.", __func__);
    return SDL_GetWindowFlags_real(window);
}

/* Override */ void SDL_Init(unsigned int flags){
    debuglog(LCF_SDL, "%s call.", __func__);
    SDL_Init_real(flags);

    /* Try to hook more functions after SDL was inited */
    late_hook();
}

/* Override */ void SDL_Quit(){
    debuglog(LCF_SDL, "%s call.", __func__);
    sendMessage(MSGB_QUIT);
#ifdef LIBTAS_DUMP
    if (tasflags.av_dumping)
        closeVideoDump();
#endif
    SDL_Quit_real();
}

/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType)
{
    debuglog(LCF_SDL | LCF_EVENTS, "%s call.", __func__);
    return SDL_PeepEvents_real(events, numevents, action, minType, maxType);
}

/* Override */ int SDL_PollEvent(SDL_Event *event)
{
    //return SDL_PollEvent_real(event);
    debuglog(LCF_SDL | LCF_EVENTS | LCF_FRAME, "%s call.", __func__);

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

    /* Important: You must call the following two functions in that order!
     * This is because the first one updates old_ai by removing elements
     * to match ai, and the second one updates old_ai by adding elements
     * to match ai. So if you call the second one before the first,
     * the keyboard array in old_ai can overflow.
     */
    int getEvent = generateKeyUpEvent(event, gameWindow);
    if (getEvent)
        return 1;

    getEvent = generateKeyDownEvent(event, gameWindow);
    if (getEvent)
        return 1;

    getEvent = generateControllerEvent(event);
    if (getEvent)
        return 1;

    return 0;
}

