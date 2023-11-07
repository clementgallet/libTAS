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

#ifndef LIBTAS_LUACONSOLEWINDOW_H_INCLUDED
#define LIBTAS_LUACONSOLEWINDOW_H_INCLUDED

#include "LuaConsoleModel.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTableView>

/* Forward declaration */
struct Context;

class LuaConsoleWindow : public QMainWindow {
    Q_OBJECT

public:
    LuaConsoleWindow(Context *c, QWidget *parent = Q_NULLPTR);

    void timerEvent(QTimerEvent *event);

private:
    Context *context;
    QPlainTextEdit *consoleText;
    
    QTableView* luaView;
    LuaConsoleModel* luaModel;
    
private slots:
    void slotAppend(QString qstr);
    void slotClear();
    void addScript();
    void removeScript();
    void clearScripts();
};

#endif
