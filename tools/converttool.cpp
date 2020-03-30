#include "converttool.h"
#include "core/showboard.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"
#include "core/workthread.h"
#include "views/whitecanvas.h"
#include "views/stateitem.h"

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QMetaMethod>
#include <QPainter>
#include <qcomponentcontainer.h>


static WorkThread& convThread() {
    static WorkThread thread("ConvertTool");
    return thread;
}

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
    if (whiteCanvas()->getToolControl(res_->url().toString()) != this) {
        loadFinished(false, "当前文档正在转换，请稍后重试");
        startTimer(3000);
        item_->show();
        return;
    }
    if (whiteCanvas()->getToolControl(res_->resource()->type()) != this) {
        loadFinished(false, "当前有其他文档正在转换，请稍后重试");
        startTimer(3000);
        item_->show();
        return;
    }
    QWeakPointer<int> l = life();
    res_->resource()->getLocalUrl().then([l, this] (QUrl url) {
        if (!l.isNull())
            convert(url);
    }, [l, this] (std::exception & e) {
        if (!l.isNull()) {
            loadFinished(false, e.what());
            startTimer(3000);
        }
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
    ResourcePage * page = whiteCanvas()->package()->newPage(startPage_ + nPage);
    page->addResource(QUrl::fromLocalFile(path), settings_);
    stateItem()->setLoading(QString("正在转换 %1/%2").arg(nPage).arg(total));
    QRectF rect = item_->scene()->sceneRect();
    convThread().postWork([path, page, rect]() {
        QPixmap pixmap(path);
        QSizeF size = (rect.size() * WhiteCanvas::THUMBNAIL_HEIGHT / rect.height());
        qreal scale = qMin(size.width() / pixmap.width(), size.height() / pixmap.height());
        QRectF rect2(0, 0, pixmap.width() * scale, pixmap.height() * scale);
        rect2.moveCenter(QPointF(size.width() / 2, size.height() / 2));
        QPixmap thumb(size.toSize());
        thumb.fill(Qt::transparent);
        QPainter painter(&thumb);
        painter.drawPixmap(rect2, pixmap, QRectF(0, 0, pixmap.width(), pixmap.height()));
        painter.end();
        WorkThread::postWork(page, [page, thumb] () {
            page->setThumbnail(thumb);
        });
    });
}

void ConvertTool::onFinished()
{
    ShowBoard::containter().release_value(sender());
    WhiteCanvas * wc = whiteCanvas();
    int startPage = startPage_;
    res_->removeFromPage();
    if (startPage == wc->package()->currentIndex())
        wc->package()->gotoNext();
}

void ConvertTool::onFailed(QString const & error)
{
    loadFinished(false, error);
    ShowBoard::containter().release_value(sender());
    startTimer(3000);
}
