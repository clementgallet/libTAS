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

#ifndef LIBTAS_CONTROLLERTABWINDOW_H_INCLUDED
#define LIBTAS_CONTROLLERTABWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>

#include "../../shared/AllInputs.h"

/* Forward declaration */
struct Context;
class ControllerWidget;

class ControllerTabWindow : public QDialog {
    Q_OBJECT

public:
    ControllerTabWindow(Context *c, QWidget *parent = Q_NULLPTR);

    ControllerWidget *controllers[AllInputs::MAXJOYS];
    Context *context;

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

public slots:
    void slotButtonToggle(int id, int button, bool pressed);
    void slotSetInputs(AllInputs &ai);
    void slotGetInputs(const AllInputs &ai);

};

#endif
