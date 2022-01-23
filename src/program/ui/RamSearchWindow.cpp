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
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>

#include "RamSearchWindow.h"
#include "RamSearchModel.h"
#include "RamWatchWindow.h"
#include "RamWatchEditWindow.h"
#include "MainWindow.h"
#include "../ramsearch/CompareOperations.h"

#include <limits>

RamSearchWindow::RamSearchWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Ram Search");

    /* Table */
    ramSearchView = new QTableView(this);
    ramSearchView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ramSearchView->setSelectionMode(QAbstractItemView::SingleSelection);
    ramSearchView->setShowGrid(false);
    ramSearchView->setAlternatingRowColors(true);
    ramSearchView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ramSearchView->horizontalHeader()->setHighlightSections(false);
    ramSearchView->verticalHeader()->hide();

    ramSearchModel = new RamSearchModel(context);
    ramSearchView->setModel(ramSearchModel);

    /* Progress bar */
    searchProgress = new QProgressBar();
    connect(&ramSearchModel->memscanner, &MemScanner::signalProgress, searchProgress, &QProgressBar::setValue);

    watchCount = new QLabel();
    // watchCount->setHeight(searchProgress->height());
    searchProgress->hide();

    QVBoxLayout *watchLayout = new QVBoxLayout;
    watchLayout->addWidget(ramSearchView);
    watchLayout->addWidget(searchProgress);
    watchLayout->addWidget(watchCount);

    /* Memory regions */
    memSpecialBox = new QCheckBox("Exclude special regions");
    memSpecialBox->setChecked(true);
    memROBox = new QCheckBox("Exclude read-only regions");
    memROBox->setChecked(true);
    memExecBox = new QCheckBox("Exclude executable regions");
    memExecBox->setChecked(true);

    memGroupBox = new QGroupBox(tr("Included Memory Flags"));
    QVBoxLayout *memLayout = new QVBoxLayout;
    memLayout->addWidget(memSpecialBox);
    memLayout->addWidget(memROBox);
    memLayout->addWidget(memExecBox);
    memGroupBox->setLayout(memLayout);

    /* Comparisons */
    comparePreviousButton = new QRadioButton("Unknown/Previous Value");
    comparePreviousButton->setChecked(true);
    compareValueButton = new QRadioButton("Specific Value:");
    comparingValueBox = new QDoubleSpinBox();
    comparingValueBox->setRange(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::max());
    comparingValueBox->setDecimals(15);

    QGroupBox *compareGroupBox = new QGroupBox(tr("Compare To"));
    QVBoxLayout *compareLayout = new QVBoxLayout;
    compareLayout->addWidget(comparePreviousButton);
    compareLayout->addWidget(compareValueButton);
    compareLayout->addWidget(comparingValueBox);
    compareGroupBox->setLayout(compareLayout);

    /* Operators */
    operatorEqualButton = new QRadioButton("Equal To");
    operatorEqualButton->setChecked(true);
    operatorNotEqualButton = new QRadioButton("Not Equal To");
    operatorLessButton = new QRadioButton("Less Than");
    operatorGreaterButton = new QRadioButton("Greater Than");
    operatorLessEqualButton = new QRadioButton("Less Than Or Equal To");
    operatorGreaterEqualButton = new QRadioButton("Greater Than Or Equal To");
    operatorDifferenceButton = new QRadioButton("Different By");
    differenceValueBox = new QDoubleSpinBox();
    differenceValueBox->setRange(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::max());
    differenceValueBox->setDecimals(5);

    QGroupBox *operatorGroupBox = new QGroupBox(tr("Comparison Operator"));
    QGridLayout *operatorLayout = new QGridLayout;
    operatorLayout->addWidget(operatorEqualButton, 0, 0, 1, 2);
    operatorLayout->addWidget(operatorNotEqualButton, 1, 0, 1, 2);
    operatorLayout->addWidget(operatorLessButton, 2, 0, 1, 2);
    operatorLayout->addWidget(operatorGreaterButton, 3, 0, 1, 2);
    operatorLayout->addWidget(operatorLessEqualButton, 4, 0, 1, 2);
    operatorLayout->addWidget(operatorGreaterEqualButton, 5, 0, 1, 2);
    operatorLayout->addWidget(operatorDifferenceButton, 6, 0);
    operatorLayout->addWidget(differenceValueBox, 6, 1);
    operatorGroupBox->setLayout(operatorLayout);

    /* Format */
    typeBox = new QComboBox();
    QStringList typeList;
    typeList << "unsigned char" << "char" << "unsigned short" << "short";
    typeList << "unsigned int" << "int" << "unsigned int64" << "int64";
    typeList << "float" << "double";
    typeBox->addItems(typeList);
    typeBox->setCurrentText("int");

    displayBox = new QComboBox();
    displayBox->addItem("decimal");
    displayBox->addItem("hexadecimal");

    formatGroupBox = new QGroupBox(tr("Format"));
    QFormLayout *formatLayout = new QFormLayout;
    formatLayout->addRow(new QLabel(tr("Type:")), typeBox);
    formatLayout->addRow(new QLabel(tr("Display:")), displayBox);
    formatGroupBox->setLayout(formatLayout);

    /* Buttons */
    newButton = new QPushButton(tr("New"));
    connect(newButton, &QAbstractButton::clicked, this, &RamSearchWindow::slotNew);

    searchButton = new QPushButton(tr("Search"));
    connect(searchButton, &QAbstractButton::clicked, this, &RamSearchWindow::slotSearch);

    QPushButton *addButton = new QPushButton(tr("Add Watch"));
    connect(addButton, &QAbstractButton::clicked, this, &RamSearchWindow::slotAdd);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(searchButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);

    /* Create the options layout */
    QVBoxLayout *optionLayout = new QVBoxLayout;
    optionLayout->addWidget(memGroupBox);
    optionLayout->addWidget(compareGroupBox);
    optionLayout->addWidget(operatorGroupBox);
    optionLayout->addWidget(formatGroupBox);
    optionLayout->addStretch(1);
    optionLayout->addWidget(buttonBox);

    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addLayout(watchLayout, 1);
    mainLayout->addLayout(optionLayout);

    setLayout(mainLayout);
    
    /* Start the update timer */
    updateTimer = new QElapsedTimer();
    updateTimer->start();

    /* Configure the call timer */
    callTimer = new QTimer(this);
    callTimer->setSingleShot(true);
    connect(callTimer, &QTimer::timeout, this, &RamSearchWindow::update);
}

