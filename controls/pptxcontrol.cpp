#include "pptxcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"

#include <QAxObject>
#include <QUrl>
#include <QGraphicsTextItem>
#include <QDir>

extern intptr_t findWindow(char const * titleParts[]);
extern bool isWindowShown(intptr_t hwnd);
extern void showWindow(intptr_t hwnd);
extern void hideWindow(intptr_t hwnd);
extern void setWindowAtTop(intptr_t hwnd);

QAxObject * PptxControl::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};

static char const * toolstr =
        "show()|开始演示|:/showboard/icons/icon_delete.png;"
        "next()|下一页|:/showboard/icons/icon_delete.png;"
        "prev()|上一页|:/showboard/icons/icon_delete.png;"
        "hide()|结束演示|:/showboard/icons/icon_delete.png";

PptxControl::PptxControl(ResourceView * res)
    : Control(res, {KeepAspectRatio, FullSelect})
    , page_(0)
    , presentation_(nullptr)
    , view_(nullptr)
    , hwnd_(0)
{
    if (application_ == nullptr) {
        application_ = new QAxObject("PowerPoint.Application");
        if (application_ == nullptr) {
            application_ = new QAxObject("Kwpp.Application");
            if (application_)
                titleParts[0] = "WPS";
        }
    }
    if (application_) {
        presentations_ = application_->querySubObject("Presentations");
        QObject::connect(presentations_, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
    }
}

PptxControl::~PptxControl()
{
    close();
    if (presentations_) {
        delete presentations_;
        presentations_ = nullptr;
    }
}

QGraphicsItem * PptxControl::create(ResourceView * res)
{
    QString path = res->url().path();
    name_ = path.mid(path.lastIndexOf('/') + 1);
    QVariant slideNumber = res->property("slideNumber");
    if (slideNumber.isValid())
        page_ = slideNumber.toInt();
    else
        page_ = 1;
    return new QGraphicsPixmapItem;
}


QString PptxControl::toolsString() const
{
    return toolstr;
}

void PptxControl::attaching()
{
    Control::attaching();
    open();
}

intptr_t PptxControl::hwnd() const
{
    return hwnd_;
}

void PptxControl::open()
{
    if (presentation_)
        return;
    QVariant localUrl = res_->property("localUrl");
    if (localUrl.isValid()) {
        open(localUrl.toUrl());
        return;
    }
    QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
    item->setPixmap(QPixmap(":/showboard/icons/icon_delete.png"));
    QWeakPointer<int> life(lifeToken_);
    res_->resource()->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        res_->setProperty("localUrl", url);
        open(url);
    });
}

void PptxControl::open(QUrl const & url)
{
    if (!presentations_)
        return;
    QAxObject * presentation = presentations_->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                url.toLocalFile(), true, false, false);
    if (presentation) {
        QObject::connect(presentation, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        presentation_ = presentation;
        total_ = presentation_->querySubObject("Slides")->property("Count").toInt();
        thumb(page_);
        emit opened();
    }
}

void PptxControl::reopen()
{
    view_ = nullptr;
    hwnd_ = 0;
    presentation_ = nullptr;
    open(); // reopen
}

void PptxControl::thumb(int page)
{
    QAxObject * slide = nullptr;
    if (page == 0) {
        slide = view_->querySubObject("Slide");
    } else {
        slide = presentation_->querySubObject("Slides(int)", page);
    }
    if (!slide)
        return;
    QString file = QDir::tempPath().replace('/', '\\') + "\\showboard.thumb.ppt.jpg";
    slide->dynamicCall("Export(QString, QString)", file, "JPG");
    QPixmap pixmap(file);
    if (!pixmap.isNull()) {
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        if (!view_) {
            item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
            sizeChanged(pixmap.size());
        }
    }
}

void PptxControl::show(int page)
{
    if (!presentation_)
        return;
    if (page == 0) {
        page = page_;
    }
    if (view_) {
        showWindow(hwnd_);
    } else {
        QAxObject * settings = presentation_->querySubObject("SlideShowSettings");
        //settings->setProperty("ShowType", "ppShowTypeSpeaker");
        settings->setProperty("ShowMediaControls", "true");
        //if (page) // will cause View be null
        //    settings->setProperty("StartingSlide", page);
        QAxObject * window = settings->querySubObject("Run()");
        view_ = window->querySubObject("View");
        QObject::connect(view_, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        hwnd_ = findWindow(titleParts);
    }
    if (page)
        jump(page);
    setWindowAtTop(hwnd_);
    QTimer * timer = new QTimer(this);
    timer->setInterval(500);
    timer->setSingleShot(false);
    timer->start();
    QObject::connect(timer, &QTimer::timeout, this, [this, timer]() {
        if (isWindowShown(hwnd_)) {
            try {
                QAxObject * slide = view_->querySubObject("Slide");
                if (slide) {
                    QVariant slideNumber = slide->property("SlideNumber");
                    if (slideNumber.isValid()) {
                        page_ = slideNumber.toInt();
                    }
                }
            } catch(...) {
            }
        } else {
            timer->stop();
            timer->deleteLater();
            reopen();
        }
    });
}

void PptxControl::next()
{
    if (view_) {
        view_->dynamicCall("Next()");
        thumb(0);
    } else if (presentation_ && page_ < total_) {
        thumb(++page_);
    }
}

void PptxControl::jump(int page)
{
    if (view_) {
        view_->dynamicCall("GotoSlide(int)", page);
        thumb(0);
    } else if (presentation_ && page_ > 0 && page_ <= total_) {
        thumb(page_ = page);
    }
}

void PptxControl::prev()
{
    if (view_) {
        view_->dynamicCall("Previous()");
        thumb(0);
    } else if (presentation_ && page_ > 1) {
        thumb(--page_);
    }
}

void PptxControl::hide()
{
    thumb(page_);
    hideWindow(hwnd_);
}

void PptxControl::close()
{
    if (!presentation_)
        return;
    delete view_;
    view_ = nullptr;
    presentation_->setProperty("Saved", true);
    presentation_->dynamicCall("Close()");
    delete presentation_;
    presentation_ = nullptr;
    total_ = 0;
    emit closed();
}

void PptxControl::detached()
{
    close();
    res_->setProperty("slideNumber", page_);
    Control::detached();
}

void PptxControl::onPropertyChanged(const QString &name)
{
    qDebug() << "onPropertyChanged" << name;
}

void PptxControl::onSignal(const QString &name, int argc, void *argv)
{
    (void) argc;
    (void) argv;
    qDebug() << "onSignal" << name;
}

void PptxControl::onException(int code, const QString &source, const QString &desc, const QString &help)
{
    (void) code;
    (void) source;
    (void) help;
    qDebug() << "onException" << desc;
}
