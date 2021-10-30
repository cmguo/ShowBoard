#include "converttool.h"
#include "showboard.h"
#include "core/resource.h"
#include "core/resourceview.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"
#include "core/workthread.h"
#include "views/whitecanvas.h"
#include "views/stateitem.h"

#include <qcomponentcontainer.h>

#ifdef SHOWBOARD_QUICK
#else
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QPainter>
#endif
#include <QMetaMethod>

static WorkThread& convThread() {
    static WorkThread thread("ConvertTool");
    return thread;
}

ConvertTool::ConvertTool(ResourceView *res)
    : Control(res, {FixedOnCanvas}, DefaultFlags)
    , startPage_(0)
{
}

ControlView *ConvertTool::create(ControlView *parent)
{
    (void) parent;
#ifdef SHOWBOARD_QUICK
    QQuickItem * item = nullptr;
#else
    QGraphicsRectItem * item = new QGraphicsRectItem;
    item->setPen(Qt::NoPen);
#endif
    return item;
}

void ConvertTool::attached()
{
    item_->setVisible(true);
    if (whiteCanvas()->getToolControl(res_->url().toString()) != this) {
        loadFinished(false, "当前文档正在转换，请稍后重试");
        startTimer(3000);
        return;
    }
    if (whiteCanvas()->getToolControl(res_->resource()->type()) != this) {
        loadFinished(false, "当前有其他文档正在转换，请稍后重试");
        startTimer(3000);
        return;
    }
    auto l = life();
    res_->resource()->getLocalUrl().then([l, this] (QUrl url) {
        if (!l.isNull())
            convert(url);
    }, [l, this] (std::exception &) {
        if (!l.isNull()) {
            loadFailed();
            startTimer(3000);
        }
    });
    stateItem()->setLoading("正在下载");
}

void ConvertTool::timerEvent(QTimerEvent *)
{
    res_->removeFromPage();
}

void ConvertTool::convert(const QUrl &url)
{
    stateItem()->setLoading("正在打开");
    QObject * converter = ShowBoard::containter().getExportValue("converter");
    if (converter == nullptr) {
        onFailed("没有找到转换工具");
        return;
    }
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
    QRectF rect = itemSceneRect(item_);
    convThread().postWork([path, page, rect]() {
        QPixmap pixmap(path);
        QSizeF size = (rect.size() * WhiteCanvas::THUMBNAIL_HEIGHT / rect.height());
        qreal scale = qMin(size.width() / pixmap.width(), size.height() / pixmap.height());
        QRectF rect2(0, 0, pixmap.width() * scale, pixmap.height() * scale);
        rect2.moveCenter(QPointF(size.width() / 2, size.height() / 2));
        QPixmap thumb(size.toSize());
        thumb.fill(Qt::transparent);
#ifdef SHOWBOARD_QUICK
#else
        QPainter painter(&thumb);
        painter.drawPixmap(rect2, pixmap, QRectF(0, 0, pixmap.width(), pixmap.height()));
        painter.end();
#endif
        WorkThread::postWork(page, [page, thumb] () {
            page->setThumbnail(thumb);
        });
    });
}

void ConvertTool::onFinished()
{
    ShowBoard::containter().releaseValue(sender());
    WhiteCanvas * wc = whiteCanvas();
    int startPage = startPage_;
    res_->removeFromPage();
    if (startPage == wc->package()->currentIndex())
        wc->package()->gotoNext();
}

void ConvertTool::onFailed(QString const & error)
{
    loadFinished(false, error);
    if (sender())
        ShowBoard::containter().releaseValue(sender());
    startTimer(3000);
}
