/*! @file
    @brief Implementation to register object classes with some references.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreReferencePool.h"
#include "AQLObjectPool.h"
#include "AQLDataInstance.h"


/*!
    @brief get the Object Holder of the selected name from the Object registered in the Reference Pool

    return temporary Holder if not exist

    @param[in] name Object name

    @return Object Holder
*/
AQLObjectHolder&		
AQLCoreReferencePool::getReference(const AQLString& name)
{
	EntityIter it = mReferencee.find(name);
	if (it == mReferencee.end())
	{
		return NULL_OBJECT_HOLDER;
	}
	return it->second;
}

/*!
    @brief set the referenced Object from the Reference Pool

	return the Object Holder if alreay registered

    @param[in] name Object name

    @return Object Holder to be registered
*/
AQLObjectHolder&
AQLCoreReferencePool::setReference(const AQLString& name)
{
	AQLObjectHolder* ret;
	// set Referencee
	EntityIter it = mReferencee.find(name);
	if (it == mReferencee.end())
	{	
		AQLObjectPool& objPool = mpDataInstance->getObjectPool();
		ret = &(mReferencee[name]);
		if (objPool.find(name))
		{
			*ret = objPool.getObject(name, ENCHKTYPE_ISDEFINED);
		} 
		else
		{
			*ret = AQLObjectHolder(NULL, false);
			ret->setName(name); 
		}
		//try 
		//{
		//	*ret = objPool.getObject(name, ENCHKTYPE_ISDEFINED);
		//} 
		//catch(AQLCoreInvalidData&)
		//{
		//	*ret = AQLObjectHolder(NULL, false);
		//	ret->setName(name); 
		//}
		ret->setReference();
	}
	else
	{
		ret = &(it->second);
	}
	return *ret;
}

/*!
    @brief delete the referenced Object from the Reference Pool

	However, if the original Object exists, an error occurs.

    @param[in] name Object name to be deleted
*/
void				
AQLCoreReferencePool::delReference(const AQLString& name)
{
	EntityIter it = mReferencee.find(name);
	if (it->second.getReferencer().size() != 0)
	{
		AQLString msg("Object [");
		msg += name;
		msg += "] is still referenced by another object";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}
	it->second.setReference(false);
	mReferencee.erase(it);
}

/*!
    @brief delete the referenced Object from the Reference Pool

	However, if the original Object exists, an error occurs.

    @param[in] name Object name to be deleted
*/
void				
AQLCoreReferencePool::delReferenceNoCheck(const AQLString& name)
{
	EntityIter it = mReferencee.find(name);
	if (it != mReferencee.end())
	{
		it->second.setReference(false);
		mReferencee.erase(it);
	}
}

/*!
    @brief resolve unresolved references as possible

*/
void				
AQLCoreReferencePool::completeDependency(void)
{
	AQLObjectPool& objPool = mpDataInstance->getObjectPool();
	EntityIter it = mReferencee.begin();
	for (; it != mReferencee.end(); ++it)
	{
		if (! it->second.isDefined())
		{
			it->second.copyWithoutUpdate(objPool.getObject(it->first, ENCHKTYPE_ISDEFINED));
			it->second.setReference();
		}
	}
}

/*!
    @brief clear the Reference data
*/
void
AQLCoreReferencePool::clear()
{
	EntityIter it = mReferencee.begin();
	for (; it != mReferencee.end(); ++it)
	{
		it->second.setReference(false);
	}		
}

////////////////////////////// PRIVATE //////////////////////////
/*!
    @brief default constructor
*/
AQLCoreReferencePool::AQLCoreReferencePool()
: mpDataInstance(NULL)
{
}

/*!
    @brief destructor
*/
AQLCoreReferencePool::~AQLCoreReferencePool()
{
}

/*!
    @brief not copy the selected Reference bool, but return the reference to itself

    @param[in] rp dummy Reference Pool(not used)
*/
AQLCoreReferencePool& 
AQLCoreReferencePool::copy(const AQLCoreReferencePool& rp)
{
	(void)rp;
	return *this;
}

/*!
    @brief set the AQLDataInstance object

    @param[in] dataInstance AQLDataInstance object to be set
*/
void 
AQLCoreReferencePool::setDataInstance(AQLDataInstance* dataInstance)
{
	mpDataInstance = dataInstance;
}
