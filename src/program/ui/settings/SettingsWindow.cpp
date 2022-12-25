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

SettingsWindow::SettingsWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Settings");

    /* Main mayout */
    QVBoxLayout* layout = new QVBoxLayout;

    QTabWidget* tabWidget = new QTabWidget();
    layout->addWidget(tabWidget);

    tabWidget->addTab(GetWrappedWidget(new RuntimePane(c), this, 125, 100), tr("Runtime"));
    tabWidget->addTab(GetWrappedWidget(new MoviePane(c), this, 125, 100), tr("Movie"));
    tabWidget->addTab(GetWrappedWidget(new InputPane(c), this, 125, 100), tr("Input"));
    tabWidget->addTab(GetWrappedWidget(new AudioPane(c), this, 125, 100), tr("Audio"));
    tabWidget->addTab(GetWrappedWidget(new VideoPane(c), this, 125, 100), tr("Video"));
    tabWidget->addTab(GetWrappedWidget(new DebugPane(c), this, 125, 100), tr("Debug"));
    tabWidget->addTab(GetWrappedWidget(new GameSpecificPane(c), this, 125, 100), tr("Game-specific"));

    // Dialog box buttons
    QDialogButtonBox* closeBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(closeBox, &QDialogButtonBox::rejected, this, &SettingsWindow::save);

    layout->addWidget(closeBox);

    setLayout(layout);
}

void SettingsWindow::save()
{
    context->config.save(context->gamepath);
    reject();
}
