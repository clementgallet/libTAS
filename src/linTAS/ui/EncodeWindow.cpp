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

#ifdef LIBTAS_ENABLE_AVDUMPING

#include "EncodeWindow.h"
#include "MainWindow.h"

#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libavutil/opt.h>
//#include <libavutil/imgutils.h>
//#include <libswscale/swscale.h>
//#include <libswresample/swresample.h>
}

static Fl_Callback start_cb;
static Fl_Callback cancel_cb;
static Fl_Callback browse_encodepath_cb;
static Fl_Callback vcodec_cb;
static Fl_Callback acodec_cb;

EncodeWindow::EncodeWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(600, 260, "Encoding configuration");

    /* Game Executable */
    encodepath = new Fl_Output(10, 30, 500, 30, "Encode file path");
    encodepath->align(FL_ALIGN_TOP_LEFT);
    encodepath->color(FL_LIGHT1);

    browseencodepath = new Fl_Button(520, 30, 70, 30, "Browse...");
    browseencodepath->callback(browse_encodepath_cb, this);

    encodepathchooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    encodepathchooser->title("Choose an encode filename");
    encodepathchooser->options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);

    videochoice = new Fl_Choice(10, 100, 450, 30, "Video codec");
    videochoice->align(FL_ALIGN_TOP_LEFT);
    videochoice->callback(vcodec_cb, this);

    videobitrate = new Fl_Input(480, 100, 100, 30, "Video bitrate");
    videobitrate->align(FL_ALIGN_TOP_LEFT);

    audiochoice = new Fl_Choice(10, 160, 450, 30, "Audio codec");
    audiochoice->align(FL_ALIGN_TOP_LEFT);
    audiochoice->callback(acodec_cb, this);

    audiobitrate = new Fl_Input(480, 160, 100, 30, "Audio bitrate");
    audiobitrate->align(FL_ALIGN_TOP_LEFT);

    start = new Fl_Button(400, 220, 70, 30, "Ok");
    start->callback(start_cb, this);

    cancel = new Fl_Button(500, 220, 70, 30, "Cancel");
    cancel->callback(cancel_cb, this);

    /* Get all encoding codecs available and fill the codec menus */

    /* Initialize libavcodec, and register all codecs and formats */
    //av_register_all();
    avcodec_register_all();

    /* Enumerate the codecs */
    AVCodec* codec = av_codec_next(nullptr);

    while(codec != nullptr)
    {
        if (av_codec_is_encoder(codec)) {
            /* Codec supports encoding */

            /* Build codec name */
            std::string codecstr = codec->long_name?codec->long_name:codec->name;

            /* Escape some characters that have a special meaning in FLTK */
            for (std::string::size_type i = 0; (i = codecstr.find('/', i)) != std::string::npos;) {
                codecstr.insert(i, "\\");
                i += 2;
            }

            if (codec->type == AVMEDIA_TYPE_VIDEO) {
                videochoice->add(codecstr.c_str(), 0, nullptr, reinterpret_cast<void*>(codec->id), 0);
            }
            if (codec->type == AVMEDIA_TYPE_AUDIO) {
                audiochoice->add(codecstr.c_str(), 0, nullptr, reinterpret_cast<void*>(codec->id), 0);
            }
        }
        //std::cout << "found codec " << codec->name << std::endl;
        // fprintf(stderr, "%s\n", codec->long_name);
        codec = av_codec_next(codec);
    }

    update_config();
    window->end();
}

