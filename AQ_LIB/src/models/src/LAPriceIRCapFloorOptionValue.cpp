//  2008, Mizuho International London.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LADataReference.h"
#include "LABasic.h"
#include "LAMathDateCalculations.h"
#include "LACoreComponentManager.h"
#include "LAPriceIRCapFloorOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LAMathVolFuncIRSABR.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAPriceDataFunction.h"
#include "LAPricePayOff.h"
#include "LABlackScholesBaseFunc.h"
#include "LABlackScholesDelayedCapletOptionPayoff.h"
#include "LABlackScholesDelayedFloorletOptionPayoff.h"
#include "LABlackScholesDigitalCapletOptionPayoff.h"
#include "LABlackScholesDigitalFloorletOptionPayoff.h"
#include "LAMathValuableEntity.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LADataProcedure.h"
#include "LAMathIndexEntity.h"
#include "LAPriceDataInterpolation.h"
#include "LAInterpolationBase.h"
#include "LAPricePortfolioValue.h"

using namespace std;

LAPriceIRCapFloorOptionValue::LAPriceIRCapFloorOptionValue()
: LALinearRatesOptionValue()
{}

//LAPriceIRCapFloorOptionValue::LAPriceIRCapFloorOptionValue(LAPriceIRCapFloorOptionValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceIRCapFloorOptionValue::~LAPriceIRCapFloorOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceIRCapFloorOptionValue::getType() const
{
	return FN_IR_CAPFLOOROPTIONVALUE;
}
/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceIRCapFloorOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_CAPFLOOROPTIONVALUE ? true : LALinearRatesOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceIRCapFloorOptionValue::getOptionPayoffName() const
{
	return FN_IR_CAPFLOOROPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceIRCapFloorOptionValue::registerData(LAPriceDataManager& dm) const
{

	LALinearRatesOptionValue::registerData(dm);
}

LACoreFunctionBase*
LAPriceIRCapFloorOptionValue::clone() const
{
    try 
	{
		return new LAPriceIRCapFloorOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

// calc option
double 
LAPriceIRCapFloorOptionValue::calcOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const
{
	LADataHolder*dh;
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);

	LADataInstance* dataInstance = e.getDataInstance();
	LAObjectPool& objPool = dataInstance->getObjectPool();
	
	LAString nb = dynamic_cast<const LADataString& >(e.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	LAString tempname = "ChangeToSwap" + nb;
	LAMathObjectValue* vcapfloor =NULL;
	const LAObjectHolder ehCur = objPool.getObject(tempname);
	if(!ehCur.isDefined())
	{	
		vcapfloor= new LAMathObjectValue(e.getDataInstance());
		objPool.set(tempname,vcapfloor);
		//name
		vcapfloor->getName().convertFromString(tempname);
		//value
		vcapfloor->setValuationMethod(FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);

		//path object
		LAString pathname = dataProvider->mpvanilla->getName().get();
		vcapfloor->LAObject::add(PRICING_DATA_PATHENTITY, new LADataReference()).convertFromString(pathname);
		
		//isdetailoutput
		vcapfloor->LAObject::add(PRICING_DATA_ISDETAILOUTPUT, new LADataString()).convertFromString("TRUE");

		//settledate
		vcapfloor->LAObject::add(PRICING_DATA_SETTLEDATE, new LADataDate(dataProvider->mAsofDate));
		//today
		vcapfloor->LAObject::add(PRICING_DATA_TODAY, new LADataDate(dataProvider->mAsofDate));
		//underlyings
		LAString understrs = e.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).convertToString();
		vcapfloor->LAObject::add(CALIBRATION_DATA_UNDERLYINGS, new LADataMultiReference()).convertFromString(understrs);

		//currency
        const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(e.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
        const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(legs.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
        vcapfloor->remove(PRICING_DATA_CURRENCY);
        vcapfloor->LAObject::add(PRICING_DATA_CURRENCY, new LADataString()).convertFromString(cashlets.get(0).getData(PRICING_DATA_CURRENCY, ISNOTNULL).convertToString());

		//cfgenerator
		vcapfloor->LAObject::add(PRICING_DATA_CFGENERATOR, new LADataProcedure()).convertFromString(FN_IR_CASHFLOWGENERATOR_STR);
		
		//resultoutput
		vcapfloor->LAObject::add(PRICING_DATA_ISRESULTOUTPUT, new LADataBool(true));

        if((dh = &e.getData(PRICING_DATA_FXRATE))->isDefined() && !dh->isNull()){
            vcapfloor->add(PRICING_DATA_FXRATE).convertFromString(dh->convertToString());
        }
		
		
		//leg select side
		LAString selectstr = (dataProvider->buysell) ? "RCV" : "PAY"; 
		legs.get(0).get().remove(PRICING_DATA_SELECTSIDE);
		legs.get(0).get().add(PRICING_DATA_SELECTSIDE, new LADataString(selectstr));
		
		dh = &(e.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			vcapfloor->LAObject::add(PRICING_DATA_ISSAVEPASTFIXING, dh->clone());
		}

		//forward shift value
		double forwardShiftValue(0.);
		if (dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR))
		{
			forwardShiftValue = dynamic_cast<const LAMathVolFuncIRSABR*>(dataProvider->mVolfunc)->getForwardShiftValue();
		}
		for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
		{
			dh = &cashlets.get(i).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK);
			if(dh->isDefined() && !dh->isNull())
			{
				LADataMultiReference& coupons = dynamic_cast<LADataMultiReference& >(dh->get());
				LAFunctionBase& method = dynamic_cast<LAPriceDataFunction& >(coupons.get(0).get().getData(PRICING_DATA_OPERATOR, ISNOTNULL).get()).getFunction();
				if (method.isTypeOf(FN_BSBASEFUNC))
				{
					LABlackScholesBaseMethod& bsbasefunc(dynamic_cast<LABlackScholesBaseMethod &>(method));
					bsbasefunc.setForwardShiftValue(forwardShiftValue);
				}
			}
		}		

	}
	else
	{
		vcapfloor	= &dynamic_cast<LAMathObjectValue &>(objPool.getObject(tempname).get());
		//settledate
		dynamic_cast<LADataDate &>(vcapfloor->getData(PRICING_DATA_SETTLEDATE, ISNOTNULL).get()).set(dataProvider->mAsofDate);
		//today
		dynamic_cast<LADataDate &>(vcapfloor->getData(PRICING_DATA_TODAY, ISNOTNULL).get()).set(dataProvider->mAsofDate);
		//currency
        const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(e.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
        const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(legs.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
        vcapfloor->remove(PRICING_DATA_CURRENCY);
        vcapfloor->LAObject::add(PRICING_DATA_CURRENCY, new LADataString()).convertFromString(cashlets.get(0).getData(PRICING_DATA_CURRENCY, ISNOTNULL).convertToString());
		
		LAString pathname = dataProvider->mpvanilla->getName().get();
		vcapfloor->getData(PRICING_DATA_PATHENTITY, ISNOTNULL).convertFromString(pathname);

	}

	//send a iscalc risk message
	bool iscalcrisk = false;
	dh = &e.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const LADataBool&>(dh->get()).get();
	vcapfloor->remove(PRICING_DATA_ISCALCRISK);
	vcapfloor->LAObject::add(PRICING_DATA_ISCALCRISK, new LADataBool(iscalcrisk));

	vcapfloor->remove(PRICING_DATA_ISSETUPPAYOFF);
	vcapfloor->LAObject::add(PRICING_DATA_ISSETUPPAYOFF, new LADataBool(true));

	LADate basedate;
	double ret = vcapfloor->value(dataProvider->mAsofDate);


	return ret;
}

// calc payoff after maturity
double				
LAPriceIRCapFloorOptionValue::calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const
{
	return calcOption(att,dataProvider,e);
}



/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceIRCapFloorOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceIROptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceIROptionValueDataProvider *>(LALinearRatesOptionValue::setUpDataProvider(basedate,object,att));

	//now set strike and tenorstring
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();

	// get DiscountCurve
	dataProvider->mDCurveType = STD;
	dh = &(leg1.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->mDCurveType = dynamic_cast<const LADataString &>(dh->get()).get();
	}
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw LACoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	std::vector< std::vector<AnalyticParam*> > ret(dataProvider->mCashletSize);
	dataProvider->mMaturityDates.resize(dataProvider->mCashletSize);
	dataProvider->mStartTerms.resize(dataProvider->mCashletSize);
	dataProvider->mFCurveTypes.resize(dataProvider->mCashletSize, STD);
	dataProvider->mDCurveTypes.resize(dataProvider->mCashletSize, STD);
	dataProvider->mUnderlying.resize(dataProvider->mCashletSize, "");
	dataProvider->mUnTenor.resize(dataProvider->mCashletSize, "");

	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	bool isFWDUpdate = false;
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		LAObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
		
		LADataMultiReference& coupons = dynamic_cast<LADataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw LACoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		LAObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		LAFunctionBase& method = dynamic_cast<LAPriceDataFunction& >(dh->get()).getFunction();

		if (method.isTypeOf(FN_BSBASEFUNC))
		{
			LABlackScholesBaseMethod &bsbasefunc = dynamic_cast<LABlackScholesBaseMethod &>(method);

			dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			LADataMultiReference& indexs = dynamic_cast<LADataMultiReference& >(dh->get());

			LAObject& eindex = indexs.get(0).get();
			//indextype == FixedRate means return payoff method
			dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			LAString indextype = dynamic_cast<const LADataString &>(dh->get());
			if (indextype.toUpper() != "FIXEDRATE")
			{
				if (!isFWDUpdate)
				{
					dh = &(eindex.getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						dataProvider->mIsFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
						dataProvider->mpFWDInter = dynamic_cast<LAInterpolationBase *>(dynamic_cast<const LAPriceDataInterpolation &>(eindex.getData(PRICING_DATA_FWDINTERPOLATION, NOCHECK).get()).getMethod().clone());
						dataProvider->mFCurveType = STD;
						dh = &(eindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							dataProvider->mFCurveType = dynamic_cast<const LADataString &>(dh->get()).get();
						}

					}
					isFWDUpdate = true;
				}
				if (dataProvider->mIsFWDInter)
				{
					LADate startdate;
					dh = &(eindex.getData(PRICING_DATA_DATESFORINDEXGENERATE, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						const DateVector &dates = dynamic_cast<const LADataDates &>(dh->get()).get();
						if (dates.empty())
						{
							throw LACoreInvalidData("dates is empty.", __FILE__, __LINE__);
						}
						startdate = dates[0];
					}
					else
					{
						const LADate &fixdate = dynamic_cast<const LADataDate &>(eindex.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL).get()).get();
						const int spotLag = dynamic_cast<const LADataInt &>(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL).get()).get();
						const LAPriceDataSlidingRule *psrule = 0;
						const LAPriceDataCalendar *pcal = 0;
						LAPriceCFGenUtility::getBusDayRuleAndCalendar(eindex, 
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
				dataProvider->mUnTenor[i] = dynamic_cast<LADataString &>(dh->get()).get();
				if (dataProvider->mUnTenor[i].size() > 0)
					dataProvider->mUnTenor[i].toUpper();

				dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL));
				const LADate& fixdate = dynamic_cast<const LADataDate& >(dh->get()).get();
				dataProvider->mMaturityDates[i] = fixdate;

				dh = &(eindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dataProvider->mFCurveTypes[i] = dynamic_cast<const LADataString &>(dh->get()).get();
				}

				dh = &(eindex.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dataProvider->mDCurveTypes[i] = dynamic_cast<const LADataString &>(dh->get()).get();
				}

				dh = &(eindex.getData(PRICING_DATA_VOLATILITYUNDERLYING, ISNOTNULL));
				dataProvider->mUnderlying[i] = dynamic_cast<const LADataString &>(dh->get()).get();
			}
			else
			{
				dataProvider->mMaturityDates[i] = dataProvider->mAsofDate;	
			}
			dh = &(ecoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			const DoubleArray& coeff = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			bsbasefunc.setParam(coeff);
			bsbasefunc.setOptionStrike();
		}
	}
	
	return dataProvider;
}

std::vector< std::vector<AnalyticParam*> >
LAPriceIRCapFloorOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	LADataHolder*dh;
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw LACoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	std::vector< std::vector<AnalyticParam*> > ret(dataProvider->mCashletSize);
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		LAObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			ret[i] = std::vector<AnalyticParam*>(0);
			continue;
		}
		
		LADataMultiReference& coupons = dynamic_cast<LADataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw LACoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		LAObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		LAFunctionBase& method = dynamic_cast<LAPriceDataFunction& >(dh->get()).getFunction();
		
		//paymentdate check
		dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const LADate& paydate = dynamic_cast<const LADataDate &>(dh->get()).get();
		std::vector<AnalyticParam*> retvec;
		if (paydate <= dataProvider->mAsofDate)
		{
			retvec = std::vector<AnalyticParam*>(0);
		}
		else if (method.isTypeOf(FN_BSBASEFUNC))
		{
			LABlackScholesBaseMethod &bsbasefunc = dynamic_cast<LABlackScholesBaseMethod &>(method);
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
LAPriceIRCapFloorOptionValue::setUpAnalyticParam(LAObject& object, LADataProvider* dp) const
{
	LADataHolder*dh;
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw LACoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		LAObject& ecash = cashlets.get(i).get();
		//paymentdate check
		dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const LADate& paydate = dynamic_cast<const LADataDate &>(dh->get()).get();
		if (paydate <= dataProvider->mAsofDate)
		{
			continue;
		}
		
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
		LADataMultiReference& coupons = dynamic_cast<LADataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw LACoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		LAObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		LAFunctionBase& method = dynamic_cast<LAPriceDataFunction& >(dh->get()).getFunction();

		if (method.isTypeOf(FN_BSBASEFUNC))
		{
			dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			LADataMultiReference& indexs = dynamic_cast<LADataMultiReference& >(dh->get());
		
			LAObject& eindex = indexs.get(0).get();
			LABlackScholesBaseMethod &bsbasefunc = dynamic_cast<LABlackScholesBaseMethod &>(method);
			// set up call spread
			if (bsbasefunc.isTypeOf(FN_IR_DIGITALCAPLETOPTIONFUNC) || bsbasefunc.isTypeOf(FN_IR_DIGITALCAPLETOPTIONFUNC))
			{
				// If digital, set call spread value to the operator.
				dh = &(object.getData(PRICING_DATA_DIGITALSPREAD, ISNOTNULL));
				double callSpreadValule = dynamic_cast<LADataDouble &>(dh->get()).get();
				LABlackScholesDigitalCapletOption& digitalBSBaseFunc = dynamic_cast<LABlackScholesDigitalCapletOption&>(bsbasefunc); // LABlackShcolesDigitalFloorletOption is inherited from LABlackScholesDigitalCapletOption
				digitalBSBaseFunc.setCallSpreadValue(callSpreadValule);

				dh = &(ecoupon.getData(PRICING_DATA_DIGITALCOUPON, ISNOTNULL));
				double digitalCoupon = dynamic_cast<const LADataDouble &>(dh->get()).get();
				digitalBSBaseFunc.setDigitalCoupon(digitalCoupon);
			}
			bsbasefunc.setOptionNumeraire();
			bsbasefunc.setOptionStrike();
			//indextype == FixedRate means return payoff method
			dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
			LAString indextype = dynamic_cast<const LADataString &>(dh->get());
			if (indextype.toUpper() == "FIXEDRATE")
			{
				bsbasefunc.setIsAfterMaturity(true);
				dataProvider->mMaturityDates[i] = dataProvider->mAsofDate;
			}
			else
			{
				if (bsbasefunc.isTypeOf(FN_IR_DELAYEDCAPLETOPTIONFUNC) || bsbasefunc.isTypeOf(FN_IR_DELAYEDFLOORLETOPTIONFUNC))
				{
					const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&>(eindex.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL).get());
					dh = &(ecash.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
					const LADate& enddate = dynamic_cast<const LADataDate &>(dh->get()).get();
					double timingterm = dc.getTerm(paydate, enddate, true);

					const LAInterpolationBase* oiscurve = &dataProvider->mpvanilla->getIRCurve(dataProvider->mPremiumCurrency).getBasisDFInterpolation();
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
						threshold = dynamic_cast<const LADataDouble&> (dh->get()).get();
					}

					DoubleArray convexityFactors(3, 0.0);
					convexityFactors[0] = timingterm;
					convexityFactors[1] = dfFromPayment2End;
					convexityFactors[2] = threshold;
					bsbasefunc.setConvexityFactors(convexityFactors);
				}

				dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL));
				const LADate& fixdate = dynamic_cast<const LADataDate& >(dh->get()).get();
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
			bsbasefunc.getAnalyticMethod(LAString("PREM"));
			bsbasefunc.getPayoffMethod(LAString("PREM"));
			bsbasefunc.getOptionTypeVector();
		}
	}
	return;
}

