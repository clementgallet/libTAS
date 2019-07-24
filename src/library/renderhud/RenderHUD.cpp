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

#include "RenderHUD.h"
#ifdef LIBTAS_ENABLE_HUD

#include "../logging.h"
#include "../hook.h"
#include <sstream>
#include "../global.h" // shared_config
#include <fontconfig/fontconfig.h>
// #include <X11/keysym.h>
#include "../ScreenCapture.h"

namespace libtas {

TTF_Font* RenderHUD::fg_font = nullptr;
TTF_Font* RenderHUD::bg_font = nullptr;
std::list<std::pair<std::string, TimeHolder>> RenderHUD::messages;
std::list<std::string> RenderHUD::watches;
int RenderHUD::outline_size = 1;
int RenderHUD::font_size = 20;

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

void RenderHUD::initFonts()
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
        debuglog(LCF_WINDOW, "Total matching fonts: ", fs->nfont);
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
            debuglog(LCF_WINDOW, "Picking font: ", fontFile);
            /* Initialize SDL TTF */
            if(TTF_Init() == -1) {
                debuglog(LCF_ERROR, "Couldn't init SDL TTF.");
                return;
            }

            fg_font = TTF_OpenFont(fontFile, 20);
            if (fg_font == NULL) {
                debuglog(LCF_ERROR, "Couldn't load font");
                return;
            }

            bg_font = TTF_OpenFont(fontFile, 20);
            if (bg_font == NULL) {
                debuglog(LCF_ERROR, "Couldn't load font");
                return;
            }

            TTF_SetFontOutline(bg_font, outline_size);
        }
        else {
            debuglog(LCF_WINDOW | LCF_ERROR, "We didn't find any regular TTF font !");
        }

        if (fs) FcFontSetDestroy(fs);
    }
}

std::unique_ptr<SurfaceARGB> RenderHUD::createTextSurface(const char* text, Color fg_color, Color bg_color)
{
    std::unique_ptr<SurfaceARGB> fg_surf = TTF_RenderText_Blended(fg_font, text, fg_color);
    std::unique_ptr<SurfaceARGB> bg_surf = TTF_RenderText_Blended(bg_font, text, bg_color);

    /* Blit text onto its outline. */
    bg_surf->blit(fg_surf.get(), outline_size, outline_size);

    return bg_surf;
}

void RenderHUD::locationToCoords(int location, int& x, int& y)
{
    int width, height;
    ScreenCapture::getDimensions(width, height);

    if (location & SharedConfig::OSD_LEFT)         x = 5;
    else if (location & SharedConfig::OSD_HCENTER) x = width / 2;
    else                                           x = width - 30;

    if (location & SharedConfig::OSD_TOP)          y = 5;
    else if (location & SharedConfig::OSD_VCENTER) y = height / 2;
    else                                           y = height - 30;

    /* Add an offset if other text was displayed here */
    int offset_index = 0;
    if (location & SharedConfig::OSD_HCENTER)     offset_index += 1;
    else if (location & SharedConfig::OSD_RIGHT)  offset_index += 2;
    if (location & SharedConfig::OSD_VCENTER)     offset_index += 3;
    else if (location & SharedConfig::OSD_BOTTOM) offset_index += 6;

    y += offsets[offset_index];
    if (location & SharedConfig::OSD_BOTTOM)
        offsets[offset_index] -= 25;
    else
        offsets[offset_index] += 25;
}

void RenderHUD::resetOffsets()
{
    memset(offsets, 0, 9 * sizeof(int));
}

void RenderHUD::renderFrame(uint64_t framecount)
{
    Color fg_color = {255, 255, 255, 0};
    Color bg_color = {0, 0, 0, 0};
    std::string framestr = std::to_string(framecount);
    switch (shared_config.recording) {
    case SharedConfig::RECORDING_READ:
        framestr.append("/").append(std::to_string(shared_config.movie_framecount));
        if (framecount > shared_config.movie_framecount)
            framestr.append(" (Finished)");
        break;
    case SharedConfig::RECORDING_WRITE:
    case SharedConfig::NO_RECORDING:
    default:
        break;
    }

    int x, y;
    locationToCoords(shared_config.osd_frame_location, x, y);
    renderText(framestr.c_str(), fg_color, bg_color, x, y);
}

