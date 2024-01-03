/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "VideoPane.h"
#include "tooltip/ToolTipCheckBox.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>

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
    QVBoxLayout* osdLayout = new QVBoxLayout;
    osdBox->setLayout(osdLayout);

    osdMenuBox = new QCheckBox(tr("Main Menu"));
    osdEncodeBox = new QCheckBox(tr("OSD on video encode"));

    osdLayout->addWidget(osdMenuBox);
    osdLayout->addWidget(osdEncodeBox);
    
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

    connect(screenCommonChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
        [this](){
            screenCommonRadio->setChecked(true);
            saveConfig();
        });
    connect(widthField, QOverload<int>::of(&QSpinBox::valueChanged), this,
    [this](){
        screenCustomRadio->setChecked(true);
        saveConfig();
    });

    connect(heightField, QOverload<int>::of(&QSpinBox::valueChanged), this,
    [this](){
        screenCustomRadio->setChecked(true);
        saveConfig();
    });
    connect(osdMenuBox, &QAbstractButton::clicked, this, &VideoPane::saveConfig);
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
            widthField->blockSignals(true);
            widthField->setValue(context->config.sc.screen_width);
            widthField->blockSignals(false);

            heightField->blockSignals(true);
            heightField->setValue(context->config.sc.screen_height);
            heightField->blockSignals(false);
        }
        else {
            /* Common screen resolution */
            screenCommonRadio->setChecked(true);
            screenCommonChoice->setCurrentIndex(index);
        }
    }
    
    osdMenuBox->setChecked(context->config.sc.osd);
    osdEncodeBox->setChecked(context->config.sc.osd_encode);

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
    
    context->config.sc.osd = osdMenuBox->isChecked();
    context->config.sc.osd_encode = osdEncodeBox->isChecked();

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
