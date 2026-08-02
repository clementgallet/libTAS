/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "EncodeWindow.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <iostream>

EncodeWindow::EncodeWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Encoding configuration");

    /* Video file */
    encodePath = new QLineEdit();

    browseEncodePath = new QPushButton("Browse...");
    connect(browseEncodePath, &QAbstractButton::clicked, this, &EncodeWindow::slotBrowseEncodePath);

    QGroupBox *encodeFileGroupBox = new QGroupBox(tr("Encode file path - you must specify a valid file extension (.mkv, .mp4, etc.)"));
    QHBoxLayout *encodeFileLayout = new QHBoxLayout;
    encodeFileLayout->addWidget(encodePath);
    encodeFileLayout->addWidget(browseEncodePath);
    encodeFileGroupBox->setLayout(encodeFileLayout);

    /* Video/Audio codecs */

    videoChoice = new QComboBox();
    videoChoice->addItem("H.264", "libx264");
    videoChoice->addItem("H.265", "libx265");
    videoChoice->addItem("FFmpeg video codec #1", "ffv1");
    videoChoice->addItem("Ut video codec", "utvideo");
    videoChoice->addItem("raw video", "rawvideo");
    connect(videoChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EncodeWindow::slotUpdate);

    videoBitrate = new QSpinBox();
    videoBitrate->setMaximum(1000000000);
    connect(videoBitrate, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EncodeWindow::slotUpdate);

    audioChoice = new QComboBox();
    audioChoice->addItem("AAC (Advanced Audio Coding)", "aac");
    audioChoice->addItem("Vorbis", "libvorbis");
    audioChoice->addItem("FLAC", "flac");
    audioChoice->addItem("PCM signed 16-bit little-endian", "pcm_s16le");
    connect(audioChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EncodeWindow::slotUpdate);

    audioBitrate = new QSpinBox();
    audioBitrate->setMaximum(1000000000);
    connect(audioBitrate, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EncodeWindow::slotUpdate);

    videoFramerateNum = new QSpinBox();
    videoFramerateNum->setMaximum(1000000000);

    videoFramerateDen = new QSpinBox();
    videoFramerateDen->setMinimum(1);
    videoFramerateDen->setMaximum(1000000000);

    videoWidth = new QSpinBox();
    videoWidth->setMinimum(1);
    videoWidth->setMaximum(1000000000);

    videoHeight = new QSpinBox();
    videoHeight->setMinimum(1);
    videoHeight->setMaximum(1000000000);
    
    videoFilter = new QComboBox();
    videoFilter->addItem("Nearest Neighbor", SharedConfig::VFILTER_POINT);
    videoFilter->addItem("Bilinear", SharedConfig::VFILTER_BILINEAR);
    videoFilter->addItem("Bicubic", SharedConfig::VFILTER_BICUBIC);
    connect(videoFilter, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EncodeWindow::slotUpdate);

    ffmpegOptions = new QLineEdit();

    QGroupBox *codecGroupBox = new QGroupBox(tr("Encode codec settings"));
    QGridLayout *encodeCodecLayout = new QGridLayout;
    encodeCodecLayout->addWidget(new QLabel(tr("Video codec:")), 0, 0);
    encodeCodecLayout->addWidget(videoChoice, 0, 1);
    encodeCodecLayout->addWidget(new QLabel(tr("Video bitrate (kbps):")), 0, 3);
    encodeCodecLayout->addWidget(videoBitrate, 0, 4);

    encodeCodecLayout->addWidget(new QLabel(tr("Audio codec:")), 1, 0);
    encodeCodecLayout->addWidget(audioChoice, 1, 1);
    encodeCodecLayout->addWidget(new QLabel(tr("Audio bitrate (kbps):")), 1, 3);
    encodeCodecLayout->addWidget(audioBitrate, 1, 4);

    encodeCodecLayout->addWidget(new QLabel(tr("ffmpeg options:")), 2, 0);
    encodeCodecLayout->addWidget(ffmpegOptions, 2, 1, 1, 4);

    encodeCodecLayout->setColumnMinimumWidth(2, 50);
    encodeCodecLayout->setColumnStretch(2, 1);
    codecGroupBox->setLayout(encodeCodecLayout);

    framerateGroupBox = new QGroupBox(tr("Custom framerate"));
    framerateGroupBox->setCheckable(true);
    framerateGroupBox->setChecked(false);

    QHBoxLayout *framerateLayout = new QHBoxLayout;
    framerateLayout->addWidget(new QLabel(tr("Video framerate:")));
    framerateLayout->addWidget(videoFramerateNum);
    framerateLayout->addWidget(new QLabel(tr("/")));
    framerateLayout->addWidget(videoFramerateDen);
    framerateLayout->setStretch(1, 1);
    framerateLayout->setStretch(3, 1);
    
    framerateGroupBox->setLayout(framerateLayout);

    resizeGroupBox = new QGroupBox(tr("Custom resolution"));
    resizeGroupBox->setCheckable(true);
    resizeGroupBox->setChecked(false);

    QGridLayout *resizeLayout = new QGridLayout;
    resizeLayout->addWidget(new QLabel(tr("Video width:")), 0, 0);
    resizeLayout->addWidget(videoWidth, 0, 1);
    resizeLayout->addWidget(new QLabel(tr("Video height:")), 0, 2);
    resizeLayout->addWidget(videoHeight, 0, 3);
    resizeLayout->addWidget(new QLabel(tr("Video filter:")), 1, 0);
    resizeLayout->addWidget(videoFilter, 1, 1, 1, 3);

    resizeGroupBox->setLayout(resizeLayout);

    encodeCodecLayout->addWidget(new QLabel(tr("Audio codec:")), 1, 0);
    encodeCodecLayout->addWidget(audioChoice, 1, 1);
    encodeCodecLayout->addWidget(new QLabel(tr("Audio bitrate (kbps):")), 1, 3);
    encodeCodecLayout->addWidget(audioBitrate, 1, 4);

    encodeCodecLayout->addWidget(new QLabel(tr("ffmpeg options:")), 2, 0);
    encodeCodecLayout->addWidget(ffmpegOptions, 2, 1, 1, 4);

    encodeCodecLayout->setColumnMinimumWidth(2, 50);
    encodeCodecLayout->setColumnStretch(2, 1);
    codecGroupBox->setLayout(encodeCodecLayout);


    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QPushButton* saveDefaultButton = new QPushButton(tr("Save as default"));
    buttonBox->addButton(saveDefaultButton, QDialogButtonBox::ApplyRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &EncodeWindow::slotOk);
    connect(saveDefaultButton, &QAbstractButton::clicked, this, &EncodeWindow::slotDefault);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EncodeWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(encodeFileGroupBox);
    mainLayout->addWidget(codecGroupBox);
    mainLayout->addWidget(framerateGroupBox);
    mainLayout->addWidget(resizeGroupBox);
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    update_config();
}

