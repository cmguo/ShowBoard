#ifndef STROKEPARSER_H
#define STROKEPARSER_H

#include "ShowBoard_global.h"
#include "strokes.h"

#include <qlazy.h>

#include <QObject>

class QIODevice;

class SHOWBOARD_EXPORT IStrokeParser : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("InheritedExport", "true")

public:
    static constexpr char const * EXPORT_ATTR_TYPE = "parser_type";

public:
    IStrokeParser(QObject *parent = nullptr);

    // return maximun of point values
    virtual stroke_point_t load(QByteArray const & type, QIODevice * stream, QList<stroke_point_t> & points, IDynamicRenderer * renderer) = 0;

signals:
    void onPoint(stroke_point_t const & pt);
};

#define REGISTER_STROKE_PARSER(ctype, type) \
    static QExport<ctype, IStrokeParser> const export_stroke_parser_##ctype(QPart::Attribute(IStrokeParser::EXPORT_ATTR_TYPE, type));

class SHOWBOARD_EXPORT StrokeParser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(std::vector<QLazy> parserTypes MEMBER parserTypes_)

public:
    static StrokeParser * instance();

public:
    explicit StrokeParser(QObject *parent = nullptr);

public:
    stroke_point_t load(QByteArray const & type, QIODevice * stream, QList<stroke_point_t> & points, IDynamicRenderer * renderer);

public slots:
    void onComposition();

private:
    std::vector<QLazy> parserTypes_;
    std::map<QString, QLazy *> parsers_;
};

#endif // STROKEPARSER_H
