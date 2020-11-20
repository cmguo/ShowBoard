#include "docxcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"
#include "office/word.h"
#include "core/workthread.h"

#include <QUrl>
#include <QDir>
#include <QMouseEvent>
#include <QGraphicsTextItem>
#include <QToolButton>

static char const * toolstr =
        "prev()|上一页|:/showboard/icon/page.prev.svg;"
        "next()|下一页|:/showboard/icon/page.next.svg;"
        ;

DocxControl::DocxControl(ResourceView * res)
    : ImageControl(res)
{
    setProperty("mipmap", 1.5);
    word_ = new Word;
    QObject::connect(word_, &Word::opened, this, &DocxControl::opened);
    QObject::connect(word_, &Word::failed, this, &DocxControl::failed);
    QObject::connect(word_, &Word::thumbed, this, &DocxControl::thumbed);
    QObject::connect(word_, &Word::closed, this, &DocxControl::closed);
}

DocxControl::~DocxControl()
{
    close();
    word_->deleteLater();
    word_ = nullptr;
}

void DocxControl::attached()
{
    open();
}

void DocxControl::open()
{
    QVariant localUrl = res_->property("localUrl");
    if (localUrl.isValid()) {
        open(localUrl.toUrl());
        return;
    }
    auto life(this->life());
    res_->resource()->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        res_->setProperty("localUrl", url);
        open(url);
    }, [this, life](std::exception &) {
        if (life.isNull())
            return;
        loadFailed();
    });
}

void DocxControl::open(QUrl const & url)
{
    Word * p = word_;
    WorkThread::postWork(p, [p, url]() {
       p->open(url.toLocalFile());
    });
}

void DocxControl::opened(int total)
{
    (void) total;
}

void DocxControl::failed(QString const & msg)
{
    loadFinished(false, msg);
}

void DocxControl::thumbed(QPixmap pixmap)
{
    if (!pixmap.isNull()) {
        setPixmap(pixmap);
    }
    item_->setCursor(Qt::ArrowCursor);
}

void DocxControl::closed()
{
}

int DocxControl::page()
{
    return word_->page();
}

void DocxControl::setPage(int n)
{
    word_->setPage(n);
}

void DocxControl::next()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->next();
    });
}

void DocxControl::jump(int page)
{
    Word * p = word_;
    WorkThread::postWork(p, [p, page]() {
        p->jump(page);
    });
}

void DocxControl::prev()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->prev();
    });
}

void DocxControl::close()
{
    Word * p = word_;
    WorkThread::postWork(p, [p]() {
        p->close();
    });
}

void DocxControl::detached()
{
    close();
}

QString DocxControl::toolsString(QByteArray const & parent) const
{
    (void) parent;
    (void) toolstr;
#ifdef QT_DEBUG
    return toolstr;
#else
    return "";//toolstr;
#endif
}

