#include "libTAS.h"

void* SDL_handle;

struct timeval current_time = { 0, 0 };
unsigned long frame_counter = 0;
unsigned char replaying = 0;
struct TasFlags tasflags;

unsigned char* recorded_inputs;
unsigned long max_inputs;
int replay_inputs_file;
unsigned long max_inputs_to_replay;

int socket_fd = 0;
void * gameWindow;

char xkeyboard[32] = {0};
char old_xkeyboard[32] = {0};

Display *display;

void __attribute__((constructor)) init(void)
{
    tasflags = (struct TasFlags){0, 1, 0, 0, LCF_ERROR, LCF_NONE};
    
    // SMB uses its own version of SDL.
    SDL_handle = dlopen("/home/clement/supermeatboy/amd64/libSDL2-2.0.so.0",
                        RTLD_LAZY);
    if (!SDL_handle)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not load SDL.");
        exit(-1);
    }

    if (!hook_SDL(SDL_handle, tasflags))
        exit(-1);

    if (!unlink(SOCKET_FILENAME))
        debuglog(LCF_SOCKET, tasflags, "Removed stall socket.");

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        debuglog(LCF_ERROR | LCF_SOCKET, tasflags, "Couldn’t bind client socket.");
        exit(-1);
    }

    if (listen(tmp_fd, 1))
    {
        debuglog(LCF_ERROR | LCF_SOCKET, tasflags, "Couldn’t listen on client socket.");
        exit(1);
    }

    debuglog(LCF_SOCKET, tasflags, "Loading complete, awaiting client connection...");

    if ((socket_fd = accept(tmp_fd, NULL, NULL)) < 0)
    {
        debuglog(LCF_ERROR | LCF_SOCKET, tasflags, "Couldn’t accept client connection.");
        exit(1);
    }

    debuglog(LCF_SOCKET, tasflags, "Client connected.");

    close(tmp_fd);
    unlink(SOCKET_FILENAME);

    recorded_inputs = malloc(sizeof(unsigned char) * 8192);
    max_inputs = 8192;

    display = XOpenDisplay(NULL);
    X11_InitKeymap();

    //proceed_commands();
}

void __attribute__((destructor)) term(void)
{
    dlclose(SDL_handle);
    close(socket_fd);

    debuglog(LCF_SOCKET, tasflags, "Exiting.");
}

time_t time(time_t* t)
{
    debuglog(LCF_TIMEGET, tasflags, "%s call - returning %d.", __func__, (long)current_time.tv_sec);
    if (t)
        *t = current_time.tv_sec;
    return current_time.tv_sec;
}

int gettimeofday(struct timeval* tv, void* tz)
{
    debuglog(LCF_TIMEGET, tasflags, "%s call - returning (%d,%d).", __func__, (long)current_time.tv_sec, (long)current_time.tv_usec);
    *tv = current_time;
    return 0;
}

void SDL_Delay(Uint32 sleep)
{
    debuglog(LCF_SDL | LCF_SLEEP, tasflags, "%s call - sleep for %u ms.", __func__, sleep);
    usleep_real(sleep*1000);
}

int usleep(useconds_t usec)
{
    debuglog(LCF_SLEEP, tasflags, "%s call - sleep for %u us.", __func__, (unsigned int)usec);
    return usleep_real(usec);
}

Uint32 SDL_GetTicks(void)
{
    Uint32 msec = current_time.tv_sec*1000 + current_time.tv_usec/1000;
    debuglog(LCF_SDL | LCF_TIMEGET, tasflags, "%s call - returning %d.", __func__, msec);
    //return SDL_GetTicks_real();
    return msec;
}

void SDL_GL_SwapWindow(void)
{
    /* Apparently, we must not put any code here before the SwapWindow call */
    SDL_GL_SwapWindow_real();

    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, tasflags, "%s call.", __func__);

    /* Once the frame is drawn, we can increment the current time by 1/60 of a
       second. This does not give an integer number of microseconds though, so
       we have to keep track of the fractional part of the microseconds
       counter, and add a spare microsecond each time it's needed. */
    const unsigned int fps = 60;
    const unsigned int integer_increment = 1000000 / fps;
    const unsigned int fractional_increment = 1000000 % fps;
    static unsigned int fractional_part = fps / 2;

    current_time.tv_usec += integer_increment;
    fractional_part += fractional_increment;
    if (fractional_part >= fps)
    {
        ++current_time.tv_usec;
        fractional_part -= fps;
    }

    if (current_time.tv_usec == 1000000)
    {
        ++current_time.tv_sec;
        current_time.tv_usec = 0;
    }

    //record_inputs();

    /*
    if (replaying)
    {replay_inputs();
        return;
    }*/

    int message = MSGB_START_FRAMEBOUNDARY;
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
    debuglog(LCF_SDL | LCF_OGL, tasflags, "%s call - setting to %d.", __func__, interval);
    return SDL_GL_SetSwapInterval_real(interval);
}
    

void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL, tasflags, "%s call - title: %s, pos: (%d,%d), size: (%d,%d), flags: %d.", __func__, title, x, y, w, h, flags);
    gameWindow = SDL_CreateWindow_real(title, x, y, w, h, flags); // Save the game window
    return gameWindow;
}

Uint32 SDL_GetWindowFlags(void* window){
    debuglog(LCF_SDL, tasflags, "%s call.", __func__);
    return SDL_GetWindowFlags_real(window);
}


const Uint8* SDL_GetKeyboardState(int* numkeys)
{
    debuglog(LCF_SDL | LCF_KEYBOARD, tasflags, "%s call.", __func__);
    xkeyboardToSDLkeyboard(display, xkeyboard, keyboard_state);
    //*numkeys = 512;
    return keyboard_state;
}

