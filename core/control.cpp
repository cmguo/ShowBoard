#include "control.h"
#include "resource.h"
#include "resourceview.h"
#include "toolbutton.h"
#include "views/whitecanvas.h"
#include "views/stateitem.h"
#include "views/itemframe.h"
#include "views/itemselector.h"
#include "views/qsshelper.h"
#include "resourcetransform.h"
#include "controltransform.h"
#include "resourcepage.h"
#include "resourcerecord.h"
#include "resourcepackage.h"
#include "varianthelper.h"

#ifdef SHOWBOARD_QUICK
#include <QQuickItem>
#include <QQuickTransform>
#else
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsTransform>
#endif
#include <QWidget>
#include <QTransform>
#include <QMetaMethod>
#include <QApplication>
#include <QScreen>
#include <QMimeData>
#include <QTimeLine>

#include <map>

#include <math.h>

static qreal MIN_SIZE = 120.0;
static qreal MAX_SIZE = 4320.0;

Control * Control::fromItem(ControlView const * item)
{
#ifdef SHOWBOARD_QUICK
    return item->property(ITEM_KEY_CONTROL).value<Control *>();
#else
    return item->data(ITEM_KEY_CONTROL).value<Control *>();
#endif
}

void Control::attachToItem(ControlView * item, Control * control)
{
#ifdef SHOWBOARD_QUICK
    item->setProperty(Control::ITEM_KEY_CONTROL, QVariant::fromValue(control));
#else
    item->setData(ITEM_KEY_CONTROL, QVariant::fromValue(control));
#endif

}

static ToolButton btnTop = { "top", "置顶", ToolButton::Static, ":/showboard/icon/top.svg" };
static ToolButton btnCopy = { "copy", "复制", ToolButton::Static, ":/showboard/icon/copy.svg" };
static ToolButton btnFastCopy = { "copy", "快速复制",
                                    ToolButton::Flags{ToolButton::Static, ToolButton::Checkable},
                                    ":/showboard/icon/copy.svg" };
static ToolButton btnDelete = { "delete", "删除", ToolButton::Static, ":/showboard/icon/delete.svg" };

Control::Control(ResourceView *res, Flags flags, Flags clearFlags)
    : flags_((DefaultFlags | flags) & ~clearFlags)
    , res_(res)
    , transform_(nullptr)
    , item_(nullptr)
    , itemObj_(nullptr)
    , realItem_(nullptr)
    , stateItem_(nullptr)
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        //flags_.setFlag(FullLayout, true);
        flags_.setFlag(CanSelect, false);
        flags_.setFlag(CanRotate, false);
    } else if (res_->flags().testFlag(ResourceView::Independent)) {
        flags_.setFlag(DefaultFlags, false);
    }
    if (flags_.testFlag(FullLayout)) {
        flags_.setFlag(FixedOnCanvas, false);
    }
    connect(&res->transform(), &ResourceTransform::changed, this, &Control::updateTransform);
    if (res_->flags() & ResourceView::SavedSession) {
        flags_ |= RestoreSession;
    }
    minMaxSize_[0] = {dp(MIN_SIZE), dp(MIN_SIZE)};
    minMaxSize_[1] = {dp(MAX_SIZE), dp(MAX_SIZE)};
    attachSubProvider(res_, true);
}

Control::~Control()
{
    attachSubProvider(nullptr, true);
    res_ = nullptr;
}

bool Control::withSelectBar() const
{
    return flags_.testFlag(WithSelectBar);
}

void Control::setWithSelectBar(bool b)
{
    flags_.setFlag(WithSelectBar, b);
}

static constexpr Control::Flags ScaleModeFlags = Control::CanScale | Control::FullLayout
        | Control::KeepAspectRatio | Control::LayoutScale | Control::ExpandScale;

Control::Flags Control::scaleMode() const
{
    return flags_ & ScaleModeFlags;
}

void Control::setScaleMode(Flags mode)
{
    flags_ = (flags_ & ~ScaleModeFlags) | (mode & ScaleModeFlags);
}

static constexpr Control::Flags SelectModeFlags = Control::CanSelect
        | Control::FullSelect | Control::HalfSelect | Control::ShowSelectMask;

Control::Flags Control::selectMode() const
{
    return flags_ & SelectModeFlags;
}

void Control::setSelectMode(Flags mode)
{
    flags_ = (flags_ & ~SelectModeFlags) | (mode & SelectModeFlags);
}

