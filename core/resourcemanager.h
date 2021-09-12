#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "ShowBoard_global.h"

#include <qlazy.h>

#include <QObject>
#include <QUrl>
#include <QMap>

class ResourceView;
class ResourceFactory;

class SHOWBOARD_EXPORT ResourceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(std::vector<QLazy> resourceTypes MEMBER resourceTypes_)
    Q_PROPERTY(std::vector<QLazy> providerTypes MEMBER providerTypes_)

public:
    Q_INVOKABLE explicit ResourceManager(QObject *parent = nullptr);

    void mapResourceType(QByteArray const & from, QByteArray const & to);

signals:

public:
    static ResourceManager * instance();

    bool isExplitSupported(QUrl const & uri) const;

    ResourceView * createResource(QUrl const & uri, QByteArray const & typeHint = nullptr) const;

    ResourceView * createResource(QByteArray const & mine, QByteArray const & data) const;

    ResourceFactory * getFactory(QByteArray const & type) const;

public slots:
    void onComposition();

private:
    void syncCommonTypes();

    QByteArray findType(QUrl const & uri, QByteArray& originType, QLazy*& lazy, QPair<int, int> const*& flags) const;

private:
    std::vector<QLazy> resourceTypes_;
    std::vector<QLazy> providerTypes_;
    std::map<QByteArray, QLazy *> resources_;
    std::map<QByteArray, QLazy *> providers_;
    std::map<QByteArray, QPair<int, int>> commonResources_;
    QMap<QByteArray, QByteArray> mapTypes_;
};

#endif // RESOURCEMANAGER_H
