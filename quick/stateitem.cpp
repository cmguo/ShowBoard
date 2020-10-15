#include "stateitem.h"
#include "data/svgcache.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "widget/qsshelper.h"

#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QDebug>
#include <QCloseEvent>
#include <QCursor>
#include <QMovie>

StateItem::StateItem(QQuickItem * parent)
    : QQuickItem(parent)
    , state_(None)
    , showBackground_(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);
    setCursor(Qt::SizeAllCursor);
    updateTransform();
}

void StateItem::setLoading()
{
    if (state_ != Loading)
        setLoading("正在打开");
}

void StateItem::setLoading(const QString &msg)
{
    (void) msg;
    if (state_ != Loading) {
        state_ = Loading;
    }
}

void StateItem::setLoaded(const QString &icon)
{
    state_ = Loaded;
    QString fileNormal(icon);
    fileNormal.replace(".svg", ".normal.svg");
    QString fileHover(icon);
    fileHover.replace(".svg", ".hover.svg");
    QString filePressed(icon);
    filePressed.replace(".svg", ".press.svg");
}

void StateItem::setFailed(QString const & error)
{
    QByteArray type = "unknown";
    QString errmsg = error;
    int n = error.indexOf("|");
    if (n > 0) {
        type = error.left(n).toUtf8();
        errmsg = error.mid(n + 1);
    }
    state_ = Failed;
    bool retry = true;
    Control * control = Control::fromItem(this);
    if (control->resource()->resource()->type().endsWith("tool"))
        retry = false;
}

void StateItem::updateTransform() {

}

bool StateItem::hitTest(QQuickItem *child, const QPointF &pt)
{
    (void) child;
    (void) pt;
    return false;
}

QRectF StateItem::boundingRect() const
{
    return rect_;
}

