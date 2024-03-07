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

#ifndef LIBTAS_RUNTIMEPANE_H_INCLUDED
#define LIBTAS_RUNTIMEPANE_H_INCLUDED

#include <QtWidgets/QWidget>

class Context;
class QComboBox;
class QCheckBox;
class ToolTipComboBox;
class ToolTipCheckBox;
class ToolTipGroupBox;
class QGroupBox;

class RuntimePane : public QWidget {
    Q_OBJECT
public:
    RuntimePane(Context *c);

    void update(int status);

    Context *context;

private:
    void initLayout();
    void initSignals();
    void initToolTips();

    void showEvent(QShowEvent *event) override;
    
    QGroupBox* generalBox;
    QGroupBox* savestateBox;
    QGroupBox* timingBox;
    
    ToolTipComboBox* localeChoice;

    ToolTipCheckBox* writingBox;
    ToolTipCheckBox* recycleBox;
    ToolTipCheckBox* steamBox;
    ToolTipCheckBox* downloadsBox;

    ToolTipCheckBox* stateIncrementalBox;
    ToolTipCheckBox* stateRamBox;
    ToolTipCheckBox* stateBacktrackBox;
    ToolTipCheckBox* stateCompressedBox;
    ToolTipCheckBox* stateUnmappedBox;
    ToolTipCheckBox* stateForkBox;

    ToolTipGroupBox* trackingBox;

    QCheckBox* trackingTimeBox;
    QCheckBox* trackingGettimeofdayBox;
    QCheckBox* trackingClockBox;
    QCheckBox* trackingClockGettimeRBox;
    QCheckBox* trackingClockGettimeMBox;
    QCheckBox* trackingSDLTicksBox;
    QCheckBox* trackingSDLCounterBox;
    QCheckBox* trackingGetTicksBox;
    QCheckBox* trackingGetTicks64Box;
    QCheckBox* trackingQueryBox;

    ToolTipComboBox* waitChoice;
    ToolTipComboBox* sleepChoice;

    ToolTipCheckBox* asyncJsBox;
    ToolTipCheckBox* asyncEvBox;
    ToolTipCheckBox* asyncXbegBox;
    ToolTipCheckBox* asyncXendBox;
    ToolTipCheckBox* asyncSDLBegBox;
    ToolTipCheckBox* asyncSDLEndBox;

public slots:
    void loadConfig();
    void saveConfig();
};

#endif