bool Control::keepAspectRatio() const
{
    return flags_.testFlag(KeepAspectRatio);
}

void Control::setKeepAspectRatio(bool b)
{
    flags_.setFlag(KeepAspectRatio, b);
}

bool Control::layoutScale() const
{
    return flags_.testFlag(LayoutScale);
}

void Control::setLayoutScale(bool b)
{
    flags_.setFlag(LayoutScale, b);
}

bool Control::expandScale() const
{
    return flags_.testFlag(ExpandScale);
}

void Control::setExpandScale(bool b)
{
    flags_.setFlag(ExpandScale, b);
}

bool Control::enterAnimate() const
{
    return flags_.testFlag(EnterAnimate);
}

void Control::setEnterAnimate(bool b)
{
    flags_.setFlag(EnterAnimate, b);
}

bool Control::selectOnLoaded() const
{
    return flags_.testFlag(SelectOnLoaded);
}

void Control::setSelectOnLoaded(bool b)
{
    flags_.setFlag(SelectOnLoaded, b);
}

void Control::attachTo(ControlView * parent, ControlView * before)
{
    bool fromPersist = false;
    if (res_->flags().testFlag(ResourceView::PersistSession))
        item_ = res_->loadSession();
    if (item_ == nullptr) {
        item_ = create(parent);
        if (flags_.testFlag(FullSelect) || flags_.testFlag(HalfSelect))
            item_->setCursor(Qt::SizeAllCursor);
    } else {
        fromPersist = true;
    }
    if (fromPersist)
        flags_.setFlag(RestorePersisted);
    realItem_ = item_;
    transform_ = new ControlTransform(res_->transform(), flags_.testFlag(LayoutScale)
                                      ? ControlTransform::RotateTranslate
                                      : ControlTransform::PureItem);
    transform_->prependToItem(item_);
    Control * canvasControl = fromItem(parent->parentItem());
    if (canvasControl && flags_.testFlag(FixedOnCanvas)) {
        ControlTransform * t = new ControlTransform(
                    static_cast<ControlTransform*>(canvasControl->transform_), true, true, true);
        t->appendToItem(realItem_);
    }
    attachToItem(item_, this);
#ifdef SHOWBOARD_QUICK
    itemObj_ = item_;
#else
    itemObj_ = item_->toGraphicsObject();
#endif
    attaching();
    QVariant withSelectBar = res_->property("withSelectBar");
    if (flags_.testFlag(WithSelectBar) && (!withSelectBar.isValid() || withSelectBar.toBool())) {
        itemFrame()->addTopBar();
    }
    realItem_->setParentItem(parent);
    if (before)
        realItem_->stackBefore(before);
    loadSettings();
    sizeChanged();
    initPosition();
    relayout();
    flags_ |= Loading;
#ifdef PROD_TEST
    setParent(whiteCanvas()); // for testbed
#endif
    whiteCanvas()->onControlLoad(true);
    attached();
    if (res_->flags().testFlag(ResourceView::Independent))
        canvasControl->attachSubProvider(this);
    if (flags_ & Loading) {
        QTimer::singleShot(500, this, [this] {
            if (flags_ & Loading)
                stateItem()->setLoading();
        });
    } else if (!(flags_ & RestoreSession)) {
        QTimer::singleShot(0, this, [this] {
            if (flags_ & SelectOnLoaded)
                whiteCanvas()->selector()->select(this);
        });
    }
}

void Control::detachFrom(ControlView *parent, ControlView *)
{
    if (res_->flags().testFlag(ResourceView::Independent))
        fromItem(whiteCanvas())->attachSubProvider(nullptr);
    detaching();
    if (flags_ & Loading)
        whiteCanvas()->onControlLoad(false);
    if (flags_ & LoadFinished)
        saveSettings();
    (void) parent;
    if (flags_ & (Selected | Adjusting))
        whiteCanvas()->selector()->unselect(this);
#ifdef SHOWBOARD_QUICK
    realItem_->setParentItem(nullptr);
#else
    realItem_->scene()->removeItem(realItem_);
#endif
    detached();
    ControlTransform::removeAllTransforms(realItem_);
    attachToItem(realItem_, nullptr);
    if (item_ != realItem_) {
        ControlTransform::removeAllTransforms(item_);
        attachToItem(item_, nullptr);
    }
    transform_ = nullptr;
    itemObj_ = nullptr;
    if (res_->flags().testFlag(ResourceView::PersistSession)
            && flags_.testFlag(LoadFinished)) {
//        if (flags_.testFlag(Loading) && stateItem_) {
//            flags_.setFlag(LoadFinished);
//            loadFinished(true); // will delete stateItem_
//        }
        res_->saveSession(item_);
        if (item_ == realItem_)
            realItem_ = nullptr;
        else
            item_->setParentItem(nullptr);
    }
    if (realItem_)
        delete realItem_;
    realItem_ = nullptr;
    item_ = nullptr;
    transform_ = nullptr;
    //deleteLater();
    delete this;
}

