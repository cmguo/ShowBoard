#include "pptxcontrol.h"
#include "resourceview.h"

#include <QAxObject>
#include <QUrl>
#include <QGraphicsRectItem>

extern intptr_t getHwnd(char const * titleParts[]);

QAxObject * PptxControl::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};

static char const * toolstr =
        "show|打开|:/showboard/icons/icon_delete.png;"
        "next|下一页|:/showboard/icons/icon_delete.png;"
        "prev|上一页|:/showboard/icons/icon_delete.png;"
        "exit|关闭|:/showboard/icons/icon_delete.png";

PptxControl::PptxControl(ResourceView * res)
    : Control(res)
    , presentation_(nullptr)
    , view_(nullptr)
{
    if (application_ == nullptr) {
        application_ = new QAxObject("PowerPoint.Application");
        if (application_ == nullptr) {
            application_ = new QAxObject("Kwpp.Application");
            if (application_)
                titleParts[0] = "WPS";
        }
        QObject::connect(application_, SIGNAL(signal(QString,int,void*)),
                         this, SLOT(onSignal(QString,int,void*)));
    }
}

PptxControl::~PptxControl()
{
    exit();
}

QGraphicsItem * PptxControl::create(ResourceView * res)
{
    (void) res;
    return new QGraphicsRectItem(QRectF(-40, -40, 80, 80));
}

QString PptxControl::toolsString() const
{
    return toolstr;
}

intptr_t PptxControl::hwnd() const
{
    return getHwnd(titleParts);
}

void PptxControl::show()
{
    if (presentation_)
        return;
    if (!localUrl_.isEmpty()) {
        open();
        return;
    }
    QWeakPointer<int> life(lifeToken_);
    res_->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        localUrl_ = url;
        open();
    });
}

void PptxControl::open()
{
    QAxObject * presentations = application_->querySubObject("Presentations");
    QObject::connect(presentations, SIGNAL(exception(int,QString,QString,QString)),
                     this, SLOT(onException(int,QString,QString,QString)));
    QAxObject * presentation = presentations->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                localUrl_.toLocalFile(), true, false, false);
    if (presentation) {
        QObject::connect(presentation, SIGNAL(signal(QString,int,void*)),
                         this, SLOT(onSignal(QString,int,void*)));
        QAxObject * settings = presentation->querySubObject("SlideShowSettings");
        settings->setProperty("ShowType", "ppShowTypeSpeaker");
        settings->setProperty("ShowMediaControls", "true");
        presentation_ = presentation;
        QAxObject * window = settings->querySubObject("Run()");
        view_ = window->querySubObject("View");
    }
}

void PptxControl::next()
{
    if (view_)
        view_->dynamicCall("Next()");
}

void PptxControl::prev()
{
    if (view_)
        view_->dynamicCall("Previous()");
}

void PptxControl::exit()
{
    if (!presentation_)
        return;
    delete view_;
    view_ = nullptr;
    presentation_->setProperty("Saved", true);
    presentation_->dynamicCall("Close()");
    delete presentation_;
    presentation_ = nullptr;
}

void PptxControl::detach()
{
    exit();
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
