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

#include "MainWindow.h"
#include "../game.h"
#include "../MovieFile.h"
#include <iostream>
#include <FL/x.H>
#include <FL/fl_ask.H>

static Fl_Callback browse_gamepath_cb;
static Fl_Callback browse_moviepath_cb;
static Fl_Callback0 set_fps_cb;
static Fl_Callback0 pause_cb;
static Fl_Callback0 fastforward_cb;
static Fl_Callback0 movie_recording_cb;
static Fl_Callback0 movie_toggle_rw_cb;
#ifdef LIBTAS_ENABLE_AVDUMPING
static Fl_Callback config_encode_cb;
static Fl_Callback toggle_encode_cb;
#endif
static Fl_Callback config_input_cb;
static Fl_Callback config_executable_cb;
static Fl_Callback sound_frequency_cb;
static Fl_Callback sound_bitdepth_cb;
static Fl_Callback sound_channel_cb;
static Fl_Callback mute_sound_cb;
static Fl_Callback logging_status_cb;
static Fl_Callback logging_print_cb;
static Fl_Callback logging_exclude_cb;
static Fl_Callback input_keyboard_cb;
static Fl_Callback input_mouse_cb;
static Fl_Callback input_joy_cb;
static Fl_Callback hotkeys_focus_cb;
static Fl_Callback inputs_focus_cb;
static Fl_Callback slowmo_cb;
#ifdef LIBTAS_ENABLE_HUD
static Fl_Callback osd_frame_cb;
static Fl_Callback osd_inputs_cb;
static Fl_Callback osd_encode_cb;
#endif
static Fl_Callback time_main_cb;
static Fl_Callback time_sec_cb;
static Fl_Callback render_soft_cb;
static Fl_Callback savestate_screen_cb;
static Fl_Callback0 cmdoptions_cb;
static Fl_Callback llvm_perf_cb;
static Fl_Callback ignore_memory_cb;
static Fl_Callback0 initial_time_cb;
static Fl_Callback prevent_savefiles_cb;
static Fl_Callback controller_inputs_cb;

MainWindow::~MainWindow()
{
    if (game_thread.joinable())
        game_thread.detach();
}

void MainWindow::build(Context* c)
{
    context = c;
    window = new Fl_Double_Window(600, 500);

    /* Menu */
    menu_bar = new Fl_Menu_Bar(0, 0, window->w(), 30);
    menu_bar->menu(menu_items);

    /* Game Executable */
    gamepath = new Fl_Output(10, 340, 500, 30, "Game Executable");
    gamepath->align(FL_ALIGN_TOP_LEFT);
    gamepath->color(FL_LIGHT1);

    browsegamepath = new Fl_Button(520, 340, 70, 30, "Browse...");
    browsegamepath->callback(browse_gamepath_cb);

    gamepathchooser = new Fl_Native_File_Chooser();
    gamepathchooser->title("Game path");

    /* Command-line options */
    cmdoptions = new Fl_Input(10, 400, 500, 30, "Command-line options");
    cmdoptions->align(FL_ALIGN_TOP_LEFT);
    cmdoptions->callback(cmdoptions_cb);
    // cmdoptions->color(FL_LIGHT1);

    /* Movie File */
    moviepath = new Fl_Output(10, 50, 500, 30, "Movie File");
    moviepath->align(FL_ALIGN_TOP_LEFT);
    moviepath->color(FL_LIGHT1);

    moviepathchooser = new Fl_Native_File_Chooser();
    moviepathchooser->title("Choose a movie file");
    moviepathchooser->filter("libTAS movie file \t*.ltm\n");

    browsemoviepath = new Fl_Button(520, 50, 70, 30, "Browse...");
    browsemoviepath->callback(browse_moviepath_cb);

    movie_recording = new Fl_Check_Button(10, 90, 160, 20, "Record Inputs");
    movie_recording->callback(movie_recording_cb);
    movie_recording->set();

    movie_read_only = new Fl_Check_Button(200, 90, 160, 20, "Read Only");
    movie_read_only->callback(movie_toggle_rw_cb);

    /* Frames per second */
    logicalfps = new Fl_Int_Input(160, 180, 40, 30, "Frames per second");
    logicalfps->callback(set_fps_cb);

    /* Pause/FF */
    pausecheck = new Fl_Check_Button(440, 140, 80, 20, "Pause");
    pausecheck->callback(pause_cb);
    fastforwardcheck = new Fl_Check_Button(440, 170, 80, 20, "Fast-forward");
    fastforwardcheck->callback(fastforward_cb);

    /* Mute */
    mutecheck = new Fl_Check_Button(440, 200, 80, 20, "Mute");
    mutecheck->callback(mute_sound_cb);
#ifndef LIBTAS_ENABLE_SOUNDPLAYBACK
    mutecheck->label("Mute (disabled)");
    mutecheck->deactivate();
#endif

    /* Frame count */
    framecount = new Fl_Output(80, 140, 80, 30, "Frames:");
    framecount->value("0");
    framecount->color(FL_LIGHT1);

    movie_framecount = new Fl_Output(180, 140, 80, 30, " / ");
    movie_framecount->color(FL_LIGHT1);

    rerecord_count = new Fl_Output(280, 180, 80, 30, "Rerecord count:");
    rerecord_count->align(FL_ALIGN_TOP_LEFT);
    rerecord_count->color(FL_LIGHT1);

    MovieFile movie(context);
    if (movie.extractMovie() == 0) {
        std::string movieframestr = std::to_string(movie.nbFramesConfig());
        movie_framecount->value(movieframestr.c_str());
        std::string rerecordstr = std::to_string(movie.nbRerecords());
        rerecord_count->value(rerecordstr.c_str());

        /* Also, by default, set the read-only mode */
        movie_read_only->set();
        context->config.sc.recording = SharedConfig::RECORDING_READ;
        context->config.sc_modified = true;
    }
    else {
        movie_framecount->value("0");
        rerecord_count->value("0");

        /* Also, by default, set the write mode */
        movie_read_only->clear();
        context->config.sc.recording = SharedConfig::RECORDING_WRITE;
        context->config.sc_modified = true;
    }

    /* Initial time */
    initial_time_sec = new Fl_Int_Input(10, 260, 100, 30, "Initial time (sec - nsec)");
    initial_time_sec->align(FL_ALIGN_TOP_LEFT);
    initial_time_sec->callback(initial_time_cb);

    initial_time_nsec = new Fl_Int_Input(130, 260, 100, 30, " - ");
    initial_time_nsec->align(FL_ALIGN_LEFT);
    initial_time_nsec->callback(initial_time_cb);

    launch = new Fl_Button(10, 450, 70, 40, "Start");
    launch->callback(launch_cb);

    launch_gdb = new Fl_Button(400, 450, 180, 40, "Start and attach gdb");
    launch_gdb->callback(launch_cb);

    pausecheck->value(!context->config.sc.running);
    fastforwardcheck->value(context->config.sc.fastforward);

    update_config();

    window->end();

#ifdef LIBTAS_ENABLE_AVDUMPING
    encode_window = new EncodeWindow(c);
#endif
    input_window = new InputWindow(c);
    executable_window = new ExecutableWindow(c);
    controller_window = new ControllerWindow(c);

    window->show();

    context->ui_window = fl_xid(window);
}

