#ifndef ISTROKEPARSER_H
#define ISTROKEPARSER_H

#include "ShowBoard_global.h"

#include <QObject>

#include <array>

class QIODevice;

typedef std::array<ushort, 3> StrokePoint;

class SHOWBOARD_EXPORT IDynamicRenderer : public QObject
{
    Q_OBJECT
public:
    virtual void setMaximun(StrokePoint const & max) = 0;

    virtual void addPoint(StrokePoint const & point) = 0;
};


class SHOWBOARD_EXPORT IStrokeParser : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("InheritedExport", "true")

public:
    static constexpr char const * EXPORT_ATTR_TYPE = "parser_type";

public:
    IStrokeParser(QObject *parent = nullptr);

    // return maximun of point values
    virtual StrokePoint load(QByteArray const & type, QIODevice * stream, QList<StrokePoint> & points, IDynamicRenderer * renderer) = 0;
};

#define REGISTER_STROKE_PARSER(ctype, type) \
    static QExport<ctype, IStrokeParser> const export_stroke_parser_##ctype(QPart::Attribute(IStrokeParser::EXPORT_ATTR_TYPE, type));

#endif // ISTROKEPARSER_H
