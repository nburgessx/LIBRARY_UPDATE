/*! @file
    @brief Implementation of data to refer multi entities.
    
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLDataMultiReference.h"
#include "AQLDataVector.h"
#include "AQLObjectHolder.h"
#include "AQLCoreReferencePool.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include <vector>

using namespace std;


/*!
	@brief return the referenced Object selced by index(starting at 0)

	@return referenced Object Holder
*/
AQLObjectHolder&
AQLDataMultiReference::get(int i) const
{
	if( mReferences.size() == 0 )
	{
		//error
		AQLString msg = "mReferences.size() = 0 : AQLDataMultiReference::get( i )";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
	}

	if (!mReferences[i]->isDefined())
	{
		//error
		AQLString msg = mRefNames[i];
		msg += " is not set";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return *(mReferences[i]);
}


/*!
    @brief set the referenced Object Data from the member variables and member variables of the specified Object

    @param[in] objHolder referenced Object Holder to be registered
    @param[in] e  original Object(usually select the Object that this Data belongs to)
*/
void
AQLDataMultiReference::setReferencee(AQLObjectHolder* objHolder, AQLObject* e)
{
    if (objHolder != NULL && e != NULL)
    {
        e->setReferencee(objHolder);
        mReferences.push_back(objHolder);
    }
}

/*!
    @brief  delete the referenced Object Data from the member variables and member variables of the specified Object

    @param[in] objHolder referenced Object Holder to be deleted
    @param[in] e  original Object(usually select the Object that this Data belongs to)
*/
void
AQLDataMultiReference::delReferencee(AQLObjectHolder* objHolder, AQLObject* e)
{
    if (objHolder != NULL && e != NULL)
    {
        unsigned int i;
        for (i = 0; i < mReferences.size(); ++i)
        {
            if (mReferences[i] == objHolder) break;
        }
        if (i != mReferences.size())
        {
            e->delReferencee(objHolder);

			mReferences.erase(mReferences.begin() + i);
            mRefNames.erase(mRefNames.begin() + i);
        }
    }
}

/*!
    @brief register the referenced Enity into ReferencePool, and get the Object(Holder)
*/
AQLObjectHolder*
AQLDataMultiReference::getReferencee(const AQLString& name, AQLObject* e)
{
    AQLObjectHolder* ret=NULL;
    if (e != NULL)
    {
        AQLDataInstance* dp = e->getDataInstance();
        if (dp != NULL)
        {
            AQLCoreReferencePool& rp = dp->getReferencePool();
            ret = &(rp.setReference(name));
        }
    }
    return ret;
}

/*!
    @brief  set member functions(mReferencees) and also set referenced Object to the Data member function based on mRefNames
*/
void
AQLDataMultiReference::setReferencer(void)
{
    // must be cleared
	if (mReferences.size() != 0) return;
    AQLObject* e = getObject();
    // is this data a part of Object?
    if (e != NULL)
    {
        //AQLCoreError ex("MultiReference Error", __FILE__,__LINE__);
        AQLCoreError ex;
	bool err = false;
        vector<AQLString>::iterator it;
        for (it=mRefNames.begin(); it!=mRefNames.end(); ++it)
        {
            try 
            {
                setReferencee(getReferencee(*it, e), e);
            }
            catch(AQLCoreError& er)
            {
                err = true;
                ex += er;
                unsigned int i = static_cast<unsigned int>(it - mRefNames.begin());
                mRefNames.erase(it);
                if (i == mRefNames.size()) break;
	
				it = mRefNames.begin() + i;

            }
        }
        if (err) throw ex;
    }
}

/*!
    @brief remove all entitites from referenced list
*/
void
AQLDataMultiReference::delReferencer(void)
{
    AQLObject* e = getObject();  // original Object
    // is this data a part of Object?
    if (e != NULL)
    {
        vector<AQLObjectHolder*>::iterator it;
        for (it=mReferences.begin(); it!=mReferences.end(); ++it)
        {
            if (*it != NULL) e->delReferencee(*it); // delete the referenced Object of the original Object
        }
    }
    // delete the referenced Object of this class
    mReferences.clear();
    mRefNames.clear();
}

/*!
    @brief default constructor
*/
AQLDataMultiReference::AQLDataMultiReference(void) : AQLPriceDataType(DATA_MULTIREFERENCE), 
    mReferences(), mRefNames()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLDataMultiReference::AQLDataMultiReference(const AQLDataMultiReference& attr) 
                    : AQLPriceDataType(DATA_MULTIREFERENCE), mReferences()
{
    mRefNames = attr.mRefNames;
    setNull(attr.isNull());
}
    
/*!
    @brief destructor
*/
AQLDataMultiReference::~AQLDataMultiReference(void)
{
    delReferencer(); //delete all Reference
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*
AQLDataMultiReference::clone() const
{
    try {
        AQLDataMultiReference* pAttr = new AQLDataMultiReference(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief return string representation(separated by a delimiter string referenced by multiple Object name)

    @return string separated by a delimiter of multiple Object name
*/
AQLString      
AQLDataMultiReference::convertToString(void) const
{
    if (isNull()) return NULL_STR;

    AQLString ret("\"");
    ret += mRefNames[0];
    for (unsigned int i = 1; i < mRefNames.size(); ++i)
    {
        ret += DATA_COLL_DEL;
        ret += mRefNames[i];
    }
    return ret + "\"";
}

/*!
    @brief set the reference from string representation(separated by a delimiter string referenced by multiple Object name)

    @param[in] str string separated by a delimiter of multiple Object name
*/
void          
AQLDataMultiReference::convertFromString(const AQLString& str)
{
    AQLString data;
    bool ret = strToData(str, data);

    // update
    update();
    // delelete previous reference
    delReferencer();

    if (ret || data == "")
    {
        setNull();
        mRefNames.clear();
        mReferences.clear();
    }
    else
    {
        setNull(false);
        // set by token the arguments
        mRefNames = data.toToken(DATA_COLL_DEL);
        // construct dependency
        setReferencer();
    }
}

/*!
    @brief add the referenced Object

    not hold ownership

    @param[in] e  the referenced Object Holder
*/
void
AQLDataMultiReference::add(AQLObjectHolder *e)
{
    if (e != NULL)
    {
        update();
        mRefNames.push_back(e->getName());
        try 
        {
            setReferencee(e, getObject());
        }
        catch (...)
        {
			mRefNames.erase( mRefNames.end() );
            throw;
        }
        setNull(false);
    }
}

/*!
    @brief remove the Object specified from the referenced Object

    @param[in] name the referenced Object to be removed
*/
void
AQLDataMultiReference::remove(const AQLString& name)
{
    try {
        unsigned int i;
        for (i=0; i < mRefNames.size(); ++i)
        {
            if (mRefNames[i] == name) break;
        }
        if (i == mRefNames.size()) return;
        // update
        update();

        if (mReferences.size() > i)
        {
            // update dependency
            delReferencee(mReferences[i], getObject());
        }
        else
        {
            mRefNames.erase(mRefNames.begin() + i);
        }
    }
    catch(...)
    {
        ;
    }
}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
AQLPriceDataType&
AQLDataMultiReference::assignment(const AQLPriceDataType& a)
{
    // not copy when a == this
    if (this == &a) return *this;

    // is the same type?
    if (a.getType() != DATA_MULTIREFERENCE) 
    {
        AQLString err = "Assignment error for AQLDataMultiReference : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    // update
    update();
    // delelete previous reference
    delReferencer();
    mRefNames = dynamic_cast<const AQLDataMultiReference&>(a).mRefNames;
    setNull(a.isNull());
    if (! isNull())
    {
        setReferencer();
    }
    return *this;
}

/*!
    @brief  compare the contents against the other objecet

    @param[in] a AQLDataMultiReference object to be compared

    @return (Object reference number of the Data) - (Object reference number of Data to be compared)
*/
int
AQLDataMultiReference::compare(const AQLPriceDataType& a) const
{
    if (a.getType() != DATA_MULTIREFERENCE) 
    {
        AQLString err = "Compare error for AQLDataMultiReference : from ";
        err += AQLString(a.getType());
        throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    return mReferences.size() -
            dynamic_cast<const AQLDataMultiReference&>(a).mReferences.size();
}

/*!
    @brief set a pointer to the Holder of the Data

    able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void
AQLDataMultiReference::setHolder(AQLDataHolder* holder)
{
    AQLPriceDataType::setHolder(holder);
    setReferencer();
}

/*!
    @brief inform the Object Data that the Data has been updated


	It is informed (to original Object) that TYPE_BROADCAST type is updated(= the referenced Object is updated)
*/
void
AQLDataMultiReference::update(void)
{
#ifndef USE_MODEL_VERSIONING
    AQLPriceDataType::update(TYPE_BROADCAST);
#else
    AQLPriceDataType::update(TYPE_NORMAL);
#endif
}
