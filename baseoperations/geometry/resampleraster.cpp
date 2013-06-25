#include <functional>
#include <future>
#include "kernel.h"
#include "raster.h"
#include "symboltable.h"
#include "ilwisoperation.h"
#include "pixeliterator.h"
#include "gridinterpolator.h"
#include "resampleraster.h"

using namespace Ilwis;
using namespace BaseOperations;


Ilwis::OperationImplementation *ResampleRaster::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return new ResampleRaster(metaid, expr);
}

ResampleRaster::ResampleRaster()
{
}

ResampleRaster::ResampleRaster(quint64 metaid, const Ilwis::OperationExpression &expr) :
    OperationImplementation(metaid, expr),
    _method(GridCoverage::ipBICUBIC)
{
}

bool ResampleRaster::execute(ExecutionContext *ctx, SymbolTable& symTable)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare(ctx,symTable)) != sPREPARED)
            return false;

    BoxedAsyncFunc resampleFun = [&](const Box3D<qint32>& box) -> bool {
        PixelIterator iterOut(_outputGC,box);
        GridInterpolator interpolator(_inputGC, _method);
        while(iterOut != iterOut.end()) {
           Voxel position = iterOut.position();
           Coordinate c = _outputGC->georeference()->pixel2Coord(Pixel_d(position.x(),(position.y())));
           Coordinate c2 = _inputGC->coordinateSystem()->coord2coord(_outputGC->coordinateSystem(),c);
           *iterOut = interpolator.coord2value(c2);
            ++iterOut;
        }
        return true;
    };

    bool res = OperationHelper::execute(ctx, resampleFun, _outputGC);

    if ( res && ctx != 0) {
        QVariant value;
        value.setValue<IGridCoverage>(_outputGC);
        ctx->addOutput(symTable,value,_outputGC->name(), itGRIDCOVERAGE, _outputGC->source() );
    }
    return res;
}

Ilwis::OperationImplementation::State ResampleRaster::prepare(ExecutionContext *, const SymbolTable & )
{
    QString gc = _expression.parm(0).value();
    QString outputName = _expression.parm(0,false).value();

    if (!_inputGC.prepare(gc)) {
        ERROR2(ERR_COULD_NOT_LOAD_2,gc,"");
        return sPREPAREFAILED;
    }
    _box = OperationHelper::initialize(_inputGC, _outputGC, _expression.parm(0),itDOMAIN);
    if ( !_outputGC.isValid()) {
        ERROR1(ERR_NO_INITIALIZED_1, "output gridcoverage");
        return sPREPAREFAILED;
    }
    IGeoReference grf;
    grf.prepare(_expression.parm(1).value());
    if ( !grf.isValid()) {
        return sPREPAREFAILED;
    }
    _outputGC->georeference(grf);
    Box2Dd env = grf->pixel2Coord(grf->size());
    _outputGC->envelope(env);
    if ( outputName != sUNDEF)
        _outputGC->setName(outputName);

    QString method = _expression.parm(2).value();
    if ( method.toLower() == "nearestneighbour")
        _method = GridCoverage::ipNEARESTNEIGHBOUR;
    else if ( method.toLower() == "bilinear")
        _method = GridCoverage::ipBILINEAR;
    else if (  method.toLower() == "bicubic")
        _method =GridCoverage::ipBICUBIC;
    else {
        ERROR3(ERR_ILLEGAL_PARM_3,"method",method,"resample");
        return sPREPAREFAILED;
    }

    return sPREPARED;
}

quint64 ResampleRaster::createMetadata()
{
    QString url = QString("ilwis://operations/resample");
    Resource res(QUrl(url), itOPERATIONMETADATA);
    res.addProperty("namespace","ilwis");
    res.addProperty("longname","resample");
    res.addProperty("syntax","resample(inputgridcoverage,targetgeoref,nearestneighbour|bilinear|bicubic)");
    res.addProperty("inparameters","3");
    res.addProperty("pin_1_type", itGRIDCOVERAGE);
    res.addProperty("pin_1_name", TR("input gridcoverage"));
    res.addProperty("pin_1_desc",TR("input gridcoverage with domain any domain"));
    res.addProperty("pin_2_type", itGEOREF);
    res.addProperty("pin_2_name", TR("target georeference"));
    res.addProperty("pin_2_desc",TR("the georeference to which the input coverage will be morphed"));
    res.addProperty("pin_3_type", itSTRING);
    res.addProperty("pin_3_name", TR("Resampling method"));
    res.addProperty("pin_3_desc",TR("The method used to aggregate pixels from the input map in the geometry of the output map"));
    res.addProperty("outparameters",1);
    res.addProperty("pout_1_type", itGRIDCOVERAGE);
    res.addProperty("pout_1_name", TR("output gridcoverage"));
    res.addProperty("pout_1_desc",TR("output gridcoverage with the domain of the input map"));
    res.prepare();
    url += "=" + QString::number(res.id());
    res.setUrl(url);

    mastercatalog()->addItems({res});
    return res.id();
}


