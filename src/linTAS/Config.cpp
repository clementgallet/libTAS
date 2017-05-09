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
#include "utils.h"
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
    prefs_shared.set("video_codec", static_cast<int>(sc.video_codec));
    prefs_shared.set("video_bitrate", sc.video_bitrate);
    prefs_shared.set("audio_codec", static_cast<int>(sc.audio_codec));
    prefs_shared.set("audio_bitrate", sc.audio_bitrate);
}

void Config::load(const std::string& gamepath) {

    /* Get the game executable name from path */
    size_t sep = gamepath.find_last_of("/");
    std::string gamename;
    if (sep != std::string::npos)
        gamename = gamepath.substr(sep + 1);
    else
        gamename = gamepath;

    /* Check if our preferences directory exists, and create it if not */
    std::string prefs_dir = getenv("HOME");
    prefs_dir += "/.libtas";

    if (create_dir(prefs_dir))
        return;

    /* Open the preferences for the game */
    prefs.reset(new Fl_Preferences(prefs_dir.c_str(), "libtas", gamename.c_str()));
    char* text;
    if (prefs->get("gameargs", text, gameargs.c_str()))
        gameargs = text;
    free(text);

    std::string default_moviefile = gamepath + ".ltm";
    prefs->get("moviefile", text, default_moviefile.c_str());
    moviefile = text;
    free(text);

    if (prefs->get("dumpfile", text, dumpfile.c_str()))
        dumpfile = text;
    free(text);

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
    Fl_Preferences prefs_shared(*prefs, "shared");

    prefs_shared.get("speed_divisor", sc.speed_divisor, sc.speed_divisor);

    int val = static_cast<int>(sc.logging_status);
    prefs_shared.get("logging_status", val, val);
    sc.logging_status = static_cast<SharedConfig::LogStatus>(val);

    val = static_cast<int>(sc.includeFlags);
    prefs_shared.get("includeFlags", val, val);
    sc.includeFlags = static_cast<LogCategoryFlag>(val);

    val = static_cast<int>(sc.excludeFlags);
    prefs_shared.get("excludeFlags", val, val);
    sc.excludeFlags = static_cast<LogCategoryFlag>(val);

    val = static_cast<int>(sc.framerate);
    prefs_shared.get("framerate", val, val);
    sc.framerate = static_cast<unsigned int>(val);

    val = static_cast<int>(sc.keyboard_support);
    prefs_shared.get("keyboard_support", val, val);
    sc.keyboard_support = static_cast<bool>(val);

    val = static_cast<int>(sc.mouse_support);
    prefs_shared.get("mouse_support", val, val);
    sc.mouse_support = static_cast<bool>(val);

    prefs_shared.get("numControllers", sc.numControllers, sc.numControllers);

    val = static_cast<int>(sc.hud_framecount);
    prefs_shared.get("hud_framecount", val, val);
    sc.hud_framecount = static_cast<bool>(val);

    val = static_cast<int>(sc.hud_inputs);
    prefs_shared.get("hud_inputs", val, val);
    sc.hud_inputs = static_cast<bool>(val);

    val = static_cast<int>(sc.hud_encode);
    prefs_shared.get("hud_encode", val, val);
    sc.hud_encode = static_cast<bool>(val);

    val = static_cast<int>(sc.prevent_savefiles);
    prefs_shared.get("prevent_savefiles", val, val);
    sc.prevent_savefiles = static_cast<bool>(val);

    prefs_shared.get("audio_bitdepth", sc.audio_bitdepth, sc.audio_bitdepth);
    prefs_shared.get("audio_channels", sc.audio_channels, sc.audio_channels);
    prefs_shared.get("audio_frequency", sc.audio_frequency, sc.audio_frequency);

    val = static_cast<int>(sc.audio_mute);
    prefs_shared.get("audio_mute", val, val);
    sc.audio_mute = static_cast<bool>(val);

    val = static_cast<int>(sc.video_codec);
    prefs_shared.get("video_codec", val, val);
    sc.video_codec = static_cast<AVCodecID>(val);

    prefs_shared.get("video_bitrate", sc.video_bitrate, sc.video_bitrate);

    val = static_cast<int>(sc.audio_codec);
    prefs_shared.get("audio_codec", val, val);
    sc.audio_codec = static_cast<AVCodecID>(val);

    prefs_shared.get("audio_bitrate", sc.audio_bitrate, sc.audio_bitrate);
}
