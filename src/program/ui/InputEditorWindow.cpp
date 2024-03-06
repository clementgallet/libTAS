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

#include "InputEditorWindow.h"
#include "InputEditorView.h"
#include "InputEditorModel.h"
#include "MarkerView.h"
#include "MarkerModel.h"
#include "MainWindow.h"

#include "Context.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QGroupBox>

InputEditorWindow::InputEditorWindow(Context* c, QWidget *parent) : QMainWindow(parent), context(c)
{
    setWindowTitle("Input Editor");

    /* Table */
    inputEditorView = new InputEditorView(c, this, parent);

    /* Markers */
    markerView = new MarkerView(c, this, parent);

    /* Signals */
    connect(markerView, &MarkerView::seekSignal, inputEditorView->inputEditorModel, &InputEditorModel::seekToFrame);
    connect(markerView, &MarkerView::scrollSignal, inputEditorView, &InputEditorView::scrollToFrame);
    connect(inputEditorView, &InputEditorView::addMarkerSignal, markerView->markerModel, &MarkerModel::addMarker);
    connect(inputEditorView, &InputEditorView::removeMarkerSignal, markerView->markerModel, &MarkerModel::removeMarker);


    QGroupBox* markerBox = new QGroupBox(tr("Markers"));
    QVBoxLayout* markerLayout = new QVBoxLayout;
    markerLayout->addWidget(markerView);
    markerBox->setLayout(markerLayout);

    /* Side View */
    QVBoxLayout *sideLayout = new QVBoxLayout;
    sideLayout->addWidget(markerBox);

    /* Main menu */
    QMenu* menu = menuBar()->addMenu(tr("Frames"));
    inputEditorView->fillMenu(menu);

    QMenu* optionMenu = menuBar()->addMenu(tr("Options"));
    
    scrollingAct = optionMenu->addAction(tr("Disable autoscrolling"), this,
        [=](bool checked){context->config.editor_autoscroll = !checked;});
    scrollingAct->setCheckable(true);

    rewindAct = optionMenu->addAction(tr("Rewind seeks to current frame"), this,
        [=](bool checked){context->config.editor_rewind_seek = checked;});

    rewindAct->setCheckable(true);

    fastforwardAct = optionMenu->addAction(tr("Disable fastforward during rewind"), this,
        [=](bool checked){context->config.editor_rewind_fastforward = !checked;});

    fastforwardAct->setCheckable(true);

    markerPauseAct = optionMenu->addAction(tr("Autopause on markers"), this,
        [=](bool checked){context->config.editor_marker_pause = checked;});

    markerPauseAct->setCheckable(true);

    /* Layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(inputEditorView);
    mainLayout->addLayout(sideLayout);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    
    update_config();
}

void InputEditorWindow::update_config()
{
    scrollingAct->setChecked(!context->config.editor_autoscroll);
    rewindAct->setChecked(context->config.editor_rewind_seek);
    fastforwardAct->setChecked(!context->config.editor_rewind_fastforward);
    markerPauseAct->setChecked(context->config.editor_marker_pause);
}

QSize InputEditorWindow::sizeHint() const
{
    QSize viewSize = inputEditorView->sizeHint();
    return QSize(viewSize.width() + inputEditorView->scrollBarWidth, 600);
}

void InputEditorWindow::resetInputs()
{
    inputEditorView->resetInputs();
    markerView->resetMarkers();
}

void InputEditorWindow::isWindowVisible(bool &visible)
{
    visible = isVisible();
}
