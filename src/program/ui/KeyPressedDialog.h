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

#ifndef LIBTAS_KEYPRESSEDDIALOG_H_INCLUDED
#define LIBTAS_KEYPRESSEDDIALOG_H_INCLUDED

#include <QDialog>

/* Very small QDialog class to catch a pressed key (or key + modifiers) */
class KeyPressedDialog : public QDialog {
    Q_OBJECT
public:
    KeyPressedDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = 0);

    bool withModifiers;

protected:
    void keyPressEvent(QKeyEvent * e);

};

#endif
