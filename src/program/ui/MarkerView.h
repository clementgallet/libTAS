/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_MARKERVIEW_H_INCLUDED
#define LIBTAS_MARKERVIEW_H_INCLUDED

#include <QtWidgets/QTableView>
#include <QtWidgets/QMenu>

/* Forward declaration */
struct Context;
class MarkerModel;

class MarkerView : public QTableView {
    Q_OBJECT

public:
    MarkerView(Context *c, QWidget *parent, QWidget *gp);

    /* Fill menu action */
    void fillMenu();

    void resetMarkers();
    MarkerModel *markerModel;

public slots:
    void mainMenu(QPoint pos);
    void seekSlot();
    void scrollSlot();
    void removeSlot();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void seekSignal(unsigned long long frame);
    void scrollSignal(unsigned long long frame);

private slots:

private:
    unsigned long long getMarkerFrameFromSelection();

    Context *context;
    QMenu *menu;

    QAction *seekAct;
    QAction *scrollAct;
    QAction *editAct;
    QAction *editFrameAct;
    QAction *removeAct;
};

#endif
