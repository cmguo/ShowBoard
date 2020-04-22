#ifndef STROKEPARSER_H
#define STROKEPARSER_H

#include "ShowBoard_global.h"
#include "istrokeparser.h"

#include <qlazy.h>

#include <QObject>

class SHOWBOARD_EXPORT StrokeParser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(std::vector<QLazy> parserTypes MEMBER parserTypes_)

public:
    static StrokeParser * instance();

public:
    explicit StrokeParser(QObject *parent = nullptr);

public:
    StrokePoint load(QByteArray const & type, QIODevice * stream, QList<StrokePoint> & points, IDynamicRenderer * renderer);

public slots:
    void onComposition();

private:
    std::vector<QLazy> parserTypes_;
    std::map<QString, QLazy *> parsers_;
};

#endif // STROKEPARSER_H
