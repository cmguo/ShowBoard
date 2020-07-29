#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <QLinkedList>
#include <QMap>

template <typename K, typename V>
class LRUCache
{
public:
    LRUCache(quint64 capacity)
        : capacity_(capacity)
        , size_(0)
    {
    }

    virtual ~LRUCache() {}

public:
    void put(K const & k, V const & v) {
        if (lruMap_.contains(k)) {
            return;
        }
        lruList_.prepend(QPair<K, V>(k, v));
        lruMap_.insert(k, lruList_.begin());
        size_ += sizeOf(v);
        while (size_ > capacity_) {
            QPair<K, V> & l = lruList_.back();
            destroy(l.first, l.second);
            size_ -= sizeOf(l.second);
            lruMap_.remove(l.first);
            lruList_.removeLast();
        }
    }

    V get(K const & k) {
        if (!lruMap_.contains(k)) {
            return V();
        }
        typename QLinkedList<QPair<K, V>>::iterator & i = lruMap_[k];
        if (i != lruList_.begin()) {
            lruList_.prepend(*i);
            lruList_.erase(i);
            i = lruList_.begin();
        }
        return i->second;
    }

    void remove(K const & k) {
        if (!lruMap_.contains(k)) {
            return;
        }
        typename QLinkedList<QPair<K, V>>::iterator i = lruMap_.take(k);
        QPair<K, V> & l = *i;
        destroy(l.first, l.second);
        size_ -= sizeOf(l.second);
        lruList_.erase(i);
    }

    bool contains(K const & k) {
        return lruMap_.contains(k);
    }

protected:
    virtual quint64 sizeOf(V const & v) = 0;

    virtual void destroy(K const & k, V const & v)
    {
        (void) k;
        (void) v;
    }

private:
    quint64 size_;
    quint64 capacity_;
    QLinkedList<QPair<K, V>> lruList_;
    QMap<K, typename QLinkedList<QPair<K, V>>::iterator> lruMap_;
};

#include <QFile>
#include <QDir>
#include <QtPromise>

struct FileLRUResource
{
    QString path;
    qint64 size;
};

class FileLRUCache : public LRUCache<QByteArray, FileLRUResource>
{
    typedef LRUCache<QByteArray, FileLRUResource> base;

public:
    FileLRUCache(QDir const & dir, quint64 capacity);

public:
    QtPromise::QPromise<QString> putStream(QUrl const & url, QSharedPointer<QIODevice> stream);

    QSharedPointer<QIODevice> getStream(QUrl const & url);

    QString putData(QUrl const & url, QByteArray data);

    QByteArray getData(QUrl const & url);

    QString getFile(QUrl const & url);

    bool contains(QUrl const & url);

    bool remove(QUrl const & url);

    FileLRUResource get(QUrl const & url, bool put = false);

protected:
    virtual quint64 sizeOf(const FileLRUResource &v) override;

    virtual void destroy(const QByteArray &k, const FileLRUResource &v) override;

private:
    static QByteArray urlMd5(QUrl const & url);

private:
    QDir dir_;
    QMap<QUrl, QtPromise::QPromise<QString>> asyncPuts_;
};

#endif // LRUCACHE_H
