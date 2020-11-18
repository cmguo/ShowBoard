#include "webview.h"

#include <QApplication>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QGraphicsProxyWidget>
#include <QWebEngineScript>
#include <QGraphicsScene>

#include <views/whitecanvas.h>

#include <controls/webcontrol.h>

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
                " --touch-events=disabled"
                " --register-pepper-plugins="
                "./pepflashplayer.dll;application/x-shockwave-flash";
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", flags);
        init = true;
    }
}

void WebView::sinit2()
{
    static bool init = false;
    if (!init) {
        sinit();
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

static void nop_del(WebView*) {}

WebView::WebView(QObject *settings)
{
    life_.reset(this, nop_del);
    sinit2();
    // make sure that touch events are delivered at all
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setStyleSheet("background:transparent");
    setPage(new WebPage(this, settings));
    connect(page(), &WebPage::fullScreenRequested, this, [](QWebEngineFullScreenRequest fullScreenRequest) {
        fullScreenRequest.accept();
    });
    connect(page(), &WebPage::loadFinished, this, [this]() {
         QObject::connect(hostWidget(), &QObject::destroyed, this, [this]() {
             hostWidget_ = nullptr;
             reload();
         }, Qt::QueuedConnection);
         hostWidget()->installEventFilter(this);
         hostWidget()->setFocus();
    });
    connect(page(), &WebPage::scrollPositionChanged, this, [this](QPointF const pos) {
        pos_ = pos;
        updateScale();
    });
}

void WebView::scroll(const QPointF &delta)
{
    QPoint p = delta.toPoint();
    QString script = QString("window.scrollBy(%1, %2);")
            .arg(p.x()).arg(p.y());
    page()->runJavaScript(script, QWebEngineScript::ApplicationWorld);
}

void WebView::scrollTo(QPointF const & pos)
{
    QPoint p = pos.toPoint();
    QString script = QString("window.scrollTo({left:%1,top:%2,behavior:'auto'});")
            .arg(p.x()).arg(p.y());
    qDebug() << "WebView::scrollTo" << pos << script;
    page()->runJavaScript(script, QWebEngineScript::ApplicationWorld);
}

void WebView::scale(qreal scale)
{
    qDebug() << "WebView::scale" << scale;
    QWidget * target = hostWidget();
    // first point
    QList<QTouchEvent::TouchPoint> touchPoints;
    QPointF c = target->geometry().center();
    sendTouchEvent(QEvent::TouchBegin, touchPoints,
                   10000001, Qt::TouchPointPressed, c - QPointF{100, 100});
    // add second point
    sendTouchEvent(QEvent::TouchUpdate, touchPoints,
                   10000002, Qt::TouchPointPressed, c + QPointF{100, 100});
    // move first point
    sendTouchEvent(QEvent::TouchUpdate, touchPoints,
                   0, Qt::TouchPointMoved, c - QPointF{100, 100} * scale);
    // move second point
    sendTouchEvent(QEvent::TouchUpdate, touchPoints,
                   1, Qt::TouchPointMoved, c + QPointF{100, 100} * scale);
    // release one point
    sendTouchEvent(QEvent::TouchUpdate, touchPoints,
                   0, Qt::TouchPointReleased);
    // release another point
    sendTouchEvent(QEvent::TouchUpdate, touchPoints,
                   0, Qt::TouchPointReleased);
}

void WebView::scaleTo(qreal scaleTo)
{
    scale(scaleTo / scale());
}

void WebView::debug()
{
    QWebEngineView * web = new QWebEngineView;
    Control * ctrl = Control::fromItem(graphicsProxyWidget());
    connect(ctrl, &QObject::destroyed, web, &QObject::deleteLater);
    web->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
    web->setMinimumSize(800, 450);
    web->setAttribute(Qt::WA_DeleteOnClose);
    QWebEnginePage * devPage = new QWebEnginePage(web);
    page()->setDevToolsPage(devPage);
    web->setPage(devPage);
    web->show();
}

void WebView::synthesizedMouseEvents()
{
    synthesizedMouse_ = true;
}

void WebView::capture()
{
//    dumpObjectTree();
    qDebug() << page()->profile()->httpCacheType();
    qDebug() << page()->profile()->cachePath();
    qDebug() << page()->profile()->httpCacheMaximumSize();
    //scrollTo({0, 100});
}

void WebView::showHide()
{
    setVisible(!isVisible());
}

void WebView::scaleUp()
{
    scale(1.2);
}

void WebView::scaleDown()
{
    scale(1.0 / 1.2);
}

bool WebView::event(QEvent *event)
{
    //qDebug() << "WebView::event: " << event->type();
    if (event->type() == QEvent::TouchBegin
            || event->type() == QEvent::TouchEnd
            || event->type() == QEvent::TouchUpdate
            || event->type() == QEvent::TouchCancel) {
        QApplication::sendEvent(hostWidget(), event);
        return true;
    } else if (event->type() == QEvent::Wheel) {
        QWebEngineView::event(event);
        event->accept();
        return true;
    } else if (event->type() == QEvent::CursorChange) {
        graphicsProxyWidget()->setCursor(hostWidget()->cursor());
    } else if (event->type() == QEvent::Show) {
        if (hostWidget_)
            hostWidget_->setFocus();
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
    if ( synthesizedMouse_ && (event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonRelease)) {
        static_cast<QMouseEvent2*>(event)->caps = 0;
    }
    if( event->type() == QEvent::Wheel && QApplication::keyboardModifiers ().testFlag(Qt::ControlModifier)) {
        event->accept();
        return true;
    }
    return false;
}

QPaintEngine *WebView::paintEngine() const
{
    static QPixmap backStore(1, 1);
    return backStore.paintEngine();
}

QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType)
{
    WhiteCanvas * canvas = static_cast<WhiteCanvas*>(
                graphicsProxyWidget()->parentItem()->parentItem());
    WebControl * control = qobject_cast<WebControl*>(
                canvas->addResource(QUrl(), {{"resourceType", "html"}}));
    return static_cast<WebView*>(control->widget());
}

void WebView::updateScale()
{
    QWeakPointer<WebView> l = life_;
    page()->runJavaScript("window.visualViewport.scale", [l] (QVariant result) {
        QSharedPointer<WebView> thiz = l.toStrongRef();
        if (!thiz) return;
        thiz->scale_ = result.toReal();
        qDebug() << "WebView::updateScale" << thiz->scale_;
        thiz->scaleChanged(thiz->scale_);
    });
}

void WebView::sendTouchEvent(QEvent::Type type, QList<QTouchEvent::TouchPoint> &points,
                             int id, Qt::TouchPointState state, const QPointF &pos)
{
    int index = id;
    if (state == Qt::TouchPointPressed) {
        QTouchEvent::TouchPoint touchPoint;
        touchPoint.setId(id);
        touchPoint.setState(state);
        touchPoint.setPos(pos);
        touchPoint.setScenePos(pos);
        index = points.size();
        points.append(touchPoint);
    } else {
        points[index].setState(state);
        if (state != Qt::TouchPointReleased) {
            points[index].setPos(pos);
            points[index].setScenePos(pos);
        }
    }
    {
        QTouchEvent event(type);
        event.setTouchPoints(points);
        QApplication::sendEvent(hostWidget_, &event);
    }
    points[index].setState(Qt::TouchPointStationary);
    if (state == Qt::TouchPointReleased) {
        points.removeAt(index);
    }
}

QQuickWidget *WebView::hostWidget()
{
    if (hostWidget_)
        return hostWidget_;
    const QByteArray hostClass = "QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget";
    for (auto w: findChildren<QWidget*>()) {
        if (hostClass == w->metaObject()->className()) {
            hostWidget_ = qobject_cast<QQuickWidget*>(w);
            return hostWidget_;
        }
    }
    Q_ASSERT(false);
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

QWebEnginePage *WebPage::createWindow(QWebEnginePage::WebWindowType type)
{
    if (mode_ == Disable) {
        return nullptr;
    }
    if (mode_ == InCurrent) {
        WebPage *page = new WebPage(this, nullptr);
        connect(page, &QWebEnginePage::urlChanged, this, [this](QUrl const & url) {
            if (QWebEnginePage *page = qobject_cast<QWebEnginePage *>(sender())){
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

