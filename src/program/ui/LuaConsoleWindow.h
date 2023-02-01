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

#ifndef LIBTAS_LUACONSOLEWINDOW_H_INCLUDED
#define LIBTAS_LUACONSOLEWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QPlainTextEdit>

class LuaConsoleWindow : public QDialog {
    Q_OBJECT

public:
    LuaConsoleWindow(QWidget *parent = Q_NULLPTR);

private:
    QPlainTextEdit *consoleText;
    
private slots:
    void slotAppend(QString qstr);
    void slotClear();
};

#endif
