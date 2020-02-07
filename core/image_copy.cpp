#include "image.h"
#include "core/workthread.h"

#include <QPixmap>

class ImageData : QObject
{
public:
    QPixmap image;
    QList<QPixmap> mipmaps;
};

Image::Image(Resource *res)
    : ResourceView(res)
    , mipmap_(0)
{
}

Image::Image(const Image &o)
    : ResourceView(o)
    , mipmap_(o.mipmap_)
{
}
