#include "pptxcontrol.h"
#include "resourceview.h"

#include <QAxObject>
#include <QUrl>
#include <QGraphicsRectItem>

extern intptr_t getHwnd(char const * titleParts[]);

QAxObject * PptxControl::application_ = nullptr;

PptxControl::PptxControl(ResourceView * res)
    : Control(res)
{
    if (application_ == nullptr)
        application_ = new QAxObject("PowerPoint.Application");
}

QGraphicsItem * PptxControl::create(ResourceView * res)
{
    QAxObject * presentations = application_->querySubObject("Presentations");
    QAxObject * presentation = presentations->querySubObject(
                "Open(const QString&, bool, bool, bool)",
                res->url().toLocalFile(), true, false, false);
    QAxObject * settings = presentation->querySubObject("SlideShowSettings");
    settings->setProperty("ShowType", "ppShowTypeSpeaker");
    settings->setProperty("ShowMediaControls", "true");
    presentation_ = presentation;
    window_ = settings->querySubObject("Run()");
    view_ = window_->querySubObject("View");
    delete settings;
    return new QGraphicsRectItem;
}

intptr_t PptxControl::hwnd() const
{
    char const * titleParts[] = {"PowerPoint", "ppt", nullptr};
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
    delete window_;
    presentation_->setProperty("Saved", true);
    presentation_->dynamicCall("Close()");
    delete presentation_;
}
