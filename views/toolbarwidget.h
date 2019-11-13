#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include "ShowBoard_global.h"

#include "core/toolbutton.h"

#include <QWidget>
#include <QMap>
#include <QList>

class QLayout;
class QStyleOptionButton;

class SHOWBOARD_EXPORT ToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolbarWidget(QWidget *parent = nullptr);

    explicit ToolbarWidget(bool horizontal, QWidget *parent = nullptr);

public:
    void setButtonTemplate(int typeId);

    void setToolButtons(QList<ToolButton *> const & buttons);

    void setToolButtons(ToolButton buttons[], int count);

    void clear();

signals:
    void buttonClicked(ToolButton * button);

    void sizeChanged(QSizeF const & size);

public slots:
    void buttonClicked();

private:
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    void addToolButton(ToolButton * button);

private:
    QMetaObject const * template_;
    QLayout * layout_;
    QMap<QWidget *, ToolButton *> buttons_;
    QList<QWidget *> splitWidget_;
    QStyleOptionButton * style_;
};

#endif // TOOLBARWIDGET_H
