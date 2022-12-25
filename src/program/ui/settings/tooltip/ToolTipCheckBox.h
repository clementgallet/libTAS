// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_TOOLTIPCHECKBOX_H_INCLUDED
#define LIBTAS_TOOLTIPCHECKBOX_H_INCLUDED

#include "ToolTipWidget.h"

#include <QCheckBox>

class ToolTipCheckBox : public ToolTipWidget<QCheckBox>
{
public:
  explicit ToolTipCheckBox(const QString& label);

private:
  QPoint getToolTipPosition() const override;
};

#endif