void EncodeWindow::update_config()
{
    if (context->config.dumpfile.empty()) {
        encodePath->setText(context->gamepath.c_str());
    }
    else {
        encodePath->setText(context->config.dumpfile.c_str());
    }

    /* Set video codec and bitrate */
    videoChoice->setCurrentIndex(context->config.sc.video_codec);
    videoBitrate->setValue(context->config.sc.video_bitrate);

    /* Set audio codec and bitrate */
    audioChoice->setCurrentIndex(context->config.sc.audio_codec);
    audioBitrate->setValue(context->config.sc.audio_bitrate);

    /* Set ffmpeg options */
    ffmpegOptions->setText(context->config.ffmpegoptions.c_str());

    /* Set video framerate */
    framerateGroupBox->setChecked(context->config.sc.video_framerate_num);
    if (context->config.sc.video_framerate_num) {
        videoFramerateNum->setValue(context->config.sc.video_framerate_num);
        videoFramerateDen->setValue(context->config.sc.video_framerate_den);
    }
    else {
        videoFramerateNum->setValue(context->config.sc.initial_framerate_num);
        videoFramerateDen->setValue(context->config.sc.initial_framerate_den);
    }

    /* Set video resize */
    resizeGroupBox->setChecked(context->config.sc.video_width);
    if (context->config.sc.video_width) {
        videoWidth->setValue(context->config.sc.video_width);
        videoHeight->setValue(context->config.sc.video_height);
        videoFilter->setCurrentIndex(context->config.sc.video_filter);
    }
    else {
        videoWidth->setValue(context->config.sc.screen_width);
        videoHeight->setValue(context->config.sc.screen_height);
        videoFilter->setCurrentIndex(0);
    }

    if (context->config.ffmpegoptions.empty()) {
        slotUpdate();
    }
}

