#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "views/stateitem.h"
#include "views/whitecanvas.h"
#include "core/resourcepage.h"
#include "data/resourcecache.h"
#include <web/webview.h>
#include <core/oomhandler.h>

#include <qproperty.h>

#include <QWebChannel>
#include <QMimeData>

#define LARGE_CANVAS_LINKAGE 1
#define LARGE_CANVAS_LINKAGE_SCALE 0

static char const * toolstr =
        "reload()|刷新|;"
        "debug()|调试|;"
        "dump()|输出|;"
        "hide()|隐藏|Checkable|;"
        "fitContent()|适合内容|;"
        "full()|全屏|Checkable|;"
        ;

static constexpr int MAX_WEB = 10;
static int totalFront = 0;

// TODO: fix multiple touch crash

void WebControl::init()
{
    WebView::sinit();
}

WebControl::WebControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, Touchable, FixedOnCanvas}, {CanRotate})
    , background_(Qt::transparent)
{
#ifdef QT_DEBUG
    setToolsString(toolstr);
#endif
    setMinSize({0.1, 0.1});
    if (!res_->flags().testFlag(ResourceView::PersistSession))
        res_->setSessionGroup(nullptr); // force persist session
    if (res_->flags().testFlag(ResourceView::Independent)) {
        flags_.setFlag(WithSelectBar, false);
    }
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        flags_.setFlag(DefaultFlags, false);
#if LARGE_CANVAS_LINKAGE_SCALE
        flags_.setFlag(CanScale, true);
#endif
#if LARGE_CANVAS_LINKAGE
        flags_.setFlag(FixedOnCanvas, true);
#else
        flags_.setFlag(FullLayout, true);
#endif
    }
}

bool WebControl::fitToContent() const
{
    return flags_.testFlag(FitToContent);
}

void WebControl::setFitToContent(bool b)
{
    flags_.setFlag(FitToContent, b);
}

bool WebControl::debugable() const
{
    return flags_.testFlag(Debugable);
}

void WebControl::setDebugable(bool b)
{
    flags_.setFlag(Debugable, b);
}

QColor WebControl::background() const
{
    return background_;
}

void WebControl::setBackground(const QColor &color)
{
    background_ = color;
}

void WebControl::setWebBridges(const QHash<QString, QObject*> &bridges)
{
    if (flags_ & RestorePersisted)
        return;
    WebView * view = static_cast<WebView *>(widget_);
    QWebChannel * channel = view->page()->webChannel();
    if (channel == nullptr)
        channel = new QWebChannel(res_);
    channel->registerObjects(bridges);
    view->page()->setWebChannel(channel);
}

QWidget * WebControl::createWidget(ResourceView * res)
{
    (void)res;
    WebView * view = new WebView(res);
    view->resize(1024, 576);
#ifndef QT_DEBUG
    view->setContextMenuPolicy(Qt::NoContextMenu);
#endif
    return view;
}

void WebControl::loadSettings()
{
    WidgetControl::loadSettings();
#ifndef QT_DEBUG
    if (debugable()) {
        setToolsString(toolstr);
    }
#endif
}

void WebControl::attached()
{
    if (++totalFront > MAX_WEB || !OomHandler::isMemoryAvailable(40 * 1024 * 1024)) {
        Control::loadFinished(false, "打开失败，内存不足");
        return;
    }
    if (res_->independent()) {
        ResourceCache::pause(this);
    }
    WebView * view = static_cast<WebView *>(widget_);
    QObject::connect(view->page(), &QWebEnginePage::loadFinished,
                     this, &WebControl::loadFinished, Qt::QueuedConnection);
    QObject::connect(view->page(), &QWebEnginePage::contentsSizeChanged,
                     this, &WebControl::contentsSizeChanged, Qt::QueuedConnection);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QObject::connect(view->page(), &QWebEnginePage::scrollPositionChanged,
                         this, &WebControl::scrollPositionChanged, Qt::QueuedConnection);
        QObject::connect(view, &WebView::scaleChanged,
                         this, &WebControl::scaleChanged, Qt::QueuedConnection);
    }

    if (flags_.testFlag(RestorePersisted)) {
        widget_->setVisible(true);
        //#if QT_VERSION >= 0x050E00
        //        qobject_cast<QWebEngineView *>(widget_)->page()->setLifecycleState(QWebEnginePage::LifecycleState::Active);
        //#endif
        // TODO: handle backup loadFinished
        Control::loadFinished(true);
        if (QQuickWindow::sceneGraphBackend() == "software") {
            int diff = property("ViewSizeChangeIndex").toInt();
            diff = diff < 0 ? 1 : -1;
            setProperty("ViewSizeChangeIndex", diff);
            widget_->resize(widget_->size() + QSize(0, diff));
        }
        return;
    }
    if (background_ != Qt::transparent) {
        QGraphicsRectItem * background = new QGraphicsRectItem(item_);
        background->setPen(QPen(Qt::NoPen));
        background->setBrush(QBrush(background_));
        QRectF rect = whiteCanvas()->rect();
        rect.moveCenter(item_->boundingRect().center());
        background->setRect(rect);
        background->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }
    item_->setFlag(QGraphicsItem::ItemIsFocusable);
