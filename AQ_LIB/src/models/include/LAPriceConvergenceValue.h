/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifndef LAPriceConvergenceValue_h
#define LAPriceConvergenceValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreValuation.h"
//#include "LADataValuation.h"
//#include "LACoreAppError.h"
//#include "LAString.h"
//#include "LACoreTemplateType.h"
//#include "LAPricePayOff.h"
//#include "LAPriceTradeValue.h"
//#include "LALinearRatesSwapTradeValue.h"


// LAPriceConvergenceValue's function id
#define FN_IR_CONVERGENCEVALUE				10149
// LAPriceConvergenceValue's function name
#define FN_IR_CONVERGENCEVALUE_STR			"fn_ir_convergencevalue"

#ifndef PRICING_DATA_CONVERGENCETARGET
#define PRICING_DATA_CONVERGENCETARGET      "ConvergenceTarget"		//  data name of convergence target
#endif
#ifndef PRICING_DATA_CONVERGENCEVALUE
#define PRICING_DATA_CONVERGENCEVALUE		"ConvergenceValue"		//  data name of convergence value
#endif
#ifndef PRICING_DATA_SUBVALUE
#define PRICING_DATA_SUBVALUE		"SubValue"		//  data name of value type
#endif
#ifndef PRICING_DATA_ISFIXEDTERM
#define PRICING_DATA_ISFIXEDTERM		"IsFixedTerm"			// data name of flag if the fixed index is in first period 
#endif
#ifndef PRICING_DATA_ORIGINALCURRENCY
#define PRICING_DATA_ORIGINALCURRENCY		"OriginalCurrency"			// data name of flag if the target trade has OriginalCurrency (used only for NDS) 
#endif

//#define __ITM_CHECK__ 
//// FROTOTYPE ////
class LADate;
class LAObject;
class LAPriceDataManager;
class LAMathFXEntity;
class LAMathIndexEntity;
class LAMathPathEntity;
class LARatesNumeraireBase;
class LAPriceAccruedInterest;
class LAPolynomialBase;
class LADataDoubleMatrix;
class LAMathPlainVanillaEntity;
class LAMathYieldCurve;
class LACoreFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class LAPriceConvergenceValue : public LACoreValuation
{
public:
	explicit LAPriceConvergenceValue(void);
     // destructor	
	virtual ~LAPriceConvergenceValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const; // %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(const LADate& basedate, 
										LAObject& object,
									const LADataValuation& att) const;


	//Remove warning:C4512
	LAPriceConvergenceValue & operator=( const LAPriceConvergenceValue & ) { return *this; }

	

protected:
    // copy constructor
    LAPriceConvergenceValue(const LAPriceConvergenceValue& v);	
private:
	void setUpCalcTarget(LAObject& object, double value) const;

	double getInitialValue(LAObject& object) const;

};
#endif
