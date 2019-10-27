#include "whitecanvas.h"

#include "itemselector.h"
#include "resourcemanager.h"
#include "controlmanager.h"
#include "control.h"
#include "resourceview.h"
#include "resourcepage.h"
#include "toolbarwidget.h"

#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

WhiteCanvas::WhiteCanvas(QObject * parent)
    : QObject(parent)
    , page_(nullptr)
{
    resource_manager_ = ResourceManager::instance();
    control_manager_ = ControlManager::instance();
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFlags(ItemIsMovable);
    setPen(QPen(Qt::NoPen));
    setBrush(QBrush(Qt::green));
    //addToGroup(new ItemSelector());
    canvas_ = new QGraphicsRectItem(this);
    canvas_->setPen(QPen(Qt::NoPen));
    canvas_->setRect(rect());
    selector_ = new ItemSelector(canvas_, this);
    selector_->setRect(rect());
    void (ToolbarWidget::*sig)(ToolButton *) = &ToolbarWidget::buttonClicked;
    QObject::connect(selector_->toolBar(), sig, this, &WhiteCanvas::toolButtonClicked);
    switchPage(new ResourcePage);
}

WhiteCanvas::~WhiteCanvas()
{
    switchPage(nullptr);
}

QVariant WhiteCanvas::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemParentHasChanged || change == ItemSceneChange) {
        QRectF rect = change == ItemParentHasChanged
                ? value.value<QGraphicsItem *>()->boundingRect()
                : value.value<QGraphicsScene *>()->sceneRect();
        rect.moveCenter({0, 0});
        setGeometry(rect);
    }
    return value;
}

void WhiteCanvas::switchPage(ResourcePage * page)
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
                         this, &WhiteCanvas::resourceInserted);
        QObject::connect(page_, &ResourcePage::rowsRemoved,
                         this, &WhiteCanvas::resourceRemoved);
        QObject::connect(page_, &ResourcePage::rowsMoved,
                         this, &WhiteCanvas::resourceMoved);
    }
}

void WhiteCanvas::addResource(QUrl const & url)
{
    ResourceView * rv = resource_manager_->createResource(url);
    addResource(rv);
}

void WhiteCanvas::addResource(ResourceView * res)
{
    selector_->select(nullptr);
    page_->addResource(res);
}

Control * WhiteCanvas::findControl(ResourceView * res)
{
    int index = page_->resources().indexOf(res);
    if (index < 0)
        return nullptr;
    QGraphicsItem * item = canvas_->childItems()[index];
    return Control::fromItem(item);
}

Control * WhiteCanvas::findControl(QUrl const & url)
{
    return findControl(page_->findResource(url));
}

Control * WhiteCanvas::topControl()
{
    QGraphicsItem * item = canvas_->childItems().back();
    return item ? Control::fromItem(item) : nullptr;
}

void WhiteCanvas::enableSelector(bool enable)
{
    selector_->setForce(enable);
}

void WhiteCanvas::setGeometry(QRectF const & rect)
{
    setRect(rect);
    canvas_->setRect(rect);
    selector_->setRect(rect);
    for (QGraphicsItem * item : canvas_->childItems()) {
        Control * ct = Control::fromItem(item);
        ct->relayout();
    }
}

void WhiteCanvas::toolButtonClicked(ToolButton * button)
{
    Control * ct = Control::fromItem(selector_->selected());
    if (button == &Control::btnCopy) {
        selector_->select(nullptr);
        page_->copyResource(ct->resource());
    } else if (button == &Control::btnDelete) {
        selector_->select(nullptr);
        page_->removeResource(ct->resource());
    } else {
        ct->handleToolButton(button);
    }
}

void WhiteCanvas::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    bool force = selector_->force_;
    selector_->force_ = true;
    selector_->mousePressEvent(event);
    selector_->force_ = force;
}

void WhiteCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    selector_->mouseMoveEvent(event);
}

void WhiteCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    selector_->mouseReleaseEvent(event);
}

void WhiteCanvas::resourceInserted(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    (void) last;
    while (first <= last) {
        insertResource(first);
        ++first;
    }
}

void WhiteCanvas::resourceRemoved(QModelIndex const &parent, int first, int last)
{
    (void) parent;
    while (first <= last) {
        removeResource(first);
        --last;
    }
}

void WhiteCanvas::resourceMoved(QModelIndex const &parent, int start, int end,
                                QModelIndex const &destination, int row)
{
    (void) parent;
    (void) destination;
    QGraphicsItem * dest = canvas_->childItems()[row];
    while (start <= end) {
        QGraphicsItem * item = canvas_->childItems()[start];
        item->stackBefore(dest);
        ++start;
    }
}

void WhiteCanvas::insertResource(int layer)
{
    ResourceView *res = page_->resources()[layer];
    Control * ct = control_manager_->createControl(res);
    ct->attach();
    ct->item()->setParentItem(canvas_); // TODO: layer
    if (layer < canvas_->childItems().size() - 1) {
        ct->item()->stackBefore(canvas_->childItems()[layer]);
    }
    ct->relayout();
}

void WhiteCanvas::removeResource(int layer)
{
    QGraphicsItem * item = canvas_->childItems()[layer];
    Control * ct = Control::fromItem(item);
    scene()->removeItem(item);
    ct->detach();
}