Fl_Menu_Item MainWindow::menu_items[] = {
    {"File", 0, nullptr, nullptr, FL_SUBMENU},
        {"Open Executable...", 0, browse_gamepath_cb},
        {"Executable Options...", 0, config_executable_cb},
        {"Open Movie...", 0, browse_moviepath_cb},
        {nullptr},
    {"Video", 0, nullptr, nullptr, FL_SUBMENU},
        {"Force software rendering", 0, render_soft_cb, nullptr, FL_MENU_TOGGLE},
        {"Add performance flags to software rendering", 0, nullptr, nullptr, FL_SUBMENU},
            {"texmem", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* minimize texture cache footprint */
            {"no_mipmap", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* MIP_FILTER_NONE always */
            {"no_linear", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* FILTER_NEAREST always */
            {"no_mip_linear", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* MIP_FILTER_LINEAR ==> _NEAREST */
            {"no_tex", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* sample white always */
            {"no_blend", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* disable blending */
            {"no_depth", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* disable depth buffering entirely */
            {"no_alphatest", 0, llvm_perf_cb, nullptr, FL_MENU_TOGGLE}, /* disable alpha testing */
            {nullptr},
#ifdef LIBTAS_ENABLE_HUD
        {"OSD", 0, nullptr, nullptr, FL_SUBMENU},
            {"Frame Count", 0, osd_frame_cb, nullptr, FL_MENU_TOGGLE},
            {"Inputs", 0, osd_inputs_cb, nullptr, FL_MENU_TOGGLE | FL_MENU_DIVIDER},
            {"OSD on video encode", 0, osd_encode_cb, nullptr, FL_MENU_TOGGLE},
            {nullptr},
#else
        {"OSD (disabled)", 0, nullptr, nullptr, FL_MENU_INACTIVE},
#endif
        {nullptr},
    {"Sound", 0, nullptr, nullptr, FL_SUBMENU},
        {"Format", 0, nullptr, nullptr, FL_SUBMENU},
            {"8000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(8000), FL_MENU_RADIO},
            {"11025 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(11025), FL_MENU_RADIO},
            {"12000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(12000), FL_MENU_RADIO},
            {"16000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(16000), FL_MENU_RADIO},
            {"22050 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(22050), FL_MENU_RADIO},
            {"24000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(24000), FL_MENU_RADIO},
            {"32000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(32000), FL_MENU_RADIO},
            {"44100 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(44100), FL_MENU_RADIO},
            {"48000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(48000), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"8 bit", 0, sound_bitdepth_cb, reinterpret_cast<void*>(8), FL_MENU_RADIO},
            {"16 bit", 0, sound_bitdepth_cb, reinterpret_cast<void*>(16), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"Mono", 0, sound_channel_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"Stereo", 0, sound_channel_cb, reinterpret_cast<void*>(2), FL_MENU_RADIO},
            {nullptr},
#ifdef LIBTAS_ENABLE_SOUNDPLAYBACK
        {"Mute Sound", 0, mute_sound_cb, nullptr, FL_MENU_TOGGLE},
#else
        {"Mute Sound (disabled)", 0, mute_sound_cb, nullptr, FL_MENU_TOGGLE | FL_MENU_INACTIVE},
#endif
        {nullptr},
    {"Runtime", 0, nullptr, nullptr, FL_SUBMENU},
        {"Time tracking", 0, nullptr, nullptr, FL_SUBMENU},
            {"Main thread", 0, nullptr, nullptr, FL_SUBMENU},
                {"time()", 0, time_main_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_TIME), FL_MENU_TOGGLE},
                {"gettimeofday()", 0, time_main_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_GETTIMEOFDAY), FL_MENU_TOGGLE},
                {"clock()", 0, time_main_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_CLOCK), FL_MENU_TOGGLE},
                {"clock_gettime()", 0, time_main_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_CLOCKGETTIME), FL_MENU_TOGGLE},
                {"SDL_GetTicks()", 0, time_main_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_SDLGETTICKS), FL_MENU_TOGGLE},
                {"SDL_GetPerformanceCounter()", 0, time_main_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER), FL_MENU_TOGGLE},
                {nullptr},
            {"Secondary threads", 0, nullptr, nullptr, FL_SUBMENU},
                {"time()", 0, time_sec_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_TIME), FL_MENU_TOGGLE},
                {"gettimeofday()", 0, time_sec_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_GETTIMEOFDAY), FL_MENU_TOGGLE},
                {"clock()", 0, time_sec_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_CLOCK), FL_MENU_TOGGLE},
                {"clock_gettime()", 0, time_sec_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_CLOCKGETTIME), FL_MENU_TOGGLE},
                {"SDL_GetTicks()", 0, time_sec_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_SDLGETTICKS), FL_MENU_TOGGLE},
                {"SDL_GetPerformanceCounter()", 0, time_sec_cb, reinterpret_cast<void*>(SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER), FL_MENU_TOGGLE},
                {nullptr},
            {nullptr},
        {"Savestates", 0, nullptr, nullptr, FL_SUBMENU},
            {"Ignore memory segments", 0, nullptr, nullptr, FL_SUBMENU},
                {"Ignore non-writeable segments", 0, ignore_memory_cb, reinterpret_cast<void*>(SharedConfig::IGNORE_NON_WRITEABLE), FL_MENU_TOGGLE},
                {"Ignore non-writeable non-anonymous segments", 0, ignore_memory_cb, reinterpret_cast<void*>(SharedConfig::IGNORE_NON_ANONYMOUS_NON_WRITEABLE), FL_MENU_TOGGLE},
                {"Ignore exec segments", 0, ignore_memory_cb, reinterpret_cast<void*>(SharedConfig::IGNORE_EXEC), FL_MENU_TOGGLE},
                {"Ignore shared segments", 0, ignore_memory_cb, reinterpret_cast<void*>(SharedConfig::IGNORE_SHARED), FL_MENU_TOGGLE},
                {nullptr},
            {"Save screen in savestates", 0, savestate_screen_cb, nullptr, FL_MENU_TOGGLE},
            {nullptr},
        {"Backup savefiles in memory", 0, prevent_savefiles_cb, nullptr, FL_MENU_TOGGLE},
        {"Debug Logging", 0, nullptr, nullptr, FL_SUBMENU},
            {"Disabled", 0, logging_status_cb, reinterpret_cast<void*>(SharedConfig::NO_LOGGING), FL_MENU_RADIO},
            {"Log to console", 0, logging_status_cb, reinterpret_cast<void*>(SharedConfig::LOGGING_TO_CONSOLE), FL_MENU_RADIO},
            {"Log to file", 0, logging_status_cb, reinterpret_cast<void*>(SharedConfig::LOGGING_TO_FILE), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"Print Categories", 0, nullptr, nullptr, FL_SUBMENU},
                {"Untested", 0, logging_print_cb, reinterpret_cast<void*>(LCF_UNTESTED), FL_MENU_TOGGLE},
                {"Desync", 0, logging_print_cb, reinterpret_cast<void*>(LCF_DESYNC), FL_MENU_TOGGLE},
                {"Frequent", 0, logging_print_cb, reinterpret_cast<void*>(LCF_FREQUENT), FL_MENU_TOGGLE},
                {"Error", 0, logging_print_cb, reinterpret_cast<void*>(LCF_ERROR), FL_MENU_TOGGLE},
                {"ToDo", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TODO), FL_MENU_TOGGLE},
                {"Frame", 0, logging_print_cb, reinterpret_cast<void*>(LCF_FRAME), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"Hook", 0, logging_print_cb, reinterpret_cast<void*>(LCF_HOOK), FL_MENU_TOGGLE},
                {"Time Set", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMESET), FL_MENU_TOGGLE},
                {"Time Get", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMEGET), FL_MENU_TOGGLE},
                {"Checkpoint", 0, logging_print_cb, reinterpret_cast<void*>(LCF_CHECKPOINT), FL_MENU_TOGGLE},
                {"Wait", 0, logging_print_cb, reinterpret_cast<void*>(LCF_WAIT), FL_MENU_TOGGLE},
                {"Sleep", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SLEEP), FL_MENU_TOGGLE},
                {"Socket", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SOCKET), FL_MENU_TOGGLE},
                {"OpenGL", 0, logging_print_cb, reinterpret_cast<void*>(LCF_OGL), FL_MENU_TOGGLE},
                {"AV Dumping", 0, logging_print_cb, reinterpret_cast<void*>(LCF_DUMP), FL_MENU_TOGGLE},
                {"SDL", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SDL), FL_MENU_TOGGLE},
                {"Memory", 0, logging_print_cb, reinterpret_cast<void*>(LCF_MEMORY), FL_MENU_TOGGLE},
                {"Keyboard", 0, logging_print_cb, reinterpret_cast<void*>(LCF_KEYBOARD), FL_MENU_TOGGLE},
                {"Mouse", 0, logging_print_cb, reinterpret_cast<void*>(LCF_MOUSE), FL_MENU_TOGGLE},
                {"Joystick", 0, logging_print_cb, reinterpret_cast<void*>(LCF_JOYSTICK), FL_MENU_TOGGLE},
                {"OpenAL", 0, logging_print_cb, reinterpret_cast<void*>(LCF_OPENAL), FL_MENU_TOGGLE},
                {"Sound", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SOUND), FL_MENU_TOGGLE},
                {"Random", 0, logging_print_cb, reinterpret_cast<void*>(LCF_RANDOM), FL_MENU_TOGGLE},
                {"Signals", 0, logging_print_cb, reinterpret_cast<void*>(LCF_SIGNAL), FL_MENU_TOGGLE},
                {"Events", 0, logging_print_cb, reinterpret_cast<void*>(LCF_EVENTS), FL_MENU_TOGGLE},
                {"Windows", 0, logging_print_cb, reinterpret_cast<void*>(LCF_WINDOW), FL_MENU_TOGGLE},
                {"File IO", 0, logging_print_cb, reinterpret_cast<void*>(LCF_FILEIO), FL_MENU_TOGGLE},
                {"Steam", 0, logging_print_cb, reinterpret_cast<void*>(LCF_STEAM), FL_MENU_TOGGLE},
                {"Threads", 0, logging_print_cb, reinterpret_cast<void*>(LCF_THREAD), FL_MENU_TOGGLE},
                {"Timers", 0, logging_print_cb, reinterpret_cast<void*>(LCF_TIMERS), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"All", 0, logging_print_cb, reinterpret_cast<void*>(LCF_ALL)},
                {"None", 0, logging_print_cb, reinterpret_cast<void*>(LCF_NONE)},
                {nullptr},
            {"Exclude Categories", 0, nullptr, nullptr, FL_SUBMENU},
                {"Untested", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_UNTESTED), FL_MENU_TOGGLE},
                {"Desync", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_DESYNC), FL_MENU_TOGGLE},
                {"Frequent", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_FREQUENT), FL_MENU_TOGGLE},
                {"Error", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_ERROR), FL_MENU_TOGGLE},
                {"ToDo", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TODO), FL_MENU_TOGGLE},
                {"Frame", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_FRAME), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"Hook", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_HOOK), FL_MENU_TOGGLE},
                {"Time Set", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMESET), FL_MENU_TOGGLE},
                {"Time Get", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMEGET), FL_MENU_TOGGLE},
                {"Checkpoint", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_CHECKPOINT), FL_MENU_TOGGLE},
                {"Wait", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_WAIT), FL_MENU_TOGGLE},
                {"Sleep", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SLEEP), FL_MENU_TOGGLE},
                {"Socket", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SOCKET), FL_MENU_TOGGLE},
                {"OpenGL", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_OGL), FL_MENU_TOGGLE},
                {"AV Dumping", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_DUMP), FL_MENU_TOGGLE},
                {"SDL", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SDL), FL_MENU_TOGGLE},
                {"Memory", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_MEMORY), FL_MENU_TOGGLE},
                {"Keyboard", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_KEYBOARD), FL_MENU_TOGGLE},
                {"Mouse", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_MOUSE), FL_MENU_TOGGLE},
                {"Joystick", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_JOYSTICK), FL_MENU_TOGGLE},
                {"OpenAL", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_OPENAL), FL_MENU_TOGGLE},
                {"Sound", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SOUND), FL_MENU_TOGGLE},
                {"Random", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_RANDOM), FL_MENU_TOGGLE},
                {"Signals", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_SIGNAL), FL_MENU_TOGGLE},
                {"Events", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_EVENTS), FL_MENU_TOGGLE},
                {"Windows", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_WINDOW), FL_MENU_TOGGLE},
                {"File IO", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_FILEIO), FL_MENU_TOGGLE},
                {"Steam", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_STEAM), FL_MENU_TOGGLE},
                {"Threads", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_THREAD), FL_MENU_TOGGLE},
                {"Timers", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_TIMERS), FL_MENU_TOGGLE | FL_MENU_DIVIDER},
                {"All", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_ALL)},
                {"None", 0, logging_exclude_cb, reinterpret_cast<void*>(LCF_NONE)},
                {nullptr},
            {nullptr},
        {nullptr},
    {"Tools", 0, nullptr, nullptr, FL_SUBMENU},
