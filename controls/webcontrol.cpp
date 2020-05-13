#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "views/stateitem.h"
#include "views/whitecanvas.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsScene>

#define LARGE_CANVAS_LINKAGE 1

static char const * toolstr = ""
        #ifdef QT_DEBUG
        "reload()|刷新|;"
        "debug()|调试|;"
        "hide()|隐藏|Checkable|;"
        "fitContent()|适合内容|;"
        "full()|全屏|Checkable|;"
        #endif
        ;

class WebView : public QWebEngineView
{
public:
    WebView()
    {
        sinit();
        // make sure that touch events are delivered at all
        setAttribute(Qt::WA_AcceptTouchEvents);
    }
protected:
    virtual bool event(QEvent * event) override;
private:
    static void sinit();
private:
    QWidget *findChildWidget(const QString &className) const;
private:
    QPointer<QWidget> childWidget;
};

WebControl::WebControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, Touchable, FixedOnCanvas}, {CanRotate})
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

QWidget * WebControl::createWidget(ResourceView * res)
{
    (void)res;
    QWebEngineView * view = new WebView();
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
    if (flags_.testFlag(RestorePersisted)) {
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
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        Control * canvasControl = Control::fromItem(whiteCanvas());
#if LARGE_CANVAS_LINKAGE
//        connect(&canvasControl->resource()->transform(), &ResourceTransform::changed,
//                this, [this, view]() {
//            qreal scale = qobject_cast<ResourceTransform*>(sender())->scale().m11();
//            view->setZoomFactor(scale);
//        });
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
    QWebEngineView * web = new QWebEngineView();
    web->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
    web->setMinimumSize(500, 300);
    web->load(QUrl("http://localhost:7777"));
    web->show();
}

bool WebView::event(QEvent *event)
{
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
    } else if (event->type() == QEvent::Wheel) {
        QWebEngineView::event(event);
        event->accept();
        return true;
    }
    return QWebEngineView::event(event);
}

void WebView::sinit()
{
    static bool init = false;
    if (!init) {
        char const * flags =
                "--allow-running-insecure-content"
                " --disable-web-security"
                " --register-pepper-plugins="
                "./pepflashplayer64.dll;application/x-shockwave-flash";
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", flags);
#ifdef QT_DEBUG
        qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "7777");
#endif
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::PluginsEnabled, true);
        QWebEngineSettings::defaultSettings()->setAttribute(
                    QWebEngineSettings::ShowScrollBars, false);
        init = true;
    }
}

QWidget *WebView::findChildWidget(const QString &className) const
{
    for (auto w: findChildren<QWidget*>())
        if (className == QString(w->metaObject()->className()))
            return w;
    return nullptr;
}
