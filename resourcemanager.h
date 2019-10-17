#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "ShowBoard_global.h"

#include <qlazy.h>

#include <QObject>
#include <QUrl>
#include <QMap>

class ResourceView;

class SHOWBOARD_EXPORT ResourceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(std::vector<QLazy> resource_types MEMBER resource_types_)

public:
    Q_INVOKABLE explicit ResourceManager(QObject *parent = nullptr);

    void mapResourceType(QString const & from, QString const & to);

signals:

public:
    ResourceView * CreateResource(QUrl const & uri);

    ResourceView * CreateResource(QString const & mine, QByteArray const & data);

public slots:
    void onComposition();

private:
    std::vector<QLazy> resource_types_;
    std::map<QString, QLazy *> resources_;
    QMap<QString, QString> mapTypes_;
};

#endif // RESOURCEMANAGER_H
