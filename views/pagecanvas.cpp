#include "pagecanvas.h"
#include "core/resourcepage.h"
#include "core/resourcemanager.h"
#include "core/controlmanager.h"
#include "core/control.h"
#include "controls/whitecanvascontrol.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>

PageCanvas::PageCanvas(QGraphicsItem * parent)
    : CanvasItem(parent)
    , page_(nullptr)
    , subCanvas_(nullptr)
    , animTimer_(0)
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

void PageCanvas::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    qDebug() << "PageCanvas" << rect();
    painter->drawPixmap(rect(), snapshot_, QRectF());
}

void PageCanvas::timerEvent(QTimerEvent *event)
{
    (void) event;
    QPointF p = pos();
    if (qFuzzyIsNull(p.x()) && qFuzzyIsNull(p.y())) {
        stopAnimate();
        return;
    }
    if (!qFuzzyIsNull(p.x())) {
        QPointF d{rect().width() / 20, 0};
        p = p.x() < 0 ? p + d : p - d;
    }
    if (!qFuzzyIsNull(p.y())) {
        QPointF d{0, rect().height() / 20};
        p = p.y() < 0 ? p + d : p - d;
    }
    qDebug() << "PageCanvas" << p;
    setPos(p);
}

QPixmap PageCanvas::thumbnail(bool snapshot)
{
    QSizeF size = scene()->sceneRect().size();
    QSizeF size2 = snapshot ? size : size / size.height() * 100;
    QPixmap pixmap(size2.toSize());
    pixmap.fill(Qt::red);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    scene()->render(&painter, QRectF(QPointF(0, 0), size2), scene()->sceneRect());
    painter.end();
    if (snapshot) {
        snapshot_ = pixmap;
        return pixmap.scaled(pixmap.width(), 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        return pixmap;
    }
}

void PageCanvas::startAnimate(int dir)
{
    if (animTimer_)
        return;
    QRectF r = scene()->sceneRect();
    if (dir & 1)
        r.adjust(r.width(), 0, r.width(), 0);
    if (dir & 2)
        r.adjust(-r.width(), 0, -r.width(), 0);
    if (dir & 4)
        r.adjust(0, r.height(), 0, r.height());
    if (dir & 8)
        r.adjust(0, -r.height(), 0, -r.height());
    setRect(r);
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setPos(-r.center());
    animTimer_ = startTimer(20);
}

bool PageCanvas::inAnimate()
{
    return animTimer_ != 0;
}

void PageCanvas::stopAnimate()
{
    setFlag(QGraphicsItem::ItemHasNoContents, true);
    setRect(QRectF());
    snapshot_ = QPixmap();
    killTimer(animTimer_);
    animTimer_ = 0;
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
    QGraphicsItem * item = childItems().back();
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
