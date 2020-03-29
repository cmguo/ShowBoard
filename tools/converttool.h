#ifndef CONVERTTOOL_H
#define CONVERTTOOL_H

#include "core/control.h"

#include <QUrl>
#include <QVariantMap>

class ConvertTool : public Control
{
    Q_OBJECT

public:
    Q_INVOKABLE ConvertTool(ResourceView* res);

protected:
    virtual QGraphicsItem * create(ResourceView *res) override;

    virtual void attached() override;

private:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    void convert(QUrl const & url);

private slots:
    void onImage(const QString& path, int nPage, int total);

    void onFinished();

    void onFailed(QString const & error);

private:
    int startPage_;
    QVariantMap settings_;
};

#endif // CONVERTTOOL_H