const LADate&
LAPriceIRCapFloorOptionValue::getMaturityDate(const LAObject& object, LADataProvider* dp) const
{
	dp;
	const LADataHolder* dh;
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);

	//when cap floor option, output the final matuirty date
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	unsigned int cashletsize = getCashletSize(object, dataProvider);

	LAObject* ecoupon;
	LAObject* ecash;
	while (cashletsize > 0 )
	{
		ecash = &(cashlets.get(cashletsize-1).get());
		dh = &(ecash->getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			cashletsize--;
			continue;
		}
		
		const LADataMultiReference& coupons = dynamic_cast<const LADataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw LACoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		
		ecoupon = &coupons.get(0).get();
		dh = &(ecoupon->getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		const LADataMultiReference& indexs = dynamic_cast<const LADataMultiReference& >(dh->get());
		LAObject& eindex = indexs.get(0).get();
		
		dh = &(eindex.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			cashletsize--;
			continue;
		}
		const LADate& finalfixingdate = dynamic_cast<const LADataDate& >(dh->get()).get();
		return finalfixingdate;

		break;
	}

	//error case
	dh = &(ecash->getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL));
	const LADate& startdate = dynamic_cast<const LADataDate &>(dh->get()).get();
	return startdate;
}

const LADate&
LAPriceIRCapFloorOptionValue::getDeliveryDate(const LAObject& object, LADataProvider* dp) const
{
	dp;
	const LADataHolder* dh;
	//when cap floor option, output the final payment date
	
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	unsigned int cashletsize = getCashletSize(object, dp);
	
	LAObject& ecash = cashlets.get(cashletsize-1).get();
	dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
	const LADate& finalpaymentdate = dynamic_cast<const LADataDate& >(dh->get()).get();
	
	return finalpaymentdate;
}

