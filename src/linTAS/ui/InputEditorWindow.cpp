/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QHBoxLayout>

#include "InputEditorWindow.h"
#include "MainWindow.h"

InputEditorWindow::InputEditorWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
    setWindowTitle("Input Editor");

    /* Table */
    inputEditorView = new InputEditorView(c, this);

    /* Layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(inputEditorView);
    setLayout(mainLayout);
}

QSize InputEditorWindow::sizeHint() const
{
    return QSize(600, 600);
}

void InputEditorWindow::update()
{
    inputEditorView->update();
}

void InputEditorWindow::resetInputs()
{
    inputEditorView->resetInputs();
}


void InputEditorWindow::isWindowVisible(bool &visible)
{
    visible = isVisible();
}
