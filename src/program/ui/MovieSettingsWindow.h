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

#ifndef LIBTAS_MOVIESETTINGSWINDOW_H_INCLUDED
#define LIBTAS_MOVIESETTINGSWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>

class Context;
class MovieFile;
class QComboBox;
class ToolTipGroupBox;
class QSpinBox;

class MovieSettingsWindow : public QDialog {
    Q_OBJECT
public:
    MovieSettingsWindow(Context *c, MovieFile *m, QWidget *parent);

    Context *context;

private:
    void showEvent(QShowEvent *event) override;
    
    MovieFile *movie;

    QComboBox *mouseSupportChoice;
    QComboBox *joystickSupportChoice;

    QSpinBox *framerateNum;
    QSpinBox *framerateDen;
    QComboBox *variableFramerate;
    QComboBox *autoRestart;

    QSpinBox *initialSec;
    QSpinBox *initialNSec;
    QSpinBox *initialMonotonicSec;
    QSpinBox *initialMonotonicNSec;
    
    ToolTipGroupBox *timeTrackingBox;
    QSpinBox *timeTrackingTime;
    QSpinBox *timeTrackingGettimeofday;
    QSpinBox *timeTrackingClock;
    QSpinBox *timeTrackingClockgettimereal;
    QSpinBox *timeTrackingClockgettimemonotonic;
    QSpinBox *timeTrackingSdlgetticks;
    QSpinBox *timeTrackingSdlgetperformancecounter;
    QSpinBox *timeTrackingGetTickCount;
    QSpinBox *timeTrackingGetTickCount64;
    QSpinBox *timeTrackingQueryPerformanceCounter;

public slots:
    void loadConfig();
    void saveConfig();
};

#endif
