#include "textcontrol2.h"
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
REGISTER_OPTION_BUTTONS(TextControl2, color, colorButtons)

TextControl2::TextControl2(ResourceView *res, Control::Flags flags, Control::Flags clearFlags) :
    Control(res, flags, clearFlags)
{

    // text2:
}

QColor TextControl2::getColor()
{
    DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(item_);
    return item->defaultTextColor();
}

void TextControl2::setColor(QColor color)
{
    DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(item_);
    item->setDefaultTextColor(color);
}

void TextControl2::test()
{

}

ControlView *TextControl2::create(ControlView *parent)
{
    DiagramTextItem* item = new DiagramTextItem;
//    item->setPlainText("TextControl2");
    return item;
}

void TextControl2::attached()
{
    //static_cast<QGraphicsTextItem*>(item_)->setTextInteractionFlags(Qt::TextEditorInteraction);
    loadFinished(true);
}

void TextControl2::copy(QMimeData &data)
{

}

QString TextControl2::toolsString(const QByteArray &parent) const
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
    setFont(QFont("华文琥珀",12));

}
//! [0]

//! [1]
QVariant DiagramTextItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
        emit selectedChange(this);
    return value;
}
//! [1]

//! [2]
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
//! [2]

//! [5]
void DiagramTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}
