#include "whitecanvaswidget.h"
#include "whitecanvas.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"

#include <QGraphicsScene>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QApplication>
#include <QScreen>
#include <QDebug>

static WhiteCanvasWidget * mainInstance_;

WhiteCanvasWidget * WhiteCanvasWidget::mainInstance()
{
    return mainInstance_;
}

WhiteCanvasWidget::WhiteCanvasWidget(QWidget *parent)
    : QGraphicsView(parent)
    , sceneSize_(QApplication::primaryScreen()->geometry().size())
{
    QRectF rect(QPointF(0, 0), sceneSize_);
    rect.moveCenter({0, 0});
    scene_ = new QGraphicsScene(rect);
    scene_->setBackgroundBrush(QBrush());
    canvas_ = new WhiteCanvas;
    scene_->addItem(canvas_);
    setScene(scene_);

    setStyleSheet("border: 0px;");
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setResizeAnchor(AnchorViewCenter);
    //setInteractive(true);
    //setTransformationAnchor(AnchorUnderMouse);
    //setDragMode(QGraphicsView::ScrollHandDrag);

    mainInstance_ = this;
}

WhiteCanvasWidget::WhiteCanvasWidget(WhiteCanvasWidget *mainView, QWidget *parent)
    : QGraphicsView(mainView->scene_, parent)
    , scene_(mainView->scene_)
    , canvas_(mainView->canvas_)
{
    setStyleSheet("border: 0px;");
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

WhiteCanvasWidget::~WhiteCanvasWidget()
{
    mainInstance_ = nullptr;
    delete canvas_;
    canvas_ = nullptr;
    delete scene_;
    scene_ = nullptr;
}

static ResourcePage * CurrentPage = reinterpret_cast<ResourcePage*>(1);

void WhiteCanvasWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "WhiteCanvasWidget resizeEvent" << event->size();
    QGraphicsView::resizeEvent(event);
    onPageChanged(CurrentPage);
}

void WhiteCanvasWidget::showEvent(QShowEvent *event)
{
    (void) event;
    window()->installEventFilter(this);
}

bool WhiteCanvasWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window() && event->type() == QEvent::Close) {
        qDebug() << "window closed";
        setResourcePackage(nullptr);
    }
    return false;
}

void WhiteCanvasWidget::onPageChanged(ResourcePage *page)
{
    bool oldLarge = canvas_->page() && canvas_->page()->canvasView();
    bool newLarge = (page == CurrentPage) ? oldLarge : page && page->canvasView();
    if (oldLarge == newLarge && page != CurrentPage)
        return;
    QRectF rect(QPointF(0, 0), newLarge ? size() : sceneSize_);
    rect.moveCenter({0, 0});
    scene_->setSceneRect(rect);
    setTransform(newLarge ? QTransform()
                          : QTransform::fromScale(width() / scene_->width(), height() / scene_->height()));
}

ResourcePackage * WhiteCanvasWidget::package()
{
    return canvas_->package();
}

void WhiteCanvasWidget::setSceneSize(QSizeF size)
{
    sceneSize_ = size;
    onPageChanged(CurrentPage);
}

void WhiteCanvasWidget::setResourcePackage(ResourcePackage * pack)
{
    ResourcePackage* old = canvas_->package();
    canvas_->setResourcePackage(nullptr);
    if (old) {
        QObject::disconnect(old, &ResourcePackage::currentPageChanged, this, &WhiteCanvasWidget::onPageChanged);
        onPageChanged(nullptr);
    }
    if (pack) {
        // handle before whitecanvas, whitecanvas depends on sceneRect
        QObject::connect(pack, &ResourcePackage::currentPageChanged, this, &WhiteCanvasWidget::onPageChanged);
        onPageChanged(pack->currentPage());
    }
    canvas_->setResourcePackage(pack);
}
