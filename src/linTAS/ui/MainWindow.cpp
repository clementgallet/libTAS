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
#include "../main.h"
#include <iostream>

void MainWindow::build(Context* c)
{
    context = c;
    window = new Fl_Double_Window(600, 400);

    /* Menu */
    menu_bar = new Fl_Menu_Bar(0, 0, window->w(), 30);
    menu_bar->menu(menu_items);

    /* Game Executable */
    gamepath = new Fl_Output(10, 300, 500, 30, "Game Executable");
    gamepath->align(FL_ALIGN_TOP_LEFT);
    gamepath->color(FL_LIGHT1);
    gamepath->value(context->gamepath.c_str());

    browsegamepath = new Fl_Button(520, 300, 70, 30, "Browse...");
    browsegamepath->callback(browse_gamepath_cb);

    gamepathchooser = new Fl_Native_File_Chooser();
    gamepathchooser->title("Game path");
    gamepathchooser->preset_file(context->gamepath.c_str());

    /* Movie File */
    moviepath = new Fl_Output(10, 50, 500, 30, "Movie File");
    moviepath->align(FL_ALIGN_TOP_LEFT);
    moviepath->color(FL_LIGHT1);
    moviepath->value(context->moviefile.c_str());

    moviepathchooser = new Fl_Native_File_Chooser();
    moviepathchooser->title("Choose a movie file");
    moviepathchooser->filter("libTAS movie file \t*.ltm\n");
    moviepathchooser->preset_file(context->moviefile.c_str());

    browsemoviepath = new Fl_Button(520, 50, 70, 30, "Browse...");
    browsemoviepath->callback(browse_moviepath_cb);

    /* Movie File Status */
    moviepack = new Fl_Pack(10, 90, window->w()-10, 30);
    moviepack->type(Fl_Pack::HORIZONTAL);
    moviepack->box(FL_ENGRAVED_FRAME);
    movie_norec = new Fl_Radio_Round_Button(0, 0, 130, 0, "No recording");
    movie_norec->callback(recording_cb);
    movie_w = new Fl_Radio_Round_Button(0, 0, 130, 0, "Overwrite");
    movie_w->callback(recording_cb);
    movie_rw = new Fl_Radio_Round_Button(0, 0, 130, 0, "Read/Write");
    movie_rw->callback(recording_cb);
    movie_ro = new Fl_Radio_Round_Button(0, 0, 130, 0, "Read Only");
    movie_ro->callback(recording_cb);
    moviepack->end();

    /* Frames per second */
    logicalfps = new Fl_Int_Input(160, 200, 40, 30, "Frames per second");
    std::string fpsstr = std::to_string(context->tasflags.framerate);
    logicalfps->value(fpsstr.c_str());
    logicalfps->callback(set_fps_cb);

    /* Pause/FF */
    pausecheck = new Fl_Check_Button(240, 140, 80, 20, "Pause");
    pausecheck->callback(pause_cb);
    fastforwardcheck = new Fl_Check_Button(240, 180, 80, 20, "Fast-forward");
    fastforwardcheck->callback(fastforward_cb);

    /* Mute */
    mutecheck = new Fl_Check_Button(240, 220, 80, 20, "Mute");
    mutecheck->callback(mute_sound_cb);

    /* Frame count */
    framecount = new Fl_Output(80, 140, 60, 30, "Frames:");
    std::string framestr = std::to_string(context->framecount);
    framecount->value(framestr.c_str());
    framecount->color(FL_LIGHT1);

    launch = new Fl_Button(10, 350, 70, 40, "Start");
    launch->callback(launch_cb);

    update(true);

    window->end();

    encode_window = new EncodeWindow(c);
    input_window = new InputWindow(c);

    window->show();
}

