#ifndef PPTXCONTROL_H
#define PPTXCONTROL_H

#include "control.h"

class ResourceView;
class QAxObject;

class PptxControl : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE PptxControl(ResourceView * res);

    virtual ~PptxControl() override;

public:
    intptr_t hwnd() const;

public slots:
    void show();
    void next();
    void prev();
    void exit();

private slots:
    void onPropertyChanged(const QString &name);

    void onSignal(const QString &name, int argc, void *argv);

    void onException(int code, const QString &source, const QString &desc, const QString &help);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

private:
    static QAxObject * application_;
    QAxObject * presentation_;
    QAxObject * view_;
};

#endif // PPTXCONTROL_H
