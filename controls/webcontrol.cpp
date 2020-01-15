#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "views/stateitem.h"
#include "views/whitecanvas.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QApplication>
#include <QGraphicsItem>

static char const * toolstr =
        "reload()|刷新|:/showboard/icons/icon_refresh.png;"
        "-full()|全屏|:/showboard/icons/icon_refresh.png;";

class TouchEventForwarder : public QObject
{
public:
    TouchEventForwarder(QWebEngineView *w, QObject *parent)
        : QObject(parent)
        , webView(w)
        , m_waState(w->testAttribute(Qt::WA_AcceptTouchEvents))
    {
        // make sure that touch events are delivered at all
        w->setAttribute(Qt::WA_AcceptTouchEvents);
        w->installEventFilter(this);
    }

    virtual ~TouchEventForwarder() override
    {
        if (webView) {
            webView->removeEventFilter(this);
            webView->setAttribute(Qt::WA_AcceptTouchEvents, m_waState);
        }
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::TouchBegin
                || event->type() == QEvent::TouchEnd
                || event->type() == QEvent::TouchUpdate
                || event->type() == QEvent::TouchCancel) {
            if (!childWidget)
                childWidget = findChildWidget("QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget");
            Q_ASSERT(childWidget);
            qDebug() << "eventFilter: " << event->type();
            QApplication::sendEvent(childWidget, event);
            return true;
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QWidget *findChildWidget(const QString &className) const
    {
        for (auto w: webView->findChildren<QWidget*>())
            if (className == QString::fromLatin1(w->metaObject()->className()))
                return w;
        return nullptr;
    }

    QPointer<QWidget> webView;
    QPointer<QWidget> childWidget;
    bool m_waState;
};


WebControl::WebControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, Touchable})
{
    static bool init = false;
    if (!init) {
        char const * flags =
                "--allow-running-insecure-content"
                " --disable-web-security"
                " --register-pepper-plugins="
                    "./pepflashplayer64_32_0_0_270.dll;application/x-shockwave-flash";
        ::_putenv_s("QTWEBENGINE_CHROMIUM_FLAGS", flags);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::PluginsEnabled, true);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::ShowScrollBars, false);
        init = true;
    }
    setToolsString(toolstr);
}

bool WebControl::layoutScale() const
{
    return flags_.testFlag(LayoutScale);
}

void WebControl::setLayoutScale(bool b)
{
    flags_.setFlag(LayoutScale, b);
}

QWidget * WebControl::createWidget(ResourceView * res)
{
    (void)res;
    QWebEngineView * view = new QWebEngineView();
    new TouchEventForwarder(view, this);
    view->resize(1024, 576);
    QObject::connect(view->page(), &QWebEnginePage::loadFinished,
                     this, &WebControl::loadFinished);
    QObject::connect(view->page(), &QWebEnginePage::contentsSizeChanged,
                     this, &WebControl::contentsSizeChanged);
    return view;
}

void WebControl::attached()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    view->load(res_->resource()->url());
}

void WebControl::loadFinished(bool ok)
{
    if (ok) {
        Control::loadFinished(ok);
    } else {
        Control::loadFinished(ok, "Load failed");
    }
}

void WebControl::contentsSizeChanged(const QSizeF &size)
{
    QSizeF d = size - QSizeF(widget_->size());
    if ((d.width() + d.height()) < 10
            || size.height() > whiteCanvas()->rect().height())
        return;
    qDebug() << "contentsSizeChanged: " << size;
    setSize(size);
}

void WebControl::reload()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    view->reload();
}

void WebControl::full()
{
    adjusting(true);
    resize(whiteCanvas()->rect().size());
    sizeChanged();
    adjusting(false);
}
