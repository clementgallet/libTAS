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

#include "HexViewWindow.h"
#include "../external/qhexview/include/QHexView/qhexview.h"
#include "../external/qhexview/include/QHexView/model/qhexdocument.h"
#include "../external/qhexview/include/QHexView/model/buffer/qdevicebuffer.h"
#include "ramsearch/IOProcessDevice.h"
#include "ramsearch/BaseAddresses.h"
#include "ramsearch/MemSection.h"
#include "ramsearch/MemLayout.h"

#include <QtWidgets/QVBoxLayout>
#include <iostream>

HexViewWindow::HexViewWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Hex View");
    iodevice = new IOProcessDevice(this);
    QHexDocument* doc = QHexDocument::fromDevice<QDeviceBuffer>(iodevice);

    view = new QHexView(this);
    view->setDocument(doc);
    connect(view, &QHexView::positionChanged, this, &HexViewWindow::positionChanged);

    selectionLabel = new QLabel();

    sectionChoice = new QComboBox();
    connect(sectionChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &HexViewWindow::switch_section);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(view);
    mainLayout->addWidget(selectionLabel);
    mainLayout->addWidget(sectionChoice);
    setLayout(mainLayout);
}

QSize HexViewWindow::sizeHint() const
{
    return QSize(740, 600);
}

void HexViewWindow::start()
{
    update_layout();
}

void HexViewWindow::update()
{
    view->viewport()->update();
    positionChanged();
}

void HexViewWindow::update_layout()
{
    /* Read the whole memory layout */
    MemLayout memlayout;

    memsections.clear();
    
    MemSection section;
    while (memlayout.nextSection(MemSection::MemAll, 0, section)) {
        memsections.push_back(section);
    }

    update_sections();
}

void HexViewWindow::update_sections()
{
    sectionChoice->clear();
    
    for (size_t i=0; i<memsections.size(); i++) {
        const MemSection& section = memsections[i];
        QString label = QString("%1-%2 %3").arg(section.addr, 0, 16).arg(section.endaddr, 0, 16).arg(section.filename.c_str());
        sectionChoice->addItem(label);
    }

    sectionChoice->setCurrentIndex(0);
    switch_section();
}

void HexViewWindow::switch_section()
{
    if (memsections.empty())
        return;
        
    const MemSection& section = memsections[sectionChoice->currentIndex()];
    iodevice->setSection(section);
    view->setBaseAddress(section.addr);
}

void HexViewWindow::seek(uintptr_t addr, int size)
{
    if (memsections.empty())
        return;
        
    qint64 seek_offset = -1;
    for (size_t i=0; i<memsections.size(); i++) {
        const MemSection& section = memsections[i];
        if (addr >= section.addr && addr < section.endaddr) {
            seek_offset = addr - section.addr;
            sectionChoice->setCurrentIndex(i);
            switch_section();
            break;
        }
    }
    
    /* If not found, update the memory layout */
    if (seek_offset == -1) {
        update_layout();

        for (size_t i=0; i<memsections.size(); i++) {
            const MemSection& section = memsections[i];
            if (addr >= section.addr && addr < section.endaddr) {
                seek_offset = addr - section.addr;
                sectionChoice->setCurrentIndex(i);
                switch_section();
                break;
            }
        }
    }

    if (seek_offset == -1)
        return;

    QHexCursor* cursor = view->hexCursor();
    cursor->move(seek_offset);
    cursor->select(seek_offset+size);
}

void HexViewWindow::positionChanged()
{
    qint64 selectionStart = view->selectionStartOffset();
    qint64 selectionEnd = view->selectionEndOffset();
    qint64 selectionLength = selectionEnd - selectionStart;
    
    switch(selectionLength) {
        case 1: {
            QByteArray selectedArray = view->selectedBytes();
            int8_t signedValue = *reinterpret_cast<const int8_t*>(selectedArray.constData());
            uint8_t unsignedValue = *reinterpret_cast<const uint8_t*>(selectedArray.constData());
            selectionLabel->setText(QString("byte: %1, unsigned byte: %2").arg((int)signedValue).arg((unsigned int)unsignedValue));
            break;
        }
        case 2: {
            QByteArray selectedArray = view->selectedBytes();
            int16_t signedValue = *reinterpret_cast<const int16_t*>(selectedArray.constData());
            uint16_t unsignedValue = *reinterpret_cast<const uint16_t*>(selectedArray.constData());
            selectionLabel->setText(QString("short: %1, unsigned short: %2").arg(signedValue).arg(unsignedValue));
            break;
        }
        case 4: {
            QByteArray selectedArray = view->selectedBytes();
            int32_t signedValue = *reinterpret_cast<const int32_t*>(selectedArray.constData());
            uint32_t unsignedValue = *reinterpret_cast<const uint32_t*>(selectedArray.constData());
            float floatValue = *reinterpret_cast<const float*>(selectedArray.constData());
            selectionLabel->setText(QString("int: %1, unsigned int: %2, float: %3").arg(signedValue).arg(unsignedValue).arg(floatValue));
            break;
        }
        case 8: {
            QByteArray selectedArray = view->selectedBytes();
            int64_t signedValue = *reinterpret_cast<const int64_t*>(selectedArray.constData());
            uint64_t unsignedValue = *reinterpret_cast<const uint64_t*>(selectedArray.constData());
            double floatValue = *reinterpret_cast<const double*>(selectedArray.constData());
            selectionLabel->setText(QString("long: %1, unsigned long: %2, double: %3").arg(signedValue).arg(unsignedValue).arg(floatValue));
            break;
        }
    }
}
