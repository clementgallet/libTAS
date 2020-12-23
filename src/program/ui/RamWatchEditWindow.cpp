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

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QMessageBox>

#include "RamWatchEditWindow.h"
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
    pointerLayout = new QFormLayout;
    for (int r=0; r<8; r++) {
        QSpinBox* offsetBox = new QSpinBox();
        offsetBox->setMaximum(1000000);
        pointerLayout->addRow(new QLabel(tr("Offset:")), offsetBox);
    }
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

        unsigned int c = pointerLayout->rowCount() - 1;
        for (unsigned int r=0; r<c; r++) {
            if (r < watch->pointer_offsets.size()) {
                pointerLayout->itemAt(c-r-1, QFormLayout::LabelRole)->widget()->setVisible(true);
                pointerLayout->itemAt(c-r-1, QFormLayout::FieldRole)->widget()->setVisible(true);
                QSpinBox* offsetBox = qobject_cast<QSpinBox*>(pointerLayout->itemAt(c-r-1, QFormLayout::FieldRole)->widget());
                offsetBox->setValue(watch->pointer_offsets[r]);
            }
            else {
                pointerLayout->itemAt(c-r-1, QFormLayout::LabelRole)->widget()->setVisible(false);
                pointerLayout->itemAt(c-r-1, QFormLayout::FieldRole)->widget()->setVisible(false);
            }
        }
    }
}

void RamWatchEditWindow::fill(const RamWatch &watch)
{
    clear();

    /* Fill address */
    addressInput->setText(QString("%1").arg(watch.address, 0, 16));

    /* Fill type */
    typeBox->setCurrentIndex(watch.type);
}

void RamWatchEditWindow::slotPointer(bool checked)
{
    /* Show/Hide all widgets in the pointer layout */
    for (int r=0; r<pointerLayout->rowCount(); r++) {
        pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->setVisible(false);
        pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget()->setVisible(false);
    }
    if (checked) {
        unsigned int c = pointerLayout->rowCount() - 1;
        pointerLayout->itemAt(c-1, QFormLayout::LabelRole)->widget()->setVisible(true);
        pointerLayout->itemAt(c-1, QFormLayout::FieldRole)->widget()->setVisible(true);
        pointerLayout->itemAt(c, QFormLayout::LabelRole)->widget()->setVisible(true);
        pointerLayout->itemAt(c, QFormLayout::FieldRole)->widget()->setVisible(true);
    }
    buttonOffsetBox->setVisible(checked);

    addressInput->setEnabled(!checked);
}

void RamWatchEditWindow::slotAddOffset()
{
    for (int r=pointerLayout->rowCount()-2; r>=0; r--) {
        if (!pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->isVisible()) {
            pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->setVisible(true);
            pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget()->setVisible(true);
            break;
        }
    }
}

void RamWatchEditWindow::slotRemoveOffset()
{
    for (int r=0; r<pointerLayout->rowCount()-2; r++) {
        if (pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->isVisible()) {
            pointerLayout->itemAt(r, QFormLayout::LabelRole)->widget()->setVisible(false);
            pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget()->setVisible(false);
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
        case RamWatch::RamUnsignedChar:
            ramwatch.reset(new RamWatchDetailed<unsigned char>(addr));
            break;
        case RamWatch::RamChar:
            ramwatch.reset(new RamWatchDetailed<char>(addr));
            break;
        case RamWatch::RamUnsignedShort:
            ramwatch.reset(new RamWatchDetailed<unsigned short>(addr));
            break;
        case RamWatch::RamShort:
            ramwatch.reset(new RamWatchDetailed<short>(addr));
            break;
        case RamWatch::RamUnsignedInt:
            ramwatch.reset(new RamWatchDetailed<unsigned int>(addr));
            break;
        case RamWatch::RamInt:
            ramwatch.reset(new RamWatchDetailed<int>(addr));
            break;
        case RamWatch::RamUnsignedLong:
            ramwatch.reset(new RamWatchDetailed<uint64_t>(addr));
            break;
        case RamWatch::RamLong:
            ramwatch.reset(new RamWatchDetailed<int64_t>(addr));
            break;
        case RamWatch::RamFloat:
            ramwatch.reset(new RamWatchDetailed<float>(addr));
            break;
        case RamWatch::RamDouble:
            ramwatch.reset(new RamWatchDetailed<double>(addr));
            break;
    }

    ramwatch->hex = (displayBox->currentIndex() == 1);
    ramwatch->label = labelInput->text().toStdString();
    ramwatch->isPointer = pointerBox->isChecked();
    if (ramwatch->isPointer) {
        std::string base = baseAddressInput->text().toStdString();

        /* Split the string with '+' or '-' sign */
        size_t sep = base.find_last_of("+0x");
        if (sep != std::string::npos)
            sep = base.find_last_of("-0x");

        if (sep != std::string::npos) {
            ramwatch->base_file = base.substr(0, sep);
            ramwatch->base_file_offset = std::stoll(base.substr(sep + 1), nullptr, 16);
        }
        else {
            ramwatch->base_file = "";
            ramwatch->base_file_offset = std::stoll(base, nullptr, 16);
        }
        /* Reset base address to it is recomputed */
        ramwatch->base_address = 0;

        ramwatch->pointer_offsets.clear();
        for (int r=pointerLayout->rowCount()-2; r>=0; r--) {
            if (pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget()->isVisible()) {
                QSpinBox* offsetBox = qobject_cast<QSpinBox*>(pointerLayout->itemAt(r, QFormLayout::FieldRole)->widget());
                ramwatch->pointer_offsets.push_back(offsetBox->value());
            }
        }
    }

    accept();
}
