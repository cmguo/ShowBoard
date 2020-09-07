#include "whitecanvascontrol.h"
#include "views/whitecanvas.h"
#include "views/positionbar.h"
#include "views/toolbarwidget.h"
#include "views/qsshelper.h"
#include "views/itemselector.h"
#include "views/animcanvas.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"

#include <QUrl>
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QGuiApplication>
#include <QScreen>

static QssHelper QSS(":/showboard/qss/canvastoolbar.qss");

static constexpr char const * toolsStr =
        "scaleUp()||UnionUpdate|:/showboard/icon/zoom_in.svg,default;"
        "scaleDown()||UnionUpdate|:/showboard/icon/zoom_out.svg,default;"
        "|;"
        "close()||NeedUpdate|:/showboard/icon/close.svg;";

WhiteCanvasControl::WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas)
    : Control(view, {}, {CanSelect, CanScale, CanRotate})
{
    setToolsString(toolsStr);
    item_ = canvas;
    item_->setTransformations({transform_});
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    realItem_ = item_;

#ifdef PROD_TEST
    setParent(static_cast<WhiteCanvas*>(item_)); // for testbed
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
    toolBar_ = toolbar->toGraphicsProxy(nullptr, true);
    toolbar->attachProvider(this);
    loadSettings();
    // adjust to scene, this is done before attaching transform
    res_->transform().translate(QPointF(0, 0));
    qDebug() << "WhiteCanvasControl" << res_->transform().transform();
    item_->scene()->addItem(toolBar_);

    int offset = dp(60);
    QRectF rect = item_->scene()->sceneRect();
    ResourceView * originResourView = view->page()->mainResource();
    QString toolbarPosition = originResourView->property("toolbarPosition").toString();
    if (toolbarPosition == "rightBottom") {
        qreal right = rect.right() - dp(110);
        offset = dp(80);
        QPointF position(right - offset, rect.bottom() - offset);
        toolBar_->setPos(position);
    } else {
        toolBar_->setPos(QPointF(0, rect.bottom() - offset));
    }
    flags_.setFlag(LoadFinished);
}

WhiteCanvasControl::WhiteCanvasControl(WhiteCanvas *canvas)
    : Control(new ResourceView(new Resource("fakecanvas", QUrl())), {},
                                          Control::CanSelect | Control::CanScale | Control::CanRotate)
{
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
        static_cast<WhiteCanvas*>(item_)->selector()->unselect(this);
    item_->setData(ITEM_KEY_CONTROL, QVariant());
    item_->setTransformations({});
    item_ = nullptr;
    realItem_ = nullptr;
}

void WhiteCanvasControl::setToolBarStyles(const QString &stylesheet)
{
    qobject_cast<ToolbarWidget*>(static_cast<QGraphicsProxyWidget*>(toolBar_)->widget())
            ->setStyleSheet(QssHelper(stylesheet));
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
    res_->transform().scale({1.25, 1.25});
}

void WhiteCanvasControl::scaleDown()
{
    res_->transform().scale({0.8, 0.8});
}

void WhiteCanvasControl::close()
{
    QTimer::singleShot(0, this, [this] () {
        res_->page()->removeFromPackage();
    });
}

QGraphicsItem *WhiteCanvasControl::create(ResourceView *res)
{
    (void) res;
    return nullptr;
}

void WhiteCanvasControl::resize(const QSizeF &size)
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        qDebug() << "WhiteCanvasControl resize" << size;
        QRectF old = item_->boundingRect();
        QRectF srect = item_->scene()->sceneRect();
        QRectF rect(QPointF(0, 0), size);
        rect.moveCenter(QPointF(0, 0));
        rect |= srect;
        static_cast<WhiteCanvas*>(item_)->setGeometry(rect);
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

void WhiteCanvasControl::adjusting(bool be)
{
    Control::adjusting(be);
    if (!be) {
        if (anim_)
            anim_->release();
        anim_ = nullptr;
    }
}

void WhiteCanvasControl::move(QPointF &delta)
{
    if (anim_) {
        if (anim_->move(delta))
            return;
        anim_ = nullptr;
    }
    QPointF d = delta;
    Control::move(delta);
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y()) && !qFuzzyIsNull(d.x())) {
        delta.setX(d.x());
        anim_ = static_cast<WhiteCanvas*>(item_)->getDragAnimation(delta.x() > 0);
        if (anim_)
            anim_->move(delta);
    }
}

void WhiteCanvasControl::gesture(const QPointF &from1, const QPointF &from2, QPointF &to1, QPointF &to2)
{
    QPointF d = (to1 + to2 - from1 - from2) / 2;
    if (anim_) {
        if (anim_->move(d))
            return;
        anim_ = nullptr;
    }
    qreal s = res_->transform().scale().m11();
    Control::gesture(from1, from2, to1, to2);
    QPointF delta = (to1 + to2 - from1 - from2) / 2;
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y()) && !qFuzzyIsNull(d.x())
            && qFuzzyIsNull((s / res_->transform().scale().m11() - 1.0) / 1000.0)) {
        delta.setX(d.x());
        anim_ = static_cast<WhiteCanvas*>(item_)->getDragAnimation(delta.x() > 0);
        if (anim_)
            anim_->move(delta);
    } else {
        d -= delta;
        to1 += d;
        to2 += d;
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
        button->setEnabled(resource()->transform().scale().m11() < 10);
        button->setVisible(flags_.testFlag(CanScale) && !flags_.testFlag(NoScaleButton));
    } else if (button->name() == "scaleDown()") {
        button->setEnabled(resource()->transform().scale().m11() > 1.1);
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
    QRectF srect = item_->scene()->sceneRect();
    QRectF crect = item_->boundingRect();
    transform.keepOuterOf(srect, crect);
}

void WhiteCanvasControl::updateTransform()
{
    ResourceTransform & transform = res_->transform();
    //qDebug() << "WhiteCanvasControl" << transform.transform();
    QRectF srect = item_->scene()->sceneRect();
    QRectF crect = item_->boundingRect();
    posBar_->update(srect, crect, transform.scale().m11(), transform.offset());
}
