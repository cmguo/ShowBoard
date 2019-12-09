#include "strokeparser.h"

#include <qlazy.hpp>

#include <QSizeF>

#include "core/resourcemanager.h"

static QImportOptional<StrokeParser, ResourceManager> import_resource_("manager", QPart::shared);
static QImportMany<StrokeParser, IStrokeParser> import_controls("parser_types", QPart::nonshared, true);

StrokeParser * StrokeParser::instance = nullptr;

StrokeParser::StrokeParser(QObject *parent)
    : QObject(parent)
    , strokeType_("stroke")
    , manager_(nullptr)
{
    instance = this;
}

void StrokeParser::onComposition()
{
    for (auto & r : parser_types_) {
        QString types = r.part()->attr(IStrokeParser::EXPORT_ATTR_TYPE);
        for (auto t : types.split(",", QString::SkipEmptyParts)) {
            manager_->mapResourceType(t, strokeType_);
            parsers_[t] = &r;
        }
    }
}

QSizeF StrokeParser::load(const QString & type, QIODevice *stream, QList<stroke_point_t> &points)
{
    std::map<QString, QLazy *>::iterator iter = parsers_.find(type);
    if (iter == parsers_.end())
        return QSizeF();
    IStrokeParser * p = iter->second->get<IStrokeParser>();
    return p->load(type, stream, points);
}

void StrokeParser::setManager(QObject *manager)
{
    manager_ = qobject_cast<ResourceManager*>(manager);
}
