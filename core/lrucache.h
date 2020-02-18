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

class FileLRUCache : public LRUCache<QString, QString>
{
    typedef LRUCache<QString, QString> base;

public:
    FileLRUCache(QDir const & dir, quint64 capacity);

public:
    QString put(QUrl const & url, QByteArray data);

    QByteArray get(QUrl const & url);

    bool contains(QUrl const & url);

    QString getFile(QUrl const & url);

protected:
    virtual quint64 sizeOf(const QString &v) override;

    virtual void destroy(const QString &k, const QString &v) override;

private:
    static QString urlMd5(QUrl const & url);

private:
    QDir dir_;
};

#endif // LRUCACHE_H
