#include "toolbuttonprovider.h"
#include "toolbutton.h"
#include "optiontoolbuttons.h"

#include <QVariant>
#include <QMetaMethod>

static ToolButtonProvider * inHandle = nullptr;

ToolButtonProvider::ToolButtonProvider(QObject * parent)
    : LifeObject(parent)
    , subProviderBefore_(nullptr)
    , subProviderAfter_(nullptr)
    , followTrigger_(false)
{
}

ToolButtonProvider::ToolButtonProvider(const ToolButtonProvider &o)
    : LifeObject(o)
    , subProviderBefore_(nullptr)
    , subProviderAfter_(nullptr)
    , followTrigger_(o.followTrigger_)
{
}

ToolButtonProvider::~ToolButtonProvider()
{
    for (ToolButton * btn : privateButtons_)
        delete btn;
    privateButtons_.clear();
    if (inHandle == this)
        inHandle = nullptr;
}

bool ToolButtonProvider::exec(QByteArray const & cmd, QGenericArgument arg0,
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
            return setOption(cmd, QVariant());
        else if (list.size() == 1)
            return setOption(cmd, list.front());
        else
            return setOption(cmd, list);
    }
    QMetaMethod method = metaObject()->method(index);
    method.parameterType(index);
    return method.invoke(this, arg0, arg1, arg2);
}

bool ToolButtonProvider::exec(QByteArray const & cmd, QStringList const & args)
{
    int index = metaObject()->indexOfSlot(cmd);
    if (index < 0) {
        if (args.size() == 0) {
            return setOption(cmd, QVariant());
        } else if (args.size() == 1) {
            return setOption(cmd, args[0]);
        } else {
            QVariantList list;
            for (QString const & arg : args)
                list.append(arg);
            return setOption(cmd, list);
        }
    }
    QMetaMethod method = metaObject()->method(index);
    if (method.parameterCount() >= 4)
        return false;
    QGenericArgument argv[4];
    QVariant varg[4];
    for (int i = 0; i < method.parameterCount(); ++i) {
        if (i < args.size())
            varg[i] = args[i];
        int t = method.parameterType(i);
        if (!varg[i].canConvert(t))
            return false;
        if (!varg[i].convert(t))
            return false;
        argv[i] = QGenericArgument(QMetaType::typeName(t), varg[i].data());
    }
    return method.invoke(this, argv[0], argv[1], argv[2], argv[3]);
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

void ToolButtonProvider::attachSubProvider(ToolButtonProvider *provider, bool before)
{
    ToolButtonProvider * & subProvider = before ? subProviderBefore_ : subProviderAfter_;
    if (provider == subProvider)
        return;
    if (subProvider) {
        subProvider->disconnect(this);
    }
    subProvider = provider;
    if (subProvider) {
        connect(subProvider, &ToolButtonProvider::buttonsChanged, this, &ToolButtonProvider::buttonsChanged);
    }
    emit buttonsChanged();
}

static std::map<QMetaObject const *, std::map<QByteArray, OptionToolButtons*>> & optionButtons()
{
    static std::map<QMetaObject const *, std::map<QByteArray, OptionToolButtons*>> smap;
    return smap;
}

static OptionToolButtons* optionButton(QMetaObject const * meta, QByteArray const & name)
{
    while (meta != &ToolButtonProvider::staticMetaObject) {
        auto iopt = optionButtons().find(meta);
        if (iopt != optionButtons().end()) {
            auto iopt2 = iopt->second.find(name);
            if (iopt2 != iopt->second.end()) {
                return iopt2->second;
            }
        }
        meta = meta->superClass();
    }
    return nullptr;
}

ToolButton* ToolButtonProvider::getStringButton(const QByteArray &name)
{
    ToolButtonProvider* in = inHandle;
    inHandle = this;
    QList<ToolButton *> btns = tools();
    inHandle = in;
    for (ToolButton * b : btns)
        if (b->name() == name)
            return b;
    return nullptr;
}

ToolButton* ToolButtonProvider::getStringButton(int index)
{
    ToolButtonProvider* in = inHandle;
    inHandle = this;
    QList<ToolButton *> btns = tools();
    inHandle = in;
    if (index < btns.size())
        return btns.at(index);
    return nullptr;
}

void ToolButtonProvider::raiseButtonsChanged()
{
    ToolButtonProvider* in = inHandle;
    inHandle = nullptr;
    emit buttonsChanged();
    inHandle = in;
}

void ToolButtonProvider::getToolButtons(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents)
{
    if (subProviderBefore_)
        subProviderBefore_->getToolButtons(buttons, parents);
    ToolButton *parent = parents.empty() ? nullptr : parents.last();
    getToolButtons(buttons, parent);
    if (subProviderAfter_)
        subProviderAfter_->getToolButtons(buttons, parents);
}

void ToolButtonProvider::getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent)
{
    if (!buttons.empty())
        buttons.append(&ToolButton::SPLITTER);
    buttons.append(tools(parent));
    if (buttons.endsWith(&ToolButton::SPLITTER))
        buttons.pop_back();
}

bool ToolButtonProvider::handleToolButton(QList<ToolButton *> const & buttons)
{
    if (subProviderBefore_ && subProviderBefore_->handleToolButton(buttons))
        return true;
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
    inHandle = this;
    bool result = handleToolButton(button, args);
    if (inHandle != this) // this maybe deleted
        return result;
    if (result && button->needUpdate()) {
        updateToolButton(button);
        if (button->unionUpdate()) {
            QList<ToolButton *> siblins;
            getToolButtons(siblins, i > 0 ? buttons.at(i - 1) : nullptr);
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
    inHandle = nullptr;
    if (!result && subProviderAfter_)
        result = subProviderAfter_->handleToolButton(buttons);
    return result;
}

bool ToolButtonProvider::handleToolButton(ToolButton *button, QStringList const & args)
{
    for (QList<ToolButton *> & btns : buttons_) {
        if (btns.contains(button)) {
            return exec(button->name(), args);
        }
    }
    return false;
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
        QMetaObject const * meta = metaObject();
        QString tools = property("toolsString").toString();
        while (meta != &QObject::staticMetaObject) {
            int i = meta->indexOfClassInfo("toolsString");
            if (i >= 0) {
                if (!tools.isEmpty()) {
                    tools.append("|;");
                }
                tools += metaObject()->classInfo(i).value();
                while (i < meta->classInfoOffset()) {
                    meta = meta->superClass();
                }
                meta = meta->superClass();
            } else {
                break;
            }
        }
        return tools;
    }
    return nullptr;
}

bool ToolButtonProvider::setOption(QByteArray const & key, QVariant value)
{
    int i = metaObject()->indexOfProperty(key);
    if (i < 0)
        return false;
    // fix convert
    QMetaProperty p = metaObject()->property(i);
    if (!value.canConvert(p.type()))
        value.convert(QMetaType::QString);
    LifeObject::setProperty(key, value);
    return true;
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
        if (inHandle == this)
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
            if (inHandle != this)
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
