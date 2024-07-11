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

#ifndef LIBTAS_INPUTEVENTWINDOW_H_INCLUDED
#define LIBTAS_INPUTEVENTWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPlainTextEdit>
#include <memory>

/* Forward declaration */
struct Context;
class AllInputs;
class InputEventModel;
class MovieFile;

class InputEventWindow : public QDialog {
    Q_OBJECT

public:
    InputEventWindow(Context *c, MovieFile *movie, QWidget *parent = Q_NULLPTR);

    void setFrame(int frame);
    
private:
    Context *context;
    QTableView *inputEventView;
    InputEventModel *inputEventModel;
    MovieFile *movie;
    int frame;

    QPushButton *newButton;

private slots:
    // void slotRecord();
    void slotNew();
    void slotRemove();
    void slotClear();
    void slotSave();
    void updateSelection();
};

#endif
