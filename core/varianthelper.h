#ifndef VARIANTHELPER_H
#define VARIANTHELPER_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QVariant>

class SHOWBOARD_EXPORT VariantHelper : public QObject
{
    Q_OBJECT
public:
    template <typename T>
    static T convert(QVariant v)
    {
        return convert(v, qMetaTypeId<T>()).value<T>();
    }

    static QVariant convert(QVariant v, int type);

    static bool convert2(QVariant & v, int type);
};

#endif // VARIANTHELPER_H
