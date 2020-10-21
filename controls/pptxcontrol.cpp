#include "pptxcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"
//#include "views/itemframe.h"
#include "office/powerpoint.h"
#include "core/workthread.h"
#include "data/urlfilecache.h"

#include <QUrl>
#include <QDir>
#include <QMouseEvent>
#include <QGraphicsTextItem>
#include <QToolButton>
#include <QApplication>
#include <QBuffer>

static char const * toolstr =
        "show()|开始演示;"
        "next()|下一页;"
        "prev()|上一页;"
        "hide()|结束演示";

PptxControl::PptxControl(ResourceView * res)
    : ImageControl(res)
    , stopButton_(nullptr)
{
    powerpoint_ = new PowerPoint;
    QObject::connect(powerpoint_, &PowerPoint::opened, this, &PptxControl::opened);
    QObject::connect(powerpoint_, &PowerPoint::failed, this, &PptxControl::failed);
    QObject::connect(powerpoint_, &PowerPoint::reopened, this, &PptxControl::reopened);
    QObject::connect(powerpoint_, &PowerPoint::thumbed, this, &PptxControl::thumbed);
    QObject::connect(powerpoint_, &PowerPoint::showed, this, &PptxControl::showed);
    QObject::connect(powerpoint_, &PowerPoint::closed, this, &PptxControl::closed);
    (void) toolstr;
#ifdef QT_DEBUG
    setToolsString(toolstr);
#endif
    setProperty("finishIcon", QString(":/showboard/icon/play.svg"));
}

PptxControl::~PptxControl()
{
    powerpoint_->deleteLater();
    powerpoint_ = nullptr;
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
    auto life(this->life());
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

static QUrl getThumbUrl(QUrl url, int slide)
{
    int i = url.path().lastIndexOf('.');
    url.setPath(url.path().replace(i, url.path().length() - i,
                                   QString("_%1.jpg").arg(slide)));
    return url;
}

void PptxControl::open(QUrl const & url)
{
    QUrl thumbUrl = getThumbUrl(url, powerpoint_->slideNumber());
    QByteArray data = Resource::getCache().getData(thumbUrl);
    if (!data.isEmpty()) {
        QPixmap pixmap;
        pixmap.loadFromData(data);
        if (!pixmap.isNull()) {
            powerpoint_->setThumbNumber(powerpoint_->slideNumber());
            powerpoint_->setSize(pixmap.size());
            setProperty("thumb", pixmap);
        }
    }
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
            && res_->property("autoShow").toBool();
    if (first) {
        QObject::connect(stateItem(), &StateItem::clicked, this, [this]() {
            show();
        });
        if (autoShow)
            show();
        QVariant thumb = property("thumb");
        if (!thumb.isNull()) {
            thumbed(thumb.value<QPixmap>());
        }
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
    else
        stopButton_->show();
}

void PptxControl::thumbed(QPixmap pixmap)
{
    if (!pixmap.isNull()) {
        if (flags_ & LoadFinished) {
            if (pixmap.size() != this->pixmap().size()) {
                pixmap = pixmap.scaled(this->pixmap().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        }
        setPixmap(pixmap);
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

extern bool supportTranslucentBackground();

void PptxControl::showStopButton()
{
    QToolButton * button = new QToolButton;
    button->setWindowFlag(Qt::FramelessWindowHint);
    button->setWindowFlag(Qt::SubWindow);
    if (supportTranslucentBackground())
        button->setAttribute(Qt::WA_TranslucentBackground);
    else
        button->setAttribute(Qt::WA_PaintOnScreen);
    button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    button->setStyleSheet("width:72px;height:72px;"
                          "background-color:#F22B3034;border-radius:8px;border:1px solid rgba(67,77,89,1);"
                          "color:#909093;font-family:'Microsoft YaHei';font-size:12px;");
    button->setIconSize({40, 40});
    button->setIcon(QPixmap(":/showboard/icon/return.png"));
    button->setText("返回课堂");
    button->installEventFilter(this);
    button->setVisible(false);
    powerpoint_->attachButton(static_cast<intptr_t>(button->winId()), QPoint(-120, -120));
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
    stopButton_->hide();
    WorkThread::postWork(p, [p]() {
        p->hide();
    });
}

void PptxControl::close()
{
    if (flags_ & LoadFinished) {
        if (!image()->pixmap().isNull()) {
            QBuffer buf;
            buf.open(QIODevice::WriteOnly);
            image()->pixmap().save(&buf, "jpg");
            QUrl thumbUrl = getThumbUrl(res_->property("localUrl").toUrl(), powerpoint_->thumbNumber());
            buf.seek(0);
            Resource::getCache().remove(thumbUrl);
            Resource::getCache().putData(thumbUrl, buf.data());
            buf.close();
        }
    }
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

void PptxControl::sizeChanged()
{
    if (!pixmap().isNull()) {
        QRectF rect = item_->boundingRect();
        stateItem()->setPos(rect.bottomRight() - QPointF(100, 100));
    }
    Control::sizeChanged();
}

bool PptxControl::eventFilter(QObject *obj, QEvent * event)
{
    if (obj != stopButton_)
        return false;
    switch (event->type()) {
    case QEvent::MouseMove: {
        QPoint lpos = obj->property("lastPos").toPoint();
        QPoint pos = static_cast<QMouseEvent*>(event)->pos();
        QPoint d = pos - lpos;
        if (qAbs(d.x()) + qAbs(d.y()) < 10)
            break;
        powerpoint_->moveButton(static_cast<intptr_t>(stopButton_->winId()), d);
        obj->setProperty("moved", true);
    } break;
    case QEvent::MouseButtonPress:
        //static_cast<QToolButton*>(stopButton_)->setIcon(
        //            QPixmap(":/showboard/icon/stop.press.svg"));
        obj->setProperty("lastPos", static_cast<QMouseEvent*>(event)->pos());
        obj->setProperty("moved", false);
        break;
    case QEvent::MouseButtonRelease:
        //static_cast<QToolButton*>(stopButton_)->setIcon(
        //            QPixmap(":/showboard/icon/return.svg"));
        if (!obj->property("moved").toBool())
            hide();
        break;
    case QEvent::Destroy: {
        PowerPoint * p = powerpoint_;
        WorkThread::postWork(p, [p]() {
            p->mayStopped();
        });
    } break;
    default:
        return false;
    }
    return true;
}


