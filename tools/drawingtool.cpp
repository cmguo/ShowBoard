#include "drawingtool.h"
#include "views/whitecanvas.h"
#include "views/qsshelper.h"
#include "views/toolbarwidget.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"

#include <QPen>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

static QssHelper QSS(":/showboard/qss/draw_finish.qss");

DrawingTool::DrawingTool(ResourceView *res)
    : Control(res, FullLayout, {DefaultFlags})
{
}

QColor DrawingTool::color() const
{
    return newSettings_.value("color", "write").value<QColor>();
}

void DrawingTool::setColor(QColor color)
{
    newSettings_.insert("color", color);
}

qreal DrawingTool::width() const
{
    return newSettings_.value("width", "1.0").toReal();
}

void DrawingTool::setWidth(qreal width)
{
    newSettings_.insert("width", width);
}

Control * DrawingTool::newControl()
{
    Control * control = whiteCanvas()->addResource(newUrl_, newSettings_);
    emit controlCreated(control);
    return control;
}

void DrawingTool::removeControl(Control *control)
{
    whiteCanvas()->removeResource(control);
}

void DrawingTool::finishControl(Control * control)
{
    whiteCanvas()->hideToolControl(this);
    emit drawFinished(control);
}

bool DrawingTool::translucent() const
{
    return item_->flags().testFlag(CanvasItem::ItemHasNoContents);
}

void DrawingTool::setTranslucent(bool on)
{
    item_->setFlag(CanvasItem::ItemHasNoContents, !on);
}

class DrawingItem : public CanvasItem
{
public:
    DrawingItem()
        : control_(nullptr)
    {
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(true);
        setAcceptTouchEvents(true);
        setFlag(ItemIsFocusable);

        QFrameEx* widget = new QFrameEx;
        widget->setObjectName("finishwidget");
        widget->setWindowFlag(Qt::FramelessWindowHint);
        widget->setStyleSheet(QSS);
        QLayout* layout = new QHBoxLayout(widget);
        widget->setLayout(layout);
        layout->setSpacing(20);
        layout->addWidget(new QLabel("闭合顶点或点击完成"));
        QPushButton * button = new QPushButton;
        button->setText(" 完成 ");
        QObject::connect(button, &QPushButton::clicked, [this]() {
            finish();
        });
        layout->addWidget(button);
        QGraphicsProxyWidget * item = new QGraphicsProxyWidget(this);
        item->setWidget(widget);
        item->hide();
        item->setFlag(ItemIsFocusable, false);
        finishItem_ = item;
    }

private:
    void finish()
    {
        DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
        Control* control = control_;
        if (control_) {
            if (!(control_->resource()->flags() & ResourceView::DrawFinised)) {
                QEvent event(QEvent::User);
                control_->event(&event);
                if (!(control_->resource()->flags() & ResourceView::DrawFinised)) {
                    qDebug() << "DrawItem: draw dropped";
                    tool->removeControl(control_);
                    control = nullptr;
                }
            }
            control_ = nullptr;
            finish_ = true;
            finishItem_->hide();
            tool->finishControl(control);
        } else if (!finish_) {
            qDebug() << "DrawItem: draw canceled";
            finish_ = true;
            tool->finishControl(control);
        }
    }

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        QBrush old = painter->brush();
        painter->setBrush(QColor("#01000000"));
        painter->drawRect(rect());
        painter->setBrush(old);
    }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (!control_) {
            DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
            control_ = tool->newControl();
            // large canvas may change scale and offset, reset it
            control_->resource()->transform().translateTo({0, 0});
            control_->resource()->transform().scaleTo(1.0);
        }
        control_->event(event);
    }

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (control_)
            control_->event(event);
    }

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (control_) {
            control_->event(event);
            if (control_->resource()->flags() & ResourceView::DrawFinised) {
                finish();
            } else if (event->flags() & 256) {
                finishItem_->setPos(event->pos() + QPointF(20, 20));
                finishItem_->show();
            }
        }
    }

    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override
    {
        if (control_)
            control_->event(event);
    }

    virtual bool sceneEvent(QEvent * event) override
    {
        if (event->type() == QEvent::FocusOut
                || event->type() == QEvent::WindowDeactivate) {
            if (!inSetFocus_)
                finish();
        } else if (event->type() == QEvent::TouchBegin) {
            event->accept();
            return true;
        }
        return CanvasItem::sceneEvent(event);
    }

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &variant) override
    {
        if (change == ItemVisibleHasChanged && variant.toBool()) {
            qDebug() << "DrawItem: start draw";
            inSetFocus_ = true;
            scene()->views().first()->setFocus();
            inSetFocus_ = false;
            setFocus();
            finish_ = false;
        }
        return CanvasItem::itemChange(change, variant);
    }

private:
    Control * control_;
    bool finish_ = false;
    bool inSetFocus_ = false;
    QGraphicsItem * finishItem_;
};

QGraphicsItem * DrawingTool::create(ResourceView *res)
{
    (void) res;
    return new DrawingItem();
}

void DrawingTool::resize(const QSizeF &size)
{
    QPointF origin(0, 0);
    QRectF rect(origin, size);
    rect.moveCenter(origin);
    static_cast<DrawingItem*>(item_)->setRect(rect);
}
