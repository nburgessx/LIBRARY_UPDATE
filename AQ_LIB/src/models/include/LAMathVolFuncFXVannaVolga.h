#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataInstance.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAInterpolationBase.h"
#include "LAAlgorithm.h"
#include "LAAnalyticFormula.h"
#include "LAPriceDataCalendar.h"

// Funciton ID of LAMathVolFuncFXVannaVolga
#define FN_VOLFUNCFXVANNAVOLGA	10062
// Function Name of LAMathVolFuncFXVannaVolga
#define FN_VOLFUNCFXVANNAVOLGA_STR	"fn_volfuncfxvannavolga"

class LAObject;
class LADataInstance;
class LADataProvider;


class LAMathVolFuncFXVannaVolga : public LAFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncFXVannaVolga(LADataInstance* dataInstance, const ATMInterpolationMethod &atmMethod,
							const std::vector<FXOptionData> &opdata, const std::vector<SmileData> &smiledata,
							const DoubleVector& matuterms365, const LAString& dYieldDataName, const LAString& fYieldName,
							double spotrate/*, const LAPriceDataCalendar& calendar, const LAPriceDataSlidingRule& slidingrule,
							LAString spotlag*/);

	//LAMathVolFuncFXVannaVolga(mpDataInstance,mAtmMethod,mFxParams,mSmileData,mMatuTerms365,
	//												mdYieldDataName,mfYieldDataName,mSpotRate);
	
	// destructor
	virtual ~LAMathVolFuncFXVannaVolga(void);
	// copy constructor
	LAMathVolFuncFXVannaVolga(const LAMathVolFuncFXVannaVolga &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;

	void						setUpVannaVolgaMatrix(AnalyticGKParam* gkParam, LADataProvider* dp) const;
	const DoubleVector&			getMarketPriceVec(double maturityTerm365) const;
	const DoubleVector&			getStrikeVec(double maturityTerm365) const;
	
	ATMInterpolationMethod getATMInterpolationMethod(void)const {return mAtmMethod;};
	const std::vector<FXOptionData>& getFXOptionData(void) const {return mOpData;};
	const std::vector<SmileData>& getSmileData(void) const {return mSmileData;};
	const DoubleVector& getMaturityTerms365(void) const {return mMatuTerms365;};


protected :

	LADataInstance* mpDataInstance;
	mutable ATMInterpolationMethod mAtmMethod;
	mutable std::vector<FXOptionData> mOpData;
	mutable	std::vector<SmileData> mSmileData;
	mutable DoubleVector mMatuTerms365;
	LAString mdYieldDataName;
	LAString mfYieldDataName;
	double mSpotRate;

	mutable std::map<double, DoubleVector> mOmegaMap;
	mutable std::map<double, DoubleVector> mStrikeMap;

};

