/*! @file
    @brief Class declaration to manage and hold Object, Data and Function.
*/


#ifndef LADataInstance_h
#define LADataInstance_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAObjectPool.h"
#include "LAObjectMaster.h"

class LAPriceDataManager;
class LAFunctionManager;

/*! 
    @brief Class to manage and hold Object, Data and Function.

     Object references between objects will be built under this class.
*/
class LADataInstance
{
public:
//	LIFECYCLE
    // default constructor
	LADataInstance(void);
    // copy constructor
	LADataInstance(const LADataInstance& dataInstance);
    // destructor
	~LADataInstance(void);

//  QUERY
    // return Data Master
    /*!
        @return Data Master
    */
	LAPriceDataManager&		getDataMaster() {return *mpDataMstr;}
    
    // return Object Master
    /*!
        @return Object Master
    */
	LAObjectMaster&			getObjectMaster() {return mObjectMstr;}

    // return Function Master
    /*!
        @return Function Master
    */
	LAFunctionManager&		getFunctionMaster() {return *mpFunctionMstr;}

    // return Object Pool
    /*!
        @return Object Pool
    */
	LAObjectPool&			getObjectPool() {return mObjectPool;}

    // return Reference Pool
    /*!
        @return Reference Pool
    */
	LACoreReferencePool&		getReferencePool() {return mObjectPool.mReferencePool;}
	
//  OPERATOR
    // assignment operator
	LADataInstance&					operator=(const LADataInstance& r) {return copy(r);}

	// serialize object pool
	void					serialize(char*& ptr, unsigned long& length) const;
	// load object pool data
	void					load(char* ptr);
	// update object pool
	void					update(char* ptr);

private:
    // shallow copy of the object
	LADataInstance&					copy(const LADataInstance&);
    // release the memory reserved for the Data Master, Function Master
    void					delMstrs(void);

	LAPriceDataManager*		mpDataMstr;		// Data Master
	LAFunctionManager*			mpFunctionMstr;	// Function Master
	int*						mpRefCount;     // Reference counter of Data Master and Function Master (memory management of a pointer variable about the discard the propriety )

	LAObjectMaster				mObjectMstr;	// Object Master
	LAObjectPool				mObjectPool;	// Object Pool
};
#endif
