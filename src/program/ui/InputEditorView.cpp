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

#include "KeyPressedDialog.h"
#include "InputEditorView.h"
#include "InputEditorModel.h"
#include "MainWindow.h"
#include "qtutils.h"
#include "settings/tooltip/BalloonTip.h"

#include "Context.h"

#include <QtWidgets/QInputDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>

#include <stdint.h>

InputEditorView::InputEditorView(Context* c, QWidget *parent, QWidget *gp) : QTableView(parent), context(c)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(true);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    MovieFile *movie = nullptr;
    MainWindow *mw = qobject_cast<MainWindow*>(gp);
    if (mw) {
        movie = &mw->gameLoop->movie;
    }

    inputEditorModel = new InputEditorModel(context, movie);
    setModel(inputEditorModel);

    connect(inputEditorModel, &InputEditorModel::inputSetChanged, this, &InputEditorView::resizeAllColumns);
    connect(this, &InputEditorView::entered, this, &InputEditorView::showMarkerToolTip);
    connect(this, &InputEditorView::entered, inputEditorModel, &InputEditorModel::setHoveredCell);
    setMouseTracking(true);

    /* Horizontal header */
    horizontalHeader()->setMinimumSectionSize(20);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setResizeContentsPrecision(1);

    /* Savestate column is fixed */
    horizontalHeader()->resizeSection(0, 20);

    /* Frame column is fixed */
    horizontalHeader()->resizeSection(1, 80);

    horizontalHeader()->setSectionsMovable(true);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setDropIndicatorShown(true);
    horizontalHeader()->setDragEnabled(true);
    horizontalHeader()->setDragDropMode(QTableView::InternalMove);

    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &InputEditorView::horizontalMenu);
    connect(horizontalHeader(), &QHeaderView::sectionMoved, this, &InputEditorView::moveAgainSection);

    /* Horizontal menu */
    horMenu = new QMenu(this);
    horMenu->addAction(tr("Rename label"), this, &InputEditorView::renameLabel);
    horMenu->addAction(tr("Add input column"), this, &InputEditorView::addInputColumn);
    horMenu->addAction(tr("Clear input column"), this, &InputEditorView::clearInputColumn);
    horMenu->addAction(tr("Remove input column"), this, &InputEditorView::removeInputColumn);
    lockAction = horMenu->addAction(tr("Lock input column"), this, &InputEditorView::lockInputColumn);
    lockAction->setCheckable(true);

    /* Vertical header */
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setMinimumSectionSize(fontMetrics().height());
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());

    /* Track vertical scrolling */
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &InputEditorView::manualScroll);

    /* Track selection changed to enable/disable menu items. */
    insertAct = nullptr;
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &InputEditorView::updateMenu);

    keyDialog = new KeyPressedDialog(c, this);
    keyDialog->withModifiers = false;

    currentMarkerText = "";

    scrollBarWidth = verticalScrollBar()->sizeHint().width() + 20;
}

void InputEditorView::fillMenu(QMenu* frameMenu)
{
    menu = frameMenu;
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &InputEditorView::mainMenu);

    insertAct = menu->addAction(tr("Insert"), this, &InputEditorView::insertInput, QKeySequence(Qt::CTRL + Qt::Key_Plus));

    /* Shortcuts for context menus are special, they won't work by default
     * because the menu is hidden, so actions need to be added to the View as
     * well. Also, starting Qt 5.10, shortcuts are hidden in context menus
     * for almost all platform styles (except KDE apparently?), and
     * the option to globally enable them (AA_DontShowShortcutsInContextMenus)
     * is buggy, so we must enable for every single action.
     */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    insertAct->setShortcutVisibleInContextMenu(true);
