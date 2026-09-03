/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceConvergenceValue.h"
#include "LAMathDefine.h"
#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LAPriceDataFunction.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAObjectHolder.h"
#include "LABasic.h"
#include "LAPricePayOff.h"
#include "LAPriceTradeValue.h"
#include "LALinearRatesSwapTradeValue.h"
#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LAPriceIRCapFloorOptionValue.h"
#include "LAPriceIRSwaptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LABlackScholesCapletOptionPayoff.h"
#include "LABlackScholesFloorletOptionPayoff.h"
#include "LABlackScholesCollarOptionPayoff.h"
#include "LABlackScholesStraddleOptionPayoff.h"
#include "LABlackScholesStrangleOptionPayoff.h"
#endif
#include "LADataInstance.h"
#include "LACompoundingFunc.h"


using namespace std;

#define LEG1FIXEDRATE "LEG1FIXEDRATE"
#define LEG2FIXEDRATE "LEG2FIXEDRATE"
#define LEG1SPREAD "LEG1SPREAD"
#define LEG2SPREAD "LEG2SPREAD"
#define STRIKE "STRIKE"
#define CAPSTRIKE "CAPSTRIKE"
#define FLOORSTRIKE "FLOORSTRIKE"


/*!
    @brief constructor
*/
LAPriceConvergenceValue::LAPriceConvergenceValue(void)
{
}

