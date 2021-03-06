#ifndef BASEDRAWER_H
#define BASEDRAWER_H


#include <QtGui/QOpenGLFunctions>
#include "iooptions.h"
#include "drawers/drawerinterface.h"
#include "box.h"
#include "identity.h"

namespace Ilwis {
namespace Geodrawer{

class RootDrawer;

class BaseDrawer : public DrawerInterface, public Ilwis::Identity
{
public:
    enum Containment { cINSIDE, cOUTSIDE, cUNKNOWN};

    bool prepare(DrawerInterface::PreparationType, const IOOptions&);
    void unprepare(PreparationType prepType);
    bool isPrepared(quint32 type=ptALL) const;
    bool draw(QOpenGLContext *, const IOOptions&) const;

    RootDrawer* rootDrawer() ;
    const RootDrawer *rootDrawer() const;
    DrawerInterface* parent();
    const DrawerInterface *parent() const;

    bool isActive() const ;
    void active(bool yesno);
    bool isValid() const;
    void selected(bool yesno);
    bool isSelected() const;
    Containment containment() const;
    void cleanUp();

    void code(const QString& code);
    QString code() const;
    quint64 id() const;
    QString name() const;
    void name(const QString& n);
    QString description() const;
    void setDescription(const QString& desc);

    std::vector<QVariant> attributes(const QString& attrNames) const;
    QVariant attribute(const QString& attrName) const;
    void attribute(const QString& attrName, const QVariant& attrib);

    QColor color(const IRepresentation& rpr,double value, DrawerInterface::ColorValueMeaning cvm = cvmTRUEVALUE);

protected:
    BaseDrawer(const QString &name, DrawerInterface *parentDrawer, RootDrawer *rootdrawer, QObject *parent=0);
    void valid(bool yesno);

    std::vector<VertexPosition> _positions;
    quint32 _prepared = 0;

private:
    bool _active = true; // unless defined otherwise, the drawer is active
    bool _valid = false;
    bool _selected = false;
    RootDrawer* _rootDrawer = 0;
    DrawerInterface* _parentDrawer = 0;
    Envelope _envelope;




};

typedef std::unique_ptr<QOpenGLFunctions> UPOpenGL;
}
}

#endif // BASEDRAWER_H
