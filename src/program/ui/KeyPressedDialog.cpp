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

#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>

#include "KeyPressedDialog.h"
#include "../KeyMapping.h"

KeyPressedDialog::KeyPressedDialog(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags) {

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
         * includes modifier keys
         */

        if (is_modifier(e->nativeVirtualKey())) {
            return QDialog::keyPressEvent(e);
        }

        /* Build modifiers */
        if (e->modifiers() & Qt::ShiftModifier) {
            modifiers |= XK_Shift_L_Flag;
        }
        if (e->modifiers() & Qt::ControlModifier) {
            modifiers |= XK_Control_L_Flag;
        }
        if (e->modifiers() & Qt::AltModifier) {
            modifiers |= XK_Alt_L_Flag;
        }
        if (e->modifiers() & Qt::MetaModifier) {
            modifiers |= XK_Meta_L_Flag;
        }
    }

    done(modifiers | e->nativeVirtualKey());
}
