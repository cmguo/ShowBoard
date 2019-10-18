#ifndef CONTROLMANAGER_H
#define CONTROLMANAGER_H

#include "ShowBoard_global.h"

#include <qlazy.h>

#include <QObject>

class Control;
class ResourceView;

class SHOWBOARD_EXPORT ControlManager : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ControlManager(QObject *parent = nullptr);

    Q_PROPERTY(std::vector<QLazy> control_types MEMBER control_types_)
signals:

public:
    static ControlManager * instance();

    Control * createControl(ResourceView * res);

public slots:
    void onComposition();

private:
    std::vector<QLazy> control_types_;
    std::map<QString, QLazy *> controls_;
};

#endif // CONTROLMANAGER_H
