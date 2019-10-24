#include "webcontrol.h"
#include "resource.h"
#include "resourceview.h"

#include <QWebEngineView>
#include <QWebEngineSettings>

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
        QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
        init = true;
    }
}

QWidget * WebControl::createWidget(ResourceView * res)
{
    QWebEngineView * view = new QWebEngineView();
    view->load(res->resource()->url());
    return view;
}
