#include "imagehelper.h"
#include "toolbutton.h"

#include <QIcon>
#include <QWidget>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

ToolButton ToolButton::SPLITTER("splitter", Static);
ToolButton ToolButton::LINE_BREAK("lineBreak", Static);
ToolButton ToolButton::LINE_SPLITTER("lineSplitter", Static);
ToolButton ToolButton::PLACE_HOOLDER("placeHolder", Static);

ToolButton::ToolButton()
{
}

ToolButton::ToolButton(const ToolButton &o)
    : name_(o.name_)
    , flags_(o.flags_)
    , icon_(o.icon_)
{
    setText(o.text());
    setCheckable(o.isCheckable());
    //QAction::setIcon(o.icon());
}

ToolButton::ToolButton(const QByteArray &name, const QString &title,
                       Flags flags, const QVariant &icon)
    : name_(name)
    , flags_(flags)
    , icon_(icon)
{
    setText(title);
    setCheckable(flags & Checkable);
}

ToolButton::ToolButton(const QByteArray &name, const QString &title,
                       const QByteArray &flags, const QVariant &icon)
    : name_(name)
    , icon_(icon)
{
    setName(name);
    setText(title);
    parseFlags(flags);
    //if (!isCustomWidget())
    //    QAction::setIcon(getIcon());
}

QRectF ToolButton::itemRect() const
{
    return property("ToolButtonItemRect").toRectF();
}

void ToolButton::setItemRect(const QRectF &rect)
{
    setProperty("ToolButtonItemRect", rect);
}

QIcon ToolButton::getIcon(QSize const & size)
{
    return makeIcon(icon_, size, !isDynamic());
}

void ToolButton::setIcon(const QVariant &icon)
{
    icon_ = icon;
    //QAction::setIcon(getIcon());
}

QString ToolButton::iconSource()
{
    if (icon_.type() != QVariant::String
            && icon_.type() != QVariant::ByteArray)
        return nullptr;
    QStringList seps = icon_.toString().split(",", QString::SkipEmptyParts);
    if (seps.empty())
        return nullptr;
    if (seps.size() == 1)
        return "qrc" + seps[0];
    QString file;
    QString state = isEnabled() ? (isChecked() ? "+normal" : "normal") : "disabled";
    state.append('=');
    for (QString const & sep : seps) {
        if (sep.startsWith(state) && !sep.endsWith("%")) {
            int n1 = file.lastIndexOf('.');
            file.replace(n1, 0, sep.mid(state.size()));
        } else if (sep.indexOf('=') < 0) {
            file = sep;
        }
    }
    return "qrc" + file;
}

QWidget *ToolButton::getCustomWidget()
{
    return icon_.value<QWidget*>();
}

ToolButton::ToolButton(const QByteArray &name, Flags flags)
    : name_(name)
    , flags_(flags)
{
}

void ToolButton::parseFlags(const QByteArray &flags)
{
    QList<QByteArray> tokens = flags.split(',');
    for (QByteArray & t : tokens) {
        if (t.isEmpty()) continue;
        if (QChar(t[0]).isUpper())
            t[0] = QChar(t[0]).toLower().toLatin1();
        setProperty(t, true);
    }
}

ToolButton * ToolButton::makeButton(const QString &desc)
{
    if (desc == "|")
        return &SPLITTER;
    if (desc == "")
        return &PLACE_HOOLDER;
    if (desc == "-")
        return &LINE_SPLITTER;
    if (desc == "\n")
        return &LINE_BREAK;
    if (desc.startsWith("-"))
        return nullptr;
    QStringList seps = desc.split("|");
    if (seps.size() >= 1) {
        return new ToolButton(
            seps[0].toUtf8(),
            seps.size() > 1 ? seps[1] : seps[0],
            seps.size() > 3 ? seps[2].toUtf8() : nullptr,
            seps.size() > 2 ? QVariant(seps.back()) : QVariant()
        );
    }
    return nullptr;
}

