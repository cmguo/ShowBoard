#ifndef MENUTOOL_H
#define MENUTOOL_H

#include "ShowBoard_global.h"

#include "controls/widgetcontrol.h"

class SHOWBOARD_EXPORT MenuTool : public WidgetControl
{
public:
    Q_INVOKABLE MenuTool(ResourceView *res);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    virtual QWidget * createWidget(ControlView * parent) override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void getToolButtons(QList<ToolButton *> & buttons, ToolButton * parent) override;

    virtual bool handleToolButton(QList<ToolButton *> const & buttons) override;
};

#endif // MENUTOOL_H
