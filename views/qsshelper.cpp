#include "qsshelper.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>

static QList<QssHelper*>& allHelpers()
{
    static QList<QssHelper*> list;
    return list;
}

static bool allApplied = false;

QssHelper::QssHelper(QString const & file)
    : file_(file)
{
    if (!allApplied)
        allHelpers().append(this);
}

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

void QssHelper::applyValuesToAllStylesheet(QMap<QByteArray, QMap<QByteArray, QString> > styleValues)
{
    allApplied = true;
    for (QssHelper* h : allHelpers()) {
        if (!h->file_.isEmpty())
            h->loadFromFile(h->file_);
        h->applyValues(styleValues);
    }
}

void QssHelper::loadFromFile(const QString &file)
{
    parse(loadText(file));
}

void QssHelper::loadFromString(const QByteArray &string)
{
    parse(string);
}

void QssHelper::loadFromData(const QByteArray &data)
{
    parse(data);
}

void QssHelper::setValue(const QByteArray &section, const QByteArray &key, const QString &value)
{
    static QString emptyValue;
    QMap<QByteArray, QString> & sec = styles_[section];
    sec[key] = value;
    cache_ = nullptr;
}

QString QssHelper::value(const QByteArray &section, const QByteArray &key)
{
    static QMap<QByteArray, QString> emptySection;
    static QString emptyValue;
    QMap<QByteArray, QString> sec = styles_.value(section, emptySection);
    return sec.value(key, emptyValue);
}

void QssHelper::applyValues(QMap<QByteArray, QMap<QByteArray, QString> > styleValues)
{
    auto iter = styleValues.keyValueBegin();
    for (; iter != styleValues.keyValueEnd(); ++iter) {
        auto iter2 = vstyles_.find((*iter).first);
        if (iter2 == vstyles_.end())
            continue;
        auto iter3 = (*iter).second.keyValueBegin();
        for (; iter3 != (*iter).second.keyValueEnd(); ++iter3) {
            auto iter4 = (*iter2).find((*iter3).first);
            if (iter4 == (*iter2).end())
                continue;
            for (QString* v : (*iter4)) {
                *v = (*iter3).second;
            }
        }
    }
    cache_ = nullptr;
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
            return;
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
