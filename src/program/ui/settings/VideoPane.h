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

#ifndef LIBTAS_VIDEOPANE_H_INCLUDED
#define LIBTAS_VIDEOPANE_H_INCLUDED

#include <QtWidgets/QWidget>

class Context;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QSpinBox;
class ToolTipCheckBox;
class QGroupBox;

class VideoPane : public QWidget {
    Q_OBJECT
public:
    VideoPane(Context *c);

    void update(int status);

    Context *context;

private:
    void initLayout();
    void initSignals();
    void initToolTips();

    void showEvent(QShowEvent *event) override;
    
    QGroupBox* screenBox;
    QGroupBox* renderingBox;
    
    QRadioButton* screenNativeRadio;
    QRadioButton* screenCommonRadio;
    QRadioButton* screenCustomRadio;
    QComboBox* screenCommonChoice;
    QSpinBox* widthField;
    QSpinBox* heightField;
    
    QCheckBox* osdFrameBox;
    QCheckBox* osdInputsBox;
    QCheckBox* osdMessagesBox;
    QCheckBox* osdRamBox;
    QCheckBox* osdLuaBox;
    QCheckBox* osdCrosshairBox;
    QCheckBox* osdEncodeBox;

    QComboBox *frameHorChoice;
    QComboBox *frameVertChoice;
    QComboBox *inputsHorChoice;
    QComboBox *inputsVertChoice;
    QComboBox *messagesHorChoice;
    QComboBox *messagesVertChoice;
    QComboBox *watchesHorChoice;
    QComboBox *watchesVertChoice;

    ToolTipCheckBox* rendSoftBox;
    ToolTipCheckBox* rendPerfBox;

public slots:
    void loadConfig();
    void saveConfig();
};

#endif
