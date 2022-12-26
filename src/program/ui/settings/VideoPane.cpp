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

// #include <QtWidgets/QLabel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>

#include "VideoPane.h"
#include "../../Context.h"
#include "tooltip/ToolTipCheckBox.h"

VideoPane::VideoPane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void VideoPane::initLayout()
{
    screenBox = new QGroupBox(tr("Screen resolution"));
    QGridLayout* screenLayout = new QGridLayout;
    screenBox->setLayout(screenLayout);

    screenNativeRadio = new QRadioButton("Native");
    screenCommonRadio = new QRadioButton("Common");
    screenCustomRadio = new QRadioButton("Custom");

    screenCommonChoice = new QComboBox();

    screenCommonChoice->addItem(tr("640x480 (4:3)"), (640 << 16) | 480);
    screenCommonChoice->addItem(tr("800x600 (4:3)"), (800 << 16) | 600);
    screenCommonChoice->addItem(tr("1024x768 (4:3)"), (1024 << 16) | 768);
    screenCommonChoice->addItem(tr("1280x720 (16:9)"), (1280 << 16) | 720);
    screenCommonChoice->addItem(tr("1280x800 (16:10)"), (1280 << 16) | 800);
    screenCommonChoice->addItem(tr("1400x1050 (4:3)"), (1400 << 16) | 1050);
    screenCommonChoice->addItem(tr("1440x900 (16:10)"), (1440 << 16) | 900);
    screenCommonChoice->addItem(tr("1600x900 (16:9)"), (1600 << 16) | 900);
    screenCommonChoice->addItem(tr("1680x1050 (16:10)"), (1680 << 16) | 1050);
    screenCommonChoice->addItem(tr("1920x1080 (16:9)"), (1920 << 16) | 1080);
    screenCommonChoice->addItem(tr("1920x1200 (16:10)"), (1920 << 16) | 1200);
    screenCommonChoice->addItem(tr("2560x1440 (16:9)"), (2560 << 16) | 1440);
    screenCommonChoice->addItem(tr("3840x2160 (16:9)"), (3840 << 16) | 2160);

    widthField = new QSpinBox();
    widthField->setMaximum(1 << 16);
    widthField->setMinimum(1);

    heightField = new QSpinBox();
    heightField->setMaximum(1 << 16);
    heightField->setMinimum(1);

    screenLayout->addWidget(screenNativeRadio, 0, 0, 1, 3);
    screenLayout->addWidget(screenCommonRadio, 1, 0);
    screenLayout->addWidget(screenCommonChoice, 1, 1, 1, 2);
    screenLayout->addWidget(screenCustomRadio, 2, 0);
    screenLayout->addWidget(widthField, 2, 1);
    screenLayout->addWidget(heightField, 2, 2);

    QGroupBox* osdBox = new QGroupBox(tr("On-screen display"));
    QGridLayout* osdLayout = new QGridLayout;
    osdBox->setLayout(osdLayout);

    /* Frame count */
    osdFrameBox = new QCheckBox(tr("Frame count"));

    frameHorChoice = new QComboBox();
    frameHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    frameHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    frameHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    frameVertChoice = new QComboBox();
    frameVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    frameVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    frameVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    /* Inputs */
    osdInputsBox = new QCheckBox(tr("Inputs"));

    inputsHorChoice = new QComboBox();
    inputsHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    inputsHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    inputsHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    inputsVertChoice = new QComboBox();
    inputsVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    inputsVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    inputsVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    /* Messages */
    osdMessagesBox = new QCheckBox(tr("Messages"));

    messagesHorChoice = new QComboBox();
    messagesHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    messagesHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    messagesHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    messagesVertChoice = new QComboBox();
    messagesVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    messagesVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    messagesVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    /* Watches */
    osdRamBox = new QCheckBox(tr("Ram Watches"));

    watchesHorChoice = new QComboBox();
    watchesHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    watchesHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    watchesHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    watchesVertChoice = new QComboBox();
    watchesVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    watchesVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    watchesVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    osdLuaBox = new QCheckBox(tr("Lua"));

    osdEncodeBox = new QCheckBox(tr("OSD on video encode"));

    osdLayout->addWidget(osdFrameBox, 0, 0);
    osdLayout->addWidget(frameHorChoice, 0, 1);
    osdLayout->addWidget(frameVertChoice, 0, 2);
    osdLayout->addWidget(osdInputsBox, 1, 0);
    osdLayout->addWidget(inputsHorChoice, 1, 1);
    osdLayout->addWidget(inputsVertChoice, 1, 2);
    osdLayout->addWidget(osdMessagesBox, 2, 0);
    osdLayout->addWidget(messagesHorChoice, 2, 1);
    osdLayout->addWidget(messagesVertChoice, 2, 2);
    osdLayout->addWidget(osdRamBox, 3, 0);
    osdLayout->addWidget(watchesHorChoice, 3, 1);
    osdLayout->addWidget(watchesVertChoice, 3, 2);
    osdLayout->addWidget(osdLuaBox, 4, 0);
    osdLayout->addWidget(osdEncodeBox, 5, 0, 1, 3);
    
    renderingBox = new QGroupBox(tr("Rendering"));
    QVBoxLayout* renderingLayout = new QVBoxLayout;
    renderingBox->setLayout(renderingLayout);

    rendSoftBox = new ToolTipCheckBox(tr("Force software rendering"));

    rendPerfBox = new ToolTipCheckBox(tr("Toggle performance tweaks"));

    renderingLayout->addWidget(rendSoftBox);
    renderingLayout->addWidget(rendPerfBox);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(screenBox);
    mainLayout->addWidget(osdBox);
    mainLayout->addWidget(renderingBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void VideoPane::initSignals()
{
    connect(screenNativeRadio, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(screenCommonRadio, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(screenCustomRadio, &QAbstractButton::clicked, this, &VideoPane::saveConfig);

    connect(screenCommonChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(widthField, QOverload<int>::of(&QSpinBox::valueChanged), this, &VideoPane::saveConfig);
    connect(heightField, QOverload<int>::of(&QSpinBox::valueChanged), this, &VideoPane::saveConfig);
    connect(osdFrameBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(frameHorChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(frameVertChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(osdInputsBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(inputsHorChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(inputsVertChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(osdMessagesBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(messagesHorChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(messagesVertChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(osdRamBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(watchesHorChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(watchesVertChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &VideoPane::saveConfig);
    connect(osdLuaBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(osdEncodeBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);

    connect(rendSoftBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
    connect(rendPerfBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
}

void VideoPane::initToolTips()
{
    rendSoftBox->setDescription("Enforce the use of Mesa's OpenGL/Vulkan software "
    "driver, which is necessary for savestates to work correctly. You will need "
    "to already be using a Mesa for rendering, and you should get a warning popup "
    "window and game startup if it is not the case."
    "<br><br><em>If unsure, leave this checked</em>");

    rendPerfBox->setDescription("Change several GL parameters (e.g. texture filtering) "
    "to save a bit of performance, useful when using software rendering."
    "<br><br><em>If unsure, leave this unchecked</em>");
}


void VideoPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void VideoPane::loadConfig()
{
    int resValue = (context->config.sc.screen_width << 16) | context->config.sc.screen_height;
    if (resValue == 0) {
        /* Native screen resolution */
        screenNativeRadio->setChecked(true);
    }
    else {
        int index = screenCommonChoice->findData(resValue);
        if (index == -1) {
            /* Custom screen resolution */
            screenCustomRadio->setChecked(true);
            widthField->setValue(context->config.sc.screen_width);
            heightField->setValue(context->config.sc.screen_height);
        }
        else {
            /* Common screen resolution */
            screenCommonRadio->setChecked(true);
            screenCommonChoice->setCurrentIndex(index);
        }
    }
    
    osdFrameBox->setChecked(context->config.sc.osd & SharedConfig::OSD_FRAMECOUNT);
    osdInputsBox->setChecked(context->config.sc.osd & SharedConfig::OSD_INPUTS);
    osdMessagesBox->setChecked(context->config.sc.osd & SharedConfig::OSD_MESSAGES);
    osdRamBox->setChecked(context->config.sc.osd & SharedConfig::OSD_RAMWATCHES);
    osdLuaBox->setChecked(context->config.sc.osd & SharedConfig::OSD_LUA);
    osdEncodeBox->setChecked(context->config.sc.osd_encode);
    
    int index;
    int hflags = SharedConfig::OSD_LEFT | SharedConfig::OSD_HCENTER | SharedConfig::OSD_RIGHT;
    int vflags = SharedConfig::OSD_TOP | SharedConfig::OSD_VCENTER | SharedConfig::OSD_BOTTOM;

    index = frameHorChoice->findData(context->config.sc.osd_frame_location & hflags);
    if (index >= 0)
        frameHorChoice->setCurrentIndex(index);

    index = frameVertChoice->findData(context->config.sc.osd_frame_location & vflags);
    if (index >= 0)
        frameVertChoice->setCurrentIndex(index);

    index = inputsHorChoice->findData(context->config.sc.osd_inputs_location & hflags);
    if (index >= 0)
        inputsHorChoice->setCurrentIndex(index);

    index = inputsVertChoice->findData(context->config.sc.osd_inputs_location & vflags);
    if (index >= 0)
        inputsVertChoice->setCurrentIndex(index);

    index = messagesHorChoice->findData(context->config.sc.osd_messages_location & hflags);
    if (index >= 0)
        messagesHorChoice->setCurrentIndex(index);

    index = messagesVertChoice->findData(context->config.sc.osd_messages_location & vflags);
    if (index >= 0)
        messagesVertChoice->setCurrentIndex(index);

    index = watchesHorChoice->findData(context->config.sc.osd_ramwatches_location & hflags);
    if (index >= 0)
        watchesHorChoice->setCurrentIndex(index);

    index = watchesVertChoice->findData(context->config.sc.osd_ramwatches_location & vflags);
    if (index >= 0)
        watchesVertChoice->setCurrentIndex(index);

    rendSoftBox->setChecked(context->config.sc.opengl_soft);
    rendPerfBox->setChecked(context->config.sc.opengl_performance);
}

void VideoPane::saveConfig()
{
    if (screenNativeRadio->isChecked()) {
        context->config.sc.screen_width = 0;
        context->config.sc.screen_height = 0;
    }
    else if (screenCommonRadio->isChecked()) {
        int resValue = screenCommonChoice->itemData(screenCommonChoice->currentIndex()).toInt();
        context->config.sc.screen_width = (resValue >> 16);
        context->config.sc.screen_height = (resValue & 0xffff);
    }
    else {
        context->config.sc.screen_width = widthField->value();
        context->config.sc.screen_height = heightField->value();
    }
    
    context->config.sc.osd = 0;
    if (osdFrameBox->isChecked())
        context->config.sc.osd |= SharedConfig::OSD_FRAMECOUNT;
    if (osdInputsBox->isChecked())
        context->config.sc.osd |= SharedConfig::OSD_INPUTS;
    if (osdMessagesBox->isChecked())
        context->config.sc.osd |= SharedConfig::OSD_MESSAGES;        
    if (osdRamBox->isChecked())
        context->config.sc.osd |= SharedConfig::OSD_RAMWATCHES;
    if (osdLuaBox->isChecked())
        context->config.sc.osd |= SharedConfig::OSD_LUA;
        
    context->config.sc.osd_encode = osdEncodeBox->isChecked();

    context->config.sc.osd_frame_location = frameHorChoice->currentData().toInt() | frameVertChoice->currentData().toInt();
    context->config.sc.osd_inputs_location = inputsHorChoice->currentData().toInt() | inputsVertChoice->currentData().toInt();
    context->config.sc.osd_messages_location = messagesHorChoice->currentData().toInt() | messagesVertChoice->currentData().toInt();
    context->config.sc.osd_ramwatches_location = watchesHorChoice->currentData().toInt() | watchesVertChoice->currentData().toInt();

    context->config.sc.opengl_soft = rendSoftBox->isChecked();
    context->config.sc.opengl_performance = rendPerfBox->isChecked();

    context->config.sc_modified = true;
}

void VideoPane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        screenBox->setEnabled(true);
        renderingBox->setEnabled(true);
        break;
    case Context::STARTING:
        screenBox->setEnabled(false);
        renderingBox->setEnabled(false);
        break;
    }
}
