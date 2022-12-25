
#include "ToolTipGroupBox.h"

#include <QStyle>
#include <QStyleOption>

ToolTipGroupBox::ToolTipGroupBox(const QString& label) : ToolTipWidget(label)
{
  setTitle(label);
}

QPoint ToolTipGroupBox::getToolTipPosition() const
{
    int title_height = 14;
    if (style()) {
        QStyleOptionGroupBox opt;
        initStyleOption(&opt);
        QRect contentsRect = style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, this);
        return pos() + QPoint(width() / 2, contentsRect.height());
    }
    return pos() + QPoint(width() / 2, title_height);
}
