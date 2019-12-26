#include "word.h"
#include "workthread.h"

#include <QAxObject>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include <Windows.h>

extern bool saveGdiImage(char* data, int size, wchar_t * file);

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
    , total_(0)
    , page_(1)
    , view_(nullptr)
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
static constexpr int wdGoToPage = 1;
static constexpr int wdGoToFirst = 1;

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
        emit opened(total_);
        thumb(page_);
    } else {
        emit failed("Open Failed");
    }
}

void Word::reopen()
{
    view_ = nullptr;
    document_ = nullptr;
    open(file_); // reopen
    emit reopened();
}

void Word::thumb(int page)
{
    QAxObject * xPage = nullptr;
    if (page == 0) {
        //range = view_->querySubObject("Slide");
    } else {
        //QAxObject* selection = application_->querySubObject("Selection");
        QAxObject* xWindow = document_->querySubObject("ActiveWindow");
        qDebug() << "xWindow" << xWindow;
        QAxObject* xPane = xWindow->querySubObject("Panes(int)", 1);
        qDebug() << "xPane" << xPane;
        xPage = xPane->querySubObject("Pages(int)", page);
        qDebug() << "xPage" << xPage;
        //QAxObject* xRectangle = xPage->querySubObject("Rectangles(int)", 1);
        //qDebug() << "xRectangle" << xRectangle;
        //QAxObject* range1 = document_->querySubObject("GoTo(int, int, int)", wdGoToPage, wdGoToFirst, page);
        //QAxObject* range2 = document_->querySubObject("GoTo(int, int, int)", wdGoToPage, wdGoToFirst, page + 1);
        //range = xRectangle->querySubObject("Range");
        //qDebug() << "range" << range;
    }
    if (!xPage)
        return;
    QByteArray bits = xPage->dynamicCall("EnhMetaFileBits()").toByteArray();
    QString file = QDir::tempPath().replace('/', '\\') + "\\showboard.thumb.word.png";
    saveGdiImage(bits.data(), bits.size(), reinterpret_cast<wchar_t*>(file.data()));
    QPixmap pixmap(file);
    qDebug() << "pixmap" << pixmap;
    emit thumbed(pixmap);
}

void Word::show(int page)
{
    if (!document_)
        return;
    if (page == 0) {
        page = page_;
    } else {
        page_ = page;
    }
    if (view_) {
    } else {
        QAxObject * settings = document_->querySubObject("SlideShowSettings");
        //settings->setProperty("ShowType", "ppShowTypeSpeaker");
        settings->setProperty("ShowMediaControls", "true");
        //if (page) // will cause View be null
        //    settings->setProperty("StartingSlide", page);
        QAxObject * window = settings->querySubObject("Run()");
        view_ = window->querySubObject("View");
        QObject::connect(view_, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
    }
    if (page)
        jump(page);
    emit showed();
}

void Word::jump(int page)
{
    if (view_) {
        view_->dynamicCall("GotoSlide(int)", page);
        thumb(0);
    } else if (document_ && page > 0 && page <= total_) {
        thumb(page_ = page);
    }
}

void Word::next()
{
    if (view_) {
        view_->dynamicCall("Next()");
        thumb(0);
    } else if (document_ && page_ < total_) {
        thumb(++page_);
    }
}

void Word::prev()
{
    if (view_) {
        view_->dynamicCall("Previous()");
        thumb(0);
    } else if (document_ && page_ > 1) {
        thumb(--page_);
    }
}

void Word::hide()
{
    thumb(page_);
}

void Word::close()
{
    if (!document_)
        return;
    qDebug() << "Word::close()";
    view_ = nullptr;
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
