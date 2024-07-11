/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_INPUTEDITORVIEW_H_INCLUDED
#define LIBTAS_INPUTEDITORVIEW_H_INCLUDED

#include <QtWidgets/QTableView>
#include <QtWidgets/QMenu>

/* Forward declaration */
struct Context;
class InputEditorModel;
class KeyPressedDialog;
class InputEventWindow;

class InputEditorView : public QTableView {
    Q_OBJECT

public:
    InputEditorView(Context *c, QWidget *parent, QWidget *gp);

    /* Fill menu action */
    void fillMenu(QMenu* menu);

    void update();
    void resetInputs();
    InputEditorModel *inputEditorModel;
    int scrollBarWidth;

public slots:
    void horizontalMenu(QPoint pos);
    void renameLabel();
    void addInputColumn();
    void clearInputColumn();
    void removeInputColumn();
    void lockInputColumn(bool checked);
    void autoholdInput(bool checked);
    void autofireInput(bool checked);

    void mainMenu(QPoint pos);
    void editEvents();
    void duplicateInput();
    void insertInput();
    void insertInputs();
    void deleteInput();
    void truncateInputs();
    void clearInput();
    void addMarker();
    void removeMarker();
    void getCurrentMarkerText(std::string &marker);

    void copyInputs();
    void cutInputs();
    void pasteInputs();
    void pasteInsertInputs();

    void manualScroll(int value);

    /* Disable or enable menu items when selection has changed */
    void updateMenu();

    void moveAgainSection(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void scrollToFrame(unsigned long long frame);
    void holdSection(int logicalIndex);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void resizeAllColumns();
    void showMarkerToolTip(const QModelIndex &index);

signals:
    void addMarkerSignal(int frame, QString text);
    void removeMarkerSignal(int frame);

private:
    Context *context;
    QMenu *horMenu;
    QMenu *menu;
    int contextSection;
    int mouseColumn;
    int mouseRow;
    int mouseMinRow;
    int mouseMaxRow;
    int minToggleRow;
    int mouseValue;
    KeyPressedDialog* keyDialog;
    InputEventWindow* inputEventWindow;
    std::string currentMarkerText;

    QAction *eventAct;
    QAction *duplicateAct;
    QAction *insertAct;
    QAction *insertsAct;
    QAction *deleteAct;
    QAction *truncateAct;
    QAction *clearAct;
    QAction *copyAct;
    QAction *cutAct;
    QAction *pasteAct;
    QAction *pasteInsertAct;
    QAction *markAct;
    QAction *unmarkAct;

    QAction *lockAction;
    QAction *autoholdAction;
    QAction *autofireAction;

    bool autoScroll = true;
    int markerTimerId = 0;
    int markerRow = 0;
    
    void addMarkerFrame(int frame);
    void hideMarkerToolTip();

    /* Apply a function to each range of selected rows,
     * and return min selected row */
    int applyToSelectedRanges(std::function<void(int, int)>);

    /* Apply a function to each range of selected rows (in decreasing order),
     * and return min selected row */
    int applyToSelectedRangesReversed(std::function<void(int, int)>);

};

#endif
