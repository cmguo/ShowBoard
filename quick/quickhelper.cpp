#include "quickhelper.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlContext>
#include <QDebug>

QObject * QuickHelper::createObject(QObject *context, const char *className, const char *module, const char *version)
{
    if (module == nullptr) {
        module = "QtQuick";
        version = "2.0";
    } else if (version == nullptr) {
        version = "1.0";
    }
    QByteArray qml = QByteArray("import ") + module + " " + version + "; "
            + className + " {}";
    QQmlComponent qc(qmlEngine(context));
    qc.setData(qml, QUrl("qrc:"));
    if (!qc.isReady())
        qWarning() << "QuickHelper::createObject" << qml << qc.errorString();
    return qc.create();
}

void QuickHelper::appendChild(QObject *parent, QObject *child)
{
    int idef = parent->metaObject()->indexOfClassInfo("DefaultProperty");
    if (idef >= 0) {
        char const * def = parent->metaObject()->classInfo(idef).value();
        QQmlListReference data(parent, def, qmlEngine(parent));
        if (data.isValid())
            data.append(child);
    }
}

void QuickHelper::clearChildren(QObject *parent)
{
    int idef = parent->metaObject()->indexOfClassInfo("DefaultProperty");
    if (idef >= 0) {
        char const * def = parent->metaObject()->classInfo(idef).value();
        QQmlListReference data(parent, def, qmlEngine(parent));
        if (data.isValid())
            data.clear();
    }
}

QObject * QuickHelper::findChild(QObject *parent, const char *className)
{
    int idef = parent->metaObject()->indexOfClassInfo("DefaultProperty");
    if (idef >= 0) {
        char const * def = parent->metaObject()->classInfo(idef).value();
        QQmlListReference data(parent, def, qmlEngine(parent));
        if (data.isValid()) {
            for (int i = 0; i < data.count(); ++i) {
                QObject * c = data.at(i);
                if (strcmp(c->metaObject()->className(), className) == 0)
                    return c;
            }
        }
    }
    return nullptr;
}
