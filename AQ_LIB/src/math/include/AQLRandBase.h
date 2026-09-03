#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDist.h"


// AQLRandBase Function ID
#define FN_RANDBASE     1101 



///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to generate random numbers.
*/
class AQLRandBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
    AQLRandBase(bool isHalleyMod = false);
    virtual ~AQLRandBase();
    //  copy constructor
//  AQLRandBase(const AQLRandBase& v);
//  QUERY
                                //======================================
                                // check whether this class derives from base class with type id
    virtual bool                isTypeOf(function_t id) const;
                                //======================================
                                // deep copy of this object
    virtual AQLCoreFunctionBase*     clone() const =0;
                                //======================================
                                // get function type
    virtual function_t          getType() const;
                                //==========================================
                                // function to generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates)=0; 
								// function to generate normal random numbers
    virtual void                getGaussians(DoubleArray& variates); 
								// function to get dimension information to be set
    const UintArray&			getDim(void)const;
								// function to get seed 
    const UlongArray&			getSeed(void)const;
								// judged if Halley mode or not
    const int					getIsHalleyMod(void)const{return mIsHalleyMod? 1: 0;}
//  OPERATION
								// function to set seed
    virtual void                setSeed(const UlongArray& seedValue);				
								// function to set dimension information(first element should be the number of dimension)
    virtual void				setDim(const UintArray& dimValue);
								// function to set parameters
    virtual void				setParam(const DoubleMatrix& param);
private:
    UintArray mDim;  
protected:
	UlongArray mSeed;//Seed Valarray
	DoubleMatrix mParam;// param 
	bool mIsHalleyMod; // halley modification flag

};

