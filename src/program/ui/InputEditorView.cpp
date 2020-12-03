/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QInputDialog>
#include <QHeaderView>
#include <QScrollBar>

#include "InputEditorView.h"
#include "MainWindow.h"
#include "qtutils.h"

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
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());

    /* Track vertical scrolling */
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &InputEditorView::manualScroll);

    /* Main menu */
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &InputEditorView::mainMenu);

    menu = new QMenu(this);
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

    keyDialog = new KeyPressedDialog(this);
    keyDialog->withModifiers = true;
}

void InputEditorView::resizeAllColumns()
{

    resizeColumnsToContents();
    horizontalHeader()->resizeSection(0, 20);
    horizontalHeader()->resizeSection(1, 80);

    /* Resize analog columns to a fixed value */
    for (int c = 2; c < inputEditorModel->columnCount(); c++)
        if (inputEditorModel->isInputAnalog(c)) {
            int size = horizontalHeader()->sectionSize(c);
            if (size < 70)
                horizontalHeader()->resizeSection(c, 70);
        }

}

void InputEditorView::update()
{
    static uint64_t last_framecount = -1;
    if (last_framecount == context->framecount)
        return;

    inputEditorModel->update();

    if (!isVisible())
        return;

    /* Enable autoscroll if current frame is not visible */
    int toprow = rowAt(rect().top());
    int bottomrow = rowAt(rect().bottom());
    if ((context->framecount >= bottomrow) || (context->framecount < toprow)) {
        autoScroll = true;
    }

    /* If autoscroll is enable, scroll to make the current frame visible */
    if (autoScroll) {
        /* Place the current frame at about a quarter of the visible frames */
        int visibleFrames = bottomrow - toprow;
        int firstVisibleFrame = context->framecount - 1 - visibleFrames/4;
        if (firstVisibleFrame < 0) firstVisibleFrame = 0;

        /* Get the first visible column */
        int leftcol = columnAt(rect().left());

        /* Scrolling triggers the signal to our manualScroll() slot, which
         * we don't want, so we disable the connection */
        disconnect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &InputEditorView::manualScroll);
        scrollTo(inputEditorModel->index(firstVisibleFrame, leftcol), QAbstractItemView::PositionAtTop);
        connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this, &InputEditorView::manualScroll);
    }
    last_framecount = context->framecount;
}

void InputEditorView::resetInputs()
{
    inputEditorModel->resetInputs();
}

void InputEditorView::mousePressEvent(QMouseEvent *event)
{
    mouseSection = -1;

    if ((event->button() != Qt::LeftButton) && (event->button() != Qt::RightButton)) {
        return QTableView::mousePressEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mousePressEvent(event);
    }

    /* Disable some items on the context menu */
    if (event->button() == Qt::RightButton) {
        if (index.row() < context->framecount) {
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
            insertAct->setEnabled(true);
            insertsAct->setEnabled(true);
            deleteAct->setEnabled(true);
            truncateAct->setEnabled(true);
            clearAct->setEnabled(true);
            cutAct->setEnabled(true);
            pasteAct->setEnabled(true);
            pasteInsertAct->setEnabled(true);
        }
        return QTableView::mousePressEvent(event);
    }

    if (index.column() < 2) {
        return QTableView::mousePressEvent(event);
    }

    selectionModel()->clear();
    mouseSection = index.column();
    mouseRow = index.row();

    /* For editable items, copy the value. Else, copy the opposite value */
    if (inputEditorModel->flags(index) & Qt::ItemIsEditable) {
        mouseValue = inputEditorModel->data(index, Qt::EditRole).toInt(nullptr);
        return QTableView::mousePressEvent(event);
    }
    else {
        mouseValue = inputEditorModel->toggleInput(index);
    }

    event->accept();
}

