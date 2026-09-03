// LAObjectHolder.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAObject.h"
#include "LACoreTemplateType.h"
#include <unordered_map>
#include <functional>		// for std::hash

// Forward Declarations
class LADataInstance;
class LAObjectPool;

// LACurveData
// ==========================================================================
typedef std::unordered_map<StandardString, StandardString> CurveDataMap;

// Helper Class to Hold the Curve Collection and Curve Index
// This is used for the key paramter for the LACurveData staticData_ map
class CurveObjectName
{
public:
	StandardString curveCollection_;
	StandardString curveIndex_;

	// Constructor
	CurveObjectName( const StandardString &curveCollection, const StandardString & curveIndex )
		: curveCollection_(curveCollection), curveIndex_(curveIndex) {}

	// Required for Static Data Map Key Comparisons
	bool operator==(const CurveObjectName& c) const
	{
	return curveCollection_ == c.curveCollection_  && curveIndex_ == c.curveIndex_;
	}

	// Required for Static Data Map Key Comparisons
	bool operator<( const CurveObjectName& c ) const
	{
		return ( ( curveCollection_ < c.curveCollection_ ) && ( curveIndex_ < c.curveIndex_ ) );
	}

	// Required for Static Data Map Key Comparisons
	bool operator()( const CurveObjectName & lhs, const CurveObjectName & rhs ) const
	{
		return ( ( lhs.curveCollection_ < rhs.curveCollection_ ) && ( lhs.curveIndex_ < rhs.curveIndex_ ) );
	}
};

// Helper Class to facilitate the CurveObjectName Hashing in the Static Data Map Object
class CurveObjectHash
{
public:
	template <typename T>
	std::size_t operator()(T t) const
	{
		auto h1 = std::hash<std::string>()( t.curveCollection_ );
		auto h2 = std::hash<std::string>()( t.curveIndex_ );
		return h1 ^ h2;
	}
};

/*! 
    @brief Class to hold object classes.

    LAObjectHolder holds pointer to LAObject. Ownership of the pointer is
	controlled from mDeleteFlag. It has the ownership if true, and false otherwise
	(in case of false, memory free should be guaranteed from others.)
	
	This class does not have the ownership of the pointer to LAObject 
	only in case that this class is members of Reference Pool

*/
class LAObjectHolder
{

public:

    // default constructor
	explicit LAObjectHolder(LAObject* e = NULL, bool deleteFlag = true);
    
	// copy constructor
	LAObjectHolder(const LAObjectHolder& e);
    
	// destructor
	~LAObjectHolder();


    // return whether or not that holds the Object
    /*!
        @retval true hold the Object object
        @retval false not hold the Object object
    */
	bool				isDefined() const {return mpObject != NULL;}

	// return the chek flag of the Object object
    /*!
        @retval true hold ownership of the Object
        @retval false not hold ownership of the Object
    */
	bool				isDeleteFlag() const {return mDeleteFlag;}

    // get object If the Object is not defined, the behavior is undetermined.
	const LAObject&     get(void) const {return *mpObject;}
    
	// get object If the Object is not defined, the behavior is undetermined.
	LAObject&			get(void) {return *mpObject;}

    // get Name
	const LAString&		getName() const {return mName;}


	// set the flag which shows the ownership of Object
	void				setDeleteFlag(bool delFlag);

    // set the Object to the Holder
	void				setEntity(LAObject* e, bool deleteFlag = true);

    // set name 
    /*!
        @param[in] name Holder name
    */
	void				setName(const LAString& name) {mName = name;}

    
	// assignment operator (behavior is undetermined when objectHolder is not defined)
	LAObjectHolder&     operator=(const LAObjectHolder& objectHolder) { return copy(objectHolder); }

    // relational operator thorough magnitude relationship of EntityID
	bool                operator==(const LAObjectHolder& objectHolder) const;

    // relational operator thorough magnitude relationship of EntityID
	bool                operator < (const LAObjectHolder& objectHolder) const;

	
	// Accessor Methods
	// If the Object is not defined, the behavior is undetermined.
	// =============================================================================

    // get a number of inremental updates for the data object
	int			        getModel() const { return mpObject->getModel(); }

#ifndef USE_MODEL_VERSIONING
    // get a number of update for Object that this Object referes
    /*!
        @return a number of update for the referenced Object
    */
	int			        getModelVersion() const 
							{return mpObject->getModelVersion();}
#endif

    // return Object ID this class holds
    object_t            getType(void) const {return mpObject->getType();}

    // find out whether the object or the inherited object matches a specified object id or not
    bool	            isTypeOf(object_t id) const {return mpObject->isTypeOf(id);}

    // return the number of Data belonging to the Object
	int                 getSize() const	{return mpObject->getSize();}

	// return an iterator pointing to the top of the Data map belonging to the Object
	AttrIter			begin(void) {return mpObject->begin();}
	
	// return an iterator pointing to the end of the Data map belonging to the Object
	AttrIter			end(void) {return mpObject->end();}
	
