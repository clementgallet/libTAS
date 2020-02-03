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

#ifndef LIBTAS_ANNOTATIONSWINDOW_H_INCLUDED
#define LIBTAS_ANNOTATIONSWINDOW_H_INCLUDED

#include <QDialog>
#include <QPlainTextEdit>

#include "../Context.h"
#include "../MovieFile.h"

class AnnotationsWindow : public QDialog {
    Q_OBJECT

public:
    AnnotationsWindow(Context *c, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = 0);

    QSize sizeHint() const override;

    /* Update text when the movie has changed */
    void update();

    /* Clear text editor */
    void clear();

private:
    Context *context;
    MovieFile *movie;

    QPlainTextEdit *annotationText;

private slots:
    void slotSave();
};

#endif
