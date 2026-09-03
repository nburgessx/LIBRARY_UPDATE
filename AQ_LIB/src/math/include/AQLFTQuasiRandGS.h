#pragma once

#include "AQLRandBase.h"
#include "AQLMersenneTwister.h"


// AQLFTQuasiRandGS Function ID
#define FN_RAND_QUASIGS     1107	//20051017
// AQLFTQuasiRandGF Function Name
#define FN_RAND_QUASIGS_STR	"fn_rand_quasigs"





///////////////////////////////////////////////////////////////////////
/*!
    @brief Class declaration of Quasi-random number generator. 
*/

class AQLFTQuasiRandGS : public AQLRandBase
{
public:

//  LIFECYCLE
	//constructor
    AQLFTQuasiRandGS();

//===================	AQLFTQuasiRandGS.h  20051014(1) start =======================//
	//copy constructor
//	AQLFTQuasiRandGS(const AQLFTQuasiRandGS&);
//===================	AQLFTQuasiRandGS.h  20051014(1)  end  =======================//
	
	//destructor
    virtual ~AQLFTQuasiRandGS();

	AQLFTQuasiRandGS & operator=( const AQLFTQuasiRandGS & ) { return *this; }

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
								
                                //==========================================
                                // function to generate uniform random numbers
    virtual void                getUniforms(DoubleArray& variates); 

                                //==========================================
                                // function to set dimension information(override the base class to match the setting of the dimension)
								//dimValue[0] total dimension
								//dimValue[1] dimension for quasi-random number
	virtual void				setDim(const UintArray& dimValue);


	
                                //==========================================
                                // function to set seed(override the base class to match the setting of the dimension)
								//Seed[0]:start number
								//Seed[1]:necessary Sequence number
								//Seed[2]:Seed For Generator Matrix
								//Seed[3]:Seed For Mersennne Twister
	virtual void                setSeed(const UlongArray& seed);


//  OPERATION
private:
	// area to memorize Generator Matrix
	UlongArray mGeneMatrix;

	// flag to judge the setting of mSeedmDim
	bool			mSeedSet,mDimSet;

	// seed of Generalization
	unsigned long	mMSeed;

	// state of Sequence Number
	unsigned long	mState; 

	// maximum of Sequence
	unsigned long	mMax; 

	// maximum number of digits
	unsigned int	mMaxDigit; 

	// Get generator Matrix recorded in UlongArray "mGeneMatrix"
	void			getGM(void);

	// Power of Irreducible Polynomial
	unsigned long	IPPower(unsigned long IPoly,unsigned int Power);//power the polynomial

	// Getting an element of Generator Matrix
	unsigned long	getEG(unsigned long IPoly,unsigned int randShift);//run Laurent expansion, and multiply the random polynomials

	// calculate figure of number by 2
	unsigned int	culcFig(unsigned long IPoly);

	//A seed of the inner rand() function only used by this class
	long			ftholdrand;
	
	//A seed Setting function of the inner rand() function only used by this class
	void			ftsrand (unsigned long seed);

	//the inner rand() function only used by this class
	int				ftrand(void);

	//MersenneTwister
	AQLMersenneTwister mInner;

	// Record of Irreducible Polynomial (1500 equations)
	static unsigned long	mIrreduciblePoly[1500];
protected:
};

