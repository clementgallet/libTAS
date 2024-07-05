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

#ifndef LIBTAS_INPUTEDITORWINDOW_H_INCLUDED
#define LIBTAS_INPUTEDITORWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QGroupBox>

/* Forward declaration */
struct Context;
class InputEditorView;
class MarkerView;

class InputEditorWindow : public QMainWindow {
    Q_OBJECT

public:
    InputEditorWindow(Context *c, QWidget *parent = Q_NULLPTR);
    void resetInputs();
    QSize sizeHint() const override;

    /* Update UI elements when config has changed */
    void update();
    void update_config();
    
    InputEditorView *inputEditorView;
    MarkerView *markerView;

public slots:
    void isWindowVisible(bool &visible);
    void updateStatusBar();
    void updateProgressBar();
    
private:
    Context *context;
    QGroupBox* markerBox;
    QAction* markerPanelAct;
    QAction* scrollingAct;
    QAction* rewindAct;
    QAction* fastforwardAct;
    QAction* markerPauseAct;
    QLabel* statusFrame;
    QProgressBar* statusSeek;
};

#endif
