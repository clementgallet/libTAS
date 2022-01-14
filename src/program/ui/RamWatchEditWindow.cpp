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

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QMessageBox>

#include <stdint.h>

#include "RamWatchEditWindow.h"
#include "../ramsearch/TypeIndex.h"
#include "../ramsearch/IRamWatchDetailed.h"
#include "../ramsearch/RamWatchDetailed.h"

RamWatchEditWindow::RamWatchEditWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Edit Watch");

    addressInput = new QLineEdit();
    valueInput = new QLineEdit();
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
    QPushButton *pokeButton = new QPushButton(tr("Poke Value"));
    connect(pokeButton, &QAbstractButton::clicked, this, &RamWatchEditWindow::slotPoke);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttonBox->addButton(pokeButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &RamWatchEditWindow::slotSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RamWatchEditWindow::reject);

    /* Create the form layout */
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Address:")), addressInput);
    formLayout->addRow(new QLabel(tr("Value:")), valueInput);
    formLayout->addRow(new QLabel(tr("Label:")), labelInput);
    formLayout->addRow(new QLabel(tr("Type:")), typeBox);
    formLayout->addRow(new QLabel(tr("Display:")), displayBox);
    formLayout->addRow(new QLabel(tr("Pointer:")), pointerBox);

    /* Create the pointer layout */
    pointerLayout = new QGridLayout;
    for (int r=0; r<POINTER_CHAIN_SIZE; r++) {
        QLineEdit* offset = new QLineEdit();
        pointerLayout->addWidget(new QLabel(tr("Offset:")), r, 0);
        pointerLayout->addWidget(offset, r, 1);
        pointerLayout->addWidget(new QLabel(tr("")), r, 2);
    }
    pointerLayout->addWidget(new QLabel(tr("Base Address:")), POINTER_CHAIN_SIZE, 0);
    pointerLayout->addWidget(baseAddressInput, POINTER_CHAIN_SIZE, 1);
    pointerLayout->addWidget(new QLabel(tr("")), POINTER_CHAIN_SIZE, 2);

    for (int r=0; r<POINTER_CHAIN_SIZE+1; r++) {
        pointerLayout->itemAtPosition(r, 0)->widget()->setVisible(false);
        pointerLayout->itemAtPosition(r, 1)->widget()->setVisible(false);
        pointerLayout->itemAtPosition(r, 2)->widget()->setVisible(false);
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
    addressInput->setText("");
    addressInput->setEnabled(true);
    valueInput->setText("");
    labelInput->setText("");
    displayBox->setCurrentIndex(0);
    typeBox->setCurrentIndex(0);

    pointerBox->setChecked(false);
    baseAddressInput->setText("");

    slotPointer(false);
}

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatchDetailed> &watch)
{
    /* Fill address */
    addressInput->setText(QString("%1").arg(watch->address, 0, 16));
    addressInput->setEnabled(!watch->isPointer);

    /* Fill value */
    valueInput->setText(watch->value_str().c_str());

    /* Fill label */
    labelInput->setText(watch->label.c_str());

    /* Fill type using virtual function */
    typeBox->setCurrentIndex(watch->type());

    /* Fill display */
    if (watch->hex)
        displayBox->setCurrentIndex(1);
    else
        displayBox->setCurrentIndex(0);

    /* Fill pointer */
    pointerBox->setChecked(watch->isPointer);

    slotPointer(watch->isPointer);

    if (watch->isPointer) {

        if (watch->base_file_offset >= 0)
            baseAddressInput->setText(QString("%1+0x%2").arg(watch->base_file.c_str()).arg(watch->base_file_offset, 0, 16));
        else
            baseAddressInput->setText(QString("%1-0x%2").arg(watch->base_file.c_str()).arg(-watch->base_file_offset, 0, 16));

        if (!watch->pointer_addresses.empty()) {
            QLabel* label = qobject_cast<QLabel*>(pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE, 2)->widget());
            label->setText(QString("-> %1").arg(watch->pointer_addresses[0], 0, 16));            
        }

        for (unsigned int r=0; r<POINTER_CHAIN_SIZE; r++) {
            if (r < watch->pointer_offsets.size()) {
                pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 0)->widget()->setVisible(true);
                pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 1)->widget()->setVisible(true);
                pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 2)->widget()->setVisible(true);

                QLineEdit* offset = qobject_cast<QLineEdit*>(pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 1)->widget());
                offset->setText(QString("%1").arg(watch->pointer_offsets[r], 0, 16));

                if (!watch->pointer_addresses.empty()) {
                    QLabel* label = qobject_cast<QLabel*>(pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 2)->widget());
                    if (r < (watch->pointer_offsets.size()-1)) {
                        if (watch->pointer_addresses[r] == 0)
                            label->setText(QString("[??????+%1] -> ??????").arg(watch->pointer_offsets[r], 0, 16));
                        else if (watch->pointer_addresses[r+1] == 0)
                            label->setText(QString("[%1+%2] -> ??????").arg(watch->pointer_addresses[r], 0, 16).arg(watch->pointer_offsets[r], 0, 16));
                        else
                            label->setText(QString("[%1+%2] -> %3").arg(watch->pointer_addresses[r], 0, 16).arg(watch->pointer_offsets[r], 0, 16).arg(watch->pointer_addresses[r+1], 0, 16));
                    }
                    else {
                        if (watch->pointer_addresses[r] == 0)
                            label->setText(QString("??????+%1 -> ??????").arg(watch->pointer_offsets[r], 0, 16));
                        else
                            label->setText(QString("%1+%2 -> %3").arg(watch->pointer_addresses[r], 0, 16).arg(watch->pointer_offsets[r], 0, 16).arg(watch->address, 0, 16));
                    }
                }
            }
            else {
                pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 0)->widget()->setVisible(false);
                pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 1)->widget()->setVisible(false);
                pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-r-1, 2)->widget()->setVisible(false);
            }
        }
    }
}

