#include "control.h"
#include "resourceview.h"
#include "toolbutton.h"
#include "views/whitecanvas.h"
#include "views/stateitem.h"
#include "views/selectbar.h"
#include "core/transformhelper.h"
#include "core/controltransform.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QTransform>
#include <QGraphicsTransform>
#include <QMetaMethod>

#include <map>

Control * Control::fromItem(QGraphicsItem * item)
{
    return item->data(ITEM_KEY_CONTROL).value<Control *>();
}

ToolButton Control::btnCopy = { "copy", "复制", ":/showboard/icons/icon_copy.png" };
ToolButton Control::btnDelete = { "delete", "删除", ":/showboard/icons/icon_delete.png" };

Control::Control(ResourceView *res, Flags flags, Flags clearFlags)
    : flags_((DefaultFlags | flags) & ~clearFlags)
    , res_(res)
    , transform_(nullptr)
    , item_(nullptr)
    , realItem_(nullptr)
    , stateItem_(nullptr)
{
    if (!(flags_ & SelfTransform))
        transform_ = new ControlTransform(res->transform());
    if (res_->flags() & ResourceView::SavedSession)
        flags_ |= RestoreSession;
}

Control::~Control()
{
    if (transform_)
        delete transform_;
    delete realItem_;
    realItem_ = nullptr;
    item_ = nullptr;
    transform_ = nullptr;
    res_ = nullptr;
}

void Control::attachTo(QGraphicsItem * parent)
{
    item_ = create(res_);
    if (transform_)
        item_->setTransformations({transform_});
    if (flags_ & WithSelectBar) {
        realItem_ = new SelectBar(item_);
        transform_ = new ControlTransform(
                    static_cast<ControlTransform*>(transform_));
    } else {
        realItem_ = item_;
    }
    attaching();
    item_->setAcceptTouchEvents(true);
    item_->setFlag(QGraphicsItem::ItemIsFocusable);
    realItem_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    if (realItem_ != item_)
        realItem_->setTransformations({transform_});
    realItem_->setParentItem(parent);
    loadSettings();
    initPosition();
    relayout();
    attached();
    if (!(flags_ & LoadFinished)) {
        stateItem()->setLoading();
    }
}

void Control::detachFrom(QGraphicsItem *parent)
{
    detaching();
    saveSettings();
    (void) parent;
    realItem_->scene()->removeItem(realItem_);
    realItem_->setTransformations({});
    realItem_->setData(ITEM_KEY_CONTROL, QVariant());
    detached();
    //deleteLater();
    delete this;
}

void Control::relayout()
{
    if (flags_ & FullLayout) {
        resize(realItem_->parentItem()->boundingRect().size());
    } else {

    }
}

void Control::attaching()
{
}

void Control::attached()
{
    loadFinished(true);
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
    for (int i = Control::metaObject()->propertyCount();
            i < metaObject()->propertyCount(); ++i) {
        QMetaProperty p = metaObject()->property(i);
        res_->setProperty(p.name(), p.read(this));
    }
    // special one
    res_->setProperty("sizeHint", sizeHint());
    for (QByteArray & k : dynamicPropertyNames())
        res_->setProperty(k, property(k));
    res_->setSaved();
}

QSizeF Control::sizeHint()
{
    return item_->boundingRect().size();
}

// called before attached
void Control::setSizeHint(QSizeF const & size)
{
    if (size.width() < 10.0) {
        QRectF rect = realItem_->parentItem()->boundingRect();
        resize(QSizeF(rect.width() * size.width(), rect.height() * size.height()));
    } else {
        resize(size);
    }
}

void Control::resize(QSizeF const & size)
{
    if (item_->type() == QGraphicsRectItem::Type) {
        QRectF rect(QPointF(0, 0), size);
        rect.moveCenter({0, 0});
        static_cast<QGraphicsRectItem*>(realItem_)->setRect(rect);
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

static qreal polygonArea(QPolygonF const & p)
{
    qreal area = 0;
    int j = 0;
    for (int i = 1; i < p.size(); ++i) {
        area += (p[j].x() + p[i].x()) * (p[j].y() - p[i].y());
        j = i;
    }
    return qAbs(area) / 2.0;
}

void Control::initPosition()
{
    if (realItem_ != item_)
        static_cast<SelectBar *>(realItem_)->updateRect();
    if (flags_ & (FullLayout | RestoreSession | PositionAtCenter))
        return;
    QGraphicsItem *parent = realItem_->parentItem();
    QPolygonF polygon;
    for (QGraphicsItem * c : parent->childItems()) {
        if (c == realItem_ || Control::fromItem(c)->flags() & FullLayout)
            continue;
        polygon = polygon.united(c->mapToParent(c->boundingRect()));
    }
    QRectF rect = parent->boundingRect();
    qreal dx = rect.width() / 3.0;
    qreal dy = rect.height() / 3.0;
    rect.adjust(0, 0, -dx - dx, -dy - dy);
    qreal minArea = dx * dy;
    QPointF pos;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            qreal area = polygonArea(polygon.intersected(rect));
            if (qFuzzyIsNull(minArea - area) && i == 1 && j == 1) {
                pos = rect.center();
            } else if (minArea > area) {
                minArea = area;
                pos = rect.center();
            }
            rect.adjust(dx, 0, dx, 0);
        }
        qreal dx3 = -dx * 3;
        rect.adjust(dx3, dy, dx3, dy);
    }
    res_->transform()->translate(pos.x(), pos.y());
}

