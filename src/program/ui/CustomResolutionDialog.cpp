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

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>

#include "CustomResolutionDialog.h"

CustomResolutionDialog::CustomResolutionDialog(QWidget *parent) : QDialog(parent) {

    setWindowTitle("");
    setFocusPolicy(Qt::ClickFocus);

    widthField = new QSpinBox();
    widthField->setMaximum(1 << 16);
    widthField->setMinimum(1);

    heightField = new QSpinBox();
    heightField->setMaximum(1 << 16);
    heightField->setMinimum(1);

    QHBoxLayout *fieldLayout = new QHBoxLayout;
    fieldLayout->addWidget(widthField);
    fieldLayout->addWidget(new QLabel(tr(":")));
    fieldLayout->addWidget(heightField);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CustomResolutionDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CustomResolutionDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addLayout(fieldLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void CustomResolutionDialog::update(int width, int height)
{
    widthField->setValue(width);
    heightField->setValue(height);
}

void CustomResolutionDialog::slotOk()
{
    int value = (widthField->value() << 16) | heightField->value();
    done(value);
}
