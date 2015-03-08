#include <QDebug>
#include <memory>
#include "kernel.h"
#include "ilwisdata.h"
#include "mastercatalog.h"
#include "iooptions.h"
#include "uicontextmodel.h"
#include "coveragelayermodel.h"
#include "visualizationmanager.h"

using namespace Ilwis;

LayerManager::LayerManager(QObject *parent) :
    QObject(parent)
{
}

LayerManager::LayerManager(QObject *parent, UIContextModel *context) : QObject(parent), _uicontext(context)
{
    IlwisTypes metatype = itCOLLECTION | itCATALOGVIEW;
    Resource res("Global Property Editors", metatype);
    CoverageLayerModel * model = new CoverageLayerModel(_layers.size(), res, _uicontext->propertyEditors(IIlwisObject()),0, this);
    model->iconPath("layers.png");
    _layers.append(model);
}
void LayerManager::addVisualizationModel(CoverageLayerModel *newmodel)
{
    _layers.insert(1,newmodel);
}

void LayerManager::addDataSource(const QUrl &url, IlwisTypes tp, Ilwis::Geodrawer::DrawerInterface *drawer)
{
    if ( tp == itUNKNOWN)
        return;
    Resource resource = mastercatalog()->name2Resource(url.toString(),tp);
    if ( !resource.isValid())
        return;
    IIlwisObject obj(resource);
    _layers.insert(1,new CoverageLayerModel(_layers.size(), resource, _uicontext->propertyEditors(obj), drawer, this));
    emit layerChanged();
}

bool LayerManager::zoomInMode() const
{
    return _zoomInMode;
}

void LayerManager::setZoomInMode(bool yesno)
{
    _zoomInMode = yesno;
}

bool LayerManager::hasSelectionDrawer() const
{
    return _hasSelectionDrawer;
}

void LayerManager::setHasSelectionDrawer(bool yesno)
{
    _hasSelectionDrawer = yesno;
}

QQmlListProperty<CoverageLayerModel> LayerManager::layers()
{
    return QQmlListProperty<CoverageLayerModel>(this, _layers);
}

CoverageLayerModel *LayerManager::layer(quint32 layerIndex){
    if ( layerIndex < _layers.size())
        return _layers[layerIndex];
    return 0;
}

void LayerManager::init()
{

}
