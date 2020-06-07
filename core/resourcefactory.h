#ifndef RESOURCEFACTORY_H
#define RESOURCEFACTORY_H

#include "resourceview.h"

#include <qlazy.h>
#include <qimport.h>

#include <QObject>
#include <QMap>

/*
 * A Factory of a set of ResourceView sub types
 *  act similarly to ResourceManager, but a custom resource type is use
 *  override this class to provider a custom resource type
 */

class SHOWBOARD_EXPORT ResourceFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(std::vector<QLazy> resource_types MEMBER resource_types_)

public:
    explicit ResourceFactory(QObject *parent = nullptr);

public:
    /*
     * child class should override this
     *  implements may call create(res, type)
     */
    virtual ResourceView * create(Resource * res) = 0;

    virtual QUrl newUrl(QByteArray const & type) const = 0;

public slots:
    void onComposition();

public:
    QList<QByteArray> resourceTypes() const;

protected:
    /*
     * create resource of type @type under (group by) this factory
     */
    ResourceView * create(Resource * res, QByteArray const & type);

private:
    std::vector<QLazy> resource_types_;
    QMap<QByteArray, QLazy *> resources_;
};

/*
 * register resource factory
 *  @ctype is base class name for resource classes in factory @ftype
 *  @type is a list of type strings seperate with ','
 */
#define REGISTER_RESOURCE_VIEW_FACTORY(ftype, ctype, type) \
    static QExport<ftype, ResourceView> const export_resource_factory_##ftype( \
        QPart::MineTypeAttribute(type), \
        QPart::Attribute(ResourceView::EXPORT_ATTR_FACTORY, "true") \
    ); \
    static QImportMany<ftype, ctype> import_resource_factory_##ftype( \
        "resource_types", QPart::nonshared);

/*
 * register resource class @ctype under factory @ftype
 *  @type is a list of type strings seperate with ','
 */
#define REGISTER_RESOURCE_VIEW_WITH_FACTORY(ftype, ctype, type) \
    static QExport<ctype, ftype> const export_resource_with_factory_##ctype( \
        QPart::MineTypeAttribute(type));

#endif // RESOURCEFACTORY_H
