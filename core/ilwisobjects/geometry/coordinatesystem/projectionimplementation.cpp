#include <functional>

#include "kernel.h"
#include "ilwis.h"
#include "geometries.h"
#include "ilwisdata.h"
#include "ellipsoid.h"
#include "geodeticdatum.h"
#include "projection.h"
#include "coordinatesystem.h"
#include "conventionalcoordinatesystem.h"
#include "projectionimplementation.h"
#include "proj4parameters.h"

using namespace Ilwis;

ProjectionImplementation::ProjectionImplementation(const QString &type) :
    _coordinateSystem(0),
    _projtype(type)
{
    _parameters[Projection::Projection::pvX0] = {rUNDEF};
    _parameters[Projection::pvY0] = {0};
    _parameters[Projection::pvK0] = {0};
    _parameters[Projection::pvLAT0] = {0};
    _parameters[Projection::pvLAT1] = {0};
    _parameters[Projection::pvLAT2] = {0};
    _parameters[Projection::pvZONE] = {1};
    _parameters[Projection::pvNORIENTED] =  {true};
    _parameters[Projection::pvAZIMYAXIS] = {0};
    _parameters[Projection::pvTILTED] =  {false};
    _parameters[Projection::pvAZIMCLINE] = {0};
    _parameters[Projection::pvHEIGHT] = {0};
    _parameters[Projection::pvLON0] =  {0};
}

QString ProjectionImplementation::type() const
{
    return _projtype;
}

void ProjectionImplementation::setCoordinateSystem(ConventionalCoordinateSystem *csy)
{
    _coordinateSystem = csy;
}

QVariant ProjectionImplementation::parameter(Projection::ProjectionParamValue type) const
{
    auto iter = _parameters.find(type);
    if ( iter != _parameters.end()) {
        return (*iter).second._value;
    }
    return QVariant();
}

void ProjectionImplementation::setParameter(Projection::ProjectionParamValue type, const QVariant &value)
{
    _parameters[type] = {value, true};
}

bool ProjectionImplementation::isEqual(const QScopedPointer<ProjectionImplementation>& projimpl) {
    QString proj4a = toProj4();
    QString proj4b = projimpl->toProj4();

    return proj4a == proj4b;
}

bool ProjectionImplementation::isSet(Projection::ProjectionParamValue type) const{
    auto iter = _parameters.find(type);
    if ( iter != _parameters.end()) {
        return (*iter).second._isSet;
    }
    return false;
}

QString ProjectionImplementation::toWKT(quint32 spaces)
{
    std::map<Projection::ProjectionParamValue, QString>  kvp = { { Projection::pvLAT1, "standard_parallel_1"}, { Projection::pvLAT2,"standard_parallel_2"},{ Projection::pvLAT0,"latitude_of_origin"}, { Projection::pvK0,"scale_factor"},
                                                                 { Projection::pvLON0,"central_meridian"}, { Projection::pvX0,"false_easting"}, { Projection::pvY0,"false_northing"}};
    QString result;
    QString indent = QString(" ").repeated(spaces);
    QString ending = spaces == 0 ? "" : "\n";

    bool isUTM = _coordinateSystem->projection()->code() == "utm";
    for(auto parm : _parameters){
        if ( parm.second._isSet){
            if ( parm.first == Projection::pvZONE){
                if ( isUTM){
                    result += indent + QString("PARAMETER[\"scale_factor\",0.9996],") + ending;
                    result += indent + QString("PARAMETER[\"false_easting\",500000],") + ending;
                    result += indent + QString("PARAMETER[\"false_northing\",0],") + ending;
                    result += indent + QString("PARAMETER[\"scale\",0.9996],")  + ending;
                    result += indent + QString("PARAMETER[\"latitude_of_origin\",0],") + ending;
                    result += indent + QString("PARAMETER[\"central_meridian\",%1]").arg(parm.second._value.toDouble() * 6.0 - 183.0);
                }
            }else {
                QString name =kvp[parm.first];
                QString v = parm.second._value.toString();
                if ( result != "")
                    result += ",";
                result += indent + QString("PARAMETER[%1,%2]").arg(name).arg(v)  + ending ;
            }
        }
    }
    return result;
}