void Control::relayout()
{
    if (flags_ & FullLayout) {
        resize(whiteCanvas()->rect().size());
        sizeChanged();
    } else {

    }
}

void Control::beforeClone()
{
    if (flags_ & LoadFinished)
        saveSettings();
}

void Control::afterClone(Control *)
{
}

void Control::copy(QMimeData &data)
{
    beforeClone();
    res_->copy(data);
    data.setProperty("OriginControl", QVariant::fromValue(life()));
    data.setProperty("OriginPage", QVariant::fromValue(res_->page()));
    connect(res_->page(), &QObject::destroyed, &data, [&data] () {
       data.setProperty("OriginPage", QVariant());
    });
}

void Control::paste(QMimeData const & data, WhiteCanvas * canvas)
{
    ResourcePage * po = data.property("OriginPage").value<ResourcePage*>();
    ResourcePage * pt = canvas->subPage();
    ResourceView * res = ResourceView::paste(data);
    if (res == nullptr)
        return;
    if (po)
        po->disconnect(&data);
    const_cast<QMimeData&>(data).setProperty("OriginPage", QVariant::fromValue(pt));
    connect(pt, &QObject::destroyed, &data, [&data] () {
        const_cast<QMimeData&>(data).setProperty("OriginPage", QVariant());
    });
    QPointF offset = data.property("Offset").toPointF();
    if (pt == po) {
        offset += QPointF(60, 60);
    } else {
        offset = -res->transform().offset();
    }
    res->transform().translate(offset);
    Control * c = canvas->addResource(res);
    QSharedPointer<LifeObject> life = data.property("OriginControl").value<QWeakPointer<LifeObject>>();
    if (life)
        qobject_cast<Control*>(life.get())->afterClone(c);
    if (po != pt && c->flags().testFlag(RestoreSession)) {
        c->flags_.setFlag(RestoreSession, false);
        c->initPosition(); // TODO: feedback to origin resourceview
        c->flags_.setFlag(RestoreSession, true);
        canvas->selector()->updateSelect(c);
        offset += res->transform().offset();
    }
    const_cast<QMimeData&>(data).setProperty("Offset", offset);
    const_cast<QMimeData&>(data).setProperty(
                "OriginControl", QVariant::fromValue(c->life()));
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
    int count = 0;
    if (itemObj_) {
        QMetaObject const * meta = itemObj_->metaObject();
        while (meta->className()[0] != 'Q' || meta->className()[1] >= 'a')
            meta = meta->superClass();
        count = meta->propertyCount();
    }
    int index;
    QObject * obj = nullptr;
    for (QByteArray & k : res_->dynamicPropertyNames()) {
        if (itemObj_ && (index = itemObj_->metaObject()->indexOfProperty(k)) >= count) {
            obj = itemObj_;
        } else if ((index = metaObject()->indexOfProperty(k)) >= 0) {
            obj = this;
        } else {
            obj = nullptr;
        }
        if (obj) {
            QMetaProperty prop = obj->metaObject()->property(index);
            QVariant v = res_->property(k);
            VariantHelper::convert2(v, prop.userType());
            prop.write(obj, v);
        }
    }
    setOverrideToolString(res_->overrideToolString());
}

void Control::saveSettings()
{
    for (QByteArray & k : dynamicPropertyNames())
        res_->setProperty(k, property(k));
    for (int i = Control::staticMetaObject.propertyCount();
            i < metaObject()->propertyCount(); ++i) {
        QMetaProperty p = metaObject()->property(i);
        if (p.isReadable())
            res_->setProperty(p.name(), p.read(this));
    }
    // special one
    res_->setProperty("sizeHint", sizeHint());
    // object item
    int count = 0;
    if (itemObj_) {
        QMetaObject const * meta = itemObj_->metaObject();
        while (meta->className()[0] != 'Q' || meta->className()[1] >= 'a')
            meta = meta->superClass();
        count = meta->propertyCount();
        for (int i = count;
                i < itemObj_->metaObject()->propertyCount(); ++i) {
            QMetaProperty p = itemObj_->metaObject()->property(i);
            if (p.isReadable())
                res_->setProperty(p.name(), p.read(itemObj_));
        }
    }
    if (flags_ & LoadFinished)
        res_->setSaved();
}

