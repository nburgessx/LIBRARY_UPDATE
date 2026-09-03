#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLObjectHolder.h"
#include "AQLCoreReferencePool.h"

#include <map>

class AQLDataInstance;

// check status in get method to get Object
enum EntityCheckType {
	ENCHKTYPE_NOCHECK,  // no check
	ENCHKTYPE_ISDEFINED// check if exists or not, and throw an exception in case of no exist
};

/*! 
    @brief Class to register and master the Object to build the reference relationship by name.   

    Since the same Object is unique, registration for Object Pool is demonstrated 
	in the pointer, and this class holds the ownership of the pointer.
	The pointer must be a region of memory reserved by new.

	This class is controlled from AQLDataInstance and impossible to be genarated from class except AQLDataInstance.
*/
class AQLObjectPool
{
public:
//  QUERY
    // return the number of registered Entities
    /*!
        @return the number of registered Entities
    */
	int                 getSize() const {return mCollection.size();}

    // get Object Holder from the Object registered in the EntityPool
	AQLObjectHolder		getObject(const AQLString& name, EntityCheckType type = ENCHKTYPE_NOCHECK);

    // return an iterator pointing to the top of the Object Holder map belonging to the EntityPool
    /*!
        @return an iterator pointing to the top of the Object Holder map
    */
	EntityIter          begin(void) {return mCollection.begin();}
	// return an iterator pointing to the end of the Object Holder map belonging to the EntityPool
    /*!
        @return an iterator pointing to the end of the Object Holder map
    */
	EntityIter          end(void) {return mCollection.end();}
	// return a constant iterator pointing to the top of the Object Holder map belonging to the EntityPool
    /*!
        @return a constant iterator pointing to the top of the Object Holder map
    */
	EntityConstIter     begin(void) const {return mCollection.begin();}
	// return a constant iterator pointing to the end of the Object Holder map belonging to the EntityPool
    /*!
        @return a constant iterator pointing to the end of the Object Holder map
    */
	EntityConstIter     end(void) const {return mCollection.end();}

	// check whether object exists in mCollection
	/*!
        @return true if it exists
    */
	bool                find(const AQLString& name);

	// return the full list of data names from object pool
	AQLStringVector		getNames() const;

//  OPERATION

    // set the Object to the Object Pool
	void				set(const AQLString& name, AQLObject* e);

    // delete Object from the Object Pool
	void                remove(const AQLString& name);

	// delete Object from the Object Pool whether it's in reference pool or not
	void				removeNoCheck(const AQLString& name);

	// clear all of reference relationship
	void				clear(void);

private:
	EntityPool			mCollection;	// registered Object(map data of Object name and Object Holder)
	AQLDataInstance*				mpDataInstance;			// AQLDataInstance object this EntityPool holds
	AQLCoreReferencePool		mReferencePool; // Reference Pool(master the Referenced object pool)
	// For data retention
	// let DeleteFlag of AQLObjectHolder set by Flase
	// free of memory by setting the flag true( remove or destractor)

	// impossible to be generated from class excep AQLDataInstance
friend class AQLDataInstance;
//  LIFECYCLE
    // default constructor
	AQLObjectPool(void);
    // destructor
	~AQLObjectPool(void);

    // assignment operator
	AQLObjectPool&  operator=(const AQLObjectPool& objPool) {return copy(objPool);}

    // shallow copy of the specified EntityPool
	AQLObjectPool&  copy(const AQLObjectPool& objPool);

    // set AQLDataInstance object
	void setDataInstance(AQLDataInstance* dataInstance);

	AQLObjectHolder NULL_OBJECT_HOLDER;
};
