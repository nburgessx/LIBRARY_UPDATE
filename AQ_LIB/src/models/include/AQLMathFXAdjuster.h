#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathPathEntity.h"
#include "AQLRatesNumeraireBase.h"
#include "AQLDataVector.h"
#include "AQLMathAttrSDE.h"
#include "AQLRatesSDEBase.h"


// Funciton ID of AQLMathFXAdjuster
#define FN_FXADJUSTER	10013
// Function Name of AQLMathFXAdjuster
#define FN_FXADJUSTER_STR	"fn_fxadjuster"


class AQLMathFXAdjuster : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathFXAdjuster(double maxFXMultiplier, const AQLMathVolFuncFX *pVolFX, bool delFlg = false, bool exAdjFlg = false);
	// destructor
	virtual ~AQLMathFXAdjuster(void);
	// copy constructor
	AQLMathFXAdjuster(const AQLMathFXAdjuster &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;	
	                            //==========================================
	                            // set path object
	void						setPathEntity(const AQLMathPathEntity &path);
	
protected :
	const double mMaxFXMultiplier; //! max fx multiplier
	const AQLMathVolFuncFX *mpVolFX; //! fx volatility function
	bool mDelFlg;                 //! pointer owner flag
	bool mExAdjFlg;                //! extra adjustment flag
	const AQLMathPathEntity *mpPath; //! path object
	
	mutable const AQLRatesNumeraireBase *mpNume;		//! numeraire to calc adjustment
	mutable std::map<double, double> mFXCorrectnum; //! member to calc adjustment
	mutable std::map<double, double> mFXCorrectden; //! member to calc adjustment

	
};
