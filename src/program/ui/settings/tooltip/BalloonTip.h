// Copyright 2020 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBTAS_BALLOONTIP_H_INCLUDED
#define LIBTAS_BALLOONTIP_H_INCLUDED

#include <QIcon>
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
  static void showBalloon(const QIcon& icon, const QString& title, const QString& msg,
                          const QPoint& pos, QWidget* parent,
                          ShowArrow show_arrow = ShowArrow::Yes);
  static void hideBalloon();

  BalloonTip(PrivateTag, const QIcon& icon, QString title, QString msg, QWidget* parent);

private:
  void updateBoundsAndRedraw(const QPoint&, ShowArrow);

protected:
  void paintEvent(QPaintEvent*) override;

private:
  QColor m_border_color;
  QPixmap m_pixmap;
  bool m_show_arrow = true;
};

#endif
