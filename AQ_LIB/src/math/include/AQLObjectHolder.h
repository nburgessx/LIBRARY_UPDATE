// AQLObjectHolder.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLObject.h"
#include "AQLCoreTemplateType.h"
#include <unordered_map>
#include <functional>		// for std::hash

// Forward Declarations
class AQLDataInstance;
class AQLObjectPool;

// AQLCurveData
// ==========================================================================
typedef std::unordered_map<StandardString, StandardString> CurveDataMap;

// Helper Class to Hold the Curve Collection and Curve Index
// This is used for the key paramter for the AQLCurveData staticData_ map
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

    AQLObjectHolder holds pointer to AQLObject. Ownership of the pointer is
	controlled from mDeleteFlag. It has the ownership if true, and false otherwise
	(in case of false, memory free should be guaranteed from others.)
	
	This class does not have the ownership of the pointer to AQLObject 
	only in case that this class is members of Reference Pool

*/
class AQLObjectHolder
{

public:

    // default constructor
	explicit AQLObjectHolder(AQLObject* e = NULL, bool deleteFlag = true);
    
	// copy constructor
	AQLObjectHolder(const AQLObjectHolder& e);
    
	// destructor
	~AQLObjectHolder();


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
	const AQLObject&     get(void) const {return *mpObject;}
    
	// get object If the Object is not defined, the behavior is undetermined.
	AQLObject&			get(void) {return *mpObject;}

    // get Name
	const AQLString&		getName() const {return mName;}


	// set the flag which shows the ownership of Object
	void				setDeleteFlag(bool delFlag);

    // set the Object to the Holder
	void				setEntity(AQLObject* e, bool deleteFlag = true);

    // set name 
    /*!
        @param[in] name Holder name
    */
	void				setName(const AQLString& name) {mName = name;}

    
	// assignment operator (behavior is undetermined when objectHolder is not defined)
	AQLObjectHolder&     operator=(const AQLObjectHolder& objectHolder) { return copy(objectHolder); }

    // relational operator thorough magnitude relationship of EntityID
	bool                operator==(const AQLObjectHolder& objectHolder) const;

    // relational operator thorough magnitude relationship of EntityID
	bool                operator < (const AQLObjectHolder& objectHolder) const;

	
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
	const AQLDataHolder& getData( const AQLString& dataName, AttrCheckType type = NOCHECK ) const
	{
		return mpObject->getData(dataName, type);
	}
    
	// get Data Holder corresponding to Data with the specified name
	AQLDataHolder&		getData( const AQLString& dataName, AttrCheckType type = NOCHECK )
	{
		return mpObject->getData(dataName, type);
	}

	// get Data Holder corresponding to Data with the specified name or alias
	const AQLDataHolder& getData( const AQLString& dataName, const AQLString& alias, AttrCheckType type = NOCHECK ) const
	{
		// try get the dataHolder for dataName... then try the alias if that fails
		if ( mpObject->getData(dataName, type).isNull() )
		{
			return mpObject->getData(alias, type);
		}
		return mpObject->getData(dataName, type);
	}
     
	// get Data Holder corresponding to Data with the specified name or alias
	AQLDataHolder&		getData( const AQLString& dataName, const AQLString& alias, AttrCheckType type = NOCHECK )
	{
		// try get the dataHolder for dataName... then try the alias if that fails
		if ( mpObject->getData(dataName, type).isNull() )
		{
			return mpObject->getData(alias, type);
		}
		return mpObject->getData(dataName, type);
	}

	// deep copy of this object
	AQLObject*			clone(void) const
	{
		return mpObject->clone();
	}

    // get AQLDataInstance object
	AQLDataInstance*				getDataInstance(void) const
	{
		return mpDataInstance;
	}

	// register Data into the Object the Holder has, and ownershp of the pointer to AQLPriceDataType is in AQLObject
    AQLDataHolder&       add(const AQLString& key, AQLPriceDataType* att) 
	{
		return mpObject->add(key, att);
	}

	// register Data into the Object the Holder has
    AQLDataHolder&       add(const AQLString& key, const AQLDataHolder& att) 
	{
		return mpObject->add(key, att);
	}

    // delete Object specified from the Object Master
    void		        remove(const AQLString& key)
	{
		mpObject->remove(key);
	}

    // register Data (remove then add) into the Object the Holder, and ownershp of the pointer to AQLPriceDataType is in AQLObject
    AQLDataHolder&       reset(const AQLString& name, AQLPriceDataType* att)
    {
        remove(name);
        return add(name, att);
    }

	// register Data  (remove then add) into the Object the Holder
    AQLDataHolder&       reset(const AQLString& name, const AQLDataHolder& att)
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
	void				print(AQLString& str) const		{mpObject->print(str);}

private:

	friend class AQLObjectPool;
	friend class AQLObjectMaster;
	friend class CurveObjectName;

    // set the AQLDataInstance object that belongs to the Holder
    void				setDataInstance(AQLDataInstance* dataInstance);

    // shallow copy of the specified Object Holder object
	AQLObjectHolder&		copy(const AQLObjectHolder& e);

    // release the memory
	void                destroy();

	// specific Methods for reference relationship
	// called from AQLCoreReferencePool
	friend class AQLCoreReferencePool;

    // inform other Holders(AQLObject*) that this Holder has been set in Reference Pool(registration Pool of the referenced Object)
    void				setReference(bool set=true);

    // shallow copy of the specified Object Holder object without broadcast
	void				copyWithoutUpdate(const AQLObjectHolder& e);

	// called from AQLObject
	friend class AQLObject;

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
	void				registerReferencer(AQLObject* att);

	// remove the registration of the original Object for this Object
	void				removeReferencer(AQLObject* att);


	AQLObject*			mpObject;		// pointer to the Object
	AQLDataInstance*		mpDataInstance; // pointer to the AQLDataInstance ojbect the Holder belongs to
	AQLString			mName;			// name of the Holder
	EntityPSet  		mReferencers;	// set of pointer to the Object refering to the Holder
	bool                mDeleteFlag;	// flag of ownership of Object(is it ok to free up space in the class of such AQLObject?)
	bool				mIsInPool;      // flag whether to be registered in the Reference Pool

	// Null Data Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
	AQLDataHolder NULL_DATA_HOLDER;

};

