#include "opengl.h"

/* Render a text on top of the game window 
 * Taken from http://stackoverflow.com/questions/5289447/using-sdl-ttf-with-opengl
 */
void RenderText(TTF_Font *font, const char* message, int sw, int sh, SDL_Color color, int x, int y) {
  glMatrixMode_real(GL_PROJECTION);
  glPushMatrix_real();
  glLoadIdentity_real();
  //glOrtho_real(0, sw, 0, sh, -1, 1); // m_Width and m_Height is the resolution of window
  glOrtho_real(0, sw, sh, 0, -1, 1); // m_Width and m_Height is the resolution of window

  glMatrixMode_real(GL_MODELVIEW);
  //glPushMatrix_real();
  glLoadIdentity_real();

  //glDisable_real(/*GL_DEPTH_TEST*/ 0x0B71);
  //glEnable_real(/*GL_TEXTURE_2D*/ 0x0DE0);
  //glEnable_real(/*GL_BLEND*/ 0x0BE2);
  //glBlendFunc_real(/*GL_SRC_ALPHA*/ 0x0302, /*GL_ONE_MINUS_SRC_ALPHA*/ 0x0303);

  /* Get previous blind texture */
  GLint last_tex = 0;
  glGetIntegerv_real(GL_TEXTURE_BINDING_2D, &last_tex);

  /* Create text texture */
//#if 0
  GLuint texture;
  glGenTextures_real(1, &texture);
  glBindTexture_real(GL_TEXTURE_2D, texture);

  SDL_Surface * sFont = TTF_RenderText_Blended(font, message, color);

  glTexParameteri_real(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri_real(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D_real(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, sFont->pixels);

  glBegin_real(GL_QUADS);
  {
    glTexCoord2f_real(0,0); glVertex2f_real(x, y);
    glTexCoord2f_real(1,0); glVertex2f_real(x + sFont->w, y);
    glTexCoord2f_real(1,1); glVertex2f_real(x + sFont->w, y + sFont->h);
    glTexCoord2f_real(0,1); glVertex2f_real(x, y + sFont->h);
  }
  glEnd_real();

  if (last_tex != 0) {
    glBindTexture_real(GL_TEXTURE_2D, last_tex);
  }

//#endif

  /*
glBegin_real(GL_QUADS);      // try drawing a quad
    glVertex2f_real(0.0, 0.0);
    glVertex2f_real(10.0, 0.0);
    glVertex2f_real(10.0, 10.0);
    glVertex2f_real(0.0, 10.0);
glEnd_real();
*/

  //glDisable_real(GL_BLEND);
  //glDisable_real(GL_TEXTURE_2D);
  //glEnable_real(GL_DEPTH_TEST);

  glMatrixMode_real(GL_PROJECTION);
  glPopMatrix_real();
  glMatrixMode_real(GL_MODELVIEW);
  //glPopMatrix_real();

  //glDeleteTextures_real(1, &texture);
  //SDL_FreeSurface_real(sFont);
}

