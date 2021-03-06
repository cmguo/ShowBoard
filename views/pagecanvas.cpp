#include "pagecanvas.h"
#include "whitecanvas.h"
#include "itemselector.h"
#include "core/resourcepage.h"
#include "core/resourcemanager.h"
#include "core/controlmanager.h"
#include "core/control.h"
#include "controls/whitecanvascontrol.h"

#include <QGraphicsScene>
#include <QPainter>

PageCanvas::PageCanvas(CanvasItem * parent)
    : CanvasItem(parent)
    , page_(nullptr)
    , subCanvas_(nullptr)
{
    resource_manager_ = ResourceManager::instance();
    control_manager_ = ControlManager::instance();
#ifdef SHOWBOARD_QUICK
#endif
}

bool PageCanvas::isPageCanvas(ControlView *view)
{
#ifdef SHOWBOARD_QUICK
    return qobject_cast<PageCanvas*>(view);
#else
    return view->type() == Type;
#endif
}

void PageCanvas::switchPage(ResourcePage * page)
{
    if (page_ != nullptr) {
        page_->disconnect(this);
        subPageChanged(nullptr);
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
        subPageChanged(page_->currentSubPage());
    }
}

void PageCanvas::relayout()
{
    for (ControlView * item : childItems()) {
        Control * ct = Control::fromItem(item);
        ct->relayout();
    }
    if (subCanvas_)
        subCanvas_->relayout();
}

ResourcePage *PageCanvas::subPage() const
{
    return subCanvas_ ? subCanvas_->subPage() : page_;
}

QPixmap PageCanvas::thumbnail(QPixmap* snapshot) const
{
    QSizeF size = itemSceneRect(this).size();
    QSizeF size2 = snapshot ? size : size / size.height() * WhiteCanvas::THUMBNAIL_HEIGHT;
    QPixmap pixmap(size2.toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    for (ControlView * sibling : parentItem()->childItems()) {
        if (sibling != this && !hasSubCanvas(static_cast<CanvasItem*>(sibling)))
            sibling->setVisible(false);
    }
#ifdef SHOWBOARD_QUICK
#else
    QBrush br = scene()->backgroundBrush();
    scene()->setBackgroundBrush(Qt::transparent);
    scene()->render(&painter, QRectF(QPointF(0, 0), size2), scene()->sceneRect());
    scene()->setBackgroundBrush(br);
#endif
    for (ControlView * sibling : parentItem()->childItems()) {
        if (sibling != this && !hasSubCanvas(static_cast<CanvasItem*>(sibling)))
            sibling->setVisible(true);
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

Control * PageCanvas::findControl(ResourceView * res) const
{
    int index = page_->resources().indexOf(res);
    if (index < 0) {
        if (subCanvas_)
            return subCanvas_->findControl(res);
        return nullptr;
    }
    if (index >= childItems().size())
        return nullptr;
    ControlView * item = childItems()[index];
    return Control::fromItem(item);
}

Control * PageCanvas::findControl(QUrl const & url) const
{
    return findControl(page_->findResource(url));
}

Control * PageCanvas::topControl() const
{
    if (subCanvas_)
        return subCanvas_->topControl();
    ControlView * item = childItems().isEmpty() ? nullptr : childItems().back();
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
        ControlView * dest = childItems()[row];
        while (start <= end) {
            ControlView * item = childItems()[start];
            item->stackBefore(dest);
            ++start;
        }
        dest->update();
    } else if (row > end) {
        ControlView * first = childItems()[start];
        while (++end < row) {
            ControlView * item = childItems()[end];
            item->stackBefore(first);
        }
        first->update();
    }
}

#include <QDebug>

void PageCanvas::subPageChanged(ResourcePage *page)
{
    qDebug() << "PageCanvas::subPageChanged" << page;
    if (page) {
        if (subCanvas_ == nullptr) {
            subCanvas_ = new PageCanvas(static_cast<CanvasItem*>(parentItem()));
            QList<ControlView*> siblings = parentItem()->childItems();
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
    if (sender()) {
        PageCanvas * canvas = this;
        if (canvas->page_->isSubPage()) {
            canvas = static_cast<PageCanvas*>(parentItem());
        }
        static_cast<WhiteCanvas*>(parentItem())->currentPageChanged(subPage());
    }
}

bool PageCanvas::hasSubCanvas(CanvasItem *canvas) const
{
    return canvas == this
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
    ControlView * item = childItems()[layer++];
    Control * ct = Control::fromItem(item);
    ct->detachFrom(this, layer < childItems().size() ? childItems()[layer] : nullptr);
}
