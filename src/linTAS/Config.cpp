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

#include "Config.h"
#include <iostream>
//#include <sys/stat.h>

Config::~Config() {
    save();
}

void Config::save() {
    if (!prefs) return;

    prefs->set("gameargs", gameargs.c_str());
    prefs->set("moviefile", moviefile.c_str());
    prefs->set("dumpfile", dumpfile.c_str());
    prefs->set("libdir", libdir.c_str());
    prefs->set("rundir", rundir.c_str());
    prefs->set("opengl_soft", opengl_soft);

    Fl_Preferences prefs_km(*prefs, "keymapping");

    Fl_Preferences prefs_km_hotkeys(prefs_km, "hotkeys");

    prefs_km_hotkeys.deleteAllEntries();

    char hdata[sizeof(HotKeyType)];
    for (auto& hmap : km.hotkey_mapping) {
        hmap.second.pack(hdata);
        std::string key_str = std::to_string(hmap.first);
        prefs_km_hotkeys.set(key_str.c_str(), static_cast<void*>(hdata), sizeof(HotKeyType));
    }

    Fl_Preferences prefs_km_inputs(prefs_km, "inputs");

    prefs_km_inputs.deleteAllEntries();

    char idata[sizeof(InputType) + sizeof(int)];
    for (auto& imap : km.input_mapping) {
        imap.second.pack(idata);
        std::string key_str = std::to_string(imap.first);
        prefs_km_inputs.set(key_str.c_str(), static_cast<void*>(idata), sizeof(InputType) + sizeof(int));
    }

    Fl_Preferences prefs_shared(*prefs, "shared");

    prefs_shared.set("speed_divisor", sc.speed_divisor);
    prefs_shared.set("logging_status", static_cast<int>(sc.logging_status));
    prefs_shared.set("includeFlags", static_cast<int>(sc.includeFlags));
    prefs_shared.set("excludeFlags", static_cast<int>(sc.excludeFlags));
    prefs_shared.set("framerate", static_cast<int>(sc.framerate));
    prefs_shared.set("keyboard_support", static_cast<int>(sc.keyboard_support));
    prefs_shared.set("mouse_support", static_cast<int>(sc.mouse_support));
    prefs_shared.set("numControllers", sc.numControllers);
    prefs_shared.set("hud_framecount", static_cast<int>(sc.hud_framecount));
    prefs_shared.set("hud_inputs", static_cast<int>(sc.hud_inputs));
    prefs_shared.set("hud_encode", static_cast<int>(sc.hud_encode));
    prefs_shared.set("prevent_savefiles", static_cast<int>(sc.prevent_savefiles));
    prefs_shared.set("audio_bitdepth", sc.audio_bitdepth);
    prefs_shared.set("audio_channels", sc.audio_channels);
    prefs_shared.set("audio_frequency", sc.audio_frequency);
    prefs_shared.set("audio_mute", static_cast<int>(sc.audio_mute));
#ifdef LIBTAS_ENABLE_AVDUMPING
    prefs_shared.set("video_codec", static_cast<int>(sc.video_codec));
    prefs_shared.set("video_bitrate", sc.video_bitrate);
    prefs_shared.set("audio_codec", static_cast<int>(sc.audio_codec));
    prefs_shared.set("audio_bitrate", sc.audio_bitrate);
#endif
    prefs_shared.set("main_gettimes_threshold", static_cast<void*>(sc.main_gettimes_threshold), sizeof(sc.main_gettimes_threshold));
    prefs_shared.set("sec_gettimes_threshold", static_cast<void*>(sc.sec_gettimes_threshold), sizeof(sc.sec_gettimes_threshold));
    prefs_shared.set("save_screenpixels", static_cast<int>(sc.save_screenpixels));
}

