#include "libTAS.h"

void* SDL_handle;
void(* SDL_GL_SwapWindow_real)(void);
void*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
Uint32(* SDL_GetWindowID_real)(void*);
int (*SDL_PollEvent_real)(SDL_Event*);
Uint32 (*SDL_GetTicks_real)(void);
Uint32 (*SDL_GetWindowFlags_real)(void*);



struct timeval current_time = { 0, 0 };
unsigned long frame_counter = 0;
unsigned int speed_divisor_factor = 1;
unsigned char running = 0;
unsigned char replaying = 0;

unsigned char* recorded_inputs;
unsigned long max_inputs;
int replay_inputs_file;
unsigned long max_inputs_to_replay;

Uint8 key_states[6] = { 0 };
Uint8 key_states_old[6] = { 0 };
int socket_fd = 0;
void * gameWindow;


void __attribute__((constructor)) init(void)
{
    // SMB uses its own version of SDL.
    SDL_handle = dlopen("/home/clement/supermeatboy/amd64/libSDL2-2.0.so.0",
                        RTLD_LAZY);
    if (!SDL_handle)
    {
        log_err("Could not load SDL.");
        exit(-1);
    }

    *(void**)&SDL_GL_SwapWindow_real = dlsym(SDL_handle, "SDL_GL_SwapWindow");
    if (!SDL_GL_SwapWindow_real)
    {
        log_err("Could not import symbol SDL_GL_SwapWindow.");
        exit(-1);
    }

    *(void**)&SDL_CreateWindow_real = dlsym(SDL_handle, "SDL_CreateWindow");
    if (!SDL_CreateWindow_real)
    {
        log_err("Could not import symbol SDL_CreateWindow.");
        exit(-1);
    }

    *(void**)&SDL_GetWindowID_real = dlsym(SDL_handle, "SDL_GetWindowID");
    if (!SDL_GetWindowID_real)
    {
        log_err("Could not import symbol SDL_GetWindowID.");
        exit(-1);
    }

    *(void**)&SDL_GetWindowFlags_real = dlsym(SDL_handle, "SDL_GetWindowFlags");
    if (!SDL_GetWindowFlags_real)
    {
        log_err("Could not import symbol SDL_GetWindowFlags.");
        exit(-1);
    }

    *(void**)&SDL_PollEvent_real = dlsym(SDL_handle, "SDL_PollEvent");
    if (!SDL_PollEvent_real)
    {
        log_err("Could not import symbol SDL_PollEvent.");
        exit(-1);
    }

    *(void**)&SDL_GetTicks_real = dlsym(SDL_handle, "SDL_GetTicks");
    if (!SDL_GetTicks_real)
    {
        log_err("Could not import symbol SDL_GetTicks.");
        exit(-1);
    }

    if (!unlink(SOCKET_FILENAME))
        log_err("Removed stall socket.");

    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    const int tmp_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (bind(tmp_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        log_err("Couldn’t bind client socket.");
        exit(-1);
    }

    if (listen(tmp_fd, 1))
    {
        log_err("Couldn’t listen on client socket.");
        exit(1);
    }

    log_err("Loading complete, awaiting client connection...");

    if ((socket_fd = accept(tmp_fd, NULL, NULL)) < 0)
    {
        log_err("Couldn’t accept client connection.");
        exit(1);
    }

    log_err("Client connected.");

    close(tmp_fd);
    unlink(SOCKET_FILENAME);

    recorded_inputs = malloc(sizeof(unsigned char) * 8192);
    max_inputs = 8192;

    proceed_commands();
}

void __attribute__((destructor)) term(void)
{
    dlclose(SDL_handle);
    close(socket_fd);

    log_err("Exiting.");
}

time_t time(time_t* t)
{
    if (t)
        *t = current_time.tv_sec;
    return current_time.tv_sec;
}

int gettimeofday(struct timeval* tv, void* tz)
{
    *tv = current_time;
    return 0;
}

void SDL_GL_SwapWindow(void)
{
    SDL_GL_SwapWindow_real();

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

    record_inputs();
    ++frame_counter;

    if (replaying)
    {replay_inputs();
        return;
    }

    if (running && speed_divisor_factor > 1)
        usleep(1000000 * (speed_divisor_factor - 1) / 60);

    proceed_commands();
}

void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    gameWindow = SDL_CreateWindow_real(title, x, y, w, h, flags); // Save the game window
    return gameWindow;
}

