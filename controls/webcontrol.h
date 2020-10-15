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
    static constexpr Flag FitToContent = CustomFlag;
    static constexpr Flag Debugable = static_cast<Flag>(CustomFlag + 1);

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

public slots:
    void fitContent();

    void full();

protected:
    virtual QWidget * createWidget(ControlView * parent) override;

    virtual void loadSettings() override;

    virtual void attached() override;

    virtual void detached() override;

    virtual void sizeChanged() override;

    virtual bool setOption(QByteArray const & key, QVariant value) override;

protected:
    virtual void loadFinished(bool ok);

private:
    void contentsSizeChanged(const QSizeF &size);

    void scrollPositionChanged(const QPointF &pos);

    void scaleChanged(qreal scale);

    void canvasTransformChanged(int elem);

private:
    QColor background_;
};

#endif // WEBCONTROL_H
