/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "KeyMapping.h"

#include <QtCore/QSettings>
#include <fcntl.h>
#include <unistd.h> // access
#include <iostream>

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
    general_settings.setValue("allow_downloads", allow_downloads);

    general_settings.setValue("datadir", datadir.c_str());
    general_settings.setValue("steamuserdir", steamuserdir.c_str());
    general_settings.setValue("tempmoviedir", tempmoviedir.c_str());
    general_settings.setValue("savestatedir", savestatedir.c_str());
    general_settings.setValue("ramsearchdir", ramsearchdir.c_str());
    general_settings.setValue("extralib32dir", extralib32dir.c_str());
    general_settings.setValue("extralib64dir", extralib64dir.c_str());

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
    settings.setValue("screenshotfile", screenshotfile.c_str());
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
    settings.setValue("editor_rewind_fastforward", editor_rewind_fastforward);
    settings.setValue("editor_marker_pause", editor_marker_pause);

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
    settings.setValue("fastforward_render", sc.fastforward_render);
    settings.setValue("logging_status", sc.logging_status);
    settings.setValue("logging_level", sc.logging_level);
    settings.setValue("logging_include_flags", sc.logging_include_flags);
    settings.setValue("logging_exclude_flags", sc.logging_exclude_flags);
    settings.setValue("framerate_num", sc.initial_framerate_num);
    settings.setValue("framerate_den", sc.initial_framerate_den);
    settings.setValue("mouse_support", sc.mouse_support);
    settings.setValue("mouse_mode_relative", sc.mouse_mode_relative);
    settings.setValue("mouse_prevent_warp", sc.mouse_prevent_warp);
    settings.setValue("nb_controllers", sc.nb_controllers);
    settings.setValue("screen_width", sc.screen_width);
    settings.setValue("screen_height", sc.screen_height);
    settings.setValue("osd", sc.osd);
    settings.setValue("osd_encode", sc.osd_encode);
    settings.setValue("prevent_savefiles", sc.prevent_savefiles);
    settings.setValue("audio_bitdepth", sc.audio_bitdepth);
    settings.setValue("audio_channels", sc.audio_channels);
    settings.setValue("audio_frequency", sc.audio_frequency);
    settings.setValue("audio_gain", sc.audio_gain);
    settings.setValue("audio_mute", sc.audio_mute);
    settings.setValue("audio_disabled", sc.audio_disabled);
    settings.setValue("video_codec", sc.video_codec);
    settings.setValue("video_bitrate", sc.video_bitrate);
    settings.setValue("video_framerate", sc.video_framerate);
    settings.setValue("audio_codec", sc.audio_codec);
    settings.setValue("audio_bitrate", sc.audio_bitrate);
    settings.setValue("locale", sc.locale);
    settings.setValue("virtual_steam", sc.virtual_steam);
    settings.setValue("openal_soft", sc.openal_soft);
    settings.setValue("opengl_soft", sc.opengl_soft);
    settings.setValue("opengl_performance", sc.opengl_performance);
    settings.setValue("async_events", sc.async_events);
    settings.setValue("wait_timeout", sc.wait_timeout);
    settings.setValue("sleep_handling", sc.sleep_handling);
    settings.setValue("game_specific_timing", sc.game_specific_timing);
    settings.setValue("game_specific_sync", sc.game_specific_sync);

    settings.beginWriteArray("main_gettimes_threshold");
    for (int t=0; t<SharedConfig::TIMETYPE_NUMTRACKEDTYPES; t++) {
        settings.setArrayIndex(t);
        settings.setValue("value", sc.main_gettimes_threshold[t]);
    }
    settings.endArray();

    settings.setValue("savestate_settings", sc.savestate_settings);

    settings.endGroup();
}

