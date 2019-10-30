#include "resourcepageitem.h"
#include "core/resourcepage.h"
#include "core/resourcemanager.h"
#include "core/controlmanager.h"
#include "core/control.h"

#include <QGraphicsScene>

ResourcePageItem::ResourcePageItem(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , page_(nullptr)
{
    resource_manager_ = ResourceManager::instance();
    control_manager_ = ControlManager::instance();
    setPen(QPen(Qt::NoPen));
}

void ResourcePageItem::switchPage(ResourcePage * page)
{
    if (page_ != nullptr) {
        page_->disconnect(this);
        for (int i = page_->resources().size() - 1; i >= 0; --i) {
            removeResource(i);
        }
        if (!page_->parent())
            page_->deleteLater();
    }
    page_ = page;
    if (page_ != nullptr) {
        for (int i = 0; i < page_->resources().size(); ++i)
            insertResource(i);
        QObject::connect(page_, &ResourcePage::rowsInserted,
                         this, &ResourcePageItem::resourceInserted);
        QObject::connect(page_, &ResourcePage::rowsRemoved,
                         this, &ResourcePageItem::resourceRemoved);
        QObject::connect(page_, &ResourcePage::rowsMoved,
                         this, &ResourcePageItem::resourceMoved);
    }
}

void ResourcePageItem::setGeometry(QRectF const & rect)
{
    setRect(rect);
    for (QGraphicsItem * item : childItems()) {
        Control * ct = Control::fromItem(item);
        ct->relayout();
    }
}

Control * ResourcePageItem::findControl(ResourceView * res)
{
    int index = page_->resources().indexOf(res);
    if (index < 0)
        return nullptr;
    QGraphicsItem * item = childItems()[index];
    return Control::fromItem(item);
}

Control * ResourcePageItem::findControl(QUrl const & url)
{
    return findControl(page_->findResource(url));
}

void ResourcePageItem::resourceInserted(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    (void) last;
    while (first <= last) {
        insertResource(first);
        ++first;
    }
}

void ResourcePageItem::resourceRemoved(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    while (first <= last) {
        removeResource(first);
        --last;
    }
}

void ResourcePageItem::resourceMoved(QModelIndex const &parent, int start, int end,
                                QModelIndex const &destination, int row)
{
    (void) parent;
    (void) destination;
    QGraphicsItem * dest = childItems()[row];
    while (start <= end) {
        QGraphicsItem * item = childItems()[start];
        item->stackBefore(dest);
        ++start;
    }
}

void ResourcePageItem::insertResource(int layer)
{
    ResourceView *res = page_->resources()[layer];
    Control * ct = control_manager_->createControl(res);
    ct->attaching();
    ct->item()->setParentItem(this);
    ct->attached();
    if (layer < childItems().size() - 1) {
        ct->item()->stackBefore(childItems()[layer]);
    }
    ct->relayout();
}

void ResourcePageItem::removeResource(int layer)
{
    QGraphicsItem * item = childItems()[layer];
    Control * ct = Control::fromItem(item);
    ct->detaching();
    scene()->removeItem(item);
    ct->detached();
}
