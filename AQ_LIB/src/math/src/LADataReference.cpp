/*! @file
    @brief Implementation of data to refer an object.
    
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LACoreReferencePool.h"
#include "LADataInstance.h"

using namespace std;

/*!
	@brief get the referenced Object Holder
	@return the referenced Object Holder
*/
const LAObjectHolder&
LADataReference::get() const
{
	if (!mpRef)
	{
		throw LACoreInvalidData("Reference Object is not set", __FILE__, __LINE__);
	}
	if (!mpRef->isDefined())
	{
		//error
		LAString msg = mRefName;
		msg += " is not set";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return *mpRef;
}

/*!
	@brief the referenced Object Holder
	@return the referenced Object Holder
*/
LAObjectHolder&
LADataReference::get()
{
	if (!mpRef)
	{
		throw LACoreInvalidData("Reference Object is not set", __FILE__, __LINE__);
	}
	if (!mpRef->isDefined())
	{
		//error
		LAString msg = mRefName;
		msg += " is not set";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return *mpRef;
}


/*!
    @brief set the referenced Object from 
	
	set the referenced Object to member functions that the Data has	
*/
void
LADataReference::setReferencer(void)
{
	if (mpRef != NULL) // 
	{
		LAObject* e = getObject();
		// part of the Object?
		if (e != NULL)
		{
			try	{
				e->setReferencee(mpRef); //set the referenced Object to the Object that this Attibute belongs to
			} 
			catch (...)
			{
				mpRef = NULL;
				mRefName="";
				setNull();
				throw;
			}
		}
	}
}

/*!
    @brief remove the referenced Object to member functions that the Data has
*/
void
LADataReference::delReferencer(void)
{
	if (mpRef != NULL)
	{
		LAObject* e = getObject();
		// is this data a part of ObjectH
		if (e != NULL)
		{
			e->delReferencee(mpRef);
		}
	}
}

/*!
    @brief with registered in the referenced Object Reference Pool, set the referenced Object to member functions that the Data has
	
*/
void
LADataReference::setReferencee(void)
{
	// suppose that it is called when valid value is given to mRefName
	mpRef = getReferencee();
	setReferencer();
}

/*!
    @brief with registered in the referenced Object Reference Pool, get the Object Holder
*/
LAObjectHolder*
LADataReference::getReferencee(void)
{
	if (isNull()) return NULL;
	// suppose that it is called when valid value is given to mRefName
	LAObject* e = getObject();
	if (e != NULL )
	{
		LADataInstance* dp = e->getDataInstance();
		if (dp  != NULL)
		{
			LACoreReferencePool& objPool = dp->getReferencePool();
			return &(objPool.setReference(mRefName));  // set Reference Pool
		}
	}
	return NULL;
}

/*!
    @brief default constructor
*/
LADataReference::LADataReference(void) : LAPriceDataType(DATA_REFERENCE)
{
	mpRef = NULL;
	mRefName="";
	setNull();
}

/*!
    @brief constructor

    @param[in] attr referenced Object Holder

*/
LADataReference::LADataReference(LAObjectHolder* attr) 
					: LAPriceDataType(DATA_REFERENCE)
{
	mpRef = attr;
	if (attr != NULL)
	{
		mRefName = attr->getName();
		setReferencer();
	}
	else
	{
		mRefName = "";
	}
	setNull(mRefName == "" ? true : false);
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataReference::LADataReference(const LADataReference& attr) 
					: LAPriceDataType(DATA_REFERENCE)
{
	mpRef = NULL;
	mRefName = attr.mRefName;
	setNull(attr.isNull());
}
    
/*!
    @brief destructor
*/
LADataReference::~LADataReference(void)
{
	delReferencer();
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
LAPriceDataType*
LADataReference::clone() const
{
    try {
        LADataReference*	pAttr = new LADataReference(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief referenced object name in the string representation

    @return referenced object name
*/
LAString      
LADataReference::convertToString(void) const
{
	if (isNull()) return NULL_STR;

	LAString ret("\"");
	ret += mRefName;
	return ret + "\"";
}

/*!
    @brief set the reference from string representation
*/
void          
LADataReference::convertFromString(const LAString& str)
{
	LAString data;
	bool ret = strToData(str, data);
	if (data == mRefName) return;
	// update
	update();
	// delete reference
	delReferencer();

	if (ret || data == "")
	{
		setNull();
		mRefName = "";
		mpRef = NULL;
	}
	else
	{
		setNull(false);
		mRefName = data;
		setReferencee();
	}
}

/*!
    @brief set the referenced Object Holder wchich does not hold owner ship
	
	@param[in] attr the referenced Object Holder
*/
void
LADataReference::set(LAObjectHolder* attr)
{
	// update is necessary
	update();
	// delelete previous reference
	delReferencer(); 
	// pointer check to the Holder
	mpRef = attr;
	if (attr != NULL)
	{
		mRefName = attr->getName();
		setNull(false);
		setReferencer();
	}
	else
	{
		mRefName = "";
		setNull();
	}
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
LAPriceDataType&
LADataReference::assignment(const LAPriceDataType& a)
{
	if (this == &a) return *this;

	// execute it after check
	if (a.getType() != DATA_REFERENCE) {
		LAString err = "Assignment error for LADataReference : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	// update is necessary
	update();
	// delelete previous reference
	delReferencer(); 
	// pointer set
	mRefName = dynamic_cast<const LADataReference&>(a).mRefName;
	setNull(a.isNull());

	setReferencee();
	return *this;
}

/*!
    @brief compare function (with respect to the other object)

    @param[in] a LADataReference to be compared

    @retval  1 (Object reference number of the Data) > (Object reference number of Data to be compared)
    @retval  0 (Object reference number of the Data) = (Object reference number of Data to be compared)
    @retval -1 (Object reference number of the Data) < (Object reference number of Data to be compared)

    @sa LAString::cmp()
*/
int
LADataReference::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_REFERENCE) 
	{
		LAString err = "Compare error for LADataReference : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull()) return 0;
	if (isNull()) return -1;
	if (a.isNull()) return 1;

	return mRefName.cmp(dynamic_cast<const LADataReference&>(a).mRefName);
}

/*!
    @brief set a pointer to the Holder of the Data

    able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void
LADataReference::setHolder(LADataHolder* holder)
{
	LAPriceDataType::setHolder(holder);
	setReferencee();
}

/*!
    @brief inform the Object Data that the Data has been updated

    It is informed (to original Object) that TYPE_BROADCAST type is updated(= the referenced Object is updated)
*/
void
LADataReference::update(void)
{
#ifndef USE_MODEL_VERSIONING
	LAPriceDataType::update(TYPE_BROADCAST);
#else  // USE_MODEL_VERSIONING
    LAPriceDataType::update(TYPE_NORMAL);
#endif
}
