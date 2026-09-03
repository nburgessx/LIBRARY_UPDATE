// LAObject.cpp

#ifdef __GNUG__
#pragma implementation
#endif

#include "LAObject.h"
#include "LADataHolder.h"
#include "LAObjectHolder.h"
#include "LACoreReferencePool.h"
#include "LADataInstance.h"
#include "ExceptionMacros.h"	// etrading exception macros e.g. AQ_REQUIRE

using namespace std;

/*!
    @brief default constructor
*/
LAObject::LAObject() : 

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

	No copy of LADataInstance information that this Object belongs to. It is noticed when
	LADataInstance is decided by setDataInstance(). Until it, no Update is noticed from Data

    @param[in] c original object
*/
LAObject::LAObject(const LAObject& c) : 

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
LAObject::~LAObject(void) 
{
    mData.clear(); // clear dependency
}

/*!
    @brief return Object ID of this class

    @return Object ID
*/
object_t    
LAObject::getType(void) const
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
LAObject::isTypeOf(object_t id) const
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
const LADataHolder&
LAObject::getData( const LAString& name, AttrCheckType checkType ) const
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
LADataHolder&
LAObject::getData( const LAString& name, AttrCheckType checkType )
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
const LADataHolder&
LAObject::getDataNoThrow( const LAString& name, AttrCheckType checkType ) const
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
LADataHolder&
LAObject::getDataNoThrow( const LAString& name, AttrCheckType checkType )
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
const LADataHolder&
LAObject::getData( const LAString& name, const LAString& alias, AttrCheckType checkType ) const
{
	// try get data using the name ... if that fails try the alias
	if ( getDataNoThrow( name, checkType ).isNull() )
	{
		return getData( alias, checkType );
	}
	return getData( name, checkType );
}
    
	// get Data Holder corresponding to Data with the specified name or alias
LADataHolder&		
LAObject::getData( const LAString& name, const LAString& alias, AttrCheckType checkType )
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
LAObject*
LAObject::clone(void) const
{
    return new LAObject(*this);
}

/*!
    @brief register the Data, and the ownership of the pointer to LADataibugte is in LAObject

    if the registered Data is selected, an exception occurs

    @param[in] name Data name to register
    @param[in] att  Data to register

    @return Data Holder
*/
LADataHolder& LAObject::add(const LAString& name, LAPriceDataType* att)
{
    LADataHolder& dh = mData[name];
    
	// Throw if already defined
	if (dh.isDefined())
    {	
        LAString msg = "Duplicate Key [" + name + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
LADataHolder& LAObject::add( const LAString& name, const LADataHolder& att)
{
    // error in case of not defined Attr
    if(!att.isDefined())
    {
        LAString msg = "Not Defined Data [" + name + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }

    LADataHolder& dh = mData[name];
    if (dh.isDefined())
    {  
		// error if defined
        LAString msg = "Duplicate Key [" + name + "]";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
void LAObject::remove(const LAString& att)
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
    @brief register the Data, and the ownership of the pointer to LADataibugte is in LAObject ( remove then add )

    if the registered Data is selected, an exception occurs

    @param[in] name Data name to register
    @param[in] att  Data to register

    @return Data Holder
*/
LADataHolder& LAObject::reset(const LAString& name, LAPriceDataType* att)
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
LADataHolder& LAObject::reset( const LAString& name, const LADataHolder& att )
{
    remove( name );
    return add( name, att );
} 

/*!
    @brief called when updating the Data, the number of Version representing number of updates is incremented

    @param[in] type Data update type(select TYPE_NORMAL)
*/
void LAObject::update(const unsigned int type) 
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
void LAObject::clear(void)
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
void LAObject::reset(void)
{
	clear();
}

/*!
    @brief set the LADataInstance which belongs to the Object

    not hold ownership of LADataInstance object

    @param[in] dataInstance pointer to the LADataInstance object that this Object belongs to
*/
void LAObject::setDataInstance(LADataInstance* dataInstance) 
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
LAObject::setToAttributes(void)
{
 
//  LAObject* e = (mpDataInstance != NULL ? this : NULL);
    AttrIter it = mData.begin();
    for (; it != mData.end(); ++it)
    {
        it->second.setParent(this);
    }
}

/*!
    @brief shallow copy of the object

    @param[in] original LAObject object

    @return the copied object
*/
LAObject&
LAObject::copy(const LAObject& a)
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
LAObject::print(const char_t*   file) const
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
LAObject::print(FStream& fout) const
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
LAObject::print(LAString& str) const
{
	str += "object_t";
	AttrConstIter it;
    for (it = mData.begin(); it != mData.end(); ++it)
	{
		str += ",";
		str += it->first;
	}
    str += "\n";
    
	str += LAString(getType()) ;	
	
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
LAObject::setReferencee(LAObjectHolder* h)
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
            LAString msg(mpHolder->mName);
            msg += " : Don't make Circular Reference Relation!!";
            delReferencee(h);
            throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
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
LAObject::delReferencee(LAObjectHolder* h)
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
LAObject::setHolder(LAObjectHolder* holder)
{
    // set Holder for Broadcast
    mpHolder = holder;
    if (checkDependency(mpHolder))
    {
        LAString msg(mpHolder->mName);
        msg += " : Don't make Circular Reference Relation!!";
        mpHolder = NULL;

        throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
    }
}

/*!
    @brief check whether or not the circular reference between the selected Object and the Object

    @param[in] h Object Holder to be checked

    @retval true  circular reference
    @retval false otherwise
*/
bool
LAObject::checkDependency(LAObjectHolder* h)
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
            LAObject* e = &((*hit)->get());
            if (e != NULL)
            {
                ret = e->checkDependency(h);
                if (ret) break;
            }
        }
    }
    return ret;
}