void Control::sizeChanged()
{
    QRectF rect = item_->boundingRect();
    QPointF center(rect.center());
    if (flags_ & LoadFinished) {
        if (!(flags_ & (Adjusting | FullLayout))) {
            // keep top left, assume top left not change
            QPointF offset = (flags_.testFlag(LayoutScale)
                              ? res_->transform().rotate()
                              : res_->transform().scaleRotate()).map(
                        item_->transform().map(center));
            move(offset);
        }
        if (res_->flags().testFlag(ResourceView::LargeCanvas))
            fromItem(whiteCanvas())->resize(rect.size());
    }
    item_->setTransform(QTransform::fromTranslate(-center.x(), -center.y()));
    if (realItem_ != item_)
        static_cast<ItemFrame *>(realItem_)->updateRect();
    if (stateItem_) {
        stateItem_->updateTransform();
    }
    ItemSelector * selector = whiteCanvas()->selector();
    selector->updateSelect(this);
}

QSizeF Control::sizeHint()
{
    return item_->boundingRect().size();
}

static void adjustSizeHint(QSizeF & size, QSizeF const & psize)
{
    if (size.width() > 0 && size.width() < 10.0)
        size.setWidth(psize.width() * size.width());
    if (size.height() < 0)
        size.setHeight(size.width() * -size.height());
    else if (size.height() < 10.0)
        size.setHeight(psize.height() * size.height());
    if (size.width() < 0)
        size.setWidth(size.width() * -size.height());
}

// called before attached
void Control::setSizeHint(QSizeF const & size)
{
    if (size.width() < 10.0 || size.height() < 10.0) {
        QSizeF size2 = size;
        Control * canvasControl = fromItem(whiteCanvas());
        QRectF rect = canvasControl ? item_->scene()->sceneRect() : whiteCanvas()->rect();
        adjustSizeHint(size2, rect.size());
        resize(size2);
    } else {
        resize(size);
    }
}

static void setMinMaxSize(QSizeF & ms, const QSizeF &size, bool min)
{
    static QSizeF screenSize = QApplication::primaryScreen()->size();
    QSizeF sz(floor(size.width()), floor(size.height()));
    qreal sw = size.width() - sz.width();
    qreal const & (*cmp) (qreal const& a, qreal const & b) = min ? &qMax<qreal> : &qMin<qreal>;
    if (qFuzzyIsNull(sw)) {
        ms.setWidth(size.width());
    } else {
        qreal w = screenSize.width() * sw;
        if (qFuzzyIsNull(sz.width())) {
            ms.setWidth(w);
        } else {
            ms.setWidth(cmp(w, sz.width()));
        }
    }
    qreal sh = size.height() - sz.height();
    if (qFuzzyIsNull(sh)) {
        ms.setHeight(size.height());
    } else {
        qreal h = screenSize.height() * sh;
        if (qFuzzyIsNull(sz.height())) {
            ms.setHeight(h);
        } else {
            ms.setHeight(cmp(h, sz.height()));
        }
    }
}

QSizeF Control::minSize()
{
    return minMaxSize_[0];
}

void Control::setMinSize(const QSizeF &size)
{
    setMinMaxSize(minMaxSize_[0], size, true);
}

QSizeF Control::maxSize()
{
    return minMaxSize_[1];
}

void Control::setMaxSize(const QSizeF &size)
{
    setMinMaxSize(minMaxSize_[1], size, false);
}

WhiteCanvas *Control::whiteCanvas()
{
    return static_cast<WhiteCanvas*>(realItem_->parentItem()->parentItem());
}

void Control::resize(QSizeF const & size)
{
    if (flags_ & (LoadFinished)) {
        return;
    }
    setProperty("delaySizeHint", size);
}

Control::SelectMode Control::selectTest(const QPointF &point)
{
    (void) point;
    return NotSelect;
}