//#if LARGE_CANVAS_LINKAGE_SCALE
//    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
//        Control * canvasControl = Control::fromItem(whiteCanvas());
//        connect(&canvasControl->resource()->transform(), &ResourceTransform::changed,
//                this, [this, view]() {
//            if (flags_.testFlag(Loading))
//                return;
//            qreal scale = qobject_cast<ResourceTransform*>(sender())->scale().m11();
//            view->scaleTo(scale);
//        });
//    }
//#endif
    if (auto data = res_->mimeData())
        view->setHtml(data->html());
    else
        view->load(res_->property("realUrl").isValid()? res_->property("realUrl").toString() : res_->url() );
}

void WebControl::detached()
{
    widget_->hide();
//#if QT_VERSION >= 0x050E00
//    qobject_cast<QWebEngineView *>(widget_)->page()->setLifecycleState(QWebEnginePage::LifecycleState::Discarded);
//#endif
    if (res_->independent()) {
        ResourceCache::resume(this);
    }
    WidgetControl::detached();
    --totalFront;
}

void WebControl::loadFinished(bool ok)
{
    if (!flags_.testFlag(Loading))
        return;
    if (ok) {
        if (!touchable())
            static_cast<WebView *>(widget_)->synthesizedMouseEvents();
        Control::loadFinished(ok);
        contentsSizeChanged({ 0, 0 });
    } else {
        QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
        view->setContent("");
        Control::loadFinished(ok, "加载失败");
    }
}

void WebControl::contentsSizeChanged(const QSizeF &size)
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
        WebControl * canvasControl = static_cast<WebControl*>(Control::fromItem(whiteCanvas()));
        canvasControl->resize(view->page()->contentsSize());
        scrollPositionChanged(view->page()->scrollPosition());
    }
    QSizeF d = size - QSizeF(widget_->size());
    if (!fitToContent() || (d.width() + d.height()) < 10
            || size.height() > whiteCanvas()->rect().height())
        return;
    setSize(size);
}

void WebControl::scrollPositionChanged(const QPointF &pos)
{
    Control * canvasControl = Control::fromItem(whiteCanvas());
    QRectF rect = canvasControl->boundRect();
    rect.moveCenter({0, 0});
    rect.setSize(item_->boundingRect().size());
    WebView * view = static_cast<WebView *>(widget_);
    canvasControl->resource()->transform()
            .translateTo(-rect.center() - pos * view->scale());
}

void WebControl::scaleChanged(qreal scale)
{
    Control * canvasControl = Control::fromItem(whiteCanvas());
    QRectF rect = canvasControl->boundRect();
    rect.moveCenter({0, 0});
    rect.setSize(item_->boundingRect().size());
    QPointF pos = (canvasControl->resource()->transform().offset()
                   + rect.center()) / -canvasControl->resource()->transform().zoom();
    canvasControl->resource()->transform().scaleTo(scale);
    canvasControl->resource()->transform()
            .translateTo(-rect.center() - pos * scale);
}

void WebControl::reload()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    view->reload();
}

void WebControl::hide()
{
    item_->setVisible(!item_->isVisible());
}

void WebControl::full()
{
    adjusting(true);
    resize(whiteCanvas()->rect().size());
    sizeChanged();
    adjusting(false);
}

void WebControl::fitContent()
{
    setFitToContent(!fitToContent());
}

void WebControl::debug()
{
    static_cast<WebView *>(widget_)->debug();
}

void WebControl::dump()
{
    static_cast<WebView *>(widget_)->dump();
}

void WebControl::scaleUp()
{
    static_cast<WebView *>(widget_)->scale(1.2);
}

void WebControl::scaleDown()
{
    static_cast<WebView *>(widget_)->scale(1.0 / 1.2);
}

