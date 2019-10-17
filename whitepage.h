#ifndef WHITEPAGE_H
#define WHITEPAGE_H

#include <QObject>

class ResourceView;

class WhitePage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourceView *> const & resources READ resources())
public:
    explicit WhitePage(QObject *parent = nullptr);

public:
    void addResource(ResourceView * res);

    void removeResource(ResourceView * res);

public:
    QList<ResourceView *> const & resources()
    {
        return resources_;
    }

public slots:

private:
    QList<ResourceView *> resources_;
};

#endif // WHITEPAGE_H
