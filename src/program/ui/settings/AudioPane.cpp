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

#include "AudioPane.h"
#include "tooltip/ToolTipCheckBox.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>

AudioPane::AudioPane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void AudioPane::initLayout()
{
    QGroupBox* formatBox = new QGroupBox(tr("Audio Format"));
    QFormLayout* formatLayout = new QFormLayout;
    formatBox->setLayout(formatLayout);

    formatLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    formatLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    freqChoice = new QComboBox();
    freqChoice->addItem(tr("8000 Hz"), 8000);
    freqChoice->addItem(tr("11025 Hz"), 11025);
    freqChoice->addItem(tr("12000 Hz"), 12000);
    freqChoice->addItem(tr("16000 Hz"), 16000);
    freqChoice->addItem(tr("22050 Hz"), 22050);
    freqChoice->addItem(tr("24000 Hz"), 24000);
    freqChoice->addItem(tr("32000 Hz"), 32000);
    freqChoice->addItem(tr("44100 Hz"), 44100);
    freqChoice->addItem(tr("48000 Hz"), 48000);

    formatLayout->addRow(new QLabel(tr("Sample rate:")), freqChoice);

    depthChoice = new QComboBox();
    depthChoice->addItem(tr("8 bit"), 8);
    depthChoice->addItem(tr("16 bit"), 16);

    formatLayout->addRow(new QLabel(tr("Bit depth:")), depthChoice);

    channelChoice = new QComboBox();
    channelChoice->addItem(tr("Mono"), 1);
    channelChoice->addItem(tr("Stereo"), 2);

    formatLayout->addRow(new QLabel(tr("Channels:")), channelChoice);

    QGroupBox* controlBox = new QGroupBox(tr("Audio Control"));
    QVBoxLayout* controlLayout = new QVBoxLayout;
    controlBox->setLayout(controlLayout);

    muteBox = new ToolTipCheckBox(tr("Mute"));
    disableBox = new ToolTipCheckBox(tr("Disable"));
    preferOpenAlBox = new ToolTipCheckBox(tr("Prefer OpenAL Soft"));

    controlLayout->addWidget(muteBox);
    controlLayout->addWidget(disableBox);
    controlLayout->addWidget(preferOpenAlBox);

    QGroupBox* gainBox = new QGroupBox(tr("Audio Gain"));
    QVBoxLayout* gainLayout = new QVBoxLayout;
    gainBox->setLayout(gainLayout);

    gainSlider = new QSlider(Qt::Vertical);
    gainSlider->setRange(0, 100);
    gainSlider->setSingleStep(1);
    gainSlider->setPageStep(10);
    gainSlider->setTracking(false);

    gainValue = new QSpinBox();
    gainValue->setRange(0, 100);

    gainLayout->addWidget(gainSlider);
    gainLayout->addWidget(gainValue);

    QVBoxLayout* const sideLayout = new QVBoxLayout;
    sideLayout->addWidget(formatBox);
    sideLayout->addWidget(controlBox);

    QHBoxLayout* const mainLayout = new QHBoxLayout;
    mainLayout->addLayout(sideLayout, 1);
    mainLayout->addWidget(gainBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void AudioPane::initSignals()
{
    connect(freqChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &AudioPane::saveConfig);
    connect(depthChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &AudioPane::saveConfig);
    connect(channelChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &AudioPane::saveConfig);
    connect(muteBox, &QAbstractButton::clicked, this, &AudioPane::saveConfig);
    connect(disableBox, &QAbstractButton::clicked, this, &AudioPane::saveConfig);    
    connect(preferOpenAlBox, &QAbstractButton::clicked, this, &AudioPane::saveConfig);
    connect(gainValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), gainSlider, &QSlider::setValue);
    connect(gainValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AudioPane::saveConfig);
    connect(gainSlider, &QSlider::valueChanged, this, &AudioPane::saveConfig);
    connect(gainSlider, &QSlider::valueChanged, gainValue, &QSpinBox::setValue);
}

void AudioPane::initToolTips()
{
    muteBox->setTitle("Mute Audio");
    muteBox->setDescription("When checked, audio will be not be heard, but it will "
    "still be present in encodes.<br><br>"
    "Unmuted audio can disrupt savestating by creating an extra thread."
    "<br><br><em>Unless you need to hear audio while TASing, leave this checked</em>");

    disableBox->setTitle("Disable Audio");
    disableBox->setDescription("When checked, tries to disable audio by reporting no audio "
    "device, or by failing to initialize the audio driver. Not all games accept to not "
    "have any audio device, so this can crash the game."
    "<br><br><em>If unsure, leave this unchecked</em>");

    preferOpenAlBox->setTitle("Prefer OpenAL Soft");
    preferOpenAlBox->setDescription("When checked, tries to use OpenAL Soft instead of libTAS' "
    "own OpenAL implementation. This can fix certain games which rely on special effects libTAS "
    "does not yet support, but has a higher risk of causing softlocks, especially with older "
    "OpenAL Soft versions. This setting does nothing if the game does not use OpenAL."
    "<br><br><em>If unsure, leave this unchecked</em>");
}

void AudioPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void AudioPane::loadConfig()
{
    int index = freqChoice->findData(context->config.sc.audio_frequency);
    if (index != -1) freqChoice->setCurrentIndex(index);

    index = depthChoice->findData(context->config.sc.audio_bitdepth);
    if (index != -1) depthChoice->setCurrentIndex(index);

    index = channelChoice->findData(context->config.sc.audio_channels);
    if (index != -1) channelChoice->setCurrentIndex(index);
    
    muteBox->setChecked(context->config.sc.audio_mute);
    disableBox->setChecked(context->config.sc.audio_disabled);
    preferOpenAlBox->setChecked(context->config.sc.openal_soft);

    gainValue->setValue(context->config.sc.audio_gain * 100.0f);
    
    /* Disconnect to not trigger valueChanged() signal */
    disconnect(gainSlider, &QSlider::valueChanged, this, &AudioPane::saveConfig);
    gainSlider->setValue(context->config.sc.audio_gain * 100.0f);
    connect(gainSlider, &QSlider::valueChanged, this, &AudioPane::saveConfig);
}

void AudioPane::saveConfig()
{
    context->config.sc.audio_frequency = freqChoice->itemData(freqChoice->currentIndex()).toInt();
    context->config.sc.audio_bitdepth = depthChoice->itemData(depthChoice->currentIndex()).toInt();
    context->config.sc.audio_channels = channelChoice->itemData(channelChoice->currentIndex()).toInt();
    context->config.sc.audio_mute = muteBox->isChecked();
    context->config.sc.audio_disabled = disableBox->isChecked();
    context->config.sc.openal_soft = preferOpenAlBox->isChecked();
    context->config.sc.audio_gain = ((float) gainValue->value()) / 100.0f;
    context->config.sc_modified = true;
}

void AudioPane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        freqChoice->setEnabled(true);
        depthChoice->setEnabled(true);
        channelChoice->setEnabled(true);
        disableBox->setEnabled(true);
        preferOpenAlBox->setEnabled(true);
        break;
    case Context::STARTING:
        freqChoice->setEnabled(false);
        depthChoice->setEnabled(false);
        channelChoice->setEnabled(false);
        disableBox->setEnabled(false);
        preferOpenAlBox->setEnabled(false);
        break;
    }
}
