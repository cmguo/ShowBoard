#include "whitecanvas.h"

#include "pagecanvas.h"
#include "toolcanvas.h"
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
    , loadingCount_(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFlags(ItemIsMovable);
    //setBrush(QBrush(Qt::green));
    //addToGroup(new ItemSelector());
    canvas_ = new PageCanvas(this);
    globalCanvas_ = new PageCanvas(this);
    tools_ = new ToolCanvas(this);
    selector_ = new ItemSelector(this);
    void (ToolbarWidget::*sig)(QList<ToolButton *> const &) = &ToolbarWidget::buttonClicked;
    QObject::connect(selector_->toolBar(), sig, this, &WhiteCanvas::toolButtonClicked);
    QObject::connect(selector_->toolBar(), &ToolbarWidget::popupButtonsRequired,
                     this, &WhiteCanvas::popupButtonsRequired);
}

WhiteCanvas::~WhiteCanvas()
{
    switchPage(nullptr);
}

ResourcePage *WhiteCanvas::page()
{
    return canvas_->page();
}

void WhiteCanvas::showToolControl(const QString &type)
{
    tools_->showToolControl(getToolControl(type));
}

void WhiteCanvas::showToolControl(Control *control)
{
    tools_->showToolControl(control);
}

void WhiteCanvas::hideToolControl(const QString &type)
{
    tools_->hideToolControl(getToolControl(type));
}

void WhiteCanvas::hideToolControl(Control *control)
{
    tools_->hideToolControl(control);
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
        return CanvasItem::sceneEvent(event);
    }
    return event->isAccepted();
}

void WhiteCanvas::switchPage(ResourcePage * page)
{
    loadingCount_ = 0;
    selector_->select(nullptr);
    canvas_->switchPage(nullptr);
    setGeometry(scene()->sceneRect());
    canvas_->switchPage(page);
}

Control * WhiteCanvas::addResource(QUrl const & url, QVariantMap settings)
{
    ResourceView * res = canvas_->page()->addResource(url, settings);
    return canvas_->findControl(res);
}

Control *WhiteCanvas::copyResource(Control *control)
{
    control->beforeClone();
    ResourceView * res = canvas_->page()->copyResource(control->resource());
    Control* copy = canvas_->findControl(res);
    control->afterClone(copy);
    return copy;
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
    return canvas_->topControl();
}

ItemSelector * WhiteCanvas::selector()
{
    return selector_;
}

void WhiteCanvas::enableSelector(bool enable)
{
    if (enable) {
        selector_->setRect(rect());
    } else {
        selector_->setRect(QRectF());
        selector_->select(nullptr);
    }
}

bool WhiteCanvas::loading()
{
    return loadingCount_;
}

void WhiteCanvas::onControlLoad(bool startOrFinished)
{
    if (startOrFinished) {
        if (++loadingCount_ == 1)
            emit loadingChanged(true);
    } else if (--loadingCount_ == 0) {
        emit loadingChanged(false);
    }
}

void WhiteCanvas::setGeometry(QRectF const & rect)
{
    setRect(rect);
    globalCanvas_->relayout();
    canvas_->relayout();
    tools_->relayout();
    if (!selector_->rect().isEmpty())
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
        copyResource(ct)->resource()->transform().translate({60, 60});
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
