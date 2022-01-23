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

#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>

#include "PointerScanWindow.h"
#include "PointerScanModel.h"
#include "RamWatchWindow.h"
#include "RamWatchEditWindow.h"
#include "MainWindow.h"
#include "../ramsearch/CompareOperations.h"
#include "../ramsearch/IRamWatchDetailed.h"
#include "../ramsearch/RamWatchDetailed.h"
#include "../ramsearch/BaseAddresses.h"

PointerScanWindow::PointerScanWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Pointer Scan");

    /* Table */
    pointerScanView = new QTableView(this);
    pointerScanView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pointerScanView->setSelectionMode(QAbstractItemView::SingleSelection);
    pointerScanView->setShowGrid(false);
    pointerScanView->setAlternatingRowColors(true);
    pointerScanView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    pointerScanView->horizontalHeader()->setHighlightSections(false);
    pointerScanView->verticalHeader()->hide();
    pointerScanView->setSortingEnabled(true);
    pointerScanView->sortByColumn(0, Qt::AscendingOrder);

    pointerScanModel = new PointerScanModel(context);
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(pointerScanModel);
    pointerScanView->setModel(proxyModel);

    /* Progress bar */
    searchProgress = new QProgressBar();
    searchProgress->setRange(0, 100);
    connect(pointerScanModel, &PointerScanModel::signalProgress, searchProgress, &QProgressBar::setValue);

    scanCount = new QLabel();
    searchProgress->hide();

    QVBoxLayout *scanLayout = new QVBoxLayout;
    scanLayout->addWidget(pointerScanView);
    scanLayout->addWidget(searchProgress);
    scanLayout->addWidget(scanCount);

    /* Form */

    addressInput = new QLineEdit();
    maxLevelInput = new QSpinBox();
    maxLevelInput->setMaximum(10);
    maxLevelInput->setValue(3);
    maxOffsetInput = new QSpinBox();
    maxOffsetInput->setMaximum(10000);
    maxOffsetInput->setValue(1000);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Address:")), addressInput);
    formLayout->addRow(new QLabel(tr("Max level:")), maxLevelInput);
    formLayout->addRow(new QLabel(tr("Max offset:")), maxOffsetInput);

    /* Buttons */
    QPushButton *searchButton = new QPushButton(tr("Search"));
    connect(searchButton, &QAbstractButton::clicked, this, &PointerScanWindow::slotSearch);

    QPushButton *addButton = new QPushButton(tr("Add Watch"));
    connect(addButton, &QAbstractButton::clicked, this, &PointerScanWindow::slotAdd);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(searchButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);

    /* Create the options layout */
    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addLayout(formLayout);
    optionLayout->addStretch(1);
    optionLayout->addWidget(buttonBox);

    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addLayout(scanLayout, 1);
    mainLayout->addLayout(optionLayout);

    setLayout(mainLayout);
}

void PointerScanWindow::slotSearch()
{
    bool ok;
    uintptr_t addr = addressInput->text().toULong(&ok, 16);

    if (!ok)
        return;

    int max_level = maxLevelInput->value();
    int max_offset = maxOffsetInput->value();

    scanCount->hide();
    searchProgress->show();

    pointerScanModel->findPointerChain(addr, max_level, max_offset);

    /* Update address count */
    searchProgress->hide();
    scanCount->show();
    scanCount->setText(QString("%1 results").arg(pointerScanModel->pointer_chains.size()));

    /* Sort results */
    for (int c=max_level; c>=0; c--) {
        pointerScanView->sortByColumn(c, Qt::AscendingOrder);
    }
}

void PointerScanWindow::slotAdd()
{
    const QModelIndex index = pointerScanView->selectionModel()->currentIndex();
    pointerScanView->selectionModel()->clear();

    /* If no watch was selected, return */
    if (!index.isValid()) {
        QMessageBox::critical(nullptr, "Error", QString("You must select an address to add a watch"));
        return;
    }

    /* Fill the watch edit window with parameters from the selected result */
    MainWindow *mw = qobject_cast<MainWindow*>(parent()->parent());
    if (mw) {
        bool ok;
        uintptr_t addr = addressInput->text().toULong(&ok, 16);
        const QModelIndex sourceIndex = proxyModel->mapToSource(index);
        const std::pair<uintptr_t, std::vector<int>> &chain = pointerScanModel->pointer_chains.at(sourceIndex.row());
        std::unique_ptr<IRamWatchDetailed> watch(new RamWatchDetailed<int>(addr));

        watch->isPointer = true;
        watch->base_address = chain.first;
        watch->base_file = BaseAddresses::getFileAndOffset(chain.first, watch->base_file_offset);
        watch->pointer_offsets = chain.second;
        std::reverse(watch->pointer_offsets.begin(), watch->pointer_offsets.end());
        mw->ramWatchWindow->editWindow->fill(watch);
        mw->ramWatchWindow->slotAdd();
    }
}
