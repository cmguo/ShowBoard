#include "word.h"
#include "workthread.h"

#include <QAxObject>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include <Windows.h>

extern bool saveGdiImage(char* data, int size, char** out, int * nout);

QAxObject * Word::application_ = nullptr;

static char const * titleParts[] = {"Word", "ppt", nullptr};

static QThread & workThread() {
    static WorkThread thread("Word");
    return thread;
}

Word::Word(QObject * parent)
    : QObject(parent)
    , documents_(nullptr)
    , document_(nullptr)
    , panes_(nullptr)
    , total_(0)
    , page_(1)
{
    moveToThread(&workThread());
}

Word::~Word()
{
    if (documents_)
        delete documents_;
    documents_ = nullptr;
}

static constexpr int wdNumberOfPagesInDocument = 4;

void Word::open(QString const & file)
{
    if (application_ == nullptr) {
        application_ = new QAxObject("Word.Application");
        if (application_ == nullptr) {
            application_ = new QAxObject("Kwps.Application");
            if (application_)
                titleParts[0] = "WPS";
        }
        if (application_) {
            application_->moveToThread(&workThread());
        }
    }
    if (application_ && !documents_) {
        documents_ = application_->querySubObject("Documents");
        QObject::connect(documents_, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
    }
    if (!documents_) {
        emit failed("No Word Application");
        return;
    }
    file_ = file;
    QAxObject * document = documents_->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                file,
                false/*ConfirmConversions*/,
                true/*ReadOnly*/,
                false/*AddToRecentFiles*/
                );
    if (document) {
        QObject::connect(document, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        document_ = document;
        total_ = document_->querySubObject("Range()")->dynamicCall("Information(int)", wdNumberOfPagesInDocument).toInt();
        qDebug() << "total_" << total_;
        QAxObject* xWindow = document_->querySubObject("ActiveWindow");
        panes_ = xWindow->querySubObject("Panes(int)", 1);
        emit opened(total_);
        thumb(page_);
    } else {
        emit failed("Open Failed");
    }
}

void Word::thumb(int page)
{
    QAxObject * xPage = nullptr;
    if (panes_) {
        xPage = panes_->querySubObject("Pages(int)", page);
    }
    if (!xPage)
        return;
    QByteArray bits = xPage->dynamicCall("EnhMetaFileBits()").toByteArray();
    char * data = nullptr; int ndata = 0;
    saveGdiImage(bits.data(), bits.size(), &data, &ndata);
    QByteArray bytes(data, ndata);
    delete [] data;
    QPixmap pixmap;
    pixmap.loadFromData(bytes);
    qDebug() << "pixmap" << pixmap;
    pixmap = pixmap.scaledToHeight(pixmap.height() / 2, Qt::SmoothTransformation);
    emit thumbed(pixmap);
}

void Word::jump(int page)
{
    if (document_ && page > 0 && page <= total_) {
        thumb(page_ = page);
    }
}

void Word::next()
{
    if (document_ && page_ < total_) {
        thumb(++page_);
    }
}

void Word::prev()
{
    if (document_ && page_ > 1) {
        thumb(--page_);
    }
}

void Word::close()
{
    if (!document_)
        return;
    qDebug() << "Word::close()";
    document_->dynamicCall("Close()");
    delete document_;
    document_ = nullptr;
    total_ = 0;
    emit closed();
}

void Word::onPropertyChanged(const QString &name)
{
    qDebug() << "onPropertyChanged" << name;
}

void Word::onSignal(const QString &name, int argc, void *argv)
{
    (void) argc;
    (void) argv;
    qDebug() << "onSignal" << name;
}

void Word::onException(int code, const QString &source, const QString &desc, const QString &help)
{
    (void) code;
    (void) source;
    (void) help;
    qDebug() << "onException" << code << source << desc << help;
}
