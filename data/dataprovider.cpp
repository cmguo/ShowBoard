#include "dataprovider.h"
#include "httpdataprovider.h"
#include "dataurlcodec.h"
#include "resourcecache.h"
#include "showboard.h"

#include <qexport.h>
#include <qlazy.h>
#include <qcomponentcontainer.h>

#include <QFile>
#include <QBuffer>

using namespace QtPromise;

REGISTER_DATA_RPOVIDER(DataDataProvider,"data")
REGISTER_DATA_RPOVIDER(FileDataProvider,"file,qrc,")
REGISTER_DATA_RPOVIDER(HttpDataProvider,"http,https")

DataProvider *DataProvider::getProvider(const QByteArray &scheme)
{
    static QVector<QLazy> types;
    static QMap<QByteArray, QLazy*> readerTypes;
    if (readerTypes.empty()) {
         types = ShowBoard::containter().getExports<DataProvider>(QPart::nonshared);
         for (auto & r : types) {
             QByteArray types = r.part()->attrMineType();
             for (auto & t : types.split(',')) {
                 readerTypes[t.trimmed()] = &r;
             }
         }
    }
    auto iter = readerTypes.find(scheme);
    if (iter == readerTypes.end())
        return nullptr;
    return iter.value()->get<DataProvider>();
}

DataProvider::DataProvider(QObject *parent)
    : QObject(parent)
{
}

/* DataDataProvider */

DataDataProvider::DataDataProvider(QObject *parent)
    : DataProvider(parent)
{
}

QtPromise::QPromise<QSharedPointer<QIODevice> > DataDataProvider::getStream(QObject * context, const QUrl &url, bool)
{
    auto data = DataUrlCodec::decodeDataUrl(url.toEncoded());
    QBuffer * buf(new QBuffer);
    buf->setData(data.data);
    buf->open(QIODevice::ReadOnly);
    if (context) {
        if (!data.mimeTypeName.isEmpty())
            context->setProperty("mimeType", data.mimeTypeName);
        if (!data.charset.isEmpty())
            context->setProperty("charset", data.charset);
    }
    return QPromise<QSharedPointer<QIODevice>>::resolve(QSharedPointer<QIODevice>(buf));
}

/* FileDataProvider */

FileDataProvider::FileDataProvider(QObject *parent)
    : DataProvider(parent)
{
}

QtPromise::QPromise<QSharedPointer<QIODevice> > FileDataProvider::getStream(QObject *, const QUrl &url, bool all)
{
    (void) all;
    QString path = url.scheme() == "qrc" ? ":" + url.path() : url.toLocalFile();
    QSharedPointer<QIODevice> file(new QFile(path));
    if (file->open(QFile::ReadOnly | QFile::ExistingOnly)) {
        return QPromise<QSharedPointer<QIODevice>>::resolve(file);
    } else {
        qDebug() << "FileDataProvider:" << file->errorString();
        return QPromise<QSharedPointer<QIODevice>>::reject(std::invalid_argument("打开失败，请重试"));
    }
}
