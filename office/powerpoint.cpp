#include "powerpoint.h"

#include <QAxObject>
#include <QThread>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QTimerEvent>

#include <objbase.h>

extern intptr_t findWindow(char const * titleParts[]);
extern bool isWindowShown(intptr_t hwnd);
extern void showWindow(intptr_t hwnd);
extern void hideWindow(intptr_t hwnd);
extern void setWindowAtTop(intptr_t hwnd);
extern void attachWindow(intptr_t hwndParent, intptr_t hwnd, int left, int top);
extern void moveChildWindow(intptr_t hwnd, int dx, int dy);
extern void setArrowCursor();

QAxObject * PowerPoint::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};

class WorkThread : public QThread
{
public:
    WorkThread(char const * name = nullptr)
    {
        if (name)
            setObjectName(name);
        start();
    }
    virtual ~WorkThread() override
    {
        quit();
        wait();
    }
private:
    virtual void run() override
    {
        CoInitialize(nullptr);
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        exec();
        CoUninitialize();
    }
};

static QThread & workThread() {
    static WorkThread thread("PowerPoint");
    return thread;
}

PowerPoint::PowerPoint(QObject * parent)
    : QObject(parent)
    , presentations_(nullptr)
    , presentation_(nullptr)
    , total_(0)
    , slideNumber_(1)
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
        application_ = new QAxObject("PowerPoint.Application");
        if (application_ == nullptr) {
            application_ = new QAxObject("Kwpp.Application");
            if (application_)
                titleParts[0] = "WPS";
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
        emit failed("No PPT Application");
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
        thumb(slideNumber_);
    } else {
        emit failed("Open Failed");
    }
}

void PowerPoint::reopen()
{
    view_ = nullptr;
    hwnd_ = 0;
    presentation_ = nullptr;
    open(file_); // reopen
    emit reopened();
}

void PowerPoint::thumb(int page)
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
    slide->dynamicCall("Export(QString, QString, long, long)", file, "JPG", 320, 180);
    QPixmap pixmap(file);
    emit thumbed(pixmap);
    setArrowCursor();
}

void PowerPoint::show(int page)
{
    if (!presentation_)
        return;
    if (page == 0) {
        page = slideNumber_;
    } else {
        slideNumber_ = page;
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
    intptr_t hwnd = hwnd_;
    postWork(&workThread(), [hwnd](){
        setWindowAtTop(hwnd);
    });
    emit showed();
    timerId_ = startTimer(500);
}

void PowerPoint::attachButton(intptr_t hwnd)
{
    attachWindow(hwnd, hwnd_, -72, -72);
}

void PowerPoint::moveButton(intptr_t hwnd, const QPoint &diff)
{
    moveChildWindow(hwnd, diff.x(), diff.y());
}

void PowerPoint::jump(int page)
{
    if (view_) {
        view_->dynamicCall("GotoSlide(int)", page);
        thumb(0);
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
    hideWindow(hwnd_);
    thumb(slideNumber_);
}

void PowerPoint::close()
{
    if (!presentation_)
        return;
    qDebug() << "PowerPoint::close()";
    view_ = nullptr;
    hwnd_ = 0;
    presentation_->dynamicCall("Close()");
    delete presentation_;
    presentation_ = nullptr;
    total_ = 0;
    emit closed();
}

void PowerPoint::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == timerId_) {
        if (isWindowShown(hwnd_)) {
            try {
                QAxObject * slide = view_->querySubObject("Slide");
                if (slide) {
                    QVariant slideNumber = slide->property("SlideNumber");
                    if (slideNumber.isValid()) {
                        slideNumber_ = slideNumber.toInt();
                    }
                }
            } catch(...) {
            }
        } else {
            killTimer(event->timerId());
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
    qDebug() << "onSignal" << name;
}

void PowerPoint::onException(int code, const QString &source, const QString &desc, const QString &help)
{
    (void) code;
    (void) source;
    (void) help;
    qDebug() << "onException" << desc;
}
