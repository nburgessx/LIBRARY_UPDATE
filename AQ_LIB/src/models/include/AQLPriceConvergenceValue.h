/*! @file
    @brief Class declaration to evaluate trade.
*/
#ifndef AQLPriceConvergenceValue_h
#define AQLPriceConvergenceValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
//#include "AQLDataValuation.h"
//#include "AQLCoreAppError.h"
//#include "AQLString.h"
//#include "AQLCoreTemplateType.h"
//#include "AQLPricePayOff.h"
//#include "AQLPriceTradeValue.h"
//#include "AQLLinearRatesSwapTradeValue.h"


// AQLPriceConvergenceValue's function id
#define FN_IR_CONVERGENCEVALUE				10149
// AQLPriceConvergenceValue's function name
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
class AQLDate;
class AQLObject;
class AQLPriceDataManager;
class AQLMathFXEntity;
class AQLMathIndexEntity;
class AQLMathPathEntity;
class AQLRatesNumeraireBase;
class AQLPriceAccruedInterest;
class AQLPolynomialBase;
class AQLDataDoubleMatrix;
class AQLMathPlainVanillaEntity;
class AQLMathYieldCurve;
class AQLCoreFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class AQLPriceConvergenceValue : public AQLCoreValuation
{
public:
	explicit AQLPriceConvergenceValue(void);
     // destructor	
	virtual ~AQLPriceConvergenceValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const; // %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object,
									const AQLDataValuation& att) const;


	//Remove warning:C4512
	AQLPriceConvergenceValue & operator=( const AQLPriceConvergenceValue & ) { return *this; }

	

protected:
    // copy constructor
    AQLPriceConvergenceValue(const AQLPriceConvergenceValue& v);	
private:
	void setUpCalcTarget(AQLObject& object, double value) const;

	double getInitialValue(AQLObject& object) const;

};
#endif
