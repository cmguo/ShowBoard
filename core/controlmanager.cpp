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
static QImportMany<ControlManager, Control> import_controls("control_types", QPart::nonshared, true);

ControlManager * ControlManager::instance()
{
    static ControlManager * manager = nullptr;
    if (manager == nullptr)
        manager = ShowBoard::containter().get_export_value<ControlManager>();
    return manager;
}

ControlManager::ControlManager(QObject *parent)
    : QObject(parent)
{

}

void ControlManager::onComposition()
{
    for (auto & r : control_types_) {
        QByteArray types = r.part()->attr(Control::EXPORT_ATTR_TYPE);
        for (auto t : types.split(',')) {
            controls_[t] = &r;
        }
    }
}

Control * ControlManager::createControl(ResourceView * res)
{
    std::map<QByteArray, QLazy *>::iterator iter = controls_.find(res->resource()->type());
    if (iter == controls_.end())
        return new UnknownControl(res);
    return iter->second->create<Control>(Q_ARG(ResourceView*, res));
}

