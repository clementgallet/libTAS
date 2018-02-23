/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LINTAS_CONTROLLERAXISWIDGET_H_INCLUDED
#define LINTAS_CONTROLLERAXISWIDGET_H_INCLUDED

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

class ControllerAxisWidget : public QWidget {
    Q_OBJECT

public:
    ControllerAxisWidget(QWidget *parent = Q_NULLPTR);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    short x_axis, y_axis;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

public slots:
    void slotSetAxes(short x, short y);
    void slotSetXAxis(int x);
    void slotSetYAxis(int y);

signals:
    void XAxisChanged(int x);
    void YAxisChanged(int y);

private:
    short clampToShort(int val);

};

#endif