#endif

    this->addAction(insertAct);

    insertsAct = menu->addAction(tr("Insert # frames"), this, &InputEditorView::insertInputs);

    markAct = menu->addAction(tr("Add marker"), this, &InputEditorView::addMarker);

    unmarkAct = menu->addAction(tr("Remove marker"), this, &InputEditorView::removeMarker);

    duplicateAct = menu->addAction(tr("Duplicate"), this, &InputEditorView::duplicateInput, QKeySequence(Qt::CTRL + Qt::Key_D));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    duplicateAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(duplicateAct);

    deleteAct = menu->addAction(tr("Delete"), this, &InputEditorView::deleteInput, QKeySequence(Qt::CTRL + Qt::Key_Minus));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    deleteAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(deleteAct);

    truncateAct = menu->addAction(tr("Truncate"), this, &InputEditorView::truncateInputs);
    clearAct = menu->addAction(tr("Clear"), this, &InputEditorView::clearInput, QKeySequence::Delete);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    clearAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(clearAct);

    menu->addSeparator();
    copyAct = menu->addAction(tr("Copy"), this, &InputEditorView::copyInputs, QKeySequence::Copy);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    copyAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(copyAct);

    cutAct = menu->addAction(tr("Cut"), this, &InputEditorView::cutInputs, QKeySequence::Cut);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    cutAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(cutAct);

    pasteAct = menu->addAction(tr("Paste"), this, &InputEditorView::pasteInputs, QKeySequence::Paste);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    pasteAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(pasteAct);

    pasteInsertAct = menu->addAction(tr("Paste Insert"), this, &InputEditorView::pasteInsertInputs, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    pasteInsertAct->setShortcutVisibleInContextMenu(true);
#endif
    this->addAction(pasteInsertAct);    
}

void InputEditorView::resizeAllColumns()
{

    resizeColumnsToContents();
    horizontalHeader()->resizeSection(InputEditorModel::COLUMN_SAVESTATE, 20);
    horizontalHeader()->resizeSection(InputEditorModel::COLUMN_FRAME, 80);

    /* Resize analog columns to a fixed value
     * Increase the other columns by a small amount, because even if it's
     * supposed to take the same place as the header, sometimes it considers
     * that it doesn't have enough space. */
    for (int c = InputEditorModel::COLUMN_SPECIAL_SIZE; c < inputEditorModel->columnCount(); c++) {
        int size = horizontalHeader()->sectionSize(c);
        if (inputEditorModel->isInputAnalog(c)) {            
            if (size < 70)
                horizontalHeader()->resizeSection(c, 70);
        }
        else {
            horizontalHeader()->resizeSection(c, size + 2);
        }
    }

}

void InputEditorView::update()
{
    static uint64_t last_framecount = -1;
    if (last_framecount == context->framecount)
        return;

    last_framecount = context->framecount;

    inputEditorModel->update();

    if (!isVisible())
        return;

    /* Check if scroll is frozen because of a rewind. Disable scroll freeze when
     * reaching the paused frame. */
    if (inputEditorModel->isScrollFreeze()) {
        if (context->seek_frame == 0) {
            inputEditorModel->setScrollFreeze(false);
            autoScroll = false;
        }

        if (inputEditorModel->isScrollFreeze())
            return;
    }

    /* Don't autoscroll if disabled in the user options */
    if (!context->config.editor_autoscroll)
        return;

    /* Enable autoscroll if current frame is not visible */
    int toprow = verticalScrollBar()->value();
    int bottomrow = toprow + verticalScrollBar()->pageStep();
    if ((context->framecount >= static_cast<unsigned int>(bottomrow)) || (context->framecount < static_cast<unsigned int>(toprow))) {
        autoScroll = true;
    }

    /* If autoscroll is enable, scroll to make the current frame visible */
    if (autoScroll) {
        scrollToFrame(context->framecount);
    }
}

void InputEditorView::scrollToFrame(unsigned long long frame)
{
    int toprow = verticalScrollBar()->value();
    int bottomrow = toprow + verticalScrollBar()->pageStep();

    /* Place the current frame at about a quarter of the visible frames */
    int visibleFrames = bottomrow - toprow;
    int firstVisibleFrame = frame - 1 - visibleFrames/4;
    if (firstVisibleFrame < 0) firstVisibleFrame = 0;

    /* Get the first visible column */
    int leftcol = columnAt(rect().left());

    /* Scrolling triggers the signal to our manualScroll() slot, which
     * we don't want, so we disable the connection */
    disconnect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &InputEditorView::manualScroll);
    scrollTo(inputEditorModel->index(firstVisibleFrame, leftcol), QAbstractItemView::PositionAtTop);
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &InputEditorView::manualScroll);
}

