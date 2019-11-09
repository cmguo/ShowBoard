#include "toolbarwidget.h"
#include "core/toolbutton.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QWidget>
#include <QStyleOptionButton>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QLabel>

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QWidget(parent)
    , template_(nullptr)
{
    layout_ = new QHBoxLayout(this);
    style_ = new QStyleOptionButton();
    style_->features = QStyleOptionButton::Flat;
    this->setObjectName(QString::fromUtf8("toolbarwidget"));
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet("QPushButton,.QLabel{color:#80ffffff;background-color:#00000000;border:none;font-size:14pt;spacing: 30px;} QPushButton{qproperty-iconSize: 30px 30px;} #toolbarwidget{background-color:#C8000000;border-radius:3px;}");
    layout_->setContentsMargins(10,10,10,10);
    this->setLayout(layout_);
}

void ToolbarWidget::setButtonTemplate(int typeId)
{
    QMetaObject const * meta = QMetaType::metaObjectForType(typeId);
    if (meta && meta->inherits(&QPushButton::staticMetaObject))
        template_ = meta;
}

static QPixmap widgetToPixmap(QWidget * widget)
{
    QPixmap pm(widget->size());
    QPainter pt(&pm);
    widget->render(&pt);
    return pm;
}

static QPixmap itemToPixmap(QGraphicsItem * item)
{
    QPixmap pm(item->boundingRect().size().toSize());
    QPainter pt(&pm);
    QStyleOptionGraphicsItem style;
    item->paint(&pt, &style);
    return pm;
}

void ToolbarWidget::setToolButtons(QList<ToolButton *> const & buttons)
{
    clear();
    for (ToolButton * b : buttons) {
        addToolButton(b);
    }
    updateGeometry(); //
}

void ToolbarWidget::setToolButtons(ToolButton buttons[], int count)
{
    clear();
    for (int i = 0; i < count; ++i) {
        addToolButton(buttons + i);
    }
    updateGeometry();
}

void ToolbarWidget::addToolButton(ToolButton * button)
{
    QPushButton * btn = template_
            ? qobject_cast<QPushButton *>(template_->newInstance())
            : new QPushButton;
    QVariant & icon = button->icon;
    if (icon.type() == QVariant::String)
        btn->setIcon(QIcon(icon.toString()));
    else if (icon.type() == QVariant::Pixmap)
        btn->setIcon(QIcon(icon.value<QPixmap>()));
    else if (icon.type() == QVariant::Image)
        btn->setIcon(QIcon(QPixmap::fromImage(icon.value<QImage>())));
    else if (icon.type() == QVariant::UserType) {
        if (icon.userType() == QMetaType::QObjectStar)
            btn->setIcon(QIcon(widgetToPixmap(icon.value<QWidget *>())));
        else if (icon.userType() == qMetaTypeId<QGraphicsItem *>())
            btn->setIcon(QIcon(itemToPixmap(icon.value<QGraphicsItem *>())));
    }
    btn->setText(QString(" %1").arg(button->title));
    void (ToolbarWidget::*slot)() = &ToolbarWidget::buttonClicked;
    QObject::connect(btn, &QPushButton::clicked, this, slot);
    if(buttons_.size()>0){
        QLabel *splitLabel = new QLabel(this);
        splitLabel->setText("|");
        layout_->addWidget(splitLabel);
        splitWidget_.append(splitLabel);
    }
    layout_->addWidget(btn);
    buttons_.insert(btn, button);
}

void ToolbarWidget::clear()
{
    for (QWidget * w : buttons_.keys()) {
        layout_->removeWidget(w);
        w->deleteLater();
    }
    for(QWidget *w : splitWidget_){
        layout_->removeWidget(w);
        w->deleteLater();
    }
    splitWidget_.clear();
    buttons_.clear();
    layout_->activate();
    QGraphicsProxyWidget * proxy = graphicsProxyWidget();
    if (proxy)
        proxy->resize(minimumSize());
}

void ToolbarWidget::buttonClicked()
{
    QWidget * btn = qobject_cast<QWidget *>(sender());
    ToolButton * b = buttons_.value(btn);
    if (b)
        emit buttonClicked(b);
}

void ToolbarWidget::resizeEvent(QResizeEvent *event)
{
    emit sizeChanged(event->size());
}
