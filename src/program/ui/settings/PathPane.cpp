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

#include "PathPane.h"
#include "tooltip/ToolTipComboBox.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>

#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>

PathPane::PathPane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void PathPane::initLayout()
{
    QGroupBox* generalBox = new QGroupBox(tr("General"));
    QGridLayout* generalLayout = new QGridLayout;
    generalBox->setLayout(generalLayout);

    dataPath = new QLineEdit();
    moviePath = new QLineEdit();
    statePath = new QLineEdit();
    
    browseDataPath = new QPushButton("Browse...");
    browseMoviePath = new QPushButton("Browse...");
    browseStatePath = new QPushButton("Browse...");

    openDataPath = new QPushButton("Open Dir...");
    openMoviePath = new QPushButton("Open Dir...");
    openStatePath = new QPushButton("Open Dir...");

    generalLayout->addWidget(new QLabel(tr("Data path:")), 0, 0);
    generalLayout->addWidget(dataPath, 0, 1);
    generalLayout->addWidget(browseDataPath, 0, 2);
    generalLayout->addWidget(openDataPath, 0, 3);
    generalLayout->addWidget(new QLabel(tr("Backup movie path:")), 1, 0);
    generalLayout->addWidget(moviePath, 1, 1);
    generalLayout->addWidget(browseMoviePath, 1, 2);
    generalLayout->addWidget(openMoviePath, 1, 3);
    generalLayout->addWidget(new QLabel(tr("Savestates path:")), 2, 0);
    generalLayout->addWidget(statePath, 2, 1);
    generalLayout->addWidget(browseStatePath, 2, 2);
    generalLayout->addWidget(openStatePath, 2, 3);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);    
}

void PathPane::initSignals()
{
    connect(browseDataPath, &QAbstractButton::clicked, this, &PathPane::slotBrowseDataPath);
    connect(browseMoviePath, &QAbstractButton::clicked, this, &PathPane::slotBrowseMoviePath);
    connect(browseStatePath, &QAbstractButton::clicked, this, &PathPane::slotBrowseStatePath);
    connect(openDataPath, &QAbstractButton::clicked, this, &PathPane::slotOpenDataPath);
    connect(openMoviePath, &QAbstractButton::clicked, this, &PathPane::slotOpenMoviePath);
    connect(openStatePath, &QAbstractButton::clicked, this, &PathPane::slotOpenStatePath);
}

void PathPane::initToolTips()
{
}

void PathPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void PathPane::loadConfig()
{
    dataPath->setText(context->config.datadir.c_str());
    moviePath->setText(context->config.tempmoviedir.c_str());
    statePath->setText(context->config.savestatedir.c_str());
}

void PathPane::saveConfig()
{
    context->config.datadir = dataPath->text().toStdString();
    context->config.tempmoviedir = moviePath->text().toStdString();
    context->config.savestatedir = statePath->text().toStdString();
}

void PathPane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        break;
    case Context::STARTING:
        break;
    }
}

void PathPane::slotBrowseDataPath()
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Choose an data path"), dataPath->text());
    if (!filename.isNull())
        dataPath->setText(filename);
}

void PathPane::slotBrowseMoviePath()
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Choose an temporary movie path"), moviePath->text());
    if (!filename.isNull())
        moviePath->setText(filename);
}

void PathPane::slotBrowseStatePath()
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Choose an data path"), statePath->text());
    if (!filename.isNull())
        statePath->setText(filename);
}

void PathPane::slotOpenDataPath()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(dataPath->text()));
}

void PathPane::slotOpenMoviePath()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(moviePath->text()));
}

void PathPane::slotOpenStatePath()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(statePath->text()));
}
