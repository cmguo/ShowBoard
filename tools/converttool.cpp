#include "converttool.h"
#include "core/showboard.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"
#include "views/whitecanvas.h"
#include "views/stateitem.h"

#include <QGraphicsRectItem>
#include <QMetaMethod>
#include <qcomponentcontainer.h>

ConvertTool::ConvertTool(ResourceView *res)
    : Control(res, {}, DefaultFlags)
    , startPage_(0)
{
}

QGraphicsItem *ConvertTool::create(ResourceView *res)
{
    (void) res;
    return new QGraphicsRectItem;
}

void ConvertTool::attached()
{
    QWeakPointer<int> l = life();
    res_->resource()->getLocalUrl().then([l, this] (QUrl url) {
        if (!l.isNull())
            convert(url);
    }, [l, this] (std::exception & e) {
        if (!l.isNull())
            loadFinished(false, e.what());
    });
    stateItem()->setLoading("正在下载");
    item_->show();
}

void ConvertTool::timerEvent(QTimerEvent *)
{
    res_->removeFromPage();
}

void ConvertTool::convert(const QUrl &url)
{
    stateItem()->setLoading("正在打开");
    QObject * converter = ShowBoard::containter().get_export_value("converter");
    connect(converter, SIGNAL(sigConvertImage(QString,int,int)),
                              this, SLOT(onImage(QString,int,int)));
    connect(converter, SIGNAL(sigConvertFinished()),
                              this, SLOT(onFinished()));
    connect(converter, SIGNAL(sigConvertFailed(QString)),
                              this, SLOT(onFailed(QString)));
    int index = converter->metaObject()->indexOfSlot("converterFile(QString)");
    QMetaMethod convert = converter->metaObject()->method(index);
    startPage_ = whiteCanvas()->package()->currentIndex();
    for (QByteArray k : res_->dynamicPropertyNames())
        settings_.insert(k, res_->property(k));
    settings_.remove("resourceType");
    convert.invoke(converter, Q_ARG(QString, url.toLocalFile()));
}

void ConvertTool::onImage(const QString &path, int nPage, int total)
{
    if (nPage == 0) return;
    whiteCanvas()->package()->newPage(startPage_ + nPage)
            ->addResource(QUrl::fromLocalFile(path), settings_);
    stateItem()->setLoading(QString("正在转换 %1/%2").arg(nPage).arg(total));
}

void ConvertTool::onFinished()
{
    ShowBoard::containter().release_value(sender());
    if (startPage_ == whiteCanvas()->package()->currentIndex())
        whiteCanvas()->package()->gotoNext();
    res_->removeFromPage();
}

void ConvertTool::onFailed(QString const & error)
{
    loadFinished(false, error);
    ShowBoard::containter().release_value(sender());
    startTimer(3000);
}