int SDL_PeepEvents(SDL_Event*      events,
                   int             numevents,
                   SDL_eventaction action,
                   Uint32          minType,
                   Uint32          maxType)
{
    debuglog(LCF_SDL | LCF_EVENTS, tasflags, "%s call.", __func__);
    return SDL_PeepEvents_real(events, numevents, action, minType, maxType);
}

int SDL_PollEvent(SDL_Event *event)
{
    //return SDL_PollEvent_real(event);
    debuglog(LCF_SDL | LCF_EVENTS, tasflags, "%s call.", __func__);

    SDL_Event myEvent;
    int isone = SDL_PollEvent_real(&myEvent);
    while (isone == 1){
        if ((myEvent.type == SDL_KEYDOWN) || (myEvent.type == SDL_KEYUP)){
/*
            if (myEvent.type == SDL_KEYDOWN)
                printf("KEYDOWN ");
            if (myEvent.type == SDL_KEYUP)
                printf("KEYUP ");
            //printf("windowID: %d\n", myEvent.key.windowID);
            //printf("timestamp: %d\n", myEvent.key.timestamp);
            printf("sym key: %d ", myEvent.key.keysym.sym);
            printf("scan key: %d\n", myEvent.key.keysym.scancode);
*/
        }
	else {
//            printf("other event: %d\n", myEvent.type);
	}
        isone = SDL_PollEvent_real(&myEvent);
    }

    //printf("PollEvent\n");

    int i;
    for (i=0; i<32; i++) {
        if (xkeyboard[i] == old_xkeyboard[i])
            continue;

        int j;
        for (j=0; j<8; j++) {
            char old_key = (old_xkeyboard[i] >> j) & 0x1;
            char new_key = (xkeyboard[i] >> j) & 0x1;

            if (old_key != new_key) {
                char keycode = (i << 3) | j;

                if (new_key) {
                    event->type = SDL_KEYDOWN;
                    event->key.state = SDL_PRESSED;
                    //printf("Pressed ");
                }
                else {
                    event->type = SDL_KEYUP;
                    event->key.state = SDL_RELEASED;
                    //printf("Release ");
                }
                event->key.windowID = SDL_GetWindowID_real(gameWindow);
                event->key.timestamp = SDL_GetTicks_real() - 1;
                //printf("myWindowID: %d\n", event->key.windowID);
                SDL_Keysym keysym;
                xkeycodeToSDL(display, &keysym, keycode);

                //printf("mySym: %d ", keysym.sym);
                //keysym.scancode = 40;
                //printf("myScan: %d ", keysym.scancode);
                event->key.keysym = keysym;
                old_xkeyboard[i] ^= (1 << j);
                return 1;
            }
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
            //char filename_buffer[1024];
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

                /*
            case 10:
                recv(socket_fd, filename_buffer, 1024, 0);
                unsigned long first_frame;
                recv(socket_fd, &first_frame, sizeof(unsigned long), 0);
                int save_inputs_file = open(filename_buffer, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (save_inputs_file < 0)
                {
                    log_err("Couldn’t open inputs file.");
                    break;
                }
                if (first_frame >= frame_counter)
                {
                    log_err("Selected first frame deosn’t exist yet.");
                    close(save_inputs_file);
                    break;
                }
                log_err("Saving inputs...");
                if (write(save_inputs_file, recorded_inputs + first_frame, frame_counter - first_frame) < 0)
                {
                    log_err("Couldn’t save inputs.");
                    close(save_inputs_file);
                    break;
                }
                log_err("Inputs saved.");
                close(save_inputs_file);

                break;

            case 11:
                recv(socket_fd, filename_buffer, 1024, 0);
                replay_inputs_file = open(filename_buffer, O_RDONLY);
                unsigned char answer = replay_inputs_file >= 0;
                send(socket_fd, &answer, sizeof(unsigned char), 0);
                if (!answer)
                    break;

                log_err("Input file opened, replaying...");

                max_inputs_to_replay = lseek(replay_inputs_file, 0, SEEK_END);
                if (!max_inputs_to_replay)
                {
                    log_err("File is empty, no input to replay.");
                    close(replay_inputs_file);
                    break;
                }
                lseek(replay_inputs_file, 0, SEEK_SET);
                replaying = 1;
                replay_inputs();
                return;
*/
            default:
                log_err("Unknown command recieved.");
            }
    }
}

void record_inputs(void)
{
    if (max_inputs == frame_counter)
    {
        max_inputs *= 2;
        recorded_inputs = realloc(recorded_inputs, sizeof(unsigned char) * max_inputs);
    }
/*
    recorded_inputs[frame_counter] =
        key_states[0] |
        key_states[1] << 1 |
        key_states[2] << 2 |
        key_states[3] << 3 |
        key_states[4] << 4 |
        key_states[5] << 5; */
}

void replay_inputs(void)
{
    unsigned char inputs;

    if (read(replay_inputs_file, &inputs, 1) <= 0)
    {
        log_err("Error reading inputs.");
        exit(-1);
    }
/*
    key_states[0] = inputs & 0x1;
    key_states[1] = (inputs >> 1) & 0x1;
    key_states[2] = (inputs >> 2) & 0x1;
    key_states[3] = (inputs >> 3) & 0x1;
    key_states[4] = (inputs >> 4) & 0x1;
    key_states[5] = (inputs >> 5) & 0x1;
*/
    if (!--max_inputs_to_replay)
    {
        close(replay_inputs_file);
        replaying = 0;
        log_err("Replaying complete.");
        send(socket_fd, &inputs, 1, 0);
    }
}