void InputEditorView::resetInputs()
{
    inputEditorModel->resetInputs();
}

void InputEditorView::updateMenu()
{
    /* Sanity check if this slot is called too early */
    if (!insertAct)
        return;
    
    /* Get the lowest selected row and count markers */
    int min_row = 1 << 30;
    const QModelIndexList indexes = selectionModel()->selectedRows();

    int markers_count = 0;
    for (const QModelIndex index : indexes) {
        if (inputEditorModel->hasMarker(index.row())) {
            markers_count++;
            currentMarkerText = inputEditorModel->getMarkerText(index.row());
        }
        if (index.row() < min_row)
            min_row = index.row();
    }

    markAct->setText(markers_count ? tr("Edit marker") : tr("Add marker"));
    unmarkAct->setText((markers_count > 1) ? tr("Remove markers") : tr("Remove marker"));
    unmarkAct->setEnabled(markers_count);

    if (static_cast<uint32_t>(min_row) < context->framecount) {
        duplicateAct->setEnabled(false);
        insertAct->setEnabled(false);
        insertsAct->setEnabled(false);
        deleteAct->setEnabled(false);
        truncateAct->setEnabled(false);
        clearAct->setEnabled(false);
        cutAct->setEnabled(false);
        pasteAct->setEnabled(false);
        pasteInsertAct->setEnabled(false);
    }
    else {
        duplicateAct->setEnabled(true);
        insertAct->setEnabled(true);
        insertsAct->setEnabled(true);
        deleteAct->setEnabled(true);
        truncateAct->setEnabled(true);
        clearAct->setEnabled(true);
        cutAct->setEnabled(true);
        pasteAct->setEnabled(true);
        pasteInsertAct->setEnabled(true);
    }
}

void InputEditorView::mousePressEvent(QMouseEvent *event)
{
    mouseColumn = -1;

    if (event->button() != Qt::LeftButton) {
        return QTableView::mousePressEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mousePressEvent(event);
    }

    if (index.column() == InputEditorModel::COLUMN_FRAME) {
        return QTableView::mousePressEvent(event);
    }

    selectionModel()->clear();
    mouseColumn = index.column();
    mouseRow = index.row();
    mouseMinRow = mouseRow;
    mouseMaxRow = mouseRow;

    /* Rewind when clicking for column */
    if (mouseColumn == InputEditorModel::COLUMN_SAVESTATE) {
        inputEditorModel->rewind(mouseRow, false);
        return;
    }

    /* For editable items, copy the value. Else, copy the opposite value */
    if (inputEditorModel->flags(index) & Qt::ItemIsEditable) {
        mouseValue = inputEditorModel->data(index, Qt::EditRole).toInt(nullptr);
        return QTableView::mousePressEvent(event);
    }
    else {
        mouseValue = inputEditorModel->toggleInput(index);

        /* Set the min row to toggle inputs */
        minToggleRow = (mouseRow<context->framecount)?mouseRow:context->framecount;
    }

    event->accept();
}

void InputEditorView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return QTableView::mouseDoubleClickEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mouseDoubleClickEvent(event);
    }

    if (index.column() != InputEditorModel::COLUMN_FRAME) {
        return QTableView::mouseDoubleClickEvent(event);
    }

    addMarkerFrame(index.row());

    event->accept();
}

void InputEditorView::wheelEvent(QWheelEvent *event)
{
    if (event->buttons() != Qt::RightButton)
        return QTableView::wheelEvent(event);

    if (event->angleDelta().y() < 0) {
        /* Push a single frame advance event */
        if (context->hotkey_pressed_queue.empty() || context->hotkey_pressed_queue.back() != HOTKEY_FRAMEADVANCE) {
            context->hotkey_pressed_queue.push(HOTKEY_FRAMEADVANCE);
            context->hotkey_released_queue.push(HOTKEY_FRAMEADVANCE);
        }
    }
    else if (event->angleDelta().y() > 0) {
        /* Only rewind when paused, which should prevent rewinding during the end
         * of another rewind */
        if (context->framecount > 0 && !context->config.sc.running)
            inputEditorModel->rewind(context->framecount - 1, false);
    }
    
    event->accept();
}

