#include "control.h"
#include "resourceview.h"
#include "toolbutton.h"
#include "views/whitecanvas.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QTransform>
#include <QGraphicsTransform>
#include <QMetaMethod>

#include <map>

class QControlTransform : public QGraphicsTransform
{
public:
    QControlTransform (QTransform * t)
        : t_(t)
    {
    }

    virtual void applyTo(QMatrix4x4 *matrix) const override
    {
        *matrix = t_->toAffine();
    }

    using QGraphicsTransform::update;

private:
    QTransform * t_;
};

Control * Control::fromItem(QGraphicsItem * item)
{
    return item->data(ITEM_KEY_CONTROL).value<Control *>();
}

ToolButton Control::btnCopy = { "copy", "复制", ":/showboard/icons/icon_copy.png" };
ToolButton Control::btnDelete = { "delete", "删除", ":/showboard/icons/icon_delete.png" };

Control::Control(ResourceView *res, Flags flags, Flags clearFlags)
    : flags_((DefaultFlags | flags) & ~clearFlags)
    , res_(res)
{
    transform_ = new QControlTransform(res->transform());
    if (res_->flags() & ResourceView::SavedSession)
        flags_ |= RestoreSession;
}

Control::~Control()
{
    delete transform_;
    delete item_;
    item_ = nullptr;
    transform_ = nullptr;
    res_ = nullptr;
}

void Control::attachTo(QGraphicsItem * parent)
{
    item_ = create(res_);
    attaching();
    item_->setAcceptTouchEvents(true);
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    item_->setTransformations({transform_});
    item_->setParentItem(parent);
    loadSettings();
    relayout();
    attached();
}

void Control::detachFrom(QGraphicsItem *parent)
{
    detaching();
    saveSettings();
    (void) parent;
    item_->scene()->removeItem(item_);
    item_->setTransformations({});
    item_->setData(ITEM_KEY_CONTROL, QVariant());
    detached();
    //deleteLater();
    delete this;
}

void Control::relayout()
{
    if (flags_ & FullLayout) {
        layout(item_->parentItem()->boundingRect());
    }
}

void Control::attaching()
{
}

void Control::attached()
{
}

void Control::detaching()
{
}

void Control::detached()
{
}

void Control::loadSettings()
{
    for (QByteArray & k : res_->dynamicPropertyNames())
        setProperty(k, res_->property(k));
}

void Control::saveSettings()
{
    for (int i = Control::metaObject()->propertyCount(); i < metaObject()->propertyCount(); ++i) {
        QMetaProperty p = metaObject()->property(i);
        res_->setProperty(p.name(), p.read(this));
    }
    for (QByteArray & k : dynamicPropertyNames())
        res_->setProperty(k, property(k));
    res_->setSaved();
}

void Control::layout(QRectF const & rect)
{
    if (item_->type() == QGraphicsRectItem::Type) {
        static_cast<QGraphicsRectItem*>(item_)->setRect(rect);
    }
}

static constexpr qreal CROSS_LENGTH = 20;

Control::SelectMode Control::selectTest(QPointF const & point)
{
    if (flags_ & FullSelect)
        return Select;
    if ((flags_ & HelpSelect) == 0)
        return NotSelect;
    QRectF rect = item_->boundingRect();
    rect.adjust(CROSS_LENGTH, CROSS_LENGTH, -CROSS_LENGTH, -CROSS_LENGTH);
    return rect.contains(point) ? NotSelect : Select;
}

QString Control::toolsString() const
{
    return nullptr;
}

