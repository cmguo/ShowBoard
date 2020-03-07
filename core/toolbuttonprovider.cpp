#include "toolbuttonprovider.h"
#include "toolbutton.h"
#include "optiontoolbuttons.h"

#include <QVariant>
#include <QMetaMethod>

ToolButtonProvider::ToolButtonProvider(QObject * parent)
    : LifeObject(parent)
    , followTrigger_(false)
{
}

ToolButtonProvider::~ToolButtonProvider()
{
    for (ToolButton * btn : privateButtons_)
        delete btn;
    privateButtons_.clear();
}

void ToolButtonProvider::exec(QByteArray const & cmd, QGenericArgument arg0,
                   QGenericArgument arg1, QGenericArgument arg2)
{
    int index = metaObject()->indexOfSlot(cmd);
    if (index < 0) {
        QVariantList list;
        if (arg0.data()) {
            list.append(QVariant(QMetaType::type(arg0.name()), arg0.data()));
            if (arg1.data()) {
                list.append(QVariant(QMetaType::type(arg1.name()), arg1.data()));
                if (arg2.data()) {
                    list.append(QVariant(QMetaType::type(arg2.name()), arg2.data()));
                }
            }
        }
        if (list.isEmpty())
            setOption(cmd, QVariant());
        else if (list.size() == 1)
            setOption(cmd, list.front());
        else
            setOption(cmd, list);
        return;
    }
    QMetaMethod method = metaObject()->method(index);
    method.parameterType(index);
    method.invoke(this, arg0, arg1, arg2);
}

