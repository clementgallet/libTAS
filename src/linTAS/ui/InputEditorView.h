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

#ifndef LINTAS_INPUTEDITORVIEW_H_INCLUDED
#define LINTAS_INPUTEDITORVIEW_H_INCLUDED

#include <QDialog>
#include <QTableView>
#include <QCheckBox>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QMenu>
#include <memory>

#include "InputEditorModel.h"
#include "../Context.h"
#include "KeyPressedDialog.h"

class InputEditorView : public QTableView {
    Q_OBJECT

public:
    InputEditorView(Context *c, QWidget *parent = Q_NULLPTR);

    void update();
    InputEditorModel *inputEditorModel;

public slots:
    // void toggleInput(const QModelIndex &index);
    void horizontalMenu(QPoint pos);
    void renameLabel();
    void addInputColumn();

    void mainMenu(QPoint pos);
    void insertInput();
    void deleteInput();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Context *context;
    QMenu *horMenu;
    QMenu *menu;
    int contextSection;
    int mouseSection;
    bool mouseValue;
    KeyPressedDialog* keyDialog;

};

#endif
