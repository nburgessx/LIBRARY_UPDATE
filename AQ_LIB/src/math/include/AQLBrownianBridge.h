#pragma once

#include "AQLRandBase.h"
#include "AQLCoreAppError.h"
#include "AQLDataValuation.h"
#include "AQLCoreTemplateType.h"
#include "AQLCoreFunctionBase.h"
#include "AQLDist.h"
#include "AQLBasic.h"
#include "AQLMersenneTwister.h"
#include "AQLFTQuasiRandGF.h"	//20051017
#include "AQLFTQuasiRandGS.h"	//20051017
#include "AQLCoreSystemError.h"		//20051017


// AQLBrownianBridge ID
#define FN_RAND_BB     1105
// AQLBrownianBridge Function Name
#define FN_RAND_BB_STR	"fn_rand_bb"




///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to provide Brownian bridge method.
*/

class AQLBrownianBridge : public AQLRandBase
{
public:
	//constructor

//	AQLBrownianBridge(const AQLRandBase& v);
	explicit AQLBrownianBridge(const AQLRandBase& v);

	//destructor
	virtual ~AQLBrownianBridge();
	//copy constructor
	AQLBrownianBridge(const AQLBrownianBridge& v);

	AQLBrownianBridge & operator=( const AQLBrownianBridge & ) { return *this; }

//  QUERY
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
                                // function to generate normal random numbers
    virtual void                getGaussians(DoubleArray& variates);
                                // function to generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates);
                                //======================================
								// function to set dimension information to be set
								//dimValue[0]	total dimention
								//dimValue[1]	LDS 1st dimension
								//dimValue[2]	MT 1st dimension
								//dimValue[3]	LDS 2nd dimension
								//dimValue[4]	MT 2nd dimension
								//dimValue[5+i]	array of calculation time[i](i=0,...,this->getDim()[0]-1)

//===================	AQLBrownianBridge.h  20051014(2) start =======================//
    virtual void                setDim(const UintArray& dimValue);
//===================	AQLBrownianBridge.h  20051014(2)  end  =======================//

								//======================================
								// function to get dimension information to be set
//    UintArray                getDim(void) const;
                                //======================================
								// set seed function	
								//Seed[0] LDS start number					@20051017 update
								//Seed[1] LDS necessary Seequence number	@20051017 update
								//Seed[2] LDS Seed For Generator Matrix	@20051017 update
								//Seed[3] Seed For Merrsenne Twister1	@20051017 update
								//Seed[4] Seed For Merrsenne Twister2	@20051017 update
    virtual void                setSeed(const UlongArray& seedValue);
								//======================================

//===================	AQLBrownianBridge.h  20051014(3) start =======================//
                                // function to set the quasi-random number generator to apply the BrownianBridge
//	virtual void				setInnerGenerator(const AQLRandBase& v);
//===================	AQLBrownianBridge.h  20051014(3)  end  =======================//

private:
	AQLRandBase					*mpRand;		// pinter to the quasi-random number, and is set when generating this class object
	AQLMersenneTwister			mMTRand1;		// one of the random number generator to provide two MT(no.1)
	AQLMersenneTwister			mMTRand2;		// one of the random number generator to provide two MT(no.2)
	UintArray				mMinUpper;		// member variables in order to simplify calculations with(no.1)
	UintArray				mMaxUnder;		// member variables in order to simplify calculations with(no.2)
	DoubleArray				mBBVariates;	// member variables in order to simplify calculations with(no.3)
	DoubleArray				mLdsVariates;	// array that contains the normal random number converted to regular LDS
	DoubleArray				mMtVariates;	// array that contains the normal random number converted to regular MT

};

