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

#include <QTableView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QFileDialog>

#include "RamWatchWindow.h"

RamWatchWindow::RamWatchWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Ram Watch");

    /* Table */
    ramWatchView = new QTableView(this);
    ramWatchView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ramWatchView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ramWatchView->setShowGrid(false);
    ramWatchView->setAlternatingRowColors(true);
    ramWatchView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ramWatchView->horizontalHeader()->setHighlightSections(false);
    ramWatchView->verticalHeader()->hide();

    ramWatchModel = new RamWatchModel();
    ramWatchView->setModel(ramWatchModel);

    /* Buttons */
    QPushButton *addWatch = new QPushButton(tr("Add Watch"));
    connect(addWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotAdd);

    QPushButton *editWatch = new QPushButton(tr("Edit Watch"));
    connect(editWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotEdit);

    QPushButton *removeWatch = new QPushButton(tr("Remove Watch"));
    connect(removeWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotRemove);

    QPushButton *scanWatch = new QPushButton(tr("Scan Pointer"));
    connect(scanWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotScanPointer);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(addWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(editWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(removeWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(scanWatch, QDialogButtonBox::ActionRole);

    /* Other buttons */
    QPushButton *saveWatch = new QPushButton(tr("Save Watches"));
    connect(saveWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotSave);

    QPushButton *loadWatch = new QPushButton(tr("Load Watches"));
    connect(loadWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotLoad);

    QDialogButtonBox *buttonBox2 = new QDialogButtonBox();
    buttonBox2->addButton(saveWatch, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(loadWatch, QDialogButtonBox::ActionRole);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(ramWatchView);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(buttonBox2);

    setLayout(mainLayout);

    editWindow = new RamWatchEditWindow(this);
    pointerScanWindow = new PointerScanWindow(c, this);
}

void RamWatchWindow::update()
{
    IRamWatchDetailed::game_pid = context->game_pid;
    ramWatchModel->update();
}

void RamWatchWindow::slotAdd()
{
    editWindow->exec();

    if (editWindow->ramwatch) {
        editWindow->ramwatch->game_pid = context->game_pid;
        ramWatchModel->addWatch(std::move(editWindow->ramwatch));
    }
}

void RamWatchWindow::slotGet(std::string &watch)
{
    static unsigned int index = 0;

    if (index >= ramWatchModel->ramwatches.size()) {
        /* We sent all watches, returning NULL */
        watch = "";
        index = 0;
        return;
    }

    watch = ramWatchModel->ramwatches[index]->label;
    watch += ": ";
    watch += ramWatchModel->ramwatches[index]->value_str();

    index++;
}

void RamWatchWindow::slotEdit()
{
    const QModelIndex index = ramWatchView->selectionModel()->currentIndex();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    int row = index.row();

    /* Fill and show the watch edit window */
    editWindow->fill(ramWatchModel->ramwatches.at(row));
    editWindow->exec();

    /* Modify the watch */
    if (editWindow->ramwatch) {
        editWindow->ramwatch->game_pid = context->game_pid;
        ramWatchModel->ramwatches[row] = std::move(editWindow->ramwatch);
        ramWatchModel->update();
    }
}

void RamWatchWindow::slotRemove()
{
    QModelIndexList indexlist = ramWatchView->selectionModel()->selectedRows();
    ramWatchView->selectionModel()->clear();

    /* If no watch was selected, return */
    if (indexlist.isEmpty())
        return;

    for (const QModelIndex index : indexlist) {
        ramWatchModel->removeWatch(index.row());
    }
}

void RamWatchWindow::slotScanPointer()
{
    const QModelIndex index = ramWatchView->selectionModel()->currentIndex();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    int row = index.row();

    /* Fill and show the watch edit window */
    pointerScanWindow->addressInput->setText(QString("%1").arg(ramWatchModel->ramwatches.at(row)->address, 0, 16));
    pointerScanWindow->exec();
}

void RamWatchWindow::slotSave()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Choose a watch file"), context->gamepath.c_str(), tr("watch files (*.wch)"));
    if (filename.isNull()) {
        return;
    }

    std::string watchFile = filename.toStdString();

    /* Check or add .wch extension */
    if ((watchFile.length() < 4) || (watchFile.compare(watchFile.length()-4, 4, ".wch") != 0)) {
        watchFile += ".wch";
    }

	QSettings watchSettings(QString(watchFile.c_str()), QSettings::IniFormat);
	watchSettings.setFallbacksEnabled(false);

    ramWatchModel->saveSettings(watchSettings);
}

void RamWatchWindow::slotLoad()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose a watch file"), context->gamepath.c_str(), tr("watch files (*.wch)"));
    if (filename.isNull()) {
        return;
    }

    std::string watchFile = filename.toStdString();

	QSettings watchSettings(QString(watchFile.c_str()), QSettings::IniFormat);
	watchSettings.setFallbacksEnabled(false);

    ramWatchModel->loadSettings(watchSettings);
}