void InputEditorView::showMarkerToolTip(const QModelIndex &index)
{
    /* Hide previous tooltip */
    hideMarkerToolTip();
    
    /* Check if there is a marker */
    if (index.column() == InputEditorModel::COLUMN_FRAME && inputEditorModel->hasMarker(index.row())) {
        markerRow = index.row();
        markerTimerId = this->startTimer(300);
    }
}

void InputEditorView::mouseMoveEvent(QMouseEvent *event)
{
    /* Check if the mouse press event was valid */
    if (mouseColumn < 0) {
        return QTableView::mouseMoveEvent(event);
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        return QTableView::mouseMoveEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mouseMoveEvent(event);
    }

    event->accept();

    /* Check if we need to toggle this input */
    if ((index.row() >= mouseMinRow) && (index.row() <= mouseMaxRow))
        return;

    /* Prevent toggle past the first toggle frame when rewinding, and
     * if we started to toggle future inputs, don't trigger a rewind */
    if (index.row() < minToggleRow) {
        return;
    }

    int newMouseValue = mouseValue;

    /* Toggle all cells from mouse to minRow-1, or maxRow+1 to mouse */
    int minLoop, maxLoop;
    if (index.row() < mouseMinRow) {
        minLoop = index.row();
        maxLoop = mouseMinRow - 1;
        mouseMinRow = index.row();
    }
    else {
        minLoop = mouseMaxRow + 1;
        maxLoop = index.row();
        mouseMaxRow = index.row();        
    }

    for (int i = minLoop; i <= maxLoop; i++) {
        /* Check if we need to alternate the input state */
        if (event->modifiers() & Qt::ControlModifier) {
            if ((i - mouseRow) % 2)
            newMouseValue = !newMouseValue;
        }
        
        /* Toggle the cell with the same row as the cell under the mouse */
        QModelIndex toggle_index = inputEditorModel->index(i, mouseColumn);
        
        inputEditorModel->setData(toggle_index, QVariant(newMouseValue), Qt::EditRole);        
    }
}

void InputEditorView::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != markerTimerId)
        return QTableView::timerEvent(event);
    
    this->killTimer(markerTimerId);
    markerTimerId = 0;

    int x = columnViewportPosition(InputEditorModel::COLUMN_FRAME) + columnWidth(InputEditorModel::COLUMN_FRAME) / 2;
    int y = rowViewportPosition(markerRow) + rowHeight(markerRow);
    QPoint pos = QPoint(x, y);

    BalloonTip::showBalloon(QIcon(), "",  QString(inputEditorModel->getMarkerText(markerRow).c_str()), this->mapToGlobal(pos), this);
}

void InputEditorView::hideEvent(QHideEvent* event)
{
    hideMarkerToolTip();
}

void InputEditorView::leaveEvent(QEvent *event)
{
    inputEditorModel->setHoveredCell(QModelIndex());
}

void InputEditorView::hideMarkerToolTip()
{
    if (markerTimerId) {
        this->killTimer(markerTimerId);
        markerTimerId = 0;
    }
    BalloonTip::hideBalloon();
}

