/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ExecutableWindow.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>

#include <utility>

namespace {

bool isValidEnvName(const QString& name)
{
    if (name.isEmpty())
        return false;

    const QChar first = name.at(0);
    if (!first.isLetter() && first != '_')
        return false;

    for (const QChar& c : name) {
        if (!c.isLetterOrNumber() && c != '_')
            return false;
    }
    return true;
}

}

ExecutableWindow::ExecutableWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
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
    QGroupBox *runPathGroupBox = new QGroupBox(tr("Working directory"));
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

    /* Proton path */
    protonPath = new QLineEdit();
    protonPath->setMinimumWidth(400);
    browseProtonPath = new QPushButton("Browse...");
    connect(browseProtonPath, &QAbstractButton::clicked, this, &ExecutableWindow::slotBrowseProtonPath);

    /* Proton path layout */
    protonPathGroupBox = new QGroupBox(tr("Proton path"));
    protonPathGroupBox->setCheckable(true);
    QHBoxLayout *libProtonLayout = new QHBoxLayout;
    libProtonLayout->addWidget(protonPath);
    libProtonLayout->addWidget(browseProtonPath);
    protonPathGroupBox->setLayout(libProtonLayout);

    /* Environment variable overrides */
    envVarsTable = new QTableWidget(0, 2);
    envVarsTable->setHorizontalHeaderLabels({tr("Variable"), tr("Value")});
    envVarsTable->horizontalHeader()->setStretchLastSection(true);
    envVarsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    envVarsTable->verticalHeader()->setVisible(false);
    envVarsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    envVarsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    envVarsTable->setAlternatingRowColors(true);

    addEnvVarButton = new QPushButton("Add");
    removeEnvVarButton = new QPushButton("Remove");
    connect(addEnvVarButton, &QAbstractButton::clicked, this, &ExecutableWindow::slotAddEnvVar);
    connect(removeEnvVarButton, &QAbstractButton::clicked, this, &ExecutableWindow::slotRemoveEnvVar);

    QLabel *envVarHelp = new QLabel(tr("Variables defined here replace the launched game's environment values."));
    envVarHelp->setWordWrap(true);

    QHBoxLayout *envVarButtonsLayout = new QHBoxLayout;
    envVarButtonsLayout->addWidget(addEnvVarButton);
    envVarButtonsLayout->addWidget(removeEnvVarButton);
    envVarButtonsLayout->addStretch();

    QVBoxLayout *envVarsLayout = new QVBoxLayout;
    envVarsLayout->addWidget(envVarHelp);
    envVarsLayout->addWidget(envVarsTable);
    envVarsLayout->addLayout(envVarButtonsLayout);

    QGroupBox *envVarsGroupBox = new QGroupBox(tr("Environment variables (override)"));
    envVarsGroupBox->setLayout(envVarsLayout);

    /* Buttons */
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExecutableWindow::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ExecutableWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(runPathGroupBox);
    mainLayout->addWidget(libPathGroupBox);
    mainLayout->addWidget(envVarsGroupBox);
    mainLayout->addWidget(protonPathGroupBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    update_config();
}

void ExecutableWindow::update_config()
{
    runPath->setText(context->config.rundir.c_str());
    libPath->setText(context->config.libdir.c_str());

    envVarsTable->setRowCount(0);
    for (const auto& env_var : context->config.env_overrides) {
        const int row = envVarsTable->rowCount();
        envVarsTable->insertRow(row);
        envVarsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(env_var.first)));
        envVarsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(env_var.second)));
    }

    protonPathGroupBox->setChecked(context->config.use_proton);
    protonPath->setText(context->config.proton_path.c_str());
}

void ExecutableWindow::slotOk()
{
    std::vector<std::pair<std::string, std::string>> envOverrides;
    envOverrides.reserve(envVarsTable->rowCount());

    for (int row = 0; row < envVarsTable->rowCount(); ++row) {
        QTableWidgetItem *nameItem = envVarsTable->item(row, 0);
        QTableWidgetItem *valueItem = envVarsTable->item(row, 1);

        const QString name = nameItem ? nameItem->text().trimmed() : QString();
        const QString value = valueItem ? valueItem->text() : QString();

        if (name.isEmpty() && value.isEmpty())
            continue;

        if (!isValidEnvName(name)) {
            QMessageBox::warning(this, tr("Invalid environment variable"),
                                 tr("Row %1 has an invalid variable name. Use [A-Za-z_][A-Za-z0-9_]*.").arg(row + 1));
            envVarsTable->setCurrentCell(row, 0);
            return;
        }

        envOverrides.emplace_back(name.toStdString(), value.toStdString());
    }

    context->config.rundir = runPath->text().toStdString();
    context->config.libdir = libPath->text().toStdString();
    context->config.env_overrides = std::move(envOverrides);
    context->config.use_proton = protonPathGroupBox->isChecked();
    context->config.proton_path = protonPath->text().toStdString();

    /* Close window */
    accept();
}

void ExecutableWindow::slotBrowseRunPath()
{
    QString defaultPath = context->config.rundir.empty()?QString(context->gamepath.c_str()):runPath->text();
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Choose a working directory"), defaultPath);
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

void ExecutableWindow::slotBrowseProtonPath()
{
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Choose a proton directory"), protonPath->text());
    if (!dirname.isNull())
        protonPath->setText(dirname);
}

void ExecutableWindow::slotAddEnvVar()
{
    const int row = envVarsTable->rowCount();
    envVarsTable->insertRow(row);
    envVarsTable->setItem(row, 0, new QTableWidgetItem());
    envVarsTable->setItem(row, 1, new QTableWidgetItem());
    envVarsTable->setCurrentCell(row, 0);
    envVarsTable->editItem(envVarsTable->item(row, 0));
}

void ExecutableWindow::slotRemoveEnvVar()
{
    const int row = envVarsTable->currentRow();
    if (row >= 0)
        envVarsTable->removeRow(row);
}
