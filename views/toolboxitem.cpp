#include "toolboxitem.h"
#include "core/resourcepage.h"
#include "core/control.h"

#include <QUrl>

ToolBoxItem::ToolBoxItem(QGraphicsItem * parent)
    : ResourcePageItem(parent)
    , shown_(nullptr)
{
    switchPage(new ResourcePage);
}

void ToolBoxItem::showToolControl(const QString &type)
{
    showItem(getToolControl(type)->item());
}

void ToolBoxItem::hideToolControl(const QString &type)
{
    QUrl url(type + ":");
    Control * ct = findControl(url);
    if (ct)
        hideItem(ct->item());
}

Control * ToolBoxItem::getToolControl(const QString &type)
{
    QUrl url(type + ":");
    ResourceView * res = page_->findResource(url);
    if (!res) {
        res = page_->addResource(url);
    }
    return findControl(res);
}

void ToolBoxItem::showItem(QGraphicsItem *item)
{
    if (item == shown_)
        return;
    if (shown_)
        shown_->hide();
    shown_ = item;
    if (shown_)
        shown_->show();
}

void ToolBoxItem::hideItem(QGraphicsItem *item)
{
    if (item == shown_) {
        shown_->hide();
        shown_ = nullptr;
    }
}

QVariant ToolBoxItem::itemChange(QGraphicsItem::GraphicsItemChange change,
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

