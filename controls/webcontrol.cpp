#include "webcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

#include <QWebEngineView>
#include <QWebEngineSettings>

static char const * toolstr =
        "reload()|刷新|:/showboard/icons/icon_delete.png;";

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
    QWebEngineView * view = new QWebEngineView();
    QObject::connect(view->page(), &QWebEnginePage::loadFinished,
                     this, &WebControl::loadFinished);
    QObject::connect(view->page(), &QWebEnginePage::contentsSizeChanged,
                     this, &WebControl::contentsSizeChanged);
    view->load(res->resource()->url());
    return view;
}

void WebControl::loadFinished()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    resize(view->page()->contentsSize());
}

void WebControl::contentsSizeChanged(const QSizeF &size)
{
    QSizeF d = QSizeF(widget_->size()) - size;
    if (qAbs(d.width() + qAbs(d.height())) < 10)
        return;
    resize(size.toSize());
}

void WebControl::reload()
{
    QWebEngineView * view = qobject_cast<QWebEngineView *>(widget_);
    view->reload();
}
