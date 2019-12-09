#include "toolbarwidget.h"
#include "core/toolbutton.h"

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
                "QPushButton{qproperty-iconSize: 30px 30px; font-family: '微软雅黑'} "
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
    , popUp_(nullptr)
{
    if (horizontal)
        layout_ = new QHBoxLayout(this);
    else
        layout_ = new QVBoxLayout(this);
    this->setObjectName(QString::fromUtf8("toolbarwidget"));
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet(STYLE);
    if (horizontal)
        layout_->setContentsMargins(10,10,10,10);
    this->setLayout(layout_);
}

void ToolbarWidget::setButtonTemplate(int typeId)
{
    QMetaObject const * meta = QMetaType::metaObjectForType(typeId);
    if (meta && meta->inherits(&QPushButton::staticMetaObject))
        template_ = meta;
}

static QPixmap widgetToPixmap(QWidget * widget, bool destroy)
{
    QPixmap pm(widget->size());
    QPainter pt(&pm);
    widget->render(&pt);
    if (destroy)
        widget->deleteLater();
    return pm;
}

static QPixmap itemToPixmap(QGraphicsItem * item, bool destroy)
{
    QPixmap pm(item->boundingRect().size().toSize());
    QPainter pt(&pm);
    QStyleOptionGraphicsItem style;
    item->paint(&pt, &style);
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
        addToolButton(layout_, b, buttons_,false);
    }
    updateGeometry(); //
}

void ToolbarWidget::setToolButtons(ToolButton buttons[], int count)
{
    clear();
    for (int i = 0; i < count; ++i) {
        addToolButton(layout_, buttons + i, buttons_,false);
    }
    updateGeometry();
}

void ToolbarWidget::updateButton(ToolButton *button)
{
    for (QWidget * w : buttons_.keys()) {
        if (buttons_.value(w) != button)
            continue;
        QPushButton * btn = qobject_cast<QPushButton*>(w);
        btn->setIcon(getIcon(button->icon, !(button->flags & ToolButton::Dynamic)));
        btn->setText(QString(" %1").arg(button->title));
    }
}

void ToolbarWidget::showPopupButtons(const QList<ToolButton *> &buttons)
{
    clearPopup();
    for (ToolButton * b : buttons) {
        addToolButton(popUp_->layout(), b, popupButtons_,true);
    }
    popUp_->show();
}

void ToolbarWidget::showPopupButtons(ToolButton *buttons, int count)
{
    clearPopup();
    for (int i = 0; i < count; ++i) {
        addToolButton(popUp_->layout(), buttons + i, popupButtons_,true);
    }
    popUp_->show();
}

void ToolbarWidget::clearPopup()
{
    if (popUp_) {
        popUp_->hide();
        clearButtons(popUp_->layout(), popupButtons_);
//        popUp_->layout()->activate();
//        QGraphicsProxyWidget * proxy = popUp_->graphicsProxyWidget();
//        if (proxy)
//            proxy->resize(popUp_->minimumSize());
    }
}

void ToolbarWidget::addToolButton(QLayout* layout, ToolButton * button, QMap<QWidget *, ToolButton *>& buttons,bool isPopButton)
{
    QPushButton * btn = template_
            ? qobject_cast<QPushButton *>(template_->newInstance())
            : new QPushButton;
    btn->setIcon(getIcon(button->icon, !(button->flags & ToolButton::Dynamic)));
    btn->setIconSize(QSize(40,40));
    btn->setText(QString(" %1").arg(button->title));
    void (ToolbarWidget::*slot)() = &ToolbarWidget::buttonClicked;
    QObject::connect(btn, &QPushButton::clicked, this, slot);
    if (!isPopButton
            && buttons.size() > 0) {
        QLabel *splitLabel = new QLabel(this);
        splitLabel->setText("|");
        layout->addWidget(splitLabel);
        splitWidget_.append(splitLabel);
    }
    buttons.insert(btn, button);
    int count = buttons.size()-1;
    if(isPopButton){
        btn->setCheckable(true);
        btn->setChecked(button->flags&ToolButton::Selected);
        QGridLayout *gridLayout = static_cast<QGridLayout*>(layout);
        gridLayout->addWidget(btn,count/4,count%4);
    }else{
        layout->addWidget(btn);
    }

}

void ToolbarWidget::clear()
{
    clearPopup();
    clearButtons(layout_, buttons_);
    for(QWidget *w : splitWidget_) {
        layout_->removeWidget(w);
        w->deleteLater();
    }
    splitWidget_.clear();
    layout_->activate();
    QGraphicsProxyWidget * proxy = graphicsProxyWidget();
    if (proxy)
        proxy->resize(minimumSize());
}

void ToolbarWidget::clearButtons(QLayout *layout, QMap<QWidget *, ToolButton *> &buttons)
{
    for (QWidget * w : buttons.keys()) {
        layout->removeWidget(w);
        ToolButton * btn = buttons.value(w);
        if (btn->flags & ToolButton::Dynamic)
            delete btn;
        w->deleteLater();
    }
    layout->update();
    buttons.clear();
}

void ToolbarWidget::createPopup()
{
    delete popUp_;
    popUp_ = new QWidget();
    popUp_->setStyleSheet(STYLE);
    popUp_->setObjectName(QString::fromUtf8("popupwidget"));
    QGraphicsProxyWidget * proxy = graphicsProxyWidget();
    if (proxy) {
        proxy = new QGraphicsProxyWidget(proxy);
        proxy->setWidget(popUp_);
    } else {
        popUp_->setParent(parentWidget()->parentWidget());
    }
    popUp_->setLayout(new QGridLayout());
}

void ToolbarWidget::buttonClicked()
{
    QWidget * btn = qobject_cast<QWidget *>(sender());
    ToolButton * button = popupButtons_.value(btn);
    if (!button) {
        button = buttons_.value(btn);
        if (!button) return;
        clearPopup();
        popupParents_.clear();
    }
    popupParents_.append(button);
    if (button->flags & ToolButton::Popup) {
        QList<ToolButton *> buttons;
        emit popupButtonsRequired(buttons, popupParents_);
        if (popUp_ == nullptr) {
            createPopup();
        }
        QGraphicsProxyWidget * proxy = graphicsProxyWidget();
        if (proxy) {
            QGraphicsProxyWidget * proxy2 = popUp_->graphicsProxyWidget();
            QPoint pos = btn->mapTo(this, QPoint(0, btn->height() + 15));
            QPointF pos2 = proxy->mapToItem(proxy2->parentItem(), QPointF(pos));
            proxy2->setPos(pos2);
        } else {
            QPoint pos = btn->mapTo(popUp_->parentWidget(), QPoint(0, btn->height() + 10));
            popUp_->move(pos);
        }
        showPopupButtons(buttons);
    } else {
        emit buttonClicked(popupParents_);
        int i = 0;
        for (; i < popupParents_.size(); ++i) {
            if (popupParents_[i]->flags & ToolButton::OptionsGroup) {
                button = popupParents_[i];
                break;
            }
        }
        if (button->flags & ToolButton::NeedUpdate) {
            updateButton(button);
        }
        popupParents_.pop_back();
        popUp_->hide();
    }
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
