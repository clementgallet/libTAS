/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_TIMETRACEWINDOW_H_INCLUDED
#define LIBTAS_TIMETRACEWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPlainTextEdit>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QItemSelection>
#include <memory>

/* Forward declaration */
struct Context;
class TimeTraceModel;

class TimeTraceWindow : public QDialog {
    Q_OBJECT

public:
    TimeTraceWindow(Context *c, QWidget *parent = Q_NULLPTR);

    QLineEdit *addressInput;

    TimeTraceModel *timeTraceModel;

    QSize sizeHint() const override;

private:
    Context *context;
    QTableView *timeTraceView;
    QSortFilterProxyModel* proxyModel;

    QPlainTextEdit *stackTraceText;

    QPushButton *startButton;

    // QProgressBar *searchProgress;
    // QLabel *scanCount;
    //
    // QSpinBox *maxLevelInput;
    // QSpinBox *maxOffsetInput;

private slots:
    void slotStacktrace(const QItemSelection &selected, const QItemSelection &deselected);
    void slotChooseHash();
    void slotClearHash();
    void slotStart();
    void slotClear();
};

#endif
