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

#include "RamWatchView.h"
#include "RamWatchWindow.h"
#include "RamWatchModel.h"
#include "PointerScanWindow.h"

#include "Context.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QSettings>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>

RamWatchWindow::RamWatchWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Ram Watch");

    /* Table */
    ramWatchView = new RamWatchView(c, this);

    /* Buttons */
    QPushButton *addWatch = new QPushButton(tr("Add Watch"));
    connect(addWatch, &QAbstractButton::clicked, ramWatchView, &RamWatchView::slotAdd);

    QPushButton *editWatch = new QPushButton(tr("Edit Watch"));
    connect(editWatch, &QAbstractButton::clicked, ramWatchView, &RamWatchView::slotEdit);

    QPushButton *removeWatch = new QPushButton(tr("Remove Watch"));
    connect(removeWatch, &QAbstractButton::clicked, ramWatchView, &RamWatchView::slotRemove);

    QPushButton *scanWatch = new QPushButton(tr("Scan Pointer"));
    connect(scanWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotScanPointer);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(addWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(editWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(removeWatch, QDialogButtonBox::ActionRole);
    buttonBox->addButton(scanWatch, QDialogButtonBox::ActionRole);

    /* Other buttons */
    QPushButton *saveWatch = new QPushButton(tr("Save Watches"));
    connect(saveWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotSave);

    QPushButton *loadWatch = new QPushButton(tr("Load Watches"));
    connect(loadWatch, &QAbstractButton::clicked, this, &RamWatchWindow::slotLoad);

    QDialogButtonBox *buttonBox2 = new QDialogButtonBox();
    buttonBox2->addButton(saveWatch, QDialogButtonBox::ActionRole);
    buttonBox2->addButton(loadWatch, QDialogButtonBox::ActionRole);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(ramWatchView);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(buttonBox2);

    setLayout(mainLayout);

    pointerScanWindow = new PointerScanWindow(c, this);
}

void RamWatchWindow::update()
{
    ramWatchView->update();
}

void RamWatchWindow::slotScanPointer()
{
    const QModelIndex index = ramWatchView->selectionModel()->currentIndex();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    int row = index.row();

    /* Fill and show the watch edit window */
    pointerScanWindow->addressInput->setText(QString("%1").arg(ramWatchView->ramWatchModel->ramwatches.at(row)->address, 0, 16));
    pointerScanWindow->exec();
}

void RamWatchWindow::slotSave()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Choose a watch file"), context->gamepath.c_str(), tr("watch files (*.wch)"));
    if (filename.isNull()) {
        return;
    }

    std::string watchFile = filename.toStdString();

    /* Check or add .wch extension */
    if ((watchFile.length() < 4) || (watchFile.compare(watchFile.length()-4, 4, ".wch") != 0)) {
        watchFile += ".wch";
    }

	QSettings watchSettings(QString(watchFile.c_str()), QSettings::IniFormat);
	watchSettings.setFallbacksEnabled(false);

    ramWatchView->ramWatchModel->saveSettings(watchSettings);
}

void RamWatchWindow::slotLoad()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose a watch file"), context->gamepath.c_str(), tr("watch files (*.wch)"));
    if (filename.isNull()) {
        return;
    }

    std::string watchFile = filename.toStdString();

	QSettings watchSettings(QString(watchFile.c_str()), QSettings::IniFormat);
	watchSettings.setFallbacksEnabled(false);

    ramWatchView->ramWatchModel->loadSettings(watchSettings);
}
