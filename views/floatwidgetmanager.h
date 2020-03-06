#ifndef FLOATWIDGETMANAGER_H
#define FLOATWIDGETMANAGER_H

#include <QObject>

class FloatWidgetManager : QObject
{
    Q_OBJECT
public:
    static FloatWidgetManager* from(QWidget * main);

private:
    FloatWidgetManager(QWidget * main);

public:
    // add/show/raise the widget, see @raiseWidget
    void addWidget(QWidget* widget);

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

private:
    QWidget * widgetUnder();

private:
    QWidget * main_;
    QWidget * widgetOn_;
    QList<QWidget*> widgets_;
    QList<int> saveStates_;
};

#endif // FLOATWIDGETMANAGER_H
