#include "powerpoint.h"
#include "axthread.h"

#include <QAxObject>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QTimerEvent>

#include <objbase.h>

extern intptr_t findWindow(char const * titleParts[]);
extern bool isWindowValid(intptr_t hwnd);
extern bool isWindowShown(intptr_t hwnd);
extern void showWindow(intptr_t hwnd);
extern void hideWindow(intptr_t hwnd);
extern void setWindowAtTop(intptr_t hwnd);
extern void attachWindow(intptr_t hwndParent, intptr_t hwnd, int left, int top);
extern void moveChildWindow(intptr_t hwnd, int dx, int dy);
extern void setArrowCursor();
extern void showCursor();
int captureImage(intptr_t hwnd, char ** out, int * nout);

QAxObject * PowerPoint::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};
static char const * titleParts2[] = {"WPS", "ppt", nullptr};

static QThread & workThread() {
    static AxThread thread("PowerPoint");
    return thread;
}

PowerPoint::PowerPoint(QObject * parent)
    : QObject(parent)
    , presentations_(nullptr)
    , presentation_(nullptr)
    , total_(0)
    , slideNumber_(1)
    , thumbNumber_(0)
    , view_(nullptr)
    , hwnd_(0)
    , timerId_(0)
{
    moveToThread(&workThread());
}

PowerPoint::~PowerPoint()
{
    if (presentations_)
        delete presentations_;
    presentations_ = nullptr;
}

