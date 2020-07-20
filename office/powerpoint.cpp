#include "powerpoint.h"
#include "axthread.h"

#include <QAxObject>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QTimerEvent>
#include <QPainter>

#include <objbase.h>

extern intptr_t findWindow(char const * titleParts[]);
extern bool isWindowValid(intptr_t hwnd);
extern bool isWindowShown(intptr_t hwnd);
extern void showWindow(intptr_t hwnd);
extern void hideWindow(intptr_t hwnd);
extern void setWindowAtTop(intptr_t hwnd);
extern void attachWindow(intptr_t hwndParent, intptr_t hwnd, int left, int top);
extern void moveChildWindow(intptr_t hwndParent, intptr_t hwnd, int dx, int dy);
extern void setArrowCursor();
extern void showCursor();
int captureImage(intptr_t hwnd, char ** out, int * nout);

QAxObject * PowerPoint::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};
static char const * titleParts2[] = {"WPS", "ppt", nullptr};

static AxThread & workThread() {
    static AxThread thread("PowerPoint");
    return thread;
}

class PptObject : public QAxObject
{
public:
    PptObject() {}

    bool setControl(QString const & control)
    {
        name_ = control;
        return QAxObject::setControl(control);
    }

    QString const & name() const { return name_; }

    QString foundControl() {
        return control_;
    }

protected:
    virtual bool initialize(IUnknown** ptr)
    {
        control_ = control();
        return QAxObject::initialize(ptr);
    }

private:
    QString name_;
    QString control_;
};

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
        std::unique_ptr<PptObject> application(new PptObject);
        if (!application->setControl("PowerPoint.Application")) {
            if (application->foundControl().startsWith("{")) {
                emit failed("software|打开失败，请关闭其他演讲文稿后重试");
                return;
            }
            if (application->setControl("Kwpp.Application")) {
                titleParts[0] = nullptr; // not check PowerPoint
            } else {
                if (application->foundControl().startsWith("{")) {
                    emit failed("software|打开失败，请关闭其他演讲文稿后重试");
                    return;
                }
                emit failed("software|未检测到PPT放映软件，请安装Office软件");
                return;
            }
        }
        application_ = application.release();
        if (application_) {
            application_->moveToThread(&workThread());
            workThread().atexit(quit);
        }
    }
    if (titleParts[0]) {
        QString caption = application_->property("Caption").toString();
        if (caption.contains("WPS"))
            titleParts[0] = nullptr;
    }
    if (application_ && !presentations_) {
        presentations_ = application_->querySubObject("Presentations");
    }
    if (!presentations_) {
        emit failed("software|未检测到PPT放映软件，请安装Office软件");
        return;
    }
    file_ = file;
    QObject::connect(presentations_, SIGNAL(exception(int,QString,QString,QString)),
                     this, SLOT(onException(int,QString,QString,QString)));
    QString file2 = file;
    if (titleParts[0])
        file2 = QString("\"%1\"").arg(file);
    QAxObject * presentation = presentations_->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                file2, true, false, false);
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
            delete [] data;
            if (size_ != pixmap.size()) {
                QSize size2 = pixmap.size();
                if (size_.width() * size2.height() > size_.height() * size2.width()) {
                    size2.setHeight(size_.height() * size2.width() / size_.width());
                } else {
                    size2.setWidth(size_.width() * size2.height() / size_.height());
                }
                QPixmap pixmap2(size_);
                QSize offset = (pixmap.size() - size2) / 2;
                QPainter pt(&pixmap2);
                pt.setRenderHint(QPainter::HighQualityAntialiasing);
                pt.drawPixmap(QRectF(0, 0, size_.width(), size_.height()), pixmap,
                              QRectF(QPoint(offset.width(), offset.height()), size2));
                pt.end();
                pixmap = pixmap2;
            }
            emit thumbed(pixmap);
            thumbNumber_ = slideNumber_;
        } else {
            slide = view_->querySubObject("Slide");
        }
    } else {
        slide = presentation_->querySubObject("Slides(int)", page);
    }
    if (!slide)
        return;
    QString file = QDir::tempPath().replace('/', '\\') + "\\showboard.thumb.ppt.jpg";
    slide->dynamicCall("Export(QString, QString, long, long)", file, "JPG");
    QPixmap pixmap(file);
    size_ = pixmap.size();
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
    int ntry = 0;
    if (view_) {
        showWindow(hwnd_);
    } else while (ntry < 2) { // only retry once
        try {
            QAxObject * settings = presentation_->querySubObject("SlideShowSettings");
            if (settings) {
                //settings->setProperty("ShowType", "ppShowTypeSpeaker");
                settings->setProperty("ShowMediaControls", "true");
                settings->setProperty("ShowPresenterView", "false");
            } else  {
                emit failed("Can't get SlideShowSettings!");
                // WPS failed if other show
                ++ntry;
                reopen();
                continue;
            }
            //if (page) // will cause View be null
            //    settings->setProperty("StartingSlide", page);
            QAxObject * window = settings->querySubObject("Run()");
            if (window) {
                view_ = window->querySubObject("View");
            }
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
            break;
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
    moveChildWindow(hwnd_, hwnd, diff.x(), diff.y());
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
    qDebug() << "hide";
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
            mayStopped();
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

void PowerPoint::quit()
{
    qDebug() << "Word::quit()";
    if (application_) {
        application_->dynamicCall("Quit()");
        delete application_;
        application_ = nullptr;
    }
}
