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

#ifndef LIBTAS_INPUTEDITORWINDOW_H_INCLUDED
#define LIBTAS_INPUTEDITORWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QMainWindow>

#include "../Context.h"

class InputEditorView;

class InputEditorWindow : public QMainWindow {
    Q_OBJECT

public:
    InputEditorWindow(Context *c, QWidget *parent = Q_NULLPTR);
    void resetInputs();
    QSize sizeHint() const override;

    /* Update UI elements when config has changed */
    void update_config();
    
    InputEditorView *inputEditorView;

public slots:
    void isWindowVisible(bool &visible);
    void scrollingSlot(bool checked);
    void rewindSlot(bool checked);
    void fastforwardSlot(bool checked);
    
private:
    Context *context;
    QAction* scrollingAct;
    QAction* rewindAct;
    QAction* fastforwardAct;
};

#endif
