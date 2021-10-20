#include "controlmanager.h"
#include "control.h"
#include "resourceview.h"
#include "resource.h"
#include "showboard.h"
#include "controls/unknowncontrol.h"

#include "controls/controls.h"
#include "tools/tools.h"

#include <qpart.h>
#include <qlazy.hpp>

static QExport<ControlManager> export_(QPart::shared);
static QImportMany<ControlManager, Control> import_controls("control_types", QPart::nonshared);

ControlManager * ControlManager::instance()
{
    static ControlManager * manager = nullptr;
    if (manager == nullptr)
        manager = ShowBoard::containter().getExportValue<ControlManager>();
    return manager;
}

ControlManager::ControlManager(QObject *parent)
    : QObject(parent)
{
}

void ControlManager::mapControlType(const QByteArray &from, const QByteArray &to)
{
    mapTypes_[from] = to;
}

void ControlManager::onComposition()
{
    for (auto & r : control_types_) {
        QByteArray types = r.part()->attrMineType();
        for (auto t : types.split(',')) {
            controls_[t] = &r;
        }
    }
}

Control * ControlManager::createControl(ResourceView * res)
{
    QByteArray type = res->resource()->type();
    type = mapTypes_.value(type, type);
    std::map<QByteArray, QLazy *>::iterator iter = controls_.find(type);
    Control * control = nullptr;
    if (iter != controls_.end())
        control = iter->second->create<Control>(Q_ARG(ResourceView*, res));
    if (control == nullptr)
        control = new UnknownControl(res);
    return control;
}

