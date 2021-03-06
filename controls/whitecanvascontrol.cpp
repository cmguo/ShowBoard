#include "whitecanvascontrol.h"
#include "views/whitecanvas.h"
#ifdef SHOWBOARD_QUICK
#include "quick/positionbar.h"
#else
#include "graphics/positionbar.h"
#endif
#include "widget/toolbarwidget.h"
#include "widget/qsshelper.h"
#include "views/itemselector.h"
#include "views/animcanvas.h"
#include "views/pageswitchevent.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"
#include "core/controltransform.h"

#include <QUrl>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QApplication>
#include <QDebug>

static QssHelper QSS(":/showboard/qss/canvastoolbar.qss");

static constexpr char const * toolsStr =
        "scaleUp()||UnionUpdate|:/showboard/icon/zoom_in.svg,default;"
        "scaleDown()||UnionUpdate|:/showboard/icon/zoom_out.svg,default;"
        "|;"
        "close()||NeedUpdate|:/showboard/icon/close.svg;";

WhiteCanvasControl::WhiteCanvasControl(ResourceView * view, ControlView * canvas)
    : Control(view, {}, {CanSelect, CanScale, CanRotate})
{
    setToolsString(toolsStr);
    item_ = canvas;
    transform_ = new ControlTransform(res_->transform(), ControlTransform::PureItem);
    transform_->appendToItem(item_);
    attachToItem(item_, this);
    realItem_ = item_;
    connect(&res_->transform(), &ResourceTransform::changed, this, [this] (int elem) {
        Control::updateTransform(elem);
    });

#ifdef PROD_TEST
    setParent(whiteCanvas()); // for testbed
#endif

    if (view->flags().testFlag(ResourceView::LargeCanvas)) {
        QObject::connect(&res_->transform(), &ResourceTransform::beforeChanged,
                         this, &WhiteCanvasControl::updatingTransform);
        QObject::connect(&res_->transform(), &ResourceTransform::changed,
                         this, &WhiteCanvasControl::updateTransform);
        PositionBar* posBar = new PositionBar(canvas);
        posBar->setInCanvas();
        posBar_ = posBar;
    }
    ToolbarWidget* toolbar = new ToolbarWidget;
    toolbar->setObjectName("canvastoolbar");
    toolbar->setStyleSheet(QSS);
    toolbar->setDragable();
#ifdef SHOWBOARD_QUICK
#else
    toolBar_ = toolbar->toGraphicsProxy(nullptr, true);
#endif
    toolbar->attachProvider(this);
    loadSettings();
    // adjust to scene, this is done before attaching transform
    res_->transform().translate(QPointF(0, 0));
    qDebug() << "WhiteCanvasControl" << res_->transform().transform();
#ifdef SHOWBOARD_QUICK
#else
    item_->scene()->addItem(toolBar_);
    int offset = dp(60);
    QRectF rect = item_->scene()->sceneRect();
    ResourceView * originResourView = view->page()->mainResource();
    QString toolbarPosition = originResourView->property("toolbarPosition").toString();
    if (toolbarPosition == "rightBottom") {
        QPointF position(rect.right() - dp(150), rect.bottom() - dp(120));
        toolBar_->setPos(position);
    } else {
        toolBar_->setPos(QPointF(0, rect.bottom() - offset));
    }
#endif
    flags_.setFlag(LoadFinished);
}

WhiteCanvasControl::WhiteCanvasControl(WhiteCanvas *canvas)
    : Control(new ResourceView(new Resource("fakecanvas", QUrl())), {},
                                          Control::CanSelect | Control::CanScale | Control::CanRotate)
{
    res_->setParent(canvas->page());
    item_ = canvas;
    realItem_ = item_;
    QObject::connect(&res_->transform(), &ResourceTransform::beforeChanged,
                     this, [&t = res_->transform()]() { t.keepAtOrigin(); });
}

