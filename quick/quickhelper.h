#ifndef QUICKHELPER_H
#define QUICKHELPER_H

#include "ShowBoard_global.h"

#include <QObject>

class SHOWBOARD_EXPORT QuickHelper : public QObject
{
    Q_OBJECT
public:
    static QObject * createObject(QObject * context, char const * className,
                           char const * module = nullptr, char const * version = nullptr);

    static void appendChild(QObject * parent, QObject * child);

    static void clearChildren(QObject * parent);

    static QObject * findChild(QObject * parent, char const * className);

};

#endif // QUICKHELPER_H
