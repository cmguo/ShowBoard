#include "toolbarwidget.h"
#include "core/toolbutton.h"
#include "core/toolbuttonprovider.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QWidget>
#include <QStyleOptionButton>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QLabel>

static const QString STYLE = "QPushButton,.QLabel{color:#80ffffff;background-color:#00000000;border:none;font-size:16pt;spacing: 30px;border:none;} "
                "QPushButton{qproperty-iconSize: 30px 30px; font-family: '微软雅黑';background-color:#00000000} "
                "QPushButton:checked{color:#1AA9EF;}"
                "#toolbarwidget{background-color:#C8000000;border-radius:3px;}"
                "#popupwidget{background-color:#C8000000;border-radius:3px;}";

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : ToolbarWidget(true, parent)
{
}

ToolbarWidget::ToolbarWidget(bool horizontal, QWidget *parent)
    : QWidget(parent)
    , template_(nullptr)
    , popupPosition_(BottomRight)
    , popUp_(nullptr)
    , provider_(nullptr)
{
    if (horizontal)
        layout_ = new QHBoxLayout(this);
    else
        layout_ = new QVBoxLayout(this);
    this->setObjectName(QString::fromUtf8("toolbarwidget"));
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet(STYLE);
    if (horizontal)
        layout_->setContentsMargins(10,10,10,10);
    this->setLayout(layout_);
    hide();
}

ToolbarWidget::~ToolbarWidget()
{
    clear();
}

void ToolbarWidget::setButtonTemplate(int typeId)
{
    QMetaObject const * meta = QMetaType::metaObjectForType(typeId);
    if (meta && meta->inherits(&QPushButton::staticMetaObject))
        template_ = meta;
}

void ToolbarWidget::setPopupPosition(PopupPosition pos)
{
    popupPosition_ = pos;
}

static QPixmap widgetToPixmap(QWidget * widget, bool destroy)
{
    QPixmap pm(widget->size());
    QPainter pt(&pm);
    pt.setRenderHint(QPainter::HighQualityAntialiasing);
    widget->render(&pt);
    if (destroy)
        widget->deleteLater();
    return pm;
}

static QPixmap itemToPixmap(QGraphicsItem * item, bool destroy)
{
    QPixmap pm(item->boundingRect().size().toSize());
    QPainter pt(&pm);
    pt.setRenderHint(QPainter::HighQualityAntialiasing);
    QStyleOptionGraphicsItem style;
    item->paint(&pt, &style);
    for (QGraphicsItem * c : item->childItems()) {
        c->paint(&pt, &style);
    }
    if (destroy)
        delete item;
    return pm;
}

static QIcon getIcon(QVariant& icon, bool replace)
{
    QIcon result;
    if (icon.type() == QVariant::Icon)
        return icon.value<QIcon>();
    else if (icon.type() == QVariant::String)
        result = QIcon(icon.toString());
    else if (icon.type() == QVariant::Pixmap)
        result = QIcon(icon.value<QPixmap>());
    else if (icon.type() == QVariant::Image)
        result = QIcon(QPixmap::fromImage(icon.value<QImage>()));
    else if (icon.type() == QVariant::UserType) {
        if (icon.userType() == QMetaType::QObjectStar) {
            result = QIcon(widgetToPixmap(icon.value<QWidget *>(), replace));
        } else if (icon.userType() == qMetaTypeId<QGraphicsItem *>()) {
            result = QIcon(itemToPixmap(icon.value<QGraphicsItem *>(), replace));
        }
    }
    if (replace)
        icon = result;
    return result;
}

void ToolbarWidget::setToolButtons(QList<ToolButton *> const & buttons)
{
    clear();
    for (ToolButton * b : buttons) {
        addToolButton(layout_, b, buttons_);
    }
    setVisible(!buttons.empty());
    updateGeometry(); //
}

void ToolbarWidget::setToolButtons(ToolButton buttons[], int count)
{
    clear();
    for (int i = 0; i < count; ++i) {
        addToolButton(layout_, buttons + i, buttons_);
    }
    setVisible(count > 0);
    updateGeometry();
}

