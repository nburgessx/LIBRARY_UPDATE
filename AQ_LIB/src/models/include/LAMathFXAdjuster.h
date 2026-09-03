#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "LAMathVolFuncFX.h"
#include "LAMathPathEntity.h"
#include "LARatesNumeraireBase.h"
#include "AQLDataVector.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"


// Funciton ID of LAMathFXAdjuster
#define FN_FXADJUSTER	10013
// Function Name of LAMathFXAdjuster
#define FN_FXADJUSTER_STR	"fn_fxadjuster"


class LAMathFXAdjuster : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathFXAdjuster(double maxFXMultiplier, const LAMathVolFuncFX *pVolFX, bool delFlg = false, bool exAdjFlg = false);
	// destructor
	virtual ~LAMathFXAdjuster(void);
	// copy constructor
	LAMathFXAdjuster(const LAMathFXAdjuster &rhs);

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
	void						setPathEntity(const LAMathPathEntity &path);
	
protected :
	const double mMaxFXMultiplier; //! max fx multiplier
	const LAMathVolFuncFX *mpVolFX; //! fx volatility function
	bool mDelFlg;                 //! pointer owner flag
	bool mExAdjFlg;                //! extra adjustment flag
	const LAMathPathEntity *mpPath; //! path object
	
	mutable const LARatesNumeraireBase *mpNume;		//! numeraire to calc adjustment
	mutable std::map<double, double> mFXCorrectnum; //! member to calc adjustment
	mutable std::map<double, double> mFXCorrectden; //! member to calc adjustment

	
};
