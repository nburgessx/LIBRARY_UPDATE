/*! @file
    @brief Class declaration to evaluate trade.
*/
#ifndef AQLLinearRatesSwapTradeValueForExo_h
#define AQLLinearRatesSwapTradeValueForExo_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLDataValuation.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "AQLPricePayOff.h"
#include "AQLLinearRatesSwapTradeValue.h"


// AQLLinearRatesSwapTradeValueForExo's function id
#define FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO				10089
// AQLLinearRatesSwapTradeValueForExo's function name
#define FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO_STR			"fn_ir_plainvanillaswaptradevalueforexo"


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
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class AQLLinearRatesSwapTradeValueForExo : public AQLLinearRatesSwapTradeValue
{
public:
    // constructor	
	explicit AQLLinearRatesSwapTradeValueForExo(AQLPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~AQLLinearRatesSwapTradeValueForExo();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object,
									const AQLDataValuation& att) const;


	//Remove warning:C4512
	AQLLinearRatesSwapTradeValueForExo & operator=( const AQLLinearRatesSwapTradeValueForExo & ) { return *this; }



	

protected:
    // copy constructor
    AQLLinearRatesSwapTradeValueForExo(const AQLLinearRatesSwapTradeValueForExo& v);	

};
#endif
