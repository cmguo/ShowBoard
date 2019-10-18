#include "whitecanvas.h"

#include "itemselector.h"
#include "resourcemanager.h"
#include "controlmanager.h"
#include "control.h"
#include "resourceview.h"
#include "resourcepage.h"

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
    QPen pen(Qt::transparent);
    setPen(pen);
    setBrush(QBrush(Qt::green));
    //addToGroup(new ItemSelector());
    canvas_ = new QGraphicsRectItem(this);
    canvas_->setPen(pen);
    canvas_->setRect(rect());
    selector_ = new ItemSelector(canvas_, this);
    selector_->setPen(pen);
    selector_->setRect(rect());
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
        setRect(rect);
        canvas_->setRect(rect);
        selector_->setRect(rect);
    }
    return value;
}

void WhiteCanvas::switchPage(ResourcePage * page)
{
    if (page_ != nullptr) {
        QList<QGraphicsItem *> items = canvas_->childItems();
        for (QGraphicsItem * item : items) {
            removeResource(item, true);
        }
        if (!page_->parent())
            page_->deleteLater();
    }
    page_ = page;
    if (page_ != nullptr) {
        for (ResourceView * res : page->resources())
            addResource(res, true);
    }
}

void WhiteCanvas::addResource(QUrl const & url)
{
    ResourceView * rv = resource_manager_->createResource(url);
    addResource(rv);
}

void WhiteCanvas::addResource(ResourceView * res, bool fromSwitch)
{
    selector_->select(nullptr);
    Control * ct = control_manager_->createControl(res);
    ct->attach();
    ct->item()->setParentItem(canvas_);
    if (!fromSwitch)
        page_->addResource(res);
}

void WhiteCanvas::copyResource(QGraphicsItem *item)
{
    ResourceView * rv = Control::fromItem(item)->resource();
    rv = rv->clone();
    addResource(rv);
}

void WhiteCanvas::removeResource(QGraphicsItem *item, bool fromSwitch)
{
    selector_->select(nullptr);
    Control * ct = Control::fromItem(item);
    scene()->removeItem(item);
    ct->detach();
    if (!fromSwitch)
        page_->removeResource(ct->resource());
}

void WhiteCanvas::enableSelector(bool enable)
{
    selector_->setForce(enable);
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
