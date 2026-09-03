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

class AQLPriceDataManager;
class AQLFunctionManager;

/*! 
    @brief Class to manage and hold Object, Data and Function.

     Object references between objects will be built under this class.
*/
class AQLDataInstance
{
public:
//	LIFECYCLE
    // default constructor
	AQLDataInstance(void);
    // copy constructor
	AQLDataInstance(const AQLDataInstance& dataInstance);
    // destructor
	~AQLDataInstance(void);

//  QUERY
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
	
//  OPERATOR
    // assignment operator
	AQLDataInstance&					operator=(const AQLDataInstance& r) {return copy(r);}

	// serialize object pool
	void					serialize(char*& ptr, unsigned long& length) const;
	// load object pool data
	void					load(char* ptr);
	// update object pool
	void					update(char* ptr);

private:
    // shallow copy of the object
	AQLDataInstance&					copy(const AQLDataInstance&);
    // release the memory reserved for the Data Master, Function Master
    void					delMstrs(void);

	AQLPriceDataManager*		mpDataMstr;		// Data Master
	AQLFunctionManager*			mpFunctionMstr;	// Function Master
	int*						mpRefCount;     // Reference counter of Data Master and Function Master (memory management of a pointer variable about the discard the propriety )

	AQLObjectMaster				mObjectMstr;	// Object Master
	AQLObjectPool				mObjectPool;	// Object Pool
};
#endif