#ifdef LIBTAS_ENABLE_AVDUMPING
        {"Configure encode...", 0, config_encode_cb},
        {"Start encode", 0, toggle_encode_cb, nullptr, FL_MENU_DIVIDER},
#else
        {"Configure encode... (disabled)", 0, nullptr, nullptr, FL_MENU_INACTIVE},
        {"Start encode (disabled)", 0, nullptr, nullptr, FL_MENU_DIVIDER | FL_MENU_INACTIVE},
#endif
        {"Slow Motion", 0, nullptr, nullptr, FL_SUBMENU},
            {"100% (normal speed)", 0, slowmo_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"50%", 0, slowmo_cb, reinterpret_cast<void*>(2), FL_MENU_RADIO},
            {"25%", 0, slowmo_cb, reinterpret_cast<void*>(4), FL_MENU_RADIO},
            {"12%", 0, slowmo_cb, reinterpret_cast<void*>(8), FL_MENU_RADIO},
            {nullptr},
        {nullptr},
    {"Input", 0, nullptr, nullptr, FL_SUBMENU},
        {"Configure mapping...", 0, config_input_cb, nullptr, FL_MENU_DIVIDER},
        {"Keyboard support", 0, input_keyboard_cb, nullptr, FL_MENU_TOGGLE},
        {"Mouse support", 0, input_mouse_cb, nullptr, FL_MENU_TOGGLE},
        {"Joystick support", 0, nullptr, nullptr, FL_SUBMENU | FL_MENU_DIVIDER},
            {"None", 0, input_joy_cb, reinterpret_cast<void*>(0), FL_MENU_RADIO},
            {"1", 0, input_joy_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"2", 0, input_joy_cb, reinterpret_cast<void*>(2), FL_MENU_RADIO},
            {"3", 0, input_joy_cb, reinterpret_cast<void*>(3), FL_MENU_RADIO},
            {"4", 0, input_joy_cb, reinterpret_cast<void*>(4), FL_MENU_RADIO},
            {nullptr},
        {"Joystick inputs (dummy)", 0, controller_inputs_cb, nullptr, FL_MENU_DIVIDER},
        {"Enable hotkeys when", 0, nullptr, nullptr, FL_SUBMENU},
            {"Game has focus", 0, hotkeys_focus_cb, reinterpret_cast<void*>(Context::FOCUS_GAME), FL_MENU_TOGGLE},
            {"UI has focus", 0, hotkeys_focus_cb, reinterpret_cast<void*>(Context::FOCUS_UI), FL_MENU_TOGGLE},
            {"Always (not working)", 0, hotkeys_focus_cb, reinterpret_cast<void*>(Context::FOCUS_ALL), FL_MENU_TOGGLE},
            {nullptr},
        {"Enable inputs when", 0, nullptr, nullptr, FL_SUBMENU},
            {"Game has focus", 0, inputs_focus_cb, reinterpret_cast<void*>(Context::FOCUS_GAME), FL_MENU_TOGGLE},
            {"UI has focus", 0, inputs_focus_cb, reinterpret_cast<void*>(Context::FOCUS_UI), FL_MENU_TOGGLE},
            {"Always", 0, inputs_focus_cb, reinterpret_cast<void*>(Context::FOCUS_ALL), FL_MENU_TOGGLE},
            {nullptr},
        {nullptr},
    {nullptr}
};

