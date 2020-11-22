#ifndef WORDSCONTROL_H
#define WORDSCONTROL_H

#include "core/control.h"

class QPainterPath;

class WordsControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(int fontWeight READ fontWeight WRITE setFontWeight)
public:
    Q_INVOKABLE WordsControl(ResourceView *res);

public slots:
    void next();

public:
    int fontWeight() const;

    void setFontWeight(int weight);

protected:
    virtual ControlView *create(ControlView *parent) override;
};

#endif // WORDSCONTROL_H
