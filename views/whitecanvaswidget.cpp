#include "whitecanvaswidget.h"
#include "whitecanvas.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"
#include "core/resourcetransform.h"
#include "core/control.h"

#include <QGraphicsScene>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QShortcut>
#include <QMimeData>
#include <QClipboard>

#include <core/resourceview.h>

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
    setScene(scene_);
    canvas_ = new WhiteCanvas;
    scene_->addItem(canvas_);
    setSceneSize(sceneSize_);

    setStyleSheet("border: 0px;");
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setResizeAnchor(AnchorViewCenter);
    //setInteractive(true);
    //setTransformationAnchor(AnchorUnderMouse);
    //setDragMode(QGraphicsView::ScrollHandDrag);

    // Delete
    new QShortcut(QKeySequence::Delete, this, SLOT(deleteSelection()));
    // Escape
    new QShortcut(QKeySequence::Cancel, this, SLOT(cancelSelection()));
    // Page Up, Page Down
    new QShortcut(QKeySequence::MoveToNextPage, this, SLOT(switchPage()));
    new QShortcut(QKeySequence::MoveToPreviousPage, this, SLOT(switchPage()));
    // Control + C, Control + V
    new QShortcut(QKeySequence::Copy, this, SLOT(copyPaste()));
    new QShortcut(QKeySequence::Paste, this, SLOT(copyPaste()));
    // Tab, Shift + Tab
    QObject::connect(new QShortcut(QKeySequence(Qt::Key_Tab), this), &QShortcut::activated,
                     this, [this]() { canvas_->selectNext(); });
    QObject::connect(new QShortcut(QKeySequence(Qt::Key_Tab | Qt::ShiftModifier), this), &QShortcut::activated,
                     this, [this]() { canvas_->selectPrev(); });
    // Direction Arrows
    for (int k : {Qt::Key_Left, Qt::Key_Up, Qt::Key_Right, Qt::Key_Down}) {
//        // Direction Arrows ( first press )
//        QShortcut * s = new QShortcut(QKeySequence(k), this);
//        s->setAutoRepeat(false);
//        QObject::connect(s, &QShortcut::activated,
//                         this, &WhiteCanvasWidget::moveSelection);
        // Direction Arrows
        new QShortcut(QKeySequence(k), this, SLOT(moveSelection()));
        // Shift + Direction Arrows
        new QShortcut(QKeySequence(k | Qt::ShiftModifier), this, SLOT(scaleSelection()));
        // Control + Direction Arrows
        new QShortcut(QKeySequence(k | Qt::ControlModifier), this, SLOT(switchPage()));
    }
    // Control + +/-
    new QShortcut(QKeySequence(Qt::Key_Plus | Qt::ControlModifier), this, SLOT(scaleSelection2()));
    new QShortcut(QKeySequence(Qt::Key_Minus | Qt::ControlModifier), this, SLOT(scaleSelection2()));
    // Space, Backspace
    new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(switchPage()));
    new QShortcut(QKeySequence(Qt::Key_Backspace), this, SLOT(switchPage()));
    // Control + Space/Backspace
    new QShortcut(QKeySequence(Qt::Key_Space | Qt::ControlModifier), this, SLOT(switchPage()));
    new QShortcut(QKeySequence(Qt::Key_Backspace | Qt::ControlModifier), this, SLOT(switchPage()));

    mainInstance_ = this;
}

WhiteCanvasWidget::WhiteCanvasWidget(WhiteCanvasWidget *mainView, QWidget *parent)
    : QGraphicsView(mainView->scene_, parent)
    , scene_(mainView->scene_)
    , canvas_(mainView->canvas_)
{
    setStyleSheet("border: 0px;");
    setAttribute(Qt::WA_AcceptDrops);
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

void WhiteCanvasWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void WhiteCanvasWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void WhiteCanvasWidget::dropEvent(QDropEvent *event)
{
    QMimeData const * data = event->mimeData();
    if (data) {
        ResourceView * res = ResourceView::paste(*data);
        if (res)
            canvas()->addResource(res);
    }
    event->setDropAction(Qt::CopyAction);
}

bool WhiteCanvasWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window() && event->type() == QEvent::Close && !event->spontaneous()) {
        qDebug() << "window closed";
        setResourcePackage(nullptr);
    }
    return false;
}

