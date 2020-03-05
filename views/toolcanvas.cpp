#include "toolcanvas.h"
#include "core/resourcepage.h"
#include "core/resourcepackage.h"
#include "core/control.h"

#include <QUrl>

ToolCanvas::ToolCanvas(QGraphicsItem * parent)
    : PageCanvas(parent)
    , shown_(nullptr)
{
    switchPage(ResourcePackage::toolPage());
}

void ToolCanvas::showToolControl(Control * control)
{
    if (control)
        showItem(control->item());
}

void ToolCanvas::hideToolControl(Control * control)
{
    if (control)
        hideItem(control->item());
}

Control * ToolCanvas::getToolControl(const QString &type)
{
    QUrl url(type.contains(':') ? type : type + ":");
    ResourceView * res = page_->findResource(url);
    if (!res) {
        res = page_->addResource(url);
    }
    return findControl(res);
}

void ToolCanvas::showItem(QGraphicsItem *item)
{
    if (item == shown_)
        return;
    if (shown_)
        shown_->hide();
    shown_ = item;
    if (shown_)
        shown_->show();
}

void ToolCanvas::hideItem(QGraphicsItem *item)
{
    if (item == shown_) {
        shown_->hide();
        shown_ = nullptr;
    }
}

QVariant ToolCanvas::itemChange(QGraphicsItem::GraphicsItemChange change,
                                         const QVariant &value)
{
    if (change == QGraphicsItem::ItemChildAddedChange) {
        value.value<QGraphicsItem *>()->hide();
    } else if (change == QGraphicsItem::ItemChildRemovedChange) {
        if (value.value<QGraphicsItem *>() == shown_)
            shown_ = nullptr;
    }
    return value;
}

