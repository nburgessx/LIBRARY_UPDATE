#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataInstance.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLAlgorithm.h"
#include "LAAnalyticFormula.h"
#include "AQLPriceDataCalendar.h"

// Funciton ID of LAMathVolFuncFXVannaVolga
#define FN_VOLFUNCFXVANNAVOLGA	10062
// Function Name of LAMathVolFuncFXVannaVolga
#define FN_VOLFUNCFXVANNAVOLGA_STR	"fn_volfuncfxvannavolga"

class AQLObject;
class AQLDataInstance;
class AQLDataProvider;


class LAMathVolFuncFXVannaVolga : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncFXVannaVolga(AQLDataInstance* dataInstance, const ATMInterpolationMethod &atmMethod,
							const std::vector<FXOptionData> &opdata, const std::vector<SmileData> &smiledata,
							const DoubleVector& matuterms365, const AQLString& dYieldDataName, const AQLString& fYieldName,
							double spotrate/*, const AQLPriceDataCalendar& calendar, const AQLPriceDataSlidingRule& slidingrule,
							AQLString spotlag*/);

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
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;

	void						setUpVannaVolgaMatrix(AnalyticGKParam* gkParam, AQLDataProvider* dp) const;
	const DoubleVector&			getMarketPriceVec(double maturityTerm365) const;
	const DoubleVector&			getStrikeVec(double maturityTerm365) const;
	
	ATMInterpolationMethod getATMInterpolationMethod(void)const {return mAtmMethod;};
	const std::vector<FXOptionData>& getFXOptionData(void) const {return mOpData;};
	const std::vector<SmileData>& getSmileData(void) const {return mSmileData;};
	const DoubleVector& getMaturityTerms365(void) const {return mMatuTerms365;};


protected :

	AQLDataInstance* mpDataInstance;
	mutable ATMInterpolationMethod mAtmMethod;
	mutable std::vector<FXOptionData> mOpData;
	mutable	std::vector<SmileData> mSmileData;
	mutable DoubleVector mMatuTerms365;
	AQLString mdYieldDataName;
	AQLString mfYieldDataName;
	double mSpotRate;

	mutable std::map<double, DoubleVector> mOmegaMap;
	mutable std::map<double, DoubleVector> mStrikeMap;

};

