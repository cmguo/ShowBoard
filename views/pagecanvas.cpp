#include "pagecanvas.h"
#include "core/resourcepage.h"
#include "core/resourcemanager.h"
#include "core/controlmanager.h"
#include "core/control.h"
#include "controls/whitecanvascontrol.h"

#include <QGraphicsScene>

PageCanvas::PageCanvas(QGraphicsItem * parent)
    : CanvasItem(parent)
    , page_(nullptr)
{
    resource_manager_ = ResourceManager::instance();
    control_manager_ = ControlManager::instance();
}

void PageCanvas::switchPage(ResourcePage * page)
{
    if (page_ != nullptr) {
        page_->disconnect(this);
        for (int i = page_->resources().size() - 1; i >= 0; --i) {
            removeResource(i);
        }
        if (page_->canvasView()) {
            delete Control::fromItem(parentItem());
        }
        if (!page_->parent())
            page_->deleteLater();
    }
    page_ = page;
    if (page_ != nullptr) {
        if (page_->canvasView()) {
            new WhiteCanvasControl(page_->canvasView(), parentItem());
        }
        for (int i = 0; i < page_->resources().size(); ++i)
            insertResource(i);
        QObject::connect(page_, &ResourcePage::rowsInserted,
                         this, &PageCanvas::resourceInserted);
        QObject::connect(page_, &ResourcePage::rowsRemoved,
                         this, &PageCanvas::resourceRemoved);
        QObject::connect(page_, &ResourcePage::rowsMoved,
                         this, &PageCanvas::resourceMoved);
    }
}

void PageCanvas::setGeometry(QRectF const & rect)
{
    setRect(rect);
    for (QGraphicsItem * item : childItems()) {
        Control * ct = Control::fromItem(item);
        ct->relayout();
    }
}

Control * PageCanvas::findControl(ResourceView * res)
{
    int index = page_->resources().indexOf(res);
    if (index < 0)
        return nullptr;
    QGraphicsItem * item = childItems()[index];
    return Control::fromItem(item);
}

Control * PageCanvas::findControl(QUrl const & url)
{
    return findControl(page_->findResource(url));
}

void PageCanvas::resourceInserted(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    (void) last;
    while (first <= last) {
        insertResource(first);
        ++first;
    }
}

void PageCanvas::resourceRemoved(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    while (first <= last) {
        removeResource(first);
        --last;
    }
}

void PageCanvas::resourceMoved(QModelIndex const &parent, int start, int end,
                                QModelIndex const &destination, int row)
{
    (void) parent;
    (void) destination;
    if (row < start) {
        QGraphicsItem * dest = childItems()[row];
        while (start <= end) {
            QGraphicsItem * item = childItems()[start];
            item->stackBefore(dest);
            ++start;
        }
        dest->update();
    } else if (row > end) {
        QGraphicsItem * first = childItems()[start];
        while (++end < row) {
            QGraphicsItem * item = childItems()[end];
            item->stackBefore(first);
        }
        first->update();
    }
}

void PageCanvas::insertResource(int layer)
{
    ResourceView *res = page_->resources()[layer];
    Control * ct = control_manager_->createControl(res);
    if (ct == nullptr)
        return;
    ct->attachTo(this);
    if (layer < childItems().size() - 1) {
        ct->item()->stackBefore(childItems()[layer]);
    }
}

void PageCanvas::removeResource(int layer)
{
    QGraphicsItem * item = childItems()[layer];
    Control * ct = Control::fromItem(item);
    ct->detachFrom(this);
}
