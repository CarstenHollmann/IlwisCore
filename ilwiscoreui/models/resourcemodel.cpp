#include <QString>
#include <QFileInfo>
#include <QUrl>
#include "kernel.h"
#include "connectorinterface.h"
#include "resource.h"
#include "ilwisdata.h"
#include "angle.h"
#include "geometries.h"
#include "ellipsoid.h"
#include "projection.h"
#include "proj4parameters.h"
#include "ilwisobject.h"
#include "mastercatalog.h"
#include "resourcemodel.h"

#define tempHardPath "h:/projects/Ilwis4/projects/client/qml/desktop/mobile/images/"
//#define tempHardPath "d:/projects/ilwis/Ilwis4/projects/client/qml/desktop/mobile/images/"

using namespace Ilwis;
//using namespace Desktopclient;

QString ResourceModel::getProperty(const QString &propertyname)
{
    if(_item.hasProperty(propertyname))
        return _item[propertyname].toString();
    return sUNDEF;
}

ResourceModel::ResourceModel()
{

}

ResourceModel::ResourceModel(const Ilwis::Resource& source, QObject *parent) :
    QObject(parent), _imagePath(sUNDEF),_type(itUNKNOWN), _isRoot(false)
{
    resource(source);
}

ResourceModel::ResourceModel(const ResourceModel &model) : QObject(model.parent())
{
    _displayName = model._displayName;

    _item = model._item;
    _imagePath = model._imagePath;
    _type = model._type;
    _isRoot = model._isRoot;
}

ResourceModel &ResourceModel::operator=(const ResourceModel &model)
{
    _displayName = model._displayName;

    _item = model._item;
    _imagePath = model._imagePath;
    _type = model._type;
    _isRoot = model._isRoot;

    return *this;
}

ResourceModel::~ResourceModel()
{

}

QString ResourceModel::imagePath() const
{
    return _imagePath;
}

quint64 ResourceModel::type() const
{
    return _type;
}

QString ResourceModel::typeName() const
{
    return TypeHelper::type2name(_type);
}

QString ResourceModel::name() const
{
    if ( _item.isValid()) {
        return _item.name();
    }
    return "";
}

QString ResourceModel::size() const
{
    if ( _item.isValid() && _item.ilwisType() != itCATALOG){
        quint64 sz = _item.size();
        if ( sz != 0)
            return  QString::number(sz);
    }
    return "";
}

QString ResourceModel::description() const
{
    if ( _item.isValid())
        return _item.description();
    return "";
}

QString ResourceModel::dimensions() const
{
    if ( _item.isValid())
        return _item.dimensions();
    return "";
}

QString ResourceModel::displayName() const
{
    return _displayName;
}

void ResourceModel::setDisplayName(const QString &name)
{
    _displayName = name;
}

QString ResourceModel::url() const
{
    return _item.url().toString();
}

QString ResourceModel::iconPath() const
{
    quint64 tp = _item.ilwisType();
    return iconPath(tp);
}

QString ResourceModel::iconPath(IlwisTypes tp)
{
    if ( tp & itRASTER)
        return "raster20CS1.png";
    else if ( tp == itPOLYGON)
        return "polygon20CS1.png";
    else if ( tp == itLINE)
        return "line20.png";
    else if ( tp == itPOINT)
        return "point20.png";
    else if ( hasType(tp, itFEATURE))
        return "feature20CS1.png";
    else if ( tp & itTABLE)
        return "table20CS1.png";
    else if ( tp & itCOORDSYSTEM)
        return "csy20.png";
    else if ( tp & itGEOREF)
        return "georeference20.png";
    else if ( tp == itCATALOG)
        return "folder.png";
    else if ( tp & itDOMAIN)
        return "domain.png";
    else if ( tp & itREPRESENTATION)
        return "representation20.png";
    else if ( hasType(tp,itNUMBER))
        return "numbers20.png";
    else if ( hasType(tp,itPROJECTION))
        return "projection20.png";
    else if ( hasType(tp,itELLIPSOID))
        return "ellipsoid20.png";
    else if ( tp & itSTRING)
        return "text20.png";
    else
        return "eye.png";
}

bool ResourceModel::isRoot() const
{
    return _isRoot;
}

QString ResourceModel::id() const
{
    if ( _item.isValid())
        return QString::number(_item.id());
    return sUNDEF;
}

Resource ResourceModel::item() const
{
    return _item;
}


QString ResourceModel::domainName() const {
    QString nme =  propertyName("domain");
    if ( nme != displayName() && nme != "")
        return nme;
    quint64 tp = _item.ilwisType();
    if ( hasType(tp, itCOVERAGE))
        return "self";
    return "";
}

