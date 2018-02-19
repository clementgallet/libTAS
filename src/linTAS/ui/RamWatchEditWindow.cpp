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

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "RamWatchEditWindow.h"
#include "../ramsearch/RamWatchDetailed.h"

// #include <sstream>

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

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatchDetailed> &watch)
{
    /* Fill address */
    addressInput->setText(QString("%1").arg(watch->address, 0, 16));

    /* Fill label */
    labelInput->setText(watch->label.c_str());

    /* Fill display */
    if (watch->hex)
        displayBox->setCurrentIndex(1);
    else
        displayBox->setCurrentIndex(0);
}

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatch> &watch)
{
    /* Fill address */
    addressInput->setText(QString("%1").arg(watch->address, 0, 16));
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

    accept();
}
