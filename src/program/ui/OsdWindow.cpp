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

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

#include "OsdWindow.h"

OsdWindow::OsdWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("On-screen Display Options");

    /* Frame count */
    frameHorChoice = new QComboBox();
    frameHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    frameHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    frameHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    frameVertChoice = new QComboBox();
    frameVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    frameVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    frameVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    QGroupBox *frameGroupBox = new QGroupBox(tr("Frame count"));
    QHBoxLayout *frameLayout = new QHBoxLayout;
    frameLayout->addWidget(new QLabel(tr("Horizontal position:")));
    frameLayout->addWidget(frameHorChoice);
    frameLayout->addWidget(new QLabel(tr("Vertical position:")));
    frameLayout->addWidget(frameVertChoice);
    frameGroupBox->setLayout(frameLayout);

    /* Inputs */
    inputsHorChoice = new QComboBox();
    inputsHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    inputsHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    inputsHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    inputsVertChoice = new QComboBox();
    inputsVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    inputsVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    inputsVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    QGroupBox *inputsGroupBox = new QGroupBox(tr("Inputs"));
    QHBoxLayout *inputsLayout = new QHBoxLayout;
    inputsLayout->addWidget(new QLabel(tr("Horizontal position:")));
    inputsLayout->addWidget(inputsHorChoice);
    inputsLayout->addWidget(new QLabel(tr("Vertical position:")));
    inputsLayout->addWidget(inputsVertChoice);
    inputsGroupBox->setLayout(inputsLayout);

    /* Messages */
    messagesHorChoice = new QComboBox();
    messagesHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    messagesHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    messagesHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    messagesVertChoice = new QComboBox();
    messagesVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    messagesVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    messagesVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    QGroupBox *messagesGroupBox = new QGroupBox(tr("Messages"));
    QHBoxLayout *messagesLayout = new QHBoxLayout;
    messagesLayout->addWidget(new QLabel(tr("Horizontal position:")));
    messagesLayout->addWidget(messagesHorChoice);
    messagesLayout->addWidget(new QLabel(tr("Vertical position:")));
    messagesLayout->addWidget(messagesVertChoice);
    messagesGroupBox->setLayout(messagesLayout);

    /* Watches */
    watchesHorChoice = new QComboBox();
    watchesHorChoice->addItem("Left", SharedConfig::OSD_LEFT);
    watchesHorChoice->addItem("Center", SharedConfig::OSD_HCENTER);
    watchesHorChoice->addItem("Right", SharedConfig::OSD_RIGHT);

    watchesVertChoice = new QComboBox();
    watchesVertChoice->addItem("Top", SharedConfig::OSD_TOP);
    watchesVertChoice->addItem("Middle", SharedConfig::OSD_VCENTER);
    watchesVertChoice->addItem("Bottom", SharedConfig::OSD_BOTTOM);

    QGroupBox *watchesGroupBox = new QGroupBox(tr("Ram Watches"));
    QHBoxLayout *watchesLayout = new QHBoxLayout;
    watchesLayout->addWidget(new QLabel(tr("Horizontal position:")));
    watchesLayout->addWidget(watchesHorChoice);
    watchesLayout->addWidget(new QLabel(tr("Vertical position:")));
    watchesLayout->addWidget(watchesVertChoice);
    watchesGroupBox->setLayout(watchesLayout);

    /* Buttons */
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &OsdWindow::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &OsdWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(frameGroupBox);
    mainLayout->addWidget(inputsGroupBox);
    mainLayout->addWidget(messagesGroupBox);
    mainLayout->addWidget(watchesGroupBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    update_config();
}

void OsdWindow::update_config()
{
    int index;
    int hflags = SharedConfig::OSD_LEFT | SharedConfig::OSD_HCENTER | SharedConfig::OSD_RIGHT;
    int vflags = SharedConfig::OSD_TOP | SharedConfig::OSD_VCENTER | SharedConfig::OSD_BOTTOM;

    index = frameHorChoice->findData(context->config.sc.osd_frame_location & hflags);
    if (index >= 0)
        frameHorChoice->setCurrentIndex(index);

    index = frameVertChoice->findData(context->config.sc.osd_frame_location & vflags);
    if (index >= 0)
        frameVertChoice->setCurrentIndex(index);

    index = inputsHorChoice->findData(context->config.sc.osd_inputs_location & hflags);
    if (index >= 0)
        inputsHorChoice->setCurrentIndex(index);

    index = inputsVertChoice->findData(context->config.sc.osd_inputs_location & vflags);
    if (index >= 0)
        inputsVertChoice->setCurrentIndex(index);

    index = messagesHorChoice->findData(context->config.sc.osd_messages_location & hflags);
    if (index >= 0)
        messagesHorChoice->setCurrentIndex(index);

    index = messagesVertChoice->findData(context->config.sc.osd_messages_location & vflags);
    if (index >= 0)
        messagesVertChoice->setCurrentIndex(index);

    index = watchesHorChoice->findData(context->config.sc.osd_ramwatches_location & hflags);
    if (index >= 0)
        watchesHorChoice->setCurrentIndex(index);

    index = watchesVertChoice->findData(context->config.sc.osd_ramwatches_location & vflags);
    if (index >= 0)
        watchesVertChoice->setCurrentIndex(index);
}

void OsdWindow::slotOk()
{
    context->config.sc.osd_frame_location = frameHorChoice->currentData().toInt() | frameVertChoice->currentData().toInt();
    context->config.sc.osd_inputs_location = inputsHorChoice->currentData().toInt() | inputsVertChoice->currentData().toInt();
    context->config.sc.osd_messages_location = messagesHorChoice->currentData().toInt() | messagesVertChoice->currentData().toInt();
    context->config.sc.osd_ramwatches_location = watchesHorChoice->currentData().toInt() | watchesVertChoice->currentData().toInt();
    context->config.sc_modified = true;

    /* Close window */
    accept();
}
