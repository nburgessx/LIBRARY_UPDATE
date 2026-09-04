/*! @file
    @brief Class declaration to evaluate trade.
*/
#ifndef AQLPriceCashValue_h
#define AQLPriceCashValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLDataValuation.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "AQLPricePayOff.h"
#include "AQLPriceTradeValue.h"


// AQLLinearRatesSwapTradeValue's function id
#define FN_IR_CASHVALUE		10060
// AQLPriceCashValue's function name
#define FN_IR_CASHVALUE_STR		"fn_ir_cashvalue"

//#define __ITM_CHECK__ 
//// FROTOTYPE ////
class AQLDate;
class AQLObject;
class AQLPriceDataManager;
class AQLMathFXEntity;
class AQLMathPlainVanillaEntity;
class AQLPriceAccruedInterest;
class AQLMathYieldCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class AQLPriceCashValue : public AQLPriceTradeValue
{
public:
    // constructor	
	explicit AQLPriceCashValue();
    // destructor	
	virtual ~AQLPriceCashValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(	const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;

	//Remove warning:C4512
	AQLPriceCashValue & operator=( const AQLPriceCashValue & ) { return *this; }


	/*!
		@brief cache class for performance up
	*/
	class AQLPriceCashValueDataProvider : public AQLPriceTradeValueDataProvider
	{
	public:
		virtual ~AQLPriceCashValueDataProvider(); 
		AQLString currency;
		double cash;
	};

protected:
    // copy constructor
    AQLPriceCashValue(const AQLPriceCashValue& v);	
	// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;
	// get FXEntity
	const AQLMathFXEntity &getFXEntity(AQLObject &object) const;

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
};
#endif
