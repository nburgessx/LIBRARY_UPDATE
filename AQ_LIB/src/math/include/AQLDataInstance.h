/*! @file
    @brief Class declaration to manage and hold Object, Data and Function.
*/


#ifndef AQLDataInstance_h
#define AQLDataInstance_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLObjectPool.h"
#include "AQLObjectMaster.h"
#include <memory>

class AQLPriceDataManager;
class AQLFunctionManager;

/*!
    @brief Class to manage and hold Object, Data and Function.

     Object references between objects will be built under this class.
*/
class AQLDataInstance
{
public:
    // default constructor
	AQLDataInstance(void);

    // Not copyable. This used to be a shallow copy sharing mpDataMstr/mpFunctionMstr behind a
    // hand-rolled, non-atomic int* refcount (clear()/copy() bare ++/--, no synchronization at all -
    // a real race if two instances sharing the count were ever destructed/copied concurrently).
    // Verified across etrading/validation/calibration/models (a full-repo grep for by-value use):
    // AQLDataInstance is passed by pointer or reference at 1000+ call sites and never genuinely
    // copied anywhere - the one non-reference/pointer hit is `new AQLDataInstance()`, not a copy.
    // So the refcount machinery was protecting a code path nothing exercises. Deleting these two
    // instead of fixing them removes the risk entirely rather than making it merely safe.
    AQLDataInstance(const AQLDataInstance& dataInstance) = delete;
    AQLDataInstance& operator=(const AQLDataInstance& r) = delete;

    // destructor
	~AQLDataInstance(void);

    // return Data Master
    /*!
        @return Data Master
    */
	AQLPriceDataManager&		getDataMaster() {return *mpDataMstr;}
    
    // return Object Master
    /*!
        @return Object Master
    */
	AQLObjectMaster&			getObjectMaster() {return mObjectMstr;}

    // return Function Master
    /*!
        @return Function Master
    */
	AQLFunctionManager&		getFunctionMaster() {return *mpFunctionMstr;}

    // return Object Pool
    /*!
        @return Object Pool
    */
	AQLObjectPool&			getObjectPool() {return mObjectPool;}

    // return Reference Pool
    /*!
        @return Reference Pool
    */
	AQLCoreReferencePool&		getReferencePool() {return mObjectPool.mReferencePool;}
	
	// serialize object pool
	void					serialize(char*& ptr, unsigned long& length) const;
	// load object pool data
	void					load(char* ptr);
	// update object pool
	void					update(char* ptr);

private:

	// Sole owners now (no more shared refcount) - unique_ptr so the destructor and this header's
	// forward declarations of AQLPriceDataManager/AQLFunctionManager still work (the actual delete
	// happens in the .cpp's out-of-line destructor, where both types are complete).
	std::unique_ptr<AQLPriceDataManager>	mpDataMstr;		// Data Master
	std::unique_ptr<AQLFunctionManager>	mpFunctionMstr;	// Function Master

	AQLObjectMaster				mObjectMstr;	// Object Master
	AQLObjectPool				mObjectPool;	// Object Pool
};
#endif
