#ifndef QSSHELPER_H
#define QSSHELPER_H

#include <ShowBoard_global.h>

#include <QByteArray>
#include <QMap>
#include <QList>
#include <QSize>
#include <QVariant>

class QScreen;

class SHOWBOARD_EXPORT QssValue : public QVariant
{
public:
    QssValue(QString const & value);

public:
    QSize toSize() const;

    int toInt() const;

    QPen toPen() const;
};

class SHOWBOARD_EXPORT QssHelper
{
public:
    QssHelper(QString const & file = nullptr);

public:
    static qreal sizeScale(QScreen * screen = nullptr);

    static qreal sizeScale(qreal size);

    static int sizeScale(int size);

    static QSizeF sizeScale(QSizeF const & size);

    static QSize sizeScale(QSize const & size);

    static int fontSizeScale(int size);

public:
    static QString loadText(QString const & file);

    typedef QString (*StyleFunc)(QString const &);

    // key/spec/value
    static void setStyleSpecValues(QMap<QByteArray, QMap<QByteArray, QString>> styleValues);

    // key/name/func
    static void setStyleFunctions(QMap<QByteArray, QMap<QByteArray, StyleFunc>> styleFunctions);

    static bool applyToAllStylesheet(QScreen * screen);

public:
    void loadFromFile(QString const & file);

    void loadFromString(QString const & string);

    void loadFromData(QByteArray const & data);

public:
    void setValue(QByteArray const & section, QByteArray const & key, QString const & value);

    QssValue value(QByteArray const & section, QByteArray const & key) const;

    void applyValues(QMap<QByteArray, QMap<QByteArray, QString>> styleValues);

    void applyFunctions(QMap<QByteArray, QMap<QByteArray, StyleFunc>> styleFunctions);

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

template <typename T>
inline T dp(T const & t)
{
    return QssHelper::sizeScale(t);
}

class Destiny : public QObject
{
    Q_OBJECT
public slots:
    static qreal dp(qreal size);
};

#endif // QSSHELPER_H
