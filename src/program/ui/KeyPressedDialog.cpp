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

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QKeyEvent>

#include "KeyPressedDialog.h"
#include "qtutils.h"

KeyPressedDialog::KeyPressedDialog(Context* c, QWidget *parent) : QDialog(parent), context(c) {

    setWindowTitle("");
    setFocusPolicy(Qt::ClickFocus);

    withModifiers = false;

    QLabel *label = new QLabel(tr("Press a key"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(label);
    setLayout(mainLayout);
}

void KeyPressedDialog::keyPressEvent(QKeyEvent * e)
{
    int modifiers = 0;
    if (withModifiers) {

        /* We ignore a key press on a modifier key if we accept a mapping that
         * includes modifier keys */
        if (context->config.km->is_modifier(e->nativeVirtualKey())) {
            return QDialog::keyPressEvent(e);
        }

        /* Build modifiers */
        modifiers = convertQtModifiers(e->modifiers());
    }

    done(modifiers | context->config.km->nativeToKeysym(e->nativeVirtualKey()));
}
