#include "qsshelper.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QApplication>
#include <QScreen>

static QList<QssHelper*>& allHelpers()
{
    static QList<QssHelper*> list;
    return list;
}

static int strToInt(QString const & size, QString & unit)
{
    static QRegExp re("[a-z]");
    int n = size.indexOf(re);
    unit = size.mid(n);
    return size.left(n).toInt();
}

static QString sizeScale(QString const & size)
{
    QString unit;
    int n = strToInt(size, unit);
    return QString::number(QssHelper::sizeScale(n)) + unit;
}

static QString sizeScale2(QString const & sizes)
{
    QStringList list(sizes.split(' '));
    for (QString & s : list)
        s = sizeScale(s);
    return list.join(' ');
}

static QMap<QByteArray, QMap<QByteArray, QString> > gStyleValues;
static QMap<QByteArray, QMap<QByteArray, QssHelper::StyleFunc>> gStyleFunctions = {
    {"font-size", {{"size", &sizeScale}}},
    {"width", {{"size", &sizeScale}}},
    {"heigth", {{"size", &sizeScale}}},
    {"margin", {{"size", &sizeScale}}},
    {"border-radius", {{"size", &sizeScale}}},
    {"qproperty-iconSize", {{"size", &sizeScale2}}}
};

static bool allApplied = false;

QssHelper::QssHelper(QString const & file)
    : file_(file)
{
    if (allApplied) {
        if (!file.isEmpty()) {
            loadFromFile(file);
            applyValues(gStyleValues);
            applyFunctions(gStyleFunctions);
        }
    } else {
        allHelpers().append(this);
    }
}

/* static */
QString QssHelper::loadText(const QString &file)
{
    QString ret;
    QFile f(file);
    if (f.open(QIODevice::ReadOnly)) {
        ret = QString::fromUtf8(f.readAll());
        f.close();
    } else {
        qDebug() << f.errorString();
    }
    return ret;
}

qreal QssHelper::sizeScale()
{
    static qreal height = QApplication::primaryScreen()->size().height();
    static qreal s = height / 1080.0;
    return s;
}

qreal QssHelper::sizeScale(qreal size)
{
    return size * sizeScale();
}

int QssHelper::sizeScale(int size)
{
    return static_cast<int>(size * sizeScale());
}

int QssHelper::fontSizeScale(int size)
{
    return static_cast<int>(size * sizeScale());
}

QSize QssHelper::sizeFromString(const QString &size)
{
    QStringList list(size.split(' '));
    QString unit;
    int w = strToInt(list[0], unit);
    int h = strToInt(list[1], unit);
    return {w, h};
}

/* static */
void QssHelper::setStyleSpecValues(QMap<QByteArray, QMap<QByteArray, QString> > styleValues)
{
    gStyleValues = styleValues;
}

/* static */
void QssHelper::setStyleFunctions(QMap<QByteArray, QMap<QByteArray, StyleFunc>> styleFunctions)
{
    gStyleFunctions = styleFunctions;
}

bool QssHelper::applyToAllStylesheet()
{
    allApplied = true;
    for (QssHelper* h : allHelpers()) {
        if (!h->file_.isEmpty())
            h->loadFromFile(h->file_);
        h->applyValues(gStyleValues);
        h->applyFunctions(gStyleFunctions);
    }
    return true;
}

void QssHelper::loadFromFile(const QString &file)
{
    parse(loadText(file));
}

void QssHelper::loadFromString(const QString &string)
{
    parse(string);
    applyValues(gStyleValues);
    applyFunctions(gStyleFunctions);
}

void QssHelper::loadFromData(const QByteArray &data)
{
    parse(data);
    applyValues(gStyleValues);
    applyFunctions(gStyleFunctions);
}

void QssHelper::setValue(const QByteArray &section, const QByteArray &key, const QString &value)
{
    static QString emptyValue;
    QMap<QByteArray, QString> & sec = styles_[section];
    sec[key] = value;
    cache_ = nullptr;
}

