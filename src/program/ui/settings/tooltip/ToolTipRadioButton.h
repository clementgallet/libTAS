// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_TOOLTIPRADIOBUTTON_H_INCLUDED
#define LIBTAS_TOOLTIPRADIOBUTTON_H_INCLUDED

#include "ToolTipWidget.h"

#include <QRadioButton>

class ToolTipRadioButton : public ToolTipWidget<QRadioButton>
{
public:
  explicit ToolTipRadioButton(const QString& label);

private:
  QPoint getToolTipPosition() const override;
};

#endif
