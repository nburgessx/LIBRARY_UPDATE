#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDataReference.h"
#include "AQLBasic.h"
#include "AQLDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceIRCapFloorOptionValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLMathVolFuncIRSABR.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLPriceDataFunction.h"
#include "AQLPricePayOff.h"
#include "AQLBlackScholesBaseFunc.h"
#include "AQLBlackScholesDelayedCapletOptionPayoff.h"
#include "AQLBlackScholesDelayedFloorletOptionPayoff.h"
#include "AQLBlackScholesDigitalCapletOptionPayoff.h"
#include "AQLBlackScholesDigitalFloorletOptionPayoff.h"
#include "AQLMathValuableEntity.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLDataProcedure.h"
#include "AQLMathIndexEntity.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLInterpolationBase.h"
#include "AQLPricePortfolioValue.h"

using namespace std;

AQLPriceIRCapFloorOptionValue::AQLPriceIRCapFloorOptionValue()
: AQLLinearRatesOptionValue()
{}

//AQLPriceIRCapFloorOptionValue::AQLPriceIRCapFloorOptionValue(AQLPriceIRCapFloorOptionValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceIRCapFloorOptionValue::~AQLPriceIRCapFloorOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceIRCapFloorOptionValue::getType() const
{
	return FN_IR_CAPFLOOROPTIONVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceIRCapFloorOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_CAPFLOOROPTIONVALUE ? true : AQLLinearRatesOptionValue::isTypeOf(id));
}