void EncodeWindow::slotUpdate()
{
    QString options = QString("-c:v %1").arg(videoChoice->currentData().toString());

    /* Disable video bitrate for lossless codecs, and add specific default settings */
    switch (videoChoice->currentIndex()) {
        case SharedConfig::VCODEC_X264:
        case SharedConfig::VCODEC_X265:
            videoBitrate->setEnabled(true);
            options.append(QString(" -b:v %1k").arg(videoBitrate->value()));
            break;
        case SharedConfig::VCODEC_FFV1:
            videoBitrate->setEnabled(false);
            options.append(" -pix_fmt bgr0 -level 1");
            break;
        case SharedConfig::VCODEC_UT:
            videoBitrate->setEnabled(false);
            options.append(" -pred median -pix_fmt gbrp");
            break;
        case SharedConfig::VCODEC_RAW:
            videoBitrate->setEnabled(false);
            break;
        default:
            videoBitrate->setEnabled(true);
            break;
    }

    options.append(QString(" -c:a %1").arg(audioChoice->currentData().toString()));
    
    /* Disable audio bitrate for lossless codecs, and add specific default settings */
    switch (audioChoice->currentIndex()) {
        case SharedConfig::ACODEC_AAC:
        case SharedConfig::ACODEC_VORBIS:
            audioBitrate->setEnabled(true);
            options.append(QString(" -b:a %1k").arg(audioBitrate->value()));
            break;
        case SharedConfig::ACODEC_FLAC:
        case SharedConfig::ACODEC_PCM:
            audioBitrate->setEnabled(false);
            break;
        default:
            audioBitrate->setEnabled(true);
            break;
    }
    
    ffmpegOptions->setText(options);
}

void EncodeWindow::slotOk()
{
    /* Fill encode filename */
    context->config.dumpfile = encodePath->text().toStdString();
    context->config.dumpfile_modified = true;

    /* Set video codec and bitrate */
    context->config.sc.video_codec = videoChoice->currentIndex();
    context->config.sc.video_bitrate = videoBitrate->value();

    /* Set audio codec and bitrate */
    context->config.sc.audio_codec = audioChoice->currentIndex();
    context->config.sc.audio_bitrate = audioBitrate->value();
    context->config.ffmpegoptions = ffmpegOptions->text().toStdString();

    if (framerateGroupBox->isChecked()) {
        context->config.sc.video_framerate_num = videoFramerateNum->value();
        context->config.sc.video_framerate_den = videoFramerateDen->value();
    }
    else {
        context->config.sc.video_framerate_num = 0;
        context->config.sc.video_framerate_den = 0;
    }

    if (resizeGroupBox->isChecked()) {
        context->config.sc.video_width = videoWidth->value();
        context->config.sc.video_height = videoHeight->value();
        context->config.sc.video_filter = videoFilter->currentIndex();
    }
    else {
        context->config.sc.video_width = 0;
        context->config.sc.video_height = 0;
        context->config.sc.video_filter = 0;
    }

    context->config.sc_modified = true;

    /* Close window */
    accept();
}

void EncodeWindow::slotDefault()
{
    context->config.ffmpegoptions = ffmpegOptions->text().toStdString();
    context->config.saveDefaultFfmpeg(context->gamepath);
}

void EncodeWindow::slotBrowseEncodePath()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Choose an encode filename"), encodePath->text());
    if (!filename.isNull())
        encodePath->setText(filename);
}