void Control::initScale(QSizeF size)
{
    if (item_->parentItem() == nullptr)
        return;
    QSizeF ps = item_->parentItem()->boundingRect().size();
    if (flags_ & CanvasBackground) {
        if (size.width() > ps.width() || size.height() > ps.height()) {
            if (size.width() < ps.width())
                size.setWidth(ps.width());
            if (size.height() < ps.height())
                size.setHeight(ps.height());
        }
        WhiteCanvas * canvas = static_cast<WhiteCanvas *>(item_->parentItem()->parentItem());
        QRectF rect(QPointF(0, 0), size);
        rect.moveCenter({0, 0});
        canvas->setGeometry(rect);
        return;
    }
    if (flags_ & (FullLayout | RestoreSession)) {
        return;
    }
    qreal scale = 1.0;
    size += QSizeF(20.0, 20.0);
    while (size.width() > ps.width() || size.height() > ps.height()) {
        size /= 2.0;
        scale /= 2.0;
    }
    //while (size.width() * 2.0 < ps.width() && size.height() * 2.0 < ps.height()) {
    //    size *= 2.
    //    scale *= 2.0;
    //}
    res_->transform()->scale(scale, scale);
    updateTransform();
}

void Control::move(QPointF const & delta)
{
    QTransform * t = res_->transform();
    t->translate(delta.x() / t->m11(), delta.y() / t->m22());
    updateTransform();
}

void Control::scale(QRectF const & origin, QRectF & result)
{
    //result = origin;
    //result.adjust(0, 0, origin.width() / -2, origin.height() / -2);
    QSizeF s1 = origin.size();
    QSizeF s2 = result.size();
    QSizeF s(s2.width() / s1.width(), s2.height() / s1.height());
    QPointF d = result.center() - origin.center();
    if (flags_ & KeepAspectRatio) {
        if (s.width() > s.height()) {
            s.setWidth(s.height());
            d.setX(d.y() * s1.width() / s1.height());
            result.setWidth(s1.width() * s.width());
        } else {
            s.setHeight(s.width());
            d.setY(d.x() * s1.height() / s1.width());
            result.setHeight(s1.height() * s.height());
        }
        result.moveCenter(origin.center() + d);
    } else {
        s2.scale(1.0 / s1.width(), 1.0 / s1.height(), Qt::IgnoreAspectRatio);
    }
    QTransform * t = res_->transform();
    QPointF p1 = item_->mapFromParent(origin.topLeft());
    t->scale(s.width(), s.height());
    p1 = item_->mapToParent(p1);
    QPointF p2 = result.topLeft();
    d = p2 - p1;
    t->translate(d.x() / t->m11(), d.y() / t->m22());
    updateTransform();
}

void Control::updateTransform()
{
    static_cast<QControlTransform *>(transform_)->update();
}

void Control::exec(QString const & cmd, QGenericArgument arg0,
                   QGenericArgument arg1, QGenericArgument arg2)
{
    int index = metaObject()->indexOfSlot(cmd.toUtf8());
    if (index < 0)
        return;
    QMetaMethod method = metaObject()->method(index);
    method.invoke(this, arg0, arg1, arg2);
}

void Control::getToolButtons(QList<ToolButton *> & buttons)
{
    if (res_->flags() & ResourceView::CanCopy)
        buttons.append(&btnCopy);
    if (res_->flags() & ResourceView::CanDelete)
        buttons.append(&btnDelete);
    buttons.append(tools());
}

void Control::handleToolButton(ToolButton *button)
{
    exec(button->name);
}

QList<ToolButton *> & Control::tools()
{
    static std::map<QMetaObject const *, QList<ToolButton *>> slist;
    auto iter = slist.find(metaObject());
    if (iter == slist.end()) {
        QList<ToolButton *> list;
        QString tools = this->toolsString();
        QStringList descs = tools.split(";", QString::SkipEmptyParts);
        for (QString desc : descs) {
            QStringList seps = desc.split("|");
            if (seps.size() >= 1) {
                list.append(new ToolButton{
                                seps[0],
                                seps.size() > 1 ? seps[1] : seps[0],
                                seps.size() > 2 ? QVariant(seps[2]) : QVariant()
                            });
            }
        }
        iter = slist.insert(std::make_pair(metaObject(), std::move(list))).first;
    }
    return iter->second;
}
