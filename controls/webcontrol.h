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
    Q_PROPERTY(WebControl::ObjectHash webBridges WRITE setWebBridges)
    Q_PROPERTY(WebControl::close_control_type closeControlType READ closeControlTypeVal WRITE setCloseControlType)

public:
    typedef QHash<QString,QObject*> ObjectHash;
    using close_control_type = std::function<bool(ToolButton *, const QStringList &)>;

    Q_INVOKABLE WebControl(ResourceView *res);

public:
    bool fitToContent() const;

    void setFitToContent(bool b);

    QColor background() const;

    void setBackground(QColor const & color);

    void setWebBridges(ObjectHash const& bridges);

    void setCloseControlType(close_control_type val) {
        close_control_type_ = val;
    }

    close_control_type closeControlTypeVal() {
        return close_control_type_;
    }

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void loadSettings() override;

    virtual void attached() override;
    virtual void detached() override;
    virtual bool handleToolButton(ToolButton * button, QStringList const & args) override;

public slots:
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
    int webViewSizeChangeIndex_ = 0;
    close_control_type close_control_type_ {nullptr};
};

Q_DECLARE_METATYPE(WebControl::ObjectHash)
Q_DECLARE_METATYPE(WebControl::close_control_type)

#endif // WEBCONTROL_H
