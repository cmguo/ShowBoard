#ifndef STROKEREADER_H
#define STROKEREADER_H

#include "ShowBoard_global.h"

#include "strokepoint.h"

#include <QObject>

#include <functional>

class QIODevice;

class SHOWBOARD_EXPORT StrokeReader : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("InheritedExport", "true")

public:
    static constexpr char const * EXPORT_ATTR_TYPE = "stroke_render_type";

    typedef std::function<void (StrokePoint const & point)> AsyncHandler;

    static StrokeReader * createReader(QIODevice * stream, QByteArray const & format);

public:
    StrokeReader(QIODevice * stream, QObject *parent = nullptr);

public:
    virtual bool getMaximun(StrokePoint & max);

    virtual bool read(StrokePoint & point) = 0;

    virtual bool startAsyncRead(AsyncHandler handler);

    virtual void close();

protected:
    QIODevice * stream_;
};

#define REGISTER_STROKE_READER(ctype, type) \
    static QExport<ctype, StrokeReader> const export_stroke_reader_##ctype(QPart::Attribute(StrokeReader::EXPORT_ATTR_TYPE, type));

#endif // STROKEREADER_H