	// return a constant iterator pointing to the top of the Data map belonging to the Object
    /*!
        @return a constant iterator pointing to the top of the Data map
    */
	AttrConstIter		begin(void) const {return mpObject->begin();}
	
	// return a constant iterator pointing to the end of the Data map belonging to the Object
    /*!
        @return a constant iterator pointing to the end of the Data map
    */
	AttrConstIter		end(void) const {return mpObject->end();}

    // get Data Holder corresponding to Data with the specified name
	const LADataHolder& getData( const LAString& dataName, AttrCheckType type = NOCHECK ) const
	{
		return mpObject->getData(dataName, type);
	}
    
	// get Data Holder corresponding to Data with the specified name
	LADataHolder&		getData( const LAString& dataName, AttrCheckType type = NOCHECK )
	{
		return mpObject->getData(dataName, type);
	}

	// get Data Holder corresponding to Data with the specified name or alias
	const LADataHolder& getData( const LAString& dataName, const LAString& alias, AttrCheckType type = NOCHECK ) const
	{
		// try get the dataHolder for dataName... then try the alias if that fails
		if ( mpObject->getData(dataName, type).isNull() )
		{
			return mpObject->getData(alias, type);
		}
		return mpObject->getData(dataName, type);
	}
     
	// get Data Holder corresponding to Data with the specified name or alias
	LADataHolder&		getData( const LAString& dataName, const LAString& alias, AttrCheckType type = NOCHECK )
	{
		// try get the dataHolder for dataName... then try the alias if that fails
		if ( mpObject->getData(dataName, type).isNull() )
		{
			return mpObject->getData(alias, type);
		}
		return mpObject->getData(dataName, type);
	}

	// deep copy of this object
	LAObject*			clone(void) const
	{
		return mpObject->clone();
	}

    // get LADataInstance object
	LADataInstance*				getDataInstance(void) const
	{
		return mpDataInstance;
	}

	// register Data into the Object the Holder has, and ownershp of the pointer to LAPriceDataType is in LAObject
    LADataHolder&       add(const LAString& key, LAPriceDataType* att) 
	{
		return mpObject->add(key, att);
	}

	// register Data into the Object the Holder has
    LADataHolder&       add(const LAString& key, const LADataHolder& att) 
	{
		return mpObject->add(key, att);
	}

    // delete Object specified from the Object Master
    void		        remove(const LAString& key)
	{
		mpObject->remove(key);
	}

    // register Data (remove then add) into the Object the Holder, and ownershp of the pointer to LAPriceDataType is in LAObject
    LADataHolder&       reset(const LAString& name, LAPriceDataType* att)
    {
        remove(name);
        return add(name, att);
    }

	// register Data  (remove then add) into the Object the Holder
    LADataHolder&       reset(const LAString& name, const LADataHolder& att)
    {
        remove(name);
        return add(name, att);
    }

    // called when updating the Data, the number of Version representing number of updates is incremented
    // return update type(TYPE_NORMAL or TYPE_BROADCAST)
	void				update(int type=TYPE_NORMAL) {mpObject->update(type);}

	// clear object
    void				clear(void) {mpObject->clear();}

	// print object
	void				print(const char_t* file) const {mpObject->print(file);}
	void				print(FStream& fout) const		{mpObject->print(fout);}
	void				print(LAString& str) const		{mpObject->print(str);}

private:

	friend class LAObjectPool;
	friend class LAObjectMaster;
	friend class CurveObjectName;

    // set the LADataInstance object that belongs to the Holder
    void				setDataInstance(LADataInstance* dataInstance);

    // shallow copy of the specified Object Holder object
	LAObjectHolder&		copy(const LAObjectHolder& e);

    // release the memory
	void                destroy();

	// specific Methods for reference relationship
	// called from LACoreReferencePool
	friend class LACoreReferencePool;

    // inform other Holders(LAObject*) that this Holder has been set in Reference Pool(registration Pool of the referenced Object)
    void				setReference(bool set=true);

    // shallow copy of the specified Object Holder object without broadcast
	void				copyWithoutUpdate(const LAObjectHolder& e);

	// called from LAObject
	friend class LAObject;

public:
    // return the number of Entities which refer the Object
    /*!
        @return 
    */
	const EntityPSet&	getReferencer(void) const {return mReferencers;}

private:

	// inform the original Object that the Object is changed
	void				broadcast(const unsigned int type=TYPE_NORMAL);

    // register the original Object into the Object the Holder has
	void				registerReferencer(LAObject* att);

	// remove the registration of the original Object for this Object
	void				removeReferencer(LAObject* att);


	LAObject*			mpObject;		// pointer to the Object
	LADataInstance*		mpDataInstance; // pointer to the LADataInstance ojbect the Holder belongs to
	LAString			mName;			// name of the Holder
	EntityPSet  		mReferencers;	// set of pointer to the Object refering to the Holder
	bool                mDeleteFlag;	// flag of ownership of Object(is it ok to free up space in the class of such LAObject?)
	bool				mIsInPool;      // flag whether to be registered in the Reference Pool

	// Null Data Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
	LADataHolder NULL_DATA_HOLDER;

};