void MainWindow::update_status()
{
    /* Update game status (active/inactive) */

    /* This function might be called by another thread */
    Fl::lock();

    Fl_Menu_Item *item;
    std::string tmpstr;

    switch (context->status) {
        case Context::INACTIVE:
            launch->label("Start");
            launch->activate();
            launch_gdb->label("Start and attach gdb");
            launch_gdb->activate();
            moviepath->activate();
            browsemoviepath->activate();
            gamepath->activate();
            browsegamepath->activate();
            cmdoptions->activate();
            logicalfps->activate();
            item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Sound/Format"));
            if (item) item->activate();
            tmpstr = std::to_string(context->config.sc.initial_time.tv_sec);
            initial_time_sec->value(tmpstr.c_str());
            initial_time_sec->activate();
            tmpstr = std::to_string(context->config.sc.initial_time.tv_nsec);
            initial_time_nsec->value(tmpstr.c_str());
            initial_time_nsec->activate();
#ifdef LIBTAS_ENABLE_AVDUMPING
            if (context->config.sc.av_dumping) {
                Fl_Menu_Item* encode_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(toggle_encode_cb));
                Fl_Menu_Item* config_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(config_encode_cb));
                context->config.sc.av_dumping = false;
                if (encode_item) encode_item->label("Start encode");
                if (config_item) config_item->activate();
            }
