#include <QString>
#include <QSharedPointer>
#include <QVector>
#include <QUrl>
#include <QFileInfo>
#include "kernel.h"
#include "ilwisdata.h"
#include "abstractfactory.h"
#include "connectorinterface.h"
#include "connectorfactory.h"
#include "mastercatalog.h"
#include "ilwisobjectconnector.h"
#include "catalogconnector.h"
#include "catalogexplorer.h"
#include "domain.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "table.h"
#include "catalog.h"
#include "dataset.h"

using namespace Ilwis;

CatalogConnector::CatalogConnector(const Resource &resource, bool load , const IOOptions &options) : IlwisObjectConnector(resource, load, options)
{

}

bool CatalogConnector::isValid() const
{
    return source().isValid();
}

bool CatalogConnector::canUse(const Resource &resource) const
{
    if (_dataProviders.size() == 0)
        const_cast<CatalogConnector *>(this)->loadExplorers();

   for(const auto& explorer : _dataProviders){
       if (explorer->canUse(resource))
           return true;
   }
   return false;
}

QFileInfo CatalogConnector::toLocalFile(const QUrl &url) const
{
    QString local = url.toLocalFile();
    QFileInfo localFile(local);
    if ( localFile.exists())
        return local;

    int index = local.lastIndexOf("/");
    if ( index == -1){
        return QFileInfo();
    }
    QString parent = local.left(index);
    QString ownSection = local.right(local.size() - index - 1);
    if (Ilwis::Resource::isRoot(parent)){ // we are at the root; '/'has been removed and has to be added again; recursion ends here
        return local; // we return local here as we don't need to reconstruct our path. local is directly attached to the root and thus has sufficient info
    }
    QUrl parentUrl(QUrl::fromLocalFile(parent));
    quint64 id = mastercatalog()->url2id(parentUrl, itCATALOG);
    if ( id == i64UNDEF)
        return localFile;
        //return QFileInfo(parent);
    Resource parentResource = mastercatalog()->id2Resource(id);

    QFileInfo parentPath =  parentResource.toLocalFile();
    if ( parentPath.fileName() == sUNDEF)
        parentPath = parent;
    QFileInfo currentPath(parentPath.absoluteFilePath() + "/"+ ownSection);
    return currentPath;
}
QFileInfo CatalogConnector::toLocalFile(const Resource &resource) const
{
    QFileInfo currentPath =  toLocalFile(resource.url());

    if ( !currentPath.exists()){
        for(const auto& explorer: _dataProviders) {
            if ( explorer->canUse(resource)){
                return explorer->resolve2Local();
            }
        }
    }
    return currentPath;
}

QString CatalogConnector::provider() const
{
    return "ilwis";
}

ConnectorInterface *CatalogConnector::create(const Ilwis::Resource &resource, bool load, const IOOptions& options)
{
    return new CatalogConnector(resource, load, options);
}

bool CatalogConnector::loadMetaData(IlwisObject *data,const IOOptions &)
{
    return loadExplorers();
}

bool CatalogConnector::loadData(IlwisObject *obj, const IOOptions &options){
    Catalog *cat = static_cast<Catalog *>(obj);
    kernel()->issues()->log(QString(TR("Scanning %1")).arg(source().url(true).toString()),IssueObject::itMessage);
    for(const auto& explorer : _dataProviders){

        // TODO clear security issues which may arise here, as
        // all _dataProviders gets passed the ioptions probably
        // not indendet for them

        IOOptions iooptions = options.isEmpty() ? ioOptions() : options;
        std::vector<Resource> items = explorer->loadItems(iooptions);
        cat->addItemsPrivate(items);
    }
    return true;
}

Ilwis::IlwisObject *CatalogConnector::create() const
{
    if ( source().hasProperty("domain"))
        return new DataSet(source());
    return new Catalog(source());
}

bool CatalogConnector::loadExplorers()
{
    if ( _dataProviders.size() > 0) // already done
        return true;
    const Ilwis::ConnectorFactory *factory = kernel()->factory<Ilwis::ConnectorFactory>("ilwis::ConnectorFactory");

    std::vector<CatalogExplorer*> explorers = factory->explorersForResource(source());
    if ( explorers.size() == 0) {
        return ERROR2(ERR_COULDNT_CREATE_OBJECT_FOR_2,"Catalog connector", source().toLocalFile());
    }
    _dataProviders.resize(explorers.size());
    int  i =0;
    for(CatalogExplorer *explorer : explorers) {
        _dataProviders[i++].reset(explorer);
    }


    return true;
}