QList<ToolButton *> ToolButton::makeButtons(QString const & tools)
{
    QList<ToolButton *> list;
    QStringList descs = tools.split(";", QString::SkipEmptyParts);
    for (QString desc : descs) {
        ToolButton * btn = makeButton(desc);
        if (btn) {
            list.append(btn);
        }
    }
    return list;
}

static QPixmap getPixmap(QVariant value, QSize const & size, bool destroy)
{
    if (value.type() == QVariant::Pixmap)
        return value.value<QPixmap>();
    else if (value.type() == QVariant::Image)
        return QPixmap::fromImage(value.value<QImage>());
    else if (value.type() == QVariant::UserType) {
        if (value.userType() == QMetaType::QObjectStar) {
            return ImageHelper::widgetToPixmap(value.value<QWidget *>(), destroy);
        } else if (value.userType() == qMetaTypeId<QGraphicsItem *>()) {
            return ImageHelper::itemToPixmap(value.value<QGraphicsItem *>(), size, destroy);
        }
    }
    return QPixmap();
}

static QMap<QString, QIcon::Mode> IconModes = {
    {"normal", QIcon::Normal},
    {"disabled", QIcon::Disabled},
    {"active", QIcon::Active}, // pressed
    {"selected", QIcon::Selected},
};

QIcon ToolButton::makeIcon(QString const & iconString, QSize const & size)
{
    QStringList seps = iconString.split(",", QString::SkipEmptyParts);
    if (seps.empty())
        return QIcon();
    if (seps.size() == 1)
        return QIcon(seps[0]);
    QString file;
    QPixmap pixmap;
    QIcon icon;
    for (QString const & sep : seps) {
        int n = sep.indexOf('=');
        if (n < 0) {
            if (sep == "default") {
                // normal=,disabled=30%,+normal=
                icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
                icon.addPixmap(ImageHelper::opacityPixmap(pixmap, 30), QIcon::Disabled, QIcon::Off);
                icon.addPixmap(pixmap, QIcon::Normal, QIcon::On);
                continue;
            }
            file = sep;
            pixmap = QIcon(file).pixmap(size);
        } else {
            QString m = sep.left(n);
            QIcon::State s = QIcon::Off;
            if (m.startsWith("+")) {
                s = QIcon::On;
                m = m.mid(1);
            }
            QString v = sep.mid(n + 1);
            QPixmap p;
            if (v.endsWith("%")) {
                int alpha = v.left(v.length() - 1).toInt();
                p = ImageHelper::opacityPixmap(pixmap, alpha);
            } else if (!v.isEmpty()) {
                int n1 = file.lastIndexOf('.');
                if (v.startsWith('/')) {
                    int n2 = file.lastIndexOf('/');
                    file.replace(n2, n1 - n2, v);
                } else {
                    file.replace(n1, 0, v);
                }
                pixmap = QIcon(file).pixmap(size);
                p = pixmap;
            } else {
                p = pixmap;
            }
            if (IconModes.contains(m))
                icon.addPixmap(p, IconModes[m], s);
        }
    }
    return icon;
}

QIcon ToolButton::makeIcon(QVariant& icon, QSize const & size, bool replace)
{
    QIcon result;
    if (icon.type() == QVariant::Icon)
        return icon.value<QIcon>();
    else if (icon.type() == QVariant::String
             || icon.type() == QVariant::ByteArray)
        result = makeIcon(icon.toString(), size);
    else if (icon.type() == QVariant::Map) {
        QVariantMap icons = icon.toMap();
        for (QString k : icons.keys()) {
            QVariant& icon2 = icons[k];
            QIcon::State s = QIcon::Off;
            if (k.startsWith("+")) {
                s = QIcon::On;
                k = k.mid(1);
            }
            if (IconModes.contains(k)) {
                result.addPixmap(getPixmap(icon2, size, replace), IconModes[k], s);
            }
        }
    } else {
        result = QIcon(getPixmap(icon, size, replace));
    }
    if (replace)
        icon = result;
    return result;
}

