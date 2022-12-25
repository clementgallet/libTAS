// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ToolTipSpinBox.h"

QPoint ToolTipSpinBox::getToolTipPosition() const
{
  return pos() + QPoint(width() / 2, height() / 2);
}