void ToolbarWidget::updateButton(ToolButton *button)
{
    ToolButton * parent = popupParents_.empty() ? nullptr : popupParents_.back();
    for (QWidget * w : buttons_.keys()) {
        if (buttons_.value(w) == button) {
            updateButton(qobject_cast<QPushButton*>(w), parent, button);
            return;
        }
    }
}

void ToolbarWidget::showPopupButtons(const QList<ToolButton *> &buttons)
{
    clearPopup();
    for (ToolButton * b : buttons) {
        addToolButton(popUp_->layout(), b, popupButtons_);
    }
    popUp_->layout()->activate();
    popUp_->updateGeometry();
}

void ToolbarWidget::showPopupButtons(ToolButton *buttons, int count)
{
    clearPopup();
    for (int i = 0; i < count; ++i) {
        addToolButton(popUp_->layout(), buttons + i, popupButtons_);
    }
    popUp_->layout()->activate();
    popUp_->updateGeometry();

}

static int row = 0;
static int col = 0;

void ToolbarWidget::clearPopup()
{
    row = col = 0;
    if (popUp_) {
        popUp_->hide();
        clearButtons(popUp_->layout(), popupButtons_);
        popUp_->layout()->activate();
        QGraphicsProxyWidget * proxy = popUp_->graphicsProxyWidget();
        if (proxy) {
            proxy->resize(popUp_->minimumSize());
        }
    }
}

void ToolbarWidget::attachProvider(ToolButtonProvider *provider)
{
    if (provider == provider_)
        return;
    if (provider_) {
        QObject::disconnect(provider_, &ToolButtonProvider::buttonsChanged, this, &ToolbarWidget::updateProvider);
    }
    provider_ = provider;
    if (provider_) {
        QObject::connect(provider_, &ToolButtonProvider::buttonsChanged, this, &ToolbarWidget::updateProvider);
        updateProvider();
    } else {
        clear();
    }
}

void ToolbarWidget::buttonClicked()
{
    QWidget * widget = qobject_cast<QWidget *>(sender());
    buttonClicked(widget);
}

void ToolbarWidget::addToolButton(QLayout* layout, ToolButton * button, QMap<QWidget *, ToolButton *>& buttons)
{
    ToolButton * parent = popupParents_.empty() ? nullptr : popupParents_.back();
    QWidget * widget = nullptr;
    if (button == &ToolButton::SPLITER) {
        QLabel *splitLabel = new QLabel(this);
        splitLabel->setText("|");
        widget = splitLabel;
    } else if (button == &ToolButton::LINE_BREAK) {
        ++row; col = 0;
        return;
    } else if (button->flags & ToolButton::CustomWidget) {
        widget = button->icon.value<QWidget*>();
        ToolButton::action_t action([this, widget]() {
            buttonClicked(widget);
        });
        widget->setProperty(ToolButton::ACTION_PROPERTY, QVariant::fromValue(action));
        widget->show();
    } else {
        QPushButton * btn = template_
                ? qobject_cast<QPushButton *>(template_->newInstance())
                : new QPushButton;
        applyButton(btn, parent, button);
        void (ToolbarWidget::*slot)() = &ToolbarWidget::buttonClicked;
        QObject::connect(btn, &QPushButton::clicked, this, slot);
        widget = btn;
    }
    if (parent) {
        QGridLayout *gridLayout = static_cast<QGridLayout*>(layout);
        gridLayout->addWidget(widget, row, col);
        ++col;
    } else {
        layout->addWidget(widget);
    }
    if (button) {
        buttons.insert(widget, button);
    }
}

void ToolbarWidget::applyButton(QPushButton * btn, ToolButton * parent, ToolButton *button)
{
    btn->setIconSize(QSize(40,40));
    btn->setIcon(getIcon(button->icon, !(button->flags & ToolButton::Dynamic)));
    if (!button->title.isEmpty()) {
        if (button->icon.isValid())
            btn->setText(" " + button->title);
        else
            btn->setText(button->title);
    }
    if (parent && (parent->flags & ToolButton::OptionsGroup)) {
        btn->setCheckable(true);
        btn->setChecked(button->flags & ToolButton::Selected);
    }
    if (button->flags & ToolButton::Checkable) {
        btn->setCheckable(true);
        btn->setChecked(button->flags & ToolButton::Checked);
    }
    btn->setObjectName(button->name);
}

