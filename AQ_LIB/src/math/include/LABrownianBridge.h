#pragma once

#include "LARandBase.h"
#include "LACoreAppError.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include "LACoreFunctionBase.h"
#include "LADist.h"
#include "LABasic.h"
#include "LAMersenneTwister.h"
#include "LAFTQuasiRandGF.h"	//20051017
#include "LAFTQuasiRandGS.h"	//20051017
#include "LACoreSystemError.h"		//20051017


// LABrownianBridge ID
#define FN_RAND_BB     1105
// LABrownianBridge Function Name
#define FN_RAND_BB_STR	"fn_rand_bb"




///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration to provide Brownian bridge method.
*/

class LABrownianBridge : public LARandBase
{
public:
	//constructor

//	LABrownianBridge(const LARandBase& v);
	explicit LABrownianBridge(const LARandBase& v);

	//destructor
	virtual ~LABrownianBridge();
	//copy constructor
	LABrownianBridge(const LABrownianBridge& v);

	LABrownianBridge & operator=( const LABrownianBridge & ) { return *this; }

//  QUERY
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

//===================	LABrownianBridge.h  20051014(2) start =======================//
    virtual void                setDim(const UintArray& dimValue);
//===================	LABrownianBridge.h  20051014(2)  end  =======================//

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

//===================	LABrownianBridge.h  20051014(3) start =======================//
                                // function to set the quasi-random number generator to apply the BrownianBridge
//	virtual void				setInnerGenerator(const LARandBase& v);
//===================	LABrownianBridge.h  20051014(3)  end  =======================//

private:
	LARandBase					*mpRand;		// pinter to the quasi-random number, and is set when generating this class object
	LAMersenneTwister			mMTRand1;		// one of the random number generator to provide two MT(no.1)
	LAMersenneTwister			mMTRand2;		// one of the random number generator to provide two MT(no.2)
	UintArray				mMinUpper;		// member variables in order to simplify calculations with(no.1)
	UintArray				mMaxUnder;		// member variables in order to simplify calculations with(no.2)
	DoubleArray				mBBVariates;	// member variables in order to simplify calculations with(no.3)
	DoubleArray				mLdsVariates;	// array that contains the normal random number converted to regular LDS
	DoubleArray				mMtVariates;	// array that contains the normal random number converted to regular MT

};

