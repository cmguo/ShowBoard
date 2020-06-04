#ifndef STROKESWRITER_H
#define STROKESWRITER_H

#include "ShowBoard_global.h"

#include "strokepoint.h"

#include <QObject>

#include <QSharedPointer>
#include <functional>

class QIODevice;

class SHOWBOARD_EXPORT StrokesWriter : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("InheritedExport", "true")

public:
    static constexpr char const * EXPORT_ATTR_TYPE = "stroke_writer_type";

    typedef std::function<void (StrokePoint const & point, int bytePos)> AsyncHandler;

    static StrokesWriter * createWriter(QIODevice * stream, QByteArray const & format);

public:
    StrokesWriter(QIODevice * stream, QObject *parent = nullptr);

    virtual ~StrokesWriter() override;

public:
    QIODevice * stream() const { return stream_; }

public:
    virtual bool setMaximun(StrokePoint & max);

    virtual bool write(StrokePoint & point) = 0;

    virtual void close();

protected:
    QIODevice * stream_;
};

#define REGISTER_STROKE_WRITER(ctype, type) \
    static QExport<ctype, StrokesWriter> const export_strokes_Writer_##ctype(QPart::Attribute(StrokesWriter::EXPORT_ATTR_TYPE, type));

#endif // STROKESWRITER_H
