#include "showboard.h"
#include "core/workthread.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/control.h"
#include "views/qsshelper.h"
#include "views/whitecanvasquick.h"
#include "views/quickwidgetitem.h"
#include "controls/webcontrol.h"

#include <qcomponentcontainer.h>

#include <QQuickWidget>

#include <data/localhttpserver.h>

QComponentContainer & ShowBoard::containter()
{
    static QComponentContainer c;
    return c;
}

void ShowBoard::init()
{
    static bool done = false;
    if (done) return;
    done = true;
    QssHelper::applyToAllStylesheet();
    qRegisterMetaType<Resource*>();
    qRegisterMetaType<QQuickWidget*>();
    qRegisterMetaType<ResourceView*>();
    qRegisterMetaType<Control*>();
    qRegisterMetaType<QHash<QString, QObject*>>();
    // qml types
    qmlRegisterType<WhiteCanvasQuick>("ShowBoard", 1, 0, "WhiteCanvasQuick");
    qmlRegisterType<QuickWidgetItem>("ShowBoard", 1, 0, "QuickWidgetItem");

    WebControl::init();

    LocalHttpServer::instance()->start();
}

void ShowBoard::exit()
{
    LocalHttpServer::instance()->stop();
    WorkThread::quitAll();
}

