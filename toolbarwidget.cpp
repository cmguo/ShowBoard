#include "toolbarwidget.h"

#include <QHBoxLayout>
#include <QPushButton>

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QWidget(parent)
{
    layout_ = new QHBoxLayout(this);
}

void ToolbarWidget::addCommond(const QString &title, const QString &icon, Action action)
{
    QPushButton * btn = new QPushButton();
    btn->setIcon(QIcon(icon));
    btn->setText(title);
    QObject::connect(btn, &QPushButton::clicked, action);
    buttons_.append(btn);
    layout_->addWidget(btn);
}

void ToolbarWidget::clear()
{
    for (QWidget * b : buttons_)
        layout_->removeWidget(b);
    buttons_.clear();
}
