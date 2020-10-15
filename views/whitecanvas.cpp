#include "views/animcanvas.h"
#include "views/pageswitchevent.h"
#include "views/whitecanvas.h"
#include "views/pagecanvas.h"
#include "views/toolcanvas.h"
#include "views/itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resourcepage.h"
#include "core/resourcepackage.h"
#include "core/resourcetransform.h"
#include "widget/toolbarwidget.h"
#include "widget/qsshelper.h"

#include <QBrush>
#include <QPen>

#ifdef SHOWBOARD_QUICK
#include <QQuickWindow>
#else
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#endif
#include <QGuiApplication>

#include <controls/whitecanvascontrol.h>

int WhiteCanvas::THUMBNAIL_HEIGHT = 108;

WhiteCanvas::WhiteCanvas(QObject * parent)
    : package_(nullptr)
    , animCanvas_(nullptr)
    , canvasControl_(nullptr)
    , loadingCount_(0)
{
    static int THUMBNAIL_HEIGHT2 = dp(THUMBNAIL_HEIGHT);
    THUMBNAIL_HEIGHT = THUMBNAIL_HEIGHT2;

    setParent(parent);
    setObjectName("WhiteCanvas");
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFlags(ItemIsMovable);
    //setBrush(QBrush(Qt::green));
    //addToGroup(new ItemSelector());
    canvas_ = new PageCanvas(this);
    globalCanvas_ = new PageCanvas(this);
    tools_ = new ToolCanvas(this);
    selector_ = new ItemSelector(this);
}

WhiteCanvas::~WhiteCanvas()
{
    tools_->switchPage(nullptr);
    switchPage(nullptr);
}

ResourcePage *WhiteCanvas::page()
{
    return canvas_->page();
}

ResourcePage *WhiteCanvas::subPage()
{
    return canvas_->subPage();
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

#ifdef SHOWBOARD_QUICK

void WhiteCanvas::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == ItemSceneChange) {
        if (value.window == nullptr) {
            tools_->switchPage(nullptr);
        } else {
            QRectF rect = itemSceneRect(this);
            setGeometry(rect);
        }
    }
}

#else

