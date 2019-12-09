#include "control.h"
#include "resourceview.h"
#include "toolbutton.h"
#include "views/whitecanvas.h"
#include "views/stateitem.h"
#include "views/itemframe.h"
#include "views/itemselector.h"
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

ToolButton Control::btnCopy = { "copy", "复制", nullptr, ":/showboard/icons/copy.svg" };
ToolButton Control::btnDelete = { "delete", "删除", nullptr, ":/showboard/icons/delete.svg" };

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
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    realItem_ = item_;
    if (flags_ & WithSelectBar) {
        itemFrame()->addTopBar();
    }
    attaching();
    item_->setAcceptTouchEvents(true);
    item_->setFlag(QGraphicsItem::ItemIsFocusable);
    realItem_->setParentItem(parent);
    loadSettings();
    sizeChanged();
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
        sizeChanged();
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

void Control::sizeChanged()
{
    QRectF rect = item_->boundingRect();
    QPointF center(rect.center());
    if (flags_ & LoadFinished) {
        QTransform t = item_->transform();
        center = t.map(center);
        move(center);
        t.translate(-center.x(), -center.y());
        item_->setTransform(t);
        if (realItem_ != item_)
            static_cast<ItemFrame *>(realItem_)->updateRect();
    } else {
        item_->setTransform(QTransform::fromTranslate(-center.x(), -center.y()));
    }
    WhiteCanvas * canvas = static_cast<WhiteCanvas *>(
                realItem_->parentItem()->parentItem());
    if (canvas->selector()->selected() == realItem_)
        canvas->selector()->updateSelect();
}

QSizeF Control::sizeHint()
{
    return item_->boundingRect().size();
}

static void adjustSizeHint(QSizeF & size, QSizeF const & psize)
{
    if (size.width() < 10.0) {
        size.setWidth(psize.width() * size.width());
    }
    if (size.height() < 0)
        size.setHeight(size.width() * -size.height());
    else if (size.height() < 10.0)
        size.setHeight(psize.height() * size.height());
}

// called before attached
void Control::setSizeHint(QSizeF const & size)
{
    if (size.width() < 10.0 || size.height() < 10.0) {
        QSizeF size2 = size;
        QRectF rect = realItem_->parentItem()->boundingRect();
        adjustSizeHint(size2, rect.size());
        resize(size2);
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

QString Control::toolsString(QString const & parent) const
{
    (void) parent;
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
        static_cast<ItemFrame *>(realItem_)->updateRect();
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
        sizeChanged();
        initScale();
        flags_ |= LoadFinished;
    } else {
        stateItem()->setFailed(iconOrMsg);
    }
}

void Control::initScale()
{
    if (realItem_ != item_)
        static_cast<ItemFrame *>(realItem_)->updateRect();
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
    if (flags_ & RestoreSession) {
        QVariant sizeHint = res_->property("sizeHint");
        if (sizeHint.isValid()) {
            QSizeF sh = sizeHint.toSizeF();
            if (sh.width() < 10.0 || sh.height() < 10.0) {
                adjustSizeHint(sh, ps);
            }
            resize(sh);
            if (realItem_ != item_)
                static_cast<ItemFrame *>(realItem_)->updateRect();
        }
        return;
    }
    if (flags_ & (FullLayout | LoadFinished)) {
        return;
    }
    if (item_ != realItem_) {
        QRectF rect(static_cast<ItemFrame *>(realItem_)->padding());
        ps.setWidth(ps.height() - rect.width());
        ps.setHeight(ps.height() - rect.height());
    }
    qreal scale = 1.0;
    while (size.width() > ps.width() || size.height() > ps.height()) {
        size /= 2.0;
        scale /= 2.0;
    }
    if (flags_ & ExpandScale) {
        while (size.width() * 2.0 < ps.width() && size.height() * 2.0 < ps.height()) {
            size *= 2.0;
            scale *= 2.0;
        }
    }
    QTransform * t = res_->transform();
    t->scale(scale / t->m11(), scale / t->m22());
    updateTransform();
    if (realItem_ != item_)
        static_cast<ItemFrame *>(realItem_)->updateRect();
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
        static_cast<ItemFrame *>(realItem_)->updateRectToChild(result);
        if (result.width() < 0 || result.height() < 0) {
            result = origin;
            static_cast<ItemFrame *>(realItem_)->setRect(origin);
            return;
        }
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
        if (item_ != realItem_) {
            static_cast<ItemFrame *>(realItem_)->setRect(origin);
        }
        return;
    }
    if (flags_ & LayoutScale) {
        resize(result.size());
        sizeChanged();
    }
    QTransform * t = res_->transform();
    TransformHelper::apply(*t, item_, result.normalized(), 0.0);
    if (item_ != realItem_) {
        static_cast<ItemFrame *>(realItem_)->updateRectFromChild(result);
        qDebug() << "updateRectFromChild" << result;
    }
    if (stateItem_)
        stateItem_->updateTransform();
    updateTransform();
}

