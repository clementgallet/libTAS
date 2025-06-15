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

#ifndef LIBTAS_PATHPANE_H_INCLUDED
#define LIBTAS_PATHPANE_H_INCLUDED

#include <QtWidgets/QWidget>

class Context;
class QLineEdit;
class QPushButton;

class PathPane : public QWidget {
    Q_OBJECT
public:
    PathPane(Context *c);

    void update(int status);

    Context *context;

private:
    void initLayout();
    void initSignals();
    void initToolTips();

    void showEvent(QShowEvent *event) override;
    
    QLineEdit *dataPath;
    QLineEdit *moviePath;
    QLineEdit *statePath;

    QPushButton* browseDataPath;
    QPushButton* browseMoviePath;
    QPushButton* browseStatePath;

    QPushButton* openDataPath;
    QPushButton* openMoviePath;
    QPushButton* openStatePath;

public slots:
    void loadConfig();
    void saveConfig();

private slots:
    void slotBrowseDataPath();
    void slotBrowseMoviePath();
    void slotBrowseStatePath();
    void slotOpenDataPath();
    void slotOpenMoviePath();
    void slotOpenStatePath();

};

#endif
