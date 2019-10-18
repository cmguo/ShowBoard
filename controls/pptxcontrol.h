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

public:
    intptr_t hwnd() const;

public slots:
    void show();
    void next();
    void prev();
    void exit();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

private:
    static QAxObject * application_;
    QAxObject * presentation_;
    QAxObject * window_;
    QAxObject * view_;
};

#endif // PPTXCONTROL_H
