#include "optiontoolbuttons.h"
#include "toolbutton.h"

#include <QGraphicsRectItem>
#include <QPen>

OptionToolButtons::OptionToolButtons(const QVariantList &values, int column)
    : values_(values)
    , column_(column)
    , lastCheck_(-1)
{
}

OptionToolButtons::~OptionToolButtons()
{
    for (ToolButton* b : buttons_) {
        if (b->flags & ToolButton::Static)
            continue;
        delete b;
    }
}

void OptionToolButtons::fill(QList<ToolButton *> &buttons, const QVariant &value)
{
    buttons.append(this->buttons(value));
}

QList<ToolButton *> OptionToolButtons::buttons(const QVariant &value)
{
    if (buttons_.isEmpty())
        makeButtons();
    int index = values_.indexOf(value);
    if (index >= 0) {
        index += index / column_;
    }
    if (index != lastCheck_) {
        if (lastCheck_ >= 0)
            buttons_[lastCheck_]->flags.setFlag(ToolButton::Selected, false);
        lastCheck_ = index;
        if (lastCheck_ >= 0)
            buttons_[lastCheck_]->flags.setFlag(ToolButton::Selected, true);
    }
    return buttons_;
}

void OptionToolButtons::update(ToolButton *button, const QVariant &value)
{
    int index = values_.indexOf(value);
    if (index >= 0 && index < buttons_.size()) {
        index += index / column_;
        if (!buttons_[index]->title.isEmpty())
            button->title = buttons_[index]->title;
        if (!buttons_[index]->icon.isNull())
            button->icon = buttons_[index]->icon;
    } else {
        QString title = buttonTitle(value);
        if (!title.isEmpty())
            button->title = title;
        QVariant icon = buttonIcon(value);
        if (!icon.isNull())
            button->icon = icon;
    }
}

void OptionToolButtons::makeButtons()
{
    ToolButton::Flags flags = nullptr;
    int c = column_;
    for (QVariant v : values_) {
        ToolButton * btn = new ToolButton({buttonName(v), buttonTitle(v), flags, buttonIcon(v)});
        if (c == 0) {
            buttons_.append(&ToolButton::LINE_BREAK);
            c = column_;
        }
        buttons_.append(btn);
        --c;
    }
}

QByteArray OptionToolButtons::buttonName(const QVariant &value)
{
    if (value.canConvert(QMetaType::QByteArray)) {
        return value.toByteArray();
    }
    return value.toString().toUtf8();
}

QString OptionToolButtons::buttonTitle(const QVariant &)
{
    return nullptr;
}

QVariant OptionToolButtons::buttonIcon(const QVariant &)
{
    return QVariant();
}

ColorToolButtons::ColorToolButtons(const QStringList &colors)
    : OptionToolButtons(colors, colors.size() / 2)
{
}

ColorToolButtons::ColorToolButtons(const QList<QColor> &colors)
    : OptionToolButtons(colors, colors.size() / 2)
{
}

QVariant ColorToolButtons::buttonIcon(const QVariant &value)
{
    QVariant vcolor(value);
    vcolor.convert(qMetaTypeId<QColor>());
    QColor color = vcolor.value<QColor>();
    return QVariant::fromValue(colorIcon(color));
}

QGraphicsItem *ColorToolButtons::colorIcon(QColor color)
{
    QGraphicsRectItem * item = new QGraphicsRectItem;
    item->setRect({1, 1, 30, 30});
    item->setPen(QPen(QColor(color.red() / 2 + 128, // mix with white
                        color.green() / 2 + 128, color.blue() / 2 + 128), 2.0));
    item->setBrush(color);
    return item;
}

StateColorToolButtons::StateColorToolButtons(const QStringList &colors)
    : OptionToolButtons(colors, colors.size() / 2)
{
}

StateColorToolButtons::StateColorToolButtons(const QList<QColor> &colors)
    : OptionToolButtons(colors, colors.size() / 2)
{
}

QVariant StateColorToolButtons::buttonIcon(const QVariant &value)
{
    QVariant vcolor(value);
    vcolor.convert(qMetaTypeId<QColor>());
    QColor color = vcolor.value<QColor>();
    QVariantMap icons;
    icons.insert("normal", QVariant::fromValue(colorIcon(color, false)));
    icons.insert("+normal", QVariant::fromValue(colorIcon(color, true)));
    return icons;
}

QGraphicsItem* StateColorToolButtons::colorIcon(QColor color, bool selected)
{
    QPainterPath ph;
    ph.addEllipse(QRectF(1, 1, 30, 30));
    QGraphicsPathItem * border = new QGraphicsPathItem(ph);
    if (selected)
        border->setPen(QPen(Qt::white, 2.0));
    else
        //border->setPen(QPen(QColor(color.red() * 3 / 4 + 64, // mix with white
        //                    color.green() * 3 / 4 + 64, color.blue() * 3 / 4 + 64), 2.0));
        border->setPen(QPen(QColor("#FF46515F"), 2.0));
    border->setBrush(QBrush());
    QPainterPath ph2;
    ph2.addEllipse(QRectF(4, 4, 24, 24));
    QGraphicsPathItem * item = new QGraphicsPathItem(ph2, border);
    item->setPen(Qt::NoPen);
    item->setBrush(color);
    return border;
}

WidthToolButtons::WidthToolButtons(const QList<qreal> &widths)
    : OptionToolButtons(widths, widths.size())
{
}

QVariant WidthToolButtons::buttonIcon(const QVariant &value)
{
    return QVariant::fromValue(widthIcon(value.toReal()));
}

QGraphicsItem *WidthToolButtons::widthIcon(qreal width)
{
    QPainterPath ph;
    ph.addEllipse(QRectF(0, 0, 32, 32));
    QGraphicsPathItem * border = new QGraphicsPathItem(ph);
    border->setPen(Qt::NoPen);
    border->setBrush(QColor("#343434"));
    QPainterPath ph2;
    QRectF rect(0, 0, width, width);
    rect.moveCenter(QPointF(16, 16));
    ph2.addEllipse(rect);
    QGraphicsPathItem * item = new QGraphicsPathItem(ph2, border);
    item->setPen(Qt::NoPen);
    item->setBrush(Qt::white);
    return border;
}

StateWidthToolButtons::StateWidthToolButtons(const QList<qreal> &widths)
    : OptionToolButtons(widths, widths.size())
{
}

QVariant StateWidthToolButtons::buttonIcon(const QVariant &value)
{
    QVariantMap icons;
    icons.insert("normal", QVariant::fromValue(widthIcon(value.toReal(), false)));
    icons.insert("+normal", QVariant::fromValue(widthIcon(value.toReal(), true)));
    return icons;
}

QGraphicsItem *StateWidthToolButtons::widthIcon(qreal width, bool selected)
{
    QPainterPath ph;
    ph.addEllipse(QRectF(1, 1, 30, 30));
    QGraphicsPathItem * border = new QGraphicsPathItem(ph);
    if (selected)
        border->setPen(QPen(Qt::white, 2));
    else
        border->setPen(Qt::NoPen);
    border->setBrush(QBrush());
    QPainterPath ph2;
    QRectF rect(0, 0, width, width);
    rect.moveCenter(QPointF(16, 16));
    ph2.addEllipse(rect);
    QGraphicsPathItem * item = new QGraphicsPathItem(ph2, border);
    item->setPen(Qt::NoPen);
    item->setBrush(Qt::white);
    return border;
}