Fl_Menu_Item MainWindow::menu_items[] = {
    {"File", 0, nullptr, nullptr, FL_SUBMENU},
        {"Open Executable...", 0, browse_gamepath_cb},
        {"Open Movie...", 0, browse_moviepath_cb},
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
            {"44100 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(44100), FL_MENU_VALUE | FL_MENU_RADIO},
            {"48000 Hz", 0, sound_frequency_cb, reinterpret_cast<void*>(48000), FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"8 bit", 0, sound_bitdepth_cb, reinterpret_cast<void*>(8), FL_MENU_RADIO},
            {"16 bit", 0, sound_bitdepth_cb, reinterpret_cast<void*>(16), FL_MENU_VALUE | FL_MENU_RADIO | FL_MENU_DIVIDER},
            {"Mono", 0, sound_channel_cb, reinterpret_cast<void*>(1), FL_MENU_RADIO},
            {"Stereo", 0, sound_channel_cb, reinterpret_cast<void*>(2), FL_MENU_VALUE | FL_MENU_RADIO},
            {nullptr},
        {"Mute Sound", 0, mute_sound_cb, nullptr, FL_MENU_TOGGLE},
        {nullptr},
    {"Tools", 0, nullptr, nullptr, FL_SUBMENU},
        {"Configure encode...", 0, config_encode_cb},
        {"Start encode", 0, toggle_encode_cb},
        {nullptr},
    {"Input", 0, nullptr, nullptr, FL_SUBMENU},
        {"Configure mapping...", 0, config_input_cb},
        {nullptr},
    {nullptr}
};

void MainWindow::update_status()
{
    /* Update game status (active/inactive) */

    /* This function might be called by another thread */
    Fl::lock();

    Fl_Menu_Item *item;

    switch (context->status) {
        case Context::INACTIVE:
            launch->label("Start");
            launch->activate();
            moviepath->activate();
            browsemoviepath->activate();
            gamepath->activate();
            browsegamepath->activate();
            logicalfps->activate();
            moviepack->activate();
            item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Sound/Format"));
            if (item) item->activate();
            if (context->tasflags.av_dumping) {
                Fl_Menu_Item* encode_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Tools/Stop encode"));
                Fl_Menu_Item* config_item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Tools/Configure encode..."));
                context->tasflags.av_dumping = 0;
                if (encode_item) encode_item->label("Start encode");
                if (config_item) config_item->activate();
            }
            break;
        case Context::STARTING:
            launch->deactivate();
            moviepath->deactivate();
            browsemoviepath->deactivate();
            gamepath->deactivate();
            browsegamepath->deactivate();
            logicalfps->deactivate();
            moviepack->deactivate();
            item = const_cast<Fl_Menu_Item*>(menu_bar->find_item("Sound/Format"));
            if (item) item->deactivate();
            break;
        case Context::ACTIVE:
            launch->activate();
            launch->label("Stop");
            break;
        case Context::QUITTING:
            launch->deactivate();
            break;
        default:
            break;
    }

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update(bool status)
{
    /* This function is called by another thread */
    Fl::lock();

    /* Update frame count */
    std::string framestr = std::to_string(context->framecount);
    framecount->value(framestr.c_str());

    if (status) {
        /* Update pause status */
        pausecheck->value(!context->tasflags.running);

        /* Update fastforward status */
        fastforwardcheck->value(context->tasflags.fastforward);

        /* Update recording state */
        switch (context->tasflags.recording) {
          case TasFlags::NO_RECORDING:
              movie_norec->setonly();
              break;
          case TasFlags::RECORDING_WRITE:
              movie_w->setonly();
              break;
          case TasFlags::RECORDING_READ_WRITE:
              movie_rw->setonly();
              break;
          case TasFlags::RECORDING_READ_ONLY:
              movie_ro->setonly();
              break;
        }

    }


    Fl::unlock();
    Fl::awake();
}

void launch_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();

    switch (mw.context->status) {
        case Context::INACTIVE:
            /* Check that there might be a thread from a previous game execution */
            if (mw.game_thread.joinable())
                mw.game_thread.join();

            /* Start game */
            mw.context->status = Context::STARTING;
            mw.update_status();
            mw.game_thread = std::thread{launchGame, nullptr};
            break;
        case Context::ACTIVE:
            mw.context->status = Context::QUITTING;
            mw.update_status();
            mw.game_thread.detach();
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

        /* Change the movie file also */
        mw.context->moviefile = mw.context->gamepath;
        mw.context->moviefile += ".ltm";
        mw.moviepath->value(mw.context->moviefile.c_str());
    }
}

void browse_moviepath_cb(Fl_Widget* w, void*)
{
    // TODO: Almost duplicate of browse_gamepath_cb...
    MainWindow& mw = MainWindow::getInstance();
    int ret = mw.moviepathchooser->show();

    const char* filename = mw.moviepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        mw.moviepath->value(filename);
        mw.context->moviefile = std::string(filename);
    }
}

