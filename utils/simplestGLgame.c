// Compile with `gcc simplestGLgame.c -lSDL2 -lGL -o simplestGLgame`

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <unistd.h>

int main()
{
    usleep(50*1000);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_Window* window = SDL_CreateWindow("Title",SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640,
            480,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    glViewport( 0, 0, 640, 480 );
    SDL_GL_SetSwapInterval(1);

    int run = 1;
    int frame = 0;

    while (run) {
        glClearColor( ((float)(frame%256))/256.0f, ((float)(frame%256))/256.0f, ((float)(frame%256))/256.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        SDL_GL_SwapWindow(window);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    run=0;
                    break;
            }
        }

        frame++;
    }
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);

    SDL_Quit();

}