void RamWatchEditWindow::fill(uintptr_t addr, int type)
{
    clear();

    /* Fill address */
    addressInput->setText(QString("%1").arg(addr, 0, 16));

    /* Fill type */
    typeBox->setCurrentIndex(type);
}

void RamWatchEditWindow::slotPointer(bool checked)
{
    /* Show/Hide all widgets in the pointer layout */
    for (int r=0; r<POINTER_CHAIN_SIZE+1; r++) {
        pointerLayout->itemAtPosition(r, 0)->widget()->setVisible(false);
        pointerLayout->itemAtPosition(r, 1)->widget()->setVisible(false);
        pointerLayout->itemAtPosition(r, 2)->widget()->setVisible(false);
    }
    if (checked) {
        pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-1, 0)->widget()->setVisible(true);
        pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-1, 1)->widget()->setVisible(true);
        pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE-1, 2)->widget()->setVisible(true);
        pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE, 0)->widget()->setVisible(true);
        pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE, 1)->widget()->setVisible(true);
        pointerLayout->itemAtPosition(POINTER_CHAIN_SIZE, 2)->widget()->setVisible(true);
    }
    buttonOffsetBox->setVisible(checked);

    addressInput->setEnabled(!checked);
}

void RamWatchEditWindow::slotAddOffset()
{
    for (int r=POINTER_CHAIN_SIZE-1; r>=0; r--) {
        if (!pointerLayout->itemAtPosition(r, 0)->widget()->isVisible()) {
            pointerLayout->itemAtPosition(r, 0)->widget()->setVisible(true);
            pointerLayout->itemAtPosition(r, 1)->widget()->setVisible(true);
            pointerLayout->itemAtPosition(r, 2)->widget()->setVisible(true);
            break;
        }
    }
}