/*!
    @brief copy constructor
	@param[in] v copy source 
*/
LAPriceConvergenceValue::LAPriceConvergenceValue(const LAPriceConvergenceValue& v)
{
	v;
}
/*!
    @brief destructor

*/
LAPriceConvergenceValue::~LAPriceConvergenceValue()
{
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
LAPriceConvergenceValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_CONVERGENCEVALUE ? true : LACoreValuation::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceConvergenceValue::clone() const
{
    try 
	{
  		return new LAPriceConvergenceValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Return this class type

	@return this class type
*/
function_t			
LAPriceConvergenceValue::getType() const
{
	return FN_IR_CONVERGENCEVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceConvergenceValue::registerData(LAPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_CONVERGENCETARGET, DATA_STRING);
	dm.setData(PRICING_DATA_CONVERGENCEVALUE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_SUBVALUE, DATA_VALUATION);
	dm.setData(PRICING_DATA_ISFIXEDTERM, DATA_BOOL);
}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
LAPriceConvergenceValue::value(const LADate& basedate, LAObject& object,
					const LADataValuation& att) const
{
	att;
	LADataHolder *dh;
	dh = &(object.getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL));
	const double targetPV = dynamic_cast<const LADataDouble &>(dh->get()).get();

	const double EPS_PV = 1.0E-7;
	const double MAX_RATE = 3.0; // 300 % 
	//const int MAX_LOOP = 10000;
	const int MAX_LOOP = 100;
	double rate0 = getInitialValue(object);
	double initialRate = rate0;
	if (LAMath::abs(initialRate) > MAX_RATE)
	{
		// set 1.0%
		rate0 = 0.01;
	}
	LADataValuation &value = dynamic_cast<LADataValuation &>(object.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
	double val0 = targetPV - value.value(basedate);

	double rate1 = rate0 + 0.01;
	setUpCalcTarget(object, rate1);
	double val1 = targetPV - value.value(basedate);

	// newton lapson
	int loopNum = MAX_LOOP;
	try
	{
		while (loopNum--)
		{
			if (LAMath::abs(val0 - val1) < EPS_PV)
			{
				break;
			}
			double dval = (val1 - val0) / (rate1 - rate0);
			double rate2 = rate1 - val1 / dval;
			rate0 = rate1;
			rate1 = rate2;
			val0 = val1;
			setUpCalcTarget(object, rate1);
			val1 = targetPV - value.value(basedate);
		}
	}
	catch (LACoreError &e)
	{
		LAString msg = "Convergence error in Rate calc (Newton Raphson). M-Lib error message is below\n";
		msg += e.getMsg();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}


	if (loopNum < 0 && LAMath::abs(val1) >= 1.0)
	{
		throw LACoreInvalidData("Convergence error in Rate calc (Newton Raphson)", __FILE__, __LINE__); 
	}

	dh = &(object.getData(PRICING_DATA_CONVERGENCETARGET, ISNOTNULL));
	LAString calcTarget = dynamic_cast<const LADataString &>(dh->get()).get();
	calcTarget.toUpper();

	object.remove(PRICING_DATA_CONVERGENCEVALUE);
	object.add(PRICING_DATA_CONVERGENCEVALUE, new LADataDouble(rate1));
	// Initialize FixedRate for Risk
	setUpCalcTarget(object, initialRate);

	dh = &(object.getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL));
	const double dirtyprice = dynamic_cast<const LADataDouble &>(dh->get()).get();
	return dirtyprice;
}

/*!
	@brief setup calc target

	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class
	
*/
void 
LAPriceConvergenceValue::setUpCalcTarget(LAObject& object, double value) const
{
	LADataHolder *dh;
	dh = &(object.getData(PRICING_DATA_CONVERGENCETARGET, ISNOTNULL));
	LAString calcTarget = dynamic_cast<const LADataString &>(dh->get()).get();
	calcTarget.toUpper();

	LADataValuation &attrValue = dynamic_cast<LADataValuation &>(object.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
	if (attrValue.getType() == FN_IR_PLAINVANILLASWAPTRADEVALUE)
	{
		//swap
		dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
		if (legs.getSize() < 2)
		{
			throw LACoreInvalidData("Leg size need 2.", __FILE__, __LINE__);
		}
		int legSide = 0;
		dh = &(object.getData(PRICING_DATA_CONVERGENCETARGET, ISNOTNULL));
		if (calcTarget == LEG1FIXEDRATE || calcTarget == LEG1SPREAD)
		{
			legSide = 0;
		}
		else if (calcTarget == LEG2FIXEDRATE || calcTarget == LEG2SPREAD)
		{
			legSide = 1;
		}
		else
		{
			throw LACoreInvalidData("CalcTarget must be only 4 types. LEG1FIXEDRATE, LEG2FIXEDRATE, LEG1SPREAD, LEG2SPREAD", __FILE__, __LINE__);
		}

		// NDS or not
		bool isNDS = false;
		dh = &(legs.get(legSide).getData(PRICING_DATA_ORIGINALCURRENCY, NOCHECK));
		if(dh->isDefined() && !dh->isNull())
			isNDS = true;

		//cashlets
		dh = &(legs.get(legSide).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			//is first period
			bool isFirstPeriod = false;
			dh = &(cashlets.get(i).getData(PRICING_DATA_ISFIXEDTERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				isFirstPeriod = dynamic_cast<const LADataBool &>(dh->get()).get();
			}
			if (isFirstPeriod) continue;

			//coupon info
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
			LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());
			// fixed coupon rates for NDS are mapped onto first values of the coupon coefficients
			if(isNDS && (calcTarget == LEG1FIXEDRATE || calcTarget == LEG2FIXEDRATE))
			{
				dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) 
					continue;

				dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
				DoubleArray coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
				coeff.front() = value;
				dynamic_cast<LADataDoubles &>(dh->get()).set(coeff);
				continue;
			}
			else if (calcTarget == LEG1SPREAD || calcTarget == LEG2SPREAD)
			{
				
				//for ois spread or compounding spread
				//index info
				dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				
				if (couponinfos.getSize() > 2)
				{
					throw LACoreInvalidData("CouponInfo size must be 2 or less.", __FILE__, __LINE__);
				}
				for (unsigned int i = 0; i < couponinfos.getSize(); i++)  // the second couponinfo is used for stub coupon
				{
					dh = &(couponinfos.get(i).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
					DoubleArray coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
					coeff.back() = value;
					dynamic_cast<LADataDoubles &>(dh->get()).set(coeff);
				}
				continue;
		
			}
			//index info
			dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;
			LADataMultiReference &indexinfos = dynamic_cast<LADataMultiReference &>(dh->get());
			if (indexinfos.getSize() != 1)
			{
				throw LACoreInvalidData("Index size must be 1.", __FILE__, __LINE__);
			}

			dh = &(indexinfos.get(0).getData(PRICING_DATA_FIXEDRATE, ISNOTNULL));
			dynamic_cast<LADataDouble &>(dh->get()).set(value);

			}
		}
	}
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	else if (attrValue.getType() == FN_IR_SWAPTIONVALUE)
	{
		//swaption
		if (calcTarget != STRIKE)
		{
			throw LACoreInvalidData("CalcTarget must be STRIKE", __FILE__, __LINE__);
		}
		dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
		dynamic_cast<LADataDouble &>(dh->get()).set(value);

	}
	else if (attrValue.getType() == FN_IR_SWAPTIONVALUEFROMCASHFLOW)
	{
		LAObjectPool& objPool = object.getDataInstance()->getObjectPool();
		//swaption
		if (calcTarget != STRIKE)
		{
			throw LACoreInvalidData("CalcTarget must be STRIKE", __FILE__, __LINE__);
		}
		dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
		dynamic_cast<LADataDouble &>(dh->get()).set(value);

		dh = &(object.getData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			return;

		bool isequivalentmode = dynamic_cast<const LADataBool&>(dh->get()).get();
		if (!isequivalentmode)
			return;

		//get legside from "ChangeToSwap"
		LAString nb = dynamic_cast<const LADataString &>(object.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		LAString chgname = "ChangeToSwap" + nb;
		int legSide = 0;
		dh = &(objPool.getObject(chgname, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_CONVERGENCETARGET, ISNOTNULL));
		LAString swaplegstr = dynamic_cast<const LADataString &>(dh->get()).get();
		swaplegstr.toUpper();
		if (swaplegstr == LEG1FIXEDRATE)
		{
			legSide = 0;
		}
		else if (swaplegstr == LEG2FIXEDRATE)
		{
			legSide = 1;
		}
		else
		{
			throw LACoreInvalidData("ChangeToSwap target must be only 2 types. LEG1FIXEDRATE, LEG2FIXEDRATE", __FILE__, __LINE__);
		}

		//update strike info
		LAString targetname = "ChangeToOriginalSwap" + nb;
		LAObject& orgentity =  objPool.getObject(targetname, ENCHKTYPE_ISDEFINED).get();

		dh = &(orgentity.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
		if (legs.getSize() < 2)
		{
			throw LACoreInvalidData("Leg size need 2.", __FILE__, __LINE__);
		}

		//cashlets
		dh = &(legs.get(legSide).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			//coupon info
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());
				if (couponinfos.getSize() != 1)
				{
					throw LACoreInvalidData("Coupon size must be 1.", __FILE__, __LINE__);
				}

				//index info
				dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				LADataMultiReference &indexinfos = dynamic_cast<LADataMultiReference &>(dh->get());
				if (indexinfos.getSize() != 1)
				{
					throw LACoreInvalidData("Index size must be 1.", __FILE__, __LINE__);
				}

				dh = &(indexinfos.get(0).getData(PRICING_DATA_FIXEDRATE, ISNOTNULL));
				dynamic_cast<LADataDouble &>(dh->get()).set(value);	
				
			}
		}
	}
	else if (attrValue.getType() == FN_IR_CAPFLOOROPTIONVALUE)
	{
		//capfloor
		dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
		if (legs.getSize() != 1)
		{
			throw LACoreInvalidData("CapFloor Leg size must be 1.", __FILE__, __LINE__);
		}
		//cashlets
		dh = &(legs.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			//coupon info
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
			LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());
			if (couponinfos.getSize() != 1)
			{
				throw LACoreInvalidData("Coupon size must be 1.", __FILE__, __LINE__);
			}

			dh = &(couponinfos.get(0).getData(PRICING_DATA_OPERATOR, ISNOTNULL));
			const LAFunctionBase &method = dynamic_cast<const LAPriceDataFunction &>(dh->get()).getFunction();
			int funcType = method.getType();
			if (funcType == FN_IR_CAPLETOPTIONFUNC || funcType == FN_IR_FLOORLETOPTIONFUNC || funcType == FN_IR_STRADDLEOPTIONFUNC)
			{
				if (calcTarget != STRIKE)
				{
					throw LACoreInvalidData("CalcTarget must be STRIKE", __FILE__, __LINE__);
				}
			dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			DoubleArray coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
			if (coeff.size() != 2)
			{
				throw LACoreInvalidData("Coefficient size must be 2.", __FILE__, __LINE__);
			}
			coeff[1] = value;
			dynamic_cast<LADataDoubles &>(dh->get()).set(coeff);
			}
			else if (funcType == FN_IR_COLLAROPTIONFUNC || funcType == FN_IR_STRANGLEOPTIONFUNC)
			{
				dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
				DoubleArray coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
				if (coeff.size() != 3)
				{
					throw LACoreInvalidData("Coefficient size must be 3.", __FILE__, __LINE__);
				}
				if (calcTarget == CAPSTRIKE)
				{
					coeff[1] = value;
				}
				else if (calcTarget == FLOORSTRIKE)
				{
					coeff[2] = value;
				}
				else
				{
					throw LACoreInvalidData("CalcTarget must be FLOORSTRIKE nor CAPSTRIKE", __FILE__, __LINE__);
				}
				dynamic_cast<LADataDoubles &>(dh->get()).set(coeff);
			}
			else
			{
				LAString msg = "This function is not supported. function name = " + dh->convertToString();
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}
#endif
	else
	{
		LAString valueStr = attrValue.convertToString();
		LAString msg = "This SubValue is not supported. SubValue = " + valueStr;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

}



/*!
	@brief get initial value

	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	
*/
double 
LAPriceConvergenceValue::getInitialValue(LAObject& object) const
{
	LADataHolder *dh;
	dh = &(object.getData(PRICING_DATA_CONVERGENCETARGET, ISNOTNULL));
	LAString calcTarget = dynamic_cast<const LADataString &>(dh->get()).get();
	calcTarget.toUpper();

	LADataValuation &attrValue = dynamic_cast<LADataValuation &>(object.getData(PRICING_DATA_SUBVALUE, ISNOTNULL).get());
	if (attrValue.getType() == FN_IR_PLAINVANILLASWAPTRADEVALUE)
	{
		//swap
		dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
		if (legs.getSize() < 2)
		{
			throw LACoreInvalidData("Leg size need 2.", __FILE__, __LINE__);
		}
		int legSide = 0;
		dh = &(object.getData(PRICING_DATA_CONVERGENCETARGET, ISNOTNULL));
		if (calcTarget == LEG1FIXEDRATE || calcTarget == LEG1SPREAD)
		{
			legSide = 0;
		}
		else if (calcTarget == LEG2FIXEDRATE || calcTarget == LEG2SPREAD)
		{
			legSide = 1;
		}
		else
		{
			throw LACoreInvalidData("CalcTarget must be only 4 types. LEG1FIXEDRATE, LEG2FIXEDRATE, LEG1SPREAD, LEG2SPREAD", __FILE__, __LINE__);
		}

		// NDS or not
		bool isNDS = false;
		dh = &(legs.get(legSide).getData(PRICING_DATA_ORIGINALCURRENCY, NOCHECK));
		if(dh->isDefined() && !dh->isNull())
			isNDS = true;

		//cashlets
		dh = &(legs.get(legSide).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());		
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			//coupon info
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());
				// fixed coupon rates for NDS are mapped onto first values of the coupon coefficients
				if(isNDS && (calcTarget == LEG1FIXEDRATE || calcTarget == LEG2FIXEDRATE))
				{
					dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) 
						continue;

					dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
					const DoubleArray &coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
					return coeff.front();
				}
				else if (calcTarget == LEG1SPREAD || calcTarget == LEG2SPREAD)
				{

					//for ois spread or compounding spread
					//index info
					dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;
					
					dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
					const DoubleArray &coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
					//for stub
					return coeff.back();
				}
				//index info
				dh = &(couponinfos.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				LADataMultiReference &indexinfos = dynamic_cast<LADataMultiReference &>(dh->get());
				if (indexinfos.getSize() != 1)
				{
					throw LACoreInvalidData("Index size must be 1.", __FILE__, __LINE__);
				}

				dh = &(indexinfos.get(0).getData(PRICING_DATA_FIXEDRATE, ISNOTNULL));
				return dynamic_cast<LADataDouble &>(dh->get()).get();
			}
		}
		throw LACoreInvalidData("Cashlet or Coupon Info is not exist.", __FILE__, __LINE__);
	}
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	else if (attrValue.getType() == FN_IR_SWAPTIONVALUE|| attrValue.getType() == FN_IR_SWAPTIONVALUEFROMCASHFLOW)
	{
		//swaption
		if (calcTarget != STRIKE)
		{
			throw LACoreInvalidData("CalcTarget must be STRIKE", __FILE__, __LINE__);
		}
		dh = &(object.getData(PRICING_DATA_STRIKE, ISNOTNULL));
		return dynamic_cast<LADataDouble &>(dh->get()).get();

	}
	else if (attrValue.getType() == FN_IR_CAPFLOOROPTIONVALUE)
	{
		//capfloor
		dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
		if (legs.getSize() != 1)
		{
			throw LACoreInvalidData("CapFloor Leg size must be 1.", __FILE__, __LINE__);
		}
		//cashlets
		dh = &(legs.get(0).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());	
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			//coupon info
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
			LADataMultiReference &couponinfos = dynamic_cast<LADataMultiReference &>(dh->get());
			if (couponinfos.getSize() != 1)
			{
				throw LACoreInvalidData("Coupon size must be 1.", __FILE__, __LINE__);
			}

			dh = &(couponinfos.get(0).getData(PRICING_DATA_OPERATOR, ISNOTNULL));
			const LAFunctionBase &method = dynamic_cast<const LAPriceDataFunction &>(dh->get()).getFunction();
			int funcType = method.getType();
			if (funcType == FN_IR_CAPLETOPTIONFUNC || funcType == FN_IR_FLOORLETOPTIONFUNC || funcType == FN_IR_STRADDLEOPTIONFUNC)
			{
				if (calcTarget != STRIKE)
				{
					throw LACoreInvalidData("CalcTarget must be STRIKE", __FILE__, __LINE__);
				}
				dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
				const DoubleArray &coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
				if (coeff.size() != 2)
				{
					throw LACoreInvalidData("Coefficient size must be 2.", __FILE__, __LINE__);
				}
				return coeff[1];
			}
			else if (funcType == FN_IR_COLLAROPTIONFUNC || funcType == FN_IR_STRANGLEOPTIONFUNC)
			{
				dh = &(couponinfos.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
				const DoubleArray &coeff = dynamic_cast<LADataDoubles &>(dh->get()).get();
				if (coeff.size() != 3)
				{
					throw LACoreInvalidData("Coefficient size must be 3.", __FILE__, __LINE__);
				}
				if (calcTarget == CAPSTRIKE)
				{
					return coeff[1];
				}
				else if (calcTarget == FLOORSTRIKE)
				{
					return coeff[2];
				}
				else
				{
					throw LACoreInvalidData("CalcTarget must be FLOORSTRIKE nor CAPSTRIKE", __FILE__, __LINE__);
				}
			}
			else
			{
				LAString msg = "This function is not supported. function name = " + dh->convertToString();
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
		throw LACoreInvalidData("Cashlet or Coupon Info is not exist.", __FILE__, __LINE__);
	}
#endif
	else
	{
		LAString valueStr = attrValue.convertToString();
		LAString msg = "This SubValue is not supported. SubValue = " + valueStr;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

}


