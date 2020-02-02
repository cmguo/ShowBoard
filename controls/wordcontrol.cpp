#include "wordcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"
#include "office/word.h"
#include "office/workthread.h"

#include <QUrl>
#include <QDir>
#include <QMouseEvent>
#include <QGraphicsTextItem>
#include <QToolButton>

static char const * toolstr =
        "show()|开始演示|:/showboard/icons/icon_delete.png;"
        "next()|下一页|:/showboard/icons/icon_delete.png;"
        "prev()|上一页|:/showboard/icons/icon_delete.png;"
        "hide()|结束演示|:/showboard/icons/icon_delete.png";

WordControl::WordControl(ResourceView * res)
    : Control(res, {KeepAspectRatio})
{
    word_ = new Word;
    QObject::connect(word_, &Word::opened, this, &WordControl::opened);
    QObject::connect(word_, &Word::failed, this, &WordControl::failed);
    QObject::connect(word_, &Word::reopened, this, &WordControl::reopened);
    QObject::connect(word_, &Word::thumbed, this, &WordControl::thumbed);
    QObject::connect(word_, &Word::showed, this, &WordControl::showed);
    QObject::connect(word_, &Word::closed, this, &WordControl::closed);
}

WordControl::~WordControl()
{
    close();
    word_->deleteLater();
    word_ = nullptr;
}

QGraphicsItem * WordControl::create(ResourceView * res)
{
    (void) res;
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem;
    item->setTransformationMode(Qt::SmoothTransformation);
    return item;
}

void WordControl::attaching()
{
    //itemFrame()->addDockItem(ItemFrame::Right, 100, Qt::red);
}

void WordControl::attached()
{
    open();
}

void WordControl::open()
{
    QVariant localUrl = res_->property("localUrl");
    if (localUrl.isValid()) {
        open(localUrl.toUrl());
        return;
    }
    QWeakPointer<int> life(this->life());
    res_->resource()->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        res_->setProperty("localUrl", url);
        open(url);
    }).fail([this, life](std::exception & e) {
        if (life.isNull())
            return;
        loadFinished(false, e.what());
    });
}

void WordControl::open(QUrl const & url)
{
    Word * p = word_;
    WorkThread::postWork(p, [p, url]() {
       p->open(url.toLocalFile());
    });
}

void WordControl::opened(int total)
{
    (void) total;
    bool first = (flags_ & LoadFinished) == 0;
    bool autoShow = !(flags_ & RestoreSession)
            && property("autoShow").toBool();
    if (first) {
        QObject::connect(stateItem(), &StateItem::clicked, this, [this]() {
            show();
        });
        if (autoShow)
            show();
    }
}

void WordControl::failed(QString const & msg)
{
    loadFinished(false, msg);
}

void WordControl::reopened()
{
}

void WordControl::showed()
{
}

void WordControl::thumbed(QPixmap pixmap)
{
    if (!pixmap.isNull()) {
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
        bool first = (flags_ & LoadFinished) == 0;
        if (first) {
            loadFinished(true, QString(":/showboard/icons/play.svg"));
        }
    }
    item_->setCursor(Qt::ArrowCursor);
}

void WordControl::closed()
{
}

int WordControl::page()
{
    return word_->page();
}

void WordControl::setPage(int n)
{
    word_->setPage(n);
}

void WordControl::show(int page)
{
    Word * p = word_;
    WorkThread::postWork(p, [p, page]() {
        p->show(page);
    });
}

void WordControl::next()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->next();
    });
}

void WordControl::jump(int page)
{
    Word * p = word_;
    WorkThread::postWork(p, [p, page]() {
        p->jump(page);
    });
}

void WordControl::prev()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->prev();
    });
}

void WordControl::hide()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->hide();
    });
    item_->setCursor(Qt::ArrowCursor);
}

void WordControl::close()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->close();
    });
}

void WordControl::detached()
{
    close();
}

QString WordControl::toolsString(QByteArray const & parent) const
{
    (void) parent;
    (void) toolstr;
#ifdef QT_DEBUG
    return toolstr;
#else
    return "";//toolstr;
#endif
}

