
#ifdef __GNUG__
#pragma implementation
#endif

#include "LAPriceDataManager.h"
#include "LADataHolder.h"
#include "LAObject.h"

using namespace std;

/*!
    @brief default data(currently unused)
*/
void
LAPriceDataManager::setUpDefault(void)
{
#if 0 // unused
	// set Data Object
#endif
}

/*!
    @brief default constructor
*/
LAPriceDataManager::LAPriceDataManager()
:mAttrCollection(), mObjectCollection()
{
	setUpDefault();
}

/*!
    @brief destructor
*/
LAPriceDataManager::~LAPriceDataManager(void)
{
}

/*!
    @brief return the Data Holder that corresponds to the name of Data

    if you do not have a name for Data, return the temporary Holder

    @param[in] name Data name

    @return Data Holder
*/
const LADataHolder& 
LAPriceDataManager::getData(const LAString& name) const
{
	return mAttrCollection.getData(name);
}

// return the Data Holder that corresponds to the name of Data or the alias
const LADataHolder&
LAPriceDataManager::getData( const LAString& name, const LAString& alias ) const
{
	// get the name data if missing try the alias data
	LADataHolder dh = getData( name );
	if( dh.isDefined() && !dh.isNull() )
	{
		return getData( name );
	}
	return getData( alias );
}

/*!
    @brief return the Data Holder that corresponds to the id of Data

    if you do not have a name for Data, return the temporary Holder

    @param[in] id Data ID

    @return Data Holder
*/
const LADataHolder& 
LAPriceDataManager::getData(object_t id) const
{
	map<object_t, LADataHolder>::const_iterator it = mObjectCollection.find(id);
	if (it == mObjectCollection.end())
	{
		return NULL_DATA_HOLDER;
	}
	return it->second;
}

// return the Data Holder that corresponds to the id of Data  or the alias
const LADataHolder&
LAPriceDataManager::getData( object_t id, object_t alias_id ) const
{
	// get the id data if missing try the alias_id data
	if( getData( id ) == NULL_DATA_HOLDER )
	{
		// must not return dh, since it will go out of scope and is a temp variable
		return getData( alias_id );
	}
	return getData( id );
}

/*!
    @brief return the array of the registered Data ID

    @return Data ID vector
*/
vector<object_t>
LAPriceDataManager::getDataArray(void) const
{
	vector<object_t> ret;
	map<object_t, LADataHolder>::const_iterator it = 
								mObjectCollection.begin();
	ret.reserve(mObjectCollection.size());
	for(; it != mObjectCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}

/*!
    @brief return the array of name of the registered Data ID

    @return the array of name of the registered Data ID
*/
vector<LAString>
LAPriceDataManager::getNameArray(void) const
{
	vector<LAString> ret;
	AttrConstIter it = mAttrCollection.begin();
	for(;it != mAttrCollection.end(); ++it)
	{
		ret.push_back(it->first);
	}
	return ret;
}

/*!
    @brief register Data name and Data id

    Data ID must have bee already set 

    @param[in] name Data name
    @param[in] id Data id

    @return Data Holder
*/
const LADataHolder&	
LAPriceDataManager::setData(const LAString& name, object_t id)
{
	const LADataHolder* dh = &mAttrCollection.getData(name, NOCHECK);
	if (dh->isDefined())
	{
		if(dh->getType() != id)
		{
			LAString msg = name;
			msg += " is already registered with different id";
			throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);			
		}
		return *dh;
	}

	const LADataHolder& h = getData(id);
	if (!h.isDefined())
	{
		LAString msg("No object_t[");
		msg += LAString(id) + "] is registered";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	return mAttrCollection.add(name, h);
}

/*!
    @brief register the Data

	LAPriceDataType Master holds ownership of Data object

    @param[in] att Data to be registered

    @return Data Holder to be set 
*/
const LADataHolder&	
LAPriceDataManager::setData(LAPriceDataType* att)
{
	if (att == NULL)
	{
		throw LACoreInvalidData("Argument is NULL", __FILE__,__LINE__);
	}
	LADataHolder& h = mObjectCollection[att->getType()];
	if (h.isDefined())
	{
		LAString msg("Duplicate object_t[");
		msg += LAString(att->getType()) + "]";
		throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}
	h.setData(att);
	return h;
}

/*!
    @brief delete Data with selected Data ID

    @param[in] type Data ID to be deleted
*/

void
LAPriceDataManager::removeData(object_t type)
{
	mObjectCollection.erase(type);
	// delete Data (with object_t equal to type) from mAttrCollection
	AttrIter it = mAttrCollection.begin();
	for(;it != mAttrCollection.end(); ++it)
	{
		if (it->second.getType() == type)
		{
			mAttrCollection.remove(it->first);
			it = mAttrCollection.begin();
		}
	}
}

/*!
    @brief remove the Data with the specified the Data name

    @param[in] name Data name to be removed
*/
void
LAPriceDataManager::removeData(const LAString& name)
{
	mAttrCollection.remove(name);
}

/*!
    @brief clear the Data data

    clear the all data registered the Data Master
*/
void				
LAPriceDataManager::clear()
{
	mObjectCollection.clear();
	mAttrCollection.clear();
}

////////////////////// PRIVATE /////////////////////////////////
/*!
    @brief copy constructor

    @param[in] a original object
*/
LAPriceDataManager::LAPriceDataManager(const LAPriceDataManager& a)
:mAttrCollection(a.mAttrCollection), 
 mObjectCollection(a.mObjectCollection)
{
}