Control::SelectMode Control::selectTest(ControlView * child, ControlView * item, QPointF const & point, bool onlyAssist)
{
    if (item_ != realItem_ && item == realItem_) { // hit frame
        return static_cast<ItemFrame*>(realItem_)->hitTest(child, point) ? Select : NotSelect;
    }
    if (stateItem_ == item) { // hit state
        if (!(flags_ & DefaultFlags))
            return NotSelect;
        return stateItem()->hitTest(child, point) ? Select : NotSelect;
    }
    if (res_->flags().testFlag(ResourceView::Independent))
        return PassSelect;
    if (onlyAssist) {
        SelectMode mode = item_ == realItem_ ? selectTest(point)
                                             : selectTest(realItem_->mapToItem(item_, point));
        if (mode == PassSelect) {
            mode = PassSelect2;
        } else if (mode != PassSelect2) {
            mode = NotSelect;
        }
        return mode;
    }
    if (flags_ & FullSelect)
        return Select;
    if ((flags_ & HalfSelect)) {
        return item_ == child ? Select : NotSelect;
    }
    if (item_ == realItem_) {
        return selectTest(point);
    } else {
        return selectTest(realItem_->mapToItem(item_, point));
    }
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
    ItemFrame * frame = nullptr;
    if (realItem_ != item_) {
        frame = static_cast<ItemFrame *>(realItem_);
        frame->updateRect();
    }
    if (flags_ & (FullLayout | RestoreSession))
        return;
    ControlView *parent = realItem_->parentItem();
    QRectF rect = whiteCanvas()->rect();
    // in large canvas, use visible rect of canvas
    Control * canvasControl = fromItem(whiteCanvas());
    if (canvasControl) {
        rect = parent->mapRectFromScene(parent->scene()->sceneRect());
        if (flags_.testFlag(FixedOnCanvas))
            rect.moveCenter({0, 0}); // center at scene
        if (!(flags_ & AutoPosition))
            res_->transform().translate(rect.center());
    }
    // if has frame and if item is not at center of frame,
    //  there will be a little offset, fix it here
    if (!(flags_ & AutoPosition)) {
        if (frame) {
            res_->transform().translate(-frame->padding().center());
        }
        QVariant posHint = res_->property("posHint");
        VariantHelper::convert2(posHint, QMetaType::QPointF);
        if (!posHint.isNull()) {
            QPointF pos = posHint.toPointF();
            QSizeF sz(pos.x(), pos.y());
            adjustSizeHint(sz, rect.size());
            pos.setX(sz.width());
            pos.setY(sz.height());
            res_->transform().translate(pos + rect.topLeft());
        }
        return;
    }
    QPolygonF polygon;
    for (ControlView * c : parent->childItems()) {
        if (c == realItem_ || fromItem(c)->flags() & FullLayout)
            continue;
        polygon = polygon.united(c->mapRectToItem(c->parentItem(), c->boundingRect()));
    }
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
    res_->transform().translate(pos);
}

void Control::loadFinished(bool ok, QString const & iconOrMsg)
{
    if (ok) {
        if (iconOrMsg.isNull()) {
            if (stateItem_) {
                ControlTransform::removeAllTransforms(stateItem_);
                delete stateItem_;
                stateItem_ = nullptr;
            }
        } else {
            stateItem()->setLoaded(iconOrMsg);
        }
        if (flags_ & LoadFinished)
            return;
        initScale();
        sizeChanged();
        flags_ |= LoadFinished;
        if (!(flags_ & RestoreSession)) {
            if ((flags_ & EnterAnimate))
                doAnimate();
            else if (flags_ & SelectOnLoaded)
                whiteCanvas()->selector()->select(this);
        }
    } else {
        if (flags_ & LoadFinished) {
            qWarning() << metaObject()->className() << iconOrMsg;
            return;
        }
        stateItem()->setFailed(iconOrMsg);
        QObject::connect(stateItem(), &StateItem::clicked, this, &Control::reload);
        sizeChanged();
        if ((flags_ & SelectOnLoaded) && !(flags_ & RestoreSession))
            whiteCanvas()->selector()->select(this);
    }
    flags_ &= ~Loading;
    whiteCanvas()->onControlLoad(false);
}

