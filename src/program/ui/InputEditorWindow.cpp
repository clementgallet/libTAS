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

#include "InputEditorWindow.h"
#include "InputEditorView.h"
#include "InputEditorModel.h"
#include "MarkerView.h"
#include "MarkerModel.h"
#include "InputChangeLogWindow.h"
#include "MainWindow.h"

#include "Context.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QSplitter>

InputEditorWindow::InputEditorWindow(Context* c, MovieFile *movie, QWidget *parent) : QMainWindow(parent), context(c)
{
    setWindowTitle("Input Editor");

    /* Table */
    inputEditorView = new InputEditorView(c, movie, this);

    /* Panels/Windows */
    markerView = new MarkerView(c, movie, this);
    inputChangeLogWindow = new InputChangeLogWindow(c, movie, this);

    /* Signals */
    connect(markerView, &MarkerView::seekSignal, inputEditorView->inputEditorModel, &InputEditorModel::seekToFrame);
    connect(markerView, &MarkerView::scrollSignal, inputEditorView, &InputEditorView::scrollToFrame);
    connect(inputEditorView, &InputEditorView::addMarkerSignal, markerView->markerModel, &MarkerModel::addMarker);
    connect(inputEditorView, &InputEditorView::removeMarkerSignal, markerView->markerModel, &MarkerModel::removeMarker);
    connect(inputEditorView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &InputEditorWindow::updateStatusBar);
    connect(inputEditorView->inputEditorModel, &InputEditorModel::stateLoaded, this, &InputEditorWindow::updateProgressBar);

    markerBox = new QGroupBox(tr("Markers"));
    QVBoxLayout* markerLayout = new QVBoxLayout;
    markerLayout->addWidget(markerView);
    markerBox->setLayout(markerLayout);

    /* Side View */
    QVBoxLayout *sideLayout = new QVBoxLayout;
    sideLayout->addWidget(markerBox);

    /* Main menu */
    QMenu* menu = menuBar()->addMenu(tr("Frames"));
    inputEditorView->fillMenu(menu);

    QMenu* panelMenu = menuBar()->addMenu(tr("Panels"));

    markerPanelAct = panelMenu->addAction(tr("Markers"), this,
        [=](bool checked){context->config.editor_panel_marker = checked; markerBox->setVisible(checked);});

    markerPanelAct->setCheckable(true);

    panelMenu->addAction(tr("Change Log"), inputChangeLogWindow, &InputChangeLogWindow::show);

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

    /* Status bar */
    statusFrame = new QLabel(tr("No frame selected"));
    statusBar()->addWidget(statusFrame);
    statusSeek = new QProgressBar();
    statusSeek->setMinimum(0);
    statusSeek->setMaximum(0);
    statusSeek->setMaximumHeight(statusBar()->height()*0.6);

    /* Layout */
    QSplitter *mainLayout = new QSplitter;
    mainLayout->addWidget(inputEditorView);
    QWidget *sideWidget = new QWidget;
    sideWidget->setLayout(sideLayout);
    mainLayout->addWidget(sideWidget);

    setCentralWidget(mainLayout);
    
    update_config();
}

void InputEditorWindow::update()
{
    inputEditorView->update();
    updateProgressBar();
}

void InputEditorWindow::update_config()
{
    markerPanelAct->setChecked(context->config.editor_panel_marker);
    markerBox->setVisible(context->config.editor_panel_marker);
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
    markerView->resetMarkers();
}

void InputEditorWindow::isWindowVisible(bool &visible)
{
    visible = isVisible();
}

void InputEditorWindow::updateStatusBar()
{
    /* Seeking take priority over selected frames */
    if (context->seek_frame)
        return;

    const QModelIndexList indexes = inputEditorView->selectionModel()->selectedRows();

    if (indexes.count() == 0)
        statusFrame->setText(tr("No frame selected"));
    else if (indexes.count() == 1)
        statusFrame->setText(tr("1 frame selected"));
    else
        statusFrame->setText(QString(tr("%1 frames selected")).arg(indexes.count()));
}

void InputEditorWindow::updateProgressBar()
{
    /* Show progress bar when seeking */
    static bool is_seeking = false;
    
    if (!context->seek_frame) {
        if (is_seeking) {
            is_seeking = false;
            statusBar()->removeWidget(statusSeek);
            statusBar()->addWidget(statusFrame);
            statusFrame->show();
            statusSeek->setMinimum(0);
            statusSeek->setMaximum(0);
            statusSeek->setFormat(tr("Loading savestate"));
        }
        return;
    }

    if (!is_seeking) {
        is_seeking = true;
        statusBar()->removeWidget(statusFrame);
        statusBar()->addWidget(statusSeek);
        statusSeek->show();
    }

    /* Set minimum and maximum on the first time after state loading.
     * Before that, it will show a busy indicator */
    if ((statusSeek->minimum() == 0) && (context->framecount <= context->seek_frame)) {
        statusSeek->setMinimum(context->framecount);
        statusSeek->setMaximum(context->seek_frame);
        statusSeek->setFormat(QString(tr("Seeking to frame %1")).arg(context->seek_frame));
    }
    
    /* Seeking frame may change during seeking */
    if ((statusSeek->minimum() != 0) && (statusSeek->maximum() != context->seek_frame)) {
        statusSeek->setMaximum(context->seek_frame);
        statusSeek->setFormat(QString(tr("Seeking to frame %1")).arg(context->seek_frame));
    }

    statusSeek->setValue(context->framecount);
}
