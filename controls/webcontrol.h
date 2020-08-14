#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "widgetcontrol.h"

#include <QColor>
#include <QHash>

class SHOWBOARD_EXPORT WebControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(bool fitToContent READ fitToContent WRITE setFitToContent)
    Q_PROPERTY(bool debugable READ debugable WRITE setDebugable)
    Q_PROPERTY(QColor background READ background WRITE setBackground)
#ifdef Q_MOC_RUN // fix QCreator error
    Q_PROPERTY(QHash<QString, QObject*> webBridges WRITE setWebBridges)
#endif

public:
    static constexpr Flag FitToContent = static_cast<Flag>(1 << 16);
    static constexpr Flag Debugable = static_cast<Flag>(1 << 17);

    static void init();

public:
    Q_INVOKABLE WebControl(ResourceView *res);

public:
    bool fitToContent() const;

    void setFitToContent(bool b);

    bool debugable() const;

    void setDebugable(bool b);

    QColor background() const;

    void setBackground(QColor const & color);

    void setWebBridges(QHash<QString, QObject*> const& bridges);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void loadSettings() override;

    virtual void attached() override;

    virtual void detached() override;

    virtual QUrl url() const;

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
    QColor background_;
    int webViewSizeChangeIndex_ = 0;
};

#endif // WEBCONTROL_H