std::vector< std::vector<LABlackScholesBase*> >
LAPriceIRCapFloorOptionValue::getAnalyticMethod(LAObject& object, LADataProvider* dp) const
{
	LADataHolder*dh;
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);

	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	if (cashlets.getSize() != dataProvider->mCashletSize)
		throw LACoreInvalidData("CapFloor Cashlets size error",__FILE__,__LINE__);

	std::vector< std::vector<LABlackScholesBase* > > ret(dataProvider->mCashletSize);
	for (unsigned int i = 0; i < dataProvider->mCashletSize; i++)
	{
		LAObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			ret[i] = std::vector<LABlackScholesBase* >(0);
			continue;
		}
		
		LADataMultiReference& coupons = dynamic_cast<LADataMultiReference& >(dh->get());
		if (coupons.getSize() != 1)
			throw LACoreInvalidData("CapFloor Coupons size error",__FILE__,__LINE__);
		LAObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
		LAFunctionBase& method = dynamic_cast<LAPriceDataFunction& >(dh->get()).getFunction();
		
		//paymentdate check
		dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const LADate& paydate = dynamic_cast<const LADataDate &>(dh->get()).get();
		std::vector<LABlackScholesBase* > retvec;
		if (paydate <= dataProvider->mAsofDate)
		{
			retvec = std::vector<LABlackScholesBase* >(0);
		}
		else if (method.isTypeOf(FN_BSBASEFUNC))
		{
			LABlackScholesBaseMethod &bsbasefunc = dynamic_cast<LABlackScholesBaseMethod &>(method);
			retvec = bsbasefunc.getAnalyticMethod(LAString("PREM"));
		}
		else
		{
			//dummy case
			//retvec.push_back(NULL);
			retvec = std::vector<LABlackScholesBase* >(0);
		}
		ret[i] = retvec;
	}
	return ret;
}

