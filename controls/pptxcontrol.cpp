#include "pptxcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"
//#include "views/itemframe.h"
#include "office/powerpoint.h"
#include "office/workthread.h"

#include <QUrl>
#include <QDir>
#include <QMouseEvent>
#include <QAxObject>
#include <QGraphicsTextItem>
#include <QToolButton>
#include <QApplication>

static char const * toolstr =
        "show()|开始演示|:/showboard/icons/icon_delete.png;"
        "next()|下一页|:/showboard/icons/icon_delete.png;"
        "prev()|上一页|:/showboard/icons/icon_delete.png;"
        "hide()|结束演示|:/showboard/icons/icon_delete.png";

PptxControl::PptxControl(ResourceView * res)
    : Control(res, {KeepAspectRatio})
    , stopButton_(nullptr)
{
    powerpoint_ = new PowerPoint;
    QObject::connect(powerpoint_, &PowerPoint::opened, this, &PptxControl::opened);
    QObject::connect(powerpoint_, &PowerPoint::failed, this, &PptxControl::failed);
    QObject::connect(powerpoint_, &PowerPoint::reopened, this, &PptxControl::reopened);
    QObject::connect(powerpoint_, &PowerPoint::thumbed, this, &PptxControl::thumbed);
    QObject::connect(powerpoint_, &PowerPoint::showed, this, &PptxControl::showed);
    QObject::connect(powerpoint_, &PowerPoint::closed, this, &PptxControl::closed);
}

PptxControl::~PptxControl()
{
    close();
    powerpoint_->deleteLater();
    powerpoint_ = nullptr;
}

QGraphicsItem * PptxControl::create(ResourceView * res)
{
    (void) res;
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem;
    item->setTransformationMode(Qt::SmoothTransformation);
    return item;
}

void PptxControl::attaching()
{
    //itemFrame()->addDockItem(ItemFrame::Right, 100, Qt::red);
}

void PptxControl::attached()
{
    open();
}

void PptxControl::open()
{
    QVariant localUrl = res_->property("localUrl");
    if (localUrl.isValid()) {
        open(localUrl.toUrl());
        return;
    }
    QWeakPointer<int> life(this->life());
    res_->resource()->getLocalUrl().then([this, life](QUrl const & url) {
        if (life.isNull())
            return;
        res_->setProperty("localUrl", url);
        open(url);
    }).fail([this, life](std::exception & e) {
        if (life.isNull())
            return;
        loadFinished(false, e.what());
    });
}

void PptxControl::open(QUrl const & url)
{
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p, url]() {
       p->open(url.toLocalFile());
    });
}

void PptxControl::opened(int total)
{
    (void) total;
    bool first = (flags_ & LoadFinished) == 0;
    bool autoShow = !(flags_ & RestoreSession)
            && property("autoShow").toBool();
    if (first) {
        QObject::connect(stateItem(), &StateItem::clicked, this, [this]() {
            show();
        });
        if (autoShow)
            show();
    }
}

void PptxControl::failed(QString const & msg)
{
    loadFinished(false, msg);
}

void PptxControl::reopened()
{
    if (stopButton_) {
        delete stopButton_;
        stopButton_ = nullptr;
    }
}

void PptxControl::showed()
{
    if (!stopButton_)
        showStopButton();
}

void PptxControl::thumbed(QPixmap pixmap)
{
    if (!pixmap.isNull()) {
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
        bool first = (flags_ & LoadFinished) == 0;
        if (first) {
            loadFinished(true, QString(":/showboard/icons/play.svg"));
            stateItem()->setPos(item_->boundingRect().bottomRight() - QPointF(100, 100));
        }
    }
}

void PptxControl::closed()
{
}

int PptxControl::slideNumber()
{
    return powerpoint_->slideNumber();
}

void PptxControl::setSlideNumber(int n)
{
    powerpoint_->setSlideNumber(n);
}

void PptxControl::show(int page)
{
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p, page]() {
        p->show(page);
    });
}

void PptxControl::showStopButton()
{
    QToolButton * button = new QToolButton;
    button->setWindowFlag(Qt::FramelessWindowHint);
    button->setWindowFlag(Qt::SubWindow);
    button->setAttribute(Qt::WA_TranslucentBackground);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //button->setStyleSheet("background-color:#80000000;");
    button->setIconSize({48, 48});
    button->setIcon(QPixmap(":/showboard/icons/stop.normal.svg"));
    button->installEventFilter(this);
    button->setVisible(false);
    powerpoint_->attachButton(static_cast<intptr_t>(button->winId()));
    button->setVisible(true);
    stopButton_ = button;
}

void PptxControl::next()
{
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p]() {
        p->next();
    });
}

void PptxControl::jump(int page)
{
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p, page]() {
        p->jump(page);
    });
}

void PptxControl::prev()
{
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p]() {
        p->prev();
    });
}

void PptxControl::hide()
{
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p]() {
        p->hide();
    });
}

void PptxControl::close()
{
    if (stopButton_) {
        delete stopButton_;
        stopButton_ = nullptr;
    }
    PowerPoint * p = powerpoint_;
    WorkThread::postWork(p, [p]() {
        p->close();
    });
}

void PptxControl::detached()
{
    close();
}

bool PptxControl::eventFilter(QObject *obj, QEvent * event)
{
    if (obj != stopButton_)
        return false;
    switch (event->type()) {
    case QEvent::MouseMove: {
        QPoint lpos = obj->property("lastPos").toPoint();
        QPoint pos = static_cast<QMouseEvent*>(event)->pos();
        powerpoint_->moveButton(static_cast<intptr_t>(stopButton_->winId()),
                        pos - lpos);
        obj->setProperty("moved", true);
    } break;
    case QEvent::MouseButtonPress:
        static_cast<QToolButton*>(stopButton_)->setIcon(
                    QPixmap(":/showboard/icons/stop.press.svg"));
        obj->setProperty("lastPos", static_cast<QMouseEvent*>(event)->pos());
        obj->setProperty("moved", false);
        break;
    case QEvent::MouseButtonRelease:
        static_cast<QToolButton*>(stopButton_)->setIcon(
                    QPixmap(":/showboard/icons/stop.normal.svg"));
        if (!obj->property("moved").toBool())
            hide();
        break;
    default:
        return false;
    }
    return true;
}

QString PptxControl::toolsString(QString const & parent) const
{
    (void) parent;
    (void) toolstr;
    return "";//toolstr;
}