void ToolbarWidget::updateButton(QPushButton * btn, ToolButton * parent, ToolButton *button)
{
    applyButton(btn, parent, button);
    if (button->flags.testFlag(ToolButton::UnionUpdate)) {
        QList<QObject*> list = children();
        int n = list.indexOf(btn);
        for (int i = n - 1; i >= 0; --i) {
            QWidget * widget2 = qobject_cast<QWidget *>(list[i]);
            if (widget2 == nullptr || (button = buttons_.value(widget2)) == nullptr)
                continue;
            if (button->flags.testFlag(ToolButton::UnionUpdate)) {
                QPushButton * btn2 = qobject_cast<QPushButton *>(widget2);
                applyButton(btn2, parent, button);
            } else {
                break;
            }
        }
        for (int i = n + 1; i < list.size(); ++i) {
            QWidget * widget2 = qobject_cast<QWidget *>(list[i]);
            if (widget2 == nullptr || (button = buttons_.value(widget2)) == nullptr)
                continue;
            if (button->flags.testFlag(ToolButton::UnionUpdate)) {
                QPushButton * btn2 = qobject_cast<QPushButton *>(widget2);
                applyButton(btn2, parent, button);
            } else {
                break;
            }
        }
    }
}

void ToolbarWidget::clear()
{
    clearPopup();
    popupParents_.clear();
    clearButtons(layout_, buttons_);
    layout_->activate();
    QGraphicsProxyWidget * proxy = graphicsProxyWidget();
    if (proxy)
        proxy->resize(minimumSize());
}

void ToolbarWidget::clearButtons(QLayout *layout, QMap<QWidget *, ToolButton *> &buttons)
{
    for (QWidget * widget : buttons.keys()) {
        layout->removeWidget(widget);
        ToolButton * btn = buttons.value(widget);
        if (btn && btn->flags & ToolButton::Dynamic)
            delete btn;
        if (btn && btn->flags & ToolButton::CustomWidget) {
            widget->hide();
            widget->setParent(nullptr);
            widget->setProperty(ToolButton::ACTION_PROPERTY, QVariant());
        } else {
            widget->deleteLater();
        }
    }
    layout->update();
    buttons.clear();
}

void ToolbarWidget::createPopup()
{
    popUp_ = new QWidget();
    popUp_->setStyleSheet(STYLE);
    popUp_->setObjectName(QString::fromUtf8("popupwidget"));
    QGraphicsProxyWidget * proxy = graphicsProxyWidget();
    if (proxy) {
        proxy = new QGraphicsProxyWidget(proxy);
        proxy->setWidget(popUp_);
        proxy->hide();
    } else {
        popUp_->setParent(parentWidget()->parentWidget());
        popUp_->hide();
    }
    popUp_->setLayout(new QGridLayout());
}

void ToolbarWidget::updateProvider()
{
    QList<ToolButton *> buttons;
    provider_->getToolButtons(buttons);
    setToolButtons(buttons);
}

