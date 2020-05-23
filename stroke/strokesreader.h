#ifndef STROKESREADER_H
#define STROKESREADER_H

#include "ShowBoard_global.h"

#include "strokepoint.h"

#include <QObject>

#include <QSharedPointer>
#include <functional>

class QIODevice;

class SHOWBOARD_EXPORT StrokesReader : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("InheritedExport", "true")

public:
    static constexpr char const * EXPORT_ATTR_TYPE = "stroke_render_type";

    typedef std::function<void (StrokePoint const & point)> AsyncHandler;

    static StrokesReader * createReader(QIODevice * stream, QByteArray const & format);

public:
    StrokesReader(QIODevice * stream, QObject *parent = nullptr);

    virtual ~StrokesReader() override;

signals:
    void asyncFinished();

public:
    virtual bool getMaximun(StrokePoint & max);

    virtual bool read(StrokePoint & point) = 0;

    virtual bool startAsyncRead(AsyncHandler handler);

    virtual void close();

public:
    void storeStreamLife(QSharedPointer<QIODevice> stream);

protected:
    QIODevice * stream_;
    QSharedPointer<QIODevice> stream2_;
};

#define REGISTER_STROKE_READER(ctype, type) \
    static QExport<ctype, StrokesReader> const export_strokes_reader_##ctype(QPart::Attribute(StrokesReader::EXPORT_ATTR_TYPE, type));

#endif // STROKESREADER_H
