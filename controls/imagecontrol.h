#ifndef IMAGECONTROL_H
#define IMAGECONTROL_H

#include "core/control.h"

class QPixmap;
class ImageData;

class ImageControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(qreal mipmap MEMBER mipmap_)

public:
    Q_INVOKABLE ImageControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attached() override;

protected:
    void setPixmap(QPixmap const & pixmap);

    friend class ImageData;
    void setMipMapPixmap(QPixmap const & pixmap, QSizeF const & sizeHint);

private:
    void adjustMipmap();

private:
    qreal mipmap_;
    QSharedPointer<ImageData> data_;
};

#endif // IMAGECONTROL_H
