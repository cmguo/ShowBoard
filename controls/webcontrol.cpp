#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QApplication>
#include <QGraphicsItem>

static char const * toolstr =
        "reload()|刷新|:/showboard/icons/icon_delete.png;";

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
    : WidgetControl(res, HelpSelect)
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
}

QString WebControl::toolsString() const
{
    return toolstr;
}

QWidget * WebControl::createWidget(ResourceView * res)
{
    (void)res;
    QWebEngineView * view = new QWebEngineView();
    new TouchEventForwarder(view, this);
    view->resize(1024, 768);
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

QSizeF WebControl::sizeHint()
{
    return widget_->size();
}

// called before attached
void WebControl::setSizeHint(QSizeF const & size)
{
    if (size.width() < 10.0) {
        QRectF rect = item_->parentItem()->boundingRect();
        resize(QSizeF(rect.width() * size.width(), rect.height() * size.height()));
    } else {
        resize(size);
    }
}

void WebControl::loadFinished()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    resize(view->page()->contentsSize());
}

void WebControl::contentsSizeChanged(const QSizeF &size)
{
    qDebug() << "contentsSizeChanged: " << size;
    QSizeF d = size - QSizeF(widget_->size());
    if ((d.width() + d.height()) < 10)
        return;
    resize(size.toSize());
}

void WebControl::reload()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    view->reload();
}
