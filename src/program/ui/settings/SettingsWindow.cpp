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

#include <QtWidgets/QLabel>
// #include <QtWidgets/QFileDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

#include "SettingsWindow.h"
#include "WrapInScrollArea.h"
#include "RuntimePane.h"
#include "AudioPane.h"
#include "InputPane.h"
#include "MoviePane.h"
#include "VideoPane.h"
#include "DebugPane.h"
#include "GameSpecificPane.h"
#include "../../Context.h"

SettingsWindow::SettingsWindow(Context* c, QWidget *parent) : QMainWindow(parent), context(c)
{
    setWindowTitle("Settings");

    /* Main mayout */
    QVBoxLayout* layout = new QVBoxLayout;

    tabWidget = new QTabWidget();
    layout->addWidget(tabWidget);

    rp = new RuntimePane(c);
    mp = new MoviePane(c);
    ip = new InputPane(c);
    ap = new AudioPane(c);
    vp = new VideoPane(c);
    gp = new DebugPane(c);
    gsp = new GameSpecificPane(c);

    tabWidget->addTab(GetWrappedWidget(rp, this, 125, 100), tr("Runtime"));
    tabWidget->addTab(GetWrappedWidget(mp, this, 125, 100), tr("Movie"));
    tabWidget->addTab(GetWrappedWidget(ip, this, 125, 100), tr("Input"));
    tabWidget->addTab(GetWrappedWidget(ap, this, 125, 100), tr("Audio"));
    tabWidget->addTab(GetWrappedWidget(vp, this, 125, 100), tr("Video"));
    tabWidget->addTab(GetWrappedWidget(gp, this, 125, 100), tr("Debug"));
    tabWidget->addTab(GetWrappedWidget(gsp, this, 125, 100), tr("Game-specific"));

    // Dialog box buttons
    QDialogButtonBox* closeBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(closeBox, &QDialogButtonBox::rejected, this, &SettingsWindow::save);

    layout->addWidget(closeBox);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void SettingsWindow::openRuntimeTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(RuntimeTab));
    show();
}

void SettingsWindow::openMovieTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(MovieTab));
    show();
}

void SettingsWindow::openInputTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(InputTab));
    show();
}

void SettingsWindow::openAudioTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(AudioTab));
    show();
}

void SettingsWindow::openVideoTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(VideoTab));
    show();
}

void SettingsWindow::openDebugTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(DebugTab));
    show();
}

void SettingsWindow::openGameSpecificTab()
{
    tabWidget->setCurrentIndex(static_cast<int>(GameSpecificTab));
    show();
}

void SettingsWindow::save()
{
    context->config.save(context->gamepath);
    hide();
}

void SettingsWindow::update(int status)
{
    rp->update(status);
    mp->update(status);
    ip->update(status);
    ap->update(status);
    vp->update(status);
    gp->update(status);
    gsp->update(status);
}