void InputEditorView::keyPressEvent(QKeyEvent *event)
{
    /* Check for the Home and End keys */
    if ((event->key() == Qt::Key_End) && (event->modifiers() == Qt::NoModifier)) {
        /* Select the last frame */
        QModelIndex newSel = inputEditorModel->index(inputEditorModel->rowCount()-1, 0);
        selectionModel()->clear();
        setCurrentIndex(newSel);
        selectionModel()->select(newSel, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        event->accept();
    }
    else if ((event->key() == Qt::Key_Home) && (event->modifiers() == Qt::NoModifier)) {
        /* Select the first frame */
        QModelIndex newSel = inputEditorModel->index(0, 0);
        selectionModel()->clear();
        setCurrentIndex(newSel);
        selectionModel()->select(newSel, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        event->accept();
    }

    /* We accept hotkeys when this window has focus */
    keysym_t mod = convertQtModifiers(event->modifiers());
    keysym_t ks = context->config.km->nativeToKeysym(event->nativeVirtualKey());

    if (context->config.km->hotkey_mapping.find(ks | mod) != context->config.km->hotkey_mapping.end()) {
        HotKey hk = context->config.km->hotkey_mapping[ks | mod];
        context->hotkey_pressed_queue.push(hk.type);
        return;
    }
    if (context->config.km->hotkey_mapping.find(ks) != context->config.km->hotkey_mapping.end()) {
        HotKey hk = context->config.km->hotkey_mapping[ks];
        context->hotkey_pressed_queue.push(hk.type);
        return;
    }

    return QTableView::keyPressEvent(event);
}

void InputEditorView::keyReleaseEvent(QKeyEvent *event)
{
    /* We accept hotkeys when this window has focus */
    keysym_t mod = convertQtModifiers(event->modifiers());
    keysym_t ks = context->config.km->nativeToKeysym(event->nativeVirtualKey());

    if (context->config.km->hotkey_mapping.find(ks | mod) != context->config.km->hotkey_mapping.end()) {
        HotKey hk = context->config.km->hotkey_mapping[ks | mod];
        context->hotkey_released_queue.push(hk.type);
        return;
    }
    if (context->config.km->hotkey_mapping.find(ks) != context->config.km->hotkey_mapping.end()) {
        HotKey hk = context->config.km->hotkey_mapping[ks];
        context->hotkey_released_queue.push(hk.type);
        return;
    }

    return QTableView::keyReleaseEvent(event);
}

void InputEditorView::horizontalMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    contextSection = horizontalHeader()->logicalIndexAt(pos);

    if (contextSection < InputEditorModel::COLUMN_SPECIAL_SIZE)
        return;

    /* Update the status of the lock action */
    lockAction->setChecked(inputEditorModel->isLockedUniqueInput(contextSection));

    /* Display the context menu */
    horMenu->popup(horizontalHeader()->viewport()->mapToGlobal(pos));
}

void InputEditorView::renameLabel()
{
    if (contextSection < InputEditorModel::COLUMN_SPECIAL_SIZE)
        return;

    QString text = QString("New label for input %1 is: ").arg(inputEditorModel->inputDescription(contextSection).c_str());
    QString newLabel = QInputDialog::getText(this, tr("Rename label"), text, QLineEdit::Normal, QString(inputEditorModel->inputLabel(contextSection).c_str()));

    if (!newLabel.isEmpty()) {
        inputEditorModel->renameLabel(contextSection, newLabel.toStdString());
    }
}

void InputEditorView::addInputColumn()
{
    /* Get an input from the user */
    keysym_t ks = keyDialog->exec();

    /* Remove the custom modifiers that we added in that function */
    ks = ks & 0xffff;

    if (ks != 0) {
        /* Get the remapped input if available */
        if (context->config.km->input_mapping.find(ks) != context->config.km->input_mapping.end()) {
            ks = context->config.km->input_mapping[ks].value;
        }

        /* Get the input with description if available */
        for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
            for (auto iter : context->config.km->input_list[i]) {
                if (iter.type == SingleInput::IT_KEYBOARD) {
                    if (iter.value == ks) {
                        inputEditorModel->addUniqueInput(iter);
                        return;
                    }
                }
            }
        }

        /* Didn't find the input in the list, insert it with the value as description */
        SingleInput si;
        si.type = SingleInput::IT_KEYBOARD;
        si.value = ks;
        si.description = std::to_string(ks);
        inputEditorModel->addUniqueInput(si);
    }
}

void InputEditorView::clearInputColumn()
{
    if (contextSection < InputEditorModel::COLUMN_SPECIAL_SIZE)
        return;

    inputEditorModel->clearUniqueInput(contextSection);
}

void InputEditorView::removeInputColumn()
{
    if (contextSection < InputEditorModel::COLUMN_SPECIAL_SIZE)
        return;

    bool removed = inputEditorModel->removeUniqueInput(contextSection);
    
    /* Show alert window if failed */
    if (!removed)
        QMessageBox::information(this, "Column not removed", tr("Column could not be removed, because the input is present in past frames."));
}