Uint32 SDL_GetWindowFlags(void* window){
    printf("GetWindowFlags\n");
    return SDL_GetWindowFlags_real(window);
}


const Uint8* SDL_GetKeyboardState(int* numkeys)
{
    printf("GetKeyboardState\n");
    int i;
    for (i=0; i<6; i++) {
        keyboard_state[used_scankeys[i]] = key_states[i];
    }
    return keyboard_state;
}

int SDL_PollEvent(SDL_Event *event)
{
    //return SDL_PollEvent_real(event);
    SDL_Event myEvent;
    int isone = SDL_PollEvent_real(&myEvent);
    while (isone == 1){
        if ((myEvent.type == SDL_KEYDOWN) || (myEvent.type == SDL_KEYUP)){
            if (myEvent.type == SDL_KEYDOWN)
                printf("KEYDOWN\n");
            if (myEvent.type == SDL_KEYUP)
                printf("KEYUP\n");
            printf("windowID: %d\n", myEvent.key.windowID);
            printf("timestamp: %d\n", myEvent.key.timestamp);
            printf("sym key: %d\n", myEvent.key.keysym.sym);
            printf("scan key: %d\n", myEvent.key.keysym.scancode);
        }
        isone = SDL_PollEvent_real(&myEvent);
    }

//    printf("PollEvent\n");
    int i;
    for (i=0; i<6; i++) {
        if (key_states[i] != key_states_old[i]) {
            printf("Pressed key n %d\n",i);
            if (key_states[i]) {
                event->type = SDL_KEYDOWN;
                event->key.state = SDL_PRESSED;
            }
            else {
                event->type = SDL_KEYUP;
                event->key.state = SDL_RELEASED;
            }
            event->key.windowID = SDL_GetWindowID_real(gameWindow);
            event->key.timestamp = SDL_GetTicks_real() - 1;
            printf("myWindowID: %d\n", event->key.windowID);
            SDL_Keysym keysym;
            keysym.sym = used_symkeys[i];
            printf("mySym: %d\n", used_symkeys[i]);
            keysym.scancode = used_scankeys[i];
            printf("myScan: %d\n", used_scankeys[i]);
            event->key.keysym = keysym;
            key_states_old[i] = key_states[i];
            return 1;
        }
    }
    return 0;
}

void proceed_commands(void)
{
    while (1)
    {
        unsigned int command;

        if (recv(socket_fd, &command, sizeof(unsigned int), MSG_DONTWAIT * running) < 0)
            return;
        else
        {
            char filename_buffer[1024];
            switch (command)
            {
            case 0:
                send(socket_fd, &frame_counter, sizeof(unsigned long), 0);
                break;

            case 1:
                key_states[0] = !key_states[0];
                break;

            case 2:
                key_states[1] = !key_states[1];
                break;

            case 3:
                key_states[2] = !key_states[2];
                break;

            case 4:
                key_states[3] = !key_states[3];
                break;

            case 5:
                key_states[4] = !key_states[4];
                break;

            case 6:
                key_states[5] = !key_states[5];
                break;

            case 7:
                running = !running;
                break;

            case 8:
                return;

            case 9:
                recv(socket_fd, &speed_divisor_factor, sizeof(unsigned int), 0);
                break;

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

            default:
                log_err("Unknown command recieved.");
            }
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

    recorded_inputs[frame_counter] =
        key_states[0] |
        key_states[1] << 1 |
        key_states[2] << 2 |
        key_states[3] << 3 |
        key_states[4] << 4 |
        key_states[5] << 5;
}

void replay_inputs(void)
{
    unsigned char inputs;

    if (read(replay_inputs_file, &inputs, 1) <= 0)
    {
        log_err("Error reading inputs.");
        exit(-1);
    }

    key_states[0] = inputs & 0x1;
    key_states[1] = (inputs >> 1) & 0x1;
    key_states[2] = (inputs >> 2) & 0x1;
    key_states[3] = (inputs >> 3) & 0x1;
    key_states[4] = (inputs >> 4) & 0x1;
    key_states[5] = (inputs >> 5) & 0x1;

    if (!--max_inputs_to_replay)
    {
        close(replay_inputs_file);
        replaying = 0;
        log_err("Replaying complete.");
        send(socket_fd, &inputs, 1, 0);
    }
}
