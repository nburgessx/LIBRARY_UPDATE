// LAObjectHolder.cpp

#ifdef __GNUG__
#pragma implementation
#endif

#include "LAObjectHolder.h"
#include "LACoreReferencePool.h"
#include "LADataInstance.h"

using namespace std;

/*!
    @brief default constructor

	Ownership of pointer to the Object is supposed to be led to LAObjectHolder
	(default is df =true) so that memory is not allowded to be released in other class.

    @param[in] e  pointer to the Object
    @param[in] df whether this Holder holds ownership of the Object
*/
LAObjectHolder::LAObjectHolder(LAObject* e, bool df)
	: mpObject(e), mpDataInstance(NULL), mName(""), mReferencers(), mDeleteFlag(df), mIsInPool(false)
{
}

// Copy Constructor
LAObjectHolder::LAObjectHolder(const LAObjectHolder& e)
{
	mDeleteFlag = false;
	mIsInPool = false;
	mName="";
	copy(e);
}

// Destructor
LAObjectHolder::~LAObjectHolder()
{
	destroy();
}

// Set the flag which shows the ownership of Object
void				
LAObjectHolder::setDeleteFlag(bool delFlag)
{
	mDeleteFlag = delFlag;
}

/*!
    @brief set the Object of the Holder

	It is controlled by the flag whether ownership of Object moves or not.

    @param[in] e Object to be set
    @param[in] df the flag which shows the ownership of Object
*/
void
LAObjectHolder::setEntity(LAObject* e, bool df)
{
	// not copy itself
	if (mpObject != e)
	{
		if (mDeleteFlag)
		{	// delete if DeleteFlag is true
			// clear the dependency
			delete mpObject;
		}
		else
		{
			if (mIsInPool && mpObject != NULL)
			{
				mpObject->setHolder(NULL);
			}
		}
		mpObject = e;

		if (e != NULL)
		{
			if (mpObject->getDataInstance() == NULL)
			{
				mpObject->setDataInstance(mpDataInstance);
			}
			if (mIsInPool)
			{
				mpObject->setHolder(this);
				broadcast();
			}
		}
	}
	mDeleteFlag = df;
}

/*!
    @brief relational operator thorough magnitude relationship of EntityID

    If the object is not defined, the behavior is undetermined.

    @param[in] objectHolder the other object to be compared
*/
bool               
LAObjectHolder::operator==(const LAObjectHolder& objectHolder) const
{
	return (mpObject->getType() == objectHolder.mpObject->getType());
}

/*!
    @brief relational operator thorough magnitude relationship of EntityID

    If the object is not defined, the behavior is undetermined.

    @param[in] objectHolder the other object to be compared
*/
bool                
LAObjectHolder::operator < (const LAObjectHolder& objectHolder) const
{
	return (mpObject->getType() < objectHolder.mpObject->getType());
}

/*!
    @brief set the LADataInstance object wchich belongs to the Object Holder

    not hold ownership of LADataInstance object and setRool() also called

    @param[in] dataInstance LADataInstance object to be set
*/
void
LAObjectHolder::setDataInstance(LADataInstance* dataInstance)
{
	mpDataInstance = dataInstance;
	if (mpObject != NULL)
	{
		mpObject->setDataInstance(dataInstance);
	}
}

/*!
    @brief shallow copy of the specified Object Holder object

	If the Object Holder to copy object is not defined, the behavior is undetermined.
    
    @param[in] e original Object Holder object

    @return Object Holder object to be copied
*/
LAObjectHolder&
LAObjectHolder::copy(const LAObjectHolder& e)
{
	copyWithoutUpdate(e);
	// Broadcast
	if (mIsInPool)
	{
		broadcast();
	}
	return *this;
}
/*!
    @brief release the memory

	free the pointer to the Object
*/
void
LAObjectHolder::destroy()
{
	if (mDeleteFlag)
	{
		mDeleteFlag = false;
		delete mpObject;
		mpObject = NULL;
	}
	if (mIsInPool && mReferencers.size() > 0)
	{
		// remove regsitration of itself
		for(;mReferencers.size() > 1;)
		{
			(*mReferencers.begin())->delReferencee(this);
		}
		// last delete
		// called again
		(*mReferencers.begin())->delReferencee(this);
	}
}

/////////////////////////////////////////////////////////////////////
/*!
    @brief inform other Holders(LAObject*) that this Holder has been set in Reference Pool(registration Pool of the referenced Object)

    @param[in] isInPool flag whether data is set in Reference Pool or not(if true, set in Pool, and false then, exclude from Pool)
*/
void				
LAObjectHolder::setReference(bool isInPool)
{
	if (mIsInPool == isInPool) return;
	mIsInPool = isInPool;
	if (mpObject != NULL)
	{
		if (mIsInPool)
		{
			mpObject->setHolder(this);
		}
		else
		{
			mpObject->setHolder(NULL);
		}
	}
}

/*!
    @brief shallow copy of the specified Object Holder object without broadcast

    @param[in] e original Object Holder
*/
void
LAObjectHolder::copyWithoutUpdate(const LAObjectHolder& e)
{
	// not copy itself
	if (this == &e) 
	{
		return;
	}
#ifdef __MDEBUG__ // adjust in ReferencePool
	if (mIsInPool)
	{
		if (e.mIsInPool) 
		{
			LAString msg("Can not copy ");
			msg += e.mName;
			msg += " to ";
			msg += mName;
			throw LACoreError(msg.getCString(), __FILE__, __LINE__);
		}
	}
#endif
	// arrangement of the old members
	if (mDeleteFlag && mpObject != e.mpObject) 
	{
		delete mpObject;
		mpObject = NULL;
	}
	// set new member
	mDeleteFlag = e.mDeleteFlag;
	if (mDeleteFlag) 
	{
		// clone copy if deleteflag is true
		if (e.mpObject != NULL) 
		{
			mpObject = e.mpObject->clone();
		}
		else
		{
			mpObject = NULL;
		}
	} 
	else
	{
		mpObject = e.mpObject;
	}
	mName = e.mName;
	mpDataInstance = e.mpDataInstance;  // shallow copy
	
	// set into Holder
	if (mpObject != NULL)
	{
		if (mpObject->getDataInstance() == NULL)
		{
			mpObject->setDataInstance(mpDataInstance);
		}
		if (mIsInPool)
		{
			mpObject->setHolder(this);
		}
	}
}

// inform the original Object that the Object is changed
// type update type(TYPE_NORMAL only)
void LAObjectHolder::broadcast(const unsigned int type)
{
	set<LAObject*>::iterator it = mReferencers.begin();
	for (; it != mReferencers.end(); ++it)
	{
#ifndef USE_MODEL_VERSIONING
		(*it)->update(type | TYPE_BROADCAST); // already set updated pointer
#else
        (*it)->update(type | TYPE_NORMAL); // already set updated pointer
#endif
	}
}

// register the original Object into the Object the Holder

void LAObjectHolder::registerReferencer(LAObject* e)
{
	mReferencers.insert(e);	
}

// remove the registration of the original Object for this Object
void LAObjectHolder::removeReferencer(LAObject* e)
{
	EntityPSet::iterator it = mReferencers.find(e);
	if (it != mReferencers.end())
	{
		mReferencers.erase(it);
	}
}

