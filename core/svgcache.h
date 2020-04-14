#ifndef SVGPOOL_H
#define SVGPOOL_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QMap>

class QSvgRenderer;
class QMovie;

class SHOWBOARD_EXPORT SvgCache : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SvgCache(QObject *parent = nullptr);

public:
    static SvgCache * instance();

    /*
     * create control with type @type, not backed by resource
     */
    QSvgRenderer * get(QString const & file);

    QMovie * getMovie(QString const & file);

private:
    QMap<QString, QSvgRenderer *> cache_;
    QMap<QString, QMovie *> cache2_;
};

#endif // SVGPOOL_H