#endif
            movie_recording->activate();
            framecount->value("0");
            {
                movie_framecount->activate();
                MovieFile movie(context);
                /* Update the movie frame count if the movie file is valid */
                if (movie.extractMovie() == 0) {
                    std::string movieframestr = std::to_string(movie.nbFramesConfig());
                    movie_framecount->value(movieframestr.c_str());
                }
            }
            break;
        case Context::STARTING:
            launch->deactivate();
            launch_gdb->deactivate();
            moviepath->deactivate();
            browsemoviepath->deactivate();
            gamepath->deactivate();
            browsegamepath->deactivate();
            cmdoptions->deactivate();
            logicalfps->deactivate();
            item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Sound/Format"));
            if (item) item->deactivate();
            initial_time_sec->deactivate();
            initial_time_nsec->deactivate();
            if ((context->config.sc.recording == SharedConfig::NO_RECORDING) ||
                (context->config.sc.recording == SharedConfig::RECORDING_WRITE)) {
                movie_framecount->value("");
                movie_framecount->deactivate();
            }
            movie_recording->deactivate();
            break;
        case Context::ACTIVE:
            if (context->attach_gdb) {
                launch_gdb->activate();
                launch_gdb->label("Stop");
            }
            else {
                launch->activate();
                launch->label("Stop");
            }
            break;
        case Context::QUITTING:
            launch->deactivate();
            launch_gdb->deactivate();
            break;
        default:
            break;
    }

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update_ui()
{
    /* This function is called by another thread */
    Fl::lock();

    /* Update pause status */
    pausecheck->value(!context->config.sc.running);

    /* Update fastforward status */
    fastforwardcheck->value(context->config.sc.fastforward);

    /* Update recording state */
    std::string movieframestr;
    switch (context->config.sc.recording) {
        case SharedConfig::RECORDING_WRITE:
            movie_read_only->clear();
            movie_framecount->value("");
            movie_framecount->deactivate();
            break;
        case SharedConfig::RECORDING_READ:
            movie_read_only->set();
            movieframestr = std::to_string(context->config.sc.movie_framecount);
            movie_framecount->value(movieframestr.c_str());
            movie_framecount->activate();
            break;
        default:
            break;
    }

    /* Update encode menus */
#ifdef LIBTAS_ENABLE_AVDUMPING
    Fl_Menu_Item* encode_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(toggle_encode_cb));
    Fl_Menu_Item* config_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(config_encode_cb));
    if (context->config.sc.av_dumping) {
        if (encode_item) encode_item->label("Stop encode");
        if (config_item) config_item->deactivate();
    }
    else {
        if (encode_item) encode_item->label("Start encode");
        if (config_item) config_item->activate();
    }
#endif

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update_framecount_time()
{
    /* This function is called by another thread */
    Fl::lock();

    /* Update frame count */
    std::string framestr = std::to_string(context->framecount);
    framecount->value(framestr.c_str());

    /* Update time */
    std::string secstr = std::to_string(context->current_time.tv_sec);
    initial_time_sec->value(secstr.c_str());
    std::string nsecstr = std::to_string(context->current_time.tv_nsec);
    initial_time_nsec->value(nsecstr.c_str());

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update_rerecordcount()
{
    /* This function is called by another thread */
    Fl::lock();

    /* Update frame count */
    std::string rerecordstr = std::to_string(context->rerecord_count);
    rerecord_count->value(rerecordstr.c_str());

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update_config()
{
    gamepath->value(context->gamepath.c_str());
    gamepathchooser->preset_file(context->gamepath.c_str());
    cmdoptions->value(context->config.gameargs.c_str());
    moviepath->value(context->config.moviefile.c_str());
    moviepathchooser->preset_file(context->config.moviefile.c_str());
    std::string fpsstr = std::to_string(context->config.sc.framerate);
    logicalfps->value(fpsstr.c_str());

    /* Define some macros to help setting menu items */
    Fl_Menu_Item* menu_item;

#define SET_TOGGLE_FROM_BOOL(cb, value) \
    menu_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(cb)); \
    if (value) \
        menu_item->set(); \
    else \
        menu_item->clear()

#define SET_TOGGLES_FROM_MASK(cb, value) \
    menu_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(cb)); \
    do { \
        if ((value) & menu_item->argument()) \
            menu_item->set(); \
        else \
            menu_item->clear(); \
      menu_item = menu_item->next(); \
    } while (menu_item->flags)

#define SET_RADIO_FROM_LIST(cb, value) \
    menu_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(cb)); \
    do { \
        if ((value) == menu_item->argument()) { \
            menu_item->setonly(); \
            break; \
        } \
        menu_item = menu_item->next(); \
    } while (menu_item->flags)

    SET_RADIO_FROM_LIST(sound_frequency_cb, context->config.sc.audio_frequency);
    SET_RADIO_FROM_LIST(sound_bitdepth_cb, context->config.sc.audio_bitdepth);
    SET_RADIO_FROM_LIST(sound_channel_cb, context->config.sc.audio_channels);

    menu_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(mute_sound_cb));
    if (context->config.sc.audio_mute) {
        menu_item->set();
        mutecheck->set();
    }
    else {
        menu_item->clear();
        mutecheck->clear();
    }

    SET_RADIO_FROM_LIST(logging_status_cb, context->config.sc.logging_status);

    SET_TOGGLES_FROM_MASK(logging_print_cb, context->config.sc.includeFlags);
    SET_TOGGLES_FROM_MASK(logging_exclude_cb, context->config.sc.excludeFlags);

    SET_RADIO_FROM_LIST(slowmo_cb, context->config.sc.speed_divisor);

    SET_TOGGLE_FROM_BOOL(input_keyboard_cb, context->config.sc.keyboard_support);
    SET_TOGGLE_FROM_BOOL(input_mouse_cb, context->config.sc.mouse_support);

    SET_RADIO_FROM_LIST(input_joy_cb, context->config.sc.numControllers);

