#include "attributesetter.h"
#include "raster.h"
#include "featurecoverage.h"
#include "feature.h"
#include "table.h"

REGISTER_PROPERTYEDITOR("attributeeditor",AttributeSetter)

AttributeSetter::AttributeSetter(QObject *parent) : PropertyEditor("attributeeditor",TR("Attributes"), QUrl("AttributeProperties.qml"), parent)
{

}

AttributeSetter::~AttributeSetter()
{

}

PropertyEditor *AttributeSetter::create()
{
    return new AttributeSetter();
}

bool AttributeSetter::canUse(const IIlwisObject &obj) const
{
    if (!obj.isValid())
        return false;

    if ( !hasType(obj->ilwisType(), itCOVERAGE)){
        return false;
    }
    if ( hasType(obj->ilwisType(), itFEATURE)){
        IFeatureCoverage features = obj.as<FeatureCoverage>();
        return features->attributeDefinitions().definitionCount() > 0;
    }
    if ( hasType(obj->ilwisType(), itRASTER)){
        IRasterCoverage raster = obj.as<RasterCoverage>();
        return raster->hasAttributes();
    }
    return false;
}

QStringList AttributeSetter::attributes()
{
    return _attributes;
}

void AttributeSetter::prepare(const IIlwisObject &obj)
{
    if ( !hasType(obj->ilwisType(), itCOVERAGE)){
        return ;
    }
    if ( hasType(obj->ilwisType(), itFEATURE)){
        IFeatureCoverage features = obj.as<FeatureCoverage>();
        for(int i =0; i < features->attributeDefinitions().definitionCount(); ++i){
            _attributes.push_back(features->attributeDefinitions().columndefinition(i).name());
        }
    }
    if ( hasType(obj->ilwisType(), itRASTER)){
        IRasterCoverage raster = obj.as<RasterCoverage>();
        if ( raster->hasAttributes()){
            for(int i=0; i < raster->attributeTable()->columnCount(); ++i){
                _attributes.push_back(raster->attributeTable()->columndefinition(i).name());
            }
        }
    }
}



