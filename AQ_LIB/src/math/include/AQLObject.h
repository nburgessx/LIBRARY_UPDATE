#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAutoPtr.h"
#include "AQLPriceDataType.h"
#include "AQLDataHolder.h"
#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreTemplateType.h"
#include "AQLObject.h"

class AQLObjectHolder;
class AQLDataInstance;

// type of Object ID
#define object_t       int

// AQLObject Object ID
#define ENTITY_BASE		1

typedef std::map<AQLString, AQLDataHolder>	EntityBase;         // map class of Data name and Data Holder
typedef std::set<AQLObjectHolder*>			EntityHolderPSet;   // set class of pointer to Object Holder
typedef std::set<AQLObject*>					EntityPSet;         // set class of pointer to Object

typedef EntityBase::iterator        AttrIter;       // iterator of map class of Data name and Data Holder
typedef EntityBase::const_iterator  AttrConstIter;  // constant iterator of map class of Data name and Data Holder

// abolish the mProcVersion, unify the mVersion
#define USE_MODEL_VERSIONING    // be sure to set effective

// Data update type
// Data update type(in case Data except for AQLDataReference and AQLDataMultiReference is updated)
// => support only TYPE_NORMAL due to the abolishment of mProcVersion
// Data update type(support only TYPE_NORMAL)
#define TYPE_NORMAL							0x0001

#ifndef USE_MODEL_VERSIONING
// Data update type(in case Data except for AQLDataReference and AQLDataMultiReference is updated)
#define TYPE_BROADCAST						0x0002
#endif

// check status in getData method to get Data
enum AttrCheckType
{
	NOCHECK,	// no check
	ISDEFINED,	// check if defined or not, and throw an exception in case of not defined
	ISNOTNULL	// check if null or not, and throw an exception in case of null
};

/*! 
    @brief Class to hold the Data collection.

	AQLObject provides the ability to update information and control Data.
	The data is versioned when added, deleted, updated.

	In addition registration of a pointer to the Object is also performed for Data
	when AQLDataInstance is set. Automatic updates are not performed unless the Data
	is derived from the Reference class
*/
class AQLObject 
{
public:
	
    // default constructor
    AQLObject();

    // copy constructor
    AQLObject(const AQLObject& c);
    
	// destructor
    virtual ~AQLObject(void);

	// get a number of update for the Data taht the Object has
    /*!
        @return a number of update for the Data
    */
	int					getModel(void) const {return mVersion;}

#ifndef USE_MODEL_VERSIONING
    // get a number of update for Object that this Object referes
    /*!
        @return a number of update for Object that this Object referes
    */
	int					getModelVersion(void) const {return mProcVersion;}
#endif

    // return object id
	virtual object_t	getType(void) const;

	// find out whether the object or the inherited object matches a specified object id or not
	virtual bool		isTypeOf(object_t id) const;

    // get the number of Data items in the Object
	size_t				getSize(void) const {return mData.size();}

    // disable calculations when the calibrateModel() method is called
    /*!
        if we do not want to re-evaluated data, then we call this function. Once the
		valuation flag is set within setNoValuation() calculations are disabled and
		the setValuation() method is not triggered.
    */
    void				setNoValuation(void) { mIsValuated = false; };

    // enable calculations when the value() method is called
    void                setValuation(void)	{ mIsValuated = true; };

    // Determine whether to re-calculate or not when using value() or calibrateModel()
    // For methods: AQLDataValuation::value() and AQLDataProcedure::calibrateModel()
    bool                IsValuated(void)	{ return mIsValuated; };

	// return an iterator pointing to the top of the Data map belonging to the Object
	AttrIter			begin(void)			{return mData.begin();}

	// return an iterator pointing to the end of the Data map belonging to the Object
	AttrIter			end(void)			{return mData.end();}

	// return a constant iterator pointing to the top of the Data map belonging to the Object
	AttrConstIter		begin(void) const	{return mData.begin();}

	// return a constant iterator pointing to the end of the Data map belonging to the Object
	AttrConstIter		end(void) const		{return mData.end();}

    // get Data Holder corresponding to Data with the specified name
    const AQLDataHolder&	getData( const AQLString& name, AttrCheckType checkType = NOCHECK ) const;
    
