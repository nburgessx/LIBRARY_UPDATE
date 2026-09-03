// AQLObject.cpp

#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLObject.h"
#include "AQLDataHolder.h"
#include "AQLObjectHolder.h"
#include "AQLCoreReferencePool.h"
#include "AQLDataInstance.h"
#include "ExceptionMacros.h"	// etrading exception macros e.g. AQ_REQUIRE

using namespace std;

/*!
    @brief default constructor
*/
AQLObject::AQLObject() : 

#ifndef USE_MODEL_VERSIONING
    mProcVersion(1),
#endif

	mVersion(1),
    mIsValuated(true),
    mpHolder(NULL),
    mData(),
    mpDataInstance(NULL)
{ 
}

/*!
    @brief copy constructor

	No copy of AQLDataInstance information that this Object belongs to. It is noticed when
	AQLDataInstance is decided by setDataInstance(). Until it, no Update is noticed from Data

    @param[in] c original object
*/
AQLObject::AQLObject(const AQLObject& c) : 

#ifndef USE_MODEL_VERSIONING
    mProcVersion(c.mProcVersion),
#endif

	mVersion(c.mVersion),
    mIsValuated(c.mIsValuated),
    mpHolder(NULL), 
    mData(c.mData), 
    mpDataInstance(NULL)
{   
	// not succeed in DataInstance information when copy
    // contact the Data at the time that DataInstance is fixed
    // Until it, no Update is noticed from Data
}

/*!
    @brief destructor
*/
AQLObject::~AQLObject(void) 
{
    mData.clear(); // clear dependency
}

/*!
    @brief return Object ID of this class

    @return Object ID
*/
object_t    
AQLObject::getType(void) const
{
    return ENTITY_BASE;
}

/*!
    @brief find out whether the object or the inherited object matches a specified object id or not

    @param[in] id Object ID

    @retval true the selected Object ID is the same with that of this Object
    @retval false otherwise
*/
bool
AQLObject::isTypeOf(object_t id) const
{
    return (id == ENTITY_BASE);
}

/*!
    @brief get Data Holder corresponding to Data with the specified name

    if you do not set the Data, return the temporary Holder

    @param[in] name Data name 
    @param[in] type check type

    @return Data Holder
*/
const AQLDataHolder&
AQLObject::getData( const AQLString& name, AttrCheckType checkType ) const
{
    AttrConstIter it = mData.find(name);
    if (it == mData.end())
    {
		if(checkType == ISDEFINED || checkType == ISNOTNULL) 
		{
			AQ_THROW( "Missing Data: " + name + " does not exist" )
		}
        return NULL_DATA_HOLDER;
    }
    if(checkType == ISNOTNULL && it->second.isNull())
    {
        AQ_THROW( "Missing Data: " + name + " does not exist" )
    }
    return it->second;
} 

/*!
    @brief get Data Holder corresponding to Data with the specified name

    if you do not set the Data, return the temporary Holder

    @param[in] name Data name
    @param[in] type check type

    @return Data Holder
*/
AQLDataHolder&
AQLObject::getData( const AQLString& name, AttrCheckType checkType )
{
    AttrIter it = mData.find(name);
    if (it == mData.end())
    {
        if( checkType == ISDEFINED || checkType == ISNOTNULL )
        {
			AQ_THROW( "Missing Data: " + name + " does not exist" )
        }
        return NULL_DATA_HOLDER;
    }
    if(checkType == ISNOTNULL && it->second.isNull())
    {
		AQ_THROW( "Missing Data: " + name + " does not exist" )			
    }
    return it->second;
}

/*!
    @brief get Data Holder corresponding to Data with the specified name

    if you do not set the Data, return the temporary Holder

    @param[in] name Data name 
    @param[in] type check type

    @return Data Holder
*/
const AQLDataHolder&
AQLObject::getDataNoThrow( const AQLString& name, AttrCheckType checkType ) const
{
    AttrConstIter it = mData.find(name);
    if (it == mData.end())
    {
		return NULL_DATA_HOLDER;
    }
    if(checkType == ISNOTNULL && it->second.isNull())
    {
        return NULL_DATA_HOLDER;
    }
    return it->second;
} 

/*!
    @brief get Data Holder corresponding to Data with the specified name

    if you do not set the Data, return the temporary Holder

    @param[in] name Data name
    @param[in] type check type

    @return Data Holder
*/
AQLDataHolder&
AQLObject::getDataNoThrow( const AQLString& name, AttrCheckType checkType )
{
    AttrIter it = mData.find(name);
    if (it == mData.end())
    {
        return NULL_DATA_HOLDER;
    }
    if(checkType == ISNOTNULL && it->second.isNull())
    {
		return NULL_DATA_HOLDER;			
    }
    return it->second;
}

// get Data Holder corresponding to Data with the specified name or alias
const AQLDataHolder&
AQLObject::getData( const AQLString& name, const AQLString& alias, AttrCheckType checkType ) const
{
	// try get data using the name ... if that fails try the alias
	if ( getDataNoThrow( name, checkType ).isNull() )
	{
		return getData( alias, checkType );
	}
	return getData( name, checkType );
}
    
	// get Data Holder corresponding to Data with the specified name or alias
