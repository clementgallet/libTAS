/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <QVBoxLayout>

#include "AnnotationsWindow.h"
#include "MainWindow.h"

AnnotationsWindow::AnnotationsWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Annotations");

    annotationText = new QPlainTextEdit();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &AnnotationsWindow::slotSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AnnotationsWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(annotationText);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    /* Get movie object */
    movie = nullptr;
    MainWindow *mw = qobject_cast<MainWindow*>(parent);
    if (mw) {
        movie = &mw->gameLoop->movie;
    }

    update();
}

QSize AnnotationsWindow::sizeHint() const
{
    return QSize(800, 400);
}

void AnnotationsWindow::update()
{
    const QString qann = QString(movie->annotations.c_str());
    annotationText->setPlainText(qann);
}

void AnnotationsWindow::clear()
{
    annotationText->setPlainText(QString());
}

void AnnotationsWindow::slotSave()
{
    const QString qann = annotationText->toPlainText();
    movie->annotations = qann.toStdString();
    movie->saveMovie();

    /* Close window */
    accept();
}
