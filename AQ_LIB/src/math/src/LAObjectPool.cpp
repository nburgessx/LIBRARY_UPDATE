/*! @file
    @brief Implementation to register object classes by map.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LAObjectPool.h"
#include "LACoreReferencePool.h"
#include "LAObject.h"
#include "LADataInstance.h"
#include <cstdio>

#ifdef _MSC_VER
using namespace std;
#endif

/*!
    @brief get Object Holder from the Object registered in the EntityPool

    @param[in] name name of Object
    @param[in] type check type

    @return Object Holder
*/
LAObjectHolder			
LAObjectPool::getObject(const LAString& name, EntityCheckType type)
{
	EntityIter it = mCollection.find(name);
	if (it == mCollection.end())
	{
		if (type == ENCHKTYPE_ISDEFINED)
		{
			LAString msg("No Data for name = ");
			msg += name;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		return NULL_OBJECT_HOLDER;
	}
	// return Holder that does not master memory
	return it->second; 
}

/*!
    @brief check whether object exists in mCollection

    @param[in] name name of Object

    @return bool
*/
bool
LAObjectPool::find(const LAString& name)
{
	return (mCollection.find(name) == mCollection.end()) ? false : true;
}

/*!
    @brief return the full list of data names from object pool

    @return all data names
*/
LAStringVector LAObjectPool::getNames() const
{
	EntityConstIter it = mCollection.begin();
	LAStringVector names;
	for (; it != mCollection.end(); ++it)
	{
		//it->second.setDeleteFlag(true);
		names.push_back(it->first);
	}
	return names;
}

/*!
    @brief se the Object to the Object Pool

	name must be unique, and ownership of the Object must be owned by this class. 
	
	@param[in] name name of Object
    @param[in] e Object object to be set
*/
void				
LAObjectPool::set(const LAString& name, LAObject* e)
{
	// generate Holde to stor Object
	LAObjectHolder& objHolder = mCollection[name];
	// set Name
	objHolder.setName(name);
	// Duplicate check
	if (objHolder.isDefined())
	{
		LAString msg("Duplicate for name = ");
		msg += name;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// Object set
	objHolder.setDataInstance(mpDataInstance);
	objHolder.setEntity(e, false);
}

/*!
    @brief delete Object from the Object Pool

	nothing is done when the selected Object has not been registered in Object Pool, 
	and error occurs when the selected Object exists in Reference Pool.

    @param[in] name Object name to delete
*/
void
LAObjectPool::remove(const LAString& name)
{
	EntityIter it = mCollection.find(name);
	if (it == mCollection.end())
	{
		return;
	}
	// In case of it is registered in UnderlyingPool(Reference Pool), then error.
	LACoreReferencePool& rp = mpDataInstance->getReferencePool();
	LAObjectHolder& h = rp.getReference(name);
	if (h.isDefined())
	{
		LAString msg(name);
		msg += " is a referenced Object. Unable to delete it";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	// delete from mCollection
	it->second.setDeleteFlag(true);
	mCollection.erase(it);
}

/*!
    @brief delete Object from the Object Pool whether they exist in reference pool or not

	nothing is done when the selected Object has not been registered in Object Pool, 
	and error occurs when the selected Object exists in Reference Pool.

    @param[in] name Object name to delete
*/
void
LAObjectPool::removeNoCheck(const LAString& name)
{	
	mReferencePool.delReferenceNoCheck(name);

	EntityIter it = mCollection.find(name);
	if (it == mCollection.end())
	{
		return;
	}
	// delete from mCollection
	it->second.setDeleteFlag(true);
	mCollection.erase(it);
}

/*!
    @brief clear all of reference relationship
*/
void
LAObjectPool::clear()
{
	// clear ReferencePool
	mReferencePool.clear();
	// memory release of LAObjectPool
	EntityIter it = mCollection.begin();
	std::vector<std::string> names;
	for (; it != mCollection.end(); ++it)
	{
		it->second.setDeleteFlag(true);
	}
	mCollection.clear();
}

////////////////////////// PRIVATE ////////////////////////////////////////
/*!
    @brief default constructor
*/
LAObjectPool::LAObjectPool(void)
: mCollection(), mReferencePool()
{
}

/*!
    @brief shallow copy of the specified EntityPool

    @param[in] objPool  original Object Pool
*/
LAObjectPool&
LAObjectPool::copy(const LAObjectPool& objPool)
{
	EntityConstIter it = objPool.mCollection.begin();
	for (;it != objPool.mCollection.end(); ++it)
	{
		LAObjectHolder& objHolder = mCollection[it->first];
		LAObject*		e = it->second.clone();
		objHolder.setDataInstance(mpDataInstance);
		objHolder.setEntity(e, false);
		objHolder.setName(it->first);
	}
	mReferencePool.completeDependency();
	return *this;
}

/*!
    @brief destructor
*/
LAObjectPool::~LAObjectPool(void)
{
	clear();
}

/*!
    @brief set the LADataInstance object

	@param[in] dataInstance LADataInstance object to be set
*/
void
LAObjectPool::setDataInstance(LADataInstance* dataInstance)
{
	mpDataInstance = dataInstance;
	mReferencePool.setDataInstance(dataInstance);
}