AQLDataHolder&		
AQLObject::getData( const AQLString& name, const AQLString& alias, AttrCheckType checkType )
{
	// try get data using the name ... if that fails try the alias
	if ( getDataNoThrow( name, checkType ).isNull() )
	{
		return getData( alias, checkType );
	}
	return getData( name, checkType );
}

/*!
    @brief deep copy of this Object

    @return the copied Object
*/
AQLObject*
AQLObject::clone(void) const
{
    return new AQLObject(*this);
}

/*!
    @brief register the Data, and the ownership of the pointer to LADataibugte is in AQLObject

    if the registered Data is selected, an exception occurs

    @param[in] name Data name to register
    @param[in] att  Data to register

    @return Data Holder
*/
AQLDataHolder& AQLObject::add(const AQLString& name, AQLPriceDataType* att)
{
    AQLDataHolder& dh = mData[name];
    
	// Throw if already defined
	if (dh.isDefined())
    {	
        AQLString msg = "Duplicate Key [" + name + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    // Update parent if if DataInstance has been set
    if (mpDataInstance != NULL)
	{ 
        dh.setParent(this);
	}
    
	// set Data to dh - update method is called if already contains data
    dh.setData(att);

	return dh;
} 

/*!
    @brief register the Data

    if the registered Data is selected, an exception occurs \
	an exception also occurs in case we select undefined Data Holder

    @param[in] name Data to register
    @param[in] att  Data Holder to register

    @return Data Holder to be registered
*/
AQLDataHolder& AQLObject::add( const AQLString& name, const AQLDataHolder& att)
{
    // error in case of not defined Attr
    if(!att.isDefined())
    {
        AQLString msg = "Not Defined Data [" + name + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    AQLDataHolder& dh = mData[name];
    if (dh.isDefined())
    {  
		// error if defined
        AQLString msg = "Duplicate Key [" + name + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    // in case of DataInstance has been set
    if (mpDataInstance != NULL)
        dh.setParent(this);
    
	// set Data to dh
    // update is called if it already contacted Data
    dh = att;

    return dh;
} 

/*!
    @brief delete Data

    nothing is done when the selected Data has not been registered

    @param[in] att Attributge name to be deleted
*/
void AQLObject::remove(const AQLString& att)
{
    AttrIter it = mData.find(att);
    if (it == mData.end()) 
    {
        return;
    }
    mData.erase(it);
    // big update when delete
    // register UPDATE

#ifndef USE_MODEL_VERSIONING
    update(TYPE_BROADCAST);
#else
    update(TYPE_NORMAL);
#endif

}

/*!
    @brief register the Data, and the ownership of the pointer to LADataibugte is in AQLObject ( remove then add )

    if the registered Data is selected, an exception occurs

    @param[in] name Data name to register
    @param[in] att  Data to register

    @return Data Holder
*/
AQLDataHolder& AQLObject::reset(const AQLString& name, AQLPriceDataType* att)
{
    remove( name );
    return add( name, att );
} 

/*!
    @brief register the Data ( remove then add )

    if the registered Data is selected, an exception occurs \
	an exception also occurs in case we select undefined Data Holder

    @param[in] name Data to register
    @param[in] att  Data Holder to register

    @return Data Holder to be registered
*/
AQLDataHolder& AQLObject::reset( const AQLString& name, const AQLDataHolder& att )
{
    remove( name );
    return add( name, att );
} 

/*!
    @brief called when updating the Data, the number of Version representing number of updates is incremented

    @param[in] type Data update type(select TYPE_NORMAL)
*/
void AQLObject::update(const unsigned int type) 
{
    ++mVersion;

#ifndef USE_MODEL_VERSIONING
    if ((type & TYPE_BROADCAST) != 0x0000)
    {   // update if TYPE_BROADCAST is set as type
        ++mProcVersion;
    }
#endif

    if (mpHolder != NULL)
    {   
        mpHolder->broadcast(type);
    }
}

/*!
    @brief clear all Data data

    clear all Data lists
*/
void AQLObject::clear(void)
{
    if (mpHolder != NULL)
    {   
		mpHolder->broadcast();
    }
    mData.clear();
    mVersion = 1;
    mIsValuated = true;

#ifndef USE_MODEL_VERSIONING
    mProcVersion = 1;
#endif

}

/*!
    @brief Initialize this Object
*/
void AQLObject::reset(void)
{
	clear();
}

/*!
    @brief set the AQLDataInstance which belongs to the Object

    not hold ownership of AQLDataInstance object

    @param[in] dataInstance pointer to the AQLDataInstance object that this Object belongs to
*/
void AQLObject::setDataInstance(AQLDataInstance* dataInstance) 
{
    mpDataInstance = dataInstance;
	// set this class into Data
    // update is called if it alredy contacted Data
    if (dataInstance != NULL)
    {
        setToAttributes();
    }
}

/*!
    @brief set a pointer to all of the Holders of the Data
*/
void
AQLObject::setToAttributes(void)
{
 
//  AQLObject* e = (mpDataInstance != NULL ? this : NULL);
    AttrIter it = mData.begin();
    for (; it != mData.end(); ++it)
    {
        it->second.setParent(this);
    }
}

/*!
    @brief shallow copy of the object

    @param[in] original AQLObject object

    @return the copied object
*/
AQLObject&
AQLObject::copy(const AQLObject& a)
{
    if (this != &a ) // no copy itself
    {
        
        if (mpHolder != NULL)
        {   
			mpHolder->broadcast();
        }
        mData = a.mData; 
        if (mpDataInstance != NULL)
        {   
			setToAttributes();
        }
#ifndef USE_MODEL_VERSIONING
        mProcVersion = a.mProcVersion;
#endif
        mVersion = a.mVersion;
        mIsValuated = a.mIsValuated;
    }
    return *this;
}

//////////// OTHER ///////////////////////////////////////////////
/*!
    @brief output (name and data data) the contents of the Data to the specified file

    @param[in] file file name
*/
void
AQLObject::print(const char_t*   file) const
{
    fstream fout(file, ios::out | ios::app);
    print(fout);
    fout.close();
}

/*!
    @brief output (name and data data) the contents of the Data to the specified file stream

    @param[in] fout file stream
*/
void
AQLObject::print(FStream& fout) const
{
   
	fout << "object_t" << ",";

	AttrConstIter it;
    for (it = mData.begin(); it != mData.end(); ++it)
    {
        fout << it->first << ",";
    }
    fout << endl;

	fout << getType() << ",";	
	
	for (it = mData.begin(); it != mData.end(); ++it)
    {
        fout << it->second.convertToString() << "," ;
    }
    fout << endl;
}

/*!
    @brief output (name and data data) the contents of the Data to the specified string object

    @param[in] str string object
*/
void
AQLObject::print(AQLString& str) const
{
	str += "object_t";
	AttrConstIter it;
    for (it = mData.begin(); it != mData.end(); ++it)
	{
		str += ",";
		str += it->first;
	}
    str += "\n";
    
	str += AQLString(getType()) ;	
	
	for (it = mData.begin(); it != mData.end(); ++it)
    {
		str += ",";
		str += it->second.convertToString();
    }
    str += "\n";
}


///////////////////////////////////////////////////////////////
// 

/*!
    @brief set the referenced Object

    @param[in] h referenced Object Holder
*/
void                
AQLObject::setReferencee(AQLObjectHolder* h)
{   // set the reference
    // check after added
    pair<EntityHolderPSet::iterator, bool> ret = mReferencees.insert(h);
    if (ret.second)
    {   
        // add holder
        h->registerReferencer(this);
        // checkDependency
        if (checkDependency(mpHolder))
        {
            AQLString msg(mpHolder->mName);
            msg += " : Don't make Circular Reference Relation!!";
            delReferencee(h);
            throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
        }
    }
    else
    {
        // already registered
        ;
    }
}

/*!
    @brief delete Object from the lists of the referenced Object

    @param[in] h referenced Object Holder to be deleted
*/
void                
AQLObject::delReferencee(AQLObjectHolder* h)
{
    EntityHolderPSet::iterator it = mReferencees.find(h);
    if (it != mReferencees.end())
    {
        mReferencees.erase(h);
        h->removeReferencer(this);
        if (h->getReferencer().size() == 0)
        {
            if (mpDataInstance!=NULL)
            {
                mpDataInstance->getReferencePool().delReference(h->getName());
            }
        }
    }
}

/*!
    @brief set the Object Holder

    @param[in] h the Object Holder
*/
void                
AQLObject::setHolder(AQLObjectHolder* holder)
{
    // set Holder for Broadcast
    mpHolder = holder;
    if (checkDependency(mpHolder))
    {
        AQLString msg(mpHolder->mName);
        msg += " : Don't make Circular Reference Relation!!";
        mpHolder = NULL;

        throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
    }
}

/*!
    @brief check whether or not the circular reference between the selected Object and the Object

    @param[in] h Object Holder to be checked

    @retval true  circular reference
    @retval false otherwise
*/
bool
AQLObject::checkDependency(AQLObjectHolder* h)
{ 
    bool ret = false;
#ifdef NO_CHECKDEPENDENCY
	return ret;
#endif
    if (h != NULL) 
    {
        if (mReferencees.size() == 0) return false;

        EntityHolderPSet::iterator hit = mReferencees.find(h);
        if (hit != mReferencees.end())
        { 
			return true;
        }

        hit = mReferencees.begin();
        for (; hit != mReferencees.end(); ++hit)
        {
            AQLObject* e = &((*hit)->get());
            if (e != NULL)
            {
                ret = e->checkDependency(h);
                if (ret) break;
            }
        }
    }
    return ret;
}
