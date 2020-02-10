#include "menutool.h"
#include "views/toolbarwidget.h"
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
        whiteCanvas()->scene()->views().first()->setFocus();
        widget_->setFocus();
    }
    if (event->type() == QEvent::FocusOut) {
        if (QApplication::focusWidget() == whiteCanvas()->scene()->views().first()
                && whiteCanvas()->scene()->focusItem() == item_) {
            widget_->setFocus();
        } else {
            whiteCanvas()->hideToolControl(this);
        }
    }
    return false;
}

QWidget * MenuTool::createWidget(ResourceView *res)
{
    (void) res;
    ToolbarWidget * widget = new ToolbarWidget(false);
    return widget;
}

void MenuTool::attached()
{
    item_->setFlag(QGraphicsItem::ItemIsFocusable);
    ToolbarWidget * widget = static_cast<ToolbarWidget*>(widget_);
    widget->installEventFilter(this);
    widget->attachProvider(this);
    loadFinished(true);
}

void MenuTool::getToolButtons(QList<ToolButton *> &buttons, const QList<ToolButton *> &parents)
{
    // skip control implements
    return ToolButtonProvider::getToolButtons(buttons, parents);
}

void MenuTool::handleToolButton(const QList<ToolButton *> &buttons)
{
    WidgetControl::handleToolButton(buttons);
    whiteCanvas()->hideToolControl(this);
}

