/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>

#include "RamSearchWindow.h"
#include "../ramsearch/CompareEnums.h"

RamSearchWindow::RamSearchWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Ram Search");

    /* Table */
    ramSearchView = new QTableView();
    ramSearchView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ramSearchView->setSelectionMode(QAbstractItemView::SingleSelection);
    ramSearchView->setShowGrid(false);
    ramSearchView->setAlternatingRowColors(true);
    ramSearchView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ramSearchView->verticalHeader()->hide();

    ramSearchModel = new RamSearchModel(context);
    ramSearchView->setModel(ramSearchModel);

    /* Progress bar */
    searchProgress = new QProgressBar();
    connect(ramSearchModel, &RamSearchModel::signalProgress, searchProgress, &QProgressBar::setValue);

    watchCount = new QLabel();
    // watchCount->setHeight(searchProgress->height());
    searchProgress->hide();

    QVBoxLayout *watchLayout = new QVBoxLayout;
    watchLayout->addWidget(ramSearchView);
    watchLayout->addWidget(searchProgress);
    watchLayout->addWidget(watchCount);


    /* Memory regions */
    memTextBox = new QCheckBox("Text");
    memDataROBox = new QCheckBox("RO Data");
    memDataRWBox = new QCheckBox("RW Data");
    memBSSBox = new QCheckBox("BSS");
    memHeapBox = new QCheckBox("Heap");
    memFileMappingBox = new QCheckBox("File Mapping");
    memAnonymousMappingROBox = new QCheckBox("Anon RO Mapping");
    memAnonymousMappingRWBox = new QCheckBox("Anon RW Mapping");
    memStackBox = new QCheckBox("Stack");
    memSpecialBox = new QCheckBox("Special");

    QGroupBox *memGroupBox = new QGroupBox(tr("Included Memory Regions"));
    QGridLayout *memLayout = new QGridLayout;
    memLayout->addWidget(memTextBox, 0, 0);
    memLayout->addWidget(memDataROBox, 1, 0);
    memLayout->addWidget(memDataRWBox, 2, 0);
    memLayout->addWidget(memBSSBox, 3, 0);
    memLayout->addWidget(memHeapBox, 4, 0);
    memLayout->addWidget(memFileMappingBox, 0, 1);
    memLayout->addWidget(memAnonymousMappingROBox, 1, 1);
    memLayout->addWidget(memAnonymousMappingRWBox, 2, 1);
    memLayout->addWidget(memStackBox, 3, 1);
    memLayout->addWidget(memSpecialBox, 4, 1);

    memGroupBox->setLayout(memLayout);

    /* Comparisons */
    comparePreviousButton = new QRadioButton("Unknown/Previous Value");
    comparePreviousButton->setChecked(true);
    compareValueButton = new QRadioButton("Specific Value:");
    comparingValueBox = new QDoubleSpinBox();

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

    QGroupBox *operatorGroupBox = new QGroupBox(tr("Comparison Operator"));
    QVBoxLayout *operatorLayout = new QVBoxLayout;
    operatorLayout->addWidget(operatorEqualButton);
    operatorLayout->addWidget(operatorNotEqualButton);
    operatorLayout->addWidget(operatorLessButton);
    operatorLayout->addWidget(operatorGreaterButton);
    operatorLayout->addWidget(operatorLessEqualButton);
    operatorLayout->addWidget(operatorGreaterEqualButton);
    operatorGroupBox->setLayout(operatorLayout);

    /* Format */
    typeBox = new QComboBox();
    QStringList typeList;
    typeList << "unsigned char" << "char" << "unsigned short" << "short";
    typeList << "unsigned int" << "int" << "unsigned int64" << "int64";
    typeList << "float" << "double";
    typeBox->addItems(typeList);

    displayBox = new QComboBox();
    displayBox->addItem("decimal");
    displayBox->addItem("hexadecimal");

    QGroupBox *formatGroupBox = new QGroupBox(tr("Format"));
    QFormLayout *formatLayout = new QFormLayout;
    formatLayout->addRow(new QLabel(tr("Type:")), typeBox);
    formatLayout->addRow(new QLabel(tr("Display:")), displayBox);
    formatGroupBox->setLayout(formatLayout);

    /* Buttons */
    QPushButton *newButton = new QPushButton(tr("New"));
    connect(newButton, &QAbstractButton::clicked, this, &RamSearchWindow::slotNew);

    QPushButton *searchButton = new QPushButton(tr("Search"));
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
}

