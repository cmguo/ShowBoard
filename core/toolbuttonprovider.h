#ifndef TOOLBUTTONPROVIDER_H
#define TOOLBUTTONPROVIDER_H

#include "ShowBoard_global.h"
#include "lifeobject.h"

#include <QObject>
#include <QMap>

class ToolButton;
class OptionToolButtons;

class SHOWBOARD_EXPORT ToolButtonProvider : public LifeObject
{
    Q_OBJECT
    Q_PROPERTY(QString overrideToolString READ overrideToolString WRITE setOverrideToolString)
public:
    ToolButtonProvider(QObject * parent = nullptr);

    ToolButtonProvider(ToolButtonProvider const & o);

    virtual ~ToolButtonProvider() override;

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

    /*
     * handle button click,
     *  copy, delete are handled by canvas and should not go here
     */
    virtual bool handleToolButton(QList<ToolButton *> const & buttons);

    QString overrideToolString() {
        return overrideToolString_;
    }

    void setOverrideToolString(const QString&);

public:
    /*
     * invoke slot by name, use for lose relation call
     */
    bool exec(QByteArray const & cmd, QGenericArgument arg0 = QGenericArgument(),
              QGenericArgument arg1 = QGenericArgument(), QGenericArgument arg2 = QGenericArgument());

    /*
     * invoke slot by name, use for lose relation call
     */
    bool exec(QByteArray const & cmd, QStringList const & args);

public:
    virtual bool setOption(QByteArray const & key, QVariant value);

    virtual QVariant getOption(QByteArray const & key);

protected:
    virtual void getToolButtons(QList<ToolButton *> & buttons,
                                ToolButton * parent);

    virtual bool handleToolButton(ToolButton * button, QStringList const & args);

    virtual void updateToolButton(ToolButton * button);

protected:
    void setToolsString(QString const & tools);

    void followTrigger(bool v = true);

    void followTrigger(QList<ToolButton *> & buttons, ToolButton * parent = nullptr);

    void attachSubProvider(ToolButtonProvider * provider, bool before = false);

    ToolButton* getStringButton(QByteArray const & name);

    ToolButton* getStringButton(int index);

    // can be called from handleToolButton
    void raiseButtonsChanged();

    /*
     * stringlized definition of context menus
     *   menu strings is seperated with ';' and menu define parts with '|'
     *   for example:
     *     "open()|打开|:/showboard/icon/icon_open.png;"
     */
    virtual QString toolsString(QByteArray const & parent = nullptr) const;

    QList<ToolButton *> tools(ToolButton * parent = nullptr);

private:

private:
    QMap<QByteArray, QList<ToolButton *>> buttons_;
    QList<ToolButton *> privateButtons_;
    ToolButtonProvider * subProviderBefore_;
    ToolButtonProvider * subProviderAfter_;
    bool followTrigger_;
    QString overrideToolString_;
};

class SHOWBOARD_EXPORT RegisterOptionsButtons
{
public:
    RegisterOptionsButtons(QMetaObject const & meta, char const * parent, OptionToolButtons & buttons);
};

#define REGISTER_OPTION_BUTTONS(type, parent, buttons) \
    static RegisterOptionsButtons export_option_buttons_##parent(type::staticMetaObject, #parent, buttons);

#endif // TOOLBUTTONPROVIDER_H
