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
}

WhiteCanvasControl::~WhiteCanvasControl()
{
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
    QRectF crect = transform.transform().map(item_->boundingRect()).boundingRect();
    qDebug() << "before" << srect << crect << transform.transform();
    if (srect.width() > crect.width() || srect.height() > crect.height()) {
        qreal s = qMax(srect.width() / crect.width(), srect.height() / crect.height());
        transform.scale({s, s});
        crect = transform.transform().map(item_->boundingRect()).boundingRect();
    }
    QPointF d;
    if (crect.left() > srect.left())
        d.setX(srect.left() - crect.left());
    else if (crect.right() < srect.right())
        d.setX(srect.right() - crect.right());
    if (crect.top() > srect.top())
        d.setY(srect.top() - crect.top());
    else if (crect.bottom() < srect.bottom())
        d.setY(srect.bottom() - crect.bottom());
    crect.translate(d.x(), d.y());
    transform.translate(d);
    qDebug() << "after" << srect << crect << transform.transform();
    Control::updateTransform();
}