void RenderHUD::renderNonDrawFrame(uint64_t nondraw_framecount)
{
    Color red_color = {255, 0, 0, 0};
    Color bg_color = {0, 0, 0, 0};
    std::string nondraw_framestr = std::to_string(nondraw_framecount);

    int x, y;
    locationToCoords(shared_config.osd_frame_location, x, y);
    renderText(nondraw_framestr.c_str(), red_color, bg_color, x, y);
}


void RenderHUD::renderInputs(AllInputs& ai)
{
    renderInputs(ai, {255, 255, 255, 0});
}

void RenderHUD::renderPreviewInputs(AllInputs& ai)
{
    renderInputs(ai, {160, 160, 160, 0});
}

void RenderHUD::renderInputs(AllInputs& ai, Color fg_color)
{
    std::ostringstream oss;

    /* Flags */
    if (ai.flags & (1 << SingleInput::FLAG_RESTART)) {
        oss << "[Restart] ";
    }
    for (int i=0; i<4; i++) {
        if (ai.flags & (1 << (SingleInput::FLAG_CONTROLLER1_ADDED+i))) {
            oss << "[J" << i << " added] ";
        }
        if (ai.flags & (1 << (SingleInput::FLAG_CONTROLLER1_REMOVED+i))) {
            oss << "[J" << i << " removed] ";
        }
    }

    /* Keyboard */
    if (shared_config.keyboard_support) {
        for (int i=0; i<AllInputs::MAXKEYS; i++) {
            if (ai.keyboard[i]) {
                oss << "[K " << XKeysymToString(ai.keyboard[i]) << "] ";
            }
        }
    }

    /* Mouse */
    if (shared_config.mouse_support) {
        if (ai.pointer_x != -1) {
            oss << "[M " << ai.pointer_x << ":" << ai.pointer_y << "] ";
        }
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B1))
            oss << "[M b1] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B2))
            oss << "[M b2] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B3))
            oss << "[M b3] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B4))
            oss << "[M b4] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B5))
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
        int x, y;
        locationToCoords(shared_config.osd_inputs_location, x, y);
        renderText(text.c_str(), fg_color, bg_color, x, y);
    }
}

void RenderHUD::insertMessage(const char* message)
{
    /* Get current time */
    TimeHolder current_time;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &current_time));

    messages.push_back(std::make_pair(std::string(message), current_time));
}

void RenderHUD::renderMessages()
{
    Color fg_color = {255, 255, 255, 0};
    Color bg_color = {0, 0, 0, 0};

    TimeHolder message_timeout;
    message_timeout = {2, 0};

    /* Get current time */
    TimeHolder current_time;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &current_time));

    auto iter = messages.begin();
    while (iter != messages.end()) {

        /* Check if we must remove the message */
        if ((current_time - iter->second) > message_timeout) {
            iter = messages.erase(iter);
        }
        else {
            int x, y;
            locationToCoords(shared_config.osd_messages_location, x, y);
            renderText(iter->first.c_str(), fg_color, bg_color, x, y);
            iter++;
        }
    }
}

void RenderHUD::insertWatch(std::string watch)
{
    watches.push_back(watch);
}

void RenderHUD::resetWatches()
{
    watches.clear();
}

void RenderHUD::renderWatches()
{
    Color fg_color = {255, 255, 255, 0};
    Color bg_color = {0, 0, 0, 0};

    for (auto iter = watches.begin(); iter != watches.end(); iter++) {
        int x, y;
        locationToCoords(shared_config.osd_ramwatches_location, x, y);
        renderText(iter->c_str(), fg_color, bg_color, x, y);
    }
}


}

#endif
