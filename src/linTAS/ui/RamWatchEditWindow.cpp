/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include "RamWatchEditWindow.h"
#include "../ramsearch/RamWatchDetailed.h"

RamWatchEditWindow::RamWatchEditWindow(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
    setWindowTitle("Edit Watch");

    addressInput = new QLineEdit();
    labelInput = new QLineEdit();

    typeBox = new QComboBox();
    QStringList typeList;
    typeList << "unsigned char" << "char" << "unsigned short" << "short";
    typeList << "unsigned int" << "int" << "unsigned int64" << "int64";
    typeList << "float" << "double";
    typeBox->addItems(typeList);

    displayBox = new QComboBox();
    displayBox->addItem("Decimal");
    displayBox->addItem("Hexadecimal");

    pointerBox = new QCheckBox("");
    connect(pointerBox, &QAbstractButton::clicked, this, &RamWatchEditWindow::slotPointer);

    /* Offset Buttons */
    QPushButton *addOffset = new QPushButton(tr("Add Offset"));
    connect(addOffset, &QAbstractButton::clicked, this, &RamWatchEditWindow::slotAddOffset);

    QPushButton *removeOffset = new QPushButton(tr("Remove Offset"));
    connect(removeOffset, &QAbstractButton::clicked, this, &RamWatchEditWindow::slotRemoveOffset);

    buttonOffsetBox = new QDialogButtonBox();
    buttonOffsetBox->addButton(addOffset, QDialogButtonBox::ActionRole);
    buttonOffsetBox->addButton(removeOffset, QDialogButtonBox::ActionRole);
    buttonOffsetBox->setVisible(false);

    baseAddressInput = new QLineEdit();

    /* Buttons */
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &RamWatchEditWindow::slotSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RamWatchEditWindow::reject);

    /* Create the form layout */
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Address:")), addressInput);
    formLayout->addRow(new QLabel(tr("Label:")), labelInput);
    formLayout->addRow(new QLabel(tr("Type:")), typeBox);
    formLayout->addRow(new QLabel(tr("Display:")), displayBox);
    formLayout->addRow(new QLabel(tr("Pointer:")), pointerBox);

    /* Create the pointer layout */
    pointerLayout = new QFormLayout;
    QSpinBox* offsetBox = new QSpinBox();
    offsetBox->setMaximum(1000000);
    pointerLayout->addRow(new QLabel(tr("Offset:")), offsetBox);
    pointerLayout->addRow(new QLabel(tr("Base Address:")), baseAddressInput);

    for (int r=0; r<pointerLayout->rowCount(); r++) {
        pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->setVisible(false);
        pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget()->setVisible(false);
    }

    /* Create the main layout */
    mainLayout = new QVBoxLayout;

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(pointerLayout);
    mainLayout->addWidget(buttonOffsetBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void RamWatchEditWindow::clear()
{
    /* Clear address */
    addressInput->setText("");
    addressInput->setEnabled(true);

    /* Clear label */
    labelInput->setText("");

    /* Clear display */
    displayBox->setCurrentIndex(0);

    /* Clear type */
    typeBox->setCurrentIndex(0);

    /* Clear pointer */
    pointerBox->setChecked(false);

    baseAddressInput->setText("");

    while (pointerLayout->rowCount() > 1) {
        pointerLayout->removeRow(0);
    }
    QSpinBox* offsetBox = new QSpinBox();
    offsetBox->setMaximum(1000000);
    pointerLayout->insertRow(0, new QLabel(tr("Offset:")), offsetBox);

    slotPointer(false);
}

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatchDetailed> &watch)
{
    /* Fill address */
    addressInput->setText(QString("%1").arg(watch->address, 0, 16));
    addressInput->setEnabled(!watch->isPointer);

    /* Fill label */
    labelInput->setText(watch->label.c_str());

    /* Fill display */
    if (watch->hex)
        displayBox->setCurrentIndex(1);
    else
        displayBox->setCurrentIndex(0);

    /* Fill pointer */
    pointerBox->setChecked(watch->isPointer);

    if (watch->isPointer) {

        baseAddressInput->setText(QString("%1").arg(watch->base_address, 0, 16));

        /* Remove offsets */
        while (pointerLayout->rowCount() > 1) {
            pointerLayout->removeRow(0);
        }

        for (int offset : watch->pointer_offsets) {
            QSpinBox* offsetBox = new QSpinBox();
            offsetBox->setMaximum(1000000);
            offsetBox->setValue(offset);
            pointerLayout->insertRow(0, new QLabel(tr("Offset:")), offsetBox);

        }
    }

    slotPointer(watch->isPointer);
}

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatch> &watch)
{
    clear();

    /* Fill address */
    addressInput->setText(QString("%1").arg(watch->address, 0, 16));
}

void RamWatchEditWindow::slotPointer(bool checked)
{
    /* Show/Hide all widgets in the pointer layout */
    for (int r=0; r<pointerLayout->rowCount(); r++) {
        pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->setVisible(checked);
        pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget()->setVisible(checked);
    }
    buttonOffsetBox->setVisible(checked);

    addressInput->setEnabled(!checked);
}

void RamWatchEditWindow::slotAddOffset()
{
    pointerLayout->insertRow(0, new QLabel(tr("Offset")), new QSpinBox());
}

void RamWatchEditWindow::slotRemoveOffset()
{
    if (pointerLayout->rowCount() > 2) {
        pointerLayout->removeRow(0);
    }
}

void RamWatchEditWindow::slotSave()
{
    bool ok;
    uintptr_t addr = addressInput->text().toULong(&ok, 16);

    if (!ok)
        reject();

    /* Build the ram watch using the right type as template */
    switch (typeBox->currentIndex()) {
        case 0:
            ramwatch.reset(new RamWatchDetailed<unsigned char>(addr));
            break;
        case 1:
            ramwatch.reset(new RamWatchDetailed<char>(addr));
            break;
        case 2:
            ramwatch.reset(new RamWatchDetailed<unsigned short>(addr));
            break;
        case 3:
            ramwatch.reset(new RamWatchDetailed<short>(addr));
            break;
        case 4:
            ramwatch.reset(new RamWatchDetailed<unsigned int>(addr));
            break;
        case 5:
            ramwatch.reset(new RamWatchDetailed<int>(addr));
            break;
        case 6:
            ramwatch.reset(new RamWatchDetailed<uint64_t>(addr));
            break;
        case 7:
            ramwatch.reset(new RamWatchDetailed<int64_t>(addr));
            break;
        case 8:
            ramwatch.reset(new RamWatchDetailed<float>(addr));
            break;
        case 9:
            ramwatch.reset(new RamWatchDetailed<double>(addr));
            break;
    }

    ramwatch->hex = (displayBox->currentIndex() == 1);
    ramwatch->label = labelInput->text().toStdString();
    ramwatch->isPointer = pointerBox->isChecked();
    if (ramwatch->isPointer) {
        ramwatch->base_address = baseAddressInput->text().toULong(&ok, 16);

        if (!ok)
            reject();

        ramwatch->pointer_offsets.clear();
        for (int r=0; r<pointerLayout->rowCount()-1; r++) {
            QSpinBox* offsetBox = qobject_cast<QSpinBox*>(pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget());
            ramwatch->pointer_offsets.push_back(offsetBox->value());
        }
    }

    accept();
}