QVariant WhiteCanvas::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange) {
        if (value.isNull()) {
            tools_->switchPage(nullptr);
        }
    } else if (change == ItemSceneHasChanged) {
        if (!value.isNull()) {
            QRectF rect = value.value<QGraphicsScene *>()->sceneRect();
            rect.moveCenter({0, 0});
            setGeometry(rect);
            tools_->switchPage(ResourcePackage::toolPage());
        }
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

#endif

bool WhiteCanvas::event(QEvent *event)
{
    switch (event->type()) {
    case PageSwitchEvent::PageSwitchStart:
        event->setAccepted(getDragAnimation(
                               static_cast<PageSwitchStartEvent*>(event)->delta().x() > 0));
        break;
    case PageSwitchEvent::PageSwitchMove:
        event->setAccepted(animCanvas_ && animCanvas_->move(
                               static_cast<PageSwitchMoveEvent*>(event)->delta()));
        break;
    case PageSwitchEvent::PageSwitchEnd:
        event->setAccepted(animCanvas_ && animCanvas_->release());
        break;
    default:
        return QObject::event(event);
    }
    return event->isAccepted();
}

void WhiteCanvas::switchPage(ResourcePage * page)
{
    if (page == this->page())
        return;
    loadingCount_ = 0;
    // handle animation & snapshot
    if (animCanvas_ && !animCanvas_->afterPageSwitch())
        animCanvas_->stopAnimate(); // will clear animCanvas_
    AnimCanvas* anim = nullptr;
    if (this->page() && !animCanvas_) {
        if (page && !this->page()->isVirtualPage()
                && !page->isVirtualPage() && !property("FromUser").toBool()) {
            QPixmap snapshot;
            this->page()->setThumbnail(canvas_->thumbnail(&snapshot));
            anim = new AnimCanvas(this);
            anim->setSnapshot(snapshot);
        } else {
            this->page()->setThumbnail(canvas_->thumbnail());
        }
    }
    // switch page
    if (canvasControl_ != Control::fromItem(this))
        delete canvasControl_;
    canvas_->switchPage(nullptr);
    setGeometry(itemSceneRect(this));
    canvas_->switchPage(page);
    canvasControl_ = qobject_cast<WhiteCanvasControl*>(Control::fromItem(this));
    if (page && canvasControl_ == nullptr)
        canvasControl_ = new WhiteCanvasControl(this);
    // handle animation
    int index = package_ ? package_->currentIndex() : -1;
    if (anim) {
        anim->stackBefore(globalCanvas_);
        anim->setDirection(index > lastPage_
                           ? AnimCanvas::RightToLeft : AnimCanvas::LeftToRight);
        anim->setAfterPageSwitch(true);
        anim->startAnimate();
        animCanvas_ = anim;
        connect(anim, &AnimCanvas::animateFinished, this, [this]() {
            delete animCanvas_;
            animCanvas_ = nullptr;
        });
    } else if (animCanvas_) {
        animCanvas_->updateCanvas();
    }
    emit currentPageChanged(canvas_->subPage());
    lastPage_ = index;
}

void WhiteCanvas::updateThunmbnail()
{
    if (page()) {
        page()->setThumbnail(canvas_->thumbnail());
    }
}

void WhiteCanvas::showSubPages(bool show)
{
    int i = childItems().indexOf(canvas_);
    while (++i < childItems().size() && childItems()[i] != globalCanvas_) {
        childItems()[i]->setVisible(show);
    }
}

AnimCanvas* WhiteCanvas::getDragAnimation(bool prev)
{
    AnimCanvas::AnimateDirection dir = prev ? AnimCanvas::LeftToRight : AnimCanvas::RightToLeft;
    if (animCanvas_) {
        if (animCanvas_->afterPageSwitch()
                || animCanvas_->inAnimate())
            return nullptr;
        animCanvas_->stopAnimate(); // will clear animCanvas_
    }
    ResourcePage * target = prev ? package_->prevPage(page()) : package_->nextPage(page());
    if (target == nullptr)
        return nullptr;
    animCanvas_ = new AnimCanvas(this);
    animCanvas_->setSnapshot(target->thumbnail());
    animCanvas_->setDirection(dir);
    animCanvas_->startAnimate();
    connect(animCanvas_, &AnimCanvas::animateFinished, this, [this](bool finished) {
        ResourcePage * target = nullptr;
        if (finished && animCanvas_->switchPage())
            target = animCanvas_->direction() == AnimCanvas::LeftToRight
                ? package_->prevPage(page())
                : package_->nextPage(page());
        delete animCanvas_;
        animCanvas_ = nullptr;
        if (target) {
            setProperty("FromUser", true);
            package_->switchPage(target);
            setProperty("FromUser", QVariant());
        }
    });
    return animCanvas_;
}

void WhiteCanvas::dragRelease()
{
    animCanvas_->release();
}

Control * WhiteCanvas::addResource(QUrl const & url, QVariantMap settings)
{
    ResourceView * res = canvas_->page()->addResource(url, settings);
    Control * ct = canvas_->findControl(res);
    // may be added to tool page
    if (ct == nullptr)
        ct = tools_->findControl(res);
    ct->setSelectOnLoaded();
    return ct;
}

Control *WhiteCanvas::addResource(ResourceView *res)
{
    canvas_->page()->addResource(res);
    Control * ct = canvas_->findControl(res);
    ct->setSelectOnLoaded();
    return ct;
}

Control *WhiteCanvas::copyResource(Control *control)
{
    control->beforeClone();
    ResourceView * res = canvas_->page()->copyResource(control->resource());
    Control* copy = canvas_->findControl(res);
    control->afterClone(copy);
    if (control == selector_->selected() && !(control->flags() & Control::Adjusting)) {
        selector_->unselect(control);
        copy->resource()->transform().translate({60, 60});
    }
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

void WhiteCanvas::select(Control *control)
{
    selector()->select(control);
}

Control *WhiteCanvas::selected()
{
    return selector_->selected();
}

Control * WhiteCanvas::selectFirst()
{
    Control * t = selectableNext(nullptr);
    if (t) {
        selector_->select(t);
        return t;
    }
    return nullptr;
}

Control * WhiteCanvas::selectNext()
{
    Control * t = selector_->selected();
    t = selectableNext(t);
    if (t) {
        selector_->select(t);
        return t;
    }
    return nullptr;
}

Control * WhiteCanvas::selectPrev()
{
    Control * t = selector_->selected();
    t = selectablePrev(t);
    if (t) {
        selector_->select(t);
        return t;
    }
    return nullptr;
}

Control * WhiteCanvas::selectLast()
{
    Control * t = selectablePrev(nullptr);
    if (t) {
        selector_->select(t);
        return t;
    }
    return nullptr;
}

Control *WhiteCanvas::selectableNext(Control * control)
{
    PageCanvas * pc = canvas_;
    int i = -1;
    ControlView * item = control ? control->item() : nullptr;
    if (item) {
         pc = static_cast<PageCanvas*>(item->parentItem());
         i = pc->childItems().indexOf(item);
    }
    while (++i < pc->childItems().size()) {
        ControlView * t = pc->childItems().at(i);
        control = Control::fromItem(t);
        if (control->flags() & Control::CanSelect)
            return control;
    }
    int j = childItems().indexOf(pc);
    while (true) {
        if (++j == childItems().count())
            j = 0;
        ControlView * pc2 = childItems().at(j);
        if (pc2 == pc)
            break;
#ifdef SHOWBOARD_QUICK
#else
        if (pc2->type() != PageCanvas::Type)
            continue;
#endif
        i = -1;
        while (++i < pc2->childItems().size()) {
            ControlView * t = pc2->childItems().at(i);
            control = Control::fromItem(t);
            if (control->flags() & Control::CanSelect)
                return control;
        }
    }
    if (item) {
        i = -1;
        while (++i < pc->childItems().size()) {
            ControlView * t = pc->childItems().at(i);
            if (t == item)
                break;
            control = Control::fromItem(t);
            if (control->flags() & Control::CanSelect)
                return control;
        }
    }
    return nullptr;
}

Control *WhiteCanvas::selectablePrev(Control * control)
{
    PageCanvas * pc = globalCanvas_;
    int i = pc->childItems().size();
    ControlView * item = control ? control->item() : nullptr;
    if (item) {
         pc = static_cast<PageCanvas*>(item->parentItem());
         i = pc->childItems().indexOf(item);
    }
    while (i > 0) {
        ControlView * t = pc->childItems().at(--i);
        control = Control::fromItem(t);
        if (control->flags() & Control::CanSelect)
            return control;
    }
    int j = childItems().indexOf(pc);
    while (true) {
        if (--j < 0)
            j = childItems().count() - 1;
        ControlView * pc2 = childItems().at(j);
        if (pc2 == pc)
            break;
#ifdef SHOWBOARD_QUICK
#else
        if (pc2->type() != PageCanvas::Type)
            continue;
#endif
        i = pc2->childItems().size();
        while (i > 0) {
            ControlView * t = pc2->childItems().at(--i);
            control = Control::fromItem(t);
            if (control->flags() & Control::CanSelect)
                return control;
        }
    }
    if (item) {
        pc = static_cast<PageCanvas*>(item->parentItem());
        i = pc->childItems().size();
        while (i > 0) {
            ControlView * t = pc->childItems().at(--i);
            control = Control::fromItem(t);
            if (control->flags() & Control::CanSelect)
                return control;
        }
    }
    return nullptr;
}

ItemSelector * WhiteCanvas::selector()
{
    return selector_;
}

WhiteCanvasControl *WhiteCanvas::canvasControl()
{
    return canvasControl_;
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

void WhiteCanvas::onControlLoading(Control * control, bool startOrFinished)
{
    emit controlLoading(control, startOrFinished);
    if (startOrFinished) {
        if (++loadingCount_ == 1)
            emit loadingChanged(true);
    } else if (--loadingCount_ == 0) {
        emit loadingChanged(false);
    }
}

void WhiteCanvas::onControlLoadFailed(Control *control, std::exception_ptr exception)
{
    emit controlFailed(control, exception);
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
