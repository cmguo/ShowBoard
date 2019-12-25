#include "toolbuttonprovider.h"
#include "toolbutton.h"

#include <QVariant>
#include <QMetaMethod>

ToolButtonProvider::ToolButtonProvider(QObject * parent)
    : LifeObject(parent)
{
}

void ToolButtonProvider::exec(QString const & cmd, QGenericArgument arg0,
                   QGenericArgument arg1, QGenericArgument arg2)
{
    int index = metaObject()->indexOfSlot(cmd.toUtf8());
    if (index < 0)
        return;
    QMetaMethod method = metaObject()->method(index);
    method.parameterType(index);
    method.invoke(this, arg0, arg1, arg2);
}

void ToolButtonProvider::exec(QString const & cmd, QStringList const & args)
{
    int index = metaObject()->indexOfSlot(cmd.toUtf8());
    if (index < 0) {
        if (args.size() == 1) {
            setOption(cmd, args[0]);
        }
        return;
    }
    QMetaMethod method = metaObject()->method(index);
    if (method.parameterCount() >= 4)
        return;
    QGenericArgument argv[4];
    QVariant varg[4];
    for (int i = 0; i < method.parameterCount(); ++i) {
        if (i < args.size())
            varg[i] = args[i];
        int t = method.parameterType(i);
        if (!varg[i].canConvert(t))
            return;
        if (!varg[i].convert(t))
            return;
        argv[i] = QGenericArgument(QMetaType::typeName(t), varg[i].data());
    }
    method.invoke(this, argv[0], argv[1], argv[2], argv[3]);
}

void ToolButtonProvider::setToolsString(const QString &tools)
{
    setProperty("toolsString", tools);
}

void ToolButtonProvider::getToolButtons(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents)
{
    getToolButtons(buttons, parents.empty() ? nullptr : parents.last());
}

void ToolButtonProvider::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (!buttons.empty())
        buttons.append(&ToolButton::SPLITER);
    buttons.append(tools(parent ? parent->name : nullptr));
    if (buttons.endsWith(&ToolButton::SPLITER))
        buttons.pop_back();
}

static bool inHandle = false;

void ToolButtonProvider::handleToolButton(QList<ToolButton *> const & buttons)
{
    inHandle = true;
    ToolButton * button = buttons.back();
    int i = 0;
    for (; i < buttons.size(); ++i) {
        if (buttons[i]->flags & ToolButton::OptionsGroup) {
            button = buttons[i];
            break;
        }
    }
    QStringList args;
    for (++i; i < buttons.size(); ++i) {
        args.append(buttons[i]->name);
    }
    exec(button->name, args);
    if (button->flags & ToolButton::NeedUpdate) {
        updateToolButton(button);
        if (button->flags & ToolButton::UnionUpdate) {
            QList<ToolButton *> siblins;
            getToolButtons(siblins, buttons.mid(0, buttons.size() - 1));
            int n = siblins.indexOf(button);
            for (int i = n - 1; i >= 0; --i) {
                if (siblins[i]->flags & ToolButton::UnionUpdate) {
                    updateToolButton(siblins[i]);
                } else {
                    break;
                }
            }
            for (int i = n + 1; i < siblins.size(); ++i) {
                if (siblins[i]->flags & ToolButton::UnionUpdate) {
                    updateToolButton(siblins[i]);
                } else {
                    break;
                }
            }
        }
    }
    inHandle = false;
}

void ToolButtonProvider::updateToolButton(ToolButton *button)
{
    (void) button;
}

QString ToolButtonProvider::toolsString(QString const & parent) const
{
    if (parent.isEmpty())
        return property("toolsString").toString();
    return nullptr;
}

void ToolButtonProvider::setOption(QString const & key, QVariant value)
{
    LifeObject::setProperty(key.toUtf8(), value);
}

QList<ToolButton *> & ToolButtonProvider::tools(QString const & parent)
{
    static std::map<QMetaObject const *, std::map<QString, QList<ToolButton *>>> slist;
    auto iter = slist.find(metaObject());
    if (iter == slist.end()) {
        std::map<QString, QList<ToolButton *>> t;
        iter = slist.insert(std::make_pair(metaObject(), std::move(t))).first;
    }
    auto iter2 = iter->second.find(parent);
    if (iter2 == iter->second.end()) {
        QString tools = this->toolsString(parent);
        iter2 = iter->second.insert(
                    std::make_pair(parent, ToolButton::makeButtons(tools))).first;
    }
    if (inHandle)
        return iter2->second;
    for (ToolButton * button : iter2->second) {
        if (button->flags & ToolButton::NeedUpdate) {
            updateToolButton(button);
        }
    }
    return iter2->second;
}
