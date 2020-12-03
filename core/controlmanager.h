#ifndef CONTROLMANAGER_H
#define CONTROLMANAGER_H

#include "ShowBoard_global.h"

#include <qlazy.h>

#include <QMap>
#include <QObject>

class Control;
class ResourceView;

class SHOWBOARD_EXPORT ControlManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(std::vector<QLazy> control_types MEMBER control_types_)

public:
    Q_INVOKABLE explicit ControlManager(QObject *parent = nullptr);

    void mapControlType(QByteArray const & from, QByteArray const & to);

public:
    static ControlManager * instance();

    /*
     * create control according to resource type
     */
    Control * createControl(ResourceView * res);

public slots:
    void onComposition();

private:
    std::vector<QLazy> control_types_;
    std::map<QByteArray, QLazy *> controls_;
    QMap<QByteArray, QByteArray> mapTypes_;
};

#endif // CONTROLMANAGER_H
