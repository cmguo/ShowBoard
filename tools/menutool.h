#ifndef MENUTOOL_H
#define MENUTOOL_H

#include "ShowBoard_global.h"

#include "core/widgetcontrol.h"

class SHOWBOARD_EXPORT MenuTool : public WidgetControl
{
public:
    Q_INVOKABLE MenuTool(ResourceView *res);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    virtual QWidget * createWidget(ResourceView *res) override;

    virtual void attached() override;

    virtual void getToolButtons(QList<ToolButton *> & buttons,
                                QList<ToolButton *> const & parents = {}) override;

    virtual void handleToolButton(QList<ToolButton *> const & buttons) override;
};

#endif // MENUTOOL_H
