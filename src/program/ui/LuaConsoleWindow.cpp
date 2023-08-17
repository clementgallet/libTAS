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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QHeaderView>
// #include <QtWidgets/QItemSelectionModel>

#include "LuaConsoleWindow.h"
#include "LuaConsoleModel.h"
#include "../lua/Print.h"
#include "../Context.h"

LuaConsoleWindow::LuaConsoleWindow(Context *c, QWidget *parent) : QMainWindow(parent), context(c)
{
    setWindowTitle("Lua Console");

    /* Main menu */
    QMenu* scriptMenu = menuBar()->addMenu(tr("Script"));
    
    scriptMenu->addAction(tr("Add script file"), this, &LuaConsoleWindow::addScript);
    scriptMenu->addAction(tr("Remove script file"), this, &LuaConsoleWindow::removeScript);
    scriptMenu->addAction(tr("Clear all script files"), this, &LuaConsoleWindow::clearScripts);
    scriptMenu->addAction(tr("Clear output window"), this, &LuaConsoleWindow::slotClear);

    /* Table */
    luaView = new QTableView(this);
    luaView->setSelectionBehavior(QAbstractItemView::SelectRows);
    luaView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    luaView->setShowGrid(false);
    luaView->setAlternatingRowColors(true);
    
    luaView->verticalHeader()->hide();

    luaModel = new LuaConsoleModel();
    luaView->setModel(luaModel);

    luaView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    luaView->horizontalHeader()->resizeSection(0, 18);
    luaView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    luaView->horizontalHeader()->resizeSection(1, 80);
    luaView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    /* Text Edit */
    consoleText = new QPlainTextEdit();
    consoleText->setReadOnly(true);
    consoleText->setMaximumBlockCount(10000);
    connect(&Lua::Print::get(), &Lua::Print::signalPrint, this, &LuaConsoleWindow::slotAppend);

    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(luaView);
    mainLayout->addWidget(consoleText);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    
    /* Configure the call timer */
    startTimer(1000);
}

void LuaConsoleWindow::timerEvent(QTimerEvent *event)
{
    luaModel->update();
}

void LuaConsoleWindow::slotAppend(QString qstr)
{
    consoleText->appendPlainText(qstr);
}

void LuaConsoleWindow::slotClear()
{
    consoleText->clear();
}

void LuaConsoleWindow::addScript()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Lua script"), context->gamepath.c_str());
    if (filename.isNull())
        return;

    luaModel->addFile(filename);
}

void LuaConsoleWindow::removeScript()
{
    /* Remove scripts from bottom to top */
    QModelIndexList indexlist = luaView->selectionModel()->selectedRows();
    std::sort(indexlist.begin(), indexlist.end());
    
    for (auto it = indexlist.crbegin(); it != indexlist.crend(); it++) {
        luaModel->removeFile(it->row());
    }
}

void LuaConsoleWindow::clearScripts()
{
    luaModel->clear();
}
