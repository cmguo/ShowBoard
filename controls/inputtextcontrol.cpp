#include "inputtextcontrol.h"
#include <QBrush>
#include <QGraphicsTextItem>
#include "../core/resourceview.h"
#include <QFont>
#include <QObject>
#include <core/optiontoolbuttons.h>
#include <QGraphicsPolygonItem>

static char const * toolstr =
        "color|颜色|Popup,OptionsGroup,NeedUpdate|;"
        #ifdef QT_DEBUG
        "test()|测试|;"
        #endif
        ;
static ColorToolButtons colorButtons({Qt::white, Qt::black, Qt::red, Qt::green, Qt::blue, Qt::yellow});
REGISTER_OPTION_BUTTONS(InputTextControl, color, colorButtons)

InputTextControl::InputTextControl(ResourceView *res, Control::Flags flags, Control::Flags clearFlags) :
    Control(res, flags, clearFlags)
{

    // text2:
}

QColor InputTextControl::getColor()
{
    DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(item_);
    return item->defaultTextColor();
}

void InputTextControl::setColor(QColor color)
{
    DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(item_);
    item->setDefaultTextColor(color);
}

void InputTextControl::test()
{

}

ControlView *InputTextControl::create(ControlView *parent)
{
    DiagramTextItem* item = new DiagramTextItem;
    return item;
}

void InputTextControl::attached()
{
    loadFinished(true);
}

void InputTextControl::copy(QMimeData &data)
{

}

QString InputTextControl::toolsString(const QByteArray &parent) const
{
    if (parent.isEmpty()) {
        return toolstr;
    }
    return nullptr;
}

//! [0]
DiagramTextItem::DiagramTextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
//    setFlag(QGraphicsItem::ItemIsMovable);
//    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemAcceptsInputMethod);
    setPlainText("QGraphicsTextItem Engine 中文 123");
    setFont(QFont("微软雅黑",12));
    setDefaultTextColor(Qt::white);

}

QVariant DiagramTextItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
        emit selectedChange(this);
    return value;
}


void DiagramTextItem::focusOutEvent(QFocusEvent *event)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
    emit lostFocus(this);
    QGraphicsTextItem::focusOutEvent(event);
}

void DiagramTextItem::focusInEvent(QFocusEvent *event)
{
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::focusInEvent(event);
}

void DiagramTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}
