#include "pagecanvas.h"
#include "whitecanvas.h"
#include "core/resourcepage.h"
#include "core/resourcemanager.h"
#include "core/controlmanager.h"
#include "core/control.h"
#include "controls/whitecanvascontrol.h"

#include <QGraphicsScene>
#include <QPainter>

PageCanvas::PageCanvas(QGraphicsItem * parent)
    : CanvasItem(parent)
    , page_(nullptr)
    , subCanvas_(nullptr)
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
        QObject::connect(page_, &ResourcePage::currentSubPageChanged,
                         this, &PageCanvas::subPageChanged);
    }
    subPageChanged(page_ ? page_->currentSubPage() : nullptr);
}

void PageCanvas::relayout()
{
    for (QGraphicsItem * item : childItems()) {
        Control * ct = Control::fromItem(item);
        ct->relayout();
    }
}

QPixmap PageCanvas::thumbnail(QPixmap* snapshot)
{
    QSizeF size = scene()->sceneRect().size();
    QSizeF size2 = snapshot ? size : size / size.height() * WhiteCanvas::THUMBNAIL_HEIGHT;
    QPixmap pixmap(size2.toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    for (QGraphicsItem * sibling : parentItem()->childItems()) {
        if (sibling != this && !hasSubCanvas(sibling))
            sibling->hide();
    }
    QBrush br = scene()->backgroundBrush();
    scene()->setBackgroundBrush(Qt::transparent);
    scene()->render(&painter, QRectF(QPointF(0, 0), size2), scene()->sceneRect());
    scene()->setBackgroundBrush(br);
    for (QGraphicsItem * sibling : parentItem()->childItems()) {
        if (sibling != this && !hasSubCanvas(sibling))
            sibling->show();
    }
    painter.end();
    if (snapshot) {
        *snapshot = pixmap;
        return pixmap.scaled(pixmap.width(), WhiteCanvas::THUMBNAIL_HEIGHT,
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        return pixmap;
    }
}

Control * PageCanvas::findControl(ResourceView * res)
{
    int index = page_->resources().indexOf(res);
    if (index < 0) {
        if (subCanvas_)
            return subCanvas_->findControl(res);
        return nullptr;
    }
    QGraphicsItem * item = childItems()[index];
    return Control::fromItem(item);
}

Control * PageCanvas::findControl(QUrl const & url)
{
    return findControl(page_->findResource(url));
}

Control * PageCanvas::topControl()
{
    if (subCanvas_)
        return subCanvas_->topControl();
    QGraphicsItem * item = childItems().isEmpty() ? nullptr : childItems().back();
    return item ? Control::fromItem(item) : nullptr;
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

void PageCanvas::subPageChanged(ResourcePage *page)
{
    if (page) {
        if (subCanvas_ == nullptr) {
            subCanvas_ = new PageCanvas(parentItem());
            QList<QGraphicsItem*> siblings = parentItem()->childItems();
            subCanvas_->stackBefore(siblings[siblings.indexOf(this) + 1]);
        }
        subCanvas_->switchPage(page);
    } else {
        if (subCanvas_) {
            subCanvas_->switchPage(page);
            delete subCanvas_;
            subCanvas_ = nullptr;
        }
    }
}

bool PageCanvas::hasSubCanvas(QGraphicsItem *canvas)
{
    return subCanvas_ == canvas
            || (subCanvas_ && subCanvas_->hasSubCanvas(canvas));
}

void PageCanvas::insertResource(int layer)
{
    ResourceView *res = page_->resources()[layer];
    Control * ct = control_manager_->createControl(res);
    if (ct == nullptr)
        return;
    ct->attachTo(this, layer < childItems().size() ? childItems()[layer] : nullptr);
}

void PageCanvas::removeResource(int layer)
{
    QGraphicsItem * item = childItems()[layer++];
    Control * ct = Control::fromItem(item);
    ct->detachFrom(this, layer < childItems().size() ? childItems()[layer] : nullptr);
}
