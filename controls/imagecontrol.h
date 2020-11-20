#ifndef IMAGECONTROL_H
#define IMAGECONTROL_H

#include "core/control.h"

class ImageData;

class QGraphicsPixmapItem;
class QPixmap;

class ImageControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(qreal borderSize READ borderSize WRITE setBorderSize)
    Q_PROPERTY(qreal mipmap READ mipmap WRITE setMipmap)

public:
    Q_INVOKABLE ImageControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

public:
    qreal borderSize() const { return borderSize_; }

    void setBorderSize(qreal borderSize);

    qreal mipmap() const { return mipmap_; }

    void setMipmap(qreal mipmap);

    qreal mipScale() const { return mipScale_; }

protected:
    virtual ControlView * create(ControlView * parent) override;

    virtual void attached() override;

    virtual void copy(QMimeData &data) override;

protected:
    QGraphicsPixmapItem * image() const { return image_; }

    void setPixmap(QPixmap const & pixmap);

    QPixmap pixmap() const;

    friend class ImageData;
    void setMipMapPixmap(QPixmap const & pixmap, QSizeF const & sizeHint);

private:
    void adjustMipmap();

    void adjustMipmap2(QSizeF const & sizeHint);

private:
    QGraphicsPixmapItem * image_;
    qreal borderSize_;
    qreal mipmap_;
    qreal mipScale_;
    QSizeF initImageSize_;
    QSharedPointer<ImageData> data_;
};

#endif // IMAGECONTROL_H
