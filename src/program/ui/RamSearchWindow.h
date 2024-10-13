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

#ifndef LIBTAS_RAMSEARCHWINDOW_H_INCLUDED
#define LIBTAS_RAMSEARCHWINDOW_H_INCLUDED

#include "ramsearch/CompareOperations.h"
#include "ramsearch/MemValue.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableView>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>
#include <memory>
#include <atomic>

/* Forward declaration */
struct Context;
class RamSearchModel;
class RamWatchWindow;
class HexViewWindow;

class RamSearchWindow : public QDialog {
    Q_OBJECT

public:
    RamSearchWindow(Context *c, HexViewWindow* view, RamWatchWindow* ram, QWidget *parent = Q_NULLPTR);

    void update();

private:
    Context *context;
    QTableView *ramSearchView;
    HexViewWindow* hexViewWindow;
    RamWatchWindow* ramWatchWindow;

    RamSearchModel *ramSearchModel;
    QProgressBar *searchProgress;
    QLabel *watchCount;

    QGroupBox *memGroupBox;
    QCheckBox *memSpecialBox;
    QCheckBox *memROBox;
    QCheckBox *memExecBox;
    QLineEdit *memBeginLine;
    QLineEdit *memEndLine;

    QRadioButton *comparePreviousButton;
    QRadioButton *compareValueButton;
    QLineEdit *comparingValueBox;

    QRadioButton *operatorEqualButton;
    QRadioButton *operatorNotEqualButton;
    QRadioButton *operatorLessButton;
    QRadioButton *operatorGreaterButton;
    QRadioButton *operatorLessEqualButton;
    QRadioButton *operatorGreaterEqualButton;
    QRadioButton *operatorDifferenceButton;
    QLineEdit *differenceValueBox;

    QGroupBox *formatGroupBox;
    QComboBox *typeBox;
    QComboBox *displayBox;
    QComboBox *alignmentBox;

    QPushButton *newButton;
    QPushButton *searchButton;
    QPushButton *stopButton;
    
    /* Timer to limit the number of update calls */
    QElapsedTimer* updateTimer;

    /* Timer to trigger the update call */
    QTimer* callTimer;

    std::atomic<bool> isSearching;

    void getCompareParameters(CompareType& compare_type, CompareOperator& compare_operator, MemValueType& compare_value, MemValueType& different_value);

    /* Actual RAM search done in another thread */
    void threadedNew(int memflags);
    void threadedSearch();

private slots:
    void slotNew();
    void slotSearch();
    void slotAdd();
    void slotHex();
    void slotStop();
    void slotTypeChanged(int index);

};

#endif
