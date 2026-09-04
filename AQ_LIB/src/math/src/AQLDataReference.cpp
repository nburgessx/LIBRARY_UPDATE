/*! @file
    @brief Implementation of data to refer an object.
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLCoreReferencePool.h"
#include "AQLDataInstance.h"

using namespace std;

/*!
	@brief get the referenced Object Holder
	@return the referenced Object Holder
*/
const AQLObjectHolder&
AQLDataReference::get() const
{
	if (!mpRef)
	{
		throw AQLCoreInvalidData("Reference Object is not set", __FILE__, __LINE__);
	}
	if (!mpRef->isDefined())
	{
		//error
		AQLString msg = mRefName;
		msg += " is not set";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return *mpRef;
}

/*!
	@brief the referenced Object Holder
	@return the referenced Object Holder
*/
AQLObjectHolder&
AQLDataReference::get()
{
	if (!mpRef)
	{
		throw AQLCoreInvalidData("Reference Object is not set", __FILE__, __LINE__);
	}
	if (!mpRef->isDefined())
	{
		//error
		AQLString msg = mRefName;
		msg += " is not set";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return *mpRef;
}


/*!
    @brief set the referenced Object from 
	
	set the referenced Object to member functions that the Data has	
*/
void
AQLDataReference::setReferencer(void)
{
	if (mpRef != NULL) // 
	{
		AQLObject* e = getObject();
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
AQLDataReference::delReferencer(void)
{
	if (mpRef != NULL)
	{
		AQLObject* e = getObject();
		// is this data a part of Object?
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
AQLDataReference::setReferencee(void)
{
	// suppose that it is called when valid value is given to mRefName
	mpRef = getReferencee();
	setReferencer();
}

/*!
    @brief with registered in the referenced Object Reference Pool, get the Object Holder
*/
AQLObjectHolder*
AQLDataReference::getReferencee(void)
{
	if (isNull()) return NULL;
	// suppose that it is called when valid value is given to mRefName
	AQLObject* e = getObject();
	if (e != NULL )
	{
		AQLDataInstance* dp = e->getDataInstance();
		if (dp  != NULL)
		{
			AQLCoreReferencePool& objPool = dp->getReferencePool();
			return &(objPool.setReference(mRefName));  // set Reference Pool
		}
	}
	return NULL;
}

/*!
    @brief default constructor
*/
AQLDataReference::AQLDataReference(void) : AQLPriceDataType(DATA_REFERENCE)
{
	mpRef = NULL;
	mRefName="";
	setNull();
}

/*!
    @brief constructor

    @param[in] attr referenced Object Holder

*/
AQLDataReference::AQLDataReference(AQLObjectHolder* attr) 
					: AQLPriceDataType(DATA_REFERENCE)
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
AQLDataReference::AQLDataReference(const AQLDataReference& attr) 
					: AQLPriceDataType(DATA_REFERENCE)
{
	mpRef = NULL;
	mRefName = attr.mRefName;
	setNull(attr.isNull());
}
    
/*!
    @brief destructor
*/
AQLDataReference::~AQLDataReference(void)
{
	delReferencer();
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*
AQLDataReference::clone() const
{
    try {
        AQLDataReference*	pAttr = new AQLDataReference(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief referenced object name in the string representation

    @return referenced object name
*/
AQLString      
AQLDataReference::convertToString(void) const
{
	if (isNull()) return NULL_STR;

	AQLString ret("\"");
	ret += mRefName;
	return ret + "\"";
}

/*!
    @brief set the reference from string representation
*/
void          
AQLDataReference::convertFromString(const AQLString& str)
{
	AQLString data;
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
AQLDataReference::set(AQLObjectHolder* attr)
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
AQLPriceDataType&
AQLDataReference::assignment(const AQLPriceDataType& a)
{
	if (this == &a) return *this;

	// execute it after check
	if (a.getType() != DATA_REFERENCE) {
		AQLString err = "Assignment error for AQLDataReference : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	// update is necessary
	update();
	// delelete previous reference
	delReferencer(); 
	// pointer set
	mRefName = dynamic_cast<const AQLDataReference&>(a).mRefName;
	setNull(a.isNull());

	setReferencee();
	return *this;
}

/*!
    @brief compare function (with respect to the other object)

    @param[in] a AQLDataReference to be compared

    @retval  1 (Object reference number of the Data) > (Object reference number of Data to be compared)
    @retval  0 (Object reference number of the Data) = (Object reference number of Data to be compared)
    @retval -1 (Object reference number of the Data) < (Object reference number of Data to be compared)

    @sa AQLString::cmp()
*/
int
AQLDataReference::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_REFERENCE) 
	{
		AQLString err = "Compare error for AQLDataReference : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull()) return 0;
	if (isNull()) return -1;
	if (a.isNull()) return 1;

	return mRefName.cmp(dynamic_cast<const AQLDataReference&>(a).mRefName);
}

/*!
    @brief set a pointer to the Holder of the Data

    able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void
AQLDataReference::setHolder(AQLDataHolder* holder)
{
	AQLPriceDataType::setHolder(holder);
	setReferencee();
}

/*!
    @brief inform the Object Data that the Data has been updated

    It is informed (to original Object) that TYPE_BROADCAST type is updated(= the referenced Object is updated)
*/
void
AQLDataReference::update(void)
{
#ifndef USE_MODEL_VERSIONING
	AQLPriceDataType::update(TYPE_BROADCAST);
#else  // USE_MODEL_VERSIONING
    AQLPriceDataType::update(TYPE_NORMAL);
#endif
}
