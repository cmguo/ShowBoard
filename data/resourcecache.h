#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include "ShowBoard_global.h"

#include <QList>
#include <QUrl>

class SHOWBOARD_EXPORT ResourceCache
{
public:
    ResourceCache();

    ~ResourceCache();

public:
    void add(QUrl const & url);

    void remove(QUrl const & url);

    void reset(QList<QUrl> const & tasks);

    void clear();

    void moveFront();

    void moveBackground();

private:
    Q_DISABLE_COPY(ResourceCache)

    bool background_ = false;
    QList<QUrl> tasks_;

public:
    static void pause();

    static void resume();

private:
    static void loadNext();
};

Q_DECLARE_METATYPE(ResourceCache*)

#endif // RESOURCECACHE_H
