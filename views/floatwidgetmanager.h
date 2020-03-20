#ifndef FLOATWIDGETMANAGER_H
#define FLOATWIDGETMANAGER_H

#include "ShowBoard_global.h"

#include <QObject>

class ToolButton;

class SHOWBOARD_EXPORT FloatWidgetManager : public QObject
{
    Q_OBJECT
public:
    static FloatWidgetManager* from(QWidget * widget);

    static QPoint getPopupPosition(QWidget * widget, ToolButton* attachButton);

    enum Flag {
        PositionAtCenter = 1,
        FullLayout = 2,
        RaiseOnShow = 4,
        HideOnLostFocus = 8,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    void setTaskBar(QWidget * bar);

private:
    FloatWidgetManager(QWidget * main);

public:
    // add/show/raise the widget, see @raiseWidget
    void addWidget(QWidget* widget, Flags flags = nullptr);

    // add/show/raise the widget, see @raiseWidget
    void addWidget(QWidget* widget, ToolButton* attachButton,
                   Flags flags = {RaiseOnShow, HideOnLostFocus});

    // remove/hide the widget
    void removeWidget(QWidget* widget);

    // move z-order after all widgets managed here,
    //  but may not last of all siblings
    void raiseWidget(QWidget* widget);

    // save visibility for restore later
    void saveVisibility();

    // call show on widget, you can call it yourself
    void showWidget(QWidget* widget);

    // call hide on widget, you can call it yourself
    void hideWidget(QWidget* widget);

    // hide all widgets managed here
    void hideAll();

    // restore visibility
    void restoreVisibility();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void relayout(QWidget * widget, Flags flags);

    void focusChanged(QWidget* old, QWidget* now);

    QPoint popupPos(QWidget * widget, ToolButton* attachButton);

private:
    QWidget * main_;
    QWidget * taskBar_;
    QWidget * widgetOn_;
    QList<QWidget*> widgets_;
    QList<int> saveStates_;
};

#endif // FLOATWIDGETMANAGER_H
