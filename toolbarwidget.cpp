#include "toolbarwidget.h"
#include "toolbutton.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QWidget>
#include <QStyleOptionButton>

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QWidget(parent)
{
    layout_ = new QHBoxLayout(this);
    style_ = new QStyleOptionButton();
    style_->features = QStyleOptionButton::Flat;
}

static QPixmap widgetToPixmap(QWidget * widget)
{
    QPixmap pm(widget->size());
    QPainter pt(&pm);
    widget->render(&pt);
    return pm;
}

void ToolbarWidget::setToolButtons(QList<ToolButton *> const & buttons)
{
    clear();
    for (ToolButton * b : buttons) {
        QToolButton * btn = new QToolButton();
        btn->setIconSize({40, 40});
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        if (b->icon.type() == QVariant::String)
            btn->setIcon(QIcon(b->icon.toString()));
        else if (b->icon.type() == QVariant::Pixmap)
            btn->setIcon(QIcon(b->icon.value<QPixmap>()));
        else if (b->icon.type() == QVariant::Image)
            btn->setIcon(QIcon(QPixmap::fromImage(b->icon.value<QImage>())));
        else if (!b->icon.isNull())
            btn->setIcon(QIcon(widgetToPixmap(b->icon.value<QWidget *>())));
        btn->setText(b->title);
        void (ToolbarWidget::*slot)() = &ToolbarWidget::buttonClicked;
        QObject::connect(btn, &QToolButton::clicked, this, slot);
        layout_->addWidget(btn);
        buttons_.insert(btn, b);
    }
    layout_->update();
}

void ToolbarWidget::clear()
{
    for (QWidget * w : buttons_.keys()) {
        layout_->removeWidget(w);
        w->deleteLater();
    }
    buttons_.clear();
}

void ToolbarWidget::buttonClicked()
{
    QWidget * btn = qobject_cast<QWidget *>(sender());
    ToolButton * b = buttons_.value(btn);
    if (b)
        emit buttonClicked(b);
}
