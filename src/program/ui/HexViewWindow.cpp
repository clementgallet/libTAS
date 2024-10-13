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

#include <QtWidgets/QVBoxLayout>
#include <iostream>

HexViewWindow::HexViewWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Hex View");
    iodevice = new IOProcessDevice(this);
    QHexDocument* doc = QHexDocument::fromDevice<QDeviceBuffer>(iodevice);

    view = new QHexView(this);
    view->setDocument(doc);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(view);
    setLayout(mainLayout);
}

void HexViewWindow::update()
{
    iodevice->setSection(BaseAddresses::getExecutableSection());
    view->setBaseAddress(BaseAddresses::getExecutableSection()->addr);
}
