#include "whitecanvascontrol.h"
#include "views/whitecanvas.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"

#include <QUrl>
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>

WhiteCanvasControl::WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas)
    : Control(view, {}, {CanSelect, CanRotate})
{
    item_ = canvas;
    item_->setTransformations({transform_});
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    realItem_ = item_;
    QObject::connect(&res_->transform(), &ResourceTransform::beforeChanged,
                     this, &WhiteCanvasControl::updateTransform);
    loadSettings();
}

WhiteCanvasControl::~WhiteCanvasControl()
{
    saveSettings();
    item_->setData(ITEM_KEY_CONTROL, QVariant());
    item_->setTransformations({});
    item_ = nullptr;
    realItem_ = nullptr;
}

QGraphicsItem *WhiteCanvasControl::create(ResourceView *res)
{
    (void) res;
    return nullptr;
}

void WhiteCanvasControl::resize(const QSizeF &size)
{
    QRectF rect(QPointF(0, 0), size);
    rect.moveCenter(QPointF(0, 0));
    rect |= item_->boundingRect();
    static_cast<WhiteCanvas*>(item_)->setGeometry(rect);
}

void WhiteCanvasControl::updateTransform()
{
    ResourceTransform & transform = res_->transform();
    QRectF srect = item_->scene()->sceneRect();
    QRectF crect = item_->boundingRect();
    transform.keepOuterOf(srect, crect);
}
