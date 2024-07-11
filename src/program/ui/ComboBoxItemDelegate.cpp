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

#include "ComboBoxItemDelegate.h"
#include "movie/MovieFile.h"
#include "../shared/inputs/SingleInput.h"

#include <QComboBox>

ComboBoxItemDelegate::ComboBoxItemDelegate(MovieFile *m, QObject *parent) : QStyledItemDelegate(parent), movie(m) {}

ComboBoxItemDelegate::~ComboBoxItemDelegate() {}

QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *cb = new QComboBox(parent);
    const int col = index.column();
    if (col == 0) {
        cb->addItem(tr("Key"), SingleInput::IT_KEYBOARD);
        cb->addItem(tr("Mouse Button"), SingleInput::IT_POINTER_BUTTON);
    }
    else {
        int type = index.data(Qt::EditRole).toULongLong() >> 32;
        switch (type) {
            case SingleInput::IT_KEYBOARD:
                for (SingleInput si : movie->editor->input_set) {
                    if (si.type == SingleInput::IT_KEYBOARD)
                        cb->addItem(QString(si.description.c_str()), si.which);
                }
                break;
            case SingleInput::IT_POINTER_BUTTON:
                cb->addItem(tr("Button 1"), SingleInput::POINTER_B1);
                cb->addItem(tr("Button 2"), SingleInput::POINTER_B2);
                cb->addItem(tr("Button 3"), SingleInput::POINTER_B3);
                cb->addItem(tr("Button 4"), SingleInput::POINTER_B4);
                cb->addItem(tr("Button 5"), SingleInput::POINTER_B5);
                break;
        }
    }
    return cb;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(cb);

    const unsigned int currentData = index.data(Qt::EditRole).toULongLong() & 0xffffffff;
    const int cbIndex = cb->findData(currentData);
    if (cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
}

void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentData(), Qt::EditRole);
}