void EncodeWindow::update_config()
{
    if (context->config.dumpfile.empty()) {
        encodepath->value(context->gamepath.c_str());
    }
    else {
        encodepath->value(context->config.dumpfile.c_str());
    }
    encodepathchooser->preset_file(encodepath->value());

    /* Browse the list of video codecs and select the item that matches
     * the value in the config using the item's user data.
     */
    const Fl_Menu_Item* vcodec_item = videochoice->menu();
    // vcodec_item = vcodec_item->first();
    while (vcodec_item) { // FIXME: This might loop if we don't find any matching value!
        if (static_cast<AVCodecID>(vcodec_item->argument()) == context->config.sc.video_codec) {
            videochoice->value(vcodec_item);
            break;
        }
        vcodec_item = vcodec_item->next();
    }

    /* Enable/disable video bitrate for lossy/lossless codecs */
    const AVCodecDescriptor* vcodec = avcodec_descriptor_get(context->config.sc.video_codec);
    if ((vcodec->props & AV_CODEC_PROP_LOSSLESS) && !(vcodec->props & AV_CODEC_PROP_LOSSY)) {
        videobitrate->deactivate();
    }
    else {
        videobitrate->activate();
    }

    /* Set video bitrate */
    std::string vbstr = std::to_string(context->config.sc.video_bitrate);
    videobitrate->value(vbstr.c_str());

    /* Same for audio codec and bitrate */
    const Fl_Menu_Item* acodec_item = audiochoice->menu();
    acodec_item = acodec_item->first();
    while (acodec_item) {
        if (static_cast<AVCodecID>(acodec_item->argument()) == context->config.sc.audio_codec) {
            audiochoice->value(acodec_item);
            break;
        }
        acodec_item = acodec_item->next();
    }

    /* Enable/disable audio bitrate for lossy/lossless codecs */
    const AVCodecDescriptor* acodec = avcodec_descriptor_get(context->config.sc.audio_codec);
    if ((acodec->props & AV_CODEC_PROP_LOSSLESS) && !(acodec->props & AV_CODEC_PROP_LOSSY)) {
        audiobitrate->deactivate();
    }
    else {
        audiobitrate->activate();
    }

    std::string abstr = std::to_string(context->config.sc.audio_bitrate);
    audiobitrate->value(abstr.c_str());
}

void start_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Fill encode filename */
    ew->context->config.dumpfile = ew->encodepath->value();
    ew->context->config.dumpfile_modified = true;

    /* Set video codec and bitrate */
    const Fl_Menu_Item* vcodec_item = ew->videochoice->mvalue();
    ew->context->config.sc.video_codec = static_cast<AVCodecID>(vcodec_item->argument());
    ew->context->config.sc.video_bitrate = std::stoi(ew->videobitrate->value());

    /* Set audio codec and bitrate */
    const Fl_Menu_Item* acodec_item = ew->audiochoice->mvalue();
    ew->context->config.sc.audio_codec = static_cast<AVCodecID>(acodec_item->argument());
    ew->context->config.sc.audio_bitrate = std::stoi(ew->audiobitrate->value());

    ew->context->config.sc_modified = true;

    /* Close window */
    ew->window->hide();
}

void cancel_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Close window */
    ew->window->hide();
}

void browse_encodepath_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    int ret = ew->encodepathchooser->show();

    const char* filename = ew->encodepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        ew->encodepath->value(filename);
    }
}

void vcodec_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    const Fl_Menu_Item* vcodec_item = ew->videochoice->mvalue();

    /* Enable/disable video bitrate for lossy/lossless codecs */
    const AVCodecDescriptor* vcodec = avcodec_descriptor_get(static_cast<AVCodecID>(vcodec_item->argument()));
    if ((vcodec->props & AV_CODEC_PROP_LOSSLESS) && !(vcodec->props & AV_CODEC_PROP_LOSSY)) {
        ew->videobitrate->deactivate();
    }
    else {
        ew->videobitrate->activate();
    }
}

void acodec_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    const Fl_Menu_Item* acodec_item = ew->audiochoice->mvalue();

    /* Enable/disable audio bitrate for lossy/lossless codecs */
    const AVCodecDescriptor* acodec = avcodec_descriptor_get(static_cast<AVCodecID>(acodec_item->argument()));
    if ((acodec->props & AV_CODEC_PROP_LOSSLESS) && !(acodec->props & AV_CODEC_PROP_LOSSY)) {
        ew->audiobitrate->deactivate();
    }
    else {
        ew->audiobitrate->activate();
    }
}

#endif
