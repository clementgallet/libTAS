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

#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include "EncodeWindow.h"

#include <iostream>

EncodeWindow::EncodeWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Encoding configuration");

    /* Video file */
    encodePath = new QLineEdit();

    browseEncodePath = new QPushButton("Browse...");
    connect(browseEncodePath, &QAbstractButton::clicked, this, &EncodeWindow::slotBrowseEncodePath);

    QGroupBox *encodeFileGroupBox = new QGroupBox(tr("Encode file path"));
    QHBoxLayout *encodeFileLayout = new QHBoxLayout;
    encodeFileLayout->addWidget(encodePath);
    encodeFileLayout->addWidget(browseEncodePath);
    encodeFileGroupBox->setLayout(encodeFileLayout);

    /* Video/Audio codecs */

    videoChoice = new QComboBox();
    videoChoice->addItem("FFmpeg video codec #1", "ffv1");
    videoChoice->addItem("H.264", "libx264");
    videoChoice->addItem("H.264 RGB", "libx264rgb");
    videoChoice->addItem("H.265", "libx265");
    videoChoice->addItem("raw video", "rawvideo");
    connect(videoChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EncodeWindow::slotUpdate);

    videoBitrate = new QSpinBox();
    videoBitrate->setMaximum(1000000000);
    connect(videoBitrate, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EncodeWindow::slotUpdate);

    audioChoice = new QComboBox();
    audioChoice->addItem("AAC (Advanced Audio Coding)", "aac");
    audioChoice->addItem("FLAC", "flac");
    audioChoice->addItem("PCM signed 16-bit little-endian", "pcm_s16le");
    audioChoice->addItem("Vorbis", "libvorbis");
    connect(audioChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EncodeWindow::slotUpdate);

    audioBitrate = new QSpinBox();
    audioBitrate->setMaximum(1000000000);
    connect(audioBitrate, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &EncodeWindow::slotUpdate);

    ffmpegOptions = new QLineEdit();
    
    mergeCheck = new QCheckBox("Automatically merge dump segments");
    connect(mergeCheck, &QAbstractButton::clicked, this, &EncodeWindow::slotMerge);

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
    encodeCodecLayout->addWidget(mergeCheck, 3, 1, 1, 4);
    

    encodeCodecLayout->setColumnMinimumWidth(2, 50);
    encodeCodecLayout->setColumnStretch(2, 1);
    codecGroupBox->setLayout(encodeCodecLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &EncodeWindow::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EncodeWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(encodeFileGroupBox);
    mainLayout->addWidget(codecGroupBox);
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
    
    mergeCheck->setChecked(context->config.merge_dump_segments);

    if (context->config.ffmpegoptions.empty()) {
        slotUpdate();
    }
}

void EncodeWindow::slotUpdate()
{
    QString options = QString("-c:v %1 -b:v %2k -c:a %3 -b:a %4k").arg(videoChoice->currentData().toString()).arg(videoBitrate->value()).arg(audioChoice->currentData().toString()).arg(audioBitrate->value());
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

    context->config.sc_modified = true;

    /* Close window */
    accept();
}

void EncodeWindow::slotBrowseEncodePath()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Choose an encode filename"), encodePath->text());
    if (!filename.isNull())
        encodePath->setText(filename);
}

void EncodeWindow::slotMerge(bool checked)
{
        context->config.merge_dump_segments = checked;
}