// vanna-volga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceIRCapFloorOptionValue::getOptionPayoffName() const
{
	return FN_IR_CAPFLOOROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceIRCapFloorOptionValue::registerData(AQLPriceDataManager& dm) const
{

	AQLLinearRatesOptionValue::registerData(dm);
}

AQLCoreFunctionBase*
AQLPriceIRCapFloorOptionValue::clone() const
{
    try 
	{
		return new AQLPriceIRCapFloorOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
AQLPriceIRCapFloorOptionValue::calcOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const
{
	AQLDataHolder*dh;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);

	AQLDataInstance* dataInstance = e.getDataInstance();
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	
	AQLString nb = dynamic_cast<const AQLDataString& >(e.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	AQLString tempname = "ChangeToSwap" + nb;
	AQLMathObjectValue* vcapfloor =NULL;
	const AQLObjectHolder ehCur = objPool.getObject(tempname);
	if(!ehCur.isDefined())
	{	
		vcapfloor= new AQLMathObjectValue(e.getDataInstance());
		objPool.set(tempname,vcapfloor);
		//name
		vcapfloor->getName().convertFromString(tempname);
		//value
		vcapfloor->setValuationMethod(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);

		//path object
		AQLString pathname = dataProvider->mpvanilla->getName().get();
		vcapfloor->AQLObject::add(PRICING_DATA_PATHENTITY, new AQLDataReference()).convertFromString(pathname);
		
		//isdetailoutput
		vcapfloor->AQLObject::add(PRICING_DATA_ISDETAILOUTPUT, new AQLDataString()).convertFromString("TRUE");

		//settledate
		vcapfloor->AQLObject::add(PRICING_DATA_SETTLEDATE, new AQLDataDate(dataProvider->mAsofDate));
		//today
		vcapfloor->AQLObject::add(PRICING_DATA_TODAY, new AQLDataDate(dataProvider->mAsofDate));
		//underlyings
		AQLString understrs = e.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).convertToString();
		vcapfloor->AQLObject::add(CALIBRATION_DATA_UNDERLYINGS, new AQLDataMultiReference()).convertFromString(understrs);

		//currency
        const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(e.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
        const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(legs.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
        vcapfloor->remove(PRICING_DATA_CURRENCY);
        vcapfloor->AQLObject::add(PRICING_DATA_CURRENCY, new AQLDataString()).convertFromString(cashlets.get(0).getData(PRICING_DATA_CURRENCY, ISNOTNULL).convertToString());

		//cfgenerator
		vcapfloor->AQLObject::add(PRICING_DATA_CFGENERATOR, new AQLDataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
		
		//resultoutput
		vcapfloor->AQLObject::add(PRICING_DATA_ISRESULTOUTPUT, new AQLDataBool(true));

        if((dh = &e.getData(PRICING_DATA_FXRATE))->isDefined() && !dh->isNull()){
            vcapfloor->add(PRICING_DATA_FXRATE).convertFromString(dh->convertToString());
        }
		
		
		//leg select side
		AQLString selectstr = (dataProvider->buysell) ? "RCV" : "PAY"; 
		legs.get(0).get().remove(PRICING_DATA_SELECTSIDE);
		legs.get(0).get().add(PRICING_DATA_SELECTSIDE, new AQLDataString(selectstr));
		
		dh = &(e.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			vcapfloor->AQLObject::add(PRICING_DATA_ISSAVEPASTFIXING, dh->clone());
		}

		//forward shift value
		double forwardShiftValue(0.);
		if (dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR))
		{
			forwardShiftValue = dynamic_cast<const AQLMathVolFuncIRSABR*>(dataProvider->mVolfunc)->getForwardShiftValue();
		}
		for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
		{
			dh = &cashlets.get(i).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK);
			if(dh->isDefined() && !dh->isNull())
			{
				AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference& >(dh->get());
				AQLFunctionBase& method = dynamic_cast<AQLPriceDataFunction& >(coupons.get(0).get().getData(PRICING_DATA_OPERATOR, ISNOTNULL).get()).getFunction();
				if (method.isTypeOf(FN_BSBASEFUNC))
				{
					AQLBlackScholesBaseMethod& bsbasefunc(dynamic_cast<AQLBlackScholesBaseMethod &>(method));
					bsbasefunc.setForwardShiftValue(forwardShiftValue);
				}
			}
		}		

	}
	else
	{
		vcapfloor	= &dynamic_cast<AQLMathObjectValue &>(objPool.getObject(tempname).get());
		//settledate
		dynamic_cast<AQLDataDate &>(vcapfloor->getData(PRICING_DATA_SETTLEDATE, ISNOTNULL).get()).set(dataProvider->mAsofDate);
		//today
		dynamic_cast<AQLDataDate &>(vcapfloor->getData(PRICING_DATA_TODAY, ISNOTNULL).get()).set(dataProvider->mAsofDate);
		//currency
        const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(e.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
        const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(legs.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
        vcapfloor->remove(PRICING_DATA_CURRENCY);
        vcapfloor->AQLObject::add(PRICING_DATA_CURRENCY, new AQLDataString()).convertFromString(cashlets.get(0).getData(PRICING_DATA_CURRENCY, ISNOTNULL).convertToString());
		
		AQLString pathname = dataProvider->mpvanilla->getName().get();
		vcapfloor->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).convertFromString(pathname);

	}

	//send a iscalc risk message
	bool iscalcrisk = false;
	dh = &e.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	vcapfloor->remove(PRICING_DATA_ISCALCRISK);
	vcapfloor->AQLObject::add(PRICING_DATA_ISCALCRISK, new AQLDataBool(iscalcrisk));

	vcapfloor->remove(PRICING_DATA_ISSETUPPAYOFF);
	vcapfloor->AQLObject::add(PRICING_DATA_ISSETUPPAYOFF, new AQLDataBool(true));

	AQLDate basedate;
	double ret = vcapfloor->value(dataProvider->mAsofDate);


	return ret;
}

// calc payoff after maturity
double				
AQLPriceIRCapFloorOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	return calcOption(att,dataProvider,e);
}



/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceIRCapFloorOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceIROptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceIROptionValueDataProvider *>(AQLLinearRatesOptionValue::setUpDataProvider(basedate,object,att));

	//now set strike and tenorstring
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();

	// get DiscountCurve
	dataProvider->mDCurveType = STD;
	dh = &(leg1.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mDCurveType = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw AQLCoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	std::vector< std::vector<AnalyticParam*> > ret(dataProvider->mCashletSize);
	dataProvider->mMaturityDates.resize(dataProvider->mCashletSize);
	dataProvider->mStartTerms.resize(dataProvider->mCashletSize);
	dataProvider->mFCurveTypes.resize(dataProvider->mCashletSize, STD);
	dataProvider->mDCurveTypes.resize(dataProvider->mCashletSize, STD);
	dataProvider->mUnderlying.resize(dataProvider->mCashletSize, "");
	dataProvider->mUnTenor.resize(dataProvider->mCashletSize, "");

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	bool isFWDUpdate = false;
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		AQLObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
		
		AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw AQLCoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		AQLFunctionBase& method = dynamic_cast<AQLPriceDataFunction& >(dh->get()).getFunction();

		if (method.isTypeOf(FN_BSBASEFUNC))
		{
			AQLBlackScholesBaseMethod &bsbasefunc = dynamic_cast<AQLBlackScholesBaseMethod &>(method);

			dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			AQLDataMultiReference& indexs = dynamic_cast<AQLDataMultiReference& >(dh->get());

			AQLObject& eindex = indexs.get(0).get();
			//indextype == FixedRate means return payoff method
			dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			AQLString indextype = dynamic_cast<const AQLDataString &>(dh->get());
			if (indextype.toUpper() != "FIXEDRATE")
			{
				if (!isFWDUpdate)
				{
					dh = &(eindex.getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						dataProvider->mIsFWDInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
						dataProvider->mpFWDInter = dynamic_cast<AQLInterpolationBase *>(dynamic_cast<const AQLPriceDataInterpolation &>(eindex.getData(PRICING_DATA_FWDINTERPOLATION, NOCHECK).get()).getMethod().clone());
						dataProvider->mFCurveType = STD;
						dh = &(eindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							dataProvider->mFCurveType = dynamic_cast<const AQLDataString &>(dh->get()).get();
						}

					}
					isFWDUpdate = true;
				}
				if (dataProvider->mIsFWDInter)
				{
					AQLDate startdate;
					dh = &(eindex.getData(PRICING_DATA_DATESFORINDEXGENERATE, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						const DateVector &dates = dynamic_cast<const AQLDataDates &>(dh->get()).get();
						if (dates.empty())
						{
							throw AQLCoreInvalidData("dates is empty.", __FILE__, __LINE__);
						}
						startdate = dates[0];
					}
					else
					{
						const AQLDate &fixdate = dynamic_cast<const AQLDataDate &>(eindex.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL).get()).get();
						const int spotLag = dynamic_cast<const AQLDataInt &>(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL).get()).get();
						const AQLPriceDataSlidingRule *psrule = 0;
						const AQLPriceDataCalendar *pcal = 0;
						AQLPriceCFGenUtility::getBusDayRuleAndCalendar(eindex, 
																	PRICING_DATA_FIXINGSLIDINGRULE,
																	PRICING_DATA_FIXINGCALENDAR,
																	eindex,
																	CALIBRATION_DATA_SLIDINGRULE,
																	CALIBRATION_DATA_CALENDAR,
																	psrule, pcal);
						if (pcal)
						{
							startdate = pcal->getBusinessDay(fixdate, spotLag);
						}
						else
						{
							startdate = fixdate;
							startdate.addDays(spotLag);
						}

					}
					dataProvider->mStartTerms[i] =  dc_act365.getTerm(basedate, startdate);
				}
				dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
				dataProvider->mUnTenor[i] = dynamic_cast<AQLDataString &>(dh->get()).get();
				if (dataProvider->mUnTenor[i].size() > 0)
					dataProvider->mUnTenor[i].toUpper();

				dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL));
				const AQLDate& fixdate = dynamic_cast<const AQLDataDate& >(dh->get()).get();
				dataProvider->mMaturityDates[i] = fixdate;

				dh = &(eindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dataProvider->mFCurveTypes[i] = dynamic_cast<const AQLDataString &>(dh->get()).get();
				}

				dh = &(eindex.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dataProvider->mDCurveTypes[i] = dynamic_cast<const AQLDataString &>(dh->get()).get();
				}

				dh = &(eindex.getData(PRICING_DATA_VOLATILITYUNDERLYING, ISNOTNULL));
				dataProvider->mUnderlying[i] = dynamic_cast<const AQLDataString &>(dh->get()).get();
			}
			else
			{
				dataProvider->mMaturityDates[i] = dataProvider->mAsofDate;	
			}
			dh = &(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			const DoubleArray& coeff = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			bsbasefunc.setParam(coeff);
			bsbasefunc.setOptionStrike();
		}
	}
	
	return dataProvider;
}

