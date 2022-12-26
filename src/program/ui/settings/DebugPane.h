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

#ifndef LIBTAS_DEBUGPANE_H_INCLUDED
#define LIBTAS_DEBUGPANE_H_INCLUDED

#include <QtWidgets/QWidget>

class Context;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QSpinBox;
class ToolTipCheckBox;
class QGroupBox;

class DebugPane : public QWidget {
    Q_OBJECT
public:
    DebugPane(Context *c);

    void update(int status);

    Context *context;

private:
    void initLayout();
    void initSignals();
    void initToolTips();
    
    void showEvent(QShowEvent *event) override;
    
    QGroupBox* generalBox;
    
    ToolTipCheckBox* debugUncontrolledBox;
    ToolTipCheckBox* debugEventsBox;
    ToolTipCheckBox* debugMainBox;
    ToolTipCheckBox* debugIOBox;
    QCheckBox* debugSigIntBox;

    QComboBox* logToChoice;

    QCheckBox* logPrintMainBox;
    QCheckBox* logPrintFrequentBox;
    QCheckBox* logPrintErrorBox;
    QCheckBox* logPrintWarningBox;
    QCheckBox* logPrintInfoBox;
    QCheckBox* logPrintTODOBox;
    QCheckBox* logPrintAVBox;
    QCheckBox* logPrintCheckpointBox;
    QCheckBox* logPrintEventsBox;
    QCheckBox* logPrintIOBox;
    QCheckBox* logPrintHookBox;
    QCheckBox* logPrintJoystickBox;
    QCheckBox* logPrintKeyboardBox;
    QCheckBox* logPrintLocaleBox;
    QCheckBox* logPrintMouseBox;
    QCheckBox* logPrintGLBox;
    QCheckBox* logPrintRandomBox;
    QCheckBox* logPrintSDLBox;
    QCheckBox* logPrintSignalsBox;
    QCheckBox* logPrintSleepBox;
    QCheckBox* logPrintSocketBox;
    QCheckBox* logPrintSoundBox;
    QCheckBox* logPrintSteamBox;
    QCheckBox* logPrintSystemBox;
    QCheckBox* logPrintTimeGetBox;
    QCheckBox* logPrintTimeSetBox;
    QCheckBox* logPrintTimersBox;
    QCheckBox* logPrintThreadsBox;
    QCheckBox* logPrintWaitBox;
    QCheckBox* logPrintWindowsBox;
    QCheckBox* logPrintWineBox;
    
    QCheckBox* logExcludeMainBox;
    QCheckBox* logExcludeFrequentBox;
    QCheckBox* logExcludeErrorBox;
    QCheckBox* logExcludeWarningBox;
    QCheckBox* logExcludeInfoBox;
    QCheckBox* logExcludeTODOBox;
    QCheckBox* logExcludeAVBox;
    QCheckBox* logExcludeCheckpointBox;
    QCheckBox* logExcludeEventsBox;
    QCheckBox* logExcludeIOBox;
    QCheckBox* logExcludeHookBox;
    QCheckBox* logExcludeJoystickBox;
    QCheckBox* logExcludeKeyboardBox;
    QCheckBox* logExcludeLocaleBox;
    QCheckBox* logExcludeMouseBox;
    QCheckBox* logExcludeGLBox;
    QCheckBox* logExcludeRandomBox;
    QCheckBox* logExcludeSDLBox;
    QCheckBox* logExcludeSignalsBox;
    QCheckBox* logExcludeSleepBox;
    QCheckBox* logExcludeSocketBox;
    QCheckBox* logExcludeSoundBox;
    QCheckBox* logExcludeSteamBox;
    QCheckBox* logExcludeSystemBox;
    QCheckBox* logExcludeTimeGetBox;
    QCheckBox* logExcludeTimeSetBox;
    QCheckBox* logExcludeTimersBox;
    QCheckBox* logExcludeThreadsBox;
    QCheckBox* logExcludeWaitBox;
    QCheckBox* logExcludeWindowsBox;
    QCheckBox* logExcludeWineBox;

public slots:
    void loadConfig();
    void saveConfig();
};

#endif
