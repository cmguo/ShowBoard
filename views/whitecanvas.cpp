#include "whitecanvas.h"

#include "resourcepageitem.h"
#include "toolboxitem.h"
#include "itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resourcepage.h"
#include "toolbarwidget.h"

#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

WhiteCanvas::WhiteCanvas(QObject * parent)
    : QObject(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFlags(ItemIsMovable);
    setPen(QPen(Qt::NoPen));
    //setBrush(QBrush(Qt::green));
    //addToGroup(new ItemSelector());
    canvas_ = new ResourcePageItem(this);
    canvas_->setRect(rect());
    tools_ = new ToolBoxItem(this);
    tools_->setRect(rect());
    selector_ = new ItemSelector(this);
    selector_->setRect(rect());
    void (ToolbarWidget::*sig)(ToolButton *) = &ToolbarWidget::buttonClicked;
    QObject::connect(selector_->toolBar(), sig, this, &WhiteCanvas::toolButtonClicked);
    switchPage(new ResourcePage);
}

WhiteCanvas::~WhiteCanvas()
{
    switchPage(nullptr);
}

void WhiteCanvas::showToolControl(const QString &type)
{
    tools_->showToolControl(type);
}

void WhiteCanvas::hideToolControl(const QString &type)
{
    tools_->hideToolControl(type);
}

Control * WhiteCanvas::getToolControl(const QString &type)
{
    return tools_->getToolControl(type);
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
    canvas_->switchPage(page);
}

Control * WhiteCanvas::addResource(QUrl const & url)
{
    ResourceView * res = canvas_->page()->addResource(url);
    return canvas_->findControl(res);
}

Control * WhiteCanvas::findControl(ResourceView * res)
{
    return canvas_->findControl(res);
}

Control * WhiteCanvas::findControl(QUrl const & url)
{
    return canvas_->findControl(url);
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

void WhiteCanvas::moveSelectionTop(bool enable)
{
    selector_->autoTop(enable);
}

void WhiteCanvas::setGeometry(QRectF const & rect)
{
    setRect(rect);
    canvas_->setGeometry(rect);
    tools_->setGeometry(rect);
    selector_->setRect(rect);
}

void WhiteCanvas::toolButtonClicked(ToolButton * button)
{
    Control * ct = Control::fromItem(selector_->selected());
    if (button == &Control::btnCopy) {
        selector_->select(nullptr);
        canvas_->page()->copyResource(ct->resource());
    } else if (button == &Control::btnDelete) {
        selector_->select(nullptr);
        canvas_->page()->removeResource(ct->resource());
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

