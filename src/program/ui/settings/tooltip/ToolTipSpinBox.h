// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_TOOLTIPSPINBOX_H_INCLUDED
#define LIBTAS_TOOLTIPSPINBOX_H_INCLUDED

#include "ToolTipWidget.h"

#include <QSpinBox>

class ToolTipSpinBox : public ToolTipWidget<QSpinBox>
{
private:
  QPoint getToolTipPosition() const override;
};

#endif