void Control::initScale()
{
    QSizeF ps = whiteCanvas()->rect().size();
    QSizeF size = item_->boundingRect().size();
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        Control * canvasControl = fromItem(whiteCanvas());
        canvasControl->flags_.setFlag(CanMove, flags_.testFlag(CanMove));
        canvasControl->flags_.setFlag(CanScale, flags_.testFlag(CanScale));
        canvasControl->flags_.setFlag(Touchable, flags_.testFlag(Touchable));
        flags_.setFlag(DefaultFlags, 0);
        if (flags_ & RestoreSession) {
            return;
        }
        canvasControl->resize(size);
        canvasControl->attached();
        return;
    }
    if (flags_ & (FullLayout | LoadFinished)) {
        return;
    }
    qreal scale = 1.0;
    QVariant delaySizeHint = property("delaySizeHint");
    if (delaySizeHint.isValid()) {
        QSizeF sh = delaySizeHint.toSizeF();
        scale = qMin(sh.width() / size.width(), sh.height() / size.height());
        if ((flags_ & ExpandScale) == 0 && scale > 1.0)
            scale = 1.0;
        else
            size = size * scale * 0.999999;
        delaySizeHint.clear();
        setProperty("delaySizeHint", delaySizeHint);
    }
    if (!(flags_ & RestoreSession)) {
        Control * canvasControl = fromItem(whiteCanvas());
        if (canvasControl) {
            ps = item_->scene()->sceneRect().size();
        }
        if (item_ != realItem_) {
            QRectF padding(static_cast<ItemFrame *>(realItem_)->padding());
            ps.setWidth(ps.width() - padding.width());
            ps.setHeight(ps.height() - padding.height());
        }
        res_->setProperty("originSize", size);
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
        if (canvasControl && !flags_.testFlag(FixedOnCanvas)) {
            qreal s = 1 / canvasControl->resource()->transform().zoom();
            size *= s;
            scale *= s;
        }
    }
    if (qFuzzyCompare(scale, 1.0))
        return;
    flags_.setFlag(Adjusting, true);
    res_->transform().scale({scale, scale});
    if (flags_ & LayoutScale) {
        resize(size);
    }
    if (item_ != realItem_) {
        res_->transform().translate(
                    -static_cast<ItemFrame *>(realItem_)->padding().center());
    }
    flags_.setFlag(Adjusting, false);
}

void Control::setSize(const QSizeF &size)
{
    QSizeF size2(size);
    adjustSizeHint(size2, whiteCanvas()->rect().size());
    if (flags_ & Adjusting) {
        setProperty("delayResize", size2);
    } else {
        resize(size2);
        sizeChanged();
    }
}

void Control::move(QPointF & delta)
{
    res_->transform().translate(delta);
}

bool Control::scale(QRectF &rect, const QRectF &direction, QPointF &delta)
{
    QRectF padding;
    if (realItem_ != item_)
        padding = itemFrame()->padding();
    bool result = res_->transform().scale(rect, direction, delta, padding,
                            flags_ & KeepAspectRatio, false, minMaxSize_);
    if (!result)
        return false;
    QRectF origin = rect;
    if (item_ != realItem_) {
        static_cast<ItemFrame *>(realItem_)->updateRectToChild(origin);
    }
    if (flags_ & LayoutScale) {
        resize(origin.size());
        sizeChanged();
    }
    return true;
}

bool Control::scale(const QRectF &direction, QPointF &delta)
{
    QRectF rect = boundRect();
    adjusting(true);
    scale(rect, direction, delta);
    adjusting(false);
    whiteCanvas()->selector()->updateSelect(this);
    return true;
}

bool Control::scale(const QPointF &center, qreal &delta)
{
    QRectF rect = boundRect();
    QRectF padding;
    QPointF center2 = center;
    if (realItem_ != item_) {
        padding = itemFrame()->padding();
        center2 = item_->mapFromItem(realItem_, center2);
    }
    center2 -= item_->boundingRect().center();
    adjusting(true);
    bool result = res_->transform().scale(rect, center2,
                                          delta, padding, false, minMaxSize_);
    if (!result)
        return false;
    QRectF origin = rect;
    if (item_ != realItem_) {
        static_cast<ItemFrame *>(realItem_)->updateRectToChild(origin);
    }
    if (flags_ & LayoutScale) {
        resize(origin.size());
        sizeChanged();
    }
    adjusting(false);
    if (realItem_->parentItem()) // maybe scaling canvas
        whiteCanvas()->selector()->updateSelect(this);
    return true;
}

