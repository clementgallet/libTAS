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

#include "MovieFileHeader.h"
#include "../../shared/version.h"

MovieFileHeader::MovieFileHeader(Context* c) : context(c)
{
    clear();
}

void MovieFileHeader::clear()
{
    /* For new movies, will be overwritten when loading a moviefile */
    framerate_num = context->config.sc.framerate_num;
    framerate_den = context->config.sc.framerate_den;
}

void MovieFileHeader::load()
{
    /* Load the config file into the context struct */
    QString configfile = context->config.tempmoviedir.c_str();
    configfile += "/config.ini";

    QSettings config(configfile, QSettings::IniFormat);
    config.setFallbacksEnabled(false);

    context->config.sc.movie_framecount = config.value("frame_count").toULongLong();
    if (!skipLoadSettings) {
        context->config.sc.mouse_support = config.value("mouse_support").toBool();

        context->config.sc.nb_controllers = config.value("nb_controllers").toInt();
        context->config.sc.initial_time_sec = config.value("initial_time_sec").toULongLong();
        context->config.sc.initial_time_nsec = config.value("initial_time_nsec").toULongLong();
        context->config.sc.initial_monotonic_time_sec = config.value("initial_monotonic_time_sec").toULongLong();
        context->config.sc.initial_monotonic_time_nsec = config.value("initial_monotonic_time_nsec").toULongLong();
        
        framerate_num = config.value("framerate_num").toUInt();
        framerate_den = config.value("framerate_den").toUInt();
        /* Compatibility with older movie format */
        if (!framerate_num) {
            framerate_num = config.value("framerate").toUInt();
            framerate_den = 1;
        }

        context->config.sc.framerate_num = framerate_num;
        context->config.sc.framerate_den = framerate_den;
        
        context->config.auto_restart = config.value("auto_restart").toBool();
        context->config.sc.variable_framerate = config.value("variable_framerate").toBool();
        
        config.beginGroup("mainthread_timetrack");
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME] = config.value("time", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] = config.value("gettimeofday", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] = config.value("clock", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME] = config.value("clock_gettime_real", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC] = config.value("clock_gettime_monotonic", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] = config.value("sdl_getticks", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] = config.value("sdl_getperformancecounter", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT] = config.value("GetTickCount", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT64] = config.value("GetTickCount64", -1).toInt();
        context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER] = config.value("QueryPerformanceCounter", -1).toInt();
        config.endGroup();
    }

    context->movie_time_sec = config.value("length_sec").toULongLong();
    context->movie_time_nsec = config.value("length_nsec").toULongLong();
    /* If no movie length field, compute from frame count and framerate */
    if (!context->movie_time_sec && !context->movie_time_nsec) {
        context->movie_time_sec = (uint64_t)(context->config.sc.movie_framecount) * context->config.sc.framerate_den / context->config.sc.framerate_num;
        context->movie_time_nsec = ((1000000000ull * (uint64_t)context->config.sc.movie_framecount * context->config.sc.framerate_den) / context->config.sc.framerate_num) % 1000000000ull;
    }

    context->rerecord_count = config.value("rerecord_count").toUInt();
    context->authors = config.value("authors").toString().toStdString();
    context->md5_movie = config.value("md5").toString().toStdString();
    savestate_framecount = config.value("savestate_frame_count").toULongLong();
}

void MovieFileHeader::loadSavestate()
{
    /* Load the config file into the context struct */
    QString configfile = context->config.tempmoviedir.c_str();
    configfile += "/config.ini";

    QSettings config(configfile, QSettings::IniFormat);
    config.setFallbacksEnabled(false);

    framerate_num = config.value("framerate_num").toUInt();
    framerate_den = config.value("framerate_den").toUInt();
    /* Compatibility with older movie format */
    if (!framerate_num) {
        framerate_num = config.value("framerate").toUInt();
        framerate_den = 1;
    }

    length_sec = config.value("length_sec").toULongLong();
    length_nsec = config.value("length_nsec").toULongLong();
    savestate_framecount = config.value("savestate_frame_count").toULongLong();
}

void MovieFileHeader::save(uint64_t tot_frames, uint64_t nb_frames)
{
    savestate_framecount = nb_frames;
    
    /* Save some parameters into the config file */
    QString configfile = context->config.tempmoviedir.c_str();
    configfile += "/config.ini";

    QSettings config(configfile, QSettings::IniFormat);
    config.setFallbacksEnabled(false);

    config.setValue("game_name", context->gamename.c_str());
    config.setValue("frame_count", static_cast<unsigned long long>(tot_frames));
    config.setValue("mouse_support", context->config.sc.mouse_support);
    config.setValue("nb_controllers", context->config.sc.nb_controllers);
    config.setValue("initial_time_sec", static_cast<unsigned long long>(context->config.sc.initial_time_sec));
    config.setValue("initial_time_nsec", static_cast<unsigned long long>(context->config.sc.initial_time_nsec));
    config.setValue("initial_monotonic_time_sec", static_cast<unsigned long long>(context->config.sc.initial_monotonic_time_sec));
    config.setValue("initial_monotonic_time_nsec", static_cast<unsigned long long>(context->config.sc.initial_monotonic_time_nsec));
    config.setValue("length_sec", static_cast<unsigned long long>(context->movie_time_sec));
    config.setValue("length_nsec", static_cast<unsigned long long>(context->movie_time_nsec));
    config.setValue("framerate_num", framerate_num);
    config.setValue("framerate_den", framerate_den);
    config.setValue("rerecord_count", context->rerecord_count);
    config.setValue("authors", context->authors.c_str());
    config.setValue("libtas_major_version", MAJORVERSION);
    config.setValue("libtas_minor_version", MINORVERSION);
    config.setValue("libtas_patch_version", PATCHVERSION);
    config.setValue("savestate_frame_count", static_cast<unsigned long long>(nb_frames));
    config.setValue("auto_restart", context->config.auto_restart);
    config.setValue("variable_framerate", context->config.sc.variable_framerate);

    if (!context->md5_game.empty())
        config.setValue("md5", context->md5_game.c_str());

    config.beginGroup("mainthread_timetrack");
    config.setValue("time", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
    config.setValue("gettimeofday", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
    config.setValue("clock", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
    config.setValue("clock_gettime_real", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME]);
    config.setValue("clock_gettime_monotonic", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC]);
    config.setValue("sdl_getticks", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
    config.setValue("sdl_getperformancecounter", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);
    config.setValue("GetTickCount", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT]);
    config.setValue("GetTickCount64", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT64]);
    config.setValue("QueryPerformanceCounter", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER]);


    config.endGroup();

    config.sync();
}
