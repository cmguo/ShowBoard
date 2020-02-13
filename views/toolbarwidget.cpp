#include "qsshelper.h"
#include "toolbarwidget.h"
#include "core/toolbutton.h"
#include "core/toolbuttonprovider.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QPushButton>

static QssHelper QSS(":/showboard/qss/toolbar.qss");

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : ToolbarWidget(true, parent)
{
}

ToolbarWidget::ToolbarWidget(bool horizontal, QWidget *parent)
    : QFrame(parent)
    , template_(nullptr)
    , popupPosition_(BottomRight)
    , popUp_(nullptr)
    , provider_(nullptr)
{
    if (horizontal)
        layout_ = new QHBoxLayout(this);
    else
        layout_ = new QVBoxLayout(this);
    setObjectName("toolbarwidget");
    setWindowFlag(Qt::FramelessWindowHint);
    setStyleSheet(QSS);
    if (horizontal)
        layout_->setContentsMargins(10, 10, 10, 10);
    setLayout(layout_);
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
    if (button == &ToolButton::SPLITTER) {
        QFrame *splitter = new QFrame(this);
        splitter->setFrameShape(QFrame::VLine);
        splitter->setFrameShadow(QFrame::Raised);
        //splitter->setLineWidth(1);
        widget = splitter;
    } else if (button == &ToolButton::LINE_BREAK) {
        ++row; col = 0;
        return;
    } else if (button == &ToolButton::LINE_SPLITTER) {
        ++row; col = -1;
        QFrame *splitter = new QFrame(this);
        splitter->setFrameShape(QFrame::HLine);
        splitter->setFrameShadow(QFrame::Sunken);
        //splitter->setLineWidth(0);
        widget = splitter;
    } else if (button == &ToolButton::PLACE_HOOLDER) {
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
        btn->setFocusPolicy(Qt::NoFocus);
        applyButton(btn, parent, button);
        void (ToolbarWidget::*slot)() = &ToolbarWidget::buttonClicked;
        QObject::connect(btn, &QPushButton::clicked, this, slot);
        widget = btn;
    }
    widget->setObjectName(button->name);
    if (parent) {
        QGridLayout *gridLayout = static_cast<QGridLayout*>(layout);
        if (col == -1) {
            gridLayout->addWidget(widget, row, 0, 1, -1);
            ++row; col = 0;
        } else {
            if (button->flags & ToolButton::CustomWidget) {
                gridLayout->setContentsMargins(0, 0, 0, 0);
            } else {
                gridLayout->setContentsMargins(10, 10, 10, 10);
            }
            gridLayout->addWidget(widget, row, col);
            ++col;
        }
    } else {
        layout->addWidget(widget);
    }
    if (button) {
        buttons.insert(widget, button);
    }
}

void ToolbarWidget::applyButton(QPushButton * btn, ToolButton * parent, ToolButton *button)
{
    //btn->setIconSize(QSize(40,40));
    btn->setIcon(button->getIcon());
    btn->setText(((button->flags & ToolButton::Popup) && !button->title.isEmpty())
                 ? button->title + " ▼" : button->title);
    if (parent && (parent->flags & ToolButton::OptionsGroup)) {
        btn->setCheckable(true);
        btn->setChecked(button->flags & ToolButton::Selected);
    }
    if (button->flags & ToolButton::Checkable) {
        btn->setCheckable(true);
        btn->setChecked(button->flags & ToolButton::Checked);
    }
    btn->setEnabled(!button->flags.testFlag(ToolButton::Disabled));
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
    popUp_ = createPopupWidget();
    QGraphicsProxyWidget * proxy = graphicsProxyWidget();
    if (proxy) {
        proxy = new QGraphicsProxyWidget(proxy);
        proxy->setWidget(popUp_);
        proxy->hide();
    } else {
        popUp_->setParent(parentWidget());
        popUp_->hide();
    }
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
        if (buttons_.empty()) // may delete & clear
            return;
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
                    layout_->activate();
                    if (graphicsProxyWidget()) {
                        graphicsProxyWidget()->update();
                        graphicsProxyWidget()->resize(minimumSize());
                    }
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
    QGraphicsItem* parent = graphicsProxyWidget();
    QPointF topLeft = btn->mapTo(this, QPoint(0, 0));
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
    QRectF bound = parent->mapFromScene(parent->scene()->sceneRect()).boundingRect();
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

QWidget *ToolbarWidget::createPopupWidget()
{
    QWidget * widget = new QFrame();
    widget->setWindowFlags(Qt::FramelessWindowHint);
    widget->setStyleSheet(QSS);
    widget->setObjectName("popupwidget");
    widget->setLayout(new QGridLayout());
    return widget;
}

void ToolbarWidget::resizeEvent(QResizeEvent *event)
{
    emit sizeChanged(event->size());
}

void ToolbarWidget::setVisible(bool visible) {
    visible &= !buttons_.empty();
    QFrame::setVisible(visible);
    if (!visible && popUp_) {
        QGraphicsProxyWidget * proxy = popUp_->graphicsProxyWidget();
        if (proxy)
            proxy->hide();
        else
            popUp_->hide();
    }
}

void ToolbarWidget::paintEvent(QPaintEvent *event)
{
    //QFrame::paintEvent(event);
}

