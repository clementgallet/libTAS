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
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>

#include "TimeTraceWindow.h"
#include "TimeTraceModel.h"
#include "../Context.h"

TimeTraceWindow::TimeTraceWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Time Trace");

    /* Table */
    timeTraceView = new QTableView(this);
    timeTraceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    timeTraceView->setSelectionMode(QAbstractItemView::SingleSelection);
    timeTraceView->setShowGrid(false);
    timeTraceView->setAlternatingRowColors(true);
    timeTraceView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    timeTraceView->horizontalHeader()->setHighlightSections(false);
    timeTraceView->verticalHeader()->setDefaultSectionSize(timeTraceView->verticalHeader()->minimumSectionSize());
    timeTraceView->verticalHeader()->hide();
    timeTraceView->setSortingEnabled(true);
    timeTraceView->sortByColumn(0, Qt::AscendingOrder);

    timeTraceModel = new TimeTraceModel(context);
    proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(timeTraceModel);
    timeTraceView->setModel(proxyModel);

    connect(timeTraceView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TimeTraceWindow::slotStacktrace);

    /* Text Edit */
    stackTraceText = new QPlainTextEdit();
    stackTraceText->setReadOnly(true);

    /* Buttons */
    QPushButton *chooseHashButton = new QPushButton(tr("Pick Hash"));
    connect(chooseHashButton, &QAbstractButton::clicked, this, &TimeTraceWindow::slotChooseHash);

    QPushButton *clearHashButton = new QPushButton(tr("Clear Hash"));
    connect(clearHashButton, &QAbstractButton::clicked, this, &TimeTraceWindow::slotClearHash);

    startButton = new QPushButton(context->config.sc.time_trace?tr("Stop Trace"):tr("Start Trace"));
    connect(startButton, &QAbstractButton::clicked, this, &TimeTraceWindow::slotStart);

    QPushButton *clearButton = new QPushButton(tr("Clear Trace"));
    connect(clearButton, &QAbstractButton::clicked, this, &TimeTraceWindow::slotClear);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(chooseHashButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(clearHashButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);

    /* Layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(timeTraceView, 1);
    mainLayout->addWidget(stackTraceText);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void TimeTraceWindow::slotStacktrace(const QItemSelection &selected, const QItemSelection &deselected)
{
    const QModelIndexList indexes = selected.indexes();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    const QModelIndex sourceIndex = proxyModel->mapToSource(indexes[0]);
    const QString backtrace = QString(timeTraceModel->getStacktrace(sourceIndex.row()).c_str());
    stackTraceText->setPlainText(backtrace);
}

void TimeTraceWindow::slotStart()
{
    context->config.sc.time_trace = !context->config.sc.time_trace;
    context->config.sc_modified = true;
    startButton->setText(context->config.sc.time_trace?tr("Stop Trace"):tr("Start Trace"));
}

void TimeTraceWindow::slotClear()
{
    timeTraceModel->clearData();
    stackTraceText->clear();
}

void TimeTraceWindow::slotChooseHash()
{
    const QModelIndex index = timeTraceView->selectionModel()->currentIndex();

    if (!index.isValid()) {
        return;
    }

    const QModelIndex sourceIndex = proxyModel->mapToSource(index);

    auto it = timeTraceModel->time_calls_map.begin();
    std::advance(it, sourceIndex.row());
    context->config.sc.busy_loop_hash = it->first;
    context->config.sc_modified = true;
}

void TimeTraceWindow::slotClearHash()
{
    context->config.sc.busy_loop_hash = 0;
    context->config.sc_modified = true;
}

QSize TimeTraceWindow::sizeHint() const
{
    return QSize(600, 600);
}
