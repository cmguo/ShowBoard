#include "pptxcontrol.h"
#include "resourceview.h"

#include <QAxObject>
#include <QUrl>
#include <QGraphicsRectItem>

extern intptr_t getHwnd(char const * titleParts[]);

QAxObject * PptxControl::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};

PptxControl::PptxControl(ResourceView * res)
    : Control(res)
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
    if (view_) {
        view_->deleteLater();
        view_ = nullptr;
    }
    if (presentation_) {
        presentation_->deleteLater();
        presentation_ = nullptr;
    }
}

QGraphicsItem * PptxControl::create(ResourceView * res)
{
    QWeakPointer<int> life(lifeToken_);
    res->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        QAxObject * presentations = application_->querySubObject("Presentations");
        QObject::connect(presentations, SIGNAL(exception(int,QString,QString,QString)),
                         this, SLOT(onException(int,QString,QString,QString)));
        QAxObject * presentation = presentations->querySubObject(
                    "Open(const QString&, bool, bool, bool)",
                    url.toLocalFile(), true, false, false);
        if (presentation) {
            QAxObject * settings = presentation->querySubObject("SlideShowSettings");
            settings->setProperty("ShowType", "ppShowTypeSpeaker");
            settings->setProperty("ShowMediaControls", "true");
            presentation_ = presentation;
            QAxObject * window = settings->querySubObject("Run()");
            view_ = window->querySubObject("View");
            delete settings;
        }
    });
    return new QGraphicsRectItem;
}

intptr_t PptxControl::hwnd() const
{
    return getHwnd(titleParts);
}

void PptxControl::show()
{
}

void PptxControl::next()
{
    view_->dynamicCall("Next()");
}

void PptxControl::prev()
{
    view_->dynamicCall("Previous()");
}

void PptxControl::exit()
{
    delete view_;
    presentation_->setProperty("Saved", true);
    presentation_->dynamicCall("Close()");
    delete presentation_;
}

void PptxControl::onPropertyChanged(const QString &name)
{
    qDebug() << "onPropertyChanged" << name;
}

void PptxControl::onSignal(const QString &name, int argc, void *argv)
{
    qDebug() << "onSignal" << name;
}

void PptxControl::onException(int code, const QString &source, const QString &desc, const QString &help)
{
    (void) code;
    (void) source;
    (void) help;
    qDebug() << "onException" << desc;
}