void WhiteCanvasWidget::onPageChanged(ResourcePage *page)
{
    bool oldLarge = canvas_->page() && canvas_->page()->isLargePage();
    bool newLarge = (page == CurrentPage) ? oldLarge : page && page->isLargePage();
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
    static QPixmap icon(":/showboard/icon/page.loading.svg");
    static QPixmap thumb((size * WhiteCanvas::THUMBNAIL_HEIGHT / size.height()).toSize());
    thumb.fill(Qt::transparent);
    QPainter painter(&thumb);
    QRect rect(0, 0, icon.width(), icon.height());
    rect.moveCenter({thumb.width() / 2, thumb.height() / 2});
    painter.drawPixmap(rect, icon);
    painter.end();
    ResourcePackage::toolPage()->setThumbnail(thumb);
    onPageChanged(CurrentPage);
    if (canvas_->page() && !canvas_->page()->isLargePage())
        canvas_->setGeometry(scene()->sceneRect());
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

void WhiteCanvasWidget::deleteSelection()
{
    if (canvas_->selected())
        canvas_->removeResource(canvas_->selected());
}

void WhiteCanvasWidget::cancelSelection()
{
    canvas_->select(nullptr);
}

void WhiteCanvasWidget::moveSelection()
{
    Control * c = canvas_->selected();
    qreal delta = 10;
    //if (!c) { c = Control::fromItem(canvas_); delta = -20; }
    QPointF d;
    if (c && c->flags().testFlag(Control::CanMove)) {
        QShortcut* s = qobject_cast<QShortcut*>(sender());
        if (s->key().matches(Qt::Key_Left)) {
            d.setX(-delta);
        } else if (s->key().matches(Qt::Key_Right)) {
            d.setX(delta);
        } else if (s->key().matches(Qt::Key_Up)) {
            d.setY(-delta);
        } else if (s->key().matches(Qt::Key_Down)) {
            d.setY(delta);
        }
        c->move(d);
    }
    if (d.isNull()) {
        switchPage();
    }
}

void WhiteCanvasWidget::scaleSelection()
{
    Control * c = canvas_->selected();
    qreal delta = 10;
    if (!c || !c->flags().testFlag(Control::CanScale))
        return;
    QPointF d;
    QShortcut* s = qobject_cast<QShortcut*>(sender());
    if (s->key().matches(QKeySequence(Qt::Key_Left | Qt::ShiftModifier))) {
        d.setX(-delta);
    } else if (s->key().matches(QKeySequence(Qt::Key_Right | Qt::ShiftModifier))) {
        d.setX(delta);
    } else if (s->key().matches(QKeySequence(Qt::Key_Up | Qt::ShiftModifier))) {
        d.setY(-delta);
    } else if (s->key().matches(QKeySequence(Qt::Key_Down | Qt::ShiftModifier))) {
        d.setY(delta);
    }
    QRectF t;
    if (qFuzzyIsNull(d.x())) {
        t.setHeight(1);
    } else {
        t.setWidth(1);
    }
    c->scale(t, d);
}

void WhiteCanvasWidget::scaleSelection2()
{
    Control * c = canvas_->selected();
    qreal delta = 1;
    if (!c) { c = Control::fromItem(canvas_); }
    if (!c || !c->flags().testFlag(Control::CanScale))
        return;
    QShortcut* s = qobject_cast<QShortcut*>(sender());
    if (s->key().matches(QKeySequence(Qt::Key_Minus | Qt::ControlModifier))) {
        delta = 1.0 / 1.2;
    } else if (s->key().matches(QKeySequence(Qt::Key_Plus | Qt::ControlModifier))) {
        delta = 1.2;
    }
    c->scale(c->item()->boundingRect().center(), delta);
}

void WhiteCanvasWidget::switchPage()
{
    QShortcut * s = qobject_cast<QShortcut*>(sender());
    bool next = true;
    if (s->key().matches(QKeySequence(Qt::Key_PageUp))
            || s->key().matches(QKeySequence(Qt::Key_Backspace))
            || s->key().matches(QKeySequence(Qt::Key_Up))
            || s->key().matches(QKeySequence(Qt::Key_Left))
            || s->key().matches(QKeySequence(Qt::Key_Up | Qt::ControlModifier))
            || s->key().matches(QKeySequence(Qt::Key_Left | Qt::ControlModifier))) {
        next = false;
    }
    if (!(s->key()[0] & Qt::ControlModifier)) {
        ResourcePage * page = package()->currentPage();
        if (page->isIndependentPage()) {
            Control * c = canvas_->findControl(page->mainResource());
            if ((next ? c->exec("next()") : c->exec("prev()")))
                return;
        }
    } else {
        if (next)
            package()->gotoNext();
        else
            package()->gotoPrevious();
    }
}

void WhiteCanvasWidget::copyPaste()
{
    QShortcut * s = qobject_cast<QShortcut*>(sender());
    if (s->key().matches(QKeySequence::Copy)) {
        Control * c = canvas_->selected();
        if (c) {
            QMimeData * data = new QMimeData;
            c->copy(*data);
            QApplication::clipboard()->setMimeData(data);
        }
    } else {
        QMimeData const * data = QApplication::clipboard()->mimeData();
        if (data) {
            ResourceView * res = ResourceView::paste(*data);
            if (res->parent() == canvas()->page()) {
                res->transform().translate({60, 60});
            } else {
                res->transform().translateTo({0, 0});
            }
            if (res) {
                Control * c = canvas()->addResource(res);
                Control::paste(*data, c);
            }
        }
    }
}
