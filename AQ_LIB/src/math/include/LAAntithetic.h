/*! @file
    @brief Declaration of a class for a negative correlation method.
	
*/

//  2005, Fixed Income Group, AlgoQuantHub.


#ifndef LAAntithetic_h
#define LAAntithetic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAAntithetic.h
//
//  SYNOPSIS    :       LAAntithetic
//  DESCRIPTION :       
//                     
//                    
//                      
//  VERSION     :
////X///////////////////X///////////////////////////////X///////////////////


#include "LARandBase.h"


// LAAntithetic Function ID
#define FN_RAND_ANTI     1103 
// LAAntithetic Function Name
#define FN_RAND_ANTI_STR	"fn_rand_anti"




///////////////////////////////////////////////////////////////////////
/*!
    @brief Declaration of a class for a negative correlation method.
*/

class LAAntithetic : public LARandBase
{
public:
	LAAntithetic();
	virtual ~LAAntithetic();
	LAAntithetic(const LAAntithetic& v);

	LAAntithetic & operator=( const LAAntithetic & ) { return *this; }

//  QUERY
								//======================================
                                // function to set the random number generator to apply a negative correlation method
	virtual void				setInnerGenerator(const LARandBase& v);
								//======================================
                                // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;
                                //======================================
                                // deep copy of this object
    virtual LACoreFunctionBase*     clone() const;
                                //======================================
                                // get function type
    virtual function_t          getType() const;
                                //======================================
                                // function to generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates);
								//======================================
								// function to set dimension information(first element should be the number of dimension)
	virtual void                setDim(const UintArray& dimValue);
                                //======================================
                                // function to set seed
	virtual void				setSeed(const UlongArray& seed);

	

private:
	LARandBase*					mpInner;
	bool						isOdd;
	DoubleArray				nextVariates;

};

#endif

