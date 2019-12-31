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
{
    QRectF rect(QApplication::primaryScreen()->geometry());
    qDebug() << "scene rect " << rect;
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

void WhiteCanvasWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "WhiteCanvasWidget resizeEvent" << event->size();
    QRectF rect(QPointF(0, 0), QSizeF(event->size()));
    rect.moveCenter({0, 0});
    //scene_->setSceneRect(rect);
    //canvas_->setGeometry(rect);
    QSize ns = event->size();
    setTransform(QTransform::fromScale(ns.width() / scene_->width(), ns.height() / scene_->height()));
    QGraphicsView::resizeEvent(event);
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


ResourcePackage * WhiteCanvasWidget::package()
{
    return canvas_->package();
}

void WhiteCanvasWidget::setResourcePackage(ResourcePackage * pack)
{
    canvas_->setResourcePackage(pack);
}
