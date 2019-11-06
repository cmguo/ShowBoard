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

static QSvgRenderer * nil = reinterpret_cast<QSvgRenderer *>(1);

QSvgRenderer * SvgCache::get(const QString &file)
{
    QSvgRenderer * renderer = cache_.value(file);
    if (!renderer) {
        renderer = new QSvgRenderer(file, this);
        if (renderer->isValid())
            cache_[file] = renderer;
        else {
            cache_[file] = nil;
            delete renderer;
            renderer = nullptr;
        }
    } else if (renderer == nil) {
        renderer = nullptr;
    }
    return renderer;
}