	// get Data Holder corresponding to Data with the specified name
    AQLDataHolder&		getData( const AQLString& name, AttrCheckType checkType = NOCHECK );

	// get Data Holder corresponding to Data with the specified name
    const AQLDataHolder&	getDataNoThrow( const AQLString& name, AttrCheckType checkType = NOCHECK ) const;
    
	// get Data Holder corresponding to Data with the specified name
    AQLDataHolder&		getDataNoThrow( const AQLString& name, AttrCheckType checkType = NOCHECK );

	// get Data Holder corresponding to Data with the specified name or alias
    const AQLDataHolder&	getData( const AQLString& name, const AQLString& alias, AttrCheckType checkType = NOCHECK ) const;
     
	// get Data Holder corresponding to Data with the specified name or alias
    AQLDataHolder&		getData(const AQLString& name, const AQLString& alias, AttrCheckType checkType = NOCHECK );

	// deep copy of this object
	virtual AQLObject*	clone(void) const;

    // get AQLDataInstance object
	AQLDataInstance*		getDataInstance(void) const {return mpDataInstance;}

	
	// register the Data, and the ownership of the pointer
    AQLDataHolder&		add(const AQLString& name, AQLPriceDataType* att);           
	
	// register the Data
    AQLDataHolder&		add(const AQLString& name, const AQLDataHolder& att);

    // delete Data
    virtual void		remove(const AQLString& name);


    // register the Data, and the ownership of the pointer to LADataibugte is in AQLObject ( remove then add )
    AQLDataHolder&		reset(const AQLString& name, AQLPriceDataType* att);           
	
    // register the Data ( remove then add )
    AQLDataHolder&		reset(const AQLString& name, const AQLDataHolder& att);


    // called when updating the Data, the number of Version representing number of updates is incremented
	virtual void		update(const unsigned int type=TYPE_NORMAL);

    // clear all Data data
	void				clear(void);
	
	// Initialize this Object
	virtual void		reset(void);

    // assignment operator
	AQLObject&			operator=(const AQLObject& a) {return copy(a);}

    // output (name and data data) the contents of the Data to the specified file
	void                print(const char_t* file) const;

	// output (name and data data) the contents of the Data to the specified file stream
	
	void                print(FStream& fout) const;
	
	// output (name and data data) the contents of the Data to the specified string object
    void				print(AQLString& str) const;

// ========================================================================================
protected:
    
	// shallow copy of the object
	virtual AQLObject&	copy(const AQLObject& a);

    // set the AQLDataInstance which belongs to the Object 
	void				setDataInstance(AQLDataInstance* dataInstance);

// ========================================================================================
private:
    
	// set a pointer to all of the Holders of the Data
	inline	void		setToAttributes(void);

// ========================================================================================
//				Reference Management
// ========================================================================================
public:

    // get a list of the Object, and it is intended to be called from the Data class
	const EntityHolderPSet&		getReferencees(void) const {return mReferencees;}

    // set the referenced Object of the Object 
	void						setReferencee(AQLObjectHolder* h);

    // delete Object from the lists of the referenced Object
	void						delReferencee(AQLObjectHolder* h);

// ========================================================================================
private:

friend class AQLObjectHolder;

    // set the Object Holder
	void				setHolder(AQLObjectHolder* holder);

    // check whether or not the circular reference between the selected Object and the Object
	bool				checkDependency(AQLObjectHolder* h);
	
#ifndef USE_MODEL_VERSIONING
	int						mProcVersion;	// number of update for the referenced Object(initial is 1)
#endif

	int								mVersion;				// number of update for Data belonging to the Object(initial is 1)
    bool							mIsValuated;			// value flag whether to calulate or not value() and calibrateModel()
	AQLObjectHolder*					mpHolder;				// pointer to Object Holder
	EntityBase						mData;					// collection data of Data belonging to the Object
	EntityHolderPSet				mReferencees;			// set of pointer to the referenced Object from this Object
	mutable AQLDataInstance*			mpDataInstance;			// pointer to the AQLDataInstance

	// Null Data Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
	AQLDataHolder NULL_DATA_HOLDER;
};
