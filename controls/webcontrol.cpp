#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "views/stateitem.h"
#include "views/whitecanvas.h"

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

#define LARGE_CANVAS_LINKAGE 1

static char const * toolstr = ""
        #ifdef QT_DEBUG
        "reload()|刷新|;"
        "debug()|调试|;"
        "dump()|输出|;"
        "hide()|隐藏|Checkable|;"
        "fitContent()|适合内容|;"
        "full()|全屏|Checkable|;"
        #endif
        ;

class WebPage : public QWebEnginePage
{
public:
    WebPage(QObject * parent, QObject *settings);
protected:
    QWebEnginePage *createWindow(WebWindowType );
};

class WebView : public QWebEngineView
{
private:
    static void sinit();
public:
    WebView(QObject * settings);
    void scaleTo(qreal scale);
    void debug();
    void synthesizedMouseEvents();
    void dump();
protected:
    virtual bool event(QEvent * event) override;
    virtual bool eventFilter(QObject * watched, QEvent * event) override;
private:
    QWidget *findChildWidget(const QString &className) const;
private:
    QPointer<QQuickWidget> childWidget_;
    qreal scale_ = 1.0;
};

// TODO: fix multiple touch crash

WebControl::WebControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, FixedOnCanvas}, {CanRotate})
    , fitToContent_(false)
    , hasBackground_(false)
{
    setToolsString(toolstr);
    setMinSize({0.1, 0.1});
    if (!res_->flags().testFlag(ResourceView::PersistSession))
        res_->setSessionGroup(nullptr); // force persist session
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        flags_.setFlag(DefaultFlags, false);
        flags_.setFlag(CanScale, true);
        flags_.setFlag(WithSelectBar, false);
#if LARGE_CANVAS_LINKAGE
        flags_.setFlag(FixedOnCanvas, true);
#else
        flags_.setFlag(FullLayout, true);
#endif
    }
}

bool WebControl::fitToContent() const
{
    return fitToContent_;
}

void WebControl::setFitToContent(bool b)
{
    fitToContent_ = b;
}

QColor WebControl::background() const
{
    return background_;
}

void WebControl::setBackground(const QColor &color)
{
    hasBackground_ = true;
    background_ = color;
}

void WebControl::setWebBridges(const WebControl::ObjectHash &bridges)
{
    if (flags_ & RestorePersisted)
        return;
    WebView * view = static_cast<WebView *>(widget_);
    QWebChannel * channel = view->page()->webChannel();
    if (channel == nullptr)
        channel = new QWebChannel(this);
    channel->registerObjects(bridges);
    view->page()->setWebChannel(channel);
}

QWidget * WebControl::createWidget(ResourceView * res)
{
    (void)res;
    QWebEngineView * view = new WebView(res);
    view->resize(1024, 576);
    QObject::connect(view->page(), &QWebEnginePage::loadFinished,
                     this, &WebControl::loadFinished);
    QObject::connect(view->page(), &QWebEnginePage::contentsSizeChanged,
                     this, &WebControl::contentsSizeChanged);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        QObject::connect(view->page(), &QWebEnginePage::scrollPositionChanged,
                         this, &WebControl::scrollPositionChanged);
    }
    return view;
}

void WebControl::loadSettings()
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)
            && !flags_.testFlag(RestoreSession)) {
        resize(item_->scene()->sceneRect().size().toSize());
    }
    WidgetControl::loadSettings();
}

void WebControl::attached()
{
    WebView * view = static_cast<WebView *>(widget_);
    if (flags_.testFlag(RestorePersisted)) {
        // TODO: handle backup loadFinished
        loadFinished(true);
        return;
    }
    if (hasBackground_) {
        QGraphicsRectItem * background = new QGraphicsRectItem(item_);
        background->setPen(QPen(Qt::NoPen));
        background->setBrush(QBrush(background_));
        QRectF rect = whiteCanvas()->rect();
        rect.moveCenter(item_->boundingRect().center());
        background->setRect(rect);
        background->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }
    item_->setFlag(QGraphicsItem::ItemIsFocusable);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        Control * canvasControl = Control::fromItem(whiteCanvas());
#if LARGE_CANVAS_LINKAGE
        connect(&canvasControl->resource()->transform(), &ResourceTransform::changed,
                this, [this, view]() {
            if (flags_.testFlag(Loading))
                return;
            qreal scale = qobject_cast<ResourceTransform*>(sender())->scale().m11();
            view->scaleTo(scale);
        });
#endif
    }
    view->load(res_->resource()->url());
}

void WebControl::loadFinished(bool ok)
{
    if (!flags_.testFlag(Loading))
        return;
    if (ok) {
        if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
            QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
            WebControl * canvasControl = static_cast<WebControl*>(Control::fromItem(whiteCanvas()));
            canvasControl->resize(view->page()->contentsSize());
        }
        WebView * view = static_cast<WebView *>(widget_);
        view->synthesizedMouseEvents();
        Control::loadFinished(ok);
    } else {
        QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
        view->setContent("");
        Control::loadFinished(ok, "加载失败");
    }
}

void WebControl::contentsSizeChanged(const QSizeF &size)
{
    QSizeF d = size - QSizeF(widget_->size());
    if (!fitToContent_ || (d.width() + d.height()) < 10
            || size.height() > whiteCanvas()->rect().height())
        return;
    qDebug() << "contentsSizeChanged: " << size;
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
    fitToContent_ = !fitToContent_;
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
        char const * flags =
                "--allow-running-insecure-content"
                " --disable-web-security"
                " --touch-events=disabled"
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
        init = true;
    }
}

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

void WebView::synthesizedMouseEvents()
{
    if (!childWidget_) {
        QWidget * widget = findChildWidget(
                    "QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget");
        childWidget_ = qobject_cast<QQuickWidget*>(widget);
    }
    childWidget_->installEventFilter(this);
}

void WebView::dump()
{
//    dumpObjectTree();
}

bool WebView::event(QEvent *event)
{
    qDebug() << "WebView::event: " << event->type();
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
        //qDebug() << "WebView::eventFilter: " << static_cast<QMouseEvent*>(event)->source();
        static_cast<QMouseEvent2*>(event)->caps = 0;
    } else if (event->type() == QEvent::CursorChange) {
        graphicsProxyWidget()->setCursor(childWidget_->cursor());
    }
    return false;
}

QWidget *WebView::findChildWidget(const QString &className) const
{
    for (auto w: findChildren<QWidget*>())
        if (className == QString(w->metaObject()->className()))
            return w;
    return nullptr;
}

// https://forum.qt.io/topic/112858/qwebengineview-how-to-open-new-tab-link-in-same-tab/2

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
    }
}

QWebEnginePage *WebPage::createWindow(QWebEnginePage::WebWindowType){
    WebPage *page = new WebPage(this, nullptr);
    connect(page, &QWebEnginePage::urlChanged, this, [this](QUrl const & url) {
        if(QWebEnginePage *page = qobject_cast<QWebEnginePage *>(sender())){
            setUrl(url);
            page->deleteLater();
        }
    });
    return page;
}
