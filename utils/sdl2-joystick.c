/* This code allow to display the GUID of a plugged-in controller
 * Can be compiled with: gcc -o sdl2-joystick sdl2-joystick.c `pkg-config --libs --cflags sdl2`
 * Source taken from http://askubuntu.com/a/368711 
 */

#include <SDL.h>

int main()
{
  SDL_Init(SDL_INIT_JOYSTICK);
  atexit(SDL_Quit);

  int num_joysticks = SDL_NumJoysticks();
  int i;
  for(i = 0; i < num_joysticks; ++i)
  {
    SDL_Joystick* js = SDL_JoystickOpen(i);
    if (js)
    {
      SDL_JoystickGUID guid = SDL_JoystickGetGUID(js);
      char guid_str[1024];
      SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
      const char* name = SDL_JoystickName(js);

      int num_axes = SDL_JoystickNumAxes(js);
      int num_buttons = SDL_JoystickNumButtons(js);
      int num_hats = SDL_JoystickNumHats(js);
      int num_balls = SDL_JoystickNumBalls(js);

      printf("%s \"%s\" axes:%d buttons:%d hats:%d balls:%d\n", 
             guid_str, name,
             num_axes, num_buttons, num_hats, num_balls);
      printf("Raw guid is");
      for (int g=0; g<16; g++)
        printf(" %d", guid.data[g]);
      printf("\n");

      SDL_JoystickClose(js);
    }
  }

  return 0;
}

