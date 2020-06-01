#include "svgcache.h"
#include "showboard.h"

#include <qcomponentcontainer.h>

#include <QMovie>
#include <QSvgRenderer>

static QExport<SvgCache> export_(QPart::shared);

SvgCache * SvgCache::instance()
{
    return ShowBoard::containter().getExportValue<SvgCache>();
}

SvgCache::SvgCache(QObject *parent)
    : QObject(parent)
{
}

static QSvgRenderer * nil = reinterpret_cast<QSvgRenderer *>(1);
static QMovie * nil2 = reinterpret_cast<QMovie *>(1);

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

QSvgRenderer *SvgCache::lock(QSvgRenderer *renderer)
{
    if (!locked_.contains(renderer)) {
        locked_.append(renderer);
        return renderer;
    }
    auto iter = cache_.keyValueBegin();
    for (; iter != cache_.keyValueEnd(); ++iter) {
        if ((*iter).second == renderer) {
            renderer = new QSvgRenderer((*iter).first, this);
            return renderer;
        }
    }
    return nullptr;
}

void SvgCache::unlock(QSvgRenderer *renderer)
{
    if (!locked_.removeOne(renderer))
        delete renderer;
}

QMovie *SvgCache::getMovie(const QString &file)
{
    QMovie * movie = cache2_.value(file);
    if (!movie) {
        movie = new QMovie(file, nullptr, this);
        if (movie->isValid())
            cache2_[file] = movie;
        else {
            cache2_[file] = nil2;
            delete movie;
            movie = nullptr;
        }
    } else if (movie == nil2) {
        movie = nullptr;
    }
    return movie;
}
