#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QQuickWidget>
#include <QWebEngineView>

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    static void sinit();

    static void sinit2();

public:
    WebView(QObject * settings);

    qreal scale() const { return scale_; }

    QPointF pos() const { return pos_; }

    void scroll(QPointF const & delta);

    void scrollTo(QPointF const & pos);

    void scale(qreal scale);

    void scaleTo(qreal scaleTo);

    void synthesizedMouseEvents();

public slots:
    void debug();

    void capture();

    void showHide();

    void scaleUp();

    void scaleDown();

signals:
    void scaleChanged(qreal);

protected:
    virtual bool event(QEvent * event) override;

    virtual bool eventFilter(QObject * watched, QEvent * event) override;

    virtual QPaintEngine * paintEngine() const override;

    virtual QWebEngineView * createWindow(QWebEnginePage::WebWindowType type) override;

private:
    void updateScale();

    void sendTouchEvent(QEvent::Type type, QList<QTouchEvent::TouchPoint> & points,
                        int id, Qt::TouchPointState state, QPointF const & pos = {});

    QQuickWidget *hostWidget();

private:
    QSharedPointer<WebView> life_;
    QQuickWidget* hostWidget_ = nullptr;
    bool synthesizedMouse_ = false;
    QPointF pos_;
    qreal scale_ = 1.0;
};

#endif // WEBVIEW_H
