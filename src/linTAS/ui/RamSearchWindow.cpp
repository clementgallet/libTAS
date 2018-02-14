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
#include <QLabel>
#include <QVBoxLayout>
#include <QVBoxLayout>

#include "RamSearchWindow.h"
#include <iostream>
#include <sstream>
#include <algorithm> // std::remove_if
#include "../ramsearch/CompareEnums.h"

// static Fl_Callback new_cb;
// static Fl_Callback search_cb;
// static Fl_Callback add_cb;

RamSearchWindow::RamSearchWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setFixedSize(800, 700);
    setWindowTitle("Ram Search");

    /* Table */
    ramSearchView = new QTableView();
    ramSearchView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ramSearchView->setSelectionMode(QAbstractItemView::SingleSelection);

    ramSearchModel = new RamSearchModel(&ram_search.ramwatches);
    ramSearchView->setModel(ramSearchModel);


    /* Progress bar */
    // search_progress = new Fl_Hor_Fill_Slider(10, 650, 480, 10);
    // search_progress->hide();
    // search_progress->selection_color(FL_BLUE);
    // search_progress->box(FL_THIN_DOWN_FRAME);
    // search_progress->slider(FL_FLAT_BOX);
    //
    // watch_count = new Fl_Box(10, 670, 480, 30);
    // watch_count->box(FL_NO_BOX);
    // watch_count->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);


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
    displayBox->addItem("Decimal");
    displayBox->addItem("Hexadecimal");

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
    optionLayout->addWidget(buttonBox);

    /* Create the main layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(ramSearchView);
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

    /* Call the RamSearch new function using the right type as template */
    switch (typeBox->currentIndex()) {
        case 0:
            ram_search.new_watches<unsigned char>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 1:
            ram_search.new_watches<char>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 2:
            ram_search.new_watches<unsigned short>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 3:
            ram_search.new_watches<short>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 4:
            ram_search.new_watches<unsigned int>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 5:
            ram_search.new_watches<int>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 6:
            ram_search.new_watches<int64_t>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 7:
            ram_search.new_watches<uint64_t>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 8:
            ram_search.new_watches<float>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
        case 9:
            ram_search.new_watches<double>(context->game_pid, memregions, compare_type, compare_operator, compare_value);
            break;
    }

    ramSearchModel->hex = (displayBox->currentIndex() == 1);
    ramSearchModel->update();

    /* Update address count */
    // std::ostringstream oss;
    // oss << ram_search.ramwatches.size();
    // oss << " addresses";
    // watch_count->copy_label(oss.str().c_str());
}

void RamSearchWindow::slotSearch()
{
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    getCompareParameters(compare_type, compare_operator, compare_value);

    // rsw->search_progress->show();
    // rsw->search_progress->bounds(0, rsw->ram_search.ramwatches.size());

    /* Update the previous_value attribute of each RamWatch object in the vector,
     * and remove objects from the vector where the search condition returns false.
     */
    int num = 0;
    ram_search.ramwatches.erase(
        std::remove_if(ram_search.ramwatches.begin(), ram_search.ramwatches.end(),
            [&compare_type, &compare_operator, &compare_value, &num] (std::unique_ptr<IRamWatch> &watch) {
                // if (!(num++ & 0xfff)) {
                //     rsw->search_progress->value(num);
                //     Fl::flush();
                // }
                return watch->check_update(compare_type, compare_operator, compare_value);
            }),
        ram_search.ramwatches.end());

    // rsw->search_progress->hide();

    /* Update table parameters */
    ramSearchModel->hex = (displayBox->currentIndex() == 1);
    ramSearchModel->compare_type = compare_type;
    ramSearchModel->compare_operator = compare_operator;
    ramSearchModel->compare_value_db = compare_value;
    ramSearchModel->update();
    // ramSearchModel->rows(rsw->ram_search.ramwatches.size());

    /* Update address count */
    // std::ostringstream oss;
    // oss << rsw->ram_search.ramwatches.size();
    // oss << " adresses";
    // rsw->watch_count->copy_label(oss.str().c_str());
}

void RamSearchWindow::slotAdd()
{
    const QModelIndex index = ramSearchView->selectionModel()->currentIndex();
    ramSearchView->selectionModel()->clear();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    int row = index.row();

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