void RamSearchWindow::update()
{
    ramSearchModel->update();
}

void RamSearchWindow::getCompareParameters(CompareType& compare_type, CompareOperator& compare_operator, double& compare_value)
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
}

void RamSearchWindow::slotNew()
{
    /* Build the memory region flag variable */
    int memregions = 0;
    if (memTextBox->isChecked())
        memregions |= MemSection::MemText;
    if (memDataROBox->isChecked())
        memregions |= MemSection::MemDataRO;
    if (memDataRWBox->isChecked())
        memregions |= MemSection::MemDataRW;
    if (memBSSBox->isChecked())
        memregions |= MemSection::MemBSS;
    if (memHeapBox->isChecked())
        memregions |= MemSection::MemHeap;
    if (memFileMappingBox->isChecked())
        memregions |= MemSection::MemFileMapping;
    if (memAnonymousMappingROBox->isChecked())
        memregions |= MemSection::MemAnonymousMappingRO;
    if (memAnonymousMappingRWBox->isChecked())
        memregions |= MemSection::MemAnonymousMappingRW;
    if (memStackBox->isChecked())
        memregions |= MemSection::MemStack;
    if (memSpecialBox->isChecked())
        memregions |= MemSection::MemSpecial;

    /* Get the comparison parameters */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    getCompareParameters(compare_type, compare_operator, compare_value);

    ramSearchModel->hex = (displayBox->currentIndex() == 1);

    watchCount->hide();
    searchProgress->show();
    searchProgress->setMaximum(ramSearchModel->predictWatchCount(memregions));

    /* Call the RamSearch new function using the right type as template */
    switch (typeBox->currentIndex()) {
        case 0:
            ramSearchModel->newWatches<unsigned char>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 1:
            ramSearchModel->newWatches<char>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 2:
            ramSearchModel->newWatches<unsigned short>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 3:
            ramSearchModel->newWatches<short>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 4:
            ramSearchModel->newWatches<unsigned int>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 5:
            ramSearchModel->newWatches<int>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 6:
            ramSearchModel->newWatches<int64_t>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 7:
            ramSearchModel->newWatches<uint64_t>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 8:
            ramSearchModel->newWatches<float>(memregions, compare_type, compare_operator, compare_value);
            break;
        case 9:
            ramSearchModel->newWatches<double>(memregions, compare_type, compare_operator, compare_value);
            break;
    }

    searchProgress->hide();
    watchCount->show();

    /* Update address count */
    watchCount->setText(QString("%1 addresses").arg(ramSearchModel->watchCount()));
}

void RamSearchWindow::slotSearch()
{
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    getCompareParameters(compare_type, compare_operator, compare_value);

    searchProgress->setMaximum(ramSearchModel->watchCount());
    watchCount->hide();
    searchProgress->show();

    ramSearchModel->searchWatches(compare_type, compare_operator, compare_value);

    /* Update address count */
    searchProgress->hide();
    watchCount->show();
    watchCount->setText(QString("%1 addresses").arg(ramSearchModel->watchCount()));

}

void RamSearchWindow::slotAdd()
{
    const QModelIndex index = ramSearchView->selectionModel()->currentIndex();
    ramSearchView->selectionModel()->clear();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    // int row = index.row();

    /* TODO! */
    /* Fill the watch edit window with parameters from the selected watch */
    // MainWindow& mw = MainWindow::getInstance();
    // mw.ramwatch_window->edit_window->fill(rsw->ram_search.ramwatches.at(r));
    //
    // mw.ramwatch_window->edit_window->window->show();
    //
    // while (mw.ramwatch_window->edit_window->window->shown()) {
    //     Fl::wait();
    // }
    //
    // if (mw.ramwatch_window->edit_window->ramwatch) {
    //     mw.ramwatch_window->edit_window->ramwatch->game_pid = rsw->context->game_pid;
    //     mw.ramwatch_window->watch_table->ramwatches.push_back(std::move(mw.ramwatch_window->edit_window->ramwatch));
    //     mw.ramwatch_window->watch_table->update();
    // }
}