WhiteCanvasControl::~WhiteCanvasControl()
{
    delete toolBar_;
    toolBar_ = nullptr;
    qDebug() << "~WhiteCanvasControl" << res_->transform().transform();
    saveSettings();
    delete posBar_;
    if (flags_ & (Selected | Adjusting))
        whiteCanvas()->selector()->unselect(this);
    attachToItem(item_, nullptr);
    ControlTransform::removeAllTransforms(item_);
    item_ = nullptr;
    realItem_ = nullptr;
}

void WhiteCanvasControl::setToolBarStyles(const QString &stylesheet)
{
    widgetFromItem(toolBar_)->setStyleSheet(QssHelper(stylesheet));
}

void WhiteCanvasControl::setNoScaleButton(bool b)
{
    flags_.setFlag(NoScaleButton, b);
}

void WhiteCanvasControl::setPosBarVisible(bool visible)
{
    if (posBar_)
        posBar_->setVisible(visible);
}

void WhiteCanvasControl::scaleUp()
{
    Control * c = static_cast<WhiteCanvas*>(item_)
            ->findControl(res_->page()->mainResource());
    if (!c->exec("scaleUp()"))
        res_->transform().scale({1.25, 1.25});
}

void WhiteCanvasControl::scaleDown()
{
    Control * c = static_cast<WhiteCanvas*>(item_)
            ->findControl(res_->page()->mainResource());
    if (!c->exec("scaleDown()"))
        res_->transform().scale({0.8, 0.8});
}

void WhiteCanvasControl::close()
{
    QTimer::singleShot(0, this, [this] () {
        res_->page()->removeFromPackage();
    });
}

ControlView *WhiteCanvasControl::create(ControlView *parent)
{
    (void) parent;
    return nullptr;
}

void WhiteCanvasControl::resize(const QSizeF &size)
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        qDebug() << "WhiteCanvasControl resize" << size;
        QRectF old = item_->boundingRect();
        QRectF srect = itemSceneRect(item_);
        QRectF rect(QPointF(0, 0), size);
        rect.moveCenter(QPointF(0, 0));
        rect |= srect;
        whiteCanvas()->setGeometry(rect);
        if (flags_.testFlag(LoadFinished)) {
            QSizeF ds = (rect.size() - old.size()) / 2;
            QPointF d{ds.width(), ds.height()};
            move(d);
        }
        setMaxSize(size.toSize() * 4);
    }
}

// called when main resource attached
void WhiteCanvasControl::attached()
{
    if (flags_.testFlag(CanScale)) {
        buttonsChanged();
    }
}

void WhiteCanvasControl::sizeChanged()
{
    // do nothing
}

void WhiteCanvasControl::adjustEnd(int source)
{
    Control::adjustEnd(source);
    pageSwitchEnd(false);
}

void WhiteCanvasControl::move(QPointF &delta)
{
    if (pageSwitchMove(delta))
        return;
    QPointF d = delta;
    Control::move(delta);
    // may from keyboard input, then not anim switch
    QEvent * oe = whiteCanvas()->selector()->currentEvent();
    if (oe && qFuzzyIsNull(delta.x()) && qAbs(delta.y()) < qAbs(d.x()) && !qFuzzyIsNull(d.x())) {
        delta.setX(d.x());
        pageSwitchStart(delta);
    }
}

void WhiteCanvasControl::gesture(GestureContext *ctx, QPointF const &to1, QPointF const &to2)
{
    QPointF d = (to1 + to2 - ctx->to1() - ctx->to2()) / 2;
    if (pageSwitchMove(d)) {
        ctx->commit(to1, to2, res_->transform().offset());
        return;
    }
    Control::gesture(ctx, to1, to2);
    QPointF delta = ctx->translate();
    QPointF left = to1 - ctx->from1();
    qDebug() << "WhiteCanvasControl::gesture" << d << delta << left;
    if (qFuzzyIsNull(delta.x()) && qAbs(delta.y()) < qAbs(d.x())) {
        delta.setX(left.x());
        delta.setY(0);
        pageSwitchStart(delta);
        ctx->adjustOffsetAfterCommit(delta);
    }
}

