#include "pptxcontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"

#include <QUrl>
#include <QDir>
#include <QMouseEvent>
#include <QAxObject>
#include <QGraphicsTextItem>
#include <QToolButton>

extern intptr_t findWindow(char const * titleParts[]);
extern bool isWindowShown(intptr_t hwnd);
extern void showWindow(intptr_t hwnd);
extern void hideWindow(intptr_t hwnd);
extern void setWindowAtTop(intptr_t hwnd);
extern void attachWindow(intptr_t hwndParent, intptr_t hwnd, int left, int top);
extern void moveChildWindow(intptr_t hwnd, int dx, int dy);

QAxObject * PptxControl::application_ = nullptr;

static char const * titleParts[] = {"PowerPoint", "ppt", nullptr};

static char const * toolstr =
        "show()|开始演示|:/showboard/icons/icon_delete.png;"
        "next()|下一页|:/showboard/icons/icon_delete.png;"
        "prev()|上一页|:/showboard/icons/icon_delete.png;"
        "hide()|结束演示|:/showboard/icons/icon_delete.png";

static constexpr int ITEM_KEY_TIMER = 1100;

PptxControl::PptxControl(ResourceView * res)
    : Control(res, {KeepAspectRatio})
    , slideNumber_(1)
    , presentation_(nullptr)
    , view_(nullptr)
    , hwnd_(0)
    , stateItem_(nullptr)
    , stopButton_(nullptr)
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
    QGraphicsItem * item = new QGraphicsPixmapItem;
    StateItem * state = new StateItem(item); // state
    QObject::connect(state, &StateItem::clicked, this, [this]() {
        show();
    });
    stateItem_ = state;
    return item;
}


void PptxControl::attached()
{
    open();
}

void PptxControl::open()
{
    if (presentation_)
        return;
    static_cast<StateItem*>(stateItem_)->setSvgFile(
                QString(":/showboard/icons/loading.svg"), 45.0);
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
        thumb(slideNumber_);
        static_cast<StateItem*>(stateItem_)->setSvgFiles(
                    QString(":/showboard/icons/play.normal.svg"),
                    QString(":/showboard/icons/play.press.svg"));
        emit opened();
    }
}

void PptxControl::reopen()
{
    view_ = nullptr;
    hwnd_ = 0;
    if (stopButton_) {
        delete stopButton_;
        stopButton_ = nullptr;
    }
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
    //slide->dynamicCall("Export(QString, QString, long, long)", file, "JPG", 320, 180);
    QPixmap pixmap(file);
    if (!pixmap.isNull()) {
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        if (!view_) {
            item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
            initScale(pixmap.size());
        }
    }
}

void PptxControl::show(int page)
{
    if (!presentation_)
        return;
    if (page == 0) {
        page = slideNumber_;
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
    int timer = startTimer(500);
    item_->setData(ITEM_KEY_TIMER, timer);
}

void PptxControl::showReturnButton()
{
    QToolButton * button = new QToolButton;
    button->setWindowFlag(Qt::FramelessWindowHint);
    button->setAttribute(Qt::WA_TranslucentBackground);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setStyleSheet("background-color:#80000000;");
    button->setIconSize({48, 48});
    button->setIcon(QPixmap(":/showboard/icons/stop.normal.svg"));
    button->installEventFilter(this);
    button->setVisible(true);
    attachWindow(static_cast<intptr_t>(button->winId()),
                 hwnd_, -72, -72);
    stopButton_ = button;
}

void PptxControl::next()
{
    if (view_) {
        view_->dynamicCall("Next()");
        thumb(0);
    } else if (presentation_ && slideNumber_ < total_) {
        thumb(++slideNumber_);
    }
}

void PptxControl::jump(int page)
{
    if (view_) {
        view_->dynamicCall("GotoSlide(int)", page);
        thumb(0);
    } else if (presentation_ && slideNumber_ > 0 && slideNumber_ <= total_) {
        thumb(slideNumber_ = page);
    }
}

void PptxControl::prev()
{
    if (view_) {
        view_->dynamicCall("Previous()");
        thumb(0);
    } else if (presentation_ && slideNumber_ > 1) {
        thumb(--slideNumber_);
    }
}

void PptxControl::hide()
{
    thumb(slideNumber_);
    hideWindow(hwnd_);
}

void PptxControl::close()
{
    if (!presentation_)
        return;
    delete view_;
    view_ = nullptr;
    hwnd_ = 0;
    if (stopButton_) {
        delete stopButton_;
        stopButton_ = nullptr;
    }
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
}

void PptxControl::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == item_->data(ITEM_KEY_TIMER)) {
        if (isWindowShown(hwnd_)) {
            if (!stopButton_)
                showReturnButton();
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

bool PptxControl::eventFilter(QObject *obj, QEvent * event)
{
    if (obj != stopButton_)
        return false;
    switch (event->type()) {
    case QEvent::MouseMove: {
        QPoint lpos = obj->property("lastPos").toPoint();
        QPoint pos = static_cast<QMouseEvent*>(event)->pos();
        moveChildWindow(static_cast<intptr_t>(stopButton_->winId()),
                        pos.x() - lpos.x(), pos.y() - lpos.y());
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

QString PptxControl::toolsString() const
{
    return toolstr;
}

intptr_t PptxControl::hwnd() const
{
    return hwnd_;
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
