#include "whitecanvaswidget.h"
#include "whitecanvas.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"

#include <QGraphicsScene>
#include <QSizePolicy>
#include <QResizeEvent>

WhiteCanvasWidget::WhiteCanvasWidget(QWidget *parent)
    : QGraphicsView(parent)
    , package_(nullptr)
{
    scene_ = new QGraphicsScene(-512, -288, 1024, 576);
    scene_->setBackgroundBrush(QBrush());
    canvas_ = new WhiteCanvas;
    scene_->addItem(canvas_);
    setScene(scene_);

    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //setBackgroundBrush(QColor(230, 200, 167));
}

WhiteCanvasWidget::~WhiteCanvasWidget()
{
    delete scene_;
    scene_ = nullptr;
    canvas_ = nullptr;
}

void WhiteCanvasWidget::resizeEvent(QResizeEvent *event)
{
    QRectF rect(QPointF(0, 0), QSizeF(event->size()));
    rect.moveCenter({0, 0});
    scene_->setSceneRect(rect);
    canvas_->setGeometry(rect);
}

void WhiteCanvasWidget::setResourcePackage(ResourcePackage * pack)
{
    if (package_) {
        QObject::disconnect(package_, &ResourcePackage::currentPageChanged,
                            canvas_, &WhiteCanvas::switchPage);
    }
    package_ = pack;
    if (package_) {
        canvas_->switchPage(package_->currentPage());
        QObject::connect(package_, &ResourcePackage::currentPageChanged,
                         canvas_, &WhiteCanvas::switchPage);
    }
}