std::vector< std::vector<AnalyticParam*> >
AQLPriceIRCapFloorOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	AQLDataHolder*dh;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw AQLCoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	std::vector< std::vector<AnalyticParam*> > ret(dataProvider->mCashletSize);
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		AQLObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			ret[i] = std::vector<AnalyticParam*>(0);
			continue;
		}
		
		AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw AQLCoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		AQLFunctionBase& method = dynamic_cast<AQLPriceDataFunction& >(dh->get()).getFunction();
		
		//paymentdate check
		dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const AQLDate& paydate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
		std::vector<AnalyticParam*> retvec;
		if (paydate <= dataProvider->mAsofDate)
		{
			retvec = std::vector<AnalyticParam*>(0);
		}
		else if (method.isTypeOf(FN_BSBASEFUNC))
		{
			AQLBlackScholesBaseMethod &bsbasefunc = dynamic_cast<AQLBlackScholesBaseMethod &>(method);
			retvec = bsbasefunc.getAnalyticParam();
		}
		else
		{
			retvec = std::vector<AnalyticParam*>(0);
		}


		ret[i] = retvec;
	}
	return ret;
}

void
AQLPriceIRCapFloorOptionValue::setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const
{
	AQLDataHolder*dh;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw AQLCoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		AQLObject& ecash = cashlets.get(i).get();
		//paymentdate check
		dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const AQLDate& paydate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
		if (paydate <= dataProvider->mAsofDate)
		{
			continue;
		}
		
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
		AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw AQLCoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		AQLFunctionBase& method = dynamic_cast<AQLPriceDataFunction& >(dh->get()).getFunction();

		if (method.isTypeOf(FN_BSBASEFUNC))
		{
			dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			AQLDataMultiReference& indexs = dynamic_cast<AQLDataMultiReference& >(dh->get());
		
			AQLObject& eindex = indexs.get(0).get();
			AQLBlackScholesBaseMethod &bsbasefunc = dynamic_cast<AQLBlackScholesBaseMethod &>(method);
			// set up call spread
			if (bsbasefunc.isTypeOf(FN_IR_DIGITALCAPLETOPTIONFUNC) || bsbasefunc.isTypeOf(FN_IR_DIGITALCAPLETOPTIONFUNC))
			{
				// If digital, set call spread value to the operator.
				dh = &(object.getData(PRICING_DATA_DIGITALSPREAD, ISNOTNULL));
				double callSpreadValule = dynamic_cast<AQLDataDouble &>(dh->get()).get();
				AQLBlackScholesDigitalCapletOption& digitalBSBaseFunc = dynamic_cast<AQLBlackScholesDigitalCapletOption&>(bsbasefunc); // AQLBlackShcolesDigitalFloorletOption is inherited from AQLBlackScholesDigitalCapletOption
				digitalBSBaseFunc.setCallSpreadValue(callSpreadValule);

				dh = &(ecoupon.getData(PRICING_DATA_DIGITALCOUPON, ISNOTNULL));
				double digitalCoupon = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
				digitalBSBaseFunc.setDigitalCoupon(digitalCoupon);
			}
			bsbasefunc.setOptionNumeraire();
			bsbasefunc.setOptionStrike();
			//indextype == FixedRate means return payoff method
			dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			AQLString indextype = dynamic_cast<const AQLDataString &>(dh->get());
			if (indextype.toUpper() == "FIXEDRATE")
			{
				bsbasefunc.setIsAfterMaturity(true);
				dataProvider->mMaturityDates[i] = dataProvider->mAsofDate;
			}
			else
			{
				if (bsbasefunc.isTypeOf(FN_IR_DELAYEDCAPLETOPTIONFUNC) || bsbasefunc.isTypeOf(FN_IR_DELAYEDFLOORLETOPTIONFUNC))
				{
					const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&>(eindex.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL).get());
					dh = &(ecash.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
					const AQLDate& enddate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
					double timingterm = dc.getTerm(paydate, enddate, true);

					const AQLInterpolationBase* oiscurve = &dataProvider->mpvanilla->getIRCurve(dataProvider->mPremiumCurrency).getBasisDFInterpolation();
					const auto& curvePro = dataProvider->mpvanilla->getIRCurvePro(dataProvider->mPremiumCurrency);
					const auto& assignedCurveMktMap = curvePro.getAssignedCurveMktMap();
					for (auto iac = assignedCurveMktMap.begin(); iac != assignedCurveMktMap.end(); ++iac)
					{
						if (iac->second == OISCURVE)
						{
							oiscurve = &curvePro.getDFInterpolation(&iac->first);
							break;
						}
					}

					const double tEnd = dataProvider->mpvanilla->getDayCount().getTerm(dataProvider->mAsofDate, enddate);
					const double tPay = dataProvider->mpvanilla->getDayCount().getTerm(dataProvider->mAsofDate, paydate);
					const double df2End = oiscurve->value(tEnd);
					const double df2Pay = oiscurve->value(tPay);
					const double dfFromPayment2End = (df2End / df2Pay);

					double threshold = -1.;
					dh = &eindex.getData(PRICING_DATA_CATHRESHOLD, NOCHECK);
					if (dh->isDefined() && !dh->isNull())
					{
						threshold = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
					}

					DoubleArray convexityFactors(3, 0.0);
					convexityFactors[0] = timingterm;
					convexityFactors[1] = dfFromPayment2End;
					convexityFactors[2] = threshold;
					bsbasefunc.setConvexityFactors(convexityFactors);
				}

				dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL));
				const AQLDate& fixdate = dynamic_cast<const AQLDataDate& >(dh->get()).get();
				//Te
				double Te = dataProvider->mBlackDayCount.getTerm(dataProvider->mAsofDate, fixdate,true);
				double actT = dc_act365.getTerm(dataProvider->mAsofDate, fixdate,true);
				dataProvider->mMaturityDates[i] = fixdate;
				
				bsbasefunc.setOptionTerm(Te);
				bsbasefunc.setOptionActualTerm(actT);
				if (0.0 >= Te)
				{
					bsbasefunc.setIsAfterMaturity(true);
				}
			}
			bsbasefunc.getAnalyticMethod(AQLString("PREM"));
			bsbasefunc.getPayoffMethod(AQLString("PREM"));
			bsbasefunc.getOptionTypeVector();
		}
	}
	return;
}

