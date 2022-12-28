// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_TOOLTIPSLIDER_H_INCLUDED
#define LIBTAS_TOOLTIPSLIDER_H_INCLUDED

#include "ToolTipWidget.h"

#include <QSlider>

class ToolTipSlider : public ToolTipWidget<QSlider>
{
public:
    explicit ToolTipSlider(Qt::Orientation orientation);

private:
    QPoint getToolTipPosition() const override;
};

#endif
