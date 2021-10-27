/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QtCore/QSettings>
#include "Config.h"
#include <fcntl.h>
#include <unistd.h> // access
#include "utils.h"
#include "KeyMapping.h"

QString Config::iniPath(const std::string& gamepath) const {
    /* Get the game executable name from path */
    std::string gamename = fileFromPath(gamepath);
    return QString("%1/%2.ini").arg(configdir.c_str()).arg(gamename.c_str());
}

void Config::save(const std::string& gamepath) {
    /* Save only if game file exists */
    if (access(gamepath.c_str(), F_OK) != 0)
        return;

    /* Save the gamepath in recent gamepaths */
    for (auto iter = recent_gamepaths.begin(); iter != recent_gamepaths.end(); iter++) {
        if (iter->compare(gamepath) == 0) {
            recent_gamepaths.erase(iter);
            break;
        }
    }
    recent_gamepaths.push_front(gamepath);

    /* Save the option in recent options */
    for (auto iter = recent_args.begin(); iter != recent_args.end(); iter++) {
        if (iter->compare(gameargs) == 0) {
            recent_args.erase(iter);
            break;
        }
    }
    recent_args.push_front(gameargs);

    /* Open the general preferences */
    QSettings general_settings(QString("%1/libTAS.ini").arg(configdir.c_str()), QSettings::IniFormat);
    general_settings.setFallbacksEnabled(false);

    general_settings.remove("recent_gamepaths");
    general_settings.beginWriteArray("recent_gamepaths");
    int i = 0;
    for (auto& path : recent_gamepaths) {
        general_settings.setArrayIndex(i++);
        general_settings.setValue("gamepath", path.c_str());
        if (i > 10) break;
    }
    general_settings.endArray();

    general_settings.setValue("debugger", debugger);

    /* Open the preferences for the game */
    QSettings settings(iniPath(gamepath), QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

    /* Save recent options */
    settings.remove("recent_args");
    settings.beginWriteArray("recent_args");
    i = 0;
    for (auto& args : recent_args) {
        settings.setArrayIndex(i++);
        settings.setValue("args", args.c_str());
        if (i > 10) break;
    }
    settings.endArray();

    settings.setValue("gameargs", gameargs.c_str());
    settings.setValue("moviefile", moviefile.c_str());
    settings.setValue("dumpfile", dumpfile.c_str());
    settings.setValue("ffmpegoptions", ffmpegoptions.c_str());
    settings.setValue("libdir", libdir.c_str());
    settings.setValue("rundir", rundir.c_str());
    settings.setValue("on_movie_end", on_movie_end);
    settings.setValue("autosave", autosave);
    settings.setValue("autosave_delay_sec", autosave_delay_sec);
    settings.setValue("autosave_frames", autosave_frames);
    settings.setValue("autosave_count", autosave_count);
    settings.setValue("auto_restart", auto_restart);
    settings.setValue("mouse_warp", mouse_warp);
    settings.setValue("use_proton", use_proton);
    settings.setValue("proton_path", proton_path.c_str());
    settings.setValue("editor_autoscroll", editor_autoscroll);
    settings.setValue("editor_rewind_seek", editor_rewind_seek);

    settings.beginGroup("keymapping");

    settings.remove("hotkeys");
    settings.beginWriteArray("hotkeys");
    i = 0;
    for (auto& hmap : km->hotkey_mapping) {
        settings.setArrayIndex(i++);
        settings.setValue("keysym", hmap.first);
        settings.setValue("hotkey", QVariant::fromValue(hmap.second));
    }
    settings.endArray();

    settings.remove("inputs");
    settings.beginWriteArray("inputs");

    i = 0;
    for (auto& imap : km->input_mapping) {
        settings.setArrayIndex(i++);
        settings.setValue("keysym", imap.first);
        settings.setValue("input", QVariant::fromValue(imap.second));
    }
    settings.endArray();

    settings.endGroup();

    settings.beginGroup("shared");

    settings.setValue("speed_divisor", sc.speed_divisor);
    settings.setValue("fastforward_mode", sc.fastforward_mode);
    settings.setValue("logging_status", sc.logging_status);
    settings.setValue("includeFlags", sc.includeFlags);
    settings.setValue("excludeFlags", sc.excludeFlags);
    settings.setValue("framerate_num", sc.framerate_num);
    settings.setValue("framerate_den", sc.framerate_den);
    settings.setValue("mouse_support", sc.mouse_support);
    settings.setValue("mouse_mode_relative", sc.mouse_mode_relative);
    settings.setValue("mouse_prevent_warp", sc.mouse_prevent_warp);
    settings.setValue("nb_controllers", sc.nb_controllers);
    settings.setValue("screen_width", sc.screen_width);
    settings.setValue("screen_height", sc.screen_height);
    settings.setValue("osd", sc.osd);
    settings.setValue("osd_encode", sc.osd_encode);
    settings.setValue("osd_frame_location", sc.osd_frame_location);
    settings.setValue("osd_inputs_location", sc.osd_inputs_location);
    settings.setValue("prevent_savefiles", sc.prevent_savefiles);
    settings.setValue("recycle_threads", sc.recycle_threads);
    settings.setValue("audio_bitdepth", sc.audio_bitdepth);
    settings.setValue("audio_channels", sc.audio_channels);
    settings.setValue("audio_frequency", sc.audio_frequency);
    settings.setValue("audio_mute", sc.audio_mute);
    settings.setValue("audio_disabled", sc.audio_disabled);
    settings.setValue("video_codec", sc.video_codec);
    settings.setValue("video_bitrate", sc.video_bitrate);
    settings.setValue("video_framerate", sc.video_framerate);
    settings.setValue("audio_codec", sc.audio_codec);
    settings.setValue("audio_bitrate", sc.audio_bitrate);
    settings.setValue("locale", sc.locale);
    settings.setValue("virtual_steam", sc.virtual_steam);
    settings.setValue("opengl_soft", sc.opengl_soft);
    settings.setValue("opengl_performance", sc.opengl_performance);
    settings.setValue("async_events", sc.async_events);
    settings.setValue("wait_timeout", sc.wait_timeout);
    settings.setValue("game_specific_timing", sc.game_specific_timing);
    settings.setValue("game_specific_sync", sc.game_specific_sync);
    settings.setValue("variable_framerate", sc.variable_framerate);

    settings.beginWriteArray("main_gettimes_threshold");
    for (int t=0; t<SharedConfig::TIMETYPE_NUMTRACKEDTYPES; t++) {
        settings.setArrayIndex(t);
        settings.setValue("value", sc.main_gettimes_threshold[t]);
    }
    settings.endArray();

    settings.setValue("savestate_settings", sc.savestate_settings);

    settings.endGroup();
}

void Config::load(const std::string& gamepath) {

    /* Open the general preferences */
    QSettings general_settings(QString("%1/libTAS.ini").arg(configdir.c_str()), QSettings::IniFormat);
    general_settings.setFallbacksEnabled(false);

    int size = general_settings.beginReadArray("recent_gamepaths");
    if (size > 0)
        recent_gamepaths.clear();
    for (int i = 0; i < size; ++i) {
        general_settings.setArrayIndex(i);
        std::string path = general_settings.value("gamepath").toString().toStdString();
        recent_gamepaths.push_back(path);
    }
    general_settings.endArray();

#ifdef __unix__
    debugger = general_settings.value("debugger", debugger).toInt();
#elif defined(__APPLE__) && defined(__MACH__)
    debugger = DEBUGGER_LLDB;
#endif

    if (gamepath.empty())
        return;

    /* Open the preferences for the game */
    QSettings settings(iniPath(gamepath), QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

    size = settings.beginReadArray("recent_args");
    recent_args.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        std::string args = settings.value("args").toString().toStdString();
        recent_args.push_back(args);
    }
    settings.endArray();

    gameargs = settings.value("gameargs", "").toString().toStdString();

    std::string default_moviefile = gamepath + ".ltm";
    moviefile = settings.value("moviefile", default_moviefile.c_str()).toString().toStdString();

    std::string default_dumpfile = gamepath + ".mkv";
    dumpfile = settings.value("dumpfile", default_dumpfile.c_str()).toString().toStdString();

    ffmpegoptions = settings.value("ffmpegoptions", ffmpegoptions.c_str()).toString().toStdString();

    libdir = settings.value("libdir", "").toString().toStdString();
    rundir = settings.value("rundir", "").toString().toStdString();

    on_movie_end = settings.value("on_movie_end", on_movie_end).toInt();
    autosave = settings.value("autosave", autosave).toBool();
    autosave_delay_sec = settings.value("autosave_delay_sec", autosave_delay_sec).toDouble();
    autosave_frames = settings.value("autosave_frames", autosave_frames).toInt();
    autosave_count = settings.value("autosave_count", autosave_count).toInt();
    auto_restart = settings.value("auto_restart", auto_restart).toBool();
    mouse_warp = settings.value("mouse_warp", mouse_warp).toBool();
    use_proton = settings.value("use_proton", use_proton).toBool();
    proton_path = settings.value("proton_path", "").toString().toStdString();
    editor_autoscroll = settings.value("editor_autoscroll", editor_autoscroll).toBool();
    editor_rewind_seek = settings.value("editor_rewind_seek", editor_rewind_seek).toBool();

    /* Load key mapping */

    settings.beginGroup("keymapping");

    size = settings.beginReadArray("hotkeys");
    km->default_hotkeys();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        keysym_t keysym = settings.value("keysym").toInt();
        HotKey hotkey = settings.value("hotkey").value<HotKey>();
        km->reassign_hotkey(hotkey, keysym);
    }
    settings.endArray();

    size = settings.beginReadArray("inputs");
    if (size > 0)
        km->input_mapping.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        keysym_t keysym = settings.value("keysym").toInt();
        SingleInput si = settings.value("input").value<SingleInput>();
        km->reassign_input(si, keysym);
    }
    settings.endArray();

    settings.endGroup();

    /* Load shared config */
    settings.beginGroup("shared");

    sc.speed_divisor = settings.value("speed_divisor", sc.speed_divisor).toInt();
    sc.fastforward_mode = settings.value("fastforward_mode", sc.fastforward_mode).toInt();
    sc.logging_status = settings.value("logging_status", sc.logging_status).toInt();
    sc.includeFlags = settings.value("includeFlags", sc.includeFlags).toInt();
    sc.excludeFlags = settings.value("excludeFlags", sc.excludeFlags).toInt();
    sc.framerate_num = settings.value("framerate_num", sc.framerate_num).toUInt();
    sc.framerate_den = settings.value("framerate_den", sc.framerate_den).toUInt();
    sc.mouse_support = settings.value("mouse_support", sc.mouse_support).toBool();
    sc.mouse_mode_relative = settings.value("mouse_mode_relative", sc.mouse_mode_relative).toBool();
    sc.mouse_prevent_warp = settings.value("mouse_prevent_warp", sc.mouse_prevent_warp).toBool();
    sc.nb_controllers = settings.value("nb_controllers", sc.nb_controllers).toInt();
    sc.screen_width = settings.value("screen_width", sc.screen_width).toInt();
    sc.screen_height = settings.value("screen_height", sc.screen_height).toInt();
    sc.osd = settings.value("osd", sc.osd).toInt();
    sc.osd_encode = settings.value("osd_encode", sc.osd_encode).toBool();
    sc.osd_frame_location = settings.value("osd_frame_location", sc.osd_frame_location).toInt();
    sc.osd_inputs_location = settings.value("osd_inputs_location", sc.osd_inputs_location).toInt();
    sc.prevent_savefiles = settings.value("prevent_savefiles", sc.prevent_savefiles).toBool();
    sc.recycle_threads = settings.value("recycle_threads", sc.recycle_threads).toBool();
    sc.audio_bitdepth = settings.value("audio_bitdepth", sc.audio_bitdepth).toInt();
    sc.audio_channels = settings.value("audio_channels", sc.audio_channels).toInt();
    sc.audio_frequency = settings.value("audio_frequency", sc.audio_frequency).toInt();
    sc.audio_mute = settings.value("audio_mute", sc.audio_mute).toBool();
    sc.audio_disabled = settings.value("audio_disabled", sc.audio_disabled).toBool();
    sc.locale = settings.value("locale", sc.locale).toInt();
    sc.virtual_steam = settings.value("virtual_steam", sc.virtual_steam).toBool();
    sc.async_events = settings.value("async_events", sc.async_events).toInt();
    sc.wait_timeout = settings.value("wait_timeout", sc.wait_timeout).toInt();
    sc.game_specific_timing = settings.value("game_specific_timing", sc.game_specific_timing).toInt();
    sc.game_specific_sync = settings.value("game_specific_sync", sc.game_specific_sync).toInt();
    sc.variable_framerate = settings.value("variable_framerate", sc.variable_framerate).toBool();

    sc.video_codec = settings.value("video_codec", sc.video_codec).toInt();
    sc.video_bitrate = settings.value("video_bitrate", sc.video_bitrate).toInt();
    sc.video_framerate = settings.value("video_framerate", sc.video_framerate).toInt();
    sc.audio_codec = settings.value("audio_codec", sc.audio_codec).toInt();
    sc.audio_bitrate = settings.value("audio_bitrate", sc.audio_bitrate).toInt();
    sc.savestate_settings = settings.value("savestate_settings", sc.savestate_settings).toInt();
    sc.opengl_soft = settings.value("opengl_soft", sc.opengl_soft).toBool();
    sc.opengl_performance = settings.value("opengl_performance", sc.opengl_performance).toBool();

    size = settings.beginReadArray("main_gettimes_threshold");
    for (int t=0; t<size; t++) {
        settings.setArrayIndex(t);
        sc.main_gettimes_threshold[t] = settings.value("value", sc.main_gettimes_threshold[t]).toInt();
    }
    settings.endArray();

    settings.endGroup();
}
