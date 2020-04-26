#include "strokesreader.h"
#include "core/showboard.h"

#include <qcomponentcontainer.h>
#include <qlazy.hpp>

#include <QIODevice>
#include <QMetaClassInfo>

StrokesReader *StrokesReader::createReader(QIODevice *stream, const QByteArray &format)
{
    static QVector<QLazy> types;
    static QMap<QByteArray, QLazy*> readerTypes;
    if (readerTypes.empty()) {
         types = ShowBoard::containter().get_exports<StrokesReader>(QPart::nonshared);
         for (auto & r : types) {
             QByteArray types = r.part()->attr(StrokesReader::EXPORT_ATTR_TYPE);
             if (types.isEmpty()) {
                 int index = r.part()->meta()->indexOfClassInfo(StrokesReader::EXPORT_ATTR_TYPE);
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
    StrokesReader * p = (*iter)->create<StrokesReader>(Q_ARG(QIODevice*,stream));
    p->setProperty(StrokesReader::EXPORT_ATTR_TYPE, format);
    return p;
}

StrokesReader::StrokesReader(QIODevice * stream, QObject *parent)
    : QObject(parent)
    , stream_(stream)
{
}

bool StrokesReader::getMaximun(StrokePoint &point)
{
    return read(point);
}

bool StrokesReader::startAsyncRead(StrokesReader::AsyncHandler)
{
    return false;
}

void StrokesReader::close()
{
    stream_->close();
}