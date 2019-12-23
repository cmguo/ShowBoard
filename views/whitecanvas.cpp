#include "whitecanvas.h"

#include "resourcepageitem.h"
#include "toolboxitem.h"
#include "itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resourcepage.h"
#include "core/resourcepackage.h"
#include "core/resourcetransform.h"
#include "toolbarwidget.h"

#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

WhiteCanvas::WhiteCanvas(QObject * parent)
    : QObject(parent)
    , package_(nullptr)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFlags(ItemIsMovable);
    setPen(QPen(Qt::NoPen));
    //setBrush(QBrush(Qt::green));
    //addToGroup(new ItemSelector());
    canvas_ = new ResourcePageItem(this);
    canvas_->setRect(rect());
    globalCanvas_ = new ResourcePageItem(this);
    globalCanvas_->setRect(rect());
    tools_ = new ToolBoxItem(this);
    tools_->setRect(rect());
    selector_ = new ItemSelector(this);
    selector_->setRect(rect());
    void (ToolbarWidget::*sig)(QList<ToolButton *> const &) = &ToolbarWidget::buttonClicked;
    QObject::connect(selector_->toolBar(), sig, this, &WhiteCanvas::toolButtonClicked);
    QObject::connect(selector_->toolBar(), &ToolbarWidget::popupButtonsRequired,
                     this, &WhiteCanvas::popupButtonsRequired);
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

bool WhiteCanvas::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        break;
    default:
        return QGraphicsRectItem::sceneEvent(event);
    }
    return event->isAccepted();
}

void WhiteCanvas::switchPage(ResourcePage * page)
{
    selector_->select(nullptr);
    canvas_->switchPage(nullptr);
    setGeometry(scene()->sceneRect());
    canvas_->switchPage(page);
}

Control * WhiteCanvas::addResource(QUrl const & url)
{
    ResourceView * res = canvas_->page()->addResource(url);
    return canvas_->findControl(res);
}

Control *WhiteCanvas::copyResource(Control *control)
{
    control->beforeClone();
    ResourceView * res = canvas_->page()->copyResource(control->resource());
    return canvas_->findControl(res);
}

void WhiteCanvas::removeResource(Control *control)
{
    canvas_->page()->removeResource(control->resource());
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

ItemSelector * WhiteCanvas::selector()
{
    return selector_;
}

void WhiteCanvas::setGeometry(QRectF const & rect)
{
    setRect(rect);
    globalCanvas_->setGeometry(rect);
    canvas_->setGeometry(rect);
    tools_->setGeometry(rect);
    selector_->setRect(rect);
}

void WhiteCanvas::setResourcePackage(ResourcePackage * pack)
{
    if (package_) {
        QObject::disconnect(package_, &ResourcePackage::currentPageChanged,
                            this, &WhiteCanvas::switchPage);
    }
    package_ = pack;
    if (package_) {
        globalCanvas_->switchPage(package_->globalPage());
        switchPage(package_->currentPage());
        QObject::connect(package_, &ResourcePackage::currentPageChanged,
                         this, &WhiteCanvas::switchPage);
    } else {
        globalCanvas_->switchPage(nullptr);
        switchPage(nullptr);
    }
}

void WhiteCanvas::toolButtonClicked(QList<ToolButton *> const & buttons)
{
    Control * ct = Control::fromItem(selector_->selected());
    ToolButton * btn = buttons.back();
    if (btn->flags & ToolButton::HideSelector) {
        selector()->selectImplied(ct->item());
    }
    if (btn == &Control::btnTop) {
        ct->resource()->moveTop();
    } else if (btn == &Control::btnCopy) {
        selector_->select(nullptr);
        ct->beforeClone();
        ResourceView* res = canvas_->page()->copyResource(ct->resource());
        res->transform().translate({60, 60});
    } else if (btn == &Control::btnFastCopy) {
        bool checked = !btn->flags.testFlag(ToolButton::Checked);
        selector_->enableFastClone(checked);
        btn->flags.setFlag(ToolButton::Checked, checked);
    } else if (btn == &Control::btnDelete) {
        selector_->select(nullptr);
        canvas_->page()->removeResource(ct->resource());
    } else {
        ct->handleToolButton(buttons);
    }
}

void WhiteCanvas::popupButtonsRequired(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents)
{
    Control * ct = Control::fromItem(selector_->selected());
    ct->getToolButtons(buttons, parents);
}
/*
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
*/
