#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include "toolbutton.h"

#include <QWidget>
#include <QMap>
#include <QList>

class QHBoxLayout;
class QStyleOptionButton;

class ToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolbarWidget(QWidget *parent = nullptr);

public:
    typedef std::function<void(void)> Action;

    void setToolButtons(QList<ToolButton *> const & buttons);

    void clear();

signals:
    void buttonClicked(ToolButton * button);

public slots:
    void buttonClicked();

private:
    QHBoxLayout * layout_;
    QMap<QWidget *, ToolButton *> buttons_;
    QStyleOptionButton * style_;
};

#endif // TOOLBARWIDGET_H
