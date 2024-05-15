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

#include "RuntimePane.h"
#include "tooltip/ToolTipComboBox.h"
#include "tooltip/ToolTipCheckBox.h"
#include "tooltip/ToolTipGroupBox.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>

RuntimePane::RuntimePane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void RuntimePane::initLayout()
{
    generalBox = new QGroupBox(tr("General"));
    QVBoxLayout* generalLayout = new QVBoxLayout;
    generalBox->setLayout(generalLayout);

    QFormLayout* localeLayout = new QFormLayout;
    localeLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    localeLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    localeChoice = new ToolTipComboBox();
    localeChoice->addItem(tr("English"), SharedConfig::LOCALE_ENGLISH);
    localeChoice->addItem(tr("Japanese"), SharedConfig::LOCALE_JAPANESE);
    localeChoice->addItem(tr("Korean"), SharedConfig::LOCALE_KOREAN);
    localeChoice->addItem(tr("Chinese, Simplified"), SharedConfig::LOCALE_CHINESE_SIMPLIFIED);
    localeChoice->addItem(tr("Chinese, Traditional"), SharedConfig::LOCALE_CHINESE_TRADITIONAL);
    localeChoice->addItem(tr("Spanish"), SharedConfig::LOCALE_SPANISH);
    localeChoice->addItem(tr("German"), SharedConfig::LOCALE_GERMAN);
    localeChoice->addItem(tr("French"), SharedConfig::LOCALE_FRENCH);
    localeChoice->addItem(tr("Italian"), SharedConfig::LOCALE_ITALIAN);
    localeChoice->addItem(tr("Native"), SharedConfig::LOCALE_NATIVE);

    localeLayout->addRow(new QLabel(tr("Force locale:")), localeChoice);

    writingBox = new ToolTipCheckBox(tr("Prevent writing to disk"));
    steamBox = new ToolTipCheckBox(tr("Virtual Steam client"));
    downloadsBox = new ToolTipCheckBox(tr("Allow downloading missing libraries"));

    generalLayout->addLayout(localeLayout);
    generalLayout->addWidget(writingBox);
    generalLayout->addWidget(steamBox);
    generalLayout->addWidget(downloadsBox);
    
    savestateBox = new QGroupBox(tr("Savestates"));
    QGridLayout* savestateLayout = new QGridLayout;
    savestateBox->setLayout(savestateLayout);

    stateIncrementalBox = new ToolTipCheckBox(tr("Incremental savestates"));
    if (!context->is_soft_dirty) {
        stateIncrementalBox->setEnabled(false);
        context->config.sc.savestate_settings &= ~SharedConfig::SS_INCREMENTAL;
    }
    stateRamBox = new ToolTipCheckBox(tr("Store savestates in RAM"));
    stateCompressedBox = new ToolTipCheckBox(tr("Compressed savestates"));
    stateUnmappedBox = new ToolTipCheckBox(tr("Skip unmapped pages"));
    stateForkBox = new ToolTipCheckBox(tr("Fork to save states"));

    savestateLayout->addWidget(stateIncrementalBox, 0, 0);
    savestateLayout->addWidget(stateRamBox, 0, 1);
    savestateLayout->addWidget(stateCompressedBox, 1, 0);
    savestateLayout->addWidget(stateUnmappedBox, 2, 0);
    savestateLayout->addWidget(stateForkBox, 2, 1);

    timingBox = new QGroupBox(tr("Timing"));
    QVBoxLayout* timingMainLayout = new QVBoxLayout;
    QFormLayout* timingLayout = new QFormLayout;
    timingBox->setLayout(timingMainLayout);
    timingLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    timingLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    trackingBox = new ToolTipGroupBox(tr("Time tracking"));
    QGridLayout* trackingLayout = new QGridLayout;
    trackingBox->setLayout(trackingLayout);

    trackingTimeBox = new QCheckBox(tr("time()"));
    trackingGettimeofdayBox = new QCheckBox(tr("gettimeofday()"));
    trackingClockBox = new QCheckBox(tr("clock()"));
    trackingClockGettimeRBox = new QCheckBox(tr("clock_gettime() realtime"));
    trackingClockGettimeMBox = new QCheckBox(tr("clock_gettime() monotonic"));
    trackingSDLTicksBox = new QCheckBox(tr("SDL_GetTicks()"));
    trackingSDLCounterBox = new QCheckBox(tr("SDL_GetPerformanceCounter()"));
    trackingGetTicksBox = new QCheckBox(tr("GetTickCount()"));
    trackingGetTicks64Box = new QCheckBox(tr("GetTickCount64()"));
    trackingQueryBox = new QCheckBox(tr("QueryPerformanceCounter()"));

    trackingLayout->addWidget(trackingTimeBox, 0, 0);
    trackingLayout->addWidget(trackingGettimeofdayBox, 1, 0);
    trackingLayout->addWidget(trackingClockBox, 2, 0);
    trackingLayout->addWidget(trackingClockGettimeRBox, 3, 0);
    trackingLayout->addWidget(trackingClockGettimeMBox, 4, 0);
    trackingLayout->addWidget(trackingSDLTicksBox, 0, 1);
    trackingLayout->addWidget(trackingSDLCounterBox, 1, 1);
    trackingLayout->addWidget(trackingGetTicksBox, 2, 1);
    trackingLayout->addWidget(trackingGetTicks64Box, 3, 1);
    trackingLayout->addWidget(trackingQueryBox, 4, 1);

    waitChoice = new ToolTipComboBox();
    waitChoice->addItem(tr("Native waits"), SharedConfig::WAIT_NATIVE);
    waitChoice->addItem(tr("Infinite waits"), SharedConfig::WAIT_INFINITE);
    waitChoice->addItem(tr("Full infinite waits"), SharedConfig::WAIT_FULL_INFINITE);
    waitChoice->addItem(tr("Finite waits"), SharedConfig::WAIT_FINITE);
    waitChoice->addItem(tr("Full waits"), SharedConfig::WAIT_FULL);
    waitChoice->addItem(tr("No waits"), SharedConfig::NO_WAIT);

    sleepChoice = new ToolTipComboBox();
    sleepChoice->addItem(tr("Never advance time"), SharedConfig::SLEEP_NEVER);
    sleepChoice->addItem(tr("Advance time on main thread"), SharedConfig::SLEEP_MAIN);
    sleepChoice->addItem(tr("Always advance time"), SharedConfig::SLEEP_ALWAYS);

    QGroupBox* asyncBox = new QGroupBox(tr("Asynchronous events"));
    QGridLayout* asyncLayout = new QGridLayout;
    asyncBox->setLayout(asyncLayout);

    asyncJsBox = new ToolTipCheckBox(tr("jsdev"));
    asyncEvBox = new ToolTipCheckBox(tr("evdev"));
    asyncXbegBox = new ToolTipCheckBox(tr("XEvents at frame beginning"));
    asyncXendBox = new ToolTipCheckBox(tr("XEvents at frame end"));
    asyncSDLBegBox = new ToolTipCheckBox(tr("SDL events at frame beginning"));
    asyncSDLEndBox = new ToolTipCheckBox(tr("SDL events at frame end"));

    asyncLayout->addWidget(asyncJsBox, 0, 0);
    asyncLayout->addWidget(asyncEvBox, 0, 1);
    asyncLayout->addWidget(asyncXbegBox, 1, 0);
    asyncLayout->addWidget(asyncXendBox, 1, 1);
    asyncLayout->addWidget(asyncSDLBegBox, 2, 0);
    asyncLayout->addWidget(asyncSDLEndBox, 2, 1);

    timingLayout->addRow(new QLabel(tr("Wait timeout:")), waitChoice);
    timingLayout->addRow(new QLabel(tr("Sleep handling:")), sleepChoice);

    timingMainLayout->addWidget(trackingBox);
    timingMainLayout->addLayout(timingLayout);
    timingMainLayout->addWidget(asyncBox);
    
    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalBox);
    mainLayout->addWidget(savestateBox);
    mainLayout->addWidget(timingBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void RuntimePane::initSignals()
{
    connect(localeChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RuntimePane::saveConfig);

    connect(writingBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(steamBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(downloadsBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);

    connect(stateIncrementalBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(stateRamBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(stateCompressedBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(stateUnmappedBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(stateForkBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);

    connect(trackingTimeBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingGettimeofdayBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingClockBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingClockGettimeRBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingClockGettimeMBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingSDLTicksBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingSDLCounterBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingGetTicksBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingGetTicks64Box, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(trackingQueryBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);

    connect(waitChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RuntimePane::saveConfig);
    connect(sleepChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RuntimePane::saveConfig);

    connect(asyncJsBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(asyncEvBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(asyncXbegBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(asyncXendBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(asyncSDLBegBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
    connect(asyncSDLEndBox, &QAbstractButton::clicked, this, &RuntimePane::saveConfig);
}

void RuntimePane::initToolTips()
{
    localeChoice->setTitle("Force Locale");
    localeChoice->setDescription("When set to anything except 'Native', "
    "libTAS will report to the game as if the system's locale is of the given country.");

    writingBox->setDescription("Prevent the game from writing files on disk, "
    "but write in memory instead. May cause issues in some games.");

    steamBox->setDescription("Implement a dummy Steam client, to be able to "
    "launch Steam games that require a connection to the Steam server. Almost none "
    "of the actual Steam features are implemented in this dummy client.");

    downloadsBox->setDescription("Some games may need some old libraries that are "
    "unavailable on newer distributions (e.g. for GameMaker Studio). If checked, libTAS "
    "will detect the missing libraries, download the registered ones and load "
    "them when running the game");

    stateIncrementalBox->setDescription("Optimize savestate size by only storing "
    "the memory pages that have been modified, at the cost of slightly more processing. "
    "This requires running on a native Linux installation (won't work on WSL2).<br><br>"
    "This is useful on games that eat a lot of memory, but is still a bit "
    "experimental and won't work for everyone."
    "<br><br><em>If unsure, leave this unchecked</em>");

    stateRamBox->setDescription("Storing savestates in RAM can provide a speedup, "
    "but libTAS does not check for available memory. If you have a SSD, this option "
    "is likely to not provide any speedup."
    "<br><br><em>If unsure, leave this unchecked</em>");

    stateCompressedBox->setDescription("Compress savestates on-the-fly using a "
    "fast compression routine (lz4). In addition to saving space, it can even "
    "lower the state saving time when the original state is very big. "
    "For some reasons, it fails to work for some people with a 'stack smashing' error <code>:(</code>"
    "<br><br><em>If unsure, leave this unchecked</em>");

    stateUnmappedBox->setDescription("Skip unmapped memory for shorter savestates, "
    "but some games crash when checking this."
    "<br><br><em>If unsure, leave this unchecked</em>");

    stateForkBox->setDescription("Fork the game process when saving a state, "
    "so that the forked process is doing the saving, and you can resume the game "
    "almost instantly without altering the state that is being saved, thanks to "
    "Linux copy-on-write magic. Useful for games that take a long time to save."
    "<br><br><em>If unsure, leave this unchecked</em>");

    trackingBox->setDescription("By checking a specific function, time will advance "
    "a bit when too many calls of that function have been made from the main thread. "
    "This prevents softlocks when a game wait in a loop for time to advance.<br><br>"
    "To determine which one to check, run the game first with all unchecked, and look "
    "at the terminal, which should prompt in case of softlock if a specific function "
    "is needed.<br><br>"
    "This can cause desyncs, so only check one if needed."
    "<br><br><em>If unsure, leave all unchecked</em>");

    waitChoice->setTitle("Wait timeout");
    waitChoice->setDescription("Set how to behave when the game is calling one of "
    "the 'wait' functions with a timeout (such as <code>pthread_cond_timedwait()</code>). "
    "This only applies when called from the main thread, other threads never advance time.<br><br>"
    "<b>Native waits:</b> Don't modify wait calls<br><br>"
    "<b>Infinite waits:</b> Waits have infinite timeout. Sync-proof, but may softlock<br><br>"
    "<b>Full infinite waits:</b> Advance time for the full timeout and wait infinitely. "
    "Sync-proof, but may still softlock and may advance time too much resulting in incorrect frame boundaries<br><br>"
    "<b>Finite waits:</b> Try to wait, and advance time if we get a timeout. Prevent "
    "softlocks but not perfectly sync-proof<br><br>"
    "<b>Full waits:</b> Advance time and try to wait<br><br>"
    "<b>No waits:</b> Wait with zero timeout"
    "<br><br><em>If unsure, leave this to 'Native waits'</em>");

    sleepChoice->setTitle("Sleep handling");
    sleepChoice->setDescription("This controls if and how does libTAS advance "
    "time when a sleep function is called (such as <code>usleep()</code>):<br><br>"
    "<b>Never advance time:</b> Never advance time when a sleep function is called. "
    "It can prevent desyncs, but can cause softlocks in some games<br><br>"
    "<b>Advance time on main thread:</b> Only advance time when a sleep function is "
    "called on main thread. This can cause desyncs for games where the main thread "
    "waits for another thread to finish by sleeping.<br><br>"
    "<b>Always advance time:</b> Always advance time when a sleep function is "
    "called from any thread. This will likely cause desyncs and odd behaviour, "
    "but can prevent softlocks in some games."
    "<br><br><em>If unsure, leave this to 'Advance time on main thread'</em>");

    asyncJsBox->setTitle("Sync jsdev events");
    asyncJsBox->setDescription("When enabled, it will wait before the beginning of each "
    "frame that all pending jsdev (joystick) events are processed. This is useful "
    "when jsdev events are processed in a separate thread, to make input handling "
    "deterministic."
    "<br><br><em>If unsure, leave this unchecked</em>");

    asyncEvBox->setTitle("Sync evdev events");
    asyncEvBox->setDescription("When enabled, it will wait before the beginning of each "
    "frame that all pending evdev (joystick) events are processed. This is useful "
    "when evdev events are processed in a separate thread, to make input handling "
    "deterministic."
    "<br><br><em>If unsure, leave this unchecked</em>");

    asyncXbegBox->setTitle("Sync XEvents at frame beginning");
    asyncXbegBox->setDescription("When enabled, it will wait before the beginning of each "
    "frame that all pending XEvents events are processed. This is useful "
    "when XEvents are processed in a separate thread, to make input handling "
    "deterministic."
    "<br><br><em>If unsure, leave this unchecked</em>");

    asyncXendBox->setTitle("Sync XEvents at frame end");
    asyncXendBox->setDescription("When enabled, it will wait at the end of each "
    "frame that all pending XEvents events are processed. This is useful "
    "when XEvents are processed in a separate thread, to make input handling "
    "deterministic."
    "<br><br><em>If unsure, leave this unchecked</em>");

    asyncSDLBegBox->setTitle("Sync SDL events at frame beginning");
    asyncSDLBegBox->setDescription("When enabled, it will wait before the beginning of each "
    "frame that all pending SDL events are processed. This is useful "
    "when SDL are processed in a separate thread, to make input handling "
    "deterministic."
    "<br><br><em>If unsure, leave this unchecked</em>");

    asyncSDLEndBox->setTitle("Sync SDL events at frame end");
    asyncSDLEndBox->setDescription("When enabled, it will wait at the end of each "
    "frame that all pending SDL events are processed. This is useful "
    "when SDL are processed in a separate thread, to make input handling "
    "deterministic."
    "<br><br><em>If unsure, leave this unchecked</em>");
}


void RuntimePane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void RuntimePane::loadConfig()
{
    int index = localeChoice->findData(context->config.sc.locale);
    if (index >= 0)
        localeChoice->setCurrentIndex(index);

    writingBox->setChecked(context->config.sc.prevent_savefiles);
    steamBox->setChecked(context->config.sc.virtual_steam);
    downloadsBox->setChecked(context->config.allow_downloads);

    stateIncrementalBox->setChecked(context->config.sc.savestate_settings & SharedConfig::SS_INCREMENTAL);
    stateRamBox->setChecked(context->config.sc.savestate_settings & SharedConfig::SS_RAM);
    stateCompressedBox->setChecked(context->config.sc.savestate_settings & SharedConfig::SS_COMPRESSED);
    stateUnmappedBox->setChecked(context->config.sc.savestate_settings & SharedConfig::SS_PRESENT);
    stateForkBox->setChecked(context->config.sc.savestate_settings & SharedConfig::SS_FORK);

    trackingTimeBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME] != -1);
    trackingGettimeofdayBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] != -1);
    trackingClockBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] != -1);
    trackingClockGettimeRBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME] != -1);
    trackingClockGettimeMBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC] != -1);
    trackingSDLTicksBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] != -1);
    trackingSDLCounterBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] != -1);
    trackingGetTicksBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT] != -1);
    trackingGetTicks64Box->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT64] != -1);
    trackingQueryBox->setChecked(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER] != -1);

    index = waitChoice->findData(context->config.sc.wait_timeout);
    if (index >= 0)
        waitChoice->setCurrentIndex(index);

    index = sleepChoice->findData(context->config.sc.sleep_handling);
    if (index >= 0)
        sleepChoice->setCurrentIndex(index);

    asyncJsBox->setChecked(context->config.sc.async_events & SharedConfig::ASYNC_JSDEV);
    asyncEvBox->setChecked(context->config.sc.async_events & SharedConfig::ASYNC_EVDEV);
    asyncXbegBox->setChecked(context->config.sc.async_events & SharedConfig::ASYNC_XEVENTS_BEG);
    asyncXendBox->setChecked(context->config.sc.async_events & SharedConfig::ASYNC_XEVENTS_END);
    asyncSDLBegBox->setChecked(context->config.sc.async_events & SharedConfig::ASYNC_SDLEVENTS_BEG);
    asyncSDLEndBox->setChecked(context->config.sc.async_events & SharedConfig::ASYNC_SDLEVENTS_END);

}

