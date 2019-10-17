#include "whitecanvas.h"

#include "itemselector.h"
#include "resourcemanager.h"
#include "controlmanager.h"
#include "control.h"
#include "resourceview.h"
#include "whitepage.h"

#include <qcomponentcontainer.h>

#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

QComponentContainer & WhiteCanvas::containter()
{
    static QComponentContainer c;
    return c;
}

WhiteCanvas::WhiteCanvas()
{
    resource_manager_ = containter().get_export_value<ResourceManager>();
    control_manager_ = containter().get_export_value<ControlManager>();
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFlags(ItemIsMovable);
    setPen(QPen());
    setBrush(QBrush(Qt::green));
    setRect(-512, -288, 1024, 576);
    //addToGroup(new ItemSelector());
    canvas_ = new QGraphicsRectItem(this);
    canvas_->setRect(rect());
    selector_ = new ItemSelector(canvas_, this);
    switchPage(new WhitePage);
}

WhiteCanvas::~WhiteCanvas()
{
    switchPage(nullptr);
}

void WhiteCanvas::switchPage(WhitePage * page)
{
    if (page_ != nullptr) {
        QList<QGraphicsItem *> items = canvas_->childItems();
        for (QGraphicsItem * item : items) {
            Control * ct = Control::fromItem(item);
            ct->detach();
        }
    }
    page_ = page;
    if (page_ != nullptr) {
        for (ResourceView * res : page->resources())
            addResource(res, true);
    }
}

void WhiteCanvas::addResource(QUrl const url)
{
    ResourceView * rv = resource_manager_->CreateResource(url);
    addResource(rv);
}

void WhiteCanvas::addResource(ResourceView * res, bool fromSwitch)
{
    selector_->select(nullptr);
    Control * ct = control_manager_->CreateControl(res);
    ct->attach(canvas_);
    if (!fromSwitch)
        page_->addResource(res);
}

void WhiteCanvas::copyResource(QGraphicsItem *item)
{
    ResourceView * rv = Control::fromItem(item)->resource();
    rv = rv->clone();
    addResource(rv);
}

void WhiteCanvas::removeResource(QGraphicsItem *item)
{
    selector_->select(nullptr);
    Control * ct = Control::fromItem(item);
    ct->detach();
    page_->removeResource(ct->resource());
}
