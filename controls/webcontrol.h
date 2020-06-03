#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "widgetcontrol.h"

#include <QColor>
#include <QHash>

class WebControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(bool fitToContent READ fitToContent WRITE setFitToContent)
    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(WebControl::ObjectHash webBridges READ webBridges WRITE setWebBridges)

public:
    typedef QHash<QString,QObject*> ObjectHash;

    Q_INVOKABLE WebControl(ResourceView *res);

public:
    bool fitToContent() const;

    void setFitToContent(bool b);

    QColor background() const;

    void setBackground(QColor const & color);

    ObjectHash webBridges() const;

    void setWebBridges(ObjectHash const& bridges);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void loadSettings() override;

    virtual void attached() override;

private slots:
    void loadFinished(bool ok);

    void contentsSizeChanged(const QSizeF &size);

    void scrollPositionChanged(const QPointF &pos);

    void reload();

    void hide();

    void full();

    void fitContent();

    void debug();

    void dump();

private:
    bool fitToContent_;
    bool hasBackground_;
    QColor background_;
};

Q_DECLARE_METATYPE(WebControl::ObjectHash)

#endif // WEBCONTROL_H
