#include "svgcache.h"
#include "showboard.h"

#include <qcomponentcontainer.h>

#include <QSvgRenderer>

static QExport<SvgCache> export_(QPart::shared);

SvgCache * SvgCache::instance()
{
    return ShowBoard::containter().get_export_value<SvgCache>();
}

SvgCache::SvgCache(QObject *parent)
    : QObject(parent)
{
}

QSvgRenderer * SvgCache::get(const QString &file)
{
    QSvgRenderer * renderer = cache_.value(file);
    if (!renderer) {
        renderer = new QSvgRenderer(file, this);
        cache_[file] = renderer;
    }
    return renderer;
}
