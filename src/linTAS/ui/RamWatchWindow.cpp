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

#include <QTableView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHeaderView>

#include "RamWatchWindow.h"

RamWatchWindow::RamWatchWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Ram Watch");

    /* Table */
    ramWatchView = new QTableView(this);
    ramWatchView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ramWatchView->setSelectionMode(QAbstractItemView::SingleSelection);
    ramWatchView->setShowGrid(false);
    ramWatchView->setAlternatingRowColors(true);
    ramWatchView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(addWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(editWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(removeWatch, QDialogButtonBox::ActionRole);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(ramWatchView);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    editWindow = new RamWatchEditWindow();
}

void RamWatchWindow::update()
{
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
    static int index = 0;

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
    const QModelIndex index = ramWatchView->selectionModel()->currentIndex();
    ramWatchView->selectionModel()->clear();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    int row = index.row();
    ramWatchModel->removeWatch(row);
}