void ToolButtonProvider::exec(QByteArray const & cmd, QStringList const & args)
{
    int index = metaObject()->indexOfSlot(cmd);
    if (index < 0) {
        if (args.size() == 0) {
            setOption(cmd, QVariant());
        } else if (args.size() == 1) {
            setOption(cmd, args[0]);
        } else {
            QVariantList list;
            for (QString const & arg : args)
                list.append(arg);
            setOption(cmd, list);
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

void ToolButtonProvider::followTrigger(bool v)
{
    followTrigger_ = v;
}

void ToolButtonProvider::followTrigger(QList<ToolButton *> &buttons, ToolButton *parent)
{
    for (ToolButton * button : buttons) {
        if (parent && parent->isOptionsGroup())
            connect(button, &ToolButton::triggered, this, [this, button, parent]() {
                handleToolButton({parent, button});
            });
        else
            connect(button, &ToolButton::triggered, this, [this, button]() {
                handleToolButton({button});
            });
    }
}

static bool inHandle = false;

static std::map<QMetaObject const *, std::map<QByteArray, OptionToolButtons*>> & optionButtons()
{
    static std::map<QMetaObject const *, std::map<QByteArray, OptionToolButtons*>> smap;
    return smap;
}

static OptionToolButtons* optionButton(QMetaObject const * meta, QByteArray const & name)
{
    auto iopt = optionButtons().find(meta);
    if (iopt != optionButtons().end()) {
        auto iopt2 = iopt->second.find(name);
        if (iopt2 != iopt->second.end()) {
            return iopt2->second;
        }
    }
    return nullptr;
}

ToolButton* ToolButtonProvider::getStringButton(const QByteArray &name)
{
    bool in = inHandle;
    inHandle = true;
    QList<ToolButton *> btns = tools();
    inHandle = in;
    for (ToolButton * b : btns)
        if (b->name() == name)
            return b;
    return nullptr;
}

ToolButton* ToolButtonProvider::getStringButton(int index)
{
    bool in = inHandle;
    inHandle = true;
    QList<ToolButton *> btns = tools();
    inHandle = in;
    if (index < btns.size())
        return btns.at(index);
    return nullptr;
}

void ToolButtonProvider::getToolButtons(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents)
{
    ToolButton *parent = parents.empty() ? nullptr : parents.last();
    getToolButtons(buttons, parent);
}

void ToolButtonProvider::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (!buttons.empty())
        buttons.append(&ToolButton::SPLITTER);
    buttons.append(tools(parent));
    if (buttons.endsWith(&ToolButton::SPLITTER))
        buttons.pop_back();
}

void ToolButtonProvider::handleToolButton(QList<ToolButton *> const & buttons)
{
    ToolButton * button = buttons.back();
    int i = 0;
    for (; i < buttons.size(); ++i) {
        if (buttons[i]->isOptionsGroup()) {
            button = buttons[i];
            break;
        }
    }
    if (i == buttons.size())
        --i;
    QStringList args;
    for (int j = i + 1; j < buttons.size(); ++j) {
        args.append(buttons[j]->name());
    }
    inHandle = true;
    handleToolButton(button, args);
    if (button->needUpdate()) {
        updateToolButton(button);
        if (button->unionUpdate()) {
            QList<ToolButton *> siblins;
            getToolButtons(siblins, buttons.mid(0, i));
            int n = siblins.indexOf(button);
            for (int i = n - 1; i >= 0; --i) {
                if (siblins[i]->unionUpdate()) {
                    updateToolButton(siblins[i]);
                } else {
                    break;
                }
            }
            for (int i = n + 1; i < siblins.size(); ++i) {
                if (siblins[i]->unionUpdate()) {
                    updateToolButton(siblins[i]);
                } else {
                    break;
                }
            }
        }
    }
    if (followTrigger_) {
        QByteArray name = button->name();
        OptionToolButtons* opt = optionButton(metaObject(), name);
        if (opt) {
            QVariant value = getOption(name);
            opt->updateValue(value);
        }
    }
    inHandle = false;
}

void ToolButtonProvider::handleToolButton(ToolButton *button, QStringList const & args)
{
    exec(button->name(), args);
}

void ToolButtonProvider::updateToolButton(ToolButton *button)
{
    QByteArray name = button->name();
    OptionToolButtons* opt = optionButton(metaObject(), name);
    if (opt) {
        QVariant value = getOption(name);
        opt->updateValue(value);
        opt->updateParent(button, value);
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

QList<ToolButton *> ToolButtonProvider::tools(ToolButton * parent)
{
    QByteArray name = parent ? parent->name() : "";
    auto ilst = buttons_.find(name);
    if (ilst != buttons_.end()) {
        if (inHandle)
            return *ilst;
        OptionToolButtons* opt = optionButton(metaObject(), name);
        if (opt) {
            QVariant value = getOption(name);
            opt->updateValue(value);
            return *ilst;
        }
        for (ToolButton *& button : *ilst) {
            if (button->needUpdate()) {
                 updateToolButton(button);
            }
        }
        return *ilst;
    }
    if (parent) {
        OptionToolButtons* opt = optionButton(metaObject(), name);
        if (opt) {
            QVariant value = getOption(name);
            QList<ToolButton *> btns = opt->getButtons(value);
            if (followTrigger_)
                followTrigger(btns, parent);
            return btns;
        }
    }
    static std::map<QMetaObject const *, std::map<QString, QList<ToolButton *>>> smap;
    auto iter = smap.find(metaObject());
    if (iter == smap.end()) {
        std::map<QString, QList<ToolButton *>> t;
        iter = smap.insert(std::make_pair(metaObject(), std::move(t))).first;
    }
    auto iter2 = iter->second.find(name);
    if (iter2 == iter->second.end()) {
        QString tools = this->toolsString(name);
        iter2 = iter->second.insert(
                    std::make_pair(name, ToolButton::makeButtons(tools))).first;
    }
    QList<ToolButton*> btns(iter2->second);
    for (ToolButton *& button : btns) {
        if (button->needUpdate()) {
            button = new ToolButton(*button);
            privateButtons_.append(button);
            if (!inHandle)
                updateToolButton(button);
        }
    }
    if (followTrigger_) {
        followTrigger(btns, parent);
    }
    buttons_.insert(name, btns);
    return btns;
}

RegisterOptionsButtons::RegisterOptionsButtons(const QMetaObject &meta, const char *parent, OptionToolButtons &buttons)
{
    optionButtons()[&meta][parent] = &buttons;
}