void RamSearchWindow::update()
{
    /* Only update on new frame and every .5 ms */
    int64_t elapsed = updateTimer->elapsed();
    if (elapsed < 500) {
        /* Call this function on timeout, if not already done */
        if (!callTimer->isActive()) {
            callTimer->start(500 - elapsed);
        }
        return;
    }
    updateTimer->start();

    ramSearchModel->update();
}

void RamSearchWindow::getCompareParameters(CompareType& compare_type, CompareOperator& compare_operator, double& compare_value, double& different_value)
{
    compare_type = CompareType::Previous;
    if (compareValueButton->isChecked()) {
        compare_type = CompareType::Value;
        compare_value = comparingValueBox->value();
    }

    compare_operator = CompareOperator::Equal;
    if (operatorNotEqualButton->isChecked())
        compare_operator = CompareOperator::NotEqual;
    if (operatorLessButton->isChecked())
        compare_operator = CompareOperator::Less;
    if (operatorGreaterButton->isChecked())
        compare_operator = CompareOperator::Greater;
    if (operatorLessEqualButton->isChecked())
        compare_operator = CompareOperator::LessEqual;
    if (operatorGreaterEqualButton->isChecked())
        compare_operator = CompareOperator::GreaterEqual;
    if (operatorDifferenceButton->isChecked()) {
        compare_operator = CompareOperator::Different;
        different_value = differenceValueBox->value();
    }
}

