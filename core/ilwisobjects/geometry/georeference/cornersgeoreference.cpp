#include "kernel.h"
#include "geometries.h"
#include "ilwisobject.h"
#include "ilwisdata.h"
#include "coordinatesystem.h"
#include "georeference.h"
#include "georefimplementation.h"
#include "simpelgeoreference.h"
#include "cornersgeoreference.h"

using namespace Ilwis;

CornersGeoReference::CornersGeoReference() : SimpelGeoReference("corners"),  _isCornersOfCorners(false)
{

}

GeoRefImplementation *CornersGeoReference::create()
{
    return new CornersGeoReference();
}

void CornersGeoReference::setEnvelope(const Envelope &env)
{
    _envelope = env;
}

bool CornersGeoReference::compute()
{
    bool a = size().isNull();
    bool b = _envelope.isValid();
    if (a || !b)
        return false;

    _a12 = _a21 = 0;
    std::vector<double> vec = _envelope.max_corner() - _envelope.min_corner();

    if (abs(vec[0]) < 1e-6 || abs(vec[1]) < 1e-6) {
        return false;
    }
    if (_isCornersOfCorners) {
        _a11  = size().xsize() / vec[0];
        _a22 = - size().ysize() / vec[1];
        _b1 = - _a11 * _envelope.min_corner().x;
        _b2 = - _a22 * _envelope.max_corner().y;
    }
    else {
        _a11 = (size().xsize() - 1) / vec[0];
        _a22 = - (size().ysize() - 1) / vec[1];
        _b1 = 0.5 - _a11 * _envelope.min_corner().x;
        _b2 = 0.5 - _a22 * _envelope.max_corner().y;
    }
    _det = _a11 * _a22;


    return true;
}


bool CornersGeoReference::isCornersOfCorners() const
{
    return _isCornersOfCorners;
}

Envelope CornersGeoReference::envelope() const
{
    return _envelope;
}

QString CornersGeoReference::typeName()
{
    return "corners";
}



