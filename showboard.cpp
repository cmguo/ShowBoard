#include "showboard.h"
#include "core/workthread.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/control.h"
#include "views/qsshelper.h"
#include "views/whitecanvasquick.h"
#include "views/quickwidgetitem.h"
#include "views/whitecanvastools.h"
#include "data/localhttpserver.h"
#include "data/resourcecache.h"
#include "controls/controls.h"

#include <qcomponentcontainer.h>

#include <QQuickWidget>

static QExport<WhiteCanvasTools, ToolButtonProvider> export_tools(QPart::shared);

QComponentContainer & ShowBoard::containter()
{
    static QComponentContainer c;
    return c;
}

void ShowBoard::init(QScreen * screen)
{
    static bool done = false;
    if (done) return;
    done = true;
    QssHelper::applyToAllStylesheet(screen);
    qRegisterMetaType<Resource*>();
    qRegisterMetaType<QQuickWidget*>();
    qRegisterMetaType<ResourceView*>();
    qRegisterMetaType<Control*>();
    qRegisterMetaType<QHash<QString, QObject*>>();
    // qml types
    qmlRegisterType<WhiteCanvasQuick>("ShowBoard", 1, 0, "WhiteCanvasQuick");
    qmlRegisterType<QuickWidgetItem>("ShowBoard", 1, 0, "QuickWidgetItem");

    LocalHttpServer::instance()->start();
}

void ShowBoard::exit()
{
    LocalHttpServer::instance()->stop();
    ResourceCache::stop();
    WorkThread::quitAll();
}