void RuntimePane::saveConfig()
{    
    context->config.sc.locale = localeChoice->currentData().toInt();

    context->config.sc.prevent_savefiles = writingBox->isChecked();
    context->config.sc.virtual_steam = steamBox->isChecked();
    context->config.allow_downloads = downloadsBox->isChecked();

    context->config.sc.savestate_settings = 0;
    context->config.sc.savestate_settings |= stateIncrementalBox->isChecked() ? SharedConfig::SS_INCREMENTAL : 0;
    context->config.sc.savestate_settings |= stateRamBox->isChecked() ? SharedConfig::SS_RAM : 0;
    context->config.sc.savestate_settings |= stateCompressedBox->isChecked() ? SharedConfig::SS_COMPRESSED : 0;
    context->config.sc.savestate_settings |= stateUnmappedBox->isChecked() ? SharedConfig::SS_PRESENT : 0;
    context->config.sc.savestate_settings |= stateForkBox->isChecked() ? SharedConfig::SS_FORK : 0;

    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME] = trackingTimeBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] = trackingGettimeofdayBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] = trackingClockBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME] = trackingClockGettimeRBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC] = trackingClockGettimeMBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] = trackingSDLTicksBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] = trackingSDLCounterBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT] = trackingGetTicksBox->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT64] = trackingGetTicks64Box->isChecked() ? 100 : -1;
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER] = trackingQueryBox->isChecked() ? 100 : -1;

    context->config.sc.wait_timeout = waitChoice->currentData().toInt();
    context->config.sc.sleep_handling = sleepChoice->currentData().toInt();

    context->config.sc.async_events = 0;
    context->config.sc.async_events |= asyncJsBox->isChecked() ? SharedConfig::ASYNC_JSDEV : 0;
    context->config.sc.async_events |= asyncEvBox->isChecked() ? SharedConfig::ASYNC_EVDEV : 0;
    context->config.sc.async_events |= asyncXbegBox->isChecked() ? SharedConfig::ASYNC_XEVENTS_BEG : 0;
    context->config.sc.async_events |= asyncXendBox->isChecked() ? SharedConfig::ASYNC_XEVENTS_END : 0;
    context->config.sc.async_events |= asyncSDLBegBox->isChecked() ? SharedConfig::ASYNC_SDLEVENTS_BEG : 0;
    context->config.sc.async_events |= asyncSDLEndBox->isChecked() ? SharedConfig::ASYNC_SDLEVENTS_END : 0;

    context->config.sc_modified = true;
}

void RuntimePane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        timingBox->setEnabled(true);
        break;
    case Context::STARTING:
        timingBox->setEnabled(false);
        break;
    }
}