void Control::loadFinished(bool ok, QString const & iconOrMsg)
{
    if (ok) {
        if (iconOrMsg.isNull()) {
            if (stateItem_) {
                delete stateItem_;
                stateItem_ = nullptr;
            }
        } else {
            stateItem()->setLoaded(iconOrMsg);
        }
        initScale();
        flags_ |= LoadFinished;
    } else {
        stateItem()->setFailed(iconOrMsg);
    }
}

void Control::initScale()
{
    if (realItem_ != item_)
        static_cast<SelectBar *>(realItem_)->updateRect();
    QSizeF ps = realItem_->parentItem()->boundingRect().size();
    QSizeF size = item_->boundingRect().size();
    if (flags_ & CanvasBackground) {
        if (size.width() > ps.width() || size.height() > ps.height()) {
            if (size.width() < ps.width())
                size.setWidth(ps.width());
            if (size.height() < ps.height())
                size.setHeight(ps.height());
        }
        WhiteCanvas * canvas = static_cast<WhiteCanvas *>(
                    realItem_->parentItem()->parentItem());
        QRectF rect(QPointF(0, 0), size);
        rect.moveCenter({0, 0});
        canvas->setGeometry(rect);
        return;
    }
    QVariant sizeHint = res_->property("sizeHint");
    if (sizeHint.isValid()) {
        QSizeF sh = sizeHint.toSizeF();
        if (sh.width() < 10.0) {
            sh = QSizeF(ps.width() * sh.width(), ps.height() * sh.height());
        }
        ps = sh;
    }
    if (flags_ & RestoreSession) {
        resize(ps);
        if (realItem_ != item_)
            static_cast<SelectBar *>(realItem_)->updateRect();
        return;
    }
    if (flags_ & (FullLayout | LoadFinished)) {
        return;
    }
    qreal scale = 1.0;
    while (size.width() > ps.width() || size.height() > ps.height()) {
        size /= 2.0;
        scale /= 2.0;
    }
    //while (size.width() * 2.0 < ps.width() && size.height() * 2.0 < ps.height()) {
    //    size *= 2.
    //    scale *= 2.0;
    //}
    QTransform * t = res_->transform();
    t->scale(scale / t->m11(), scale / t->m22());
    updateTransform();
    if (stateItem_) {
        stateItem_->updateTransform();
    }
}

void Control::move(QPointF const & delta)
{
    QTransform * t = res_->transform();
    t->translate(delta.x() / t->m11(), delta.y() / t->m22());
    updateTransform();
}

void Control::scale(QRectF const & origin, QRectF const & direction,
                    QPointF const & diff, QRectF & result)
{
    //result = origin;
    //result.adjust(0, 0, origin.width() / -2, origin.height() / -2);
    qDebug() << origin << " -> " << result;
    bool positive = qFuzzyIsNull(direction.left() - direction.top());
    bool byWidth = qFuzzyIsNull(direction.top()) && qFuzzyIsNull(direction.height());
    bool byHeight = qFuzzyIsNull(direction.left()) && qFuzzyIsNull(direction.width());
    result = origin.adjusted(
                diff.x() * direction.left(), diff.y() * direction.top(),
                diff.x() * direction.width(), diff.y() * direction.height());
    QPointF c0 = result.center();
    QPointF d0 = c0 - origin.center();
    if (item_ != realItem_) {
        static_cast<SelectBar *>(realItem_)->updateRectToChild(result);
        c0 = result.center();
    }
    QSizeF s1 = item_->boundingRect().size();
    QSizeF s2 = result.size();
    QSizeF s(s2.width() / s1.width(), s2.height() / s1.height());
    QPointF d = d0;
    if (flags_ & KeepAspectRatio) {
        qreal sign = positive ? 1.0 : -1.0;
        if (byHeight || (s.width() < s.height() && !byWidth)) {
            d.setX(d.y() * sign * s1.width() / s1.height());
            //d.setX(d.x() * s.height() / s.width());
            result.setWidth(s1.width() * s.height());
        } else {
            d.setY(d.x() * sign * s1.height() / s1.width());
            //d.setY(d.y() * s.width() / s.height());
            result.setHeight(s1.height() * s.width());
        }
        result.moveCenter(c0 - d0 + d);
    } else {
        s2.scale(1.0 / s1.width(), 1.0 / s1.height(), Qt::IgnoreAspectRatio);
    }
    if (result.height() < 120) {
        result = origin;
        return;
    }
    if (flags_ & LayoutScale) {
        resize(result.size());
    }
    QTransform * t = res_->transform();
    TransformHelper::apply(*t, item_, result.normalized(), 0.0);
    if (item_ != realItem_) {
        static_cast<SelectBar *>(realItem_)->updateRectFromChild(result);
        qDebug() << "updateRectFromChild" << result;
    }
    if (stateItem_)
        stateItem_->updateTransform();
    updateTransform();
}

void Control::select(bool selected)
{
    if (realItem_ != item_)
        static_cast<SelectBar *>(realItem_)->setSelected(selected);
}

StateItem * Control::stateItem()
{
    if (stateItem_)
        return stateItem_;
    stateItem_ = new StateItem(item_);
    stateItem_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    return stateItem_;
}

void Control::updateTransform()
{
    static_cast<ControlTransform *>(transform_)->update();
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
