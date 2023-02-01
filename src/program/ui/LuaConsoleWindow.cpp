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

#include "LuaConsoleWindow.h"
#include "../lua/Print.h"

LuaConsoleWindow::LuaConsoleWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Lua Console");

    /* Text Edit */
    consoleText = new QPlainTextEdit();
    consoleText->setReadOnly(true);
    consoleText->setMaximumBlockCount(10000);
    connect(&Lua::Print::get(), &Lua::Print::signalPrint, this, &LuaConsoleWindow::slotAppend);

    /* Buttons */
    QPushButton *clearButton = new QPushButton(tr("Clear"));
    connect(clearButton, &QAbstractButton::clicked, this, &LuaConsoleWindow::slotClear);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);

    /* Layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(consoleText);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void LuaConsoleWindow::slotAppend(QString qstr)
{
    consoleText->appendPlainText(qstr);
}

void LuaConsoleWindow::slotClear()
{
    consoleText->clear();
}
