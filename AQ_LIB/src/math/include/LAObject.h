#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAutoPtr.h"
#include "LAPriceDataType.h"
#include "LADataHolder.h"
#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LACoreTemplateType.h"
#include "LAObject.h"

class LAObjectHolder;
class LADataInstance;

// type of Object ID
#define object_t       int

// LAObject Object ID
#define ENTITY_BASE		1

typedef std::map<LAString, LADataHolder>	EntityBase;         // map class of Data name and Data Holder
typedef std::set<LAObjectHolder*>			EntityHolderPSet;   // set class of pointer to Object Holder
typedef std::set<LAObject*>					EntityPSet;         // set class of pointer to Object

typedef EntityBase::iterator        AttrIter;       // iterator of map class of Data name and Data Holder
typedef EntityBase::const_iterator  AttrConstIter;  // constant iterator of map class of Data name and Data Holder

// abolish the mProcVersion, unify the mVersion
#define USE_MODEL_VERSIONING    // be sure to set effective

// Data update type
// Data update type(in case Data except for LADataReference and LADataMultiReference is updated)
// => support only TYPE_NORMAL due to the abolishment of mProcVersion
// Data update type(support only TYPE_NORMAL)
#define TYPE_NORMAL							0x0001

#ifndef USE_MODEL_VERSIONING
// Data update type(in case Data except for LADataReference and LADataMultiReference is updated)
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

	LAObject provides the ability to update information and control Data.
	The data is versioned when added, deleted, updated.

	In addition registration of a pointer to the Object is also performed for Data
	when LADataInstance is set. Automatic updates are not performed unless the Data
	is derived from the Reference class
*/
class LAObject 
{
public:
	
    // default constructor
    LAObject();

    // copy constructor
    LAObject(const LAObject& c);
    
	// destructor
    virtual ~LAObject(void);

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
    // For methods: LADataValuation::value() and LADataProcedure::calibrateModel()
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
    const LADataHolder&	getData( const LAString& name, AttrCheckType checkType = NOCHECK ) const;
    
	// get Data Holder corresponding to Data with the specified name
    LADataHolder&		getData( const LAString& name, AttrCheckType checkType = NOCHECK );

	// get Data Holder corresponding to Data with the specified name
    const LADataHolder&	getDataNoThrow( const LAString& name, AttrCheckType checkType = NOCHECK ) const;
    
	// get Data Holder corresponding to Data with the specified name
    LADataHolder&		getDataNoThrow( const LAString& name, AttrCheckType checkType = NOCHECK );

	// get Data Holder corresponding to Data with the specified name or alias
    const LADataHolder&	getData( const LAString& name, const LAString& alias, AttrCheckType checkType = NOCHECK ) const;
     
	// get Data Holder corresponding to Data with the specified name or alias
    LADataHolder&		getData(const LAString& name, const LAString& alias, AttrCheckType checkType = NOCHECK );

	// deep copy of this object
	virtual LAObject*	clone(void) const;

    // get LADataInstance object
	LADataInstance*		getDataInstance(void) const {return mpDataInstance;}

	
	// register the Data, and the ownership of the pointer
    LADataHolder&		add(const LAString& name, LAPriceDataType* att);           
	
	// register the Data
    LADataHolder&		add(const LAString& name, const LADataHolder& att);

    // delete Data
    virtual void		remove(const LAString& name);


    // register the Data, and the ownership of the pointer to LADataibugte is in LAObject ( remove then add )
    LADataHolder&		reset(const LAString& name, LAPriceDataType* att);           
	
    // register the Data ( remove then add )
    LADataHolder&		reset(const LAString& name, const LADataHolder& att);


    // called when updating the Data, the number of Version representing number of updates is incremented
	virtual void		update(const unsigned int type=TYPE_NORMAL);

    // clear all Data data
	void				clear(void);
	
	// Initialize this Object
	virtual void		reset(void);

    // assignment operator
	LAObject&			operator=(const LAObject& a) {return copy(a);}

    // output (name and data data) the contents of the Data to the specified file
	void                print(const char_t* file) const;

	// output (name and data data) the contents of the Data to the specified file stream
	
	void                print(FStream& fout) const;
	
	// output (name and data data) the contents of the Data to the specified string object
    void				print(LAString& str) const;

// ========================================================================================
protected:
    
	// shallow copy of the object
	virtual LAObject&	copy(const LAObject& a);

    // set the LADataInstance which belongs to the Object 
	void				setDataInstance(LADataInstance* dataInstance);

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
	void						setReferencee(LAObjectHolder* h);

    // delete Object from the lists of the referenced Object
	void						delReferencee(LAObjectHolder* h);

// ========================================================================================
private:

friend class LAObjectHolder;

    // set the Object Holder
	void				setHolder(LAObjectHolder* holder);

    // check whether or not the circular reference between the selected Object and the Object
	bool				checkDependency(LAObjectHolder* h);
	
#ifndef USE_MODEL_VERSIONING
	int						mProcVersion;	// number of update for the referenced Object(initial is 1)
#endif

	int								mVersion;				// number of update for Data belonging to the Object(initial is 1)
    bool							mIsValuated;			// value flag whether to calulate or not value() and calibrateModel()
	LAObjectHolder*					mpHolder;				// pointer to Object Holder
	EntityBase						mData;					// collection data of Data belonging to the Object
	EntityHolderPSet				mReferencees;			// set of pointer to the referenced Object from this Object
	mutable LADataInstance*			mpDataInstance;			// pointer to the LADataInstance

	// Null Data Holder used when (Holder) has not been set yet Data corresponding to the name of the Data.
	LADataHolder NULL_DATA_HOLDER;
};
