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

#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD.h"
#include "../logging.h"
#include "../hook.h"
#include <sstream>
#include <X11/Xlib.h> // For the KeySym type
#include "../../shared/SharedConfig.h"
#include <fontconfig/fontconfig.h>

RenderHUD::RenderHUD()
{
    outline_size = 1;
    font_size = 20;
}

RenderHUD::~RenderHUD()
{
    if (fg_font)
        TTF_CloseFont(fg_font);
    if (bg_font)
        TTF_CloseFont(bg_font);
    if (TTF_WasInit())
        TTF_Quit();
}

void RenderHUD::init()
{
    /* Code taken from http://stackoverflow.com/questions/10542832 */
    FcConfig* config = FcInitLoadConfigAndFonts();
    FcPattern* pat = FcPatternCreate();
    FcPatternAddString(pat, FC_STYLE, reinterpret_cast<const FcChar8*>("Regular"));
    FcObjectSet* os = FcObjectSetBuild (FC_FAMILY, FC_FILE, (char *) 0);
    FcFontSet* fs = FcFontList(config, pat, os);
    // debuglog(LCF_WINDOW, "Total matching fonts: ", fs->nfont);
    for (int i=0; fs && i < fs->nfont; ++i) {
        FcPattern* font = fs->fonts[i];
        FcChar8 *file, *family;
        if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
            FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch) {
            if (FcStrStr(file, reinterpret_cast<const FcChar8*>(".ttf"))) {
                debuglog(LCF_WINDOW, "Picking font: ", file, " (family ", family, ")");
                init(reinterpret_cast<char*>(file));
                if (fs) FcFontSetDestroy(fs);
                return;
            }
        }
    }

    debuglog(LCF_WINDOW | LCF_ERROR, "We didn't find any regular TTF font !");
    if (fs) FcFontSetDestroy(fs);
}

void RenderHUD::init(const char* path)
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

void RenderHUD::size(int& width, int& height) {}

std::unique_ptr<SurfaceARGB> RenderHUD::createTextSurface(const char* text, Color fg_color, Color bg_color)
{
    std::unique_ptr<SurfaceARGB> fg_surf = TTF_RenderText_Blended(fg_font, text, fg_color);
    std::unique_ptr<SurfaceARGB> bg_surf = TTF_RenderText_Blended(bg_font, text, bg_color);

    /* Blit text onto its outline. */
    bg_surf->blit(fg_surf.get(), outline_size, outline_size);

    return bg_surf;
}

void RenderHUD::renderFrame(int framecount)
{
    Color fg_color = {255, 255, 255, 0};
    Color bg_color = {0, 0, 0, 0};
    std::string text = std::to_string(framecount);
    renderText(text.c_str(), fg_color, bg_color, 2, 2);
}

void RenderHUD::renderInputs(AllInputs& ai)
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
    for (int i=0; i<shared_config.numControllers; i++) {
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
    Color fg_color = {255, 255, 255, 0};
    Color bg_color = {0, 0, 0, 0};
    std::string text = oss.str();
    if (!text.empty()) {
        /* Get size of the window */
        int width, height;
        size(width, height);
        renderText(text.c_str(), fg_color, bg_color, 2, height-25);
    }
}

#endif