const AQLDate&
AQLPriceIRCapFloorOptionValue::getMaturityDate(const AQLObject& object, AQLDataProvider* dp) const
{
	dp;
	const AQLDataHolder* dh;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);

	//when cap floor option, output the final matuirty date
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	unsigned int cashletsize = getCashletSize(object, dataProvider);

	AQLObject* ecoupon;
	AQLObject* ecash;
	while (cashletsize > 0 )
	{
		ecash = &(cashlets.get(cashletsize-1).get());
		dh = &(ecash->getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			cashletsize--;
			continue;
		}
		
		const AQLDataMultiReference& coupons = dynamic_cast<const AQLDataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw AQLCoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		
		ecoupon = &coupons.get(0).get();
		dh = &(ecoupon->getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		const AQLDataMultiReference& indexs = dynamic_cast<const AQLDataMultiReference& >(dh->get());
		AQLObject& eindex = indexs.get(0).get();
		
		dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			cashletsize--;
			continue;
		}
		const AQLDate& finalfixingdate = dynamic_cast<const AQLDataDate& >(dh->get()).get();
		return finalfixingdate;

		break;
	}

	//error case
	dh = &(ecash->getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
	const AQLDate& startdate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
	return startdate;
}

const AQLDate&
AQLPriceIRCapFloorOptionValue::getDeliveryDate(const AQLObject& object, AQLDataProvider* dp) const
{
	dp;
	const AQLDataHolder* dh;
	//when cap floor option, output the final payment date
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	unsigned int cashletsize = getCashletSize(object, dp);
	
	AQLObject& ecash = cashlets.get(cashletsize-1).get();
	dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
	const AQLDate& finalpaymentdate = dynamic_cast<const AQLDataDate& >(dh->get()).get();
	
	return finalpaymentdate;
}

