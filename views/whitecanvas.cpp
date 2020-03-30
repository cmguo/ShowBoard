#include "animcanvas.h"
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
#include <QGuiApplication>

WhiteCanvas::WhiteCanvas(QObject * parent)
    : QObject(parent)
    , package_(nullptr)
    , animCanvas_(nullptr)
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

void WhiteCanvas::showToolControl(const QString &typeOrUrl)
{
    tools_->showToolControl(getToolControl(typeOrUrl));
}

void WhiteCanvas::showToolControl(Control *control)
{
    tools_->showToolControl(control);
}

void WhiteCanvas::hideToolControl(const QString &typeOrUrl)
{
    tools_->hideToolControl(getToolControl(typeOrUrl));
}

void WhiteCanvas::hideToolControl(Control *control)
{
    tools_->hideToolControl(control);
}

Control * WhiteCanvas::getToolControl(const QString &typeOrUrl)
{
    return tools_->getToolControl(typeOrUrl);
}

QVariant WhiteCanvas::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        QRectF rect = value.value<QGraphicsScene *>()->sceneRect();
        rect.moveCenter({0, 0});
        setGeometry(rect);
        tools_->switchPage(ResourcePackage::toolPage());
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
    case QEvent::GraphicsSceneWheel:
        if (Control * c = Control::fromItem(this)) {
            QPointF d;
            if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
                d.setX(static_cast<QGraphicsSceneWheelEvent*>(event)->delta());
            else
                d.setY(static_cast<QGraphicsSceneWheelEvent*>(event)->delta());
            c->resource()->transform().translate(d);
        }
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
    AnimCanvas* anim = nullptr;
    if (this->page() && !animCanvas_) {
        if (page && !this->page()->isVirtualPage()
                && !page->isVirtualPage() && !property("FromUser").toBool())
            anim = new AnimCanvas(this);
        this->page()->setThumbnail(canvas_->thumbnail(anim ? &anim->snapshot() : nullptr));
    }
    canvas_->switchPage(nullptr);
    setGeometry(scene()->sceneRect());
    canvas_->switchPage(page);
    int index = package_ ? package_->currentIndex() : -1;
    if (anim) {
        anim->stackBefore(globalCanvas_);
        anim->startAnimate(index > lastPage_ ? PageCanvas::RightToLeft : PageCanvas::LeftToRight);
        animCanvas_ = anim;
        connect(anim, &AnimCanvas::animateFinished, this, [this]() {
            delete animCanvas_;
            animCanvas_ = nullptr;
        });
    } else if (animCanvas_) {
        animCanvas_->updateAnimate();
    }
    lastPage_ = index;
}

void WhiteCanvas::updateThunmbnail()
{
    if (page()) {
        page()->setThumbnail(canvas_->thumbnail());
    }
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
    if (control->item() == selector_->selected() && !(control->flags() & Control::Adjusting)) {
        selector_->select(nullptr);
        copy->resource()->transform().translate({60, 60});
    }
    return copy;
}

void WhiteCanvas::removeResource(Control *control)
{
    if (control->item() == selector_->selected())
        selector_->select(nullptr);
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

void WhiteCanvas::select(Control *control)
{
    selector()->select(control ? control->item() : nullptr);
}

Control *WhiteCanvas::selected()
{
    QGraphicsItem * t = selector_->selected();
    return t ? Control::fromItem(t) : nullptr;
}

Control * WhiteCanvas::selectFirst()
{
    QGraphicsItem * t = selectableNext(nullptr);
    if (t) {
        selector_->select(t);
        return Control::fromItem(t);
    }
    return nullptr;
}

Control * WhiteCanvas::selectNext()
{
    QGraphicsItem * t = selector_->selected();
    t = selectableNext(t);
    if (t) {
        selector_->select(t);
        return Control::fromItem(t);
    }
    return nullptr;
}

Control * WhiteCanvas::selectPrev()
{
    QGraphicsItem * t = selector_->selected();
    t = selectablePrev(t);
    if (t) {
        selector_->select(t);
        return Control::fromItem(t);
    }
    return nullptr;
}

Control * WhiteCanvas::selectLast()
{
    QGraphicsItem * t = selectablePrev(nullptr);
    if (t) {
        selector_->select(t);
        return Control::fromItem(t);
    }
    return nullptr;
}

QGraphicsItem *WhiteCanvas::selectableNext(QGraphicsItem *item)
{
    PageCanvas * pc = canvas_;
    int i = -1;
    if (item) {
         pc = static_cast<PageCanvas*>(item->parentItem());
         i = pc->childItems().indexOf(item);
    }
    while (++i < pc->childItems().size()) {
        QGraphicsItem * t = pc->childItems().at(i);
        if (Control::fromItem(t)->flags() & Control::CanSelect)
            return t;
    }
    pc = pc == canvas_ ? globalCanvas_ : canvas_;
    i = -1;
    while (++i < pc->childItems().size()) {
        QGraphicsItem * t = pc->childItems().at(i);
        if (Control::fromItem(t)->flags() & Control::CanSelect)
            return t;
    }
    if (item) {
        pc = static_cast<PageCanvas*>(item->parentItem());
        i = -1;
        while (++i < pc->childItems().size()) {
            QGraphicsItem * t = pc->childItems().at(i);
            if (Control::fromItem(t)->flags() & Control::CanSelect)
                return t;
        }
    }
    return nullptr;
}

QGraphicsItem *WhiteCanvas::selectablePrev(QGraphicsItem *item)
{
    PageCanvas * pc = globalCanvas_;
    int i = pc->childItems().size();
    if (item) {
         pc = static_cast<PageCanvas*>(item->parentItem());
         i = pc->childItems().indexOf(item);
    }
    while (i > 0) {
        QGraphicsItem * t = pc->childItems().at(--i);
        if (Control::fromItem(t)->flags() & Control::CanSelect)
            return t;
    }
    pc = pc == canvas_ ? globalCanvas_ : canvas_;
    i = pc->childItems().size();
    while (i > 0) {
        QGraphicsItem * t = pc->childItems().at(--i);
        if (Control::fromItem(t)->flags() & Control::CanSelect)
            return t;
    }
    if (item) {
        pc = static_cast<PageCanvas*>(item->parentItem());
        i = pc->childItems().size();
        while (i > 0) {
            QGraphicsItem * t = pc->childItems().at(--i);
            if (Control::fromItem(t)->flags() & Control::CanSelect)
                return t;
        }
    }
    return nullptr;
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
    if (btn->isHideSelector()) {
        selector()->selectImplied(ct->item());
    }
    if (btn == &Control::btnTop) {
        ct->resource()->moveTop();
    } else if (btn == &Control::btnCopy) {
        copyResource(ct);
    } else if (btn == &Control::btnFastCopy) {
        bool checked = !btn->isChecked();
        selector_->enableFastClone(checked);
        btn->setChecked(checked);
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
