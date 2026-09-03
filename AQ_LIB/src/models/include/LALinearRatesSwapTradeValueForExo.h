/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifndef LALinearRatesSwapTradeValueForExo_h
#define LALinearRatesSwapTradeValueForExo_h


#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreValuation.h"
#include "LADataValuation.h"
#include "LACoreAppError.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include "LAPricePayOff.h"
#include "LALinearRatesSwapTradeValue.h"


// LALinearRatesSwapTradeValueForExo's function id
#define FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO				10089
// LALinearRatesSwapTradeValueForExo's function name
#define FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO_STR			"fn_ir_plainvanillaswaptradevalueforexo"


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
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class LALinearRatesSwapTradeValueForExo : public LALinearRatesSwapTradeValue
{
public:
    // constructor	
	explicit LALinearRatesSwapTradeValueForExo(LAPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~LALinearRatesSwapTradeValueForExo();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
    // evaluation function
	virtual double              value(const LADate& basedate, 
										LAObject& object,
									const LADataValuation& att) const;


	//Remove warning:C4512
	LALinearRatesSwapTradeValueForExo & operator=( const LALinearRatesSwapTradeValueForExo & ) { return *this; }



	

protected:
    // copy constructor
    LALinearRatesSwapTradeValueForExo(const LALinearRatesSwapTradeValueForExo& v);	

};
#endif
