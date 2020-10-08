#include "widgetcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QWidget>
#include <QKeyEvent>
#include <QMetaEnum>

WidgetControl::WidgetControl(ResourceView *res, Flags flags, Flags clearFlags)
    : Control(res, flags, clearFlags)
    , widget_(nullptr)
{
    if (!QMetaType::hasRegisteredConverterFunction<QStringList, QList<Qt::Key>>())
        QMetaType::registerConverter<QStringList, QList<Qt::Key>>([] (QStringList list) {
            QList<Qt::Key> l;
            for (auto & s : list) {
                l.append(QVariant(s).value<Qt::Key>());
            }
            return l;
        });
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

void WidgetControl::setOverrideShotcuts(const QList<Qt::Key> &keys)
{
    overrideShotcuts_ = keys;
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
    if (watched != widget_)
        return false;
    if (event->type() == QEvent::Resize) {
        QtPromise::resolve().then([l = life(), this]() {
            if (!l.isNull() && !flags_.testFlag(Adjusting))
                sizeChanged();
        });
    } else if (event->type() == QEvent::ShortcutOverride) {
        if (overrideShotcuts_.contains(static_cast<Qt::Key>(static_cast<QKeyEvent*>(event)->key()))) {
            event->accept();
            return true;
        }
    }
    return false;
}

void WidgetControl::resize(QSizeF const & size)
{
    QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
    item->resize(size);
}