void ToolbarWidget::buttonClicked(QWidget * widget)
{
    QPushButton* btn = qobject_cast<QPushButton*>(widget);
    ToolButton * button = popupButtons_.value(widget);
    if (!button) {
        button = buttons_.value(widget);
        if (!button) return;
        bool isPopup = !popupButtons_.empty();
        clearPopup();
        if (isPopup && popupParents_.endsWith(button)) {
            popupParents_.pop_back();
            if (btn && button->flags & ToolButton::Checkable) {
                btn->setChecked(button->flags.testFlag(ToolButton::Checked));
            }
            return;
        }
        popupParents_.clear();
    }
    popupParents_.append(button);
    if (btn && button->flags & ToolButton::Popup) {
        if (button->flags & ToolButton::Checkable) {
            btn->setChecked(button->flags.testFlag(ToolButton::Checked));
        }
        QList<ToolButton *> buttons;
        getPopupButtons(buttons, popupParents_);
        if (popUp_ == nullptr) {
            createPopup();
        }
        showPopupButtons(buttons);
        QGraphicsProxyWidget * proxy = graphicsProxyWidget();
        if (proxy) {
            QGraphicsProxyWidget * proxy2 = popUp_->graphicsProxyWidget();
            proxy2->setPos(popupPosition(btn, proxy2));
        } else {
            QPoint pos = btn->mapTo(popUp_->parentWidget(), QPoint(0, btn->height() + 10));
            popUp_->move(pos);
        }
        popUp_->show();
    } else {
        onButtonClicked(button);
        popupParents_.pop_back();
        int i = 0;
        ToolButton * parent = popupParents_.empty() ? nullptr : popupParents_.back();
        for (; i < popupParents_.size(); ++i) {
            if (popupParents_[i]->flags & ToolButton::OptionsGroup) {
                button = popupParents_[i];
                if (i > 0)
                    parent = popupParents_[i - 1];
                break;
            }
        }
        if (button->flags & ToolButton::NeedUpdate) {
            ToolButton * parent = i == 0 ? nullptr : popupParents_[i - 1];
            for (QWidget * w : buttons_.keys()) {
                if (buttons_.value(w) == button) {
                    updateButton(qobject_cast<QPushButton*>(w), parent, button);
                    if (graphicsProxyWidget())
                        graphicsProxyWidget()->update();
                    break;
                }
            }
        }
        if (!popupButtons_.empty()) {
            clearPopup();
            popupParents_.pop_back();
        }
    }
}

QPointF ToolbarWidget::popupPosition(QPushButton *btn, QGraphicsProxyWidget *popup)
{
    QGraphicsProxyWidget* proxy = graphicsProxyWidget();
    QGraphicsItem* parent = proxy->parentItem();
    QPointF topLeft = btn->mapTo(this, QPoint(0, 0));
    topLeft = proxy->mapToParent(topLeft);
    QSizeF size = popup->size();
    switch (popupPosition_) {
    case TopLeft:
        topLeft += QPointF(-size.width(), -size.height() - 15);
        break;
    case TopCenter:
        topLeft += QPointF(-(size.width() - btn->width()) / 2, -size.height() - 15);
        break;
    case TopRight:
        topLeft += QPointF(0, -size.height() - 15);
        break;
    case BottomLeft:
        topLeft += QPointF(-size.width(), btn->height() + 15);
        break;
    case BottomCenter:
        topLeft += QPointF(-(size.width() - btn->width()) / 2, btn->height() + 15);
        break;
    case BottomRight:
        topLeft += QPointF(0, btn->height() + 15);
        break;
    }
    QRectF bound = parent->boundingRect();
    QRectF bound2(topLeft, size);
    if (bound2.left() < bound.left())
        topLeft.setX(bound.left());
    else if (bound2.right() > bound.right())
        topLeft.setX(bound.left() + bound.right() - bound2.right());
    if (bound2.top() < bound.top())
        topLeft.setY(topLeft.y() + size.height() + btn->height() + 30);
    else if (bound2.bottom() > bound.bottom())
        topLeft.setY(topLeft.y() - size.height() - btn->height() - 30);
    return parent->mapToItem(popup->parentItem(), topLeft);
}

void ToolbarWidget::getPopupButtons(QList<ToolButton *> &buttons, const QList<ToolButton *> &parents)
{
    if (provider_)
        provider_->getToolButtons(buttons, parents);
    else
        emit popupButtonsRequired(buttons, popupParents_);
}

void ToolbarWidget::onButtonClicked(ToolButton *)
{
    if (provider_)
        provider_->handleToolButton(popupParents_);
    else
        emit buttonClicked(popupParents_);
}

void ToolbarWidget::resizeEvent(QResizeEvent *event)
{
    emit sizeChanged(event->size());
}

void ToolbarWidget::setVisible(bool visible) {
    QWidget::setVisible(visible);
    if (!visible && popUp_) {
        QGraphicsProxyWidget * proxy = popUp_->graphicsProxyWidget();
        if (proxy)
            proxy->hide();
        else
            popUp_->hide();
    }
}