void Config::saveDefaultFfmpeg(const std::string& gamepath) {
    /* Save only if game file exists */
    if (access(gamepath.c_str(), F_OK) != 0)
        return;

    /* Open the general preferences */
    QSettings general_settings(QString("%1/libTAS.ini").arg(configdir.c_str()), QSettings::IniFormat);
    general_settings.setFallbacksEnabled(false);

    general_settings.setValue("ffmpegoptions", ffmpegoptions.c_str());
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
    allow_downloads = general_settings.value("allow_downloads", allow_downloads).toInt();

    ffmpegoptions = general_settings.value("ffmpegoptions", ffmpegoptions.c_str()).toString().toStdString();

    char *path;
    if (general_settings.contains("datadir")) {
        datadir = general_settings.value("datadir").toString().toStdString();        
    }
    else {
        path = getenv("XDG_DATA_HOME");
        if (path) {
            datadir = path;
        }
        else {
            datadir = getenv("HOME");
            datadir += "/.local/share";
        }
        datadir += "/libTAS";
    }

    std::string subpath;

    subpath = datadir + "/steam";
    steamuserdir = general_settings.value("steamuserdir", subpath.c_str()).toString().toStdString();

    subpath = datadir + "/movie";
    tempmoviedir = general_settings.value("tempmoviedir", subpath.c_str()).toString().toStdString();

    subpath = datadir + "/states";
    savestatedir = general_settings.value("savestatedir", subpath.c_str()).toString().toStdString();

    subpath = datadir + "/ramsearch";
    ramsearchdir = general_settings.value("ramsearchdir", subpath.c_str()).toString().toStdString();

    subpath = datadir + "/lib_i386";
    extralib32dir = general_settings.value("extralib32dir", subpath.c_str()).toString().toStdString();

    subpath = datadir + "/lib_amd64";
    extralib64dir = general_settings.value("extralib64dir", subpath.c_str()).toString().toStdString();

    createDirectories();

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

    std::string default_screenshotfile = dirFromPath(gamepath) + "/screenshot.png";
    screenshotfile = settings.value("screenshotfile", default_screenshotfile.c_str()).toString().toStdString();

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
    editor_rewind_fastforward = settings.value("editor_rewind_fastforward", editor_rewind_fastforward).toBool();
    editor_marker_pause = settings.value("editor_marker_pause", editor_marker_pause).toBool();

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
    sc.fastforward_render = settings.value("fastforward_render", sc.fastforward_render).toInt();
    sc.logging_status = settings.value("logging_status", sc.logging_status).toInt();
    sc.logging_level = settings.value("logging_level", sc.logging_level).toUInt();
    sc.logging_include_flags = settings.value("logging_include_flags", sc.logging_include_flags).toUInt();
    sc.logging_exclude_flags = settings.value("logging_exclude_flags", sc.logging_exclude_flags).toUInt();
    sc.initial_framerate_num = settings.value("framerate_num", sc.initial_framerate_num).toUInt();
    sc.initial_framerate_den = settings.value("framerate_den", sc.initial_framerate_den).toUInt();
    sc.mouse_support = settings.value("mouse_support", sc.mouse_support).toBool();
    sc.mouse_mode_relative = settings.value("mouse_mode_relative", sc.mouse_mode_relative).toBool();
    sc.mouse_prevent_warp = settings.value("mouse_prevent_warp", sc.mouse_prevent_warp).toBool();
    sc.nb_controllers = settings.value("nb_controllers", sc.nb_controllers).toInt();
    sc.screen_width = settings.value("screen_width", sc.screen_width).toInt();
    sc.screen_height = settings.value("screen_height", sc.screen_height).toInt();
    sc.osd = settings.value("osd", sc.osd).toBool();
    sc.osd_encode = settings.value("osd_encode", sc.osd_encode).toBool();
    sc.prevent_savefiles = settings.value("prevent_savefiles", sc.prevent_savefiles).toBool();
    sc.audio_bitdepth = settings.value("audio_bitdepth", sc.audio_bitdepth).toInt();
    sc.audio_channels = settings.value("audio_channels", sc.audio_channels).toInt();
    sc.audio_frequency = settings.value("audio_frequency", sc.audio_frequency).toInt();
    sc.audio_gain = settings.value("audio_gain", sc.audio_gain).toFloat();
    sc.audio_mute = settings.value("audio_mute", sc.audio_mute).toBool();
    sc.audio_disabled = settings.value("audio_disabled", sc.audio_disabled).toBool();
    sc.openal_soft = settings.value("openal_soft", sc.openal_soft).toBool();
    sc.locale = settings.value("locale", sc.locale).toInt();
    sc.virtual_steam = settings.value("virtual_steam", sc.virtual_steam).toBool();
    sc.async_events = settings.value("async_events", sc.async_events).toInt();
    sc.wait_timeout = settings.value("wait_timeout", sc.wait_timeout).toInt();
    sc.sleep_handling = settings.value("sleep_handling", sc.sleep_handling).toInt();
    sc.game_specific_timing = settings.value("game_specific_timing", sc.game_specific_timing).toInt();
    sc.game_specific_sync = settings.value("game_specific_sync", sc.game_specific_sync).toInt();

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

void Config::createDirectories()
{
    if (create_dir(datadir) < 0) {
        std::cerr << "Cannot create dir " << datadir << std::endl;
        return;
    }

    if (create_dir(steamuserdir) < 0) {
        std::cerr << "Cannot create dir " << steamuserdir << std::endl;
        return;
    }

    if (create_dir(tempmoviedir) < 0) {
        std::cerr << "Cannot create dir " << tempmoviedir << std::endl;
        return;
    }

    if (create_dir(savestatedir) < 0) {
        std::cerr << "Cannot create dir " << savestatedir << std::endl;
        return;
    }

    if (create_dir(ramsearchdir) < 0) {
        std::cerr << "Cannot create dir " << ramsearchdir << std::endl;
        return;
    }

    if (create_dir(extralib32dir) < 0) {
        std::cerr << "Cannot create dir " << extralib32dir << std::endl;
        return;
    }

    if (create_dir(extralib64dir) < 0) {
        std::cerr << "Cannot create dir " << extralib64dir << std::endl;
        return;
    }
}
