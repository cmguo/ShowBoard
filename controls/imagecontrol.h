#ifndef IMAGECONTROL_H
#define IMAGECONTROL_H

#include "core/control.h"

class ImageData;

class QGraphicsPixmapItem;
class QPixmap;

class ImageControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(qreal mipmap READ mipmap WRITE setMipmap)

public:
    Q_INVOKABLE ImageControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

public:
    qreal mipmap() const { return mipmap_; }

    void setMipmap(qreal mipmap);

    qreal mipScale() const { return mipScale_; }

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attached() override;

    virtual void copy(QMimeData &data) override;

protected:
    void setPixmap(QPixmap const & pixmap);

    QPixmap pixmap() const;

    friend class ImageData;
    void setMipMapPixmap(QPixmap const & pixmap, QSizeF const & sizeHint);

private:
    void adjustMipmap();

    void adjustMipmap2(QSizeF const & sizeHint);

private:
    QGraphicsPixmapItem * image_;
    qreal mipmap_;
    qreal mipScale_;
    QSharedPointer<ImageData> data_;
};

#endif // IMAGECONTROL_H
