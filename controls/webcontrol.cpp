#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "views/stateitem.h"
#include "views/whitecanvas.h"
#include "core/resourcepage.h"
#include "data/resourcecache.h"

#include <qproperty.h>

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QQuickWidget>
#include <QQuickItem>
#include <QWebEngineFullScreenRequest>
#include <QWebChannel>
#include <QWebEngineProfile>

#define LARGE_CANVAS_LINKAGE 1
#define LARGE_CANVAS_LINKAGE_SCALE 0
#define DISABLE_TOUCH 1

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

class WebView : public QWebEngineView
{
public:
    static void sinit();
public:
    WebView(QObject * settings);
    void scaleTo(qreal scale);
    void debug();
#if DISABLE_TOUCH
    void synthesizedMouseEvents();
#endif
    void dump();
protected:
    virtual bool event(QEvent * event) override;
#if DISABLE_TOUCH
    virtual bool eventFilter(QObject * watched, QEvent * event) override;
#endif
    virtual QWebEngineView * createWindow(QWebEnginePage::WebWindowType type) override;
private:
    QWidget *findChildWidget(const QString &className) const;
private:
    QPointer<QQuickWidget> childWidget_;
    qreal scale_ = 1.0;
};

// TODO: fix multiple touch crash

void WebControl::init()
{
    WebView::sinit();
}

WebControl::WebControl(ResourceView * res)
#if DISABLE_TOUCH
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, FixedOnCanvas}, {CanRotate})
#else
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, Touchable, FixedOnCanvas}, {CanRotate})
#endif
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
    QObject::connect(view->page(), &QWebEnginePage::loadFinished,
                     this, &WebControl::loadFinished, Qt::QueuedConnection);
    QObject::connect(view->page(), &QWebEnginePage::contentsSizeChanged,
                     this, &WebControl::contentsSizeChanged, Qt::QueuedConnection);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QObject::connect(view->page(), &QWebEnginePage::scrollPositionChanged,
                         this, &WebControl::scrollPositionChanged, Qt::QueuedConnection);
    }
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

static int WebViewSizeChangeArra[4]{1,-1,1,-1};