void Config::load(const std::string& gamepath) {

    /* Get the game executable name from path */
    size_t sep = gamepath.find_last_of("/");
    std::string gamename;
    if (sep != std::string::npos)
        gamename = gamepath.substr(sep + 1);
    else
        gamename = gamepath;

    /* Open the preferences for the game */
    prefs.reset(new Fl_Preferences(configdir.c_str(), "libtas", gamename.c_str()));
    char* text;
    if (prefs->get("gameargs", text, gameargs.c_str()))
        gameargs = text;
    free(text);

    std::string default_moviefile = gamepath + ".ltm";
    prefs->get("moviefile", text, default_moviefile.c_str());
    moviefile = text;
    free(text);

    std::string default_dumpfile = gamepath + ".mkv";
    prefs->get("dumpfile", text, default_dumpfile.c_str());
    dumpfile = text;
    free(text);

    if (prefs->get("libdir", text, libdir.c_str()))
        libdir = text;
    free(text);

    if (prefs->get("rundir", text, rundir.c_str()))
        rundir = text;
    free(text);

    int val;

    val = static_cast<int>(opengl_soft);
    prefs->get("opengl_soft", val, val);
    opengl_soft = static_cast<bool>(val);

    /* Load key mapping */

    Fl_Preferences prefs_km(*prefs, "keymapping");

    if (prefs_km.groupExists("hotkeys")) {
        Fl_Preferences prefs_km_hotkeys(prefs_km, "hotkeys");

        km.hotkey_mapping.clear();

        int n_entries = prefs_km_hotkeys.entries();
        for (int i=0; i<n_entries; i++) {
            const char* key = prefs_km_hotkeys.entry(i);
            const char def_data[sizeof(HotKeyType)] = {};
            void* vdata;
            if (prefs_km_hotkeys.get(key, vdata, def_data, sizeof(HotKeyType))) {
                HotKey hk;
                hk.unpack(static_cast<char*>(vdata));
                std::string key_str(key);
                km.hotkey_mapping[static_cast<KeySym>(std::stoi(key_str))] = hk;
            }
            free(vdata);
        }
    }

    if (prefs_km.groupExists("inputs")) {
        Fl_Preferences prefs_km_inputs(prefs_km, "inputs");

        km.input_mapping.clear();

        int n_entries = prefs_km_inputs.entries();
        for (int i=0; i<n_entries; i++) {
            const char* key = prefs_km_inputs.entry(i);
            const char def_data[sizeof(InputType) + sizeof(int)] = {};
            void* vdata;
            if (prefs_km_inputs.get(key, vdata, def_data, sizeof(InputType) + sizeof(int))) {
                SingleInput si;
                si.unpack(static_cast<char*>(vdata));
                std::string key_str(key);
                km.input_mapping[static_cast<KeySym>(std::stoi(key_str))] = si;
            }
            free(vdata);
        }
    }

    /* Load shared config */

    Fl_Preferences prefs_shared(*prefs, "shared");

    #define GETWITHTYPE(member, type) \
        val = static_cast<int>(sc.member); \
        prefs_shared.get(#member, val, val); \
        sc.member = static_cast<type>(val)

    GETWITHTYPE(speed_divisor, int);
    GETWITHTYPE(logging_status, SharedConfig::LogStatus);
    GETWITHTYPE(includeFlags, LogCategoryFlag);
    GETWITHTYPE(excludeFlags, LogCategoryFlag);
    GETWITHTYPE(framerate, unsigned int);
    GETWITHTYPE(keyboard_support, bool);
    GETWITHTYPE(mouse_support, bool);
    GETWITHTYPE(keyboard_support, bool);
    GETWITHTYPE(numControllers, int);
    GETWITHTYPE(hud_framecount, bool);
    GETWITHTYPE(hud_inputs, bool);
    GETWITHTYPE(hud_encode, bool);
    GETWITHTYPE(prevent_savefiles, bool);
    GETWITHTYPE(audio_bitdepth, int);
    GETWITHTYPE(audio_channels, int);
    GETWITHTYPE(audio_frequency, int);
    GETWITHTYPE(audio_mute, bool);
    #ifdef LIBTAS_ENABLE_AVDUMPING
    GETWITHTYPE(video_codec, AVCodecID);
    GETWITHTYPE(video_bitrate, int);
    GETWITHTYPE(audio_codec, AVCodecID);
    GETWITHTYPE(audio_bitrate, int);
    #endif
    GETWITHTYPE(save_screenpixels, bool);

    const int def_data[SharedConfig::TIMETYPE_NUMTRACKEDTYPES] = {};
    void* vdata;
    if (prefs_shared.get("main_gettimes_threshold", vdata, def_data, sizeof(sc.main_gettimes_threshold))) {
        int* tdata = static_cast<int*>(vdata);
        for (int t=0; t<SharedConfig::TIMETYPE_NUMTRACKEDTYPES; t++)
            sc.main_gettimes_threshold[t] = tdata[t];
    }
    free(vdata);

    if (prefs_shared.get("sec_gettimes_threshold", vdata, def_data, sizeof(sc.sec_gettimes_threshold))) {
        int* tdata = static_cast<int*>(vdata);
        for (int t=0; t<SharedConfig::TIMETYPE_NUMTRACKEDTYPES; t++)
            sc.sec_gettimes_threshold[t] = tdata[t];
    }
    free(vdata);
}
