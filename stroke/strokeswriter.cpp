#include "strokeswriter.h"
#include "showboard.h"

#include <qcomponentcontainer.h>
#include <qlazy.hpp>

#include <QIODevice>
#include <QMetaClassInfo>

StrokesWriter *StrokesWriter::createWriter(QIODevice *stream, const QByteArray &format)
{
    static QVector<QLazy> types;
    static QMap<QByteArray, QLazy*> WriterTypes;
    if (WriterTypes.empty()) {
         types = ShowBoard::containter().getExports<StrokesWriter>(QPart::nonshared);
         for (auto & r : types) {
             QByteArray types = r.part()->attr(StrokesWriter::EXPORT_ATTR_TYPE);
             if (types.isEmpty()) {
                 int index = r.part()->meta()->indexOfClassInfo(StrokesWriter::EXPORT_ATTR_TYPE);
                 if (index >= 0)
                     types = r.part()->meta()->classInfo(index).value();
             }
             for (auto t : types.split(',')) {
                 WriterTypes[t.trimmed()] = &r;
             }
         }
    }
    auto iter = WriterTypes.find(format);
    if (iter == WriterTypes.end())
        return nullptr;
    StrokesWriter * p = (*iter)->create<StrokesWriter>(Q_ARG(QIODevice*,stream));
    p->setProperty(StrokesWriter::EXPORT_ATTR_TYPE, format);
    return p;
}

StrokesWriter::StrokesWriter(QIODevice * stream, QObject *parent)
    : QObject(parent)
    , stream_(stream)
{
}

StrokesWriter::~StrokesWriter()
{
    close();
}

bool StrokesWriter::setMaximun(StrokePoint &point)
{
    return write(point);
}

void StrokesWriter::close()
{
    stream_->close();
}