void InputEditorView::lockInputColumn(bool checked)
{
    if (contextSection < InputEditorModel::COLUMN_SPECIAL_SIZE)
        return;

    inputEditorModel->lockUniqueInput(contextSection, checked);
}

void InputEditorView::mainMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    // contextSection = inputEditorView->verticalHeader()->logicalIndexAt(pos);

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    /* Disable context menu for non-frame columns */
    if (index.column() != InputEditorModel::COLUMN_FRAME) {
        return;
    }

    /* Display the context menu */
    menu->popup(viewport()->mapToGlobal(pos));
}

void InputEditorView::getCurrentMarkerText(std::string &marker)
{
    marker = currentMarkerText;
}

void InputEditorView::addMarker()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    return addMarkerFrame(indexes[0].row());
}

void InputEditorView::addMarkerFrame(int frame)
{
    /* Obtain description if there's already a marker here */
    bool ok;
	QString text = QString("Marker text for frame %1: ").arg(frame);
    QString newText = QInputDialog::getText(
        this,
        (inputEditorModel->hasMarker(frame) ? tr("Edit marker") : tr("Add marker")),
        text,
        QLineEdit::Normal,
        QString(inputEditorModel->getMarkerText(frame).c_str()),
        &ok);

    if (ok) {
        emit addMarkerSignal(frame, newText);
    }
    
    InputEditorView::updateMenu();
}

void InputEditorView::removeMarker()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    for (const QModelIndex index : indexes)
        if (inputEditorModel->hasMarker(index.row()))
            emit removeMarkerSignal(index.row());
    
    InputEditorView::updateMenu();
}

void InputEditorView::duplicateInput()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    inputEditorModel->insertRows(indexes[0].row(), 1, true);
}

void InputEditorView::insertInput()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    inputEditorModel->insertRows(indexes[0].row(), 1, false);
}

void InputEditorView::insertInputs()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    bool ok;
    int nbFrames = QInputDialog::getInt(this, tr("Insert frames"), tr("Number of frames to insert: "), 1, 0, 100000, 1, &ok);

    if (ok) {
        inputEditorModel->insertRows(indexes[0].row(), nbFrames, false);
    }
}

int InputEditorView::applyToSelectedRanges(std::function<void(int, int)> func)
{
    /* Extract ranges of selected rows */
    QModelIndexList indexlist = selectionModel()->selectedRows();
    std::sort(indexlist.begin(), indexlist.end());
    
    int cur_row = -1;
    int min_row = -1;
    for (const QModelIndex index : indexlist) {
        if (min_row == -1) {
            min_row = index.row();
            cur_row = index.row();
        }
        else {
            if (index.row() == (cur_row + 1)) {
                cur_row = index.row();
            }
            else {
                func(min_row, cur_row);
                min_row = index.row();
                cur_row = index.row();
            }
        }
    }
    
    if (min_row != -1) {
        func(min_row, cur_row);
    }

    if (indexlist.isEmpty())
        return -1;
        
    return indexlist.first().row();
}

int InputEditorView::applyToSelectedRangesReversed(std::function<void(int, int)> func)
{
    /* Extract ranges of selected rows */
    QModelIndexList indexlist = selectionModel()->selectedRows();
    std::sort(indexlist.begin(), indexlist.end());
    
    int cur_row = -1;
    int max_row = -1;
    for (auto it = indexlist.crbegin(); it != indexlist.crend(); it++) {
        if (max_row == -1) {
            max_row = it->row();
            cur_row = it->row();
        }
        else {
            if (it->row() == (cur_row - 1)) {
                cur_row = it->row();
            }
            else {
                func(cur_row, max_row);
                max_row = it->row();
                cur_row = it->row();
            }
        }
    }

    if (max_row != -1) {
        func(cur_row, max_row);
    }

    if (indexlist.isEmpty())
        return -1;
        
    return indexlist.first().row();
}

