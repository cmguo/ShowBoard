#include "resource.h"

Resource::Resource(QString const & type, QUrl const & url)
    : url_(url)
    , type_(type)
{
}

Resource::Resource(Resource const & o)
    : url_(o.url_)
    , type_(o.type_)
    , size_(o.size_)
{
}