void set_fps_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Int_Input *ii = (Fl_Int_Input*) w;
    std::string fpsstr = ii->value();
    mw.context->tasflags.framerate = std::stoi(fpsstr);
}

void pause_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Check_Button *cb = (Fl_Check_Button*) w;
    int cb_val = static_cast<int>(cb->value());
    mw.context->tasflags.running = !cb_val;
    mw.context->tasflags_modified = true;
}

void fastforward_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Check_Button *cb = (Fl_Check_Button*) w;
    int cb_val = static_cast<int>(cb->value());
    mw.context->tasflags.fastforward = cb_val;
    mw.context->tasflags_modified = true;
}

void recording_cb(Fl_Widget* w)
{
    MainWindow& mw = MainWindow::getInstance();
    if (mw.movie_norec->value() == 1)
        mw.context->tasflags.recording = TasFlags::NO_RECORDING;
    if (mw.movie_w->value() == 1)
        mw.context->tasflags.recording = TasFlags::RECORDING_WRITE;
    if (mw.movie_rw->value() == 1)
        mw.context->tasflags.recording = TasFlags::RECORDING_READ_WRITE;
    if (mw.movie_ro->value() == 1)
        mw.context->tasflags.recording = TasFlags::RECORDING_READ_ONLY;
}

void config_encode_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.encode_window->window->show();

    while (mw.encode_window->window->shown()) {
        Fl::wait();
    }
}

void toggle_encode_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    /* Set encode status */
    Fl_Menu_Item* encode_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->mvalue());
    Fl_Menu_Item* config_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item("Tools/Configure encode..."));

    if (mw.context->tasflags.av_dumping == 0) {
        mw.context->tasflags.av_dumping = 1;
        mw.context->tasflags_modified = true;
        mw.context->dumpfile_modified = true;
        encode_item->label("Stop encode");
        if (config_item) config_item->deactivate();
    }
    else {
        mw.context->tasflags.av_dumping = 0;
        mw.context->tasflags_modified = true;
        encode_item->label("Start encode");
        if (config_item) config_item->activate();
    }
}

void config_input_cb(Fl_Widget* w, void*)
{
    MainWindow& mw = MainWindow::getInstance();
    mw.input_window->window->show();

    while (mw.input_window->window->shown()) {
        Fl::wait();
    }
}

void sound_frequency_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int frequency = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.audio_frequency = frequency;
}

void sound_bitdepth_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int bitdepth = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.audio_bitdepth = bitdepth;
}

void sound_channel_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    int channel = static_cast<int>(reinterpret_cast<intptr_t>(v));
    mw.context->config.audio_channels = channel;
}

void mute_sound_cb(Fl_Widget* w, void* v)
{
    MainWindow& mw = MainWindow::getInstance();
    Fl_Menu_Item* mute_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item("Sound/Mute Sound"));

    if (mw.context->config.audio_mute) {
        if (mute_item) mute_item->clear();
        mw.context->config.audio_mute = false;
        mw.context->config_modified = true;
    }
    else {
        if (mute_item) mute_item->set();
        mw.context->config.audio_mute = true;
        mw.context->config_modified = true;
    }
    mw.mutecheck->value(mw.context->config.audio_mute);
}
