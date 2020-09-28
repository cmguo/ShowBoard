#include "webview.h"

#include <QApplication>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QGraphicsProxyWidget>
#include <QWebEngineScript>

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

static void nop_del(WebView*) {}

WebView::WebView(QObject *settings)
{
    life_.reset(this, nop_del);
    sinit();
    // make sure that touch events are delivered at all
    setAttribute(Qt::WA_AcceptTouchEvents);
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
    QTouchEvent::TouchPoint touchPoint;
    QPointF c = target->geometry().center();
    touchPoint.setId(10000001);
    touchPoint.setState(Qt::TouchPointPressed);
    touchPoint.setPos(c - QPointF{100, 100});
    touchPoint.setScenePos(touchPoint.pos());
    touchPoints.append(touchPoint);
    { // TouchBegin
        QTouchEvent event(QEvent::TouchBegin);
        event.setTouchPoints(touchPoints);
        QApplication::sendEvent(target, &event);
    }
    // add second point
    touchPoints[0].setState(Qt::TouchPointStationary);
    touchPoint.setId(10000002);
    touchPoint.setPos(c + QPointF{100, 100});
    touchPoint.setScenePos(touchPoint.pos());
    touchPoints.append(touchPoint);
    { // TouchUpdate
        QTouchEvent event(QEvent::TouchUpdate);
        event.setTouchPoints(touchPoints);
        QApplication::sendEvent(target, &event);
    }
    // move one point
    touchPoints[0].setState(Qt::TouchPointMoved);
    touchPoints[0].setPos(c - QPointF{100, 100} * scale);
    touchPoints[0].setScenePos(touchPoints[0].pos());
//    touchPoints[1].setState(Qt::TouchPointStationary);
//    { // TouchUpdate
//        QTouchEvent event(QEvent::TouchUpdate);
//        event.setTouchPoints(touchPoints);
//        QApplication::sendEvent(target, &event);
//    }
//    // move another point
//    touchPoints[0].setState(Qt::TouchPointStationary);
    touchPoints[1].setState(Qt::TouchPointMoved);
    touchPoints[1].setPos(c + QPointF{100, 100} * scale);
    touchPoints[1].setScenePos(touchPoints[1].pos());
    { // TouchUpdate
        QTouchEvent event(QEvent::TouchUpdate);
        event.setTouchPoints(touchPoints);
        QApplication::sendEvent(target, &event);
    }
    // release one point
    touchPoints[0].setState(Qt::TouchPointReleased);
    touchPoints[1].setState(Qt::TouchPointStationary);
    { // TouchUpdate
        QTouchEvent event(QEvent::TouchUpdate);
        event.setTouchPoints(touchPoints);
        QApplication::sendEvent(target, &event);
    }
    // release another point
    touchPoints.pop_front();
    touchPoints[0].setState(Qt::TouchPointReleased);
    { // TouchEnd
        QTouchEvent event(QEvent::TouchEnd);
        event.setTouchPoints(touchPoints);
        QApplication::sendEvent(target, &event);
    }
}

void WebView::scaleTo(qreal scaleTo)
{
    scale(scaleTo / scale());
}

void WebView::debug()
{
    QWebEngineView * web = new QWebEngineView(this);
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


void WebView::dump()
{
//    dumpObjectTree();
    qDebug() << page()->profile()->httpCacheType();
    qDebug() << page()->profile()->cachePath();
    qDebug() << page()->profile()->httpCacheMaximumSize();
    scrollTo({0, 100});
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