void RamSearchWindow::slotNew()
{
    if (context->status != Context::ACTIVE)
        return;
    
    /* If there are results, then clear the current scan and enable all boxes */
    if (ramSearchModel->scanCount() != 0) {
        newButton->setText(tr("New"));
        memGroupBox->setDisabled(false);
        formatGroupBox->setDisabled(false);
        ramSearchModel->clear();
        watchCount->setText("");
        return;
    }

    /* Disable buttons during the process */
    newButton->setDisabled(true);
    searchButton->setDisabled(true);

    /* Build the memory region flag variable */
    int memflags = 0;
    if (memSpecialBox->isChecked())
        memflags |= MemSection::MemNoSpecial;
    if (memROBox->isChecked())
        memflags |= MemSection::MemNoRO;
    if (memExecBox->isChecked())
        memflags |= MemSection::MemNoExec;

    /* Get the comparison parameters */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    double different_value;
    getCompareParameters(compare_type, compare_operator, compare_value, different_value);

    ramSearchModel->hex = (displayBox->currentIndex() == 1);

    watchCount->hide();
    searchProgress->show();
    searchProgress->setMaximum(ramSearchModel->predictScanCount(memflags));

    /* Call the RamSearch new function using the right type */
    ramSearchModel->newWatches(memflags, typeBox->currentIndex(), compare_type, compare_operator, compare_value, different_value);

    searchProgress->hide();
    watchCount->show();

    /* Don't display values if too many results */
    if ((ramSearchModel->memscanner.display_scan_count() == 0) && (ramSearchModel->scanCount() != 0))
        watchCount->setText(QString("%1 addresses (results are not shown above %2)").arg(ramSearchModel->scanCount()).arg(ramSearchModel->memscanner.DISPLAY_THRESHOLD));
    else
        watchCount->setText(QString("%1 addresses").arg(ramSearchModel->scanCount()));
        
    /* Change the button to "Stop" and disable some boxes */
    if (ramSearchModel->scanCount() != 0) {
        newButton->setText(tr("Stop"));
        memGroupBox->setDisabled(true);
        formatGroupBox->setDisabled(true);
    }
    else {
        newButton->setText(tr("New"));
        memGroupBox->setDisabled(false);
        formatGroupBox->setDisabled(false);
    }
    
    newButton->setDisabled(false);
    searchButton->setDisabled(false);
}

void RamSearchWindow::slotSearch()
{
    /* Disable buttons during the process */
    newButton->setDisabled(true);
    searchButton->setDisabled(true);

    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    double different_value;
    getCompareParameters(compare_type, compare_operator, compare_value, different_value);

    searchProgress->setMaximum(ramSearchModel->scanSize());
    watchCount->hide();
    searchProgress->show();

    ramSearchModel->searchWatches(compare_type, compare_operator, compare_value, different_value);

    /* Update address count */
    searchProgress->hide();
    watchCount->show();
    
    /* Don't display values if too many results */
    if ((ramSearchModel->memscanner.display_scan_count() == 0) && (ramSearchModel->scanCount() != 0))
        watchCount->setText(QString("%1 addresses (results are not shown above %2)").arg(ramSearchModel->scanCount()).arg(ramSearchModel->memscanner.DISPLAY_THRESHOLD));
    else
        watchCount->setText(QString("%1 addresses").arg(ramSearchModel->scanCount()));

    /* Change the button to "New" if no results */
    if (ramSearchModel->scanCount() == 0) {
        newButton->setText(tr("New"));
        memGroupBox->setDisabled(false);
        formatGroupBox->setDisabled(false);
    }
    
    newButton->setDisabled(false);
    searchButton->setDisabled(false);
}

void RamSearchWindow::slotAdd()
{
    const QModelIndex index = ramSearchView->selectionModel()->currentIndex();
    ramSearchView->selectionModel()->clear();

    /* If no watch was selected, return */
    if (!index.isValid()) {
        QMessageBox::critical(nullptr, "Error", QString("You must select an address to add a watch"));
        return;
    }

    int row = index.row();

    /* Fill the watch edit window with parameters from the selected watch */

    MainWindow *mw = qobject_cast<MainWindow*>(parent());
    if (mw) {
        
        mw->ramWatchWindow->editWindow->fill(ramSearchModel->address(row), typeBox->currentIndex());
        mw->ramWatchWindow->slotAdd();
    }
}
