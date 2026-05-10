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

#include "SettingsWindow.h"
#include "WrapInScrollArea.h"
#include "RuntimePane.h"
#include "AudioPane.h"
#include "InputPane.h"
#include "MoviePane.h"
#include "VideoPane.h"
#include "DebugPane.h"
#include "GameSpecificPane.h"
#include "PathPane.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

SettingsWindow::SettingsWindow(Context* c, QWidget *parent) : QMainWindow(parent), context(c)
{
    setWindowTitle("Settings");
    currentStatus = context->status;

    /* Main mayout */
    QVBoxLayout* layout = new QVBoxLayout;

    tabWidget = new QTabWidget();
    layout->addWidget(tabWidget);

    tabWidget->addTab(new QWidget(this), tr("Runtime"));
    tabWidget->addTab(new QWidget(this), tr("Movie"));
    tabWidget->addTab(new QWidget(this), tr("Input"));
    tabWidget->addTab(new QWidget(this), tr("Audio"));
    tabWidget->addTab(new QWidget(this), tr("Video"));
    tabWidget->addTab(new QWidget(this), tr("Debug"));
    tabWidget->addTab(new QWidget(this), tr("Game-specific"));
    tabWidget->addTab(new QWidget(this), tr("Paths"));

    for (int index = 0; index < tabWidget->count(); ++index) {
        tabPages[index] = tabWidget->widget(index);
    }

    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        ensureTab(static_cast<TabIndex>(index));
    });

    // Dialog box buttons
    QDialogButtonBox* closeBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(closeBox, &QDialogButtonBox::rejected, this, &SettingsWindow::save);

    layout->addWidget(closeBox);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    ensureTab(RuntimeTab);
}

void SettingsWindow::openRuntimeTab()
{
    openTab(RuntimeTab);
}

void SettingsWindow::openMovieTab()
{
    openTab(MovieTab);
}

void SettingsWindow::openInputTab()
{
    openTab(InputTab);
}

void SettingsWindow::openAudioTab()
{
    openTab(AudioTab);
}

void SettingsWindow::openVideoTab()
{
    openTab(VideoTab);
}

void SettingsWindow::openDebugTab()
{
    openTab(DebugTab);
}

void SettingsWindow::openGameSpecificTab()
{
    openTab(GameSpecificTab);
}

void SettingsWindow::openPathTab()
{
    openTab(PathTab);
}

void SettingsWindow::openTab(TabIndex index)
{
    ensureTab(index);
    tabWidget->setCurrentIndex(static_cast<int>(index));
    show();
    raise();
    activateWindow();
}

QWidget *SettingsWindow::createTabWidget(TabIndex index)
{
    switch (index) {
    case RuntimeTab:
        if (!rp) {
            rp = new RuntimePane(context);
            rp->loadConfig();
            rp->update(currentStatus);
            return GetWrappedWidget(rp, this, 125, 100);
        }
        return tabPages[RuntimeTab];
    case MovieTab:
        if (!mp) {
            mp = new MoviePane(context);
            mp->loadConfig();
            mp->update(currentStatus);
            return GetWrappedWidget(mp, this, 125, 100);
        }
        return tabPages[MovieTab];
    case InputTab:
        if (!ip) {
            ip = new InputPane(context);
            ip->loadConfig();
            ip->update(currentStatus);
            return GetWrappedWidget(ip, this, 125, 100);
        }
        return tabPages[InputTab];
    case AudioTab:
        if (!ap) {
            ap = new AudioPane(context);
            ap->loadConfig();
            ap->update(currentStatus);
            return GetWrappedWidget(ap, this, 125, 100);
        }
        return tabPages[AudioTab];
    case VideoTab:
        if (!vp) {
            vp = new VideoPane(context);
            vp->loadConfig();
            vp->update(currentStatus);
            return GetWrappedWidget(vp, this, 125, 100);
        }
        return tabPages[VideoTab];
    case DebugTab:
        if (!gp) {
            gp = new DebugPane(context);
            gp->loadConfig();
            gp->update(currentStatus);
            return GetWrappedWidget(gp, this, 125, 100);
        }
        return tabPages[DebugTab];
    case GameSpecificTab:
        if (!gsp) {
            gsp = new GameSpecificPane(context);
            gsp->loadConfig();
            gsp->update(currentStatus);
            return GetWrappedWidget(gsp, this, 125, 100);
        }
        return tabPages[GameSpecificTab];
    case PathTab:
        if (!pp) {
            pp = new PathPane(context);
            pp->loadConfig();
            pp->update(currentStatus);
            return GetWrappedWidget(pp, this, 125, 100);
        }
        return tabPages[PathTab];
    }

    return nullptr;
}

void SettingsWindow::ensureTab(TabIndex index)
{
    const int tabIndex = static_cast<int>(index);
    if ((tabIndex < 0) || (tabIndex >= tabWidget->count())) {
        return;
    }

    QWidget *page = createTabWidget(index);
    QWidget *oldPage = tabPages[tabIndex];
    if (!page || (page == oldPage)) {
        return;
    }

    QString label = tabWidget->tabText(tabIndex);
    tabPages[tabIndex] = page;
    tabWidget->removeTab(tabIndex);
    tabWidget->insertTab(tabIndex, page, label);
    if (oldPage) {
        oldPage->deleteLater();
    }
}

void SettingsWindow::save()
{
    context->config.save(context->gamepath);
    hide();
}

void SettingsWindow::loadConfig()
{
    if (rp) rp->loadConfig();
    if (mp) mp->loadConfig();
    if (ip) ip->loadConfig();
    if (ap) ap->loadConfig();
    if (vp) vp->loadConfig();
    if (gp) gp->loadConfig();
    if (gsp) gsp->loadConfig();
    if (pp) pp->loadConfig();
}

void SettingsWindow::update(int status)
{
    currentStatus = status;
    if (rp) rp->update(status);
    if (mp) mp->update(status);
    if (ip) ip->update(status);
    if (ap) ap->update(status);
    if (vp) vp->update(status);
    if (gp) gp->update(status);
    if (gsp) gsp->update(status);
    if (pp) pp->update(status);
}