void Control::select(bool selected)
{
    if (realItem_ != item_)
        static_cast<ItemFrame *>(realItem_)->setSelected(selected);
}

ItemFrame * Control::itemFrame()
{
    if (item_ != realItem_) {
        return static_cast<ItemFrame*>(realItem_);
    }
    ItemFrame * frame = new ItemFrame(item_);
    realItem_ = frame;
    transform_ = new ControlTransform(
                static_cast<ControlTransform*>(transform_));
    realItem_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    realItem_->setTransformations({transform_});
    return frame;
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
    method.parameterType(index);
    method.invoke(this, arg0, arg1, arg2);
}

void Control::exec(QString const & cmd, QStringList const & args)
{
    int index = metaObject()->indexOfSlot(cmd.toUtf8());
    if (index < 0) {
        if (args.size() == 1) {
            setProperty(cmd.toUtf8(), args[0]);
            res_->setProperty(cmd.toUtf8(), args[0]);
        }
        return;
    }
    QMetaMethod method = metaObject()->method(index);
    if (method.parameterCount() >= 4)
        return;
    QGenericArgument argv[4];
    QVariant varg[4];
    for (int i = 0; i < method.parameterCount(); ++i) {
        if (i < args.size())
            varg[i] = args[i];
        int t = method.parameterType(i);
        if (!varg[i].canConvert(t))
            return;
        varg[i].convert(t);
        argv[i] = QGenericArgument(QMetaType::typeName(t), varg[i].data());
    }
    method.invoke(this, argv[0], argv[1], argv[2], argv[3]);
}

void Control::getToolButtons(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents)
{
    if (parents.isEmpty()) {
        buttons.append(tools());
        if (res_->flags() & ResourceView::CanCopy)
            buttons.append(&btnCopy);
        if (res_->flags() & ResourceView::CanDelete)
            buttons.append(&btnDelete);
    } else {
        buttons.append(tools(parents.last()->name));
    }
}

void Control::handleToolButton(QList<ToolButton *> const & buttons)
{
    ToolButton * button = buttons.back();
    int i = 0;
    for (; i < buttons.size(); ++i) {
        if (buttons[i]->flags & ToolButton::OptionsGroup) {
            button = buttons[i];
            break;
        }
    }
    if (button->flags & ToolButton::HideSelector) {
        WhiteCanvas * canvas = static_cast<WhiteCanvas *>(
                    realItem_->parentItem()->parentItem());
        if (canvas->selector()->selected() == realItem_)
            canvas->selector()->selectImplied(realItem_);
    }
    QStringList args;
    for (++i; i < buttons.size(); ++i) {
        args.append(buttons[i]->name);
    }
    exec(button->name, args);
    if (button->flags & ToolButton::NeedUpdate) {
        updateToolButton(button);
    }
}

void Control::updateToolButton(ToolButton *button)
{
    (void) button;
}

QList<ToolButton *> & Control::tools(QString const & parent)
{
    static std::map<QMetaObject const *, std::map<QString, QList<ToolButton *>>> slist;
    auto iter = slist.find(metaObject());
    if (iter == slist.end()) {
        std::map<QString, QList<ToolButton *>> t;
        iter = slist.insert(std::make_pair(metaObject(), std::move(t))).first;
    }
    auto iter2 = iter->second.find(parent);
    if (iter2 == iter->second.end()) {
        QString tools = this->toolsString(parent);
        iter2 = iter->second.insert(
                    std::make_pair(parent, ToolButton::makeButtons(tools))).first;
    }
    for (ToolButton * button : iter2->second) {
        if (button->flags & ToolButton::NeedUpdate) {
            updateToolButton(button);
        }
    }
    return iter2->second;
}
