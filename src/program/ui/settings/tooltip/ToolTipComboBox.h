// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_TOOLTIPCOMBOBOX_H_INCLUDED
#define LIBTAS_TOOLTIPCOMBOBOX_H_INCLUDED

#include "ToolTipWidget.h"

#include <QComboBox>

class ToolTipComboBox : public ToolTipWidget<QComboBox>
{
private:
    QPoint getToolTipPosition() const override;
};

#endif