void InputEditorView::mouseMoveEvent(QMouseEvent *event)
{
    /* Check if the mouse press event was valid */
    if (mouseSection < 0) {
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

    int newMouseValue = mouseValue;

    /* Check if we need to alternate the input state */
    if (event->modifiers() & Qt::ControlModifier) {
        if ((index.row() - mouseRow) % 2)
            newMouseValue = !newMouseValue;
    }

    /* Toggle the cell with the same row as the cell under the mouse */
    QModelIndex toggle_index = inputEditorModel->index(index.row(), mouseSection);

    inputEditorModel->setData(toggle_index, QVariant(newMouseValue), Qt::EditRole);
    event->accept();
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
    xcb_keysym_t mod = convertQtModifiers(event->modifiers());

    if (context->config.km.hotkey_mapping.find(event->nativeVirtualKey() | mod) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[event->nativeVirtualKey() | mod];
        context->hotkey_pressed_queue.push(hk.type);
        return;
    }
    if (context->config.km.hotkey_mapping.find(event->nativeVirtualKey()) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[event->nativeVirtualKey()];
        context->hotkey_pressed_queue.push(hk.type);
        return;
    }

    return QTableView::keyPressEvent(event);
}

void InputEditorView::keyReleaseEvent(QKeyEvent *event)
{
    /* We accept hotkeys when this window has focus */
    xcb_keysym_t mod = convertQtModifiers(event->modifiers());

    if (context->config.km.hotkey_mapping.find(event->nativeVirtualKey() | mod) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[event->nativeVirtualKey() | mod];
        context->hotkey_released_queue.push(hk.type);
        return;
    }
    if (context->config.km.hotkey_mapping.find(event->nativeVirtualKey()) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[event->nativeVirtualKey()];
        context->hotkey_released_queue.push(hk.type);
        return;
    }

    return QTableView::keyReleaseEvent(event);
}

void InputEditorView::horizontalMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    contextSection = horizontalHeader()->logicalIndexAt(pos);

    if (contextSection < 2)
        return;

    /* Update the status of the lock action */
    lockAction->setChecked(inputEditorModel->isLockedUniqueInput(contextSection));

    /* Display the context menu */
    horMenu->popup(horizontalHeader()->viewport()->mapToGlobal(pos));
}

void InputEditorView::renameLabel()
{
    if (contextSection < 2)
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
    xcb_keysym_t ks = keyDialog->exec();

    /* Remove the custom modifiers that we added in that function */
    ks = ks & 0xffff;

    /* Get the input with description if available */
    for (auto iter : context->config.km.input_list) {
        if (iter.type == SingleInput::IT_KEYBOARD) {
            if (iter.value == ks) {
                inputEditorModel->addUniqueInput(iter);
                return;
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

void InputEditorView::clearInputColumn()
{
    if (contextSection < 2)
        return;

    inputEditorModel->clearUniqueInput(contextSection);
}

void InputEditorView::removeInputColumn()
{
    if (contextSection < 2)
        return;

    inputEditorModel->removeUniqueInput(contextSection);    
}

void InputEditorView::lockInputColumn(bool checked)
{
    if (contextSection < 2)
        return;

    inputEditorModel->lockUniqueInput(contextSection, checked);
}

void InputEditorView::mainMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    // contextSection = inputEditorView->verticalHeader()->logicalIndexAt(pos);

    /* Display the context menu */
    menu->popup(viewport()->mapToGlobal(pos));
}

void InputEditorView::insertInput()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    inputEditorModel->insertRows(indexes[0].row(), 1);
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
        inputEditorModel->insertRows(indexes[0].row(), nbFrames);
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

    inputEditorModel->clearClipboard();
    applyToSelectedRanges([this](int min, int max){inputEditorModel->copyInputs(min, max-min+1);});
}

void InputEditorView::cutInputs()
{
    if (!selectionModel()->hasSelection())
        return;

    inputEditorModel->clearClipboard();
    applyToSelectedRanges([this](int min, int max){inputEditorModel->copyInputs(min, max-min+1);});
    /* Removing rows must be done in reversed order so that row indices are valid */
    applyToSelectedRangesReversed([this](int min, int max){inputEditorModel->removeRows(min, max-min+1);});
}

void InputEditorView::pasteInputs()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return;

    int nbFrames = inputEditorModel->pasteInputs(indexes[0].row());

    /* Select the pasted inputs */
    QModelIndex top = inputEditorModel->index(indexes[0].row(), 0);
    QModelIndex bottom = inputEditorModel->index(indexes[0].row()+nbFrames-1, 0);
    selectionModel()->clear();
    selectionModel()->select(QItemSelection(top, bottom), QItemSelectionModel::Select | QItemSelectionModel::Rows);
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
    if ((oldVisualIndex >= 2) && (newVisualIndex >= 2)) {
        inputEditorModel->moveInputs(oldVisualIndex-2, newVisualIndex-2);
    }

    /* Disconnect before moving back */
    disconnect(horizontalHeader(), &QHeaderView::sectionMoved, this, &InputEditorView::moveAgainSection);
    horizontalHeader()->moveSection(newVisualIndex, oldVisualIndex);
    connect(horizontalHeader(), &QHeaderView::sectionMoved, this, &InputEditorView::moveAgainSection);

    /* We probably need to resize columns */
    resizeAllColumns();
}
