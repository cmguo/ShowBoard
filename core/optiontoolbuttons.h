#ifndef OPTIONTOOLBUTTONS_H
#define OPTIONTOOLBUTTONS_H

#include "ShowBoard_global.h"

#include <QVariant>
#include <QList>
#include <QColor>

class ToolButton;
class ToolButtonProvider;

class SHOWBOARD_EXPORT OptionToolButtons
{
public:
    OptionToolButtons(QVariantList const & values, int column = 0);

    template<typename T>
    OptionToolButtons(QList<T> const & values, int column = 0)
        : OptionToolButtons(toVarList(values), column)
    {
    }

    virtual ~OptionToolButtons();

public:
    void getButtons(QList<ToolButton *> & getButtons, QVariant const & value);

    QList<ToolButton*> getButtons(QVariant const & value);

    void updateValue(QVariant const & value);

    void updateParent(ToolButton * button, QVariant const & value);

protected:
    template<typename T>
    static QVariantList toVarList(T const & list)
    {
        QVariantList vlist;
        for (auto i : list)
            vlist.append(QVariant::fromValue(i));
        return vlist;
    }

    void makeButtons();

    virtual QByteArray buttonName(QVariant const & value);

    virtual QString buttonTitle(QVariant const & value);

    virtual QVariant buttonIcon(QVariant const & value);

private:
    OptionToolButtons(OptionToolButtons const &);

private:
    QVariantList const values_;
    int column_;
    int lastCheck_;
    QList<ToolButton *> buttons_;
};

class QGraphicsItem;

class SHOWBOARD_EXPORT ColorToolButtons : public OptionToolButtons
{
public:
    ColorToolButtons(QStringList const & colors);

    ColorToolButtons(QList<QColor> const & colors);

protected:
    virtual QVariant buttonIcon(const QVariant &value) override;

private:
    static QGraphicsItem* colorIcon(QColor color);
};

class SHOWBOARD_EXPORT StateColorToolButtons : public OptionToolButtons
{
public:
    StateColorToolButtons(QStringList const & colors);

    StateColorToolButtons(QList<QColor> const & colors);

protected:
    virtual QVariant buttonIcon(const QVariant &value) override;

private:
    static QGraphicsItem* colorIcon(QColor color, bool selected);
};

class SHOWBOARD_EXPORT WidthToolButtons : public OptionToolButtons
{
public:
    WidthToolButtons(QList<qreal> const & widths);

protected:
    virtual QVariant buttonIcon(const QVariant &value) override;

private:
    static QGraphicsItem* widthIcon(qreal width);
};

class SHOWBOARD_EXPORT StateWidthToolButtons : public OptionToolButtons
{
public:
    StateWidthToolButtons(QList<qreal> const & widths, QColor color = Qt::white);

protected:
    virtual QVariant buttonIcon(const QVariant &value) override;

private:
    static QGraphicsItem* widthIcon(qreal width, bool selected, QColor color);

private:
    QColor color_;
};

#endif // OPTIONTOOLBUTTONS_H
