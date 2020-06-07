#ifndef STROKESWRITER_H
#define STROKESWRITER_H

#include "ShowBoard_global.h"

#include "strokepoint.h"

#include <QObject>

class QIODevice;

class SHOWBOARD_EXPORT StrokesWriter : public QObject
{
    Q_OBJECT
public:
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
    static QExport<ctype, StrokesWriter> const export_strokes_Writer_##ctype(QPart::MineTypeAttribute(type));

#endif // STROKESWRITER_H
