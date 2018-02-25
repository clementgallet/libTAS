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

#ifndef LINTAS_RAMSEARCHWINDOW_H_INCLUDED
#define LINTAS_RAMSEARCHWINDOW_H_INCLUDED

#include <QDialog>
#include <QTableView>
#include <QCheckBox>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QLabel>
#include <memory>

#include "RamSearchModel.h"
#include "../Context.h"

class RamSearchWindow : public QDialog {
    Q_OBJECT

public:
    RamSearchWindow(Context *c, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = 0);

    void update();

private:
    Context *context;
    QTableView *ramSearchView;

    RamSearchModel *ramSearchModel;
    QProgressBar *searchProgress;
    QLabel *watchCount;

    QCheckBox *memTextBox;
    QCheckBox *memDataROBox;
    QCheckBox *memDataRWBox;
    QCheckBox *memBSSBox;
    QCheckBox *memHeapBox;
    QCheckBox *memFileMappingBox;
    QCheckBox *memAnonymousMappingROBox;
    QCheckBox *memAnonymousMappingRWBox;
    QCheckBox *memStackBox;
    QCheckBox *memSpecialBox;

    QRadioButton *comparePreviousButton;
    QRadioButton *compareValueButton;
    QDoubleSpinBox *comparingValueBox;

    QRadioButton *operatorEqualButton;
    QRadioButton *operatorNotEqualButton;
    QRadioButton *operatorLessButton;
    QRadioButton *operatorGreaterButton;
    QRadioButton *operatorLessEqualButton;
    QRadioButton *operatorGreaterEqualButton;

    QComboBox *typeBox;
    QComboBox *displayBox;

    void getCompareParameters(CompareType& compare_type, CompareOperator& compare_operator, double& compare_value);

private slots:
    void slotNew();
    void slotSearch();
    void slotAdd();

};

#endif