QString QssHelper::value(const QByteArray &section, const QByteArray &key) const
{
    static QMap<QByteArray, QString> emptySection;
    static QString emptyValue;
    QMap<QByteArray, QString> sec = styles_.value(section, emptySection);
    return sec.value(key, emptyValue);
}

void QssHelper::applyValues(QMap<QByteArray, QMap<QByteArray, QString> > styleValues)
{
    auto iter = styleValues.keyValueBegin(); // key
    for (; iter != styleValues.keyValueEnd(); ++iter) {
        auto iter2 = vstyles_.find((*iter).first); // same key
        if (iter2 == vstyles_.end())
            continue;
        auto iter3 = (*iter).second.keyValueBegin(); // spec
        for (; iter3 != (*iter).second.keyValueEnd(); ++iter3) {
            auto iter4 = (*iter2).find((*iter3).first); // same spec
            if (iter4 == (*iter2).end())
                continue;
            for (QString* v : (*iter4)) {
                *v = (*iter3).second;
                cache_ = nullptr;
            }
        }
    }
}

void QssHelper::applyFunctions(QMap<QByteArray, QMap<QByteArray, StyleFunc>> styleFunctions)
{
    auto iter = styleFunctions.keyValueBegin(); // key
    for (; iter != styleFunctions.keyValueEnd(); ++iter) {
        auto iter2 = vstyles_.find((*iter).first); // same key
        if (iter2 == vstyles_.end())
            continue;
        auto iter3 = (*iter).second.keyValueBegin(); // name
        for (; iter3 != (*iter).second.keyValueEnd(); ++iter3) {
            auto iter4 = (*iter2).find((*iter3).first); // same key
            if (iter4 == (*iter2).end())
                continue;
            for (QString* v : (*iter4)) {
                *v = (*iter3).second(*v);
                cache_ = nullptr;
            }
        }
    }
}

QssHelper::operator QString () const
{
    if (!cache_.isEmpty())
        return cache_;
    QTextStream ts(&cache_, QIODevice::WriteOnly);
    auto iter = styles_.keyValueBegin();
    for (; iter != styles_.keyValueEnd(); ++iter) {
        ts << (*iter).first << QStringLiteral("{\n");
        auto iter2 = (*iter).second.keyValueBegin();
        for (; iter2 != (*iter).second.keyValueEnd(); ++iter2) {
            ts << "  " << (*iter2).first << ": " << (*iter2).second << ";\n";
        }
        ts << "}\n\n";
    }
    if (!file_.isEmpty() && cache_.isEmpty()) {
        cache_ = loadText(file_);
    }
    return cache_;
}

void QssHelper::parse(const QString &style)
{
    cache_ = style;
    QStringList lines = style.split("\n");
    QString comment;
    QByteArray section;
    QMap<QByteArray, QString> kvs;
    for (QString l : lines) {
        if (l.isEmpty())
            continue;
        comment.clear();
        int n = l.indexOf("/*");
        if (n >= 0) {
            int n1 = l.indexOf("*/", n + 2);
            if (n1 > n) {
                comment = l.mid(n + 2, n1 - n - 2).trimmed();
                l.remove(n, n1 + 2 - n);
            }
        }
        n = l.indexOf("{");
        if (n >= 0) {
            section = l.mid(0, n).trimmed().toUtf8();
            continue;
        }
        n = l.indexOf("}");
        if (n >= 0) {
            styles_[section] = kvs;
            kvs.clear();
            section.clear();
            continue;
        }
        n = l.indexOf(":");
        if (n > 0) {
            int n1 = l.indexOf(";", n + 1);
            if (n1 > n) {
                QByteArray k = l.mid(0, n).trimmed().toUtf8();
                kvs[k] = l.mid(n + 1, n1 - n - 1).trimmed();
                if (comment.startsWith('[') && comment.endsWith(']')) {
                    QByteArray v = comment.mid(1, comment.size() - 2).toUtf8();
                    vstyles_[k][v].append(&kvs[k]);
                }
            }
        }
    }
    if (!kvs.isEmpty()) {
        styles_[""] = kvs;
    }
}
