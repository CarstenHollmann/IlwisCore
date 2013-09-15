#ifndef ITEMDOMAIN_H
#define ITEMDOMAIN_H

#include "range.h"
#include "itemrange.h"
#include "itemiterator.h"

namespace Ilwis {

class ItemRange;

/*!
ItemDomains are domains that handle discrete data. For example the domain of a thematic classification. There are various
item domains that are all defined by the item type they control. This is passed as template parameter to the domain.
The range of the domain contains all the valid items for that domain.
 */
template<class D> class ItemDomain : public Domain
{
public:
    template<typename T> friend class ItemIterator;

    ItemDomain<D>()  {
    }
    ItemDomain(const Resource& res) : Domain(res) {
    }

    ~ItemDomain() {
    }
    Domain::Containement contains(const QVariant& val) const{
        if(item(val.toString()) != 0)
            return Domain::cSELF;

        if ( parent().isValid())
            if (parent()->contains(val) == Domain::cSELF)
                return Domain::cPARENT;
        return Domain::cNONE;
    }
    /*!
     returns a string representation of the item pointed to by the index
     * \param 0 based index, if the index is invalid sUNDEF will be returned
     * \return the string representation or sUNDEDF in case no items are defined
     */
    QString value(const QVariant& v) const {
        if (_range.isNull()) {
            ERROR1(ERR_NO_INITIALIZED_1, name());
            return sUNDEF;
        }
        return _range->value(v.toInt());
    }
    /*!
     returns a pointer to the domain item pointed to by the index
     * \param 0 based index, if the index is invalid 0 will be returned
     * \return a (pointer to) domain item or 0 if no items are defined
     */
    SPDomainItem item(quint32 index) const {
        if (_range.isNull()) {
            ERROR1(ERR_NO_INITIALIZED_1, name());
            return SPDomainItem();
        }
        return _range->item(index) ;
    }
    /*!
     returns a pointer to the item that is identified by the string name
     * \param a name that must be in the range of the domain. If not a 0 pointer will be returned
     * \return a (pointer to) domain item or 0 if no items are defined
     */
    SPDomainItem item(const QString& nam) const{
        if (_range.isNull()) {
            ERROR1(ERR_NO_INITIALIZED_1, name());
            return SPDomainItem();
        }
        if(considerParent() ) {

        }
        return _range->item(nam) ;
    }
    /*!
     Adds an item of the templated type to the range. If no range is yet defined, one will be created
     * \param the item to be added. Note that ownership of the item is transferred to the range. no delete allowed
     *
     */
    void addItem(DomainItem* thing) {
        if (thing == 0)
            return;
        if ( _range.isNull()) {
            _range.reset(D::createRange());
        }
        if (parent().isValid()) {
            IDomain dm = parent();
            IlwisData<ItemDomain<D>> itemdom = dm.get<ItemDomain<D>>();
            if(!itemdom.isValid()){
                ERROR2(ERR_COULD_NOT_CONVERT_2,TR("domain"), TR("correct item domain"));
                return;
            }
            SPDomainItem item = itemdom->item(thing->name());
            if (item.isNull()){
                ERROR2(ERR_NOT_FOUND2,thing->name(), TR("parent domain"));
                return;
            }
            delete thing;
            _range->add(item);
        }else
            _range->add(thing);
    }
    /*!
     removes an item from the range
     * \param the item to be removed
     */
    void removeItem(const QString& nme){
        if (_range.isNull()) {
            ERROR1(ERR_NO_INITIALIZED_1, name());
            return ;
        }
        _range->remove(nme);
    }

    void setRange(const ItemRange& range)
    {
        _range.reset(D::createRange());
        _range->addRange(range);
    }
    quint32 count() const {
        if (_range.isNull()) {
            ERROR1(ERR_NO_INITIALIZED_1, name());
            return iUNDEF;
        }
        return _range->count();
    }

    QString theme() const {
        return _theme;
    }
    void setTheme(const QString& theme) {
        _theme = theme;
    }

    void setParent(const IDomain& dm){
        if ( !hasType(dm->ilwisType(), itITEMDOMAIN) ) {
            return;
        }
        if (!hasType(dm->valueType(), valueType())) {
            return;
        }
        IlwisData<ItemDomain<D>> dmitem = dm.get<ItemDomain<D>>();
        if ( theme() != dmitem->theme())
            return;

        bool ok = _range->alignWithParent(dm);
        if (!ok)
            return ;

        Domain::setParent(dm);

    }

    IlwisTypes ilwisType() const {
        return itITEMDOMAIN;
    }

    IlwisTypes valueType() const {
        return D::valueTypeS();
    }

    ItemIterator<D> begin() const {
        return ItemIterator<D>(*this);
    }

    ItemIterator<D> end() const {
        return ItemIterator<D>(*this, count());
    }

protected:
    SPRange getRange() const{
        return _range.dynamicCast<Range>();
    }

private:
    bool considerParent() const{
        return !parent().isValid();
    }

    SPItemRange _range;
    QString _theme;
};

template<typename T> Ilwis::ItemIterator<T> begin(const Ilwis::IlwisData<Ilwis::ItemDomain<T>>& idom) {
    return idom->begin();
}

template<typename T> Ilwis::ItemIterator<T> end(const Ilwis::IlwisData<Ilwis::ItemDomain<T>>& idom) {
    return idom->end();
}

class ThematicItem;
class IndexedIdentifier;
class NamedIdentifier;
class NumericItem;

typedef IlwisData<ItemDomain<ThematicItem>>  IThematicDomain ;
typedef IlwisData<ItemDomain<IndexedIdentifier>>  IIndexedIdDomain ;
typedef IlwisData<ItemDomain<NamedIdentifier>>  INamedIdDomain ;
typedef IlwisData<ItemDomain<NumericItem>>  INumericItemDomain ;
typedef ItemDomain<ThematicItem>  ThematicDomain ;
typedef ItemDomain<IndexedIdentifier>  IndexedIdDomain ;
typedef ItemDomain<NamedIdentifier>  NamedIdDomain ;
typedef ItemDomain<NumericItem>  NumericItemDomain ;

}



#endif // ITEMDOMAIN_H
