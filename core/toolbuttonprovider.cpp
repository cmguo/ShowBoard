#include "toolbuttonprovider.h"
#include "toolbutton.h"
#include "optiontoolbuttons.h"

#include <QVariant>
#include <QMetaMethod>

ToolButtonProvider::ToolButtonProvider(QObject * parent)
    : LifeObject(parent)
{
}

ToolButtonProvider::~ToolButtonProvider()
{
    for (ToolButton * btn : nonSharedButtons_)
        delete btn;
    nonSharedButtons_.clear();
}

void ToolButtonProvider::exec(QByteArray const & cmd, QGenericArgument arg0,
                   QGenericArgument arg1, QGenericArgument arg2)
{
    int index = metaObject()->indexOfSlot(cmd);
    if (index < 0)
        return;
    QMetaMethod method = metaObject()->method(index);
    method.parameterType(index);
    method.invoke(this, arg0, arg1, arg2);
}

void ToolButtonProvider::exec(QByteArray const & cmd, QStringList const & args)
{
    int index = metaObject()->indexOfSlot(cmd);
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
    ToolButton * button = buttons.back();
    int i = 0;
    for (; i < buttons.size(); ++i) {
        if (buttons[i]->flags & ToolButton::OptionsGroup) {
            button = buttons[i];
            break;
        }
    }
    if (i == buttons.size())
        --i;
    if (button == buttons.back() && !button->flags.testFlag(ToolButton::UnionUpdate)) {
        handleToolButton(button); // do simple handle
        return;
    }
    QStringList args;
    for (int j = i + 1; j < buttons.size(); ++j) {
        args.append(buttons[j]->name);
    }
    inHandle = true;
    exec(button->name, args);
    if (button->flags & ToolButton::NeedUpdate) {
        updateToolButton(button);
        if (button->flags.testFlag(ToolButton::UnionUpdate)) {
            QList<ToolButton *> siblins;
            getToolButtons(siblins, buttons.mid(0, i));
            int n = siblins.indexOf(button);
            for (int i = n - 1; i >= 0; --i) {
                if (siblins[i]->flags.testFlag(ToolButton::UnionUpdate)) {
                    updateToolButton(siblins[i]);
                } else {
                    break;
                }
            }
            for (int i = n + 1; i < siblins.size(); ++i) {
                if (siblins[i]->flags.testFlag(ToolButton::UnionUpdate)) {
                    updateToolButton(siblins[i]);
                } else {
                    break;
                }
            }
        }
    }
    inHandle = false;
}

void ToolButtonProvider::handleToolButton(ToolButton *button)
{
    exec(button->name, nullptr);
    if (button->flags & ToolButton::NeedUpdate)
        updateToolButton(button);
}

static std::map<QMetaObject const *, std::map<QByteArray, OptionToolButtons*>> & optionButtons()
{
    static std::map<QMetaObject const *, std::map<QByteArray, OptionToolButtons*>> smap;
    return smap;
}

void ToolButtonProvider::updateToolButton(ToolButton *button)
{
    auto iopt = optionButtons().find(metaObject());
    if (iopt != optionButtons().end()) {
        auto iopt2 = iopt->second.find(button->name);
        if (iopt2 != iopt->second.end()) {
            QVariant value = getOption(button->name);
            iopt2->second->update(button, value);
        }
    }
}

QString ToolButtonProvider::toolsString(QByteArray const & parent) const
{
    if (parent.isEmpty()) {
        int i = metaObject()->indexOfClassInfo("toolsString");
        if (i >= 0)
            return metaObject()->classInfo(i).value();
        return property("toolsString").toString();
    }
    return nullptr;
}

void ToolButtonProvider::setOption(QByteArray const & key, QVariant value)
{
    int i = metaObject()->indexOfProperty(key);
    if (i < 0)
        return;
    QMetaProperty p = metaObject()->property(i);
    if (!value.canConvert(p.type()))
        value.convert(QMetaType::QString);
    LifeObject::setProperty(key, value);
}

QVariant ToolButtonProvider::getOption(const QByteArray &key)
{
    return property(key);
}

QList<ToolButton *> ToolButtonProvider::tools(QByteArray const & parent)
{
    auto iopt = optionButtons().find(metaObject());
    if (iopt != optionButtons().end()) {
        auto iopt2 = iopt->second.find(parent);
        if (iopt2 != iopt->second.end()) {
            QVariant value = getOption(parent);
            return iopt2->second->buttons(value);
        }
    }
    static std::map<QMetaObject const *, std::map<QString, QList<ToolButton *>>> smap;
    auto iter = smap.find(metaObject());
    if (iter == smap.end()) {
        std::map<QString, QList<ToolButton *>> t;
        iter = smap.insert(std::make_pair(metaObject(), std::move(t))).first;
    }
    auto iter2 = iter->second.find(parent);
    if (iter2 == iter->second.end()) {
        QString tools = this->toolsString(parent);
        iter2 = iter->second.insert(
                    std::make_pair(parent, ToolButton::makeButtons(tools))).first;
    }
    QList<ToolButton*> btns(iter2->second);
    for (ToolButton *& button : btns) {
        if (button->flags & ToolButton::NeedUpdate) {
            ToolButton* button2 = nonSharedButtons_.value(button);
            if (button2 == nullptr) {
                button2 = new ToolButton(*button);
                nonSharedButtons_.insert(button, button2);
            }
            if (!inHandle)
                updateToolButton(button2);
            button = button2;
        }
    }
    return btns;
}

RegisterOptionsButtons::RegisterOptionsButtons(const QMetaObject &meta, const char *parent, OptionToolButtons &buttons)
{
    optionButtons()[&meta][parent] = &buttons;
}