//multiple unit
double  
LAPriceIRCapFloorOptionValue::multipleUnit(const LAObject& object, LADataProvider* dp) const
{
	object;
	dp;
	return 1.0;
}


//get cashlet size
unsigned int 
LAPriceIRCapFloorOptionValue::getCashletSize(const LAObject& object, LADataProvider* dp) const
{
	unsigned int ret = 0;
	const LADataHolder* dh;

	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& unders = dynamic_cast<const LADataMultiReference& >(dh->get());
	LAObject& leg1 = unders.get(0).get();
	
	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference& >(dh->get());
	ret = cashlets.getSize();

	return ret;
}




/*!
	@brief create new cache class
	@return cache class
*/
LADataProvider*
LAPriceIRCapFloorOptionValue::createNewDataProvider() const
{
	LAPriceIROptionValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LAPriceIROptionValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

//
//----------
//

// Output cashflow
void
LAPriceIRCapFloorOptionValue::outputResult( LAObject& object, LADataProvider* dp ) const
{
	// ! Get valuable object
	LAString nb = dynamic_cast<const LADataString& >(object.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	LAString tempname = "ChangeToSwap" + nb;
	
	LAObjectPool& objPool = object.getDataInstance()->getObjectPool(); 
	LAObjectHolder objHolder = objPool.getObject( tempname, ENCHKTYPE_NOCHECK );
	LAMathObjectValue* vcapfloor = NULL;
	if ( objHolder.isDefined() )
		vcapfloor = dynamic_cast< LAMathObjectValue* >( &( objHolder.get() ) );
	else
		throw LACoreInvalidData( "vcapfloor is not set! LAPriceIRCapFloorOptionVlaue::outputResult() ",__FILE__,__LINE__);
	
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast< LALinearRatesOptionValueDataProvider* >( dataProvider );

	// ! Get Volatility, Strike, ExpiryTerm of each cashlet
	vector< vector<AnalyticParam* > > analyticParams = dataProvider->mParam;
	DoubleMatrix vols_mat;
	DoubleMatrix strikes_mat;
	DoubleMatrix expiryTerms_mat;

	double forwardShiftValue(0.);
	if (dataProvider->mVolfunc->isTypeOf(FN_VOLFUNCIRSABR))
	{
		forwardShiftValue = dynamic_cast<const LAMathVolFuncIRSABR*>(dataProvider->mVolfunc)->getForwardShiftValue();
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
	LAString cfCalcStartDateName = PRICING_DATA_CFCALCSTARTDATE_LEG + LAString( 1 );
	object.remove( cfCalcStartDateName );
	object.add( cfCalcStartDateName, new LADataDates() ).convertFromString( vcapfloor->getData( cfCalcStartDateName, ISDEFINED ).convertToString() );	
	
	// ! CFCalcEndDate
	LAString cfCalcEndDateName = PRICING_DATA_CFCALCENDDATE_LEG + LAString( 1 );
	object.remove( cfCalcEndDateName );
	object.add( cfCalcEndDateName, new LADataDates() ).convertFromString( vcapfloor->getData( cfCalcEndDateName, ISDEFINED ).convertToString() );

	// ! PaymentDate
	LAString paymentDateName = PRICING_DATA_PAYMENTDATE_LEG + LAString( 1 );
	object.remove( paymentDateName );
	object.add( paymentDateName, new LADataDates() ).convertFromString( vcapfloor->getData( paymentDateName, ISDEFINED ).convertToString() );
	
	// ! PaymentTime
	LAString time = PRICING_DATA_CASHLETVALUETIME_LEG + LAString( 1 );		
	object.remove(time);
	object.add( time, new LADataDoubles() ).convertFromString( vcapfloor->getData( time, ISDEFINED ).convertToString() );
	
	// ! Cachflow
	LAString cfname = PRICING_DATA_CASHLETVALUE_LEG + LAString( 1 );		
	object.remove( cfname );
	object.add( cfname, new LADataDoubles() ).convertFromString( vcapfloor->getData( cfname, ISDEFINED ).convertToString() );

	// ! PV
	LAString pvname = PRICING_DATA_PVVALUE_LEG + LAString( 1 );
	object.remove( pvname );
	object.add( pvname, new LADataDoubles() ).convertFromString( vcapfloor->getData( pvname, ISDEFINED ).convertToString() );

	// ! DF
	LAString dfName = PRICING_DATA_DF_LEG + LAString( 1 );
	object.remove( dfName );
	object.add( dfName, new LADataDoubles() ).convertFromString( vcapfloor->getData( dfName, ISDEFINED ).convertToString() );
	
	// ! Gearing
	LAString gearingName = PRICING_DATA_GEARING_LEG + LAString( 1 );
	object.remove( gearingName );
	object.add( gearingName, new LADataDoubles() ).convertFromString( vcapfloor->getData( gearingName, ISDEFINED ).convertToString() );

	// ! Forward
	LAString forwardName = PRICING_DATA_FORWARD_LEG + LAString( 1 );
	object.remove( forwardName );
	object.add( forwardName, new LADataDoubles() ).convertFromString( vcapfloor->getData( forwardName, ISDEFINED ).convertToString() );

	// ! AccrualDays
	LAString accDaysName = PRICING_DATA_ACCRUALDAYS_LEG + LAString( 1 );
	object.remove( accDaysName );
	object.add( accDaysName, new LADataDoubles() ).convertFromString( vcapfloor->getData( accDaysName, ISDEFINED ).convertToString() );

	// ! Notional
	LAString notionalName = PRICING_CALIBRATION_DATAOTIONAL_LEG + LAString( 1 );
	object.remove( notionalName );
	object.add( notionalName, new LADataDoubles() ).convertFromString( vcapfloor->getData( notionalName, ISDEFINED ).convertToString() );

	// ! FixingDate
	LAString fixingDateName = PRICING_DATA_FIXINGDATE_LEG + LAString( 1 );
	object.remove( fixingDateName );
	object.add( fixingDateName, new LADataDates() ).convertFromString( vcapfloor->getData( fixingDateName, ISDEFINED ).convertToString() );

	// ! FixingFlag
	LAString fixingFlagName = PRICING_DATA_FIXINGFLAG_LEG + LAString( 1 );
	object.remove( fixingFlagName );
	object.add( fixingFlagName, new LADataStrings() ).convertFromString( vcapfloor->getData( fixingFlagName, ISDEFINED ).convertToString() );

	for (int i=0; i<vols_mat.size(); i++)
	{
		// ! Volatility
		LAString volName = PRICING_DATA_VOLATILITY_LEG + LAString( 1 ) + LAString("_") + LAString( i + 1 );
		object.remove( volName );
		object.add( volName, new LADataDoubles( vols_mat[i] ) );

		// ! Strike
		LAString strikeName = PRICING_DATA_STRIKE_LEG + LAString( 1 ) + LAString("_") + LAString( i + 1 );
		object.remove( strikeName );
		object.add( strikeName, new LADataDoubles( strikes_mat[i] ) );

		// ! ExpiryTerm
		LAString expiryTermName = PRICING_DATA_EXPIRYTERM_LEG + LAString( 1 );
		object.remove( expiryTermName );
		object.add( expiryTermName, new LADataDoubles( expiryTerms_mat[i] ) );
	}

	// ! Get OptionValue and PremiumValue
	object.remove( PRICING_DATA_OPTIONVALUE );
	object.add( PRICING_DATA_OPTIONVALUE, new LADataDouble( dataProvider->mOptionValue ) );

	object.remove( PRICING_DATA_PREMIUMVALUE );
	object.add( PRICING_DATA_PREMIUMVALUE, new LADataDouble( dataProvider->mPremiumValue ) );
}

void 
LAPriceIRCapFloorOptionValue::setUpNumeraireCurrency(const LAObject& trade, LALinearRatesOptionValueDataProvider* dataProvider) const
{
    const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(legs.get(0).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
    const LAString& ccy = dynamic_cast<const LADataString&>(cashlets.get(0).get().getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
    dataProvider->mnumerairecur = ccy;
    dataProvider->mnumerairecur.toUpper();
}

bool LAPriceIRCapFloorOptionValue::hasCashflow(const LAObject& trade) const
{   
    const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    for(size_t i = 0; i < legs.getSize(); i++){
        const LADataHolder* dh = &legs.get(i).getData(PRICING_DATA_CASHLETS);
        if(!dh->isDefined() || dh->isNull()) return false;
    }
    return true;
}

double
LAPriceIRCapFloorOptionValue::value(const LADate& basedate, LAObject& inst, const LADataValuation& att) const
{
    if(hasCashflow(inst)){
        return LALinearRatesOptionValue::value(basedate, inst, att);
    }
    else{
        const double pv = 0;
        inst.remove(PRICING_DATA_DIRTYPRICE);
        inst.add(PRICING_DATA_DIRTYPRICE, new LADataDouble(pv));
        inst.remove(PRICING_DATA_CLEANPRICE);
        inst.add(PRICING_DATA_CLEANPRICE, new LADataDouble(pv));
		return pv;
    }
}
