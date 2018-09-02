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

#include <QSettings>
#include "Config.h"

QString Config::iniPath(const std::string& gamepath) const {
    /* Get the game executable name from path */
    size_t sep = gamepath.find_last_of("/");
    std::string gamename;
    if (sep != std::string::npos)
        gamename = gamepath.substr(sep + 1);
    else
        gamename = gamepath;

    return QString("%1/%2.ini").arg(configdir.c_str()).arg(gamename.c_str());
}

void Config::save(const std::string& gamepath) {

    /* Open the preferences for the game */
    QSettings settings(iniPath(gamepath), QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

    settings.setValue("gameargs", gameargs.c_str());
    settings.setValue("moviefile", moviefile.c_str());
    settings.setValue("dumpfile", dumpfile.c_str());
    settings.setValue("ffmpegoptions", ffmpegoptions.c_str());
    settings.setValue("libdir", libdir.c_str());
    settings.setValue("rundir", rundir.c_str());
    settings.setValue("opengl_soft", opengl_soft);
    settings.setValue("on_movie_end", on_movie_end);

    settings.beginGroup("keymapping");

    settings.remove("hotkeys");
    settings.beginWriteArray("hotkeys");
    int i = 0;
    for (auto& hmap : km.hotkey_mapping) {
        settings.setArrayIndex(i++);
        settings.setValue("keysym", hmap.first);
        settings.setValue("hotkey", QVariant::fromValue(hmap.second));
    }
    settings.endArray();

    settings.remove("inputs");
    settings.beginWriteArray("inputs");

    i = 0;
    for (auto& imap : km.input_mapping) {
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
    settings.setValue("keyboard_support", sc.keyboard_support);
    settings.setValue("mouse_support", sc.mouse_support);
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
    settings.setValue("video_codec", sc.video_codec);
    settings.setValue("video_bitrate", sc.video_bitrate);
    settings.setValue("audio_codec", sc.audio_codec);
    settings.setValue("audio_bitrate", sc.audio_bitrate);
    settings.setValue("locale", sc.locale);

    settings.beginWriteArray("main_gettimes_threshold");
    for (int t=0; t<SharedConfig::TIMETYPE_NUMTRACKEDTYPES; t++) {
        settings.setArrayIndex(t);
        settings.setValue("value", sc.main_gettimes_threshold[t]);
    }
    settings.endArray();

    settings.beginWriteArray("sec_gettimes_threshold");
    for (int t=0; t<SharedConfig::TIMETYPE_NUMTRACKEDTYPES; t++) {
        settings.setArrayIndex(t);
        settings.setValue("value", sc.sec_gettimes_threshold[t]);
    }
    settings.endArray();

    settings.setValue("save_screenpixels", sc.save_screenpixels);
    settings.setValue("incremental_savestates", sc.incremental_savestates);
    settings.setValue("savestates_in_ram", sc.savestates_in_ram);

    settings.endGroup();
}

void Config::load(const std::string& gamepath) {

    /* Open the preferences for the game */
    QSettings settings(iniPath(gamepath), QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

    gameargs = settings.value("gameargs", "").toString().toStdString();

    std::string default_moviefile = gamepath + ".ltm";
    moviefile = settings.value("moviefile", default_moviefile.c_str()).toString().toStdString();

    std::string default_dumpfile = gamepath + ".mkv";
    dumpfile = settings.value("dumpfile", default_dumpfile.c_str()).toString().toStdString();

    ffmpegoptions = settings.value("ffmpegoptions", ffmpegoptions.c_str()).toString().toStdString();

    libdir = settings.value("libdir", "").toString().toStdString();
    rundir = settings.value("rundir", "").toString().toStdString();

    opengl_soft = settings.value("opengl_soft", opengl_soft).toBool();
    on_movie_end = settings.value("on_movie_end", on_movie_end).toInt();

    /* Load key mapping */

    settings.beginGroup("keymapping");

    int size = settings.beginReadArray("hotkeys");
    if (size > 0)
        km.hotkey_mapping.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        KeySym keysym = settings.value("keysym").toInt();
        HotKey hotkey = settings.value("hotkey").value<HotKey>();
        km.hotkey_mapping[keysym] = hotkey;
    }
    settings.endArray();

    size = settings.beginReadArray("inputs");
    if (size > 0)
        km.input_mapping.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        KeySym keysym = settings.value("keysym").toInt();
        SingleInput si = settings.value("input").value<SingleInput>();
        km.input_mapping[keysym] = si;
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
    sc.keyboard_support = settings.value("keyboard_support", sc.keyboard_support).toBool();
    sc.mouse_support = settings.value("mouse_support", sc.mouse_support).toBool();
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
    sc.locale = settings.value("locale", sc.locale).toInt();

    sc.video_codec = settings.value("video_codec", sc.video_codec).toInt();
    sc.video_bitrate = settings.value("video_bitrate", sc.video_bitrate).toInt();
    sc.audio_codec = settings.value("audio_codec", sc.audio_codec).toInt();
    sc.audio_bitrate = settings.value("audio_bitrate", sc.audio_bitrate).toInt();
    sc.save_screenpixels = settings.value("save_screenpixels", sc.save_screenpixels).toBool();
    sc.incremental_savestates = settings.value("incremental_savestates", sc.incremental_savestates).toBool();
    sc.savestates_in_ram = settings.value("savestates_in_ram", sc.savestates_in_ram).toBool();

    size = settings.beginReadArray("main_gettimes_threshold");
    for (int t=0; t<size; t++) {
        settings.setArrayIndex(t);
        sc.main_gettimes_threshold[t] = settings.value("value", sc.main_gettimes_threshold[t]).toInt();
    }
    settings.endArray();

    size = settings.beginReadArray("sec_gettimes_threshold");
    for (int t=0; t<size; t++) {
        settings.setArrayIndex(t);
        sc.sec_gettimes_threshold[t] = settings.value("value", sc.sec_gettimes_threshold[t]).toInt();
    }
    settings.endArray();

    settings.endGroup();
}
