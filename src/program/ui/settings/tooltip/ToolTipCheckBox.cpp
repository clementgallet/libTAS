// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ToolTipCheckBox.h"

#include <QStyle>
#include <QStyleOption>

ToolTipCheckBox::ToolTipCheckBox(const QString& label) : ToolTipWidget(label)
{
  setTitle(label);
}

QPoint ToolTipCheckBox::getToolTipPosition() const
{
  int checkbox_width = 18;
  if (style())
  {
    QStyleOptionButton opt;
    initStyleOption(&opt);
    checkbox_width =
        style()->subElementRect(QStyle::SubElement::SE_CheckBoxIndicator, &opt, this).width();
  }

  return pos() + QPoint(checkbox_width / 2, height() / 2);
}