#ifdef LIBTAS_ENABLE_HUD
    SET_TOGGLE_FROM_BOOL(osd_frame_cb, context->config.sc.hud_framecount);
    SET_TOGGLE_FROM_BOOL(osd_inputs_cb, context->config.sc.hud_inputs);
    SET_TOGGLE_FROM_BOOL(osd_encode_cb, context->config.sc.hud_encode);
#endif

    menu_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(time_main_cb));
    while (menu_item->flags) {
        if (context->config.sc.main_gettimes_threshold[menu_item->argument()] == -1)
            menu_item->clear();
        else
            menu_item->set();
        menu_item = menu_item->next();
    }

    menu_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item(time_sec_cb));
    while (menu_item->flags) {
        if (context->config.sc.sec_gettimes_threshold[menu_item->argument()] == -1)
            menu_item->clear();
        else
            menu_item->set();
      menu_item = menu_item->next();
    }

    SET_TOGGLES_FROM_MASK(hotkeys_focus_cb, context->hotkeys_focus);
    SET_TOGGLES_FROM_MASK(inputs_focus_cb, context->inputs_focus);

    SET_TOGGLE_FROM_BOOL(render_soft_cb, context->config.opengl_soft);
    SET_TOGGLE_FROM_BOOL(savestate_screen_cb, context->config.sc.save_screenpixels);

    SET_TOGGLES_FROM_MASK(ignore_memory_cb, context->config.sc.ignore_sections);

    std::string secstr = std::to_string(context->config.sc.initial_time.tv_sec);
    initial_time_sec->value(secstr.c_str());

    std::string nsecstr = std::to_string(context->config.sc.initial_time.tv_nsec);
    initial_time_nsec->value(nsecstr.c_str());

    SET_TOGGLE_FROM_BOOL(prevent_savefiles_cb, context->config.sc.prevent_savefiles);
}

void launch_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.context->attach_gdb = (static_cast<Fl_Button*>(w) == mw.launch_gdb);

    switch (mw.context->status) {
        case Context::INACTIVE:
            /* Prompt a confirmation message if overwriting a movie file */
            if (mw.context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
                struct stat sb;
                if (stat(mw.context->config.moviefile.c_str(), &sb) == 0) {
                    int choice = fl_choice("The movie file %s does exist. Do you want to overwrite it?", "Yes", "No", 0, mw.context->config.moviefile.c_str());
                    if (choice == 1)
                        break;
                }
            }

            /* Check that there might be a thread from a previous game execution */
            if (mw.game_thread.joinable())
                mw.game_thread.join();

            /* Start game */
            mw.context->status = Context::STARTING;
            mw.update_status();
            mw.game_thread = std::thread{launchGame, mw.context};
            break;
        case Context::ACTIVE:
            mw.context->status = Context::QUITTING;
            mw.context->config.sc.running = true;
            mw.context->config.sc_modified = true;
            mw.update_ui();
            mw.update_status();
            mw.game_thread.detach();
            break;
        case Context::RESTARTING:
            mw.game_thread.join();
            mw.context->status = Context::STARTING;
            mw.update_status();
            mw.game_thread = std::thread{launchGame, mw.context};
            break;
        default:
            break;
    }
}

void browse_gamepath_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    int ret = mw.gamepathchooser->show();

    const char* filename = mw.gamepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        mw.gamepath->value(filename);
        mw.context->gamepath = std::string(filename);

        /* Try to load the game-specific pref file */
        mw.context->config.load(mw.context->gamepath);

        /* Update the UI accordingly */
        mw.update_config();
#ifdef LIBTAS_ENABLE_AVDUMPING
        mw.encode_window->update_config();
#endif
        mw.executable_window->update_config();
        mw.input_window->update();
    }
}

void browse_moviepath_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    int ret = mw.moviepathchooser->show();

    const char* filename = mw.moviepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        mw.moviepath->value(filename);
        mw.context->config.moviefile = std::string(filename);
        MovieFile movie(mw.context);
        if (movie.extractMovie() == 0) {
            /* If the moviefile is valid, update the frame and rerecord counts */
            std::string movieframestr = std::to_string(movie.nbFramesConfig());
            mw.movie_framecount->value(movieframestr.c_str());
            std::string rerecordstr = std::to_string(movie.nbRerecords());
            mw.rerecord_count->value(rerecordstr.c_str());

            /* Also, by default, set the read-only mode */
            mw.movie_read_only->set();
            mw.context->config.sc.recording = SharedConfig::RECORDING_READ;
            mw.context->config.sc_modified = true;
        }
        else {
            mw.movie_framecount->value("0");
            mw.rerecord_count->value("0");

            /* Also, by default, set the write mode */
            mw.movie_read_only->clear();
            mw.context->config.sc.recording = SharedConfig::RECORDING_WRITE;
            mw.context->config.sc_modified = true;
        }
    }
}

void set_fps_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Int_Input *ii = (Fl_Int_Input*) w;
    std::string fpsstr = ii->value();
    mw.context->config.sc.framerate = std::stoi(fpsstr);
}

