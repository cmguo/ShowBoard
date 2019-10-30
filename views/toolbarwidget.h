#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include "ShowBoard_global.h"

#include "core/toolbutton.h"

#include <QWidget>
#include <QMap>
#include <QList>

class QHBoxLayout;
class QStyleOptionButton;

class SHOWBOARD_EXPORT ToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolbarWidget(QWidget *parent = nullptr);

public:
    void setToolButtons(QList<ToolButton *> const & buttons);

    void setToolButtons(ToolButton buttons[], int count);

    void clear();

signals:
    void buttonClicked(ToolButton * button);

public slots:
    void buttonClicked();

private:
    void addToolButton(ToolButton * button);

private:
    QHBoxLayout * layout_;
    QMap<QWidget *, ToolButton *> buttons_;
    QStyleOptionButton * style_;
};

#endif // TOOLBARWIDGET_H