void WebControl::attached()
{
    if (++totalFront > MAX_WEB) {
        Control::loadFinished(false, "打开失败，内存不足");
        return;
    }
    if (res_->independent()) {
        ResourceCache::pause(this);
    }
    WebView * view = static_cast<WebView *>(widget_);
    if (flags_.testFlag(RestorePersisted)) {
        widget_->setVisible(true);
        //#if QT_VERSION >= 0x050E00
        //        qobject_cast<QWebEngineView *>(widget_)->page()->setLifecycleState(QWebEnginePage::LifecycleState::Active);
        //#endif
        // TODO: handle backup loadFinished
        loadFinished(true);
        if(QQuickWindow::sceneGraphBackend() == "software"){
            int diff = WebViewSizeChangeArra[webViewSizeChangeIndex_];
            widget_->resize(widget_->size() + QSize(diff, diff));
            webViewSizeChangeIndex_ = (++webViewSizeChangeIndex_) % 4;
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
#if LARGE_CANVAS_LINKAGE_SCALE
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        Control * canvasControl = Control::fromItem(whiteCanvas());
        connect(&canvasControl->resource()->transform(), &ResourceTransform::changed,
                this, [this, view]() {
            if (flags_.testFlag(Loading))
                return;
            qreal scale = qobject_cast<ResourceTransform*>(sender())->scale().m11();
            view->scaleTo(scale);
        });
    }
#endif
    view->load(url());
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

QUrl WebControl::url() const
{
    return res_->resource()->url();
}

void WebControl::loadFinished(bool ok)
{
    if (!flags_.testFlag(Loading))
        return;
    if (ok) {
#if DISABLE_TOUCH
        WebView * view = static_cast<WebView *>(widget_);
        view->synthesizedMouseEvents();
#endif
        Control::loadFinished(ok);
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
    canvasControl->resource()->transform()
            .translateTo(-rect.center() - pos);
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

/* WebView */

void WebView::sinit()
{
    static bool init = false;
    if (!init) {
        QMetaType::registerConverter<QVariantMap, QHash<QString, QObject*>>([](QVariantMap from) {
            QHash<QString, QObject*> hash;
            for (auto i = from.keyValueBegin(); i != from.keyValueEnd(); ++i) {
                hash.insert((*i).first, (*i).second.value<QObject*>());
            }
            return hash;
        });
        char const * flags =
                "--allow-running-insecure-content"
                " --disable-web-security"
        #if DISABLE_TOUCH
                " --touch-events=disabled"
        #endif
                " --register-pepper-plugins="
                "./pepflashplayer64.dll;application/x-shockwave-flash";
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", flags);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::PluginsEnabled, true);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::FullScreenSupportEnabled, true);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::ErrorPageEnabled, false);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::ShowScrollBars, false);

//        QWebEngineProfile::defaultProfile()->setHttpCacheMaximumSize(
//                    1024 * 1024 * 1024); // 1G
        init = true;
    }
}

class WebPage : public QWebEnginePage
{
public:
    enum NewPageMode
    {
        Disable,
        InCurrent,
        NewView
    };
public:
    WebPage(QObject * parent, QObject *settings);
protected:
    QWebEnginePage *createWindow(WebWindowType);

private:
    NewPageMode mode_;
};

WebView::WebView(QObject *settings)
{
    sinit();
    // make sure that touch events are delivered at all
    setAttribute(Qt::WA_AcceptTouchEvents);
    setPage(new WebPage(this, settings));
    connect(page(), &WebPage::fullScreenRequested, this, [](QWebEngineFullScreenRequest fullScreenRequest) {
        fullScreenRequest.accept();
    });
}

void WebView::scaleTo(qreal scale)
{
    //    if (!childWidget_) {
    //        QWidget * widget = findChildWidget(
    //                    "QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget");
    //        childWidget_ = qobject_cast<QQuickWidget*>(widget);
//    }
    if (!childWidget_)
        return;
    const QTouchDevice *dev = nullptr;
    const QPointF localPos = page()->scrollPosition() + QPointF(width(), height()) / 2 / scale_;
    const QPointF windowPos = QPointF(width(), height()) / 2;
    const QPointF screenPos = mapToGlobal(windowPos.toPoint());
    qreal value = scale / scale_ - 1.0;
    qDebug() << "WebView::scale" << scale << value;
    qDebug() << "WebView::scale" << localPos << windowPos << screenPos;
    scale_ = scale;
    ulong sequenceId = 0;
    quint64 intArgument = 0;
    QNativeGestureEvent event1(Qt::BeginNativeGesture, dev, localPos, windowPos, screenPos, value, sequenceId, intArgument);
    QNativeGestureEvent event2(Qt::ZoomNativeGesture, dev, localPos, windowPos, screenPos, value, sequenceId, intArgument);
    QNativeGestureEvent event3(Qt::EndNativeGesture, dev, localPos, windowPos, screenPos, value, sequenceId, intArgument);
    QApplication::sendEvent(childWidget_, &event1);
    QApplication::sendEvent(childWidget_, &event2);
    QApplication::sendEvent(childWidget_, &event3);
}

void WebView::debug()
{
    QWebEngineView * web = new QWebEngineView();
    web->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
    web->setMinimumSize(800, 450);
    QWebEnginePage * devPage = new QWebEnginePage;
    page()->setDevToolsPage(devPage);
    web->setPage(devPage);
    web->show();
}

#if DISABLE_TOUCH
void WebView::synthesizedMouseEvents()
{
    if (!childWidget_) {
        QWidget * widget = findChildWidget(
                    "QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget");
        childWidget_ = qobject_cast<QQuickWidget*>(widget);
        QObject::connect(childWidget_, &QObject::destroyed, this, [this]() {
            reload();
            synthesizedMouseEvents();
        }, Qt::QueuedConnection);
    }
    childWidget_->installEventFilter(this);
}
#endif

void WebView::dump()
{
//    dumpObjectTree();
    qDebug() << page()->profile()->httpCacheType();
    qDebug() << page()->profile()->cachePath();
    qDebug() << page()->profile()->httpCacheMaximumSize();
}

bool WebView::event(QEvent *event)
{
    //qDebug() << "WebView::event: " << event->type();
    if (event->type() == QEvent::TouchBegin
            || event->type() == QEvent::TouchEnd
            || event->type() == QEvent::TouchUpdate
            || event->type() == QEvent::TouchCancel) {
        if (!childWidget_) {
            QWidget * widget = findChildWidget(
                        "QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget");
            childWidget_ = qobject_cast<QQuickWidget*>(widget);
        }
        Q_ASSERT(childWidget_);
        QApplication::sendEvent(childWidget_, event);
        return true;
    } else if (event->type() == QEvent::Wheel) {
        QWebEngineView::event(event);
        event->accept();
        return true;
    }
    return QWebEngineView::event(event);
}

#if DISABLE_TOUCH

class Q_GUI_EXPORT QMouseEvent2 : public QInputEvent
{
public:
    QPointF l, w, s;
    Qt::MouseButton b;
    Qt::MouseButtons mouseState;
    int caps;
    QVector2D velocity;
};

bool WebView::eventFilter(QObject *watched, QEvent *event)
{
    (void) watched;
    //if (event->type() != QEvent::Timer && event->type() != QEvent::MouseMove)
    //    qDebug() << "WebView::eventFilter: " << event->type();
    if (event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonRelease) {
        //qDebug() << "WebView::eventFilter: " << static_cast<QMouseEvent*>(event);
        static_cast<QMouseEvent2*>(event)->caps = 0;
    } else if (event->type() == QEvent::CursorChange) {
        graphicsProxyWidget()->setCursor(childWidget_->cursor());
    }
    return false;
}

#endif

QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType)
{
    WhiteCanvas * canvas = static_cast<WhiteCanvas*>(
                graphicsProxyWidget()->parentItem()->parentItem());
    WebControl * control = qobject_cast<WebControl*>(
                canvas->addResource(QUrl(), {{"resourceType", "html"}}));
    return static_cast<WebView*>(control->widget());
}

QWidget *WebView::findChildWidget(const QString &className) const
{
    for (auto w: findChildren<QWidget*>())
        if (className == QString(w->metaObject()->className()))
            return w;
    return nullptr;
}

// https://forum.qt.io/topic/112858/qwebengineview-how-to-open-new-tab-link-in-same-tab/2

static QVector<QByteArray> const AttributeNames = {
    "AutoLoadImages",
    "JavascriptEnabled",
    "JavascriptCanOpenWindows",
    "JavascriptCanAccessClipboard",
    "LinksIncludedInFocusChain",
    "LocalStorageEnabled",
    "LocalContentCanAccessRemoteUrls",
    "XSSAuditingEnabled",
    "SpatialNavigationEnabled",
    "LocalContentCanAccessFileUrls",
    "HyperlinkAuditingEnabled",
    "ScrollAnimatorEnabled",
    "ErrorPageEnabled",
    "PluginsEnabled",
    "FullScreenSupportEnabled",
    "ScreenCaptureEnabled",
    "WebGLEnabled",
    "Accelerated2dCanvasEnabled",
    "AutoLoadIconsForPage",
    "TouchIconsEnabled",
    "FocusOnNavigationEnabled",
    "PrintElementBackgrounds",
    "AllowRunningInsecureContent",
    "AllowGeolocationOnInsecureOrigins",
    "AllowWindowActivationFromJavaScript",
    "ShowScrollBars",
    "PlaybackRequiresUserGesture",
    "WebRTCPublicInterfacesOnly",
    "JavascriptCanPaste",
    "DnsPrefetchEnabled",
};

Q_DECLARE_METATYPE(WebPage::NewPageMode)

WebPage::WebPage(QObject *parent, QObject *setting)
    : QWebEnginePage(parent)
{
    if (setting) {
        for (QByteArray const & key : setting->dynamicPropertyNames()) {
            int i = AttributeNames.indexOf(key);
            if (i >= 0)
                settings()->setAttribute(
                            static_cast<QWebEngineSettings::WebAttribute>(i),
                            setting->property(key).toBool());
        }
        mode_ = setting->property("newPageMode").value<NewPageMode>();
    }
}

QWebEnginePage *WebPage::createWindow(QWebEnginePage::WebWindowType type) {
    if (mode_ == Disable) {
        return nullptr;
    }
    if (mode_ == InCurrent) {
        WebPage *page = new WebPage(this, nullptr);
        connect(page, &QWebEnginePage::urlChanged, this, [this](QUrl const & url) {
            if(QWebEnginePage *page = qobject_cast<QWebEnginePage *>(sender())){
                setUrl(url);
                page->deleteLater();
            }
        });
        return page;
    }
    if (mode_ == NewView) {
        return QWebEnginePage::createWindow(type);
    }
    return nullptr;
}

