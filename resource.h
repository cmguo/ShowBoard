#ifndef RESOURCE_H
#define RESOURCE_H

#include <QObject>
#include <QUrl>
#include <QSizeF>

class Resource : public QObject
{
    Q_OBJECT
public:
    Resource(QString const & type, QUrl const & url);

    Resource(Resource const & o);

    Q_PROPERTY(QUrl const url READ url())
    Q_PROPERTY(QString const type READ type())
    Q_PROPERTY(QSizeF size MEMBER size_)

signals:
    void sizeChanged();

public slots:
    QUrl const & url() const
    {
        return url_;
    }

    QString const & type() const
    {
        return type_;
    }

private:
    QUrl const url_;
    QString const type_;
    QSizeF size_;
};

#endif // RESOURCE_H
