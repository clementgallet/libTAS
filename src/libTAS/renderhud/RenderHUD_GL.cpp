/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

//#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD_GL.h"
#include "../logging.h"
#include "../hook.h"

void (*glGenTextures_real)(int n, unsigned int* tex);
void (*glBindTexture_real)(int target, unsigned int tex);
void (*glTexImage2D_real)(int, int, int, int, int, int, int, int, const void*);
void (*glBegin_real)( int mode );
void (*glEnd_real)( void );
void (*glVertex2f_real)( float x, float y );
void (*glTexCoord2f_real)( float s, float t );
void (*glDeleteTextures_real)( int n, const unsigned int *textures);
void (*glEnable_real)( int cap );
void (*glDisable_real)( int cap );
void (*glVertexPointer_real)(int size, int type, int stride, const void* pointer);
void (*glDrawArrays_real)( int mode, int first, int count);

void (*glMatrixMode_real)(int mode);
void (*glPushMatrix_real)(void);
void (*glPopMatrix_real)(void);
void (*glLoadIdentity_real)(void);
void (*glOrtho_real)(double left, double right, double bottom, double top, double near, double far);
void (*glBlendFunc_real)(int sfactor, int dfactor);
void (*glTexParameteri_real)(int target, int pname, int param);
void (*glGetIntegerv_real)( int pname, GLint* data);
void (*glGetBooleanv_real)( int pname, GLboolean* data);
void (*glUseProgram_real)(unsigned int program);

RenderHUD_GL::~RenderHUD_GL()
{
    /* We should destroy our created GL texture here,
     * but they are not handled yet.
     */
}

void RenderHUD_GL::init()
{
    RenderHUD::init();

    LINK_SUFFIX(glGenTextures, "libGL");
    LINK_SUFFIX(glBindTexture, "libGL");
    LINK_SUFFIX(glTexImage2D, "libGL");
    LINK_SUFFIX(glBegin, "libGL");
    LINK_SUFFIX(glEnd, "libGL");
    LINK_SUFFIX(glVertex2f, "libGL");
    LINK_SUFFIX(glTexCoord2f, "libGL");
    LINK_SUFFIX(glDeleteTextures, "libGL");
    LINK_SUFFIX(glEnable, "libGL");
    LINK_SUFFIX(glDisable, "libGL");
    LINK_SUFFIX(glVertexPointer, "libGL");
    LINK_SUFFIX(glDrawArrays, "libGL");
    LINK_SUFFIX(glMatrixMode, "libGL");
    LINK_SUFFIX(glPushMatrix, "libGL");
    LINK_SUFFIX(glPopMatrix, "libGL");
    LINK_SUFFIX(glLoadIdentity, "libGL");
    LINK_SUFFIX(glOrtho, "libGL");
    LINK_SUFFIX(glBlendFunc, "libGL");
    LINK_SUFFIX(glTexParameteri, "libGL");
    LINK_SUFFIX(glGetIntegerv, "libGL");
    LINK_SUFFIX(glGetBooleanv, "libGL");
    LINK_SUFFIX(glUseProgram, "libGL");
}

void RenderHUD_GL::enterRender(void)
{
    glGetIntegerv_real(GL_CURRENT_PROGRAM, &oldProgram);
    glUseProgram_real(0);

    GLint viewport[4];
    glGetIntegerv_real(GL_VIEWPORT, viewport);

    glMatrixMode_real(GL_PROJECTION);
    glPushMatrix_real();
    glLoadIdentity_real();
    //glOrtho_real(0, sw, sh, 0, -1, 1);
    glOrtho_real(0, viewport[2], viewport[3], 0, -1, 1);

    glMatrixMode_real(GL_MODELVIEW);
    glPushMatrix_real();
    glLoadIdentity_real();

    glDisable_real(GL_DEPTH_TEST);

    glGetBooleanv_real(GL_TEXTURE_2D, &oldTex2DEnabled);

    glEnable_real(GL_TEXTURE_2D);

    glGetBooleanv_real(GL_BLEND, &oldBlendEnabled);

    glGetIntegerv_real(GL_BLEND_SRC_ALPHA, &oldBlendSrc);
    glGetIntegerv_real(GL_BLEND_DST_ALPHA, &oldBlendDst);

    glEnable_real(GL_BLEND);
    glBlendFunc_real(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Get previous blind texture */
    glGetIntegerv_real(GL_TEXTURE_BINDING_2D, &oldTex);
}

void RenderHUD_GL::renderText(const char* text, SDL_Color fg_color, SDL_Color bg_color, int x, int y)
{
    static int inited = 0;
    if (inited == 0) {
        init();
        inited = 1;
    }

    enterRender();

    /* TODO: Manage GL textures!!! */
    static GLuint texture = 0;
    if (texture == 0) {
        glGenTextures_real(1, &texture);
    }
    glBindTexture_real(GL_TEXTURE_2D, texture);

    SDL_Surface* surf = createTextSurface(text, fg_color, bg_color);

    glTexParameteri_real(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri_real(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* TODO: We should lock the surface I guess */
    glTexImage2D_real(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surf->pixels);

    glBegin_real(GL_QUADS);
    {
        glTexCoord2f_real(0,0); glVertex2f_real(x, y);
        glTexCoord2f_real(1,0); glVertex2f_real(x + surf->w, y);
        glTexCoord2f_real(1,1); glVertex2f_real(x + surf->w, y + surf->h);
        glTexCoord2f_real(0,1); glVertex2f_real(x, y + surf->h);
    }
    glEnd_real();

    destroyTextSurface();

    exitRender();
}

void RenderHUD_GL::exitRender(void)
{
    if (oldTex != 0) {
        glBindTexture_real(GL_TEXTURE_2D, oldTex);
    }

    if (! oldBlendEnabled )
        glDisable_real(GL_BLEND);
    glBlendFunc_real(oldBlendSrc, oldBlendDst);

    if (! oldTex2DEnabled )
        glDisable_real(GL_TEXTURE_2D);

    glMatrixMode_real(GL_PROJECTION);
    glPopMatrix_real();
    glMatrixMode_real(GL_MODELVIEW);
    glPopMatrix_real();

    glUseProgram_real(oldProgram);
} 

