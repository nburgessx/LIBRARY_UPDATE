#ifndef LAPriceFXSingleBarrierOptionValue_h
#define LAPriceFXSingleBarrierOptionValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceFXOptionValue.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABasic.h"
#include "LADist.h"
#include "LADataValuation.h"
#include "LAFindRootBrent.h"
#include "LAGaussLegendre.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAMathVolFuncFXStrangleSolver.h"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;


class LAPriceFXSingleBarrierOptionValue : public LAPriceFXOptionValue
{
public:
	// Default constructor
	LAPriceFXSingleBarrierOptionValue();
	// Destructor
	~LAPriceFXSingleBarrierOptionValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//hishida vannavolga
								//======================================
								// Return option method name
	virtual LAString			getOptionPayoffName() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(LAObject& object, LADataProvider* dataProvider) const;
	
	
	// set up dataProvider
	virtual LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const;
private:
	
};
#endif

