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
    Q_PROPERTY(std::vector<QLazy> resource_types MEMBER resource_types_)

public:
    Q_INVOKABLE explicit ResourceManager(QObject *parent = nullptr);

    void mapResourceType(QString const & from, QString const & to);

signals:

public:
    static ResourceManager * instance();

    bool isExplitSupported(QUrl const & uri) const;

    ResourceView * createResource(QUrl const & uri, QString const & typeHint = nullptr) const;

    ResourceView * createResource(QString const & mine, QByteArray const & data) const;

    ResourceFactory * getFactory(QString const & type) const;

public slots:
    void onComposition();

private:
    QString findType(QUrl const & uri, QString& originType, QLazy*& lazy, QPair<int, int> const*& flags) const;

private:
    std::vector<QLazy> resource_types_;
    std::map<QString, QLazy *> resources_;
    std::map<QString, QPair<int, int>> commonResources_;
    QMap<QString, QString> mapTypes_;
};

#endif // RESOURCEMANAGER_H
