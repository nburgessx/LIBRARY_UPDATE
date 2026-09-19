/*! @file
    @brief Declaration of a class for a negative correlation method.
*/



#ifndef AQLAntithetic_h
#define AQLAntithetic_h


#include "AQLRandBase.h"


// AQLAntithetic Function ID
#define FN_RAND_ANTI     1103 
// AQLAntithetic Function Name
#define FN_RAND_ANTI_STR	"fn_rand_anti"




///////////////////////////////////////////////////////////////////////
/*!
    @brief Declaration of a class for a negative correlation method.
*/

class AQLAntithetic : public AQLRandBase
{
public:
	AQLAntithetic();
	virtual ~AQLAntithetic();
	AQLAntithetic(const AQLAntithetic& v);

	AQLAntithetic & operator=( const AQLAntithetic & ) { return *this; }

								//======================================
                                // function to set the random number generator to apply a negative correlation method
	virtual void				setInnerGenerator(const AQLRandBase& v);
								//======================================
                                // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;
                                //======================================
                                // deep copy of this object
    virtual AQLCoreFunctionBase*     clone() const;
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
	AQLRandBase*					mpInner;
	bool						isOdd;
	DoubleArray				nextVariates;

};

#endif

