// adapted from GLFW, glfw.sf.net
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <zbuffer.h>
#include <SDL.h>

int main(int argc, char **argv) {
    // initialize SDL video:
    int winSizeX=640;
    int winSizeY=480;
    if(SDL_Init(SDL_INIT_VIDEO)<0) {
        fprintf(stderr,"ERROR: cannot initialize SDL video.\n");
        return 1;
    }
    SDL_Surface* screen = NULL;
    if((screen=SDL_SetVideoMode( winSizeX, winSizeY, 32, SDL_SWSURFACE)) == 0 ) {
        fprintf(stderr,"ERROR: Video mode set failed.\n");
        return 1;
    }

    // initialize TinyGL:
    unsigned int pitch;
    int	mode;
    switch( screen->format->BitsPerPixel ) {
    case  8:
        fprintf(stderr,"ERROR: Palettes are currently not supported.\n");
        return 1;
    case 16:
        pitch = screen->pitch;
        mode = ZB_MODE_5R6G5B;
        break;
    case 24:
        pitch = ( screen->pitch * 2 ) / 3;
        mode = ZB_MODE_RGB24;
        break;
    case 32:
        pitch = screen->pitch / 2;
        mode = ZB_MODE_RGBA;
        break;
    default:
        return 1;
        break;
    }
    ZBuffer *frameBuffer = ZB_open( winSizeX, winSizeY, mode, 0, 0, 0, 0);
    glInit( frameBuffer );

    // set viewport
    glViewport( 0, 0, winSizeX, winSizeY);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    // main loop:
    int frames=0;
    int x=0;
    double  t, t0, fps;
    char    titlestr[ 200 ];
    int running = GL_TRUE;
    t0 = (double)SDL_GetTicks()/1000.0;
    while( running ) {
        // calculate and display FPS (frames per second):
        t = (double)SDL_GetTicks()/1000.0;
        if( (t-t0) > 1.0 || frames == 0 ) {
            fps = (double)frames / (t-t0);
            sprintf( titlestr, "Spinning Triangle (%.1f FPS)", fps );
            SDL_WM_SetCaption(titlestr,0);
            t0 = t;
            frames = 0;
        }
        ++frames;

        // Clear color buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Select and setup the projection matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPerspective( 65.0f, (GLfloat)winSizeX/(GLfloat)winSizeY, 1.0f, 100.0f );

        // Select and setup the modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        glRotatef(-90, 1,0,0);
        glTranslatef(0,0,-1.0f);

        // Draw a rotating colorful triangle
        glTranslatef( 0.0f, 14.0f, 0.0f );
        glRotatef( 0.3*(GLfloat)x + (GLfloat)t*100.0f, 0.0f, 0.0f, 1.0f );
        glBegin( GL_TRIANGLES );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( -5.0f, 0.0f, -4.0f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( 5.0f, 0.0f, -4.0f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 0.0f, 0.0f, 6.0f );
        glEnd();

        // swap buffers:
        if ( SDL_MUSTLOCK(screen) && (SDL_LockSurface(screen)<0) ) {
            fprintf(stderr, "SDL ERROR: Can't lock screen: %s\n", SDL_GetError());
            return 1;
        }
        ZB_copyFrameBuffer(frameBuffer, screen->pixels, pitch);
        if ( SDL_MUSTLOCK(screen) ) SDL_UnlockSurface(screen);
        SDL_Flip(screen);

        // check if the ESC key was pressed or the window was closed:
        SDL_Event evt;
        while( SDL_PollEvent( &evt ) ) switch(evt.type) {
        case SDL_KEYDOWN:
            if(evt.key.keysym.sym==SDLK_ESCAPE)
                running=0;
            break;
        case SDL_QUIT:
            running=0;
            break;
        }
    }
    // cleanup:
    ZB_close(frameBuffer);
    if(SDL_WasInit(SDL_INIT_VIDEO))
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return 0;
}
