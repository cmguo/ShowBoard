#include "strokereader.h"
#include "core/showboard.h"

#include <qcomponentcontainer.h>
#include <qlazy.hpp>

#include <QIODevice>
#include <QMetaClassInfo>

StrokeReader *StrokeReader::createReader(QIODevice *stream, const QByteArray &format)
{
    static QVector<QLazy> types;
    static QMap<QByteArray, QLazy*> readerTypes;
    if (readerTypes.empty()) {
         types = ShowBoard::containter().get_exports<StrokeReader>(QPart::nonshared);
         for (auto & r : types) {
             QByteArray types = r.part()->attr(StrokeReader::EXPORT_ATTR_TYPE);
             if (types.isEmpty()) {
                 int index = r.part()->meta()->indexOfClassInfo(StrokeReader::EXPORT_ATTR_TYPE);
                 if (index >= 0)
                     types = r.part()->meta()->classInfo(index).value();
             }
             for (auto t : types.split(',')) {
                 readerTypes[t.trimmed()] = &r;
             }
         }
    }
    auto iter = readerTypes.find(format);
    if (iter == readerTypes.end())
        return nullptr;
    StrokeReader * p = (*iter)->create<StrokeReader>(Q_ARG(QIODevice*,stream));
    p->setProperty(StrokeReader::EXPORT_ATTR_TYPE, format);
    return p;
}

StrokeReader::StrokeReader(QIODevice * stream, QObject *parent)
    : QObject(parent)
    , stream_(stream)
{
}

bool StrokeReader::getMaximun(StrokePoint &point)
{
    return read(point);
}

bool StrokeReader::startAsyncRead(StrokeReader::AsyncHandler)
{
    return false;
}

void StrokeReader::close()
{
    stream_->close();
}
