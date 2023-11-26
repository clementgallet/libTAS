// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_TOOLTIPWIDGET_H_INCLUDED
#define LIBTAS_TOOLTIPWIDGET_H_INCLUDED

#include "BalloonTip.h"

#include <optional>
#include <QString>

constexpr int TOOLTIP_DELAY = 300;

template <class Derived>
class ToolTipWidget : public Derived
{
public:
    using Derived::Derived;

    void setTitle(QString title) { m_title = std::move(title); }

    void setDescription(QString description) { m_description = std::move(description); }

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEvent* event) override
#else
    void enterEvent(QEnterEvent* event) override
#endif
    {
        if (m_timer_id)
            return;
        m_timer_id = this->startTimer(TOOLTIP_DELAY);
    }

    void leaveEvent(QEvent* event) override { killAndHide(); }
    void hideEvent(QHideEvent* event) override { killAndHide(); }

    void timerEvent(QTimerEvent* event) override
    {
        this->killTimer(m_timer_id);
        m_timer_id = 0;

        BalloonTip::showBalloon(QIcon(), m_title, m_description,
            this->parentWidget()->mapToGlobal(getToolTipPosition()), this);
    }

    virtual QPoint getToolTipPosition() const = 0;

    void killAndHide()
    {
        if (m_timer_id) {
            this->killTimer(m_timer_id);
            m_timer_id = 0;
        }
        BalloonTip::hideBalloon();
    }   

    int m_timer_id = 0;
    QString m_title;
    QString m_description;
};

#endif
