#include "menutool.h"
#include "widget/toolbarwidget.h"
#include "views/whitecanvas.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QApplication>
#include <QDebug>

MenuTool::MenuTool(ResourceView *res)
    : WidgetControl(res, {}, {CanSelect, CanRotate, CanScale})
{
}

bool MenuTool::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::Show) {
#ifdef SHOWBOARD_QUICK
#else
        whiteCanvas()->scene()->views().first()->setFocus();
#endif
        widget_->show(); // sometimes not sync
        widget_->setFocus();
    }
    if (event->type() == QEvent::FocusOut) {
#ifdef SHOWBOARD_QUICK
        if (false) {
#else
        if (QApplication::focusWidget() == whiteCanvas()->scene()->views().first()
                && whiteCanvas()->scene()->focusItem() == item_) {
#endif
            widget_->setFocus();
        } else {
            widget_->hide(); // sometimes not sync
            whiteCanvas()->hideToolControl(this);
        }
    }
    return false;
}

QWidget *MenuTool::createWidget(ControlView *parent)
{
    (void) parent;
    ToolbarWidget * widget = new ToolbarWidget(false);
    return widget;
}

void MenuTool::attaching()
{
    ToolbarWidget * widget = static_cast<ToolbarWidget*>(widget_);
    widget->attachProvider(this);
}

void MenuTool::attached()
{
#ifdef SHOWBOARD_QUICK
#else
    item_->setFlag(QGraphicsItem::ItemIsFocusable);
#endif
    ToolbarWidget * widget = static_cast<ToolbarWidget*>(widget_);
    widget->installEventFilter(this);
    loadFinished(true);
}

void MenuTool::getToolButtons(QList<ToolButton *> &buttons, ToolButton * parent)
{
    // skip control implements
    return ToolButtonProvider::getToolButtons(buttons, parent);
}

bool MenuTool::handleToolButton(QList<ToolButton *> const & buttons)
{
    whiteCanvas()->hideToolControl(this);
    return ToolButtonProvider::handleToolButton(buttons);
}

