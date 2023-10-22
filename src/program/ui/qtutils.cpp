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

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include "../KeyMapping.h"
#include "qtutils.h"

unsigned int convertQtModifiers(Qt::KeyboardModifiers flags)
{
    unsigned int modifiers = 0;
    if (flags & Qt::ShiftModifier) {
        modifiers |= XK_Shift_Flag;
    }
    if (flags & Qt::ControlModifier) {
        modifiers |= XK_Control_Flag;
    }
    if (flags & Qt::AltModifier) {
        modifiers |= XK_Meta_Flag;
    }
    if (flags & Qt::MetaModifier) {
        modifiers |= XK_Alt_Flag;
    }

    return modifiers;
}

bool isLightTheme()
{
    const auto theme_window_color = QGuiApplication::palette().color(QPalette::Base);
    const auto theme_window_hsv = theme_window_color.toHsv();
    const auto brightness = theme_window_hsv.value();
    return brightness > 128;
}
