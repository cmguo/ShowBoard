#include "toolbutton.h"

#include <QIcon>
#include <QWidget>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

ToolButton ToolButton::SPLITTER({"splitter", nullptr, Static, QVariant()});
ToolButton ToolButton::LINE_BREAK({"lineBreak", nullptr, Static, QVariant()});
ToolButton ToolButton::LINE_SPLITTER({"lineSplitter", nullptr, Static, QVariant()});
ToolButton ToolButton::PLACE_HOOLDER({"placeHolder", nullptr, Static, QVariant()});

QIcon ToolButton::getIcon()
{
    return getIcon(icon, !(flags & ToolButton::Dynamic));
}

ToolButton::Flags ToolButton::makeFlags(const QString &str)
{
   QStringList tokens = str.split(",", QString::SkipEmptyParts);
   Flags flags;
   for (QString const & t : tokens) {
       flags |= QVariant(t).value<Flag>();
   }
   return flags;
}

ToolButton * ToolButton::makeButton(const QString &desc)
{
    if (desc == "|")
        return &SPLITTER;
    if (desc == "")
        return &PLACE_HOOLDER;
    if (desc == "-")
        return &LINE_SPLITTER;
    if (desc.startsWith("-"))
        return nullptr;
    QStringList seps = desc.split("|");
    if (seps.size() >= 1) {
        return new ToolButton{
            seps[0].toUtf8(),
            seps.size() > 1 ? seps[1] : seps[0],
            seps.size() > 3 ? ToolButton::makeFlags(seps[2]) : nullptr,
            seps.size() > 2 ? QVariant(seps.back()) : QVariant()
        };
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

static QPixmap widgetToPixmap(QWidget * widget, bool destroy)
{
    QPixmap pm(widget->size());
    pm.fill(Qt::transparent);
    QPainter pt(&pm);
    pt.setRenderHint(QPainter::HighQualityAntialiasing);
    widget->render(&pt);
    pt.end();
    if (destroy)
        widget->deleteLater();
    return pm;
}

static QPixmap itemToPixmap(QGraphicsItem * item, bool destroy)
{
    QRectF rect = item->boundingRect();
    QPoint size = (rect.center() * 2).toPoint();
    QPixmap pm(size.x(), size.y());
    pm.fill(Qt::transparent);
    QPainter pt(&pm);
    pt.setRenderHint(QPainter::HighQualityAntialiasing);
    QStyleOptionGraphicsItem style;
    item->paint(&pt, &style);
    for (QGraphicsItem * c : item->childItems()) {
        c->paint(&pt, &style);
    }
    pt.end();
    if (destroy)
        delete item;
    return pm;
}

static QPixmap opacityPixmap(QPixmap pixmap, int opacity)
{
    QPixmap pm = QPixmap(pixmap.size());
    pm.fill(Qt::transparent);
    QPainter pt(&pm);
    pt.setOpacity(opacity / 100.0);
    QRect rc(0, 0, pm.width(), pm.height());
    pt.drawPixmap(rc, pixmap, rc);
    pt.end();
    return pm;
}

static QPixmap getPixmap(QVariant value, bool destroy)
{
    if (value.type() == QVariant::Pixmap)
        return value.value<QPixmap>();
    else if (value.type() == QVariant::Image)
        return QPixmap::fromImage(value.value<QImage>());
    else if (value.type() == QVariant::UserType) {
        if (value.userType() == QMetaType::QObjectStar) {
            return widgetToPixmap(value.value<QWidget *>(), destroy);
        } else if (value.userType() == qMetaTypeId<QGraphicsItem *>()) {
            return itemToPixmap(value.value<QGraphicsItem *>(), destroy);
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

QIcon ToolButton::makeIcon(QString const & iconString)
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
                // normal=70%,disabled=30%,active=
                icon.addPixmap(opacityPixmap(pixmap, 70), QIcon::Normal, QIcon::Off);
                icon.addPixmap(opacityPixmap(pixmap, 30), QIcon::Disabled, QIcon::Off);
                icon.addPixmap(pixmap, QIcon::Active, QIcon::Off);
                continue;
            }
            file = sep;
            pixmap.load(file);
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
                p = opacityPixmap(pixmap, alpha);
            } else if (!v.isEmpty()) {
                int n1 = file.lastIndexOf('.');
                file.replace(n1, 0, v);
                pixmap.load(file);
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

QIcon ToolButton::getIcon(QVariant& icon, bool replace)
{
    QIcon result;
    if (icon.type() == QVariant::Icon)
        return icon.value<QIcon>();
    else if (icon.type() == QVariant::String)
        result = makeIcon(icon.toString());
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
                result.addPixmap(getPixmap(icon2, replace), IconModes[k], s);
            }
        }
    } else {
        result = QIcon(getPixmap(icon, replace));
    }
    if (replace)
        icon = result;
    return result;
}