void WhiteCanvasControl::getToolButtons(QList<ToolButton *> &buttons, QList<ToolButton *> const & parents)
{
    // not include default buttons of Control
    Control::getToolButtons(buttons, parents);
    if (parents.isEmpty()) {
        ToolButton * closeBtn = getStringButton("close()");
        buttons.removeOne(closeBtn);
        buttons.append(&ToolButton::SPLITTER);
        buttons.append(closeBtn);
    }
}

void WhiteCanvasControl::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    // not include default buttons of Control
    ToolButtonProvider::getToolButtons(buttons, parent);
}

void WhiteCanvasControl::updateToolButton(ToolButton *button)
{
    if (button->name() == "scaleUp()") {
        button->setEnabled(resource()->transform().zoom() < 10);
        button->setVisible(flags_.testFlag(CanScale) && !flags_.testFlag(NoScaleButton));
    } else if (button->name() == "scaleDown()") {
        button->setEnabled(resource()->transform().zoom() > 1.1);
        button->setVisible(flags_.testFlag(CanScale) && !flags_.testFlag(NoScaleButton));
    } else if (button->name() == "close()") {
        button->setVisible(res_->flags().testFlag(ResourceView::CanDelete));
    } else {
        ToolButtonProvider::updateToolButton(button);
    }
}

void WhiteCanvasControl::updatingTransform()
{
    ResourceTransform & transform = res_->transform();
    QRectF srect = itemSceneRect(item_);
    QRectF crect = item_->boundingRect();
    transform.keepOuterOf(srect, crect);
}

void WhiteCanvasControl::updateTransform()
{
    ResourceTransform & transform = res_->transform();
    //qDebug() << "WhiteCanvasControl" << transform.transform();
    QRectF srect = itemSceneRect(item_);
    QRectF crect = item_->boundingRect();
    posBar_->update(srect, crect, transform.zoom(), transform.offset());
}

WhiteCanvas * WhiteCanvasControl::whiteCanvas()
{
    return static_cast<WhiteCanvas*>(item_);
}

void WhiteCanvasControl::pageSwitchStart(const QPointF &delta)
{
    QEvent * oe = whiteCanvas()->selector()->currentEvent();
    if (!oe || oe->type() == QEvent::GraphicsSceneWheel)
        return;
    PageSwitchStartEvent e(delta);
    e.setOriginEvent(oe);
    Control * c = static_cast<WhiteCanvas*>(item_)
            ->findControl(res_->page()->mainResource());
    if (c && c->event(&e) && e.isAccepted()) {
        pageSwitch_ = c;
        whiteCanvas()->showSubPages(false);
    } else {
#ifdef QT_DEBUG
        pageSwitch_ = whiteCanvas();
        if (!pageSwitch_->event(&e) || !e.isAccepted())
            pageSwitch_ = nullptr;
#endif
    }
    if (pageSwitch_)
        pageSwitchMove(delta);
}

bool WhiteCanvasControl::pageSwitchMove(const QPointF &delta)
{
    if (pageSwitch_) {
        PageSwitchMoveEvent e(delta);
        e.setOriginEvent(whiteCanvas()->selector()->currentEvent());
        if (!pageSwitch_->event(&e) || !e.isAccepted())
            pageSwitchEnd(true);
    }
    return pageSwitch_ != nullptr;
}

bool WhiteCanvasControl::pageSwitchEnd(bool cancel)
{
    if (pageSwitch_ == nullptr)
        return false;
    if (pageSwitch_ != whiteCanvas())
        whiteCanvas()->showSubPages(true);
    QObject * ps = pageSwitch_;
    pageSwitch_ = nullptr;
    if (cancel) {
        PageSwitchEndEvent e;
        e.setOriginEvent(whiteCanvas()->selector()->currentEvent());
        return ps->event(&e) && e.isAccepted();
    } else {
        PageSwitchEndEvent * e = new PageSwitchEndEvent;
        // delay PageSwitchEndEvent event,
        //  avoid lost InkStrokeControl's filter soon
        QApplication::postEvent(ps, e);
        return true;
    }
}