void Control::gesture(GestureContext *ctx, QPointF const &to1, QPointF const &to2)
{
    if (!ctx->configged()) {
        ctx->config(flags_ & CanScale, flags_ & CanRotate, flags_ & CanMove, false);
    }
    if (flags_ & CanScale) {
        // size may changed
        QSizeF size = item_->boundingRect().size();
        ctx->limitScales(
                    qMax(minMaxSize_[0].width() / size.width(), minMaxSize_[0].height() / size.height()),
                    qMin(minMaxSize_[1].width() / size.width(), minMaxSize_[1].height() / size.height()));
    }
    res_->transform().gesture(ctx, to1, to2);
    if (ctx->layoutScale()) {
        QRectF rect = realItem_->boundingRect();
        rect.setWidth(rect.width() * ctx->scale());
        rect.setHeight(rect.height() * ctx->scale());
        if (item_ != realItem_) {
            static_cast<ItemFrame *>(realItem_)->updateRectToChild(rect);
        }
        resize(rect.size());
        sizeChanged();
    } else if (item_ != realItem_) {
        static_cast<ItemFrame *>(realItem_)->updateRect();
    }
}

void Control::rotate(QPointF const & from, QPointF & to)
{
    res_->transform().rotate(from, to);
}

void Control::rotate(QPointF const & center, QPointF const & from, QPointF &to)
{
    res_->transform().rotate(center, from, to);
}

void Control::setGeometry(const QRectF &geometry)
{
    QSizeF s = geometry.size() - boundRect().size();
    QPointF d(s.width(), s.height());
    scale(QRectF(0, 0, 1, 1), d);
    res_->transform().translateTo(geometry.center());
}

QRectF Control::boundRect() const
{
    QRectF rect = realItem_->boundingRect();
    if (item_ == realItem_) {
        rect.moveCenter({0, 0});
        if (!flags_.testFlag(LayoutScale)) {
            QTransform const & scale = res_->transform().scale();
            rect = QRectF(rect.x() * scale.m11(), rect.y() * scale.m22(),
                          rect.width() * scale.m11(), rect.height() * scale.m22());
        }
    }
    if (!(flags_ & LoadFinished) && stateItem_) {
        rect |= stateItem_->boundingRect();
    }
    return rect;
}

void Control::select(bool selected)
{
    flags_.setFlag(Selected, selected);
    if (realItem_ != item_)
        static_cast<ItemFrame *>(realItem_)->setSelected(selected);
}

class TransformRecord : public ResourceRecord
{
public:
    TransformRecord(ResourceTransform & transform) : transform_(transform), value_(transform) {}
    virtual void undo() override { std::swap(transform_, value_); }
    virtual void redo() override { std::swap(transform_, value_); }
    void reset() { value_ = transform_; changed_ = false; }
    void setChanged() { changed_ = true; }
    bool changed() const { return changed_; }
private:
    ResourceTransform & transform_;
    ResourceTransform value_;
    bool changed_ = false;
};

Q_DECLARE_METATYPE(TransformRecord*)

void Control::adjusting(bool be)
{
    flags_.setFlag(Adjusting, be);
    TransformRecord * tr = property("transformRecord").value<TransformRecord*>();
    if (be) {
        if (tr) {
            tr->reset();
        } else {
            tr = new TransformRecord(res_->transform());
            setProperty("transformRecord", QVariant::fromValue(tr));
        }
    } else {
        QVariant delayResize = property("delayResize");
        if (delayResize.isValid()) {
            setProperty("delayResize", QVariant());
            resize(delayResize.toSizeF());
            sizeChanged();
        }
        if (tr && tr->changed()) {
            res_->package()->records()->commit(tr);
            setProperty("transformRecord", QVariant());
        }
    }
}

ItemFrame * Control::itemFrame()
{
    if (item_ != realItem_) {
        return static_cast<ItemFrame*>(realItem_);
    }
    ItemFrame * frame = new ItemFrame(item_);
    realItem_ = frame;
    attachToItem(realItem_, this);
    ControlTransform * frameTransform = transform_->addFrameTransform();
    frameTransform->appendToItem(realItem_);
    ControlTransform::shiftLastTranform(item_, realItem_);
    return frame;
}

void Control::loadStream()
{
    auto l = life();
    res_->resource()->getStream().then([this, l] (QSharedPointer<QIODevice> stream) {
        if (l.isNull()) return;
        onStream(stream.get());
        loadFinished(true);
    }).fail([this, l](std::exception& e) {
        if (l.isNull()) return;
        loadFinished(false, e.what());
    });
}

void Control::loadData()
{
    auto l = life();
    res_->resource()->getData().then([this, l] (QByteArray data) {
        if (l.isNull()) return;
        onData(data);
        loadFinished(true);
    }).fail([this, l](std::exception& e) {
        if (l.isNull()) return;
        loadFinished(false, e.what());
    });
}

