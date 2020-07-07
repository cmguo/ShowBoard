#include "whitecanvascontrol.h"
#include "views/whitecanvas.h"
#include "views/positionbar.h"
#include "views/toolbarwidget.h"
#include "views/qsshelper.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"

#include <QUrl>
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QTimer>

static QssHelper QSS(":/showboard/qss/canvastoolbar.qss");

static constexpr char const * toolsStr =
        "scaleUp()||UnionUpdate|:/showboard/icon/zoom_in.svg,default;"
        "scaleDown()||UnionUpdate|:/showboard/icon/zoom_out.svg,default;"
        "|;"
        "close()||NeedUpdate|:/showboard/icon/close.svg;";

WhiteCanvasControl::WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas)
    : Control(view, {}, {CanSelect, CanScale, CanRotate})
{
    setToolsString(toolsStr);
    item_ = canvas;
    item_->setTransformations({transform_});
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    realItem_ = item_;
    if (view->flags().testFlag(ResourceView::LargeCanvas)) {
        QObject::connect(&res_->transform(), &ResourceTransform::beforeChanged,
                         this, &WhiteCanvasControl::updatingTransform);
        QObject::connect(&res_->transform(), &ResourceTransform::changed,
                         this, &WhiteCanvasControl::updateTransform);
        PositionBar* posBar = new PositionBar(canvas);
        posBar->setInCanvas();
        posBar_ = posBar;
    }
    ToolbarWidget* toolbar = new ToolbarWidget;
    toolbar->setObjectName("canvastoolbar");
    toolbar->setStyleSheet(QSS);
    toolbar->setDragable();
    toolBar_ = toolbar->toGraphicsProxy(nullptr, true);
    toolbar->attachProvider(this);
    loadSettings();
    // adjust to scene, this is done before attaching transform
    res_->transform().translate(QPointF(0, 0));
    qDebug() << "WhiteCanvasControl" << res_->transform().transform();
    item_->scene()->addItem(toolBar_);
    toolBar_->setPos(QPointF(0, item_->scene()->sceneRect().bottom() - 60));
    flags_.setFlag(LoadFinished);
}

WhiteCanvasControl::~WhiteCanvasControl()
{
    delete toolBar_;
    toolBar_ = nullptr;
    qDebug() << "~WhiteCanvasControl" << res_->transform().transform();
    saveSettings();
    delete posBar_;
    item_->setData(ITEM_KEY_CONTROL, QVariant());
    item_->setTransformations({});
    item_ = nullptr;
    realItem_ = nullptr;
}

void WhiteCanvasControl::setToolBarStyles(const QString &stylesheet)
{
    qobject_cast<ToolbarWidget*>(static_cast<QGraphicsProxyWidget*>(toolBar_)->widget())
            ->setStyleSheet(QssHelper(stylesheet));
}

void WhiteCanvasControl::setPosBarVisible(bool visible)
{
    posBar_->setVisible(visible);
}

void WhiteCanvasControl::scaleUp()
{
    res_->transform().scale({1.25, 1.25});
}

void WhiteCanvasControl::scaleDown()
{
    res_->transform().scale({0.8, 0.8});
}

void WhiteCanvasControl::close()
{
    QTimer::singleShot(0, this, [this] () {
        res_->page()->removeFromPackage();
    });
}

QGraphicsItem *WhiteCanvasControl::create(ResourceView *res)
{
    (void) res;
    return nullptr;
}

void WhiteCanvasControl::resize(const QSizeF &size)
{
    if (res_->flags().testFlag(ResourceView::LargeCanvas)) {
        qDebug() << "WhiteCanvasControl resize" << size;
        QRectF old = item_->boundingRect();
        QRectF srect = item_->scene()->sceneRect();
        QRectF rect(QPointF(0, 0), size);
        rect.moveCenter(QPointF(0, 0));
        rect |= srect;
        static_cast<WhiteCanvas*>(item_)->setGeometry(rect);
        if (flags_.testFlag(LoadFinished)) {
            QSizeF ds = (rect.size() - old.size()) / 2;
            QPointF d{ds.width(), ds.height()};
            move(d);
        }
    }
}

// called when main resource attached
void WhiteCanvasControl::attached()
{
    if (flags_.testFlag(CanScale)) {
        buttonsChanged();
    }
}

void WhiteCanvasControl::sizeChanged()
{
    // do nothing
}

void WhiteCanvasControl::getToolButtons(QList<ToolButton *> &buttons, QList<ToolButton *> const & parents)
{
    // not include default buttons of Control
    Control::getToolButtons(buttons, parents);
    if (parents.isEmpty()) {
        ToolButton * closeBtn = getStringButton("close()");
        buttons.removeOne(closeBtn);
        buttons.append(&ToolButton::SPLITTER);
        buttons.append(closeBtn);
    }
}

void WhiteCanvasControl::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    // not include default buttons of Control
    ToolButtonProvider::getToolButtons(buttons, parent);
}

void WhiteCanvasControl::updateToolButton(ToolButton *button)
{
    if (button->name() == "scaleUp()") {
        button->setEnabled(resource()->transform().scale().m11() < 10);
        button->setVisible(flags_.testFlag(CanScale));
    } else if (button->name() == "scaleDown()") {
        button->setEnabled(resource()->transform().scale().m11() > 1.1);
        button->setVisible(flags_.testFlag(CanScale));
    } else if (button->name() == "close()") {
        button->setVisible(res_->flags().testFlag(ResourceView::CanDelete));
    } else {
        ToolButtonProvider::updateToolButton(button);
    }
}

void WhiteCanvasControl::updatingTransform()
{
    ResourceTransform & transform = res_->transform();
    QRectF srect = item_->scene()->sceneRect();
    QRectF crect = item_->boundingRect();
    transform.keepOuterOf(srect, crect);
}

void WhiteCanvasControl::updateTransform()
{
    ResourceTransform & transform = res_->transform();
    //qDebug() << "WhiteCanvasControl" << transform.transform();
    QRectF srect = item_->scene()->sceneRect();
    QRectF crect = item_->boundingRect();
    posBar_->update(srect, crect, transform.scale().m11(), transform.offset());
}
