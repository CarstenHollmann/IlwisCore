#ifndef ILWISCONTEXT_H
#define ILWISCONTEXT_H

#include "kernel_global.h"
#include <QThreadStorage>
#include "ilwisconfiguration.h"
#include "ilwisdata.h"

namespace Ilwis{

class Catalog;
typedef Ilwis::IlwisData<Ilwis::Catalog> ICatalog;

/*!
 * \brief The IlwisContext class A singleton object that can be reached from everywhere in the system that gives access to a number of properties that describe the context of an Ilwis system
 *
 *The context gives access to properties like working location, systemlocation, memory properties etc.
 */
class KERNELSHARED_EXPORT IlwisContext
{
public:
    friend KERNELSHARED_EXPORT IlwisContext* context();

    IlwisContext();
    ~IlwisContext();

    void loadIlwisLocationFile(QFileInfo configFile);
    void addSystemLocation(const QUrl &resource);
    void removeSystemLocation(const QUrl &resource);
    QFileInfo ilwisFolder() const;
    ICatalog workingCatalog() const;
    const ICatalog& systemCatalog() const;
    void setWorkingCatalog(const Ilwis::ICatalog &cat);
    QUrl cacheLocation() const;
    QUrl persistentInternalCatalog() const;
    quint64 memoryLeft() const;
    quint64 changeMemoryLeft(quint64 amount);
    IlwisConfiguration& configurationRef();
    const IlwisConfiguration& configuration() const;
    QFileInfo resourceRoot() const;
    QString ipv4() const;
    QString currentUser() const;
    int runMode() const;
    void runMode(int mode);

private:
    void init();
    static IlwisContext *_context;

    std::vector<QUrl> _systemlocations;
    //QThreadStorage<Catalog *> _workingCatalog;
    ICatalog _workingCatalog;
    ICatalog _systemCatalog;
    quint64 _memoryLimit;
    quint64 _memoryLeft;
    QFileInfo _ilwisDir;
    IlwisConfiguration _configuration;
    QUrl _cacheLocation;
    QUrl _persistentInternalCatalog;
    QString _ipv4 = sUNDEF;
    QString _currentUser;
    int _runMode = rmDESKTOP;
};


KERNELSHARED_EXPORT IlwisContext* context();

template<typename ValueType> ValueType ilwisconfig(const QString& key, const ValueType& defaultValue){
    return context()->configurationRef()(key, defaultValue);
}

template<> inline QString ilwisconfig(const QString& key, const QString& defaultValue){
    return context()->configurationRef()(key, defaultValue);
}


}




//KERNELSHARED_EXPORT Ilwis::IlwisContext* context();

#endif // ILWISCONTEXT_H
