#ifndef INPUTTEXTCONTROL_H
#define INPUTTEXTCONTROL_H

#include "core/control.h"

#include <QGraphicsItem>


class QGraphicsTextItem;

class DiagramTextItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 3 };

    DiagramTextItem(QGraphicsItem *parent = nullptr);

    int type() const override { return Type; }

signals:
    void lostFocus(DiagramTextItem *item);
    void selectedChange(QGraphicsItem *item);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

class InputTextControl : public Control
{
    Q_OBJECT
    Q_PROPERTY(QString content MEMBER content_)
    Q_PROPERTY(QColor color WRITE setColor READ getColor)

public:
    Q_INVOKABLE InputTextControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

    QColor getColor();
    void setColor(QColor color);

public slots:
    void test();

protected:
    virtual ControlView * create(ControlView * parent) override;
    virtual void attached() override;
    virtual void copy(QMimeData &data) override;
    virtual QString toolsString(QByteArray const & parent) const override;

private:
    QString content_;
};


#endif // INPUTTEXTCONTROL_H
