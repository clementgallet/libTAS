// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_BALLOONTIP_H_INCLUDED
#define LIBTAS_BALLOONTIP_H_INCLUDED

#include <QColor>
#include <QPixmap>
#include <QWidget>

class BalloonTip : public QWidget
{
  Q_OBJECT

  struct PrivateTag
  {
  };

public:
  enum class ShowArrow
  {
    Yes,
    No
  };
  static void showBalloon(const QString& title, const QString& msg,
                          const QPoint& target_arrow_tip_position, QWidget* parent,
                          ShowArrow show_arrow = ShowArrow::Yes, int border_width = 1);
  static void hideBalloon();

  BalloonTip(PrivateTag, const QString& title, QString msg, QWidget* const parent);

private:
  void updateBoundsAndRedraw(const QPoint& target_arrow_tip_position, ShowArrow show_arrow, int border_width);

protected:
  void paintEvent(QPaintEvent*) override;

private:
  QColor m_border_color;
  QPixmap m_pixmap;
};

#endif
