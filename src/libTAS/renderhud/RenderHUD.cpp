/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD.h"
#include "../logging.h"
#include "../hook.h"
#include <sstream>
#include "../global.h" // shared_config
#include <fontconfig/fontconfig.h>
#include <X11/keysym.h>
#include "../ScreenCapture.h"

namespace libtas {

TTF_Font* RenderHUD::fg_font = nullptr;
TTF_Font* RenderHUD::bg_font = nullptr;

RenderHUD::RenderHUD()
{
    outline_size = 1;
    font_size = 20;

    if (!fg_font || !bg_font) {
        /* Find an installed regular font in the system using fontconfig */
        /* Code taken from http://stackoverflow.com/questions/10542832 */
        FcConfig* config = FcInitLoadConfigAndFonts();
        FcPattern* pat = FcPatternCreate();
        FcPatternAddString(pat, FC_STYLE, reinterpret_cast<const FcChar8*>("Regular"));
        FcPatternAddString(pat, FC_LANG, reinterpret_cast<const FcChar8*>("en-US"));
        FcObjectSet* os = FcObjectSetBuild (FC_FAMILY, FC_FILE, (char *) 0);
        FcFontSet* fs = FcFontList(config, pat, os);
        debuglog(LCF_WINDOW, "Total matching fonts: ", fs->nfont);
        for (int i=0; fs && i < fs->nfont; ++i) {
            FcPattern* font = fs->fonts[i];
            FcChar8 *file, *family;
            if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
                FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch) {
                if (FcStrStr(file, reinterpret_cast<const FcChar8*>(".ttf"))) {
                    debuglog(LCF_WINDOW, "Picking font: ", file, " (family ", family, ")");
                    initFonts(reinterpret_cast<char*>(file));
                    if (fs) FcFontSetDestroy(fs);
                    return;
                }
            }
        }

        debuglog(LCF_WINDOW | LCF_ERROR, "We didn't find any regular TTF font !");
        if (fs) FcFontSetDestroy(fs);
    }
}

RenderHUD::~RenderHUD()
{
    if (fg_font) {
        TTF_CloseFont(fg_font);
        fg_font = nullptr;
    }
    if (bg_font) {
        TTF_CloseFont(bg_font);
        bg_font = nullptr;
    }
    if (TTF_WasInit())
        TTF_Quit();
}

void RenderHUD::initFonts(const char* path)
{
    debuglog(LCF_WINDOW, "Try opening font ", path);

    /* Initialize SDL TTF */
    if(TTF_Init() == -1) {
        debuglog(LCF_ERROR, "Couldn't init SDL TTF.");
        return;
    }

    fg_font = TTF_OpenFont(path, 20);
    if (fg_font == NULL) {
        debuglog(LCF_ERROR, "Couldn't load font");
        return;
    }

    bg_font = TTF_OpenFont(path, 20);
    if (bg_font == NULL) {
        debuglog(LCF_ERROR, "Couldn't load font");
        return;
    }

    TTF_SetFontOutline(bg_font, outline_size);
}

std::unique_ptr<SurfaceARGB> RenderHUD::createTextSurface(const char* text, Color fg_color, Color bg_color)
{
    std::unique_ptr<SurfaceARGB> fg_surf = TTF_RenderText_Blended(fg_font, text, fg_color);
    std::unique_ptr<SurfaceARGB> bg_surf = TTF_RenderText_Blended(bg_font, text, bg_color);

    /* Blit text onto its outline. */
    bg_surf->blit(fg_surf.get(), outline_size, outline_size);

    return bg_surf;
}

void RenderHUD::renderFrame(unsigned int framecount)
{
    Color fg_color = {255, 255, 255, 0};
    Color bg_color = {0, 0, 0, 0};
    std::string framestr = std::to_string(framecount);
    switch (shared_config.recording) {
    case SharedConfig::RECORDING_READ:
        framestr.append("/").append(std::to_string(shared_config.movie_framecount));
        break;
    case SharedConfig::RECORDING_WRITE:
    case SharedConfig::NO_RECORDING:
    default:
        break;
    }

    // int width, height;
    // ScreenCapture::getDimensions(width, height);
    renderText(framestr.c_str(), fg_color, bg_color, 2, 2);
}

void RenderHUD::renderNonDrawFrame(unsigned int nondraw_framecount)
{
    Color red_color = {255, 0, 0, 0};
    Color bg_color = {0, 0, 0, 0};
    std::string nondraw_framestr = std::to_string(nondraw_framecount);

    // int width, height;
    // ScreenCapture::getDimensions(width, height);
    renderText(nondraw_framestr.c_str(), red_color, bg_color, 2, 30);
}


void RenderHUD::renderInputs(AllInputs& ai)
{
    renderInputs(ai, -50, {255, 255, 255, 0});
}

void RenderHUD::renderPreviewInputs(AllInputs& ai)
{
    renderInputs(ai, -25, {160, 160, 160, 0});
}

void RenderHUD::renderInputs(AllInputs& ai, int off_y, Color fg_color)
{
    std::ostringstream oss;

    /* Keyboard */
    if (shared_config.keyboard_support) {
        for (int i=0; i<AllInputs::MAXKEYS; i++) {
            if (ai.keyboard[i] != XK_VoidSymbol) {
                oss << "[K " << XKeysymToString(ai.keyboard[i]) << "] ";
            }
        }
    }

    /* Mouse */
    if (shared_config.mouse_support) {
        if (ai.pointer_x != -1) {
            oss << "[M " << ai.pointer_x << ":" << ai.pointer_y << "] ";
        }
        if (ai.pointer_mask & Button1Mask)
            oss << "[M b1] ";
        if (ai.pointer_mask & Button2Mask)
            oss << "[M b2] ";
        if (ai.pointer_mask & Button3Mask)
            oss << "[M b3] ";
        if (ai.pointer_mask & Button4Mask)
            oss << "[M b4] ";
        if (ai.pointer_mask & Button5Mask)
            oss << "[M b5] ";
    }

    /* Joystick */
    for (int i=0; i<shared_config.nb_controllers; i++) {
        for (int j=0; j<AllInputs::MAXAXES; j++) {
            if (ai.controller_axes[i][j] != 0)
                oss << "[J" << i << " a" << j << ":" << ai.controller_axes[i][j] << "] ";
        }

        for (int j=0; j<16; j++) {
            if (ai.controller_buttons[i] & (1 << j))
                oss << "[J" << i << " b" << j << "] ";
        }
    }

    /* Render */
    Color bg_color = {0, 0, 0, 0};
    std::string text = oss.str();
    if (!text.empty()) {
        /* Get size of the window */
        int width, height;
        ScreenCapture::getDimensions(width, height);
        renderText(text.c_str(), fg_color, bg_color, 2, height + off_y);
    }
}

}

#endif