std::vector< std::vector<AQLBlackScholesBase*> >
AQLPriceIRCapFloorOptionValue::getAnalyticMethod(AQLObject& object, AQLDataProvider* dp) const
{
	AQLDataHolder*dh;
	AQLPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceIROptionValueDataProvider*>(dp);

	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw AQLCoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	std::vector< std::vector<AQLBlackScholesBase* > > ret(dataProvider->mCashletSize);
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		AQLObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			ret[i] = std::vector<AQLBlackScholesBase* >(0);
			continue;
		}
		
		AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw AQLCoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		AQLFunctionBase& method = dynamic_cast<AQLPriceDataFunction& >(dh->get()).getFunction();
		
		//paymentdate check
		dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const AQLDate& paydate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
		std::vector<AQLBlackScholesBase* > retvec;
		if (paydate <= dataProvider->mAsofDate)
		{
			retvec = std::vector<AQLBlackScholesBase* >(0);
		}
		else if (method.isTypeOf(FN_BSBASEFUNC))
		{
			AQLBlackScholesBaseMethod &bsbasefunc = dynamic_cast<AQLBlackScholesBaseMethod &>(method);
			retvec = bsbasefunc.getAnalyticMethod(AQLString("PREM"));
		}
		else
		{
			//dummy case
			//retvec.push_back(NULL);
			retvec = std::vector<AQLBlackScholesBase* >(0);
		}
		ret[i] = retvec;
	}
	return ret;
}