void PowerPoint::open(QString const & file)
{
    if (application_ == nullptr) {
        application_ = new QAxObject;
        if (!application_->setControl("PowerPoint.Application")) {
            if (application_->setControl("Kwpp.Application")) {
                titleParts[0] = nullptr; // not check PowerPoint
            } else {
                delete application_;
                application_ = nullptr;
            }
        }
        if (application_) {
            application_->moveToThread(&workThread());
        }
    }
    if (application_ && !presentations_) {
        presentations_ = application_->querySubObject("Presentations");
        QObject::connect(presentations_, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
    }
    if (!presentations_) {
        emit failed("software|未检测到PPT放映软件，请安装Office软件");
        return;
    }
    file_ = file;
    QAxObject * presentation = presentations_->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                file, true, false, false);
    if (presentation) {
        QObject::connect(presentation, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        presentation_ = presentation;
        total_ = presentation_->querySubObject("Slides")->property("Count").toInt();
        emit opened(total_);
        if (thumbNumber_ != slideNumber_)
            thumb(slideNumber_);
    } else {
        emit failed("打开失败，请联系技术支持");
    }
}

void PowerPoint::reopen()
{
    close();
    open(file_); // reopen
    showCursor();
    emit reopened();
}

void PowerPoint::thumb(int page)
{
    QAxObject * slide = nullptr;
    if (page == 0) {
        if (isWindowShown(hwnd_)) {
            char * data = nullptr;
            int size = 0;
            captureImage(hwnd_, &data, & size);
            QPixmap pixmap;
            pixmap.loadFromData(reinterpret_cast<uchar*>(data), static_cast<uint>(size));
            emit thumbed(pixmap);
            thumbNumber_ = slideNumber_;
            setArrowCursor();
        } else {
            slide = view_->querySubObject("Slide");
        }
    } else {
        slide = presentation_->querySubObject("Slides(int)", page);
    }
    if (!slide)
        return;
    QString file = QDir::tempPath().replace('/', '\\') + "\\showboard.thumb.ppt.jpg";
    slide->dynamicCall("Export(QString, QString, long, long)", file, "JPG", 320, 180);
    QPixmap pixmap(file);
    emit thumbed(pixmap);
    thumbNumber_ = slideNumber_;
    setArrowCursor();
}

void PowerPoint::show(int page)
{
    if (!presentation_)
        return;
    qDebug() << "show" << page;
    if (page == 0) {
        //page = slideNumber_;
    } else {
        slideNumber_ = page;
    }
    if (view_) {
        showWindow(hwnd_);
    } else {
        try {
            QAxObject * settings = presentation_->querySubObject("SlideShowSettings");
            if (settings) {
                //settings->setProperty("ShowType", "ppShowTypeSpeaker");
                settings->setProperty("ShowMediaControls", "true");
                settings->setProperty("ShowPresenterView", "false");
            } else {
                emit failed("Can't get SlideShowSettings!");
                return;
            }
            //if (page) // will cause View be null
            //    settings->setProperty("StartingSlide", page);
            QAxObject * window = settings->querySubObject("Run()");
            if (window)
                view_ = window->querySubObject("View");
            if (view_) {
                QObject::connect(view_, SIGNAL(exception(int,QString,QString,QString)),
                                 this, SLOT(onException(int,QString,QString,QString)));
                QFileInfo fi(file_);
                QByteArray name = fi.baseName().left(8).toLocal8Bit();
                if (titleParts[0]) {
                    titleParts[1] = name;
                    hwnd_ = findWindow(titleParts);
                }
                // WPS will imitate Microsoft office sometime, so we find WPS window also
                if (hwnd_ == 0) {
                    titleParts2[1] = name;
                    hwnd_ = findWindow(titleParts2);
                }
            }
            if (hwnd_ == 0) {
                emit failed(view_ ? "Can't find play window!" : "Can't start play view!");
                return;
            }
            qDebug() << "Found play window" << reinterpret_cast<void *>(hwnd_);
        } catch (...) {
        }
        if (page == 0) {
            page = slideNumber_;
        }
    }
    if (page)
        jump(page);
    intptr_t hwnd = hwnd_;
    WorkThread::postWork(&workThread(), [hwnd](){
        setWindowAtTop(hwnd);
    });
    emit showed();
    if (view_)
        timerId_ = startTimer(500);
}

void PowerPoint::attachButton(intptr_t hwnd, const QPoint & pos)
{
    attachWindow(hwnd, hwnd_, pos.x(), pos.y());
}

void PowerPoint::moveButton(intptr_t hwnd, const QPoint &diff)
{
    moveChildWindow(hwnd, diff.x(), diff.y());
}

void PowerPoint::jump(int page)
{
    if (view_) {
        view_->dynamicCall("GotoSlide(int)", page);
        if (slideNumber_ != page) {
            slideNumber_ = page;
            thumb(0);
        }
    } else if (presentation_ && page > 0 && page <= total_) {
        thumb(slideNumber_ = page);
    }
}

void PowerPoint::next()
{
    if (view_) {
        view_->dynamicCall("Next()");
        thumb(0);
    } else if (presentation_ && slideNumber_ < total_) {
        thumb(++slideNumber_);
    }
}

void PowerPoint::prev()
{
    if (view_) {
        view_->dynamicCall("Previous()");
        thumb(0);
    } else if (presentation_ && slideNumber_ > 1) {
        thumb(--slideNumber_);
    }
}

void PowerPoint::hide()
{
    killTimer(timerId_);
    timerId_ = 0;
    thumb(0);
    //hideWindow(hwnd_);
    //showCursor();
    reopen();
}

void PowerPoint::close()
{
    if (!presentation_)
        return;
    killTimer(timerId_);
    timerId_ = 0;
    qDebug() << "PowerPoint close";
    view_ = nullptr;
    hwnd_ = 0;
    presentation_->dynamicCall("Close()");
    delete presentation_;
    presentation_ = nullptr;
    total_ = 0;
    emit closed();
}

void PowerPoint::mayStopped()
{
    if (view_) {
        qDebug() << "PowerPoint mayStopped";
        reopen();
    }
}

void PowerPoint::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == timerId_) {
        if (isWindowValid(hwnd_)) {
            try {
                QAxObject * slide = view_->querySubObject("Slide");
                if (slide) {
                    QVariant slideNumber = slide->property("SlideNumber");
                    if (!slideNumber.isNull()) {
                        slideNumber_ = slideNumber.toInt();
                    }
                }
            } catch(...) {
            }
        } else {
            reopen();
        }
    }
}

void PowerPoint::onPropertyChanged(const QString &name)
{
    qDebug() << "onPropertyChanged" << name;
}

void PowerPoint::onSignal(const QString &name, int argc, void *argv)
{
    (void) argc;
    (void) argv;
    qDebug() << "PowerPoint onSignal" << name;
}

void PowerPoint::onException(int code, const QString &source, const QString &desc, const QString &help)
{
    (void) code;
    (void) source;
    (void) help;
    qDebug() << "PowerPoint onException" << desc;
}
