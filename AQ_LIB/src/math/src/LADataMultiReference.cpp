/*! @file
    @brief Implementation of data to refer multi entities.
    
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LADataMultiReference.h"
#include "LADataVector.h"
#include "LAObjectHolder.h"
#include "LACoreReferencePool.h"
#include "LAString.h"
#include "LADataInstance.h"
#include <vector>

using namespace std;


/*!
	@brief return the referenced Object selced by index(starting at 0)

	@return referenced Object Holder
*/
LAObjectHolder&
LADataMultiReference::get(int i) const
{
	if( mReferences.size() == 0 )
	{
		//error
		LAString msg = "mReferences.size() = 0 : LADataMultiReference::get( i )";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
	}

	if (!mReferences[i]->isDefined())
	{
		//error
		LAString msg = mRefNames[i];
		msg += " is not set";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	return *(mReferences[i]);
}


/*!
    @brief set the referenced Object Data from the member variables and member variables of the specified Object

    @param[in] objHolder referenced Object Holder to be registered
    @param[in] e  original Object(usually select the Object that this Data belongs to)
*/
void
LADataMultiReference::setReferencee(LAObjectHolder* objHolder, LAObject* e)
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
LADataMultiReference::delReferencee(LAObjectHolder* objHolder, LAObject* e)
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
LAObjectHolder*
LADataMultiReference::getReferencee(const LAString& name, LAObject* e)
{
    LAObjectHolder* ret=NULL;
    if (e != NULL)
    {
        LADataInstance* dp = e->getDataInstance();
        if (dp != NULL)
        {
            LACoreReferencePool& rp = dp->getReferencePool();
            ret = &(rp.setReference(name));
        }
    }
    return ret;
}

/*!
    @brief  set member functions(mReferencees) and also set referenced Object to the Data member function based on mRefNames
*/
void
LADataMultiReference::setReferencer(void)
{
    // must be cleared
	if (mReferences.size() != 0) return;
    LAObject* e = getObject();
    // is this data a part of ObjectÅH
    if (e != NULL)
    {
        //LACoreError ex("MultiReference Error", __FILE__,__LINE__);
        LACoreError ex;
	bool err = false;
        vector<LAString>::iterator it;
        for (it=mRefNames.begin(); it!=mRefNames.end(); ++it)
        {
            try 
            {
                setReferencee(getReferencee(*it, e), e);
            }
            catch(LACoreError& er)
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
LADataMultiReference::delReferencer(void)
{
    LAObject* e = getObject();  // original Object
    // is this data a part of ObjectÅH
    if (e != NULL)
    {
        vector<LAObjectHolder*>::iterator it;
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
LADataMultiReference::LADataMultiReference(void) : LAPriceDataType(DATA_MULTIREFERENCE), 
    mReferences(), mRefNames()
{
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LADataMultiReference::LADataMultiReference(const LADataMultiReference& attr) 
                    : LAPriceDataType(DATA_MULTIREFERENCE), mReferences()
{
    mRefNames = attr.mRefNames;
    setNull(attr.isNull());
}
    
/*!
    @brief destructor
*/
LADataMultiReference::~LADataMultiReference(void)
{
    delReferencer(); //delete all Reference
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
LAPriceDataType*
LADataMultiReference::clone() const
{
    try {
        LADataMultiReference* pAttr = new LADataMultiReference(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief return string representation(separated by a delimiter string referenced by multiple Object name)

    @return string separated by a delimiter of multiple Object name
*/
LAString      
LADataMultiReference::convertToString(void) const
{
    if (isNull()) return NULL_STR;

    LAString ret("\"");
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
LADataMultiReference::convertFromString(const LAString& str)
{
    LAString data;
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
LADataMultiReference::add(LAObjectHolder *e)
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
LADataMultiReference::remove(const LAString& name)
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
LAPriceDataType&
LADataMultiReference::assignment(const LAPriceDataType& a)
{
    // not copy when a == this
    if (this == &a) return *this;

    // is the same typeÅH
    if (a.getType() != DATA_MULTIREFERENCE) 
    {
        LAString err = "Assignment error for LADataMultiReference : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    // update
    update();
    // delelete previous reference
    delReferencer();
    mRefNames = dynamic_cast<const LADataMultiReference&>(a).mRefNames;
    setNull(a.isNull());
    if (! isNull())
    {
        setReferencer();
    }
    return *this;
}

/*!
    @brief  compare the contents against the other objecet

    @param[in] a LADataMultiReference object to be compared

    @return (Object reference number of the Data) Å| (Object reference number of Data to be compared)
*/
int
LADataMultiReference::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_MULTIREFERENCE) 
    {
        LAString err = "Compare error for LADataMultiReference : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }

    return mReferences.size() -
            dynamic_cast<const LADataMultiReference&>(a).mReferences.size();
}

/*!
    @brief set a pointer to the Holder of the Data

    able to clear the relationship between the Data Holder by setting a NULL

    @param[in] holder Data Holder to be set
*/
void
LADataMultiReference::setHolder(LADataHolder* holder)
{
    LAPriceDataType::setHolder(holder);
    setReferencer();
}

/*!
    @brief inform the Object Data that the Data has been updated


	It is informed (to original Object) that TYPE_BROADCAST type is updated(= the referenced Object is updated)
*/
void
LADataMultiReference::update(void)
{
#ifndef USE_MODEL_VERSIONING
    LAPriceDataType::update(TYPE_BROADCAST);
#else
    LAPriceDataType::update(TYPE_NORMAL);
#endif
}
