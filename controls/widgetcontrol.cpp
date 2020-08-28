#include "widgetcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QWidget>

WidgetControl::WidgetControl(ResourceView *res, Flags flags, Flags clearFlags)
    : Control(res, flags, clearFlags)
    , widget_(nullptr)
{
}

WidgetControl::~WidgetControl()
{
    if (widget_)
        delete widget_;
    widget_ = nullptr;
}

bool WidgetControl::touchable() const
{
    return flags_.testFlag(Touchable);
}

void WidgetControl::setTouchable(bool b)
{
    flags_.setFlag(Touchable, b);
    item_->setAcceptTouchEvents(b);
}

QWidget *WidgetControl::widget()
{
    return widget_;
}

QGraphicsItem * WidgetControl::create(ResourceView *res)
{
    res_ = res;
    widget_ = createWidget(res);
    QGraphicsProxyWidget * item = new QGraphicsProxyWidget();
    item->setData(100000001,res->property("through").toInt());
    if (flags_.testFlag(Touchable))
        item->setAcceptTouchEvents(true);
    item->setAutoFillBackground(false);
    item->setWidget(widget_);
    //resize(widget_->size());
    return item;
}

void WidgetControl::attaching()
{
    if (!widget_) // restore from persist session
        widget_ = static_cast<QGraphicsProxyWidget*>(item_)->widget();
    itemObj_ = widget_;
    widget_->installEventFilter(this);
}

void WidgetControl::detached()
{
    widget_->removeEventFilter(this);
    if (res_->flags().testFlag(ResourceView::PersistSession))
        widget_ = nullptr;
    else
        static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
}

bool WidgetControl::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == widget_ && event->type() == QEvent::Resize) {
        QtPromise::resolve().then([l = life(), this]() {
            if (!l.isNull() && !flags_.testFlag(Adjusting))
                sizeChanged();
        });
    }
    return false;
}

void WidgetControl::resize(QSizeF const & size)
{
    QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
    item->resize(size);
}
