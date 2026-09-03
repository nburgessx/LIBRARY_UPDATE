#ifndef LAPriceFXDigitalOptionValue_h
#define LAPriceFXDigitalOptionValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceFXOptionValue.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQLGaussLegendre.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAMathVolFuncFXStrangleSolver.h"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;


class LAPriceFXDigitalOptionValue : public LAPriceFXOptionValue
{
public:
	// Default constructor
	LAPriceFXDigitalOptionValue();
	// Destructor
	~LAPriceFXDigitalOptionValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//hishida vannavolga
								//======================================
								// Return option funcname
	virtual AQLString			getOptionPayoffName() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const;
	
	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
private:
	
};
#endif

