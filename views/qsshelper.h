#ifndef QSSHELPER_H
#define QSSHELPER_H

#include <ShowBoard_global.h>

#include <QByteArray>
#include <QMap>
#include <QList>

class SHOWBOARD_EXPORT QssHelper
{
public:
    QssHelper(QString const & file = nullptr);

public:
    static QString loadText(QString const & file);

    static void applyValuesToAllStylesheet(QMap<QByteArray, QMap<QByteArray, QString>> styleValues);

public:
    void loadFromFile(QString const & file);

    void loadFromString(QByteArray const & string);

    void loadFromData(QByteArray const & data);

public:
    void setValue(QByteArray const & section, QByteArray const & key, QString const & value);

    QString value(QByteArray const & section, QByteArray const & key);

    void applyValues(QMap<QByteArray, QMap<QByteArray, QString>> styleValues);

public:
    operator QString() const;

private:
    void parse(QString const & style);

private:
    QString file_;
    mutable QString cache_;
    QMap<QByteArray, QMap<QByteArray, QString>> styles_;
    QMap<QByteArray, QMap<QByteArray, QList<QString*>>> vstyles_;
};

#endif // QSSHELPER_H