void Control::loadText()
{
    auto l = life();
    res_->resource()->getText().then([this, l] (QString text) {
        if (l.isNull()) return;
        onText(text);
        loadFinished(true);
    }).fail([this, l](std::exception& e) {
        if (l.isNull()) return;
        loadFinished(false, e.what());
    });
}

void Control::reload()
{
    QObject::disconnect(stateItem(), &StateItem::clicked, this, &Control::reload);
    if (!(flags_ & LoadFinished)) {
        flags_ |= Loading;
        stateItem()->setLoading();
        whiteCanvas()->onControlLoad(true);
        attached(); // reload
    }
}

void Control::onStream(QIODevice *stream)
{
    (void) stream;
    throw std::runtime_error("Not implemets onStream");
}

void Control::onData(QByteArray data)
{
    (void) data;
    throw std::runtime_error("Not implemets onData");
}

void Control::onText(QString text)
{
    (void) text;
    throw std::runtime_error("Not implemets onText");
}

void Control::doAnimate()
{
    QTimeLine * tl = new QTimeLine(400, this);
    QObject::connect(tl, &QTimeLine::finished, this, [=] () {
        if (flags_ & SelectOnLoaded)
            whiteCanvas()->selector()->select(this);
        delete tl;
    });
    tl->setProperty("scale", 1.0);
    realItem_->setOpacity(0);
    QObject::connect(tl, &QTimeLine::valueChanged, this, [=] (qreal v) {
        qreal scale = 1.06 - qAbs(v * 0.12 - 0.06);
        qreal diff = scale / tl->property("scale").toReal();
        tl->setProperty("scale", scale);
        QTransform tr = realItem_->transform();
        tr.scale(diff, diff);
        realItem_->setTransform(tr);
        realItem_->setOpacity(v);
    });
    tl->start();
}

void Control::getToolButtons(QList<ToolButton *> &buttons, ToolButton * parent)
{
    ToolButtonProvider::getToolButtons(buttons, parent);
    if (parent == nullptr) {
        btnFastCopy.setChecked(false);
        if (!buttons.empty())
            buttons.append(&ToolButton::SPLITTER);
        if (res_->canMoveTop())
            buttons.append(&btnTop);
        if (res_->flags() & ResourceView::CanCopy)
            buttons.append(&btnCopy);
        if (res_->flags().testFlag(ResourceView::CanFastCopy))
            buttons.append(&btnFastCopy);
        if (res_->flags() & ResourceView::CanDelete)
            buttons.append(&btnDelete);
        if (buttons.endsWith(&ToolButton::SPLITTER))
            buttons.pop_back();
    }
}

bool Control::handleToolButton(ToolButton *btn, const QStringList &args)
{
    if (btn->isHideSelector()) {
        whiteCanvas()->selector()->selectImplied(this);
    }
    if (btn == &btnTop) {
        res_->moveTop();
    } else if (btn == &btnCopy) {
        whiteCanvas()->copyResource(this);
    } else if (btn == &btnFastCopy) {
        bool checked = !btn->isChecked();
        whiteCanvas()->selector()->enableFastClone(checked);
        btn->setChecked(checked);
    } else if (btn == &btnDelete) {
        res_->removeFromPage();
    } else {
        return ToolButtonProvider::handleToolButton(btn, args);
    }
    return true;
}

void Control::updateTransform(int elem)
{
    if (flags_.testFlag(Adjusting)) {
        TransformRecord * tr = property("transformRecord").value<TransformRecord*>();
        if (tr)
            tr->setChanged();
        return;
    }
    if ((elem & 4) == 0)
        return;
    flags_.setFlag(Adjusting, true);
    if (flags_.testFlag(LayoutScale)) {
        QSizeF sz = res_->property("originSize").toSizeF();
        QSizeF zm = res_->transform().zoom2d();
        qDebug() << "Control::updateTransform" << sz << zm;
        resize({sz.width() * zm.width(), sz.height() * zm.height()});
    }
    sizeChanged();
    flags_.setFlag(Adjusting, false);
}

StateItem * Control::stateItem()
{
    if (stateItem_)
        return stateItem_;
    stateItem_ = new StateItem(item_);
    attachToItem(stateItem_, this);
    ControlTransform * ct = new ControlTransform(static_cast<ControlTransform*>(transform_), true, false, false);
    ct->appendToItem(stateItem_);
    return stateItem_;
}