void pause_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    if (mw.context->status == Context::INACTIVE) {
        /* If the game is inactive, set the value directly */
        Fl_Check_Button *cb = (Fl_Check_Button*) w;
        int cb_val = static_cast<int>(cb->value());
        mw.context->config.sc.running = !cb_val;
    }
    else {
        /* Else, let the game thread set the value */
        mw.context->hotkey_queue.push(HOTKEY_PLAYPAUSE);
    }
}

void fastforward_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Check_Button *cb = (Fl_Check_Button*) w;
    int cb_val = static_cast<int>(cb->value());
    mw.context->config.sc.fastforward = cb_val;
    mw.context->config.sc_modified = true;
}

void movie_recording_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    if (mw.movie_recording->value() == 1) {
        if (mw.movie_read_only->value() == 1) {
            mw.context->config.sc.recording = SharedConfig::RECORDING_READ;
        }
        else {
            mw.context->config.sc.recording = SharedConfig::RECORDING_WRITE;
        }

        /* Enable the other movie UI elements */
        mw.movie_read_only->activate();
        mw.moviepath->activate();
        mw.browsemoviepath->activate();
    }
    else {
        mw.context->config.sc.recording = SharedConfig::NO_RECORDING;

        /* Disable the other movie UI elements */
        mw.movie_read_only->deactivate();
        mw.moviepath->deactivate();
        mw.browsemoviepath->deactivate();
    }
    mw.context->config.sc_modified = true;
}

void movie_toggle_rw_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    /* If the game is running, we let the main thread deal with movie toggling.
     * Else, we set the recording mode.
     */
    if (mw.context->status == Context::INACTIVE) {
        if (mw.movie_read_only->value() == 1) {
            mw.context->config.sc.recording = SharedConfig::RECORDING_READ;
        }
        else {
            mw.context->config.sc.recording = SharedConfig::RECORDING_WRITE;
        }
    }
    else {
        mw.context->hotkey_queue.push(HOTKEY_READWRITE);
    }
}

#ifdef LIBTAS_ENABLE_AVDUMPING
void config_encode_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.encode_window->update_config();
    mw.encode_window->window->show();

    while (mw.encode_window->window->shown()) {
        Fl::wait();
    }
}
#endif

void config_executable_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.executable_window->update_config();
    mw.executable_window->window->show();

    while (mw.executable_window->window->shown()) {
        Fl::wait();
    }
}

#ifdef LIBTAS_ENABLE_AVDUMPING
void toggle_encode_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();

    /* Prompt a confirmation message for overwriting an encode file */
    if (!mw.context->config.sc.av_dumping) {
        struct stat sb;
        if (stat(mw.context->config.dumpfile.c_str(), &sb) == 0) {
            /* Pause the game during the choice */
            mw.context->config.sc.running = false;
            mw.context->config.sc_modified = true;

            int choice = fl_choice("The encode file %s does exist. Do you want to overwrite it?", "Yes", "No", 0, mw.context->config.dumpfile.c_str());
            if (choice == 1)
                return;
        }
    }

    /* TODO: Using directly the hotkey does not check for existing file */
    mw.context->hotkey_queue.push(HOTKEY_TOGGLE_ENCODE);

}
#endif

void config_input_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.input_window->update();
    mw.input_window->window->show();

    while (mw.input_window->window->shown()) {
        Fl::wait();
    }
}

void controller_inputs_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    // mw.controller_window->update();
    mw.controller_window->window->show();

    while (mw.controller_window->window->shown()) {
        Fl::wait();
    }
}


void sound_frequency_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int frequency = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.audio_frequency = frequency;
}

void sound_bitdepth_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int bitdepth = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.audio_bitdepth = bitdepth;
}

void sound_channel_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int channel = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.audio_channels = channel;
}

void mute_sound_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* mute_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(mute_sound_cb));

    if (mw.context->config.sc.audio_mute) {
        if (mute_item) mute_item->clear();
        mw.context->config.sc.audio_mute = false;
        mw.context->config.sc_modified = true;
    }
    else {
        if (mute_item) mute_item->set();
        mw.context->config.sc.audio_mute = true;
        mw.context->config.sc_modified = true;
    }
    mw.mutecheck->value(mw.context->config.sc.audio_mute);
}

void logging_status_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    SharedConfig::LogStatus logstatus = static_cast<SharedConfig::LogStatus>(reinterpret_cast<intptr_t>(v));
    mw.context->config.sc.logging_status = logstatus;
}

void logging_print_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    LogCategoryFlag logcat = static_cast<LogCategoryFlag>(reinterpret_cast<intptr_t>(v));
    if (logcat == LCF_ALL) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_print_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->set();
            log_item = log_item->next();
        }
        mw.context->config.sc.includeFlags = LCF_ALL;
    }
    else if (logcat == LCF_NONE) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_print_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->clear();
            log_item = log_item->next();
        }
        mw.context->config.sc.includeFlags = LCF_NONE;
    }
    else {
        mw.context->config.sc.includeFlags ^= logcat;
    }
    mw.context->config.sc_modified = true;
}

void logging_exclude_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    LogCategoryFlag logcat = static_cast<LogCategoryFlag>(reinterpret_cast<intptr_t>(v));
    if (logcat == LCF_ALL) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_exclude_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->set();
            log_item = log_item->next();
        }
        mw.context->config.sc.excludeFlags = LCF_ALL;
    }
    else if (logcat == LCF_NONE) {
        /* Get the first item of the log categories */
        Fl_Menu_Item* log_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(logging_exclude_cb));
        while (log_item->argument() != LCF_ALL) {
            log_item->clear();
            log_item = log_item->next();
        }
        mw.context->config.sc.excludeFlags = LCF_NONE;
    }
    else {
        mw.context->config.sc.excludeFlags ^= logcat;
    }
    mw.context->config.sc_modified = true;
}

