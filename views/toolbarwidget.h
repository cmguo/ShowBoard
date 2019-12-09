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

    void showPopupButtons(QList<ToolButton *> const & buttons);

    void showPopupButtons(ToolButton buttons[], int count);

    void updateButton(ToolButton * button);

    void clear();

    void clearPopup();

signals:
    void buttonClicked(QList<ToolButton *> const & buttons);

    void popupButtonsRequired(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents);

    void sizeChanged(QSizeF const & size);

public slots:
    void buttonClicked();

private:
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual void setVisible(bool visible) override;

private:
    void addToolButton(QLayout * layout, ToolButton * button, QMap<QWidget *, ToolButton *>& buttons,bool isPopButton);

    void clearButtons(QLayout * layout, QMap<QWidget *, ToolButton *>& buttons);

    void createPopup();

private:
    QMetaObject const * template_;
    QLayout * layout_;
    QMap<QWidget *, ToolButton *> buttons_;
    QList<QWidget *> splitWidget_;
    //
    QWidget * popUp_ = nullptr;
    QMap<QWidget *, ToolButton *> popupButtons_;
    QList<ToolButton *> popupParents_;
    QList<QWidget *> popupSplitWidget_;
};

#endif // TOOLBARWIDGET_H
