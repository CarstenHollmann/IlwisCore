#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "kernel.h"
#include "dataformat.h"

using namespace Ilwis;

DataFormat::DataFormat()
{
}

DataFormat::DataFormat(const QString &code, const QString connector)
{
    QSqlQuery db(kernel()->database());
    QString stmt = QString("select %1 from dataformats where code='" + code + "'").arg(code);
    if ( connector != sUNDEF)
        stmt += " and connector='" + connector + "'";

    if (db.exec(stmt) && db.next()) {
        _properties[fpCODE] = code;
        _properties[fpNAME] = set(db.value("name").toString());
        _properties[fpDESCRIPTION] = set(db.value("description").toString());
        _properties[fpEXTENSION] = set(db.value("extension"));
        _properties[fpCONTAINER] = set(db.value("container").toString());
        _properties[fpCONNECTOR] = set(db.value("connector").toString());
        _properties[fpDATATYPE] = set(db.value("datatype").toULongLong());
        _properties[fpREADWRITE] = set(db.value("readwrite").toString());
        _isValid = true;
    }

}

QVariantList DataFormat::getFormatProperties(FormatProperties prop, IlwisTypes types, QString connector, QString code){
    QVariantList result;
    QSqlQuery db(kernel()->database());
    QString field= "";
    switch( prop){
        case fpCODE:
            field = "code"; break;
        case fpNAME:
            field = "name"; break;
        case fpDESCRIPTION:
            field = "description"; break;
        case fpEXTENSION:
            field = "extension"; break;
        case fpCONTAINER:
            field = "type"; break;
        case fpDATATYPE:
            field = "datatype"; break;
        case fpCONNECTOR:
            field = "connector"; break;
        case fpREADWRITE:
            field = "readwrite"; break;
        case fpEXTENDEDTYPE:
            field = "extendedtype"; break;
    }

    QString stmt = QString("select %1 from dataformats where (datatype | %2) != 0").arg(field).arg(types);
    if ( code != sUNDEF)
        stmt += " and code='" + code + "'";
    if ( connector != sUNDEF)
        stmt += " and connector='" + connector + "'";

    if (db.exec(stmt)) {
        while(db.next()){
            QVariant var = db.value(0).toString();
            if ( var.type() == QMetaType::QString){
                QStringList parts = var.toString().split(",");
                for(QString part : parts) {
                    result += part;
                }
            }
        }

    }
    return result;
}

bool DataFormat::supports(DataFormat::FormatProperties fp, IlwisTypes tp, const QVariant &prop, const QString& connector)
{
    QVariantList props = DataFormat::getFormatProperties(fp, tp,connector);
    for(QVariant& p : props){
        if ( p == prop)
            return true;
    }
    return false;
}

bool DataFormat::setFormatInfo(const QString& path, const QString connector) {
    QFile file;
    file.setFileName(path);
    if (file.open(QIODevice::ReadOnly)) {
        QString settings = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(settings.toUtf8());
        if ( !doc.isNull()){
            QSqlQuery sqlPublic(kernel()->database());
            QJsonObject obj = doc.object();
            QJsonValue formats = obj.value("Formats");
            if ( formats.isArray()){
                QJsonArray arrFormats = formats.toArray();
                for(auto iter = arrFormats.begin(); iter != arrFormats.end(); ++iter) {
                    auto jsonValue  = *iter;
                    if ( jsonValue.isObject()) {
                        QJsonObject objv = jsonValue.toObject();
                        QString code = objv.value("code").toString(sUNDEF);
                        QString name = objv.value("name").toString(sUNDEF);
                        QString desc = objv.value("description").toString(sUNDEF);
                        QString type = objv.value("type").toString(sUNDEF);
                        QString ext = objv.value("extension").toString(sUNDEF);
                        QString datatp = objv.value("datatypes").toString(sUNDEF);
                        quint64 ilwtype = itUNKNOWN;
                        QStringList parts = datatp.split(",");
                        for(QString tp : parts)
                            ilwtype |= IlwisObject::name2Type(tp);
                        QString rw = objv.value("readwrite").toString("r");
                        QString extt = objv.value("extendedtype").toString(sUNDEF);
                        quint64 exttypes = itUNKNOWN;
                        parts = extt.split(",");
                        for(QString tp : parts)
                            exttypes |= IlwisObject::name2Type(tp);

                        QString parms = QString("'%1','%2','%3','%4','%5',%6,'%7','%8',%9").arg(code,name,desc, ext,type).arg(ilwtype).arg(connector).arg(rw).arg(exttypes);
                        QString stmt = QString("INSERT INTO dataformats VALUES(%1)").arg(parms);
                        bool ok = sqlPublic.exec(stmt);
                        if (!ok) {
                            return kernel()->issues()->logSql(sqlPublic.lastError());
                        }
                    }

                }
                return true;
            }
        }

    }
    return false;
}

QVariant DataFormat::property(DataFormat::FormatProperties prop) const
{
    auto iter = _properties.find(prop);
    if (  iter != _properties.end())
        return (*iter).second;
    return QVariant();
}

bool DataFormat::isValid() const
{
    return _isValid;
}

QVariant DataFormat::set(const QVariant &original) const
{
    if ( original.type() == QMetaType::QString){
        if ( original.toString() == "")
            return sUNDEF;
    }
    if ( original.type() == QMetaType::ULongLong){
        bool ok;
        original.toULongLong(&ok);
        if (!ok )
            return itUNKNOWN;
    }
    return original;
}
