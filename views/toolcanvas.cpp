#include "toolcanvas.h"
#include "core/resourcepage.h"
#include "core/resourcepackage.h"
#include "core/control.h"

#include <QUrl>

ToolCanvas::ToolCanvas(CanvasItem * parent)
    : PageCanvas(parent)
    , shown_(nullptr)
{
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

Control * ToolCanvas::getToolControl(const QString &typeOrUrl)
{
    ResourceView * res = typeOrUrl.contains(':')
            ? page_->findResource(QUrl(typeOrUrl))
            : page_->findResource(typeOrUrl.toUtf8());
    if (!res) {
        QUrl url(typeOrUrl.contains(':') ? typeOrUrl : typeOrUrl + ":");
        res = page_->addResource(url);
    }
    return findControl(res);
}

void ToolCanvas::showItem(ControlView *item)
{
    if (item == shown_)
        return;
    if (shown_)
        shown_->setVisible(false);
    shown_ = item;
    if (shown_)
        shown_->setVisible(true);
}

void ToolCanvas::hideItem(ControlView *item)
{
    if (item == shown_) {
        shown_->setVisible(false);
        shown_ = nullptr;
    }
}

#ifdef SHOWBOARD_QUICK
void ToolCanvas::itemChange(ItemChange change, const ItemChangeData &value)
#else
QVariant ToolCanvas::itemChange(GraphicsItemChange change, const QVariant &value)
#endif
{
    if (change == ItemChildAddedChange) {
#ifdef SHOWBOARD_QUICK
        value.item->setVisible(false);
#else
        value.value<QGraphicsItem *>()->hide();
#endif
    } else if (change == ItemChildRemovedChange) {
#ifdef SHOWBOARD_QUICK
        if (value.item == shown_)
#else
        if (value.value<QGraphicsItem *>() == shown_)
#endif
            shown_ = nullptr;
    }
#ifndef SHOWBOARD_QUICK
    return value;
#endif
}

