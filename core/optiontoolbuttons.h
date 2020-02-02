#ifndef OPTIONTOOLBUTTONS_H
#define OPTIONTOOLBUTTONS_H

#include "ShowBoard_global.h"

#include <QVariant>
#include <QList>

class ToolButton;

class SHOWBOARD_EXPORT OptionToolButtons
{
public:
    OptionToolButtons(QVariantList const & values, int column);

    template<typename T>
    OptionToolButtons(QList<T> const & values, int column)
        : OptionToolButtons(toVarList(values), column)
    {
    }

    virtual ~OptionToolButtons();

public:
    virtual void fill(QList<ToolButton *> & buttons, QVariant const & value);

    QList<ToolButton*> buttons(QVariant const & value);

    virtual void update(ToolButton * button, QVariant const & value);

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
    StateWidthToolButtons(QList<qreal> const & widths);

protected:
    virtual QVariant buttonIcon(const QVariant &value) override;

private:
    static QGraphicsItem* widthIcon(qreal width, bool selected);
};

#endif // OPTIONTOOLBUTTONS_H