QString ResourceModel::domainType() const {
    QString nme = propertyTypeName(itDOMAIN, "domain");
    if ( nme != "")
        return nme;
    quint64 tp = _item.ilwisType();
    if ( hasType(tp, itCOVERAGE))
        return "IndexedIdentifier";
    return "";
}

QString ResourceModel::proj42DisplayName(const QString& proj4Def) const{
    Proj4Parameters parms(proj4Def);
    QString projName = Projection::projectionCode2Name(parms["proj"]);
    QString ellipse = Ellipsoid::ellipsoidCode2Name(parms["proj"]);
    if ( ellipse == sUNDEF)
        return sUNDEF;
    return projName + "/" + ellipse;
}

QString ResourceModel::coordinateSystemName() const {
    QString nme =  propertyName("coordinatesystem");
    if ( nme != displayName() && nme != "" && nme != sUNDEF)
        return nme;
    if ( nme == ""){
        nme = _item["coordinatesystem"].toString();
        if ( nme != ""){
            int index = nme.toLower().indexOf("code=");
            if ( index == -1){
                nme = _item.code();
            }
            if ((index = nme.toLower().indexOf("code=epsg")) != -1){
                nme = nme.mid(5);
            }
            else if ( (index = nme.toLower().indexOf("code=proj4")) != -1){
                nme = proj42DisplayName(nme.mid(5));
            }else {
                nme = Projection::projectionCode2Name(nme.mid(5));
            }
        }
    }
    return nme != sUNDEF ? nme : "";
}

QString ResourceModel::coordinateSystemType() const {
    QString txt = propertyTypeName(itCOORDSYSTEM, "coordinatesystem");
    return txt.left(txt.size() - QString("CoordinateSystem").size());
}

QString ResourceModel::geoReferenceName() const
{
    QString nme = propertyName("georeference");
    if ( nme != displayName())
        return nme;
    return "";
}

QString ResourceModel::geoReferenceType() const
{
    return propertyTypeName(itGEOREF, "georeference");
}



void ResourceModel::resource(const Ilwis::Resource& res)
{
    Resource item = res;
    if ( item.name() == sUNDEF) {
        QString name  = res.url().toString(QUrl::RemoveScheme | QUrl::RemoveQuery | QUrl::RemovePassword | QUrl::StripTrailingSlash);
        name = name.mid(3);
        item.name(name, false);
    }

    _type = item.ilwisType();
    _item = item;

    if ( item.url().toString() == "file://"){
        _displayName = "root";
        _isRoot = true;
    }
    else if ( item.url().scheme() == "file") {
        QFileInfo inf(_item.url().toLocalFile());
        QString path = inf.absolutePath();
        _isRoot = inf.isRoot();
        _displayName = item.name();
        QFileInfo thumbPath = path + "/thumbs/" + _displayName + ".png";
        if ( thumbPath.exists()) {
            _imagePath =  "file:///" +  thumbPath.absoluteFilePath();
        } else {
            if ( item.ilwisType() == itCATALOG) {
                _imagePath = "catalog.png";
            }
            if ( hasType(item.ilwisType(), itRASTER))
                 _imagePath = "remote.png";
            else if ( hasType(item.ilwisType(), itFEATURE))
                _imagePath = "polygon.png";
            else if ( hasType(item.ilwisType(), itCOORDSYSTEM))
                _imagePath = "csy.png";
            else if ( hasType(item.ilwisType(), itGEOREF))
                _imagePath = "grf.png";
            else if ( hasType(item.ilwisType(), itTABLE))
                _imagePath = "table.jpg";
            else if ( hasType(item.ilwisType(), itDOMAIN))
                _imagePath = "domainn.png";
            else if ( hasType(item.ilwisType(), itREPRESENTATION))
                _imagePath = "representation20.png";
            else
                _imagePath = "blank.png";
        }
    }else
       _displayName = item.name();
}

Ilwis::Resource ResourceModel::resource() const
{
    return _item;
}

Ilwis::Resource& ResourceModel::resourceRef()
{
    return _item;
}

QString ResourceModel::propertyName( const QString& property) const{
    if ( _item.isValid()) {
        bool ok;
        quint64 iddomain =  _item[property].toLongLong(&ok);
        if ( ok) {
            return Ilwis::mastercatalog()->id2Resource(iddomain).name();
        }
    }
    return "";

}

QString  ResourceModel::propertyTypeName(quint64 typ, const QString& propertyName) const {
    if ( _item.isValid()) {
        if (_item.extendedType() & typ) {
            bool ok;
            quint64 idprop =  _item[propertyName].toLongLong(&ok);
            if ( ok) {
                quint64 tp = Ilwis::mastercatalog()->id2Resource(idprop).ilwisType();
                return Ilwis::IlwisObject::type2Name(tp);
            }
        }
    }
    return "";
}

