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
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>

#include "InputEditorWindow.h"
#include "MainWindow.h"

InputEditorWindow::InputEditorWindow(Context* c, QWidget *parent) : QMainWindow(parent), context(c)
{
    setWindowTitle("Input Editor");

    /* Table */
    inputEditorView = new InputEditorView(c, this, parent);

    /* Main menu */
    QMenu* menu = menuBar()->addMenu(tr("Frames"));
    inputEditorView->fillMenu(menu);

    QMenu* optionMenu = menuBar()->addMenu(tr("Options"));
    
    scrollingAct = optionMenu->addAction(tr("Disable autoscrolling"), this, &InputEditorWindow::scrollingSlot);
    scrollingAct->setCheckable(true);

    rewindAct = optionMenu->addAction(tr("Rewind seeks to current frame"), this, &InputEditorWindow::rewindSlot);
    rewindAct->setCheckable(true);

    /* Layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(inputEditorView);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    
    update_config();
}

void InputEditorWindow::update_config()
{
    scrollingAct->setChecked(!context->config.editor_autoscroll);
    rewindAct->setChecked(context->config.editor_rewind_seek);
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

void InputEditorWindow::scrollingSlot(bool checked)
{
    context->config.editor_autoscroll = !checked;
}

void InputEditorWindow::rewindSlot(bool checked)
{
    context->config.editor_rewind_seek = checked;
}