void InputEditorView::deleteInput()
{
    if (!selectionModel()->hasSelection())
        return;

    /* Removing rows must be done in reversed order so that row indices are valid */
    int min_row = applyToSelectedRangesReversed([this](int min, int max){inputEditorModel->removeRows(min, max-min+1);});
    
    /* Select the next frame */
    QModelIndex newSel = inputEditorModel->index(min_row, 0);
    selectionModel()->clear();
    setCurrentIndex(newSel);
    selectionModel()->select(newSel, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void InputEditorView::truncateInputs()
{
    QMessageBox confirmBox;

    confirmBox.setText("Really truncate the TAS?");
    confirmBox.setInformativeText("Do you want to truncate all inputs beyond this point?");

    confirmBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    confirmBox.setDefaultButton(QMessageBox::Yes);

    if (confirmBox.exec() != QMessageBox::Yes) return;



    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    int nbRows = inputEditorModel->rowCount();

    inputEditorModel->removeRows(indexes[0].row()+1, nbRows-indexes[0].row()-1);
}

void InputEditorView::clearInput()
{
    for (const QModelIndex index : selectionModel()->selectedRows()) {
        inputEditorModel->clearInput(index.row());
    }
}

void InputEditorView::copyInputs()
{
    if (!selectionModel()->hasSelection())
        return;

    std::ostringstream inputString;
    applyToSelectedRanges([this, &inputString](int min, int max){inputEditorModel->copyInputs(min, max-min+1, inputString);});
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString clipText(inputString.str().c_str());
    clipboard->setText(clipText);
}

void InputEditorView::cutInputs()
{
    if (!selectionModel()->hasSelection())
        return;

    copyInputs();
    
    /* Removing rows must be done in reversed order so that row indices are valid */
    applyToSelectedRangesReversed([this](int min, int max){inputEditorModel->removeRows(min, max-min+1);});
}

void InputEditorView::pasteInputs()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    /* If we selected only one row, paste the all inputs at that row */
    if (indexes.count() == 1) {
        int nbFrames = inputEditorModel->pasteInputs(indexes[0].row());

        /* Select the pasted inputs */
        QModelIndex top = inputEditorModel->index(indexes[0].row(), 0);
        QModelIndex bottom = inputEditorModel->index(indexes[0].row()+nbFrames-1, 0);
        selectionModel()->clear();
        selectionModel()->select(QItemSelection(top, bottom), QItemSelectionModel::Select | QItemSelectionModel::Rows);        
        setCurrentIndex(top);
    }
    
    /* Else, paste inputs inside the selected range, with repeated inputs if
     * there are more selected rows that frames in the clipboard.
     * This allows for exemple to copy a single frame and paste it to multiple
     * rows. */
    else {
        applyToSelectedRanges([this](int min, int max){inputEditorModel->pasteInputsInRange(min, max-min+1);});
    }
}

void InputEditorView::pasteInsertInputs()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    int nbFrames = inputEditorModel->pasteInsertInputs(indexes[0].row());

    /* Select the pasted inputs */
    QModelIndex top = inputEditorModel->index(indexes[0].row(), 0);
    QModelIndex bottom = inputEditorModel->index(indexes[0].row()+nbFrames-1, 0);
    selectionModel()->clear();
    selectionModel()->select(QItemSelection(top, bottom), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    setCurrentIndex(top);
}

void InputEditorView::manualScroll(int)
{
    autoScroll = false;
}

void InputEditorView::moveAgainSection(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    /* We want to keep track of the order of inputs, and not keep a mess
     * between logical and visual indices. So when a user move a section, we
     * move it back and make the change in our list.
     */

    /* Skip if moving the first two columns */
    if ((oldVisualIndex >= InputEditorModel::COLUMN_SPECIAL_SIZE) && (newVisualIndex >= InputEditorModel::COLUMN_SPECIAL_SIZE)) {
        inputEditorModel->moveInputs(oldVisualIndex-InputEditorModel::COLUMN_SPECIAL_SIZE, newVisualIndex-InputEditorModel::COLUMN_SPECIAL_SIZE);
    }

    /* Disconnect before moving back */
    disconnect(horizontalHeader(), &QHeaderView::sectionMoved, this, &InputEditorView::moveAgainSection);
    horizontalHeader()->moveSection(newVisualIndex, oldVisualIndex);
    connect(horizontalHeader(), &QHeaderView::sectionMoved, this, &InputEditorView::moveAgainSection);

    /* We probably need to resize columns */
    resizeAllColumns();
}