void input_keyboard_cb(Fl_Widget*, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* keyboard_item = mw.menu_bar->mvalue();

    if (keyboard_item && (keyboard_item->value() == 0)) {
        mw.context->config.sc.keyboard_support = false;
    }
    else {
        mw.context->config.sc.keyboard_support = true;
    }
    mw.context->config.sc_modified = true;
}

void input_mouse_cb(Fl_Widget*, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* mouse_item = mw.menu_bar->mvalue();

    if (mouse_item && (mouse_item->value() == 0)) {
        mw.context->config.sc.mouse_support = false;
    }
    else {
        mw.context->config.sc.mouse_support = true;
    }
    mw.context->config.sc_modified = true;
}

void input_joy_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int nb_joy = static_cast<int>(reinterpret_cast<intptr_t>(v));

    mw.context->config.sc.numControllers = nb_joy;
    mw.context->config.sc_modified = true;
}

void hotkeys_focus_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int focus_state = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->hotkeys_focus ^= focus_state;

    switch (focus_state) {
    case Context::FOCUS_GAME:
        /* If the game was not launched, don't do anything */
        if (! mw.context->game_window ) return;

        if (mw.menu_bar->mvalue()->value())
            XSelectInput(mw.context->display, mw.context->game_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
        else
            XSelectInput(mw.context->display, mw.context->game_window, FocusChangeMask);
        break;

    case Context::FOCUS_UI:
        if (mw.menu_bar->mvalue()->value())
            XSelectInput(mw.context->display, mw.context->ui_window, KeyPressMask | KeyReleaseMask | FocusChangeMask);
        else
            XSelectInput(mw.context->display, mw.context->ui_window, FocusChangeMask);
        break;

    case Context::FOCUS_ALL:
        /* TODO */
        break;

    default:
        break;
    }
}

void inputs_focus_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int focus_state = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->inputs_focus ^= focus_state;
}

void slowmo_cb(Fl_Widget*, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int spdiv = static_cast<int>(reinterpret_cast<intptr_t>(v));

    mw.context->config.sc.speed_divisor = spdiv;
    mw.context->config.sc_modified = true;
}

#ifdef LIBTAS_ENABLE_HUD
void osd_frame_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* osd_item = mw.menu_bar->mvalue();

    mw.context->config.sc.hud_framecount = osd_item->value();
    mw.context->config.sc_modified = true;
}

void osd_inputs_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* osd_item = mw.menu_bar->mvalue();

    mw.context->config.sc.hud_inputs = osd_item->value();
    mw.context->config.sc_modified = true;
}

void osd_encode_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* osd_item = mw.menu_bar->mvalue();

    mw.context->config.sc.hud_encode = osd_item->value();
    mw.context->config.sc_modified = true;
}
#endif

void time_main_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int timetype = static_cast<int>(reinterpret_cast<intptr_t>(v));
    const Fl_Menu_Item* timetype_item = mw.menu_bar->mvalue();

    if (timetype_item->value())
        mw.context->config.sc.main_gettimes_threshold[timetype] = 100; // TODO: Arbitrary
    else
        mw.context->config.sc.main_gettimes_threshold[timetype] = -1;
    mw.context->config.sc_modified = true;
}

void time_sec_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int timetype = static_cast<int>(reinterpret_cast<intptr_t>(v));
    const Fl_Menu_Item* timetype_item = mw.menu_bar->mvalue();

    if (timetype_item->value())
        mw.context->config.sc.sec_gettimes_threshold[timetype] = 1000; // TODO: Arbitrary
    else
        mw.context->config.sc.sec_gettimes_threshold[timetype] = -1;
    mw.context->config.sc_modified = true;
}

void render_soft_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* menu_item = mw.menu_bar->mvalue();

    mw.context->config.opengl_soft = menu_item->value();
}

void savestate_screen_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    const Fl_Menu_Item* menu_item = mw.menu_bar->mvalue();

    mw.context->config.sc.save_screenpixels = menu_item->value();
    mw.context->config.sc_modified = true;
}

void cmdoptions_cb(Fl_Widget*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.context->config.gameargs = mw.cmdoptions->value();
}

void llvm_perf_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();

    mw.context->config.llvm_perf = "";

    Fl_Menu_Item* menu_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(llvm_perf_cb));
    for (;menu_item->flags;menu_item = menu_item->next()) {
        if (menu_item->value()) {
            mw.context->config.llvm_perf += menu_item->label();
            mw.context->config.llvm_perf.push_back(',');
        }
    }
    /* Remove the trailing comma */
    mw.context->config.llvm_perf.pop_back();
}

void ignore_memory_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int ignore = static_cast<int>(reinterpret_cast<intptr_t>(v));

    mw.context->config.sc.ignore_sections ^= ignore;
    mw.context->config.sc_modified = true;
}

void initial_time_cb(Fl_Widget*)
{
    MainWindow& mw = MainWindow::getInstance();
    std::string secstr = mw.initial_time_sec->value();
    mw.context->config.sc.initial_time.tv_sec = std::stoi(secstr);

    std::string nsecstr = mw.initial_time_nsec->value();
    mw.context->config.sc.initial_time.tv_nsec = std::stoi(nsecstr);
}

void prevent_savefiles_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* menu_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item(prevent_savefiles_cb));

    mw.context->config.sc.prevent_savefiles = menu_item->value();
    mw.context->config.sc_modified = true;
}

void alert_dialog(void* alert_msg)
{
    MainWindow& mw = MainWindow::getInstance();

    /* Pause the game */
    mw.context->config.sc.running = false;
    mw.context->config.sc_modified = true;

    /* Bring FLTK to foreground
     * taken from https://stackoverflow.com/a/28404920
     */
    XEvent event = {0};
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(mw.context->display, "_NET_ACTIVE_WINDOW", False);
    event.xclient.window = mw.context->ui_window;
    event.xclient.format = 32;

    XSendEvent(mw.context->display, DefaultRootWindow(mw.context->display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XMapRaised(mw.context->display, mw.context->ui_window);

    /* Show alert window */
    std::string* alert_str = static_cast<std::string*>(alert_msg);
    fl_alert("%s", alert_str->c_str());
    free(alert_str);
}
