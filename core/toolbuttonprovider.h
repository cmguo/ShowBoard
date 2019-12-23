#ifndef TOOLBUTTONPROVIDER_H
#define TOOLBUTTONPROVIDER_H

#include "ShowBoard_global.h"
#include "lifeobject.h"

#include <QObject>

class ToolButton;

class SHOWBOARD_EXPORT ToolButtonProvider : public LifeObject
{
    Q_OBJECT
public:
    ToolButtonProvider(QObject * parent = nullptr);

signals:
    void buttonsChanged();

public:
    /*
     * collect context menu of this control
     *  copy, delete is add according to flags
     *  other menus can be defined with toolsString()
     */
    virtual void getToolButtons(QList<ToolButton *> & buttons,
                                QList<ToolButton *> const & parents = {});

    virtual void getToolButtons(QList<ToolButton *> & buttons,
                                ToolButton * parent);

    /*
     * handle button click,
     *  copy, delete are handled by canvas and should not go here
     */
    virtual void handleToolButton(QList<ToolButton *> const & buttons);

    virtual void updateToolButton(ToolButton * button);

public:
    /*
     * invoke slot by name, use for lose relation call
     */
    void exec(QString const & cmd, QGenericArgument arg0 = QGenericArgument(),
              QGenericArgument arg1 = QGenericArgument(), QGenericArgument arg2 = QGenericArgument());

    /*
     * invoke slot by name, use for lose relation call
     */
    void exec(QString const & cmd, QStringList const & args);

protected:
    void setToolsString(QString const & tools);

    /*
     * stringlized definition of context menus
     *   menu strings is seperated with ';' and menu define parts with '|'
     *   for example:
     *     "open()|打开|:/showboard/icons/icon_open.png;"
     */
    virtual QString toolsString(QString const & parent = QString()) const;

    virtual void setOption(QString const & key, QVariant value);

    QList<ToolButton *> & tools(QString const & parent = QString());
};

#endif // TOOLBUTTONPROVIDER_H
