/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "RenderHUD_Base_Linux.h"

#include "../logging.h"
#include "../hook.h"
#include "../global.h" // Global::shared_config
#include <fontconfig/fontconfig.h>
#include "SurfaceARGB.h"
// #include <X11/keysym.h>
#include "../ScreenCapture.h"
#include "../GlobalState.h"

namespace libtas {

TTF_Font* RenderHUD_Base_Linux::fg_font = nullptr;
TTF_Font* RenderHUD_Base_Linux::bg_font = nullptr;
int RenderHUD_Base_Linux::outline_size = 1;
int RenderHUD_Base_Linux::font_size = 20;

RenderHUD_Base_Linux::~RenderHUD_Base_Linux()
{
    GlobalNative gn;
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

void RenderHUD_Base_Linux::initFonts()
{
    if (!fg_font || !bg_font) {
        /* Find an installed regular font in the system using fontconfig */
        /* Code taken from http://stackoverflow.com/questions/10542832 */
        GlobalNative gn;
        FcConfig* config = FcInitLoadConfigAndFonts();
        FcPattern* pat = FcPatternCreate();
        FcPatternAddString(pat, FC_STYLE, reinterpret_cast<const FcChar8*>("Regular"));
        FcPatternAddString(pat, FC_LANG, reinterpret_cast<const FcChar8*>("en-US"));
        FcObjectSet* os = FcObjectSetBuild (FC_FAMILY, FC_FILE, (char *) 0);
        FcFontSet* fs = FcFontList(config, pat, os);
        debuglogstdio(LCF_WINDOW, "Total matching fonts: %d", fs->nfont);
        char* fontFile = nullptr;
        for (int i=0; fs && i < fs->nfont; ++i) {
            FcPattern* font = fs->fonts[i];
            FcChar8 *file, *family;
            if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
                FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch) {

                /* In priority, choose one of the following font */
                if (FcStrStr(file, reinterpret_cast<const FcChar8*>("FreeSans.ttf")) ||
                    FcStrStr(file, reinterpret_cast<const FcChar8*>("Gentium-R.ttf")) ||
                    FcStrStr(file, reinterpret_cast<const FcChar8*>("LiberationSans-Regular.ttf")) ||
                    FcStrStr(file, reinterpret_cast<const FcChar8*>("Ubuntu-R.ttf"))) {

                    fontFile = reinterpret_cast<char*>(file);
                    break;
                }

                if (FcStrStr(file, reinterpret_cast<const FcChar8*>(".ttf"))) {
                    // debuglog(LCF_WINDOW, "   Font: ", file, " (family ", family, ")");

                    /* Otherwise, pick this font */
                    if (!fontFile) {
                        fontFile = reinterpret_cast<char*>(file);
                    }
                }
            }
        }

        if (fontFile) {
            debuglogstdio(LCF_WINDOW, "Picking font: %s", fontFile);
            /* Initialize SDL TTF */
            if(TTF_Init() == -1) {
                debuglogstdio(LCF_ERROR, "Couldn't init SDL TTF.");
                return;
            }

            fg_font = TTF_OpenFont(fontFile, font_size);
            if (fg_font == NULL) {
                debuglogstdio(LCF_ERROR, "Couldn't load font");
                return;
            }

            bg_font = TTF_OpenFont(fontFile, font_size);
            if (bg_font == NULL) {
                debuglogstdio(LCF_ERROR, "Couldn't load font");
                return;
            }

            TTF_SetFontOutline(bg_font, outline_size);
        }
        else {
            debuglogstdio(LCF_WINDOW | LCF_ERROR, "We didn't find any regular TTF font !");
            Global::shared_config.osd = 0;
        }

        if (fs) FcFontSetDestroy(fs);
    }
}

void RenderHUD_Base_Linux::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    if (!fg_font || !bg_font) {
        initFonts();
        if (!fg_font || !bg_font)
            return;
    }

    std::unique_ptr<SurfaceARGB> fg_surf = TTF_RenderText_Blended(fg_font, text, fg_color);
    std::unique_ptr<SurfaceARGB> bg_surf = TTF_RenderText_Blended(bg_font, text, bg_color);

    if (bg_surf) {
        /* Blit text onto its outline. */
        bg_surf->blit(fg_surf.get(), outline_size, outline_size);

        /* Change the coords so that the text fills on screen */
        int width, height;
        ScreenCapture::getDimensions(width, height);

        x = (x + bg_surf->w + 5) > width ? (width - bg_surf->w - 5) : x;    
        y = (y + bg_surf->h + 5) > height ? (height - bg_surf->h - 5) : y;

        renderSurface(std::move(bg_surf), x, y);
    }
    else {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "Could not generate a text surface!");
    }
}

}
