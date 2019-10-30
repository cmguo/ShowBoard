#include "pptxcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"

#include <QAxObject>
#include <QUrl>
#include <QGraphicsTextItem>

extern intptr_t getHwnd(char const * titleParts[]);
extern bool isHwndShown(intptr_t hwnd);

QAxObject * PptxControl::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};

static char const * toolstr =
        "open()|打开|:/showboard/icons/icon_delete.png;"
        "next()|下一页|:/showboard/icons/icon_delete.png;"
        "prev()|上一页|:/showboard/icons/icon_delete.png;"
        "close()|关闭|:/showboard/icons/icon_delete.png";

PptxControl::PptxControl(ResourceView * res)
    : Control(res)
    , presentation_(nullptr)
    , view_(nullptr)
    , startIndex_(0)
{
    if (application_ == nullptr) {
        application_ = new QAxObject("PowerPoint.Application");
        if (application_ == nullptr) {
            application_ = new QAxObject("Kwpp.Application");
            if (application_)
                titleParts[0] = "WPS";
        }
    }
}

PptxControl::~PptxControl()
{
    close();
}

QGraphicsItem * PptxControl::create(ResourceView * res)
{
    QString path = res->url().path();
    name_ = path.mid(path.lastIndexOf('/') + 1);
    return new QGraphicsTextItem(name_ + "(NotStart)");
}

QString PptxControl::toolsString() const
{
    return toolstr;
}

intptr_t PptxControl::hwnd() const
{
    return hwnd_;
}

void PptxControl::open(int page)
{
    if (presentation_)
        return;
    if (page == 0) {
        QVariant slideNumber = res_->property("slideNumber");
        if (slideNumber.isValid())
            page = slideNumber.toInt();
    }
    startIndex_ = page;
    if (!localUrl_.isEmpty()) {
        open_();
        return;
    }
    QWeakPointer<int> life(lifeToken_);
    res_->resource()->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        localUrl_ = url;
        open_();
    });
}

void PptxControl::open_()
{
    QAxObject * presentations = application_->querySubObject("Presentations");
    QObject::connect(presentations, SIGNAL(exception(int,QString,QString,QString)),
                     this, SLOT(onException(int,QString,QString,QString)));
    QAxObject * presentation = presentations->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                localUrl_.toLocalFile(), true, false, false);
    if (presentation) {
        QObject::connect(presentation, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        QAxObject * settings = presentation->querySubObject("SlideShowSettings");
        settings->setProperty("ShowType", "ppShowTypeSpeaker");
        settings->setProperty("ShowMediaControls", "true");
        presentation_ = presentation;
        QAxObject * window = settings->querySubObject("Run()");
        view_ = window->querySubObject("View");
        QObject::connect(view_, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        QGraphicsTextItem * item = static_cast<QGraphicsTextItem*>(item_);
        item->setPlainText(name_ + "(Showing)");
        if (startIndex_)
            jump(startIndex_);
        hwnd_ = getHwnd(titleParts);
        QTimer * timer = new QTimer(this);
        timer->setInterval(500);
        timer->setSingleShot(false);
        timer->start();
        QObject::connect(timer, &QTimer::timeout, this, [this, timer]() {
            if (isHwndShown(hwnd_)) {
                try {
                    QAxObject * slide = view_->querySubObject("Slide");
                    if (slide) {
                        QVariant slideNumber = slide->property("SlideNumber");
                        qDebug() << "slideNumber:" << slideNumber;
                        res_->setProperty("slideNumber", slideNumber);
                    }
                } catch(...) {
                }
            } else {
                close();
                timer->stop();
                timer->deleteLater();
            }
        });
        emit opened();
    }
}

void PptxControl::next()
{
    if (view_)
        view_->dynamicCall("Next()");
}

void PptxControl::jump(int page)
{
    if (view_)
        view_->dynamicCall("GotoSlide(int)", page);
}

void PptxControl::prev()
{
    if (view_)
        view_->dynamicCall("Previous()");
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
    QGraphicsTextItem * item = static_cast<QGraphicsTextItem*>(item_);
    item->setPlainText(name_ + "(Finished)");
    emit closed();
}

void PptxControl::detached()
{
    close();
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
