// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ToolTipComboBox.h"

QPoint ToolTipComboBox::getToolTipPosition() const
{
  return pos() + QPoint(width() / 2, height() / 2);
}
