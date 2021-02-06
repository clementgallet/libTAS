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

#include <QtWidgets/QHBoxLayout>

#include "InputEditorWindow.h"
#include "MainWindow.h"

InputEditorWindow::InputEditorWindow(Context* c, QWidget *parent) : QDialog(nullptr)
{
    setWindowTitle("Input Editor");

    /* Table */
    inputEditorView = new InputEditorView(c, this, parent);

    /* Layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(inputEditorView);
    setLayout(mainLayout);
}

QSize InputEditorWindow::sizeHint() const
{
    QSize viewSize = inputEditorView->sizeHint();
    return QSize(viewSize.width(), 600);
}

void InputEditorWindow::resetInputs()
{
    inputEditorView->resetInputs();
}


void InputEditorWindow::isWindowVisible(bool &visible)
{
    visible = isVisible();
}
