#ifndef STROKEPARSER_H
#define STROKEPARSER_H

#include "ShowBoard_global.h"
#include "strokes.h"

#include <qlazy.h>

#include <QObject>

class QIODevice;
class ResourceManager;

class SHOWBOARD_EXPORT IStrokeParser : public QObject
{
    Q_OBJECT
public:
    static constexpr char const * EXPORT_ATTR_TYPE = "parser_type";

    virtual QSizeF load(QString const & type, QIODevice * stream, QList<stroke_point_t> & points) = 0;
};

#define REGISTER_STROKE_PARSER(ctype, type) \
    static QExport<ctype, IStrokeParser> const export_stroke_parser_##ctype(QPart::Attribute(IStrokeParser::EXPORT_ATTR_TYPE, type));

class SHOWBOARD_EXPORT StrokeParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray strokeType MEMBER strokeType_)
    Q_PROPERTY(QObject* manager WRITE setManager)
    Q_PROPERTY(std::vector<QLazy> parser_types MEMBER parser_types_)

public:
    static StrokeParser * instance;

public:
    explicit StrokeParser(QObject *parent = nullptr);

public:
    QSizeF load(QString const & ext, QIODevice * stream, QList<stroke_point_t> & points);

    void setManager(QObject* manager);

public slots:
    void onComposition();

private:
    QByteArray strokeType_;
    ResourceManager * manager_;
    std::vector<QLazy> parser_types_;
    std::map<QString, QLazy *> parsers_;
};

#endif // STROKEPARSER_H