void RamWatchEditWindow::slotRemoveOffset()
{
    for (int r=0; r<POINTER_CHAIN_SIZE-1; r++) {
        if (pointerLayout->itemAtPosition(r, 0)->widget()->isVisible()) {
            pointerLayout->itemAtPosition(r, 0)->widget()->setVisible(false);
            pointerLayout->itemAtPosition(r, 1)->widget()->setVisible(false);
            pointerLayout->itemAtPosition(r, 2)->widget()->setVisible(false);
            break;
        }
    }
}

void RamWatchEditWindow::slotPoke()
{
    /* Save fields into the ram watch */
    slotSave();

    int res = ramwatch->poke_value(valueInput->text().toStdString());

    if (res < 0) {
        if (res == EFAULT) {
            QMessageBox::critical(nullptr, "Error", QString("Poking failed because address is outside the game's accessible address space."));
        }
        else if (res == EPERM) {
            QMessageBox::critical(nullptr, "Error", QString("Poking failed because we don't have permission to write at the game address."));
        }
        else if (res == ESRCH) {
            QMessageBox::critical(nullptr, "Error", QString("Poking failed because the game does not appear to be running."));
        }
        else {
            QMessageBox::critical(nullptr, "Error", QString("Poking failed."));
        }
    }
    else {
        QMessageBox::information(nullptr, "Success", QString("Poking succeeded."));
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
        case RamUnsignedChar:
            ramwatch.reset(new RamWatchDetailed<unsigned char>(addr));
            break;
        case RamChar:
            ramwatch.reset(new RamWatchDetailed<char>(addr));
            break;
        case RamUnsignedShort:
            ramwatch.reset(new RamWatchDetailed<unsigned short>(addr));
            break;
        case RamShort:
            ramwatch.reset(new RamWatchDetailed<short>(addr));
            break;
        case RamUnsignedInt:
            ramwatch.reset(new RamWatchDetailed<unsigned int>(addr));
            break;
        case RamInt:
            ramwatch.reset(new RamWatchDetailed<int>(addr));
            break;
        case RamUnsignedLong:
            ramwatch.reset(new RamWatchDetailed<uint64_t>(addr));
            break;
        case RamLong:
            ramwatch.reset(new RamWatchDetailed<int64_t>(addr));
            break;
        case RamFloat:
            ramwatch.reset(new RamWatchDetailed<float>(addr));
            break;
        case RamDouble:
            ramwatch.reset(new RamWatchDetailed<double>(addr));
            break;
    }

    ramwatch->hex = (displayBox->currentIndex() == 1);
    ramwatch->label = labelInput->text().toStdString();
    ramwatch->isPointer = pointerBox->isChecked();
    if (ramwatch->isPointer) {
        std::string base = baseAddressInput->text().toStdString();

        /* Split the string with '+' or '-' sign */
        size_t sep = base.find("+0x");
        if (sep == std::string::npos)
            sep = base.find("-0x");

        if (sep != std::string::npos) {
            ramwatch->base_file = base.substr(0, sep);
            try {
                ramwatch->base_file_offset = std::stoll(base.substr(sep), nullptr, 16);
            }
            catch (const std::invalid_argument& ia) {
                ramwatch->base_file_offset = 0;                
            }
        }
        else {
            ramwatch->base_file = "";
            try {
                ramwatch->base_file_offset = std::stoll(base, nullptr, 16);
            }
            catch (const std::invalid_argument& ia) {
                ramwatch->base_file_offset = 0;
            }            
        }
        /* Reset base address to it is recomputed */
        ramwatch->base_address = 0;

        ramwatch->pointer_offsets.clear();
        for (int r=POINTER_CHAIN_SIZE-1; r>=0; r--) {
            if (pointerLayout->itemAtPosition(r, 1)->widget()->isVisible()) {
                QLineEdit* offset = qobject_cast<QLineEdit*>(pointerLayout->itemAtPosition(r, 1)->widget());

                int val = offset->text().toInt(&ok, 16);
                if (ok)
                    ramwatch->pointer_offsets.push_back(val);
                else
                    ramwatch->pointer_offsets.push_back(0);
            }
        }
    }

    accept();
}
