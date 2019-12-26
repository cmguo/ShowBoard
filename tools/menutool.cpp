#include "menutool.h"
#include "views/toolbarwidget.h"

MenuTool::MenuTool(ResourceView *res)
    : WidgetControl(res, {PositionAtCenter}, {CanSelect, CanRotate, CanScale})
{
}

QWidget * MenuTool::createWidget(ResourceView *res)
{
    (void) res;
    ToolbarWidget * widget = new ToolbarWidget(false);
    return widget;
}

void MenuTool::attached()
{
    ToolbarWidget * widget = static_cast<ToolbarWidget*>(widget_);
    widget->attachProvider(this);
    loadFinished(true);
}

void MenuTool::getToolButtons(QList<ToolButton *> &buttons, const QList<ToolButton *> &parents)
{
    // skip control implements
    return ToolButtonProvider::getToolButtons(buttons, parents);
}