//multiple unit
double  
AQLPriceIRCapFloorOptionValue::multipleUnit(const AQLObject& object, AQLDataProvider* dp) const
{
	object;
	dp;
	return 1.0;
}


//get cashlet size
unsigned int 
AQLPriceIRCapFloorOptionValue::getCashletSize(const AQLObject& object, AQLDataProvider* dp) const
{
	unsigned int ret = 0;
	const AQLDataHolder* dh;

	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& unders = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	AQLObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference& >(dh->get());
	ret = cashlets.getSize();

	return ret;
}




/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLPriceIRCapFloorOptionValue::createNewDataProvider() const
{
	AQLPriceIROptionValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLPriceIROptionValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

//
//----------
//

// Output cashflow
void
AQLPriceIRCapFloorOptionValue::outputResult( AQLObject& object, AQLDataProvider* dp ) const
{
	// ! Get valuable object
	AQLString nb = dynamic_cast<const AQLDataString& >(object.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	AQLString tempname = "ChangeToSwap" + nb;
	
	AQLObjectPool& objPool = object.getDataInstance()->getObjectPool(); 
	AQLObjectHolder objHolder = objPool.getObject( tempname, ENCHKTYPE_NOCHECK );
	AQLMathObjectValue* vcapfloor = NULL;
	if ( objHolder.isDefined() )
		vcapfloor = dynamic_cast< AQLMathObjectValue* >( &( objHolder.get() ) );
	else
		throw AQLCoreInvalidData( "vcapfloor is not set! AQLPriceIRCapFloorOptionVlaue::outputResult() ",__FILE__,__LINE__);
	
	AQLLinearRatesOptionValueDataProvider* dataProvider = dynamic_cast< AQLLinearRatesOptionValueDataProvider* >( dataProvider );

	// ! Get Volatility, Strike, ExpiryTerm of each cashlet
	vector< vector<AnalyticParam* > > analyticParams = dataProvider->mParam;
	DoubleMatrix vols_mat;
	DoubleMatrix strikes_mat;
	DoubleMatrix expiryTerms_mat;

	double forwardShiftValue(0.);
	if (dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR))
	{
		forwardShiftValue = dynamic_cast<const AQLMathVolFuncIRSABR*>(dataProvider->mVolfunc)->getForwardShiftValue();
	}

	if ( analyticParams.size() > 0 )
	{

		unsigned int maxColsize = 0;
		for (unsigned int i = 0; i < analyticParams.size(); i++)
			maxColsize = (analyticParams[i].size() > maxColsize) ? analyticParams[i].size() : maxColsize;

		
		for ( size_t j = 0; j < maxColsize; j++ )
		{
			DoubleVector vols;
			DoubleVector strikes;
			DoubleVector expiryTerms;
			
			for ( size_t i = 0; i < analyticParams.size(); i++ )
			{
				if ( analyticParams[i].size() > j)
				{
					strikes.push_back( analyticParams[i][j]->K );
					
					const type_info& info = typeid( *analyticParams[i][j] );
					AnalyticBKParam bkParam;
					if ( info == typeid( bkParam ) )
					{
						AnalyticBKParam* tmp(dynamic_cast< AnalyticBKParam* >( analyticParams[i][j] ));
						if (tmp->F + forwardShiftValue > MIN_RATE && tmp->K + forwardShiftValue > MIN_RATE)
						{
							vols.push_back( analyticParams[i][j]->Vol );
						}
						else
						{
							vols.push_back( 0. );
						}
						expiryTerms.push_back( tmp->Te );
					}
					else
					{
						vols.push_back( 0. );
						expiryTerms.push_back( 0. );
					}
				}
				else
				{
					vols.push_back(0.0);
					strikes.push_back(0.0);
					expiryTerms.push_back(0.0);
				
				}
			}
		
			vols_mat.push_back(vols);
			strikes_mat.push_back(strikes);
			expiryTerms_mat.push_back(expiryTerms);
		}
	}




	// ! Set Cashlet to TradeEntity
	// ! CFCalcStartDate
	AQLString cfCalcStartDateName = PRICING_DATA_CFCALCSTARTDATE_LEG + AQLString( 1 );
	object.remove( cfCalcStartDateName );
	object.add( cfCalcStartDateName, new AQLDataDates() ).convertFromString( vcapfloor->getData( cfCalcStartDateName, ISDEFINED ).convertToString() );	
	
	// ! CFCalcEndDate
	AQLString cfCalcEndDateName = PRICING_DATA_CFCALCENDDATE_LEG + AQLString( 1 );
	object.remove( cfCalcEndDateName );
	object.add( cfCalcEndDateName, new AQLDataDates() ).convertFromString( vcapfloor->getData( cfCalcEndDateName, ISDEFINED ).convertToString() );

	// ! PaymentDate
	AQLString paymentDateName = PRICING_DATA_PAYMENTDATE_LEG + AQLString( 1 );
	object.remove( paymentDateName );
	object.add( paymentDateName, new AQLDataDates() ).convertFromString( vcapfloor->getData( paymentDateName, ISDEFINED ).convertToString() );
	
	// ! PaymentTime
	AQLString time = PRICING_DATA_CASHLETVALUETIME_LEG + AQLString( 1 );		
	object.remove(time);
	object.add( time, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( time, ISDEFINED ).convertToString() );
	
	// ! Cachflow
	AQLString cfname = PRICING_DATA_CASHLETVALUE_LEG + AQLString( 1 );		
	object.remove( cfname );
	object.add( cfname, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( cfname, ISDEFINED ).convertToString() );

	// ! PV
	AQLString pvname = PRICING_DATA_PVVALUE_LEG + AQLString( 1 );
	object.remove( pvname );
	object.add( pvname, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( pvname, ISDEFINED ).convertToString() );

	// ! DF
	AQLString dfName = PRICING_DATA_DF_LEG + AQLString( 1 );
	object.remove( dfName );
	object.add( dfName, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( dfName, ISDEFINED ).convertToString() );
	
	// ! Gearing
	AQLString gearingName = PRICING_DATA_GEARING_LEG + AQLString( 1 );
	object.remove( gearingName );
	object.add( gearingName, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( gearingName, ISDEFINED ).convertToString() );

	// ! Forward
	AQLString forwardName = PRICING_DATA_FORWARD_LEG + AQLString( 1 );
	object.remove( forwardName );
	object.add( forwardName, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( forwardName, ISDEFINED ).convertToString() );

	// ! AccrualDays
	AQLString accDaysName = PRICING_DATA_ACCRUALDAYS_LEG + AQLString( 1 );
	object.remove( accDaysName );
	object.add( accDaysName, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( accDaysName, ISDEFINED ).convertToString() );

	// ! Notional
	AQLString notionalName = PRICING_CALIBRATION_DATAOTIONAL_LEG + AQLString( 1 );
	object.remove( notionalName );
	object.add( notionalName, new AQLDataDoubles() ).convertFromString( vcapfloor->getData( notionalName, ISDEFINED ).convertToString() );

	// ! FixingDate
	AQLString fixingDateName = PRICING_DATA_FIXINGDATE_LEG + AQLString( 1 );
	object.remove( fixingDateName );
	object.add( fixingDateName, new AQLDataDates() ).convertFromString( vcapfloor->getData( fixingDateName, ISDEFINED ).convertToString() );

	// ! FixingFlag
	AQLString fixingFlagName = PRICING_DATA_FIXINGFLAG_LEG + AQLString( 1 );
	object.remove( fixingFlagName );
	object.add( fixingFlagName, new AQLDataStrings() ).convertFromString( vcapfloor->getData( fixingFlagName, ISDEFINED ).convertToString() );

	for (int i=0; i<vols_mat.size(); i++)
	{
		// ! Volatility
		AQLString volName = PRICING_DATA_VOLATILITY_LEG + AQLString( 1 ) + AQLString("_") + AQLString( i + 1 );
		object.remove( volName );
		object.add( volName, new AQLDataDoubles( vols_mat[i] ) );

		// ! Strike
		AQLString strikeName = PRICING_DATA_STRIKE_LEG + AQLString( 1 ) + AQLString("_") + AQLString( i + 1 );
		object.remove( strikeName );
		object.add( strikeName, new AQLDataDoubles( strikes_mat[i] ) );

		// ! ExpiryTerm
		AQLString expiryTermName = PRICING_DATA_EXPIRYTERM_LEG + AQLString( 1 );
		object.remove( expiryTermName );
		object.add( expiryTermName, new AQLDataDoubles( expiryTerms_mat[i] ) );
	}

	// ! Get OptionValue and PremiumValue
	object.remove( PRICING_DATA_OPTIONVALUE );
	object.add( PRICING_DATA_OPTIONVALUE, new AQLDataDouble( dataProvider->mOptionValue ) );

	object.remove( PRICING_DATA_PREMIUMVALUE );
	object.add( PRICING_DATA_PREMIUMVALUE, new AQLDataDouble( dataProvider->mPremiumValue ) );
}

void 
AQLPriceIRCapFloorOptionValue::setUpNumeraireCurrency(const AQLObject& trade, AQLLinearRatesOptionValueDataProvider* dataProvider) const
{
    const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(legs.get(0).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
    const AQLString& ccy = dynamic_cast<const AQLDataString&>(cashlets.get(0).get().getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
    dataProvider->mnumerairecur = ccy;
    dataProvider->mnumerairecur.toUpper();
}

bool AQLPriceIRCapFloorOptionValue::hasCashflow(const AQLObject& trade) const
{   
    const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    for(size_t i = 0; i < legs.getSize(); i++){
        const AQLDataHolder* dh = &legs.get(i).getData(PRICING_DATA_CASHLETS);
        if(!dh->isDefined() || dh->isNull()) return false;
    }
    return true;
}

double
AQLPriceIRCapFloorOptionValue::value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const
{
    if(hasCashflow(inst)){
        return AQLLinearRatesOptionValue::value(basedate, inst, att);
    }
    else{
        const double pv = 0;
        inst.remove(PRICING_DATA_DIRTYPRICE);
        inst.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(pv));
        inst.remove(PRICING_DATA_CLEANPRICE);
        inst.add(PRICING_DATA_CLEANPRICE, new AQLDataDouble(pv));
		return pv;
    }
}
