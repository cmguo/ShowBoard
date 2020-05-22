#include "whitecanvascontrol.h"
#include "views/whitecanvas.h"
#include "views/positionbar.h"
#include "views/toolbarwidget.h"
#include "views/qsshelper.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"

#include <QUrl>
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>
#include <QGraphicsProxyWidget>

static QssHelper QSS(":/showboard/qss/canvastoolbar.qss");

static constexpr char const * toolsStr =
        "scaleUp()||UnionUpdate|:/showboard/icon/zoom_in.svg,default;"
        "scaleDown()||UnionUpdate|:/showboard/icon/zoom_out.svg,default;";

WhiteCanvasControl::WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas)
    : Control(view, {}, {CanSelect, CanScale, CanRotate})
{
    setToolsString(toolsStr);
    item_ = canvas;
    item_->setTransformations({transform_});
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    realItem_ = item_;
    QObject::connect(&res_->transform(), &ResourceTransform::beforeChanged,
                     this, &WhiteCanvasControl::updatingTransform);
    QObject::connect(&res_->transform(), &ResourceTransform::changed,
                     this, &WhiteCanvasControl::updateTransform);
    PositionBar* posBar = new PositionBar(canvas);
    posBar->setInCanvas();
    posBar_ = posBar;
    ToolbarWidget* toolbar = new ToolbarWidget;
    toolbar->setObjectName("canvastoolbar");
    toolbar->attachProvider(this);
    toolbar->setStyleSheet(QSS);
    toolbar->setDragable();
    toolBar_ = toolbar->toGraphicsProxy(nullptr, true);
    loadSettings();
    // adjust to scene, this is done before attaching transform
    res_->transform().translate(QPointF(0, 0));
    qDebug() << "WhiteCanvasControl" << res_->transform().transform();
    item_->scene()->addItem(toolBar_);
    toolBar_->setPos(QPointF(0, item_->scene()->sceneRect().bottom() - 60));
}

WhiteCanvasControl::~WhiteCanvasControl()
{
    delete toolBar_;
    toolBar_ = nullptr;
    qDebug() << "~WhiteCanvasControl" << res_->transform().transform();
    saveSettings();
    delete posBar_;
    item_->setData(ITEM_KEY_CONTROL, QVariant());
    item_->setTransformations({});
    item_ = nullptr;
    realItem_ = nullptr;
}

void WhiteCanvasControl::setPosBarVisible(bool visible)
{
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

QGraphicsItem *WhiteCanvasControl::create(ResourceView *res)
{
    (void) res;
    return nullptr;
}

void WhiteCanvasControl::resize(const QSizeF &size)
{
    qDebug() << "WhiteCanvasControl resize" << size;
    QRectF old = item_->boundingRect();
    QRectF rect(QPointF(0, 0), size);
    rect.moveCenter(QPointF(0, 0));
    rect |= old;
    static_cast<WhiteCanvas*>(item_)->setGeometry(rect);
    QSizeF ds = (rect.size() - old.size()) / 2;
    QPointF d{ds.width(), ds.height()};
    move(d);
}

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

void WhiteCanvasControl::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (parent == nullptr) {
        if (!flags_.testFlag(CanScale))
            return;
    }
    return Control::getToolButtons(buttons, parent);
}

void WhiteCanvasControl::updateToolButton(ToolButton *button)
{
    if (button->name() == "scaleUp()") {
        button->setEnabled(resource()->transform().scale().m11() < 10);
    } else if (button->name() == "scaleDown()") {
        button->setEnabled(resource()->transform().scale().m11() > 1.1);
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
