
#ifndef LIBTAS_TOOLTIPGROUPBOX_H_INCLUDED
#define LIBTAS_TOOLTIPGROUPBOX_H_INCLUDED

#include "ToolTipWidget.h"

#include <QGroupBox>

class ToolTipGroupBox : public ToolTipWidget<QGroupBox>
{
public:
    explicit ToolTipGroupBox(const QString& label);

private:
    QPoint getToolTipPosition() const override;
};

#endif
