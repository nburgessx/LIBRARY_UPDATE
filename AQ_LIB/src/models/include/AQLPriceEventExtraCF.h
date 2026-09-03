#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceEventBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "AQLPricePayOffTool.h"



// AQLPriceEventExtraCF's function id
#define FN_IR_TRIGGERACTIONEXTRACF		2005
// AQLPriceEventExtraCF's function name
#define FN_IR_TRIGGERACTIONEXTRACF_STR	"fn_ir_triggeractionextracf"




///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(extra cf action).
*/
class AQLPriceEventExtraCF : public AQLPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceEventExtraCF();
	// copy constructor
	AQLPriceEventExtraCF(const AQLPriceEventExtraCF& v);
    // destructor	
	virtual ~AQLPriceEventExtraCF();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

    // execute trigger action
	virtual void	            doAction(const AQLDate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<AQLPriceEventHolder*>& futureaction,
										 std::vector<AQLPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter);
	// set up this class
	virtual	void				setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									AQLObject& triggerinfo,
									const AQLPricePayOff& payoff,
									bool isCall = false);


	// calculate extra cf
	virtual double				calcExtraCF(const AQLDate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<AQLPriceEventHolder*>& futureaction,
										 std::vector<AQLPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter,
										 AQLPricePayOffToolHolder& ph) const;

	AQLPriceEventExtraCF &operator=(const AQLPriceEventExtraCF &);

protected:

	const DateVector*  mpActionDates; // action dates
	const DateVector*  mpExpiryDates; // expiry dates 
	AQLFunctionBase*	   mpExtraCFFunc;// extra cf function
	const DoubleMatrix* mpCoefficients;// coefficinet
	std::vector<std::pair<unsigned int, unsigned int> > mInputsInfo; // inputs information of extra cf function

	const AQLMathFXEntity*	mpFX_for_ExtraCF;// FX Rate for extra cf
	AQLString	mBaseCur;// base currency
	AQLString	mExtraCFCur;// extra cf currency
	AQLStringVector mExtraCFCurs;
	bool mIsMultiExtraCF;

    DoubleMatrix	   mPastCouponPayOff;// past coupon payoff;
	DoubleMatrix	   mPastExtraCF;// past extra cf;
	DoubleMatrix	   mPastNotionalExchange;// past notional exchange;

	std::vector<std::vector<AQLPriceIndexToolBase*> >		mIndex;// index
	std::map<AQLDate, std::vector<AQLPriceIndexToolBase*> >	mIndexMap;// indexmap

	DoubleVector mSettlementAdjustRatios;

private:
	// destroy
	void destroy();
	// copyIndex
	void copyIndex(const AQLPriceEventExtraCF& v);

};

