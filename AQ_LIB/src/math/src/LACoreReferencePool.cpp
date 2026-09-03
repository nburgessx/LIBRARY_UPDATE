/*! @file
    @brief Implementation to register object classes with some references.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LACoreReferencePool.h"
#include "LAObjectPool.h"
#include "LADataInstance.h"


/*!
    @brief get the Object Holder of the selected name from the Object registered in the Reference Pool

    return temporary Holder if not exist

    @param[in] name Object name

    @return Object Holder
*/
LAObjectHolder&		
LACoreReferencePool::getReference(const LAString& name)
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
LAObjectHolder&
LACoreReferencePool::setReference(const LAString& name)
{
	LAObjectHolder* ret;
	// set Referencee
	EntityIter it = mReferencee.find(name);
	if (it == mReferencee.end())
	{	
		LAObjectPool& objPool = mpDataInstance->getObjectPool();
		ret = &(mReferencee[name]);
		if (objPool.find(name))
		{
			*ret = objPool.getObject(name, ENCHKTYPE_ISDEFINED);
		} 
		else
		{
			*ret = LAObjectHolder(NULL, false);
			ret->setName(name); 
		}
		//try 
		//{
		//	*ret = objPool.getObject(name, ENCHKTYPE_ISDEFINED);
		//} 
		//catch(LACoreInvalidData&)
		//{
		//	*ret = LAObjectHolder(NULL, false);
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
LACoreReferencePool::delReference(const LAString& name)
{
	EntityIter it = mReferencee.find(name);
	if (it->second.getReferencer().size() != 0)
	{
		LAString msg("Object [");
		msg += name;
		msg += "] is still referenced by another object";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
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
LACoreReferencePool::delReferenceNoCheck(const LAString& name)
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
LACoreReferencePool::completeDependency(void)
{
	LAObjectPool& objPool = mpDataInstance->getObjectPool();
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
LACoreReferencePool::clear()
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
LACoreReferencePool::LACoreReferencePool()
: mpDataInstance(NULL)
{
}

/*!
    @brief destructor
*/
LACoreReferencePool::~LACoreReferencePool()
{
}

/*!
    @brief not copy the selected Reference bool, but return the reference to itself

    @param[in] rp dummy Reference Pool(not used)
*/
LACoreReferencePool& 
LACoreReferencePool::copy(const LACoreReferencePool& rp)
{
	(void)rp;
	return *this;
}

/*!
    @brief set the LADataInstance object

    @param[in] dataInstance LADataInstance object to be set
*/
void 
LACoreReferencePool::setDataInstance(LADataInstance* dataInstance)
{
	mpDataInstance = dataInstance;
}
