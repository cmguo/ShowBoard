#include "strokeparser.h"

#include <qcomponentcontainer.h>
#include <qlazy.hpp>

#include <QSizeF>
#include <QMetaClassInfo>

#include "core/resourcemanager.h"

static QImportMany<StrokeParser, IStrokeParser> import_controls("parserTypes", QPart::any, true);

StrokeParser * StrokeParser::instance()
{
    return QComponentContainer::globalInstance().get_export_value<StrokeParser>(QPart::shared);
}

StrokeParser::StrokeParser(QObject *parent)
    : QObject(parent)
{
}

void StrokeParser::onComposition()
{
    for (auto & r : parserTypes_) {
        QByteArray types = r.part()->attr(IStrokeParser::EXPORT_ATTR_TYPE);
        if (types.isEmpty()) {
            int index = r.part()->meta()->indexOfClassInfo(IStrokeParser::EXPORT_ATTR_TYPE);
            if (index >= 0)
                types = r.part()->meta()->classInfo(index).value();
        }
        for (auto t : types.split(',')) {
            parsers_[t.trimmed()] = &r;
        }
    }
}

StrokePoint StrokeParser::load(const QByteArray & type, QIODevice *stream, QList<StrokePoint> &points, IDynamicRenderer * renderer)
{
    std::map<QString, QLazy *>::iterator iter = parsers_.find(type);
    if (iter == parsers_.end())
        return StrokePoint{0, 0, 0};
    IStrokeParser * p = iter->second->get<IStrokeParser>();
    return p->load(type, stream, points, renderer);
}

