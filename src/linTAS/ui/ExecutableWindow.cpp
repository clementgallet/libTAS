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

#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ExecutableWindow.h"

ExecutableWindow::ExecutableWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Executable Options");

    /* Run path */
    runPath = new QLineEdit();
    // runPath->setReadOnly(true);
    runPath->setClearButtonEnabled(true);
    runPath->setMinimumWidth(400);

    browseRunPath = new QPushButton("Browse...");
    connect(browseRunPath, &QAbstractButton::clicked, this, &ExecutableWindow::slotBrowseRunPath);

    /* Run path layout */
    QGroupBox *runPathGroupBox = new QGroupBox(tr("Run path"));
    QHBoxLayout *runPathLayout = new QHBoxLayout;
    runPathLayout->addWidget(runPath);
    runPathLayout->addWidget(browseRunPath);
    runPathGroupBox->setLayout(runPathLayout);

    /* Lib path */
    libPath = new QLineEdit();
    // libPath->setReadOnly(true);
    libPath->setClearButtonEnabled(true);
    libPath->setMinimumWidth(400);

    browseLibPath = new QPushButton("Browse...");
    connect(browseLibPath, &QAbstractButton::clicked, this, &ExecutableWindow::slotBrowseLibPath);

    /* Lib path layout */
    QGroupBox *libPathGroupBox = new QGroupBox(tr("Library path"));
    QHBoxLayout *libPathLayout = new QHBoxLayout;
    libPathLayout->addWidget(libPath);
    libPathLayout->addWidget(browseLibPath);
    libPathGroupBox->setLayout(libPathLayout);

    /* Buttons */
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExecutableWindow::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ExecutableWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(runPathGroupBox);
    mainLayout->addWidget(libPathGroupBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    update_config();
}

void ExecutableWindow::update_config()
{
    runPath->setText(context->config.rundir.c_str());
    libPath->setText(context->config.libdir.c_str());
}

void ExecutableWindow::slotOk()
{
    context->config.rundir = runPath->text().toStdString();
    context->config.libdir = libPath->text().toStdString();

    /* Close window */
    accept();
}

void ExecutableWindow::slotBrowseRunPath()
{
    QString defaultPath = context->config.rundir.empty()?QString(context->gamepath.c_str()):runPath->text();
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Choose an run directory"), defaultPath);
    if (!dirname.isNull())
        runPath->setText(dirname);
}

void ExecutableWindow::slotBrowseLibPath()
{
    QString defaultPath = context->config.libdir.empty()?QString(context->gamepath.c_str()):libPath->text();
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Choose an library directory"), defaultPath);
    if (!dirname.isNull())
        libPath->setText(dirname);
}
