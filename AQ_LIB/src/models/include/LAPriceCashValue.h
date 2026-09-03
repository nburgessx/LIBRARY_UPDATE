/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifndef LAPriceCashValue_h
#define LAPriceCashValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLDataValuation.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "LAPricePayOff.h"
#include "LAPriceTradeValue.h"


// LALinearRatesSwapTradeValue's function id
#define FN_IR_CASHVALUE		10060
// LAPriceCashValue's function name
#define FN_IR_CASHVALUE_STR		"fn_ir_cashvalue"

//#define __ITM_CHECK__ 
//// FROTOTYPE ////
class AQLDate;
class AQLObject;
class AQLPriceDataManager;
class LAMathFXEntity;
class LAMathPlainVanillaEntity;
class LAPriceAccruedInterest;
class LAMathYieldCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class LAPriceCashValue : public LAPriceTradeValue
{
public:
    // constructor	
	explicit LAPriceCashValue();
    // destructor	
	virtual ~LAPriceCashValue();
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
	LAPriceCashValue & operator=( const LAPriceCashValue & ) { return *this; }


	/*!
		@brief cache class for performance up
	*/
	class LAPriceCashValueDataProvider : public LAPriceTradeValueDataProvider
	{
	public:
		virtual ~LAPriceCashValueDataProvider(); 
		AQLString currency;
		double cash;
	};

protected:
    // copy constructor
    LAPriceCashValue(const LAPriceCashValue& v);	
	// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;
	// get FXEntity
	const LAMathFXEntity &getFXEntity(AQLObject &object) const;

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
};
#endif
