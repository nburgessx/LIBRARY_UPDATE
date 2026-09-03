/*! @file
    @brief Implementation of class to generate cashlets and trigger/call schedule

*/
//  2006, Mizuho International London..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCashFlowGenerator.h"

#include <algorithm>

#include "LAObject.h"
#include "LADataProcedure.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
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
#include "LAPriceDataInterpolation.h"

#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAConstant.h"

#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"

#include "LAMathIndexEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"

#include "LAMathPlainVanillaEntity.h"
#include "LAInterpolationBase.h"
#include "LACompoundingFunc.h"
#include "LALinearFunc.h"
#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LALinearRatesOptionValueDataProvider.h"
#endif

#include "LAPriceConvergenceValue.h"
#include "LAMathYieldCurvePro.h"

#include "LAPricePayOff.h"

#include "LAPriceCouponForDigital2.h"
#include "LAPriceTradeValue.h"
//#include "LAPriceNDSSwaptionValue.h"

using namespace std;

#define CASHLET		"Cashlet"
#define COUPONINFO	"CouponInfo"
#define NOTIONALCOUPONINFO	"NotionalCouponInfo"
#define INDEXINFO	"IndexInfo"
#define ODDINDEXINFO	"OddIndexInfo"
#define RANGEACCUREINDEXINFO	"RangeAccrueIndexInfo"
#define RANGEACCUREBOUNDARYINDEXINFO	"RangeAccrueBoundaryIndexInfo"
#define RANGEINFO	"RangeInfo"
#define BUSINESS_DAYS "BUSINESS_DAYS"
#define LEG_S		"Leg"
#define EQUALIZATION	"EQUALIZATION"
#define AMOUNTSETTING	"AMOUNTSETTING"

#define DETAIL "DETAIL"
#define MANUAL	"MANUAL"
#define PAYMENTDATE "PAYMENTDATE"

#define BOTH	"BOTH"

#define FIXED_RATE	"FIXEDRATE"
#define OIS	"OIS"

#define FRN	"FRN"
#define ESTIMATION	"ESTIMATION"

#ifndef	CURRENTINDEX
#define CURRENTINDEX		"CurrentIndex"
#endif
#ifndef	CLOSESTINDEX
#define CLOSESTINDEX		"ClosestIndex"
#endif
#ifndef	NEXTINDEX
#define NEXTINDEX			"NextIndex"
#endif
#ifndef	PREVIOUSINDEX
#define PREVIOUSINDEX		"PreviousIndex"
#endif
#ifndef	STARTENDDATEMETHOD
#define STARTENDDATEMETHOD	"StartEndDateMethod"
#endif
#ifndef	SPOTRATERATIOMETHOD
#define SPOTRATERATIOMETHOD "SpotRateRatioMethod"
#endif
#ifndef	GRIDTERMRATIOMETHOD
#define GRIDTERMRATIOMETHOD "GridTermRatioMethod"
#endif

#ifndef	BUSINESS_DAYS
#define	BUSINESS_DAYS	"BUSINESS_DAYS"
#endif
#ifndef	WEEKLY
#define	WEEKLY			"WEEKLY"
#endif
#ifndef	NONE
#define	NONE			"NONE"
#endif
#ifndef	ANNUAL
#define ANNUAL		"ANNUAL"
#endif
#ifndef	SEMI_ANNUAL
#define SEMI_ANNUAL	"SEMI-ANNUAL"
#endif
#ifndef QUARTERLY
#define QUARTERLY	"QUARTERLY"
#endif
#ifndef MONTHLY
#define MONTHLY		"MONTHLY"
#endif

#ifdef VISUAL_STUDIO_2010_ANALYTICS
#define LIBOR "LIBOR"
#endif

#ifndef XCCYBASIS
#define XCCYBASIS	"XCCYBASIS"
#endif

//This definition must be coonsistent with the definition in LAMathIndexEntity.cpp
#ifndef SPOTLAG
#define SPOTLAG 0
#endif

LAPriceCashFlowGenerator::CashletCreator::CashletCreator(const LAPriceCashFlowGenerator* cf_gen_,
                                                      LAObject* leg_)
: is_first_stub(false),
  is_last_stub(false)
{
	cf_gen = cf_gen_;
    leg = leg_;
    objectPool = &leg->getDataInstance()->getObjectPool();
}

void
LAPriceCashFlowGenerator::CashletCreator::setup()
{
    LADataHolder* dh;
    
    cf_gen->calcNotionalArray(*leg, cf_size, notional_array);
    is_comp_roll               = (dh=&leg->getData(PRICING_DATA_ISCOMPOUNDINGCOUPON))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    timing                     = dynamic_cast<const LADataString&>(leg->getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL).get()).get();
    is_arrear                  = LAPriceCFGenUtility::isArrear(timing);
    is_notional_exchange_start = (dh=&leg->getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART))->isDefined() && !dh->isNull() &&  dynamic_cast<const LADataBool&>(dh->get()).get();
    is_notional_exchange_end   = (dh=&leg->getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND))->isDefined() && !dh->isNull() &&  dynamic_cast<const LADataBool&>(dh->get()).get();
    name                       = dynamic_cast<const LADataString&>(leg->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
    coupon_infos               = &dynamic_cast<const LADataMultiReference&>(leg->getData(PRICING_DATA_COUPONINFOS, ISNOTNULL).get());
    is_renotional              = (dh=&leg->getData(PRICING_DATA_ISRENOTIONAL))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    first_odd_idx_type         = (dh=&leg->getData(PRICING_DATA_FIRSTODDINDEXTYPE))->isDefined() && !dh->isNull() ? dynamic_cast<const LADataString&>(dh->get()).get() : "";
    last_odd_idx_type          = (dh=&leg->getData(PRICING_DATA_LASTODDINDEXTYPE))->isDefined() && !dh->isNull() ? dynamic_cast<const LADataString&>(dh->get()).get() : "";
    currency                   = (dh=&leg->getData(PRICING_DATA_CURRENCY))->isDefined() && !dh->isNull() ? dh : NULL;
    fx_rate                    = (dh=&leg->getData(PRICING_DATA_FXRATE))->isDefined() && !dh->isNull() ? dh : NULL;
	is_notionalcfcoupon		   = (dh=&leg->getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS))->isDefined() && !dh->isNull() ? true : false;

	dh = &leg->getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, NOCHECK);
	if(dh->isDefined() && !dh->isNull())
		notionalcfcoupon_infos = &dynamic_cast<const LADataMultiReference&>(dh->get());
}

LAObject*
LAPriceCashFlowGenerator::CashletCreator::createNotionalCashlet(const size_t cf_pos,
                                                             const LAString& name)
{
	std::unique_ptr<LAObject> ret(new LAObject());

    const LADate payment_date = payment_dates[cf_pos];




    ret->add(CALIBRATION_DATA_NAME, new LADataString(name));
    ret->add(PRICING_DATA_PAYMENTDATE, new LADataDate(payment_date));
    ret->add(PRICING_CALIBRATION_DATAOTIONAL, new LADataDouble(notional_array[cf_pos]));
    if(currency!=NULL) ret->add(PRICING_DATA_CURRENCY, *currency);
    if(fx_rate!=NULL) ret->add(PRICING_DATA_FXRATE, *fx_rate);





    
    if(cf_pos < cf_size - 1
        && (!is_notional_exchange_start || cf_pos > 0)
        && is_arrear 
        && is_notional_exchange_end
        && notional_array[cf_pos] - notional_array[cf_pos + 1] != 0)
    {
        ret->add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(notional_array[cf_pos] - notional_array[cf_pos + 1]));
    }
    if(cf_pos > 0 && cf_pos < cf_size - 1
        && !is_arrear 
        && is_notional_exchange_end
        && notional_array[cf_pos - 1] - notional_array[cf_pos] != 0)
    {
        ret->add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(notional_array[cf_pos - 1] - notional_array[cf_pos]));
    }
    if(cf_pos == cf_size - 1
        && is_notional_exchange_end)
    {
		// notionalcf coupon setup is only performed for the last cashlet
		if(is_notionalcfcoupon)
		{
			LAString notionalcfcouponinfo_name = name + "_" + NOTIONALCOUPONINFO;
			LADate advancepaymentdate = payment_dates[cf_pos-1]; 
			LADate start_date = payment_dates[cf_pos];
			LADate end_date = payment_dates[cf_pos];

			LAObjectHolder objHolder;
			objHolder.setEntity(cf_gen->createCouponInfo(notionalcfcouponinfo_name,
					advancepaymentdate, 
					payment_dates[cf_pos], 
					payment_dates_unadj[cf_pos],
					start_date,
					end_date,
					notionalcfcoupon_infos->get(0).get(), 
					*leg));
			objectPool->remove(notionalcfcouponinfo_name);
			objectPool->set(notionalcfcouponinfo_name, &objHolder.get());
			objHolder.setDeleteFlag(false);

			LADataHolder* dh = &ret->add(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, new LADataMultiReference());
			dh->convertFromString(notionalcfcouponinfo_name);
		}
		else
			ret->add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(notional_array[cf_pos]));
    }
    if(cf_pos == 0
        && is_notional_exchange_start)
    {
		LADataHolder &dh = ret->getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			LADataDouble &notionalCF = dynamic_cast<LADataDouble &>(dh.get());
			double val = notionalCF.get();
			val -= notional_array[cf_pos];
			notionalCF.set(val);
		}
		else
		{
			ret->add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(-notional_array[cf_pos]));
		}        
        if(is_renotional)
        {
            ret->remove(PRICING_CALIBRATION_DATAOTIONAL);
            ret->add(PRICING_CALIBRATION_DATAOTIONAL, new LADataDouble(0.0));

			LADataHolder* dh = &(leg->getData(PRICING_DATA_ISFIRSTNOTIONALRESET, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				if(dynamic_cast<LADataBool &>(dh->get()).get())
				{
					ret->remove(PRICING_CALIBRATION_DATAOTIONALCF);
				}
			}

			LADate fixingdate;
			dh = &(leg->getData(PRICING_DATA_RENOTIONALOFFSET, ISNOTNULL));
			int offset = dynamic_cast<const LADataInt&>(dh->get()).get();		
			//leg.remove(PRICING_DATA_RENOTIONALOFFSET);
			if (offset == 0) 
				fixingdate = payment_date;
			else
			{
				//calendar
				LAPriceDataCalendar cal;
				dh = &(leg->getData(PRICING_DATA_RENOTIONALCALENDAR, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
				}
				else
				{
					dh = &(leg->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
					cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
				}
				fixingdate = cal.getBusinessDay(payment_date, -offset);
			}
			ret->remove(PRICING_DATA_RENOTIONALFIXINGDATE);
			ret->add(PRICING_DATA_RENOTIONALFIXINGDATE, new LADataDate(fixingdate));
        }
    }

    return ret.release();
}

LAObject* 
LAPriceCashFlowGenerator::CashletCreator::createCashlet(const size_t cf_pos,
                                                     const LAString& name,
                                                     const LADate& start_date,
                                                     const LADate& end_date,
                                                     const DateVector& roll_payment_dates,
                                                     const DateVector& roll_start_dates,
                                                     const DateVector& roll_end_dates,
													 const double fixed_rate,
													 const bool is_digital,
													 const size_t cf_pos2,
													 const LAString* p_basename)
{
	const bool is_fixed_coupon = (fixed_rate > static_cast<double>(DEFAULT_SMALL_FIXEDRATE));

    std::unique_ptr<LAObject> ret(createNotionalCashlet(cf_pos, name));





    ret->add(PRICING_DATA_PAYMENTTIMING, leg->getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
    ret->add(PRICING_DATA_CFCALCSTARTDATE, new LADataDate(start_date));
    ret->add(PRICING_DATA_CFCALCENDDATE, new LADataDate(end_date));
    ret->add(PRICING_DATA_DAYCOUNT, leg->getData(PRICING_DATA_DAYCOUNT, ISNOTNULL));



    LADataHolder* dh;
    if((dh=&leg->getData(PRICING_DATA_COMPOUND_ON_ALL_DAYS))->isDefined() && !dh->isNull()){
        ret->add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(dynamic_cast<const LADataBool&>(dh->get())));
    }



    //for stub generate
	unsigned int first_cfpos = 0;
	if (is_arrear && is_notional_exchange_start)
	{
		++first_cfpos;
	}
	unsigned int last_cfpos = cf_size - 1;
	if (!is_arrear && is_notional_exchange_end)
	{
		--last_cfpos;
	}
    LAString stubindextype;
    if(first_cfpos == cf_pos && is_first_stub)
	{
		stubindextype = first_odd_idx_type;
	}
    else if(last_cfpos == cf_pos && is_last_stub)
	{
		stubindextype = last_odd_idx_type;
	}



    if(is_comp_roll && !is_fixed_coupon){
        ret->add(PRICING_DATA_CFCALCSTARTDATES, new LADataDates(roll_start_dates));
        ret->add(PRICING_DATA_CFCALCENDDATES, new LADataDates(roll_end_dates));
        ret->add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction(dynamic_cast<const LAPriceDataFunction&>(leg->getData(PRICING_DATA_COMPOUNDING_FUNCTION, ISNOTNULL).get())));
        ret->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new LADataBool(true));
    }


    LADate advancepaymentdate;
    if (!is_arrear) advancepaymentdate = payment_dates[cf_pos];
    else if (cf_pos > 0) advancepaymentdate = payment_dates[cf_pos - 1];
    else
    {
        dh = &(leg->getData(PRICING_DATA_STARTDATE, ISNOTNULL));
        const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();

        advancepaymentdate = LAPriceCFGenUtility::getDate(startdate, 
                                                       *leg, 
                                                       CALIBRATION_DATA_SLIDINGRULE, 
                                                       CALIBRATION_DATA_CALENDAR);
    }

	bool is_doublerange = false;
	dh = &(leg->getData(PRICING_DATA_ISDOUBLERANGE, NOCHECK));
    if (dh->isDefined() && !dh->isNull())
    {
        is_doublerange = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	if (is_doublerange && !p_basename)
	{
		throw LACoreInvalidData("p_basename is NULL", __FILE__, __LINE__);
	}

	if (is_doublerange && cf_pos2 > 0)
	{
		LAObject *couponinfo = new LAObject;
		LAString couponinfo_name = name + "_" + COUPONINFO + "1_" + LADataInt(cf_pos2).convertToString();

		couponinfo->remove(CALIBRATION_DATA_NAME);
		couponinfo->add(CALIBRATION_DATA_NAME, new LADataString(couponinfo_name));
		LAString fixedRateAttr = "";
		if (cf_pos2 == 1)
		{
			fixedRateAttr = PRICING_DATA_RANGEACCRUEFIXEDRATE1;
		}
		else if (cf_pos2 == 2)
		{
			fixedRateAttr = PRICING_DATA_RANGEACCRUEFIXEDRATE2;
		}
		else if (cf_pos2 == 3)
		{
			fixedRateAttr = PRICING_DATA_RANGEACCRUEFIXEDRATE3;
		}
		const double fixedRate = dynamic_cast<const LADataDouble &>(leg->getData(fixedRateAttr, ISNOTNULL).get()).get();
		const DoubleArray fixedRates(1, fixedRate);
		couponinfo->add(PRICING_DATA_COEFFICIENT, new LADataDoubles(fixedRates));
		couponinfo->add(PRICING_DATA_OPERATOR, new LAPriceDataFunction()).convertFromString(FN_CONSTANT_STR);
		couponinfo->add(PRICING_DATA_CURRENCY, leg->getData(PRICING_DATA_CURRENCY, ISNOTNULL));

		LAString couponinfo_name_base = *p_basename + "_" + COUPONINFO + "1";
		dh = &(objectPool->getObject(couponinfo_name_base, ENCHKTYPE_ISDEFINED).getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		couponinfo->add(PRICING_DATA_INDEXINFOS, dh->clone());

		dh = &ret->add(PRICING_DATA_COUPONINFOS, new LADataMultiReference());
		dh->convertFromString(couponinfo_name);
		LAObjectHolder eholder;
		eholder.setEntity(couponinfo, true);
		objectPool->remove(couponinfo_name);
		objectPool->set(couponinfo_name, &eholder.get());
		eholder.setDeleteFlag(false);

	}
	else
	{
		LAString couponinfo_str;
		for (unsigned int j = 0; j < coupon_infos->getSize(); j++)
		{
			if (is_digital && is_fixed_coupon && j != 1) continue; //if it is digital product & fixed rate, only coupon2 remains.

			if (!(is_digital && is_fixed_coupon) && j != 0) couponinfo_str += ":";

			LAString couponinfo_name = name + "_" + COUPONINFO + LADataInt(j + 1).convertToString();

			if(!is_comp_roll || stubindextype==""){
				LAObjectHolder eholder;
				eholder.setEntity(cf_gen->createCouponInfo(couponinfo_name, 
					advancepaymentdate, 
					payment_dates[cf_pos], 
					payment_dates_unadj[cf_pos],
					start_date,
					end_date,
					coupon_infos->get(j).get(), 
					*leg, 
					roll_payment_dates, 
					roll_start_dates, 
					roll_end_dates, 
					stubindextype,
					fixed_rate,
					is_digital), 
					true);
				if(is_comp_roll){
					eholder.add(PRICING_DATA_CFCALCSTARTDATE, new LADataDate(start_date));
					eholder.add(PRICING_DATA_CFCALCENDDATE, new LADataDate(end_date));
				}

				objectPool->remove(couponinfo_name);
				objectPool->set(couponinfo_name, &eholder.get());
				eholder.setDeleteFlag(false);
				couponinfo_str += couponinfo_name;
			}
			else{
				LAObjectHolder eholder;
				eholder.setEntity(cf_gen->createCouponInfo(couponinfo_name, 
					advancepaymentdate, 
					payment_dates[cf_pos], 
					payment_dates_unadj[cf_pos], 
					start_date,
					end_date,
					coupon_infos->get(j).get(), 
					*leg, 
					roll_payment_dates, 
					roll_start_dates, 
					roll_end_dates, 
					"",
					fixed_rate,
					is_digital), 
					true);

				objectPool->remove(couponinfo_name);
				objectPool->set(couponinfo_name, &eholder.get());
				eholder.setDeleteFlag(false);
				couponinfo_str += couponinfo_name;

				if(cf_pos == first_cfpos && is_first_stub){
					LAString temp_name = couponinfo_name + "_FirstStub";
					LAObjectHolder temp_eh;
					temp_eh.setEntity(cf_gen->createCouponInfo(temp_name,
						advancepaymentdate, 
						payment_dates[cf_pos], 
						payment_dates_unadj[cf_pos], 
						start_date,
						end_date,
						coupon_infos->get(j).get(), 
						*leg, 
						stubindextype,
						fixed_rate,
						is_digital), 
						true);
					temp_eh.add(PRICING_DATA_CFCALCSTARTDATE, new LADataDate(roll_start_dates[0]));
					temp_eh.add(PRICING_DATA_CFCALCENDDATE, new LADataDate(roll_end_dates[0]));

					objectPool->remove(temp_name);
					objectPool->set(temp_name, &temp_eh.get());
					temp_eh.setDeleteFlag(false);
					couponinfo_str += ":";
					couponinfo_str += temp_name;
					ret->add(PRICING_DATA_FIRSTSTUBCOUPON, new LADataString(temp_name));
				}
				if(cf_pos == last_cfpos && is_last_stub){
					LAString temp_name = couponinfo_name + "_LastStub";
					LAObjectHolder temp_eh;
					temp_eh.setEntity(cf_gen->createCouponInfo(temp_name,
						advancepaymentdate, 
						payment_dates[cf_pos], 
						payment_dates_unadj[cf_pos], 
						start_date,
						end_date,
						coupon_infos->get(j).get(), 
						*leg, 
						stubindextype,
						fixed_rate,
						is_digital), 
						true);
					temp_eh.add(PRICING_DATA_CFCALCSTARTDATE, new LADataDate(roll_start_dates.back()));
					temp_eh.add(PRICING_DATA_CFCALCENDDATE, new LADataDate(roll_end_dates.back()));

					objectPool->remove(temp_name);
					objectPool->set(temp_name, &temp_eh.get());
					temp_eh.setDeleteFlag(false);
					couponinfo_str += ":";
					couponinfo_str += temp_name;
					ret->add(PRICING_DATA_LASTSTUBCOUPON, new LADataString(temp_name));
				}
			}
		}
		dh = &ret->add(PRICING_DATA_COUPONINFOS, new LADataMultiReference());
		dh->convertFromString(couponinfo_str);
	}

	if (is_fixed_coupon)
	{
		ret->remove(PRICING_DATA_ISFIXEDTERM);
		ret->add(PRICING_DATA_ISFIXEDTERM, new LADataBool(true));
	}

    // Coupon selet oeprator
    dh = &(leg->getData(PRICING_DATA_COUPONSELECTOPERATOR, NOCHECK));
    if (dh->isDefined() && !dh->isNull() && !(is_digital && fixed_rate > static_cast<double>(DEFAULT_SMALL_FIXEDRATE))) //if it is digital product & fixed rate, remove operator.
		ret->add(PRICING_DATA_COUPONSELECTOPERATOR, *dh);

    // Round
    dh = &(leg->getData(PRICING_DATA_ROUNDFUNCTION, NOCHECK));
    if (dh->isDefined() && !dh->isNull())
    {
        ret->add(PRICING_DATA_ROUNDFUNCTION, *dh);
        dh = &(leg->getData(PRICING_DATA_ROUNDDIGIT, ISNOTNULL));
        ret->add(PRICING_DATA_ROUNDDIGIT, *dh);
    }

    // isamortize
    dh = &(leg->getData(PRICING_DATA_ISAMORTIZE, ISNOTNULL));
    bool isamortize = dynamic_cast<const LADataBool&>(dh->get()).get();

    // DENOMINATOR
	dh = &(leg->getData(PRICING_DATA_DENOMINATOR, NOCHECK));
    if (dh->isDefined() && !dh->isNull())
    {
        double denominator = dynamic_cast<const LADataDouble&>(dh->get()).get();

        if (isamortize)
            ret->add(PRICING_DATA_DENOMINATOR, new LADataDouble(denominator / notional_array[0] * notional_array[cf_pos]));
        else
            ret->add(PRICING_DATA_DENOMINATOR, new LADataDouble(denominator));
    }

    if (is_renotional)
    {
		if(isamortize)
		{
			throw LACoreInvalidData("amortization on renotional cashflow is not supported.", __FILE__, __LINE__);
		}
        LADate advancepaymentdate;
        if (!is_arrear) advancepaymentdate = payment_dates[cf_pos];
        else if (cf_pos > 0) advancepaymentdate = payment_dates[cf_pos - 1];
        else
        {
            dh = &(leg->getData(PRICING_DATA_STARTDATE, ISNOTNULL));
            const LADate& temp_startdate = dynamic_cast<const LADataDate&>(dh->get()).get();

            advancepaymentdate = LAPriceCFGenUtility::getDate(temp_startdate, 
                                                           *leg, 
                                                           CALIBRATION_DATA_SLIDINGRULE, 
                                                           CALIBRATION_DATA_CALENDAR);
        }

        cf_gen->createRenotionalInfo(advancepaymentdate, 
                                     payment_dates[cf_pos], 
                                     payment_dates_unadj[cf_pos], 
                                     start_date,
                                     end_date,
                                     *ret.get(), 
                                     *leg);
    }

    // Range Accrue
    dh = &(leg->getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK));
    if (dh->isDefined() && !dh->isNull())
    {
        if (dynamic_cast<const LADataBool &>(dh->get()).get())
        {
            ret->add(PRICING_DATA_ISRANGEACCRUE, new LADataBool(true));
			
			// range accrue strike
			dh = &(leg->getData(PRICING_DATA_RANGEACCRUESTRIKE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_RANGEACCRUESTRIKE, *dh);
			}

			dh = &(leg->getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
            {
				const LADataMultiReference& rangeInfos = dynamic_cast<const LADataMultiReference&>(dh->get());
				LAString rangeinfo_str;
				if (is_doublerange)
				{
					if (rangeInfos.getSize() != 2)
					{
						throw LACoreInvalidData("Double Range must have 2 Range Infos", __FILE__, __LINE__);
					}
					rangeinfo_str = *p_basename + "_" + RANGEINFO + "1:" + *p_basename + "_" + RANGEINFO + "2";
					if (cf_pos2 == 0)
					{
						for (unsigned int j = 0; j < rangeInfos.getSize(); j++)
						{
							const LAString rangeinfo_name = name + "_" + RANGEINFO + LADataInt(j + 1).convertToString();
							LAObjectHolder eholder;
							eholder.setEntity(cf_gen->createRangeAccrueInfo(rangeinfo_name, rangeInfos.get(j).get(), 
								start_date, end_date, is_arrear, cf_pos, payment_dates, payment_dates_unadj), true);
							objectPool->remove(rangeinfo_name);
							objectPool->set(rangeinfo_name, &eholder.get());
							eholder.setDeleteFlag(false);
						}
					}
					dh = &ret->add(PRICING_DATA_RANGEACCRUEINFOS, new LADataMultiReference());
					dh->convertFromString(rangeinfo_str);

					if (cf_pos2 == 0)
					{
						ret->add(PRICING_DATA_ISANDCONDITION, new LADataBool(false));
					}
					else if (cf_pos2 == 1)
					{
						ret->add(PRICING_DATA_ISANDCONDITION, new LADataBool(true));
					}
					else if (cf_pos2 == 2)
					{
						ret->add(PRICING_DATA_ISANDCONDITION, new LADataBool(false));
						ret->add(PRICING_DATA_ISEXCLUDEANDCONDITION, new LADataBool(true));
					}
					else if (cf_pos2 == 3)
					{
						ret->add(PRICING_DATA_ISANDCONDITION, new LADataBool(false));
						ret->add(PRICING_DATA_ISNOTCONDITION, new LADataBool(true));
					}
				}
				else
				{	
					for (unsigned int j = 0; j < rangeInfos.getSize(); j++)
					{
						if (j != 0) rangeinfo_str += ":";
						const LAString rangeinfo_name = name + "_" + RANGEINFO + LADataInt(j + 1).convertToString();
						LAObjectHolder eholder;
						eholder.setEntity(cf_gen->createRangeAccrueInfo(rangeinfo_name, rangeInfos.get(j).get(), 
							start_date, end_date, is_arrear, cf_pos, payment_dates, payment_dates_unadj), true);
						objectPool->remove(rangeinfo_name);
						objectPool->set(rangeinfo_name, &eholder.get());
						eholder.setDeleteFlag(false);

						rangeinfo_str += rangeinfo_name;
					}
					dh = &ret->add(PRICING_DATA_RANGEACCRUEINFOS, new LADataMultiReference());
					dh->convertFromString(rangeinfo_str);

					// range accrue condition is And or OR
					dh = &(leg->getData(PRICING_DATA_ISANDCONDITION, ISNOTNULL));
					if (dh->isDefined() && !dh->isNull())
					{
						ret->add(PRICING_DATA_ISANDCONDITION, *dh);
					}				
				}
			}
			else
			{
				// range accrue indexinfos
				dh = &(leg->getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL));
				const LADataMultiReference& raindexinfos
					= dynamic_cast<const LADataMultiReference&>(dh->get());
				LAString raindexinfo_str;
				for (unsigned int j = 0; j < raindexinfos.getSize(); j++)
				{
					if (j != 0) raindexinfo_str += ":";

					LAString raindexinfo_name = name + "_" + RANGEACCUREINDEXINFO + LADataInt(j + 1).convertToString();

					LAObjectHolder eholder;

					// observation offset
					dh = &(leg->getData(PRICING_DATA_RANGEACCRUEOBSERVATIONOFFSET, NOCHECK));
					LADate obsSDate = start_date;
					LADate obsEDate = end_date;
					if(dh->isDefined() && !dh->isNull())
					{
						int offset = dynamic_cast<const LADataInt&>(dh->get()).get();
						LAString offset_str = LAString(offset) + LAString("d");
						const LAPriceDataSlidingRule& srl
							= dynamic_cast<const LAPriceDataSlidingRule& >(leg->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
						const LAPriceDataCalendar& cal 
							= dynamic_cast<const LAPriceDataCalendar& >(leg->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
						// range observation start date
						obsSDate = LAMathDateCalculations::getDate(start_date, offset_str, srl, &cal, false);
						// range observation end date
						obsEDate = LAMathDateCalculations::getDate(end_date, offset_str, srl, &cal, false);
					}

					eholder.setEntity(cf_gen->createRangeAccrueIndexInfo(raindexinfo_name, 
						obsSDate, obsEDate, raindexinfos.get(j).get()), true);
					objectPool->remove(raindexinfo_name);
					objectPool->set(raindexinfo_name, &eholder.get());
					eholder.setDeleteFlag(false);

					raindexinfo_str += raindexinfo_name;

				}
				dh = &ret->add(PRICING_DATA_RANGEACCRUEINDEXINFOS, new LADataMultiReference());
				dh->convertFromString(raindexinfo_str);

				// range accrue operator
				dh = &(leg->getData(PRICING_DATA_RANGEACCRUEOPERATOR, ISNOTNULL));
				ret->add(PRICING_DATA_RANGEACCRUEOPERATOR, *dh);
				// range accrue coefficient
				dh = &(leg->getData(PRICING_DATA_RANGEACCRUECOEFFICIENT, ISNOTNULL));
				ret->add(PRICING_DATA_RANGEACCRUECOEFFICIENT, *dh);
				// range accrue max
				dh = &(leg->getData(PRICING_DATA_RANGEACCRUEMAX, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					ret->add(PRICING_DATA_RANGEACCRUEMAX, *dh);
				}
				// range accrue min
				dh = &(leg->getData(PRICING_DATA_RANGEACCRUEMIN, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					ret->add(PRICING_DATA_RANGEACCRUEMIN, *dh);
				}

				// boundary index infos
				dh = &(leg->getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK));
				if(dh->isDefined() && !dh->isNull())
				{
					const LADataMultiReference& boundaryIndexInfos 
						= dynamic_cast<const LADataMultiReference&>(dh->get());

					LAString boundaryIndexInfo_str;
					for (unsigned int j = 0; j < boundaryIndexInfos.getSize(); j++)
					{
						if (j != 0) boundaryIndexInfo_str += ":";

						LAString indexinfo_name 
							= name + "_" + RANGEACCUREBOUNDARYINDEXINFO + LADataInt(j + 1).convertToString();

						LADate advancepaymentdate;
						if (!is_arrear) advancepaymentdate = payment_dates[cf_pos];
						else if (cf_pos > 0) advancepaymentdate = payment_dates[cf_pos - 1];
						else
						{
							dh = &(leg->getData(PRICING_DATA_STARTDATE, ISNOTNULL));
							const LADate& temp_startdate = dynamic_cast<const LADataDate&>(dh->get()).get();

							advancepaymentdate = LAPriceCFGenUtility::getDate(temp_startdate, *leg, 
								CALIBRATION_DATA_SLIDINGRULE, CALIBRATION_DATA_CALENDAR);
						}

						DateVector tmpdates;
						LAObjectHolder eholder;
						eholder.setEntity(cf_gen->createIndexInfo(indexinfo_name, 
																  advancepaymentdate, 
																  payment_dates[cf_pos], 
																  payment_dates_unadj[cf_pos], 
																  start_date, 
																  end_date, 
																  boundaryIndexInfos.get(j).get(),
																  *leg,
																  tmpdates,
																  tmpdates,
																  tmpdates), true);
						objectPool->remove(indexinfo_name);
						objectPool->set(indexinfo_name, &eholder.get());
						eholder.setDeleteFlag(false);

						boundaryIndexInfo_str += indexinfo_name;
					}
					dh = &ret->add(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, new LADataMultiReference());
					dh->convertFromString(boundaryIndexInfo_str);

					// range accrue max boundary operator
					dh = &(leg->getData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR, ISNOTNULL));
					ret->add(PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR, *dh);

					// range accrue min boundary operator
					dh = &(leg->getData(PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR, ISNOTNULL));
					ret->add(PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR, *dh);

					// range accrue max boundary coefficient
					dh = &(leg->getData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT, ISNOTNULL));
					ret->add(PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT, *dh);

					// range accrue min boundary coefficient
					dh = &(leg->getData(PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT, ISNOTNULL));
					ret->add(PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT, *dh);
				}
			}
            // range accrue same observation date
            dh = &(leg->getData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS, NOCHECK));
            if (dh->isDefined() && !dh->isNull())
            {
                ret->add(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS, *dh);
            }
            // range accrue businessdays base
            dh = &(leg->getData(PRICING_DATA_BUSINESSDAYSBASE, NOCHECK));
            if (dh->isDefined() && !dh->isNull())
            {
                ret->add(PRICING_DATA_BUSINESSDAYSBASE, *dh);
            }

            // observation offset
            dh = &(leg->getData(PRICING_DATA_RANGEACCRUEOBSERVATIONOFFSET, NOCHECK));
            if(dh->isDefined() && !dh->isNull())
            {
                int offset = dynamic_cast<const LADataInt&>(dh->get()).get();
                LAString offset_str = LAString(offset) + LAString("d");
                const LAPriceDataSlidingRule& srl
                    = dynamic_cast<const LAPriceDataSlidingRule& >(leg->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
                const LAPriceDataCalendar& cal 
                    = dynamic_cast<const LAPriceDataCalendar& >(leg->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
                // range observation start date
                LADate obsSDate = LAMathDateCalculations::getDate(start_date, offset_str, srl, &cal, false);
                dh = &ret->add(PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE, new LADataDate());
                dynamic_cast<LADataDate&>(dh->get()).set(obsSDate);
                // range observation end date
                LADate obsEDate = LAMathDateCalculations::getDate(end_date, offset_str, srl, &cal, false);
                dh = &ret->add(PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE, new LADataDate());
                dynamic_cast<LADataDate&>(dh->get()).set(obsEDate); 
            }

			// operator for range accrue hit rate
			dh = &(leg->getData(PRICING_DATA_HITRATEOPERATOR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_HITRATEOPERATOR, *dh);
			}				
			// coefficient for range accrue hit rate
			dh = &(leg->getData(PRICING_DATA_HITRATECOEFFICIENT, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_HITRATECOEFFICIENT, *dh);
			}				
			// min for range accrue hit rate
			dh = &(leg->getData(PRICING_DATA_HITRATEMIN, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_HITRATEMIN, *dh);
			}				
			// max for range accrue hit rate
			dh = &(leg->getData(PRICING_DATA_HITRATEMAX, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_HITRATEMAX, *dh);
			}				
			// operator for final coupon
			dh = &(leg->getData(PRICING_DATA_OPERATOR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_OPERATOR, *dh);
			}				
			// coefficient for final coupon
			dh = &(leg->getData(PRICING_DATA_COEFFICIENT, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				ret->add(PRICING_DATA_COEFFICIENT, *dh);
			}				
		}
    }

	// non-deliverable swaps
	dh = &leg->getData(PRICING_DATA_SETTLEMENTCURRENCY, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		ret->add(PRICING_DATA_SETTLEMENTCURRENCY, new LADataString(dynamic_cast<LADataString&>(dh->get())));

		const LAPriceDataSlidingRule* psrule = NULL;
		const LAPriceDataCalendar* pcal = NULL;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(*leg,
			PRICING_DATA_SETTLEMENTFIXINGSLIDINGRULE,
			PRICING_DATA_SETTLEMENTFIXINGCALENDAR,
			*leg,
			PRICING_DATA_SETTLEMENTFIXINGSLIDINGRULE,
			PRICING_DATA_SETTLEMENTFIXINGCALENDAR,
			psrule, pcal);
		const int offset = dynamic_cast<const LADataInt&>((leg->getData(PRICING_DATA_SETTLEMENTFIXINGOFFSET, NOCHECK)).get());
		const LAString offset_str = LAString(offset) + LAString("d");
		const LADate paydate = dynamic_cast<const LADataDate&>(ret->getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get());
		const LADate fixgdate = LAMathDateCalculations::getDate(paydate, offset_str, *psrule, pcal, false);
		ret->add(PRICING_DATA_SETTLEMENTFIXINGDATE, new LADataDate(fixgdate));
	}

    return ret.release();
}

///////////////////////////////////////////////////////////////////////
/*!
    @brief default constructor
*/
LAPriceCashFlowGenerator::LAPriceCashFlowGenerator()
: LACoreProcedure()
{
}
/*!
    @brief destructor

*/
LAPriceCashFlowGenerator::~LAPriceCashFlowGenerator()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
LAPriceCashFlowGenerator::isTypeOf(function_t id) const
{
	return (id == FN_IR_CASHFLOWGENERATOR ? true :
						LACoreProcedure::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to original object
*/
LACoreFunctionBase*		
LAPriceCashFlowGenerator::clone() const
{
    try 
	{
        return new LAPriceCashFlowGenerator();
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Retern class type

	@return class type
*/
function_t			
LAPriceCashFlowGenerator::getType() const
{
	return FN_IR_CASHFLOWGENERATOR;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceCashFlowGenerator::registerData(LAPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_STARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ENDTERM, DATA_STRING);
	dm.setData(PRICING_DATA_FREQUENCY, DATA_STRING);
	dm.setData(PRICING_DATA_PAYMENTTIMING, DATA_STRING);
	dm.setData(PRICING_DATA_FIXINGTIMING, DATA_STRING);
	dm.setData(PRICING_DATA_ACTIONTIMING, DATA_STRING);
	dm.setData(PRICING_DATA_FIRSTODDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_LASTODDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_FIRSTODDINDEXTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_LASTODDINDEXTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_ODDINDEXINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_COUPONDAY, DATA_INT);
	dm.setData(PRICING_DATA_COUPONINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_CASHLETS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_PAYMENTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_CFCALCSTARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_CFCALCENDDATE, DATA_DATE);
	dm.setData(PRICING_CALIBRATION_DATAOTIONAL, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_FXRATE, DATA_REFERENCE);
	dm.setData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, DATA_BOOL);
	dm.setData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, DATA_BOOL);
	dm.setData(PRICING_DATA_DAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_EXTRACF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_COUPONSELECTOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_INDEXINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_OBSERVATIONSTARTTERM, DATA_STRING);
	dm.setData(PRICING_DATA_OBSERVATIONSTARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_OBSERVATIONENDTERM, DATA_STRING);
	dm.setData(PRICING_DATA_OBSERVATIONENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_OBSERVATIONSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_OBSERVATIONCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_OBSERVATIONSAMEDAYS, DATA_INT);
	dm.setData(PRICING_DATA_OFFSET, DATA_INT);
	dm.setData(PRICING_DATA_FIXINGSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_FIXINGCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_FIXINGSPECIALOFFSET, DATA_STRING);
	dm.setData(PRICING_DATA_FIXINGSPECIALDAY, DATA_INTS);
	dm.setData(PRICING_DATA_FIXINGDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ISAMORTIZE, DATA_BOOL);
	dm.setData(PRICING_DATA_AMORTIZETYPE, DATA_STRING);
	dm.setData(PRICING_DATA_AMORTIZEAMOUNT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_AMORTIZEROUNDFUNCTION, DATA_STRING);
	dm.setData(PRICING_DATA_AMORTIZEROUNDDIGIT, DATA_INT);
	dm.setData(PRICING_DATA_ISAMORTIZE1STFRACTION, DATA_BOOL);
	dm.setData(PRICING_DATA_ISFIXINGPAYMENTDATEBASE, DATA_BOOL);
	dm.setData(PRICING_DATA_CASHFLOWSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_CASHFLOWCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_CALLINFO, DATA_REFERENCE);
	dm.setData(PRICING_DATA_TRIGGERINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_INPUTTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_ACTIONSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_ACTIONCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_ACTIONOFFSET, DATA_INT);
	dm.setData(PRICING_DATA_ACTIONFIXINGCALENDAR, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_ACTIONFIXINGSLIDINGRULE, DATA_CALENDAR);
	dm.setData(PRICING_DATA_ACTIONFIXINGSPECIALOFFSET, DATA_STRING);
	dm.setData(PRICING_DATA_ACTIONFIXINGSPECIALDAY, DATA_INTS);
	dm.setData(PRICING_DATA_ACTIONDATES, DATA_DATES);
	dm.setData(PRICING_DATA_EXPIRYDATES, DATA_DATES);
	dm.setData(PRICING_DATA_EXTRACFOFFSET, DATA_INT);
//	dm.setData(PRICING_DATA_EXTRACFS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_EXTRACFDATES, DATA_DATES);
	dm.setData(PRICING_DATA_INDEXENTITY, DATA_REFERENCE);
	dm.setData(PRICING_DATA_STARTTERM, DATA_STRING);
	dm.setData(PRICING_DATA_TARGETLEG, DATA_STRING);
	dm.setData(PRICING_DATA_ISACTIONDETAIL, DATA_BOOL);
	dm.setData(PRICING_DATA_ACTIONSTARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ACTIONENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ACTIONFREQUENCY, DATA_STRING);
	dm.setData(PRICING_DATA_ACTIONFIRSTODDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ACTIONLASTODDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ACTIONDAY, DATA_INT);
	dm.setData(PRICING_DATA_ACTIONSTARTTERM, DATA_STRING);
	dm.setData(PRICING_DATA_ACTIONENDTERM, DATA_STRING);
	dm.setData(PRICING_DATA_FIXEDTERMS, DATA_STRINGS);
	dm.setData(PRICING_DATA_FIXEDDATES, DATA_DATES);
	dm.setData(PRICING_DATA_FIXEDRATES, DATA_DOUBLES);
	dm.setData(PRICING_DATA_FIXEDRATE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_COUPONCHANGEINFO, DATA_REFERENCE);
	dm.setData(CALIBRATION_DATA_UNDERLYINGS, DATA_MULTIREFERENCE);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_DENOMINATOR, DATA_DOUBLE);
	dm.setData(PRICING_DATA_ROUNDFUNCTION, DATA_STRING);
	dm.setData(PRICING_DATA_ROUNDDIGIT, DATA_INT);	
	dm.setData(PRICING_DATA_OBSERVATIONFREQUENCY, DATA_STRING);	
	dm.setData(PRICING_DATA_OBSERVATIONDAY, DATA_INT);	
	dm.setData(PRICING_DATA_INDEXTYPE, DATA_STRING);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_EXTRACFFUNC, DATA_FUNCTION);
	dm.setData(CALIBRATION_DATA_FEEAMOUNTS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_FEEPAYMENTDATES, DATA_DATES);
	dm.setData(CALIBRATION_DATA_FEECURRENCIES, DATA_STRINGS);
	dm.setData(CALIBRATION_DATA_FEEDISCOUNTCURVES, DATA_STRINGS);

	dm.setData(PRICING_DATA_ISFUNDINGCHANGE, DATA_BOOL);
	dm.setData(PRICING_DATA_FUNDINGTARGETLEG, DATA_STRING);
	dm.setData(PRICING_DATA_BASEFUNDINGCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_BASEFUNDINGSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BASEFUNDINGSPREADS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_BASEFUNDINGNOTIONAL, DATA_DOUBLE);
	dm.setData(PRICING_DATA_BASEFUNDINGDAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_BASEFUNDINGSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_BASEFUNDINGCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT, DATA_DAYCOUNT);

	dm.setData(PRICING_DATA_MODIFIEDFUNDINGCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGNOTIONAL, DATA_DOUBLE);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGSPREADS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, DATA_DAYCOUNT);

	dm.setData(PRICING_DATA_BASEFUNDINGFORECASTCURVE, DATA_STRING);
	dm.setData(PRICING_DATA_MODIFIEDFUNDINGFORECASTCURVE, DATA_STRING);
	dm.setData(PRICING_DATA_FUNDINGCHANGEINFO,	DATA_REFERENCE);
	
	dm.setData(PRICING_DATA_EXTRACFFORINITIALEXCHANGE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_FXFORINITIALEXCHANGE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_FUNDINGSPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_OBSERVATIONDATES, DATA_DATES);
	dm.setData(PRICING_DATA_OBSERVATIONRATES, DATA_DOUBLES);
	dm.setData(PRICING_DATA_OBSERVATIONOPERATOR, DATA_FUNCTION);
	//range accrue
	dm.setData(PRICING_DATA_ISRANGEACCRUE, DATA_BOOL);
	dm.setData(PRICING_DATA_RANGEACCRUEINDEXINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_RANGEACCRUEOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_RANGEACCRUESTRIKE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUECOEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_RANGEACCRUEMAX, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUEMIN, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS, DATA_INT);
	dm.setData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDATE,DATA_DATE);
	dm.setData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS,DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR,DATA_FUNCTION);
	dm.setData(PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR,DATA_FUNCTION);
	dm.setData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT,DATA_DOUBLES);
	dm.setData(PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT,DATA_DOUBLES);
	dm.setData(PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE,DATA_DATE);
	dm.setData(PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE,DATA_DATE);
	dm.setData(PRICING_DATA_RANGEACCRUEOBSERVATIONOFFSET, DATA_INT);
	dm.setData(PRICING_DATA_RANGEACCRUEINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_ISANDCONDITION, DATA_BOOL);
	dm.setData(PRICING_DATA_ISNOTCONDITION, DATA_BOOL);
	dm.setData(PRICING_DATA_ISEXCLUDEANDCONDITION, DATA_BOOL);
	dm.setData(PRICING_DATA_BUSINESSDAYSBASE, DATA_BOOL);
	dm.setData(PRICING_DATA_ISDOUBLERANGE, DATA_BOOL);
	dm.setData(PRICING_DATA_RANGEACCRUEFIXEDRATE1, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUEFIXEDRATE2, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUEFIXEDRATE3, DATA_DOUBLE);
	dm.setData(PRICING_DATA_HITRATEOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_HITRATECOEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_HITRATEMAX, DATA_DOUBLE);
	dm.setData(PRICING_DATA_HITRATEMIN, DATA_DOUBLE);

	dm.setData( PRICING_DATA_ISRENOTIONAL, DATA_BOOL );
	dm.setData( PRICING_DATA_RENOTIONALFIXINGDATE, DATA_DATE );
	dm.setData(PRICING_DATA_ISFORWARDROLL, DATA_BOOL);
	dm.setData(PRICING_DATA_DATESFORINDEXGENERATE, DATA_DATES);
	dm.setData(PRICING_DATA_INDEXGENERATEMETHOD, DATA_STRING);
	dm.setData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, DATA_BOOL);
	dm.setData(PRICING_DATA_ISFIRSTNOTIONALRESET, DATA_BOOL);
	dm.setData(PRICING_DATA_RENOTIONALCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_RENOTIONALOFFSET, DATA_INT);
	dm.setData(PRICING_DATA_RENOTIONALTIMING, DATA_STRING);
	dm.setData(PRICING_DATA_RENOTIONALSLIDINGRULE, DATA_SLIDINGRULE);
	
	dm.setData(PRICING_DATA_ROLLCONVENTION, DATA_STRING);
	dm.setData(PRICING_DATA_ROLLFREQUENCY, DATA_STRING);

	dm.setData(PRICING_DATA_FIRSTFIXINGRATE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_FIRSTFIXINGBASEDATE, DATA_DATE);

	dm.setData(PRICING_DATA_ISBONDFIXED, DATA_BOOL);
	dm.setData(CALIBRATION_DATA_ISRENOTIONALSETUPFINISHED, DATA_BOOL);
	dm.setData(PRICING_DATA_ISBONDFUNDINGLEG, DATA_BOOL);
	dm.setData(PRICING_DATA_BONDFUNDINGCURRENCY, DATA_STRING);

	// Data for ODMS reverse mapping (not used in a valuation)
	dm.setData(CALIBRATION_DATA_COUPONEXRATE, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_NOTIONALUNIT, DATA_DOUBLE);

	// used for NDS
	dm.setData(PRICING_DATA_ISNONDELIVERABLELEG, DATA_BOOL);
	dm.setData(PRICING_DATA_ORIGINALCURRENCY, DATA_STRING);	

	dm.setData(PRICING_DATA_CONVEXITYADJUSTMENT, DATA_STRING);
}


/*!
	@brief Generate CashFlow

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
LAPriceCashFlowGenerator::calibrateModel(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const
{

	(void)basedate; (void)att; //20070411--Nagase--åxçêçÌèúÇgccÇ…Ç‡ëŒâû
	//setup leg maturity
	setUpLegMaturity(object);


	LADataHolder* dh;

	//for equivalent strike for swaption value
	setUpEquivalentStrike(object);

	//leg object
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
	//create Cashlets
    for (unsigned int i = 0; i < legs.getSize(); i++){
        modifyLegInfo(legs.get(i).get());
		createCashlets(legs.get(i).get());
    }

	//wheather if we will exit without any special modification of the trade
	dh = &object.getData(PRICING_DATA_DONOTMODIFYTRADE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		if(dynamic_cast<LADataBool&>(dh->get()).get())
			return;
	}

	//for completetion of creating bond funding
	setUpBondFundingLeg(object);

	//for stub swap
	createOddTradeEntity(object);
	
	//setup call schedule
	setUpCallSchedule(object);
	//setup trigger schedule
	setUpTriggerSchedule(object);

	// setup for non-deliverabale products
	setUpNonDeliverable(object);

	//setup fundingchange
	setUpFundingChange(object);

	// ! Setup renotional for mark to market currency swap
	setUpRenotional( object );


}

/*!
	@brief Create cashlets

	@param[in, out] leg object
*/
void
LAPriceCashFlowGenerator::createCashlets(LAObject& leg) const  
{
    LAString input_type = dynamic_cast<const LADataString&>(leg.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL).get()).get();
    input_type.toUpper();
    if(input_type == MANUAL) return;

	clearCashlets(leg);
    CashletCreator creator(this, &leg);

    DateVector paymentdates, paymentdates_unadjust, startdates, enddates;
    DateVector rolldates, rolldates_unadjust, rollstartdates, rollenddates;
	bool is_first_stub(false), is_last_stub(false);
    calcPaymentDates(leg,
                     paymentdates, paymentdates_unadjust, startdates, enddates,
                     rolldates, rolldates_unadjust, rollstartdates, rollenddates,
					 is_first_stub, is_last_stub);
    const unsigned int cf_size = paymentdates.size();
    creator.setCfSize(cf_size);
    creator.setPyamentDates(paymentdates, paymentdates_unadjust);
    creator.setStubType(is_first_stub, is_last_stub);
    creator.setup();



    const LAString leg_name = dynamic_cast<const LADataString&>(leg.getData(CALIBRATION_DATA_NAME).get()).get();


	// for fixed terms
	DateVector fixeddates;
	DoubleArray fixedrates;
	bool is_digital = false;
	LADataHolder* dh = &(leg.getData(PRICING_DATA_FIXEDTERMS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		fixeddates = getFixedDates(leg);
		fixedrates = dynamic_cast<const LADataDoubles&>(leg.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL).get()).get();
		if (fixeddates.size() != fixedrates.size())
		{
			//error
			LAString msg = "FixedRates is not same size as FixedDates.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		// for digital product
		dh = &(leg.getData(PRICING_DATA_COUPONSELECTOPERATOR, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAString& cpnop = dynamic_cast<const LAPriceDataFunction&>(dh->get()).convertToString();
			if (cpnop.findString(FN_CPNSLTOPERATORFORDIGITAL2_STR) != -1) is_digital = true;
		}
	}
	bool is_doublerange = false;
	dh =  &(leg.getData(PRICING_DATA_ISDOUBLERANGE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		is_doublerange = dynamic_cast<const LADataBool&>(dh->get()).get();
	}

    LAString cashlet_names = "";
    unsigned int k = 0;              // indices for start/end dates
    unsigned int sPos = 0, ePos = 0, payPos = 0; // indices for compound dates
    for(unsigned int i = 0; i < cf_size; i++){
        if (i != 0) cashlet_names += ":";
		const LAString cashlet_name = leg_name + "_" + CASHLET + LADataInt(i + 1).convertToString();
		cashlet_names += cashlet_name;

		const bool is_notional_exchange_only =
			(i == 0 && creator.getIsArrear() && creator.getIsNotionalExchangeStart()) ||
			(i == cf_size - 1 && !creator.getIsArrear() && creator.getIsNotionalExchangeEnd());

        DateVector comppayvec,compstartvec,compendvec;
        if (creator.getIsCompRoll() && !is_notional_exchange_only)
        {
            unsigned int allSize = rolldates.size();
            if (allSize != rolldates_unadjust.size())
                throw LACoreInvalidData("roll generator size error",__FILE__,__LINE__);

            if (!LAAlgorithm::find<DateVector, LADate>(rollenddates, enddates[k],0, rollenddates.size() -1, ePos))
                throw LACoreInvalidData("roll end date error",__FILE__,__LINE__);

            unsigned int rollSize = ePos + 1 -sPos;

            comppayvec.resize(rollSize);
            compstartvec.resize(rollSize);
            compendvec.resize(rollSize);

            if (rollstartdates.size() != rollenddates.size())
                throw LACoreInvalidData("roll generator size error",__FILE__,__LINE__);

            unsigned int scfPos = 0;
            if (!LAAlgorithm::find<DateVector, LADate>(rollstartdates, startdates[k],0, rollstartdates.size() -1, scfPos))
                throw LACoreInvalidData("roll calcd start date error",__FILE__,__LINE__);

			if (creator.getIsArrear() && creator.getIsNotionalExchangeStart())
				payPos = sPos + 1;
			else 
				payPos = sPos;
            for (unsigned int l = 0; l < rollSize ; l++)
            {
                comppayvec[l] = rolldates[l+ payPos];
                compstartvec[l] = rollstartdates[l + scfPos];
                compendvec[l] = rollenddates[l + scfPos];
            }
            //end
            sPos = ePos + 1;
        }

		// get fixed rate
		double fixedrate = static_cast<double>(DEFAULT_SMALL_FIXEDRATE);
		for (unsigned int l = 0; l < fixeddates.size(); ++l)
		{
			if (fixeddates[l] >= enddates[k])
			{
				fixedrate = fixedrates[l];
				break;
			}
		}

        std::unique_ptr<LAObject> cashlet;
		if (is_doublerange)
		{
			if(is_notional_exchange_only){
				cashlet.reset(creator.createNotionalCashlet(i, cashlet_name));
			}
			else{
				cashlet.reset(creator.createCashlet(i,
													cashlet_name,
													startdates[k],
													enddates[k],
													comppayvec,
													compstartvec,
													compendvec,
													fixedrate,
													is_digital,
													0,
													&cashlet_name));
			}

			LAObjectPool& objPool = leg.getDataInstance()->getObjectPool();
			objPool.remove(cashlet_name);
			objPool.set(cashlet_name, cashlet.release());


			unsigned int fixedrate_size = 3;
			for (unsigned int l = 0; l < fixedrate_size; ++l)
			{
				const LAString cashlet_name2 = cashlet_name + "_" + LADataInt(l + 1).convertToString();
				cashlet.reset(creator.createCashlet(i,
													cashlet_name2,
													startdates[k],
													enddates[k],
													comppayvec,
													compstartvec,
													compendvec,
													fixedrate,
													is_digital,
													l + 1,
													&cashlet_name));
				objPool.remove(cashlet_name2);
				objPool.set(cashlet_name2, cashlet.release());
				cashlet_names += ":" + cashlet_name2;

			}
			k++;
			
		}
		else
		{
			if(is_notional_exchange_only){
				cashlet.reset(creator.createNotionalCashlet(i, cashlet_name));
			}
			else{
				cashlet.reset(creator.createCashlet(i,
													cashlet_name,
													startdates[k],
													enddates[k],
													comppayvec,
													compstartvec,
													compendvec,
													fixedrate,
													is_digital));
				k++;
			}

			LAObjectPool& objPool = leg.getDataInstance()->getObjectPool();
			objPool.remove(cashlet_name);
			objPool.set(cashlet_name, cashlet.release());
		}
    }
	leg.add(PRICING_DATA_CASHLETS, new LADataMultiReference()).convertFromString(cashlet_names);
}

/*!
	@brief	Create range accrueal information object that is refered from Coupon informarion object refered from cashlet object 
	@param[in] startdate startdate of interest calculation(after holiday adjust)
	@param[in] enddate enddate of interest calculation(after holiday adjust)
	@param[in] rangeinfo information object that represents range accrue (master)informarion
	
	@return	pointer of object that represents index information

*/
LAObject*
LAPriceCashFlowGenerator::createRangeAccrueInfo(const LAString name, const LAObject& rangeinfo, const LADate& startdate, 
											 const LADate& enddate, const bool is_arrear, const int cf_pos, 
											 const DateVector& payment_dates, const DateVector& payment_dates_unadj) const
{
	const LADataHolder *dh;
	LAObject *prangeinfo = rangeinfo.clone();
	LAObjectHolder objHolder;
	objHolder.setEntity(prangeinfo, true);
	LAObjectPool& objPool = rangeinfo.getDataInstance()->getObjectPool();

	// name
	LADataHolder *ahNm = &(prangeinfo->getData(CALIBRATION_DATA_NAME, NOCHECK));
	if (!ahNm->isDefined())
	{
		ahNm = &(prangeinfo->add(CALIBRATION_DATA_NAME, new LADataString()));
	}
    ahNm->convertFromString(name);

    // range accrue indexinfos
    dh = &(rangeinfo.getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL));
    const LADataMultiReference& raindexinfos = dynamic_cast<const LADataMultiReference&>(dh->get());
    LAString raindexinfo_str;
    for (unsigned int j = 0; j < raindexinfos.getSize(); j++)
    {
        if (j != 0) raindexinfo_str += ":";

        LAString raindexinfo_name = name + "_" + RANGEACCUREINDEXINFO + LADataInt(j + 1).convertToString();

        LAObjectHolder eholder;

        // observation offset
        dh = &(rangeinfo.getData(PRICING_DATA_RANGEACCRUEOBSERVATIONOFFSET, NOCHECK));
        LADate obsSDate = startdate;
        LADate obsEDate = enddate;
        if(dh->isDefined() && !dh->isNull())
        {
            int offset = dynamic_cast<const LADataInt&>(dh->get()).get();
            LAString offset_str = LAString(offset) + LAString("d");
            const LAPriceDataSlidingRule& srl
                = dynamic_cast<const LAPriceDataSlidingRule& >(rangeinfo.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
            const LAPriceDataCalendar& cal 
                = dynamic_cast<const LAPriceDataCalendar& >(rangeinfo.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
            // range observation start date
            obsSDate = LAMathDateCalculations::getDate(startdate, offset_str, srl, &cal, false);
            // range observation end date
            obsEDate = LAMathDateCalculations::getDate(enddate, offset_str, srl, &cal, false);
        }
		
        eholder.setEntity(this->createRangeAccrueIndexInfo(raindexinfo_name, obsSDate, obsEDate, 
															raindexinfos.get(j).get()), true);
		objPool.remove(raindexinfo_name);
		objPool.set(raindexinfo_name, &eholder.get());
        eholder.setDeleteFlag(false);

        raindexinfo_str += raindexinfo_name;
    }
	prangeinfo->remove(PRICING_DATA_RANGEACCRUEINDEXINFOS);
    LADataHolder* ahIdx = &prangeinfo->add(PRICING_DATA_RANGEACCRUEINDEXINFOS, new LADataMultiReference());
    ahIdx->convertFromString(raindexinfo_str);

    // boundary index infos
    dh = &(rangeinfo.getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK));
    if(dh->isDefined() && !dh->isNull())
    {
        const LADataMultiReference& boundaryIndexInfos 
            = dynamic_cast<const LADataMultiReference&>(dh->get());

        LAString boundaryIndexInfo_str;
        for (unsigned int j = 0; j < boundaryIndexInfos.getSize(); j++)
        {
            if (j != 0) boundaryIndexInfo_str += ":";

            LAString indexinfo_name 
                = name + "_" + RANGEACCUREBOUNDARYINDEXINFO + LADataInt(j + 1).convertToString();

            LADate advancepaymentdate;
            if (!is_arrear) advancepaymentdate = payment_dates[cf_pos];
            else if (cf_pos > 0) advancepaymentdate = payment_dates[cf_pos - 1];
            else
            {
                dh = &(rangeinfo.getData(PRICING_DATA_STARTDATE, ISNOTNULL));
                const LADate& temp_startdate = dynamic_cast<const LADataDate&>(dh->get()).get();

                advancepaymentdate = LAPriceCFGenUtility::getDate(temp_startdate, *prangeinfo, 
                    CALIBRATION_DATA_SLIDINGRULE, CALIBRATION_DATA_CALENDAR);
            }

            DateVector tmpdates;
            LAObjectHolder eholder;
            eholder.setEntity(this->createIndexInfo(indexinfo_name, 
                                                      advancepaymentdate, 
                                                      payment_dates[cf_pos], 
                                                      payment_dates_unadj[cf_pos], 
                                                      startdate, 
                                                      enddate, 
                                                      boundaryIndexInfos.get(j).get(),
                                                      *prangeinfo,
                                                      tmpdates,
                                                      tmpdates,
                                                      tmpdates), true);
            objPool.remove(indexinfo_name);
            objPool.set(indexinfo_name, &eholder.get());
            eholder.setDeleteFlag(false);

            boundaryIndexInfo_str += indexinfo_name;
        }
		prangeinfo->remove(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS);
        ahIdx = &prangeinfo->add(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, new LADataMultiReference());
        ahIdx->convertFromString(boundaryIndexInfo_str);
    }
	objHolder.setDeleteFlag(false);
	return prangeinfo;
}

/*!
	@brief	Calculate payment dates and interest calculation dates

	@param[in, out] leg object
	@param[out] paymentdates paymentdates (after holiday adjust)
	@param[out] paymentdates_unadjust paymentdates (before holiday adjust)
	@param[out] startdates startdates of interest calculation(after holiday adjust)
	@param[out] enddates enddates of interest calculation(after holiday adjust)
	@param[in] string of frequency if empty, frequency is called from frequency data. 
*/
void
LAPriceCashFlowGenerator::calcPaymentDates(LAObject& leg, 
										DateVector& paymentdates,
										DateVector& paymentdates_unadjust,
										DateVector& startdates,
										DateVector& startdates_unadjust,
										DateVector& enddates,
										DateVector& enddates_unadjust,
                                        const LADate& start_date,
                                        const LADate& end_date,
										const LAString& freq,
                                        const LADate* first_odd_date,
                                        const LADate* last_odd_date) const
{
	const LADataHolder* dh;
	
	//isArrear
	dh = &(leg.getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
	const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
	bool isarrear = LAPriceCFGenUtility::isArrear(timing);

	//coupon day
	int day;
	const int* pday = NULL;
	dh = &(leg.getData(PRICING_DATA_COUPONDAY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		day = dynamic_cast<const LADataInt&>(dh->get()).get();
		pday = &day;
	}

	//roll convention
	const LAString* pRollConv = NULL;
	dh = &(leg.getData(PRICING_DATA_ROLLCONVENTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		pRollConv = &dynamic_cast<const LADataString&>(dh->get()).get();
	}



	// isNotionalChangeAtStat
    const bool isnotionalchangestart = (dh=&leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    const bool isnotionalchangeend = (dh=&leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();


	//isforwardroll
	bool isforwardroll = true;
	dh = &(leg.getData(PRICING_DATA_ISFORWARDROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		isforwardroll = dynamic_cast<const LADataBool&>(dh->get()).get();

	
	///////////////////////////////////////
	//payment dates before holiday adjust//
	///////////////////////////////////////
	DateVector tmp_paymentdates_unadjust;
	const LAPriceDataSlidingRule* pSldRule = NULL;
	const LAPriceDataCalendar* pCal = NULL;

	if(freq == BUSINESS_DAYS || freq == DAILY)
	{
		dh = &(leg.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
		pSldRule = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());

		dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, NOCHECK));
		if(dh->isDefined() && !dh->isNull()){
			pCal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		}
		
		isforwardroll = true;
	}
	LAMathDateCalculations::generateSchedule(start_date, 
		                           end_date, 
								   freq, 
								   isarrear,
								   first_odd_date, 
								   last_odd_date, 
								   pday, 
								   tmp_paymentdates_unadjust, 
								   pSldRule, 
								   pCal, 
								   isforwardroll, 
								   pRollConv);
	
	paymentdates_unadjust = tmp_paymentdates_unadjust;
	if (isarrear && isnotionalchangestart)
		paymentdates_unadjust.insert(paymentdates_unadjust.begin(), start_date);
	else if(!isarrear && isnotionalchangeend)
		paymentdates_unadjust.push_back(end_date);
		

	//////////////////////////////////////
	//payment dates after holiday adjust//
	//////////////////////////////////////
	
	paymentdates = paymentdates_unadjust;
	//sliding rule
	dh = &(leg.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
	const LAPriceDataSlidingRule& srule = dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		//calendar
		dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
		const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		for (unsigned int i = 0; i < paymentdates_unadjust.size(); i++)		
			paymentdates[i] = srule.getDate(paymentdates_unadjust[i], cal);
	}
	if (paymentdates.size() > 1 && paymentdates[0] == paymentdates[1])
	{
		paymentdates.erase(paymentdates.begin());
		paymentdates_unadjust.erase(paymentdates_unadjust.begin());
	}
	if (paymentdates.size() > 1 && paymentdates.back() == paymentdates[paymentdates.size() - 2])
	{
		paymentdates.erase(paymentdates.end() - 1);
		paymentdates_unadjust.erase(paymentdates_unadjust.end() - 1);
	}

	///////////////////////////
	//startdates and enddates//
	///////////////////////////
	startdates.resize(tmp_paymentdates_unadjust.size());
	enddates.resize(tmp_paymentdates_unadjust.size());
	if (isarrear)
	{
		startdates[0] = start_date;
		for (unsigned int i = 1; i < startdates.size(); i++)
			startdates[i] = tmp_paymentdates_unadjust[i - 1];
		enddates = tmp_paymentdates_unadjust;
	}
	else
	{	
		startdates = tmp_paymentdates_unadjust;
		for (unsigned int i = 1; i < startdates.size(); i++)
			enddates[i - 1] = tmp_paymentdates_unadjust[i];
		enddates[enddates.size() - 1] = end_date;
	}
	startdates_unadjust = startdates;
	enddates_unadjust = enddates;

	const LAPriceDataSlidingRule* psrule = &srule;
	const LAPriceDataCalendar* pcal = NULL;
	//sliding rule
	dh = &(leg.getData(PRICING_DATA_CASHFLOWSLIDINGRULE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		psrule = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		dh = &(leg.getData(PRICING_DATA_CASHFLOWCALENDAR, NOCHECK));
		if (dh->isDefined() && !dh->isNull())	
			pcal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
		else
		{
            dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
			pcal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
		}
	}

	if (pcal != NULL)
	{
		for (unsigned int i = 0; i < startdates.size(); i++)		
		{
			startdates[i] = psrule->getDate(startdates[i], *pcal);
			enddates[i] = psrule->getDate(enddates[i], *pcal);
		}	
	}



}

/*!
	@brief	Calculate notional correspond to each payment date

	@param[in] leg leg object
	@param[in] cfsize cashflow number
	@param[out] notional_array notional array 

*/
void
LAPriceCashFlowGenerator::calcNotionalArray(const LAObject& leg,
										const unsigned int cfsize,
										DoubleArray& notional_array) const
{
	notional_array.clear();
	
	const LADataHolder* dh;
	// notional
	dh = &(leg.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL));
	double notional = dynamic_cast<const LADataDouble&>(dh->get()).get();

	// isamortize
	const bool isamortize = (dh = &leg.getData(PRICING_DATA_ISAMORTIZE))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
	

	if (!isamortize)
	{
		notional_array.resize(cfsize, notional);
		return;
	}

	// isArrear payment
	dh = &(leg.getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
	const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
	bool isarrear = LAPriceCFGenUtility::isArrear(timing);
	
	// isNotionalChangeAtStat/End
    const bool isnotionalchangestart = (dh = &leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    const bool isnotionalchangeend = (dh = &leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();

	unsigned int couponcfsize = cfsize;
	if (isnotionalchangestart && isnotionalchangeend) couponcfsize -= 1;

	// IsFirstFraction
	dh = &(leg.getData(PRICING_DATA_ISAMORTIZE1STFRACTION, ISNOTNULL));
	bool isfirstfraction = dynamic_cast<const LADataBool&>(dh->get()).get();


	// AmortizeType
	dh = &(leg.getData(PRICING_DATA_AMORTIZETYPE, ISNOTNULL));
	LAString amortize_type = dynamic_cast<const LADataString&>(dh->get()).get();
	amortize_type.toUpper();
	double amount;
	if (amortize_type == EQUALIZATION)
	{
		// notional payment of one time
		amount = notional / couponcfsize;
		dh = &(leg.getData(PRICING_DATA_AMORTIZEROUNDFUNCTION, ISNOTNULL));
		const LAString& roundfunction = dynamic_cast<const LADataString&>(dh->get()).get();
		dh = &(leg.getData(PRICING_DATA_AMORTIZEROUNDDIGIT, ISNOTNULL));
		int rounddigit = dynamic_cast<const LADataInt&>(dh->get()).get();
		amount = LAPriceCFGenUtility::round(amount, roundfunction, rounddigit);		
	}
	else if (amortize_type == AMOUNTSETTING)
	{
		dh = &(leg.getData(PRICING_DATA_AMORTIZEAMOUNT, ISNOTNULL));
		amount = dynamic_cast<const LADataDouble&>(dh->get()).get();
	}
	else
	{
		//error
		LAString msg = "AmortizeType : " + amortize_type;
		msg += " is not support";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//fraction
	double fraction = notional - amount * couponcfsize;

	if (isarrear && isnotionalchangestart) notional_array.push_back(notional);
		
	double remains = notional;
	for (unsigned int i = 0; i < couponcfsize; i++)
	{
		notional_array.push_back(remains);
		if (i == couponcfsize - 1) break;
		if (isfirstfraction && i == 0) remains -= (amount + fraction);
		else remains -= amount;
	}

	if (!isarrear && isnotionalchangeend) notional_array.push_back(remains);

}

/*!
	@brief	Clear cashlets
	@param[in, out] leg leg object  
*/
void
LAPriceCashFlowGenerator::clearCashlets(LAObject& leg) const
{
	LADataHolder* dh;

	// object pool
	LAObjectPool& objPool = leg.getDataInstance()->getObjectPool();

	// reference to cachlets 
	dh = &(leg.getData(PRICING_DATA_CASHLETS, NOCHECK));
	if (!dh->isDefined()) return;


	//////////////////////////////////////////
	// if there exist cashlets, delete them //
	//////////////////////////////////////////
	LADataMultiReference& cashlets
		= dynamic_cast<LADataMultiReference&>(dh->get());
	
	while (cashlets.getSize() > 0)
	{
		LAObjectHolder& objHolder = cashlets.get(0);
		// coupon infos
		dh = &(objHolder.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LADataMultiReference& couponinfos
				= dynamic_cast<LADataMultiReference&>(dh->get());

			while (couponinfos.getSize() > 0)
			{		
				LAObjectHolder& eh2 = couponinfos.get(0);
				LAString couponinfo_name = eh2.getName();
				// index infos
				dh = &(eh2.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
				LADataMultiReference& indexinfos
					= dynamic_cast<LADataMultiReference&>(dh->get());
			
				// remove this couponinfo
				couponinfos.remove(couponinfo_name);

				// remove indexinfos from object pool;
				while (indexinfos.getSize() > 0)
				{
					LAString indexinfo_name = indexinfos.get(0).getName();
					indexinfos.remove(indexinfo_name);
					objPool.remove(indexinfo_name);
				}
				// remove couponinfo from object pool;
				objPool.remove(couponinfo_name);
			}
		}
		LAString cashlet_name = objHolder.getName();
		// remove this cashlets
		cashlets.remove(cashlet_name);
		// remove cashlet from object pool
		objPool.remove(cashlet_name);
	}
	
	leg.remove(PRICING_DATA_CASHLETS);
}



/*!
	@brief	Create coupon information object that is refered from cashlet object
	@param[in] name name of coupon information
	@param[in] advancepaymentdate(after holiday adjust) advance paymantdate of this coupon payment(arrear case).this coupon paymentdate(advance case).					
	@param[in] paymentdate(after holiday adjust) 
	@param[in] paymentdate_unadjust(before holiday adjust) 
	@param[in] startdate startdate of interest calculation(after holiday adjust)
	@param[in] enddate enddate of interest calculation(after holiday adjust)
	@param[in] couponinfo coupon information object that represents coupon (master)informarion
	@param[in] leg leg object
	@param[in] stub index type
	
	@return	pointer of object that represents coupon information
*/

LAObject*
LAPriceCashFlowGenerator::createCouponInfo(const LAString name,
										const LADate& advancepaymentdate,
										const LADate& paymentdate,
										const LADate& paymentdate_unadjust, 
										const LADate& startdate,
										const LADate& enddate,
										const LAObject& couponinfo,
										const LAObject& leg,
										const DateVector& rolldates,
										const DateVector& rollstartdates,
										const DateVector& rollenddates,
										LAString stubindextype,
										const double fixedrate,
										const bool is_digital
										) const
{
	LADataHolder* dh;
	LAObjectHolder eholder;
    LAObject* pcouponinfo = couponinfo.clone();
	eholder.setEntity(pcouponinfo, true);
	
	// name
	dh = &(pcouponinfo->getData(CALIBRATION_DATA_NAME, NOCHECK));
	if (!dh->isDefined())
		dh = &(pcouponinfo->add(CALIBRATION_DATA_NAME, new LADataString()));
    dh->convertFromString(name);
	
	// object pool
	LAObjectPool& objPool = couponinfo.getDataInstance()->getObjectPool();
	
	// for fixed rate
	if (is_digital && fixedrate > static_cast<double>(DEFAULT_SMALL_FIXEDRATE))
	{
		DoubleArray coeffs;
		coeffs.push_back(1.0);
		coeffs.push_back(0.0);
		pcouponinfo->remove(PRICING_DATA_COEFFICIENT);
		pcouponinfo->add(PRICING_DATA_COEFFICIENT, new LADataDoubles(coeffs));
	}

	// index infos
	const LADataHolder* cah = &(couponinfo.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
	const LADataMultiReference& indexinfos
		= dynamic_cast<const LADataMultiReference&>(cah->get());

    const LADataHolder* ch;
    const bool compounding_all_days = (ch=&leg.getData(PRICING_DATA_COMPOUND_ON_ALL_DAYS))->isDefined() && !ch->isNull() && dynamic_cast<const LADataBool&>(ch->get()).get();

	LAString indexinfo_str;
	LAObjectHolder objHolder;
	for (unsigned int i = 0; i < indexinfos.getSize(); i++)
	{
		if (i != 0) indexinfo_str += ":";
			
		LAString indexinfo_name = name + "_" + INDEXINFO + LADataInt(i + 1).convertToString();

        indexinfos.get(i).remove(PRICING_DATA_COMPOUND_ON_ALL_DAYS);
        indexinfos.get(i).add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(compounding_all_days));
		objHolder.setEntity(createIndexInfo(indexinfo_name, 
                                     advancepaymentdate,
			                         paymentdate, 
                                     paymentdate_unadjust, 
                                     startdate, 
                                     enddate, 
                                     indexinfos.get(i).get(),
                                     leg,
			                         rolldates,
                                     rollstartdates,
                                     rollenddates,
									 fixedrate), 
                     true);
		objPool.remove(indexinfo_name);
		objPool.set(indexinfo_name, &objHolder.get());
		objHolder.setDeleteFlag(false);
			
		indexinfo_str += indexinfo_name;
	}
	dh = &(pcouponinfo->getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
	dh->convertFromString(indexinfo_str);
				
	// observation enddate
	dh = &(pcouponinfo->getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAString& endstr = dynamic_cast<const LADataString&>(dh->get()).get();

		const LAPriceDataSlidingRule* psrule;
		const LAPriceDataCalendar* pcal;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(*pcouponinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													leg, 
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);		

		const LADate& edate
			= LAMathDateCalculations::getDate(paymentdate_unadjust, endstr, *psrule, pcal, false);	

		pcouponinfo->add(PRICING_DATA_OBSERVATIONENDDATE, new LADataDate(edate));
		pcouponinfo->remove(PRICING_DATA_OBSERVATIONENDTERM);	


		//observation startdate
		LADate sdate;
		dh = &(pcouponinfo->getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			sdate = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				sdate = psrule->getDate(sdate, *pcal);
		}
		else
		{
			dh = &(pcouponinfo->getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
			const LAString& startstr = dynamic_cast<const LADataString&>(dh->get()).get();
			sdate = LAMathDateCalculations::getDate(paymentdate_unadjust, startstr, *psrule, pcal, false);	
			
			pcouponinfo->add(PRICING_DATA_OBSERVATIONSTARTDATE, new LADataDate(sdate));
			pcouponinfo->remove(PRICING_DATA_OBSERVATIONSTARTTERM);	
		}
	
		if (edate < sdate)
		{
			//error
			LAString msg = "ObservationEndDate is before ObservationStartDate";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}
	}
	
	pcouponinfo->remove(PRICING_DATA_ODDINDEXINFOS);
	if (stubindextype.size() > 0 && stubindextype != CURRENTINDEX )
	{

		// index infos
		//const LADataHolder* stubcah = &(couponinfo.getData(PRICING_DATA_ODDINDEXINFOS, ISNOTNULL));
		const LADataHolder* stubcah = &(couponinfo.getData(PRICING_DATA_ODDINDEXINFOS));
		if (stubcah->isDefined() && !stubcah->isNull())
		{
			const LADataMultiReference& stubindexinfos
				= dynamic_cast<const LADataMultiReference&>(stubcah->get());

			LAString stubindexinfo_str;
			for (unsigned int i = 0; i < stubindexinfos.getSize(); i++)
			{
				if (i != 0) stubindexinfo_str += ":";
					
				LAString stubindexinfo_name = name + "_" + ODDINDEXINFO + LADataInt(i + 1).convertToString();
					
				objHolder.setEntity(createIndexInfo(stubindexinfo_name, 
                                             advancepaymentdate,
					                         paymentdate, 
                                             paymentdate_unadjust, 
                                             startdate, 
                                             enddate, 
                                             stubindexinfos.get(i).get(),
                                             leg,
					                         rolldates,
                                             rollstartdates,
                                             rollenddates,
											 fixedrate), 
                             true);
				objPool.remove(stubindexinfo_name);
				objPool.set(stubindexinfo_name, &objHolder.get());
				objHolder.setDeleteFlag(false);
					
				stubindexinfo_str += stubindexinfo_name;
			}

			//reset indexinfo
			LAString mdyindexinfo = indexinfo_str;
			if (stubindexinfo_str != "")
				mdyindexinfo += ":" + stubindexinfo_str;
			
			dh = &(pcouponinfo->getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
			dh->convertFromString(mdyindexinfo);

			//check operator
			unsigned int mdyindexsize = mdyindexinfo.toToken(':').size();
			dh = &(pcouponinfo->getData(PRICING_DATA_OPERATOR, ISNOTNULL));
			LAPriceDataFunction& couponope = dynamic_cast<LAPriceDataFunction &>(dh->get());
			//if (!couponope.isTypeOf(FN_LINEAR) && !couponope.isTypeOf(FN_CONSTANT))
			//	throw LACoreInvalidData("OddOperatorError",__FILE__,__LINE__);
			//reset coefficient
			dh = &(pcouponinfo->getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			DoubleVector orgcoeffs = dynamic_cast<LADataDoubles &>(dh->get()).get();

			unsigned int addsize = stubindexinfos.getSize();
			//DoubleVector oddcoeffs(orgcoeffs.size() + addsize, -9999.0);
			DoubleVector oddcoeffs(orgcoeffs.size() + addsize, orgcoeffs.front());
			oddcoeffs.back() = orgcoeffs.back();
			dynamic_cast<LADataDoubles &>(dh->get()).set(oddcoeffs);
		}
		else //in case of no odd indexinfo but must change coefficient into -9999 (corresponding to STARTENDDATEMETHOD)
		{
			//reset coefficient
			dh = &(pcouponinfo->getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
			DoubleVector orgcoeffs = dynamic_cast<LADataDoubles &>(dh->get()).get();
			
			//DoubleVector oddcoeffs(orgcoeffs.size(), -9999.0);
			DoubleVector oddcoeffs(orgcoeffs.size(), orgcoeffs.front());
			oddcoeffs.back() = orgcoeffs.back();
			dynamic_cast<LADataDoubles &>(dh->get()).set(oddcoeffs);
		}
		
		pcouponinfo->add(PRICING_DATA_ODDINDEXTYPE, new LADataString(stubindextype));
	}

    //if(rolldates.size() > 0){
    //    pcouponinfo->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new LADataBool(true));
    //}
	
	eholder.setDeleteFlag(false);
	return pcouponinfo;
}

/*!
	@brief	Create index information object that is refered from Coupon informarion object refered from cashlet object 
	@param[in] name name of index information
	@param[in] advancepaymentdate(after holiday adjust) advance paymantdate of this coupon payment
	@param[in] paymentdate(after holiday adjust) 
	@param[in] paymentdate_unadjust(before holiday adjust) 
	@param[in] startdate startdate of interest calculation(after holiday adjust)
	@param[in] enddate enddate of interest calculation(after holiday adjust)
	@param[in] index information object that represents index (master)informarion
	
	@return	pointer of object that represents index information

*/
LAObject*
LAPriceCashFlowGenerator::createIndexInfo(const LAString name,
										const LADate& advancepaymentdate,
										const LADate& paymentdate,
										const LADate& paymentdate_unadjust, 
										const LADate& startdate,
										const LADate& enddate,
										const LAObject& indexinfo,
                                        const LAObject& leg,
										const DateVector& rolldates,
										const DateVector& rollstartdates,
										const DateVector& rollenddates,
										const double fixedrate
										) const
{
	LADataHolder* dh;
	LAObject* pindex = indexinfo.clone();
	LAObjectHolder objHolder;
	objHolder.setEntity(pindex, true);

	
	// name
	dh = &(pindex->getData(CALIBRATION_DATA_NAME, NOCHECK));
	if (!dh->isDefined())
		dh = &(pindex->add(CALIBRATION_DATA_NAME, new LADataString()));
    dh->convertFromString(name);

	// for fixed rate
	if (fixedrate > static_cast<double>(DEFAULT_SMALL_FIXEDRATE))
	{
		pindex->remove(PRICING_DATA_INDEXNAME);
		pindex->remove(PRICING_DATA_INDEXTYPE);
		pindex->remove(PRICING_DATA_FIXEDRATE);
		pindex->remove(PRICING_DATA_FIXINGDATE);
		pindex->add(PRICING_DATA_INDEXNAME, new LADataString(PRICING_DATA_FIXEDRATE));
		pindex->add(PRICING_DATA_INDEXTYPE, new LADataString(PRICING_DATA_FIXEDRATE));
		pindex->add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrate));
		LADate date;
		const LADataHolder& dah = leg.getData(CALIBRATION_DATA_TRADEDATE, NOCHECK);
		if (dah.isDefined() && !dah.isNull())
			date = dynamic_cast<const LADataDate&>(dah.get()).get();
		else
			date = dynamic_cast<const LADataDate&>(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
		pindex->add(PRICING_DATA_FIXINGDATE, new LADataDate(date));

		pindex->remove(IR_MODEL_DATA_ACCESSORY);
		pindex->remove(CALIBRATION_DATA_CALENDAR);
		pindex->remove(IR_MODEL_DATA_CURRENCY);
		pindex->remove(IR_MODEL_DATA_DAYCOUNT);
		pindex->remove(IR_MODEL_DATA_FREQUENCY);
		pindex->remove(CALIBRATION_DATA_SLIDINGRULE);
		pindex->remove(PRICING_DATA_SPOTLAG);
		pindex->remove(PRICING_DATA_BASISCURVE);
		pindex->remove(PRICING_DATA_BASISINTERPOLATION);
		pindex->remove(PRICING_DATA_FIXINGTIMING);
		pindex->remove(PRICING_DATA_OFFSET);
		pindex->remove(PRICING_DATA_FIXINGCALENDAR);
		pindex->remove(PRICING_DATA_FIXINGSLIDINGRULE);
		pindex->remove(PRICING_DATA_INDEXGENERATEMETHOD);
		pindex->remove(PRICING_DATA_VOLATILITYUNDERLYING);
	}

	//index type
	dh = &(pindex->getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
	LAString indextype = dynamic_cast<const LADataString&>(dh->get());
	indextype.toUpper();
	if (indextype == FIXED_RATE) 
	{
		objHolder.setDeleteFlag(false);
		return pindex;
	}

	LADate fixingdate;

	// observation enddate
	dh = &(pindex->getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{

		const LAString& endstr = dynamic_cast<const LADataString&>(dh->get()).get();
	
		const LAPriceDataSlidingRule* psrule;
		const LAPriceDataCalendar* pcal;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(*pindex, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													*pindex, 
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);		
		const LADate& edate
			= LAMathDateCalculations::getDate(paymentdate_unadjust, endstr, 
										*psrule,
										pcal,
										false);	
		
		pindex->add(PRICING_DATA_OBSERVATIONENDDATE, new LADataDate(edate));
		pindex->remove(PRICING_DATA_OBSERVATIONENDTERM);		

		// observation startdate
		LADate sdate;
		dh = &(pindex->getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			sdate = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				sdate = psrule->getDate(sdate, *pcal);		
		}
		else
		{
			dh = &(pindex->getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
			const LAString& startstr = dynamic_cast<const LADataString&>(dh->get()).get();
			sdate = LAMathDateCalculations::getDate(paymentdate_unadjust, startstr, 
										*psrule,
										pcal,
										false);
			
			pindex->add(PRICING_DATA_OBSERVATIONSTARTDATE, new LADataDate(sdate));
			pindex->remove(PRICING_DATA_OBSERVATIONSTARTTERM);	

		}
		
		fixingdate = edate;
		if (edate < sdate)
		{
			//error
			LAString msg = "ObservationEndDate is before ObservationStartDate";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}
	
		// fixing date
        dh = &(pindex->getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (!dh->isDefined())
			dh = &pindex->add(PRICING_DATA_FIXINGDATE, new LADataDate());
		dynamic_cast<LADataDate&>(dh->get()).set(fixingdate); 
		
	}
	else if (rolldates.size() > 0)
	{
		// LIBOR Compounding

		// isArrear
		dh = &pindex->getData(PRICING_DATA_FIXINGTIMING);
        if(!dh->isDefined() || dh->isNull()){
            const LADataHolder* temp_ah = &leg.getData(PRICING_DATA_FIXINGTIMING, ISNOTNULL);
            dh = &pindex->add(PRICING_DATA_FIXINGTIMING, new LADataString());
            dh->convertFromString(temp_ah->convertToString());
        }
		const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
		bool isarrear = LAPriceCFGenUtility::isArrear(timing);
		

		bool isfixingpaybase = true;
		dh = &(pindex->getData(PRICING_DATA_ISFIXINGPAYMENTDATEBASE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			isfixingpaybase = dynamic_cast<const LADataBool &>(dh->get());

		DateVector fixingbasedates(rolldates.size());
		DateVector fixingdates(rolldates.size());
		for (unsigned int i = 0; i < rolldates.size(); i++)
		{
			if (isfixingpaybase && !isarrear)
				fixingbasedates[i]= (0==i) ? advancepaymentdate : rolldates[i-1];
			else if (isfixingpaybase && isarrear)
				fixingbasedates[i]= rolldates[i];
			else if (isfixingpaybase && !isarrear)
				fixingbasedates[i]= rollstartdates[i];
			else 
				fixingbasedates[i]= rollenddates[i];

			dh = &(pindex->getData(PRICING_DATA_OFFSET, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				int offset = dynamic_cast<const LADataInt&>(dh->get()).get();		
				if (offset == 0) 
					fixingdates[i] = fixingbasedates[i];
				else
				{
					//calendar
					LAPriceDataCalendar cal;
					dh = &(pindex->getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
					}
					else
					{
						dh = &(pindex->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
						cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
					}
					fixingdates[i] = cal.getBusinessDay(fixingbasedates[i], -offset);
				}
			}
			else // case of fixing special date
			{
				dh = &(pindex->getData(PRICING_DATA_FIXINGSPECIALOFFSET, ISNOTNULL));
				LAString offset = dynamic_cast<const LADataString&>(dh->get()).get();		
				
				
				dh = &(pindex->getData(PRICING_DATA_FIXINGSPECIALDAY, ISNOTNULL));
				IntArray specialday = dynamic_cast<const LADataInts&>(dh->get()).get();	
				if (specialday.size() != 12)
				{
					//error
					LAString msg = "fixing special day size must be 12";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
				}
				
				const LAPriceDataSlidingRule* psrule = NULL;
				const LAPriceDataCalendar* pcal = NULL;
			
				LAPriceCFGenUtility::getBusDayRuleAndCalendar(*pindex, 
															PRICING_DATA_FIXINGSLIDINGRULE,
															PRICING_DATA_FIXINGCALENDAR,
															*pindex,
															CALIBRATION_DATA_SLIDINGRULE,
															CALIBRATION_DATA_CALENDAR,
															psrule, pcal);		
				fixingdates[i] = LAPriceCFGenUtility::getDate(fixingbasedates[i], offset, specialday, *psrule, pcal);
				
			}
		}

		pindex->remove(PRICING_DATA_OFFSET);
		pindex->remove(PRICING_DATA_FIXINGCALENDAR);
		pindex->remove(PRICING_DATA_FIXINGSPECIALOFFSET);
		pindex->remove(PRICING_DATA_FIXINGSPECIALDAY);
		pindex->remove(PRICING_DATA_FIXINGSLIDINGRULE);
		pindex->remove(PRICING_DATA_FIXINGCALENDAR);
				
        pindex->remove(PRICING_DATA_FIXINGDATES);
		pindex->remove(PRICING_DATA_CFCALCSTARTDATES);
		pindex->remove(PRICING_DATA_CFCALCENDDATES);
		pindex->remove(PRICING_DATA_ISCOMPOUNDINGCOUPON);
		pindex->add(PRICING_DATA_FIXINGDATES, new LADataDates(fixingdates));
		pindex->add(PRICING_DATA_CFCALCSTARTDATES, new LADataDates(rollstartdates));
		pindex->add(PRICING_DATA_CFCALCENDDATES, new LADataDates(rollenddates));
        pindex->add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new LADataBool(true));

		// conpounding daycount
		dh = &(pindex->getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			pindex->remove(PRICING_DATA_COMPOUNDINGDAYCOUNT);
			pindex->add(PRICING_DATA_COMPOUNDINGDAYCOUNT, pindex->getData(PRICING_DATA_DAYCOUNT, ISNOTNULL).get().clone());
		}

		// ois
		if (indextype == OIS)
		{
			// obserbation freq
			dh = &(pindex->getData(PRICING_DATA_OBSERVATIONFREQUENCY, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				pindex->remove(PRICING_DATA_OBSERVATIONFREQUENCY);
				pindex->add(PRICING_DATA_OBSERVATIONFREQUENCY, new LADataString(BUSINESS_DAYS));
			}
			// obserbation start
			dh = &(pindex->getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				pindex->remove(PRICING_DATA_OBSERVATIONSTARTDATE);
				pindex->add(PRICING_DATA_OBSERVATIONSTARTDATE, new LADataDate(startdate));
			}
			const LAPriceDataSlidingRule* psrule = NULL;
			const LAPriceDataCalendar* pcal = NULL;
	
			LAPriceCFGenUtility::getBusDayRuleAndCalendar(*pindex, 
													   PRICING_DATA_FIXINGSLIDINGRULE,
													   PRICING_DATA_FIXINGCALENDAR,
													   *pindex,
													   CALIBRATION_DATA_SLIDINGRULE,
													   CALIBRATION_DATA_CALENDAR,
													   psrule, pcal);
			if (!pcal)
			{
				throw LACoreInvalidData("Calendar is not set", __FILE__, __LINE__);	
			}
			// obserbation end
			dh = &(pindex->getData(PRICING_DATA_OBSERVATIONENDDATE, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				LADate ob_enddate = pcal->getBusinessDay(enddate, -1);
				if (ob_enddate < startdate)
				{
					ob_enddate = startdate;
				}
				pindex->remove(PRICING_DATA_OBSERVATIONENDDATE);
				pindex->add(PRICING_DATA_OBSERVATIONENDDATE, new LADataDate(ob_enddate));
			}

			// conpounding daycount
			dh = &(pindex->getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				pindex->remove(PRICING_DATA_COMPOUNDINGDAYCOUNT);
				pindex->add(PRICING_DATA_COMPOUNDINGDAYCOUNT, pindex->getData(PRICING_DATA_DAYCOUNT, ISNOTNULL).get().clone());
			}

		}

	}
	else
	{
		LADate fixingbasedate;
		// isArrear
		dh = &(pindex->getData(PRICING_DATA_FIXINGTIMING, ISNOTNULL));
		const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
		bool isarrear = LAPriceCFGenUtility::isArrear(timing);
		if (isarrear && advancepaymentdate == paymentdate)//index fixing timing is arrear but payment timing is advance
		{
			//error
			LAString msg = "Index fixing timing is arrear but payment timing is advance";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}
		dh = &(pindex->getData(PRICING_DATA_ISFIXINGPAYMENTDATEBASE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()
			&& dynamic_cast<const LADataBool&>(dh->get()).get() == true)
			fixingbasedate = isarrear ? paymentdate : advancepaymentdate;
		else
			fixingbasedate = isarrear ? enddate : startdate;
		

		dh = &(pindex->getData(PRICING_DATA_OFFSET, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			int offset = dynamic_cast<const LADataInt&>(dh->get()).get();		
			pindex->remove(PRICING_DATA_OFFSET);
			if (offset == 0) 
				fixingdate = fixingbasedate;
			else
			{
				//calendar
				LAPriceDataCalendar cal;
				dh = &(pindex->getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
					pindex->remove(PRICING_DATA_FIXINGCALENDAR);
				}
				else
				{
					dh = &(pindex->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
					cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
				}
				fixingdate = cal.getBusinessDay(fixingbasedate, -offset);
			}
		}
		else // case of fixing special date
		{
			dh = &(pindex->getData(PRICING_DATA_FIXINGSPECIALOFFSET, ISNOTNULL));
			LAString offset = dynamic_cast<const LADataString&>(dh->get()).get();		
			pindex->remove(PRICING_DATA_FIXINGSPECIALOFFSET);
			
			dh = &(pindex->getData(PRICING_DATA_FIXINGSPECIALDAY, ISNOTNULL));
			IntArray specialday = dynamic_cast<const LADataInts&>(dh->get()).get();	
			if (specialday.size() != 12)
			{
				//error
				LAString msg = "fixing special day size must be 12";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}
			pindex->remove(PRICING_DATA_FIXINGSPECIALDAY);
			
			const LAPriceDataSlidingRule* psrule = NULL;
			const LAPriceDataCalendar* pcal = NULL;
		
			LAPriceCFGenUtility::getBusDayRuleAndCalendar(*pindex, 
														PRICING_DATA_FIXINGSLIDINGRULE,
														PRICING_DATA_FIXINGCALENDAR,
														*pindex,
														CALIBRATION_DATA_SLIDINGRULE,
														CALIBRATION_DATA_CALENDAR,
														psrule, pcal);		
			

			fixingdate = LAPriceCFGenUtility::getDate(fixingbasedate, offset, specialday, *psrule, pcal);
			pindex->remove(PRICING_DATA_FIXINGSLIDINGRULE);
			pindex->remove(PRICING_DATA_FIXINGCALENDAR);
		}

		pindex->add(PRICING_DATA_FIXINGDATE, new LADataDate(fixingdate));
	}
	
	//fixedrate
	dh = &(pindex->getData(PRICING_DATA_FIXEDDATES, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& fixeddates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(pindex->getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
		const DoubleArray& fixedrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();

		unsigned int pos;
		if (LAAlgorithm::find<DateVector, LADate>(fixeddates, fixingdate, 0, fixeddates.size() - 1, pos))
		{
			pindex->add(PRICING_DATA_FIXEDRATE, new LADataDouble(fixedrates.at(pos)));
		}
	
		pindex->remove(PRICING_DATA_FIXEDDATES);
		pindex->remove(PRICING_DATA_FIXEDRATES);
	}

	dh = &(pindex->getData(PRICING_DATA_FIRSTFIXINGRATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		double firstfixingrate = dynamic_cast<LADataDouble &>(dh->get());
		dh = &(pindex->getData(PRICING_DATA_FIRSTFIXINGBASEDATE, ISDEFINED));
		LADate basedate = dynamic_cast<LADataDate &>(dh->get()).get();
		if (fixingdate <= basedate)
		{
			pindex->add(PRICING_DATA_FIXEDRATE, new LADataDouble(firstfixingrate));
		}

		pindex->remove(PRICING_DATA_FIRSTFIXINGRATE);
		pindex->remove(PRICING_DATA_FIRSTFIXINGBASEDATE);

	}








	//for frn method
	//set frn method only in libor
	dh = &(pindex->getData(PRICING_DATA_INDEXGENERATEMETHOD, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString indexmethod = dynamic_cast<LADataString &>(dh->get());
		dh = &(pindex->getData(PRICING_DATA_FIXINGTIMING, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
			bool isarrear = LAPriceCFGenUtility::isArrear(timing);
			if (!isarrear && indexmethod.toUpper() == FRN && indextype == LIBOR)
			{
				DateVector dates(2);
				dates[0] = startdate;
				dates[1] = enddate;
				pindex->remove(PRICING_DATA_DATESFORINDEXGENERATE);
				pindex->add(PRICING_DATA_DATESFORINDEXGENERATE, new LADataDates(dates));
			}
		}
		pindex->remove(PRICING_DATA_INDEXGENERATEMETHOD);
	}


	objHolder.setDeleteFlag(false);
	return pindex;
}

/*!
	@brief get fixed dates from leg
	@param[in] leg
	
	@return fixed dates

*/
DateVector
LAPriceCashFlowGenerator::getFixedDates(const LAObject& leg) const
{
	const LAStringVector fixedTerms = dynamic_cast<const LADataStrings&>(leg.getData(PRICING_DATA_FIXEDTERMS, ISNOTNULL).get()).get();
	const LADate& sDate     = dynamic_cast<const LADataDate&>(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
	const LAPriceDataSlidingRule* pSrule = &dynamic_cast<const LAPriceDataSlidingRule&>(leg.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
	const LAPriceDataCalendar* pCal = NULL;
	//sliding rule
	const LADataHolder* dh = &(leg.getData(PRICING_DATA_CASHFLOWSLIDINGRULE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pSrule = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	if (pSrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		//calendar
		dh = &(leg.getData(PRICING_DATA_CASHFLOWCALENDAR, NOCHECK));
		if (dh->isDefined() && !dh->isNull())	
			pCal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
		else
		{
			dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
			pCal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
		}
	}
	//roll convention
	const LAString* pRollConv = NULL;
	dh = &(leg.getData(PRICING_DATA_ROLLCONVENTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		pRollConv = &dynamic_cast<const LADataString&>(dh->get()).get();
	}

	DateVector ret;
	for (int i = 0; i < fixedTerms.size(); ++i)
	{
		const LADate date = LAMathDateCalculations::getDate(sDate, fixedTerms[i], *pSrule, pCal, true, pRollConv);
		ret.push_back(date);
	}
	return ret;
}

/*!
	@brief	Create range accrueal index information object that is refered from Coupon informarion object refered from rangeinfo object 
	@param[in] startdate startdate of interest calculation(after holiday adjust)
	@param[in] enddate enddate of interest calculation(after holiday adjust)
	@param[in] index information object that represents index (master)informarion
	
	@return	pointer of object that represents index information

*/
LAObject*
LAPriceCashFlowGenerator::createRangeAccrueIndexInfo(const LAString name,
												  const LADate& startdate,
												  const LADate& enddate,
												  const LAObject& indexinfo) const
{
	LADataHolder *dh;
	LAObject *pindex = indexinfo.clone();
	LAObjectHolder objHolder;
	objHolder.setEntity(pindex, true);

	// name
	dh = &(pindex->getData(CALIBRATION_DATA_NAME, NOCHECK));
	if (!dh->isDefined())
	{
		dh = &(pindex->add(CALIBRATION_DATA_NAME, new LADataString()));
	}
    dh->convertFromString(name);
	// observation calendar
	dh = &(pindex->getData(PRICING_DATA_OBSERVATIONCALENDAR, NOCHECK));
	if (!dh->isDefined())
	{
		pindex->remove(PRICING_DATA_OBSERVATIONCALENDAR);
		dh = &(pindex->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
		pindex->add(PRICING_DATA_OBSERVATIONCALENDAR, *dh);
	}
	// observation slidingrule
	dh = &(pindex->getData(PRICING_DATA_OBSERVATIONSLIDINGRULE, NOCHECK));
	if (!dh->isDefined())
	{
		pindex->remove(PRICING_DATA_OBSERVATIONSLIDINGRULE);
		dh = &(pindex->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
		pindex->add(PRICING_DATA_OBSERVATIONSLIDINGRULE, *dh);
	}
	// observation start date
	pindex->remove(PRICING_DATA_OBSERVATIONSTARTDATE);
	pindex->add(PRICING_DATA_OBSERVATIONSTARTDATE, new LADataDate(startdate));
	// observation end date
	pindex->remove(PRICING_DATA_OBSERVATIONENDDATE);
	pindex->add(PRICING_DATA_OBSERVATIONENDDATE, new LADataDate(enddate));
	// observation frequency(business day)
	pindex->remove(PRICING_DATA_OBSERVATIONFREQUENCY);
	pindex->add(PRICING_DATA_OBSERVATIONFREQUENCY, new LADataString(BUSINESS_DAYS));	
	// observation operator
	pindex->remove(PRICING_DATA_OBSERVATIONOPERATOR);
	LAPriceDataFunction *pfunc = new LAPriceDataFunction(new LAConstant(0.0), FN_CONSTANT_STR);
	pindex->add(PRICING_DATA_OBSERVATIONOPERATOR, pfunc);

	// observation dates and rates
	pindex->remove(PRICING_DATA_OBSERVATIONDATES);
	pindex->remove(PRICING_DATA_OBSERVATIONRATES);
	dh = &(pindex->getData(PRICING_DATA_FIXEDDATES, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector &dates = dynamic_cast<const LADataDates &>(dh->get()).get();
		dh = &(pindex->getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
		const DoubleVector &rates = dynamic_cast<const LADataDoubles &>(dh->get()).get();
		if (dates.size() != rates.size())
		{
			throw LACoreInvalidData("FixedDates size and FixedRates size is not same.", __FILE__, __LINE__);
		}
		DateVector observationDates;
		DoubleArray observationRates;
		for (unsigned i = 0; i < dates.size(); ++i)
		{
			if (dates[i] >= enddate)
			{
				break;
			}
			if (dates[i] >= startdate && dates[i] < enddate)
			{
				observationDates.push_back(dates[i]);
				observationRates.push_back(rates[i]);

			}
		}
		if (!observationDates.empty())
		{
			// observation dates
			pindex->add(PRICING_DATA_OBSERVATIONDATES, new LADataDates(observationDates));
			// observation rates
			pindex->add(PRICING_DATA_OBSERVATIONRATES, new LADataDoubles(observationRates));
		}
		pindex->remove(PRICING_DATA_FIXEDRATES);
		pindex->remove(PRICING_DATA_FIXEDDATES);
	}
	
	objHolder.setDeleteFlag(false);
	return pindex;
}


/*!
	@brief	setup leg maturity 
	@param[in, out] trade trade object

*/
void
LAPriceCashFlowGenerator::setUpLegMaturity(LAObject& trade) const
{
	LADataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) continue;
		
		//set end date
		dh = &(legs.get(i).getData(PRICING_DATA_ENDTERM, ISNOTNULL));
		//term
		const LAString& termstr = dynamic_cast<const LADataString&>(dh->get()).get();
		//start date
		dh = &(legs.get(i).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
		const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		//end date
		const LADate& enddate = LAMathDateCalculations::getDate(startdate, termstr, true);
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (!dh->isDefined())
			dh = &(legs.get(i).add(PRICING_DATA_ENDDATE, new LADataDate()));
		dynamic_cast<LADataDate&>(dh->get()).set(enddate);
		
	}
}

/*!
	@brief	setup call schedule
	@param[in, out] trade trade object

*/
void
LAPriceCashFlowGenerator::setUpCallSchedule(LAObject& trade) const
{
	const LADataHolder* dh;
	LADataHolder* ah_;
	//call info 
	ah_ = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!ah_->isDefined() || ah_->isNull()) return;	//no call information( = not callable trade) 

	LAObjectHolder& callinfo = dynamic_cast<LADataReference&>(ah_->get()).get();
	
	// call schedule type
	dh = &(callinfo.getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
	LAString schtype = dynamic_cast<const LADataString&>(dh->get()).get();
	schtype.toUpper();

	if (schtype == MANUAL) return;	//manual input 
	

	DateVector calldates, dates_unadjust, fixingdates, fixingbasedates;
	//leg object
	ah_ = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(ah_->get());
	LAObjectHolder& leg = legs.get(0);

	const LAPriceDataSlidingRule* pSrule = NULL;
	const LAPriceDataCalendar*	pCal = NULL;
	const LAPriceDataSlidingRule* pSrule_fixing = NULL;
	const LAPriceDataCalendar*	pCal_fixing = NULL;
	///////////////////////////
	// calc call action dates//
	///////////////////////////
	if (schtype == PAYMENTDATE)	// call cation dates are same as payment dates
	{
		//check payment dates are same between both legs
		if (legs.getSize() > 1)
		{
			vector<DateVector>	tmp(legs.getSize());
			for (unsigned int i = 0; i < legs.getSize(); i++)
			{
				dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
				const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
				tmp[i].resize(cashlets.getSize());
				for (unsigned int j = 0; j < cashlets.getSize(); j++)
				{
					dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
					dh = &(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
					tmp[i][j] = dynamic_cast<const LADataDate&>(dh->get()).get();
				}			
			}
			for (unsigned int i = 1; i < tmp.size(); i++)
			{
				if (tmp[0].size() != tmp[i].size())
				{
					//error
					LAString msg = "Payment Dates are not same between both legs";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
				}
				for (unsigned int j = 0; j < tmp[0].size(); j++)
				{
					if (tmp[0][j] != tmp[i][j])
					{
						//error
						LAString msg = "Payment Dates are not same between both legs";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
					}
				}
			}		
		}
		
		dh = &(callinfo.getData(PRICING_DATA_FIXINGTIMING, ISNOTNULL));
		const LAString& fixingtiming = dynamic_cast<const LADataString&>(dh->get()).get(); 
		bool isfixingarrear = LAPriceCFGenUtility::isArrear(fixingtiming);
		bool isactionarrear = true;

		//get PaymentDate
		//cashlet
		dh = &(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
		
		bool flag = true;
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment

			if (flag)
			{
				dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
				const LAString& actiontiming = dynamic_cast<const LADataString&>(dh->get()).get(); 
				isactionarrear = LAPriceCFGenUtility::isArrear(actiontiming);
				if (isfixingarrear == true && isactionarrear == false)
				{
					//error
					LAString msg = "call fixing timing is arrear but action timing is advance";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
				}
				flag = false;
			}

			dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			calldates.push_back(dynamic_cast<const LADataDate&>(dh->get()).get());
		}

		fixingbasedates = calldates;
		if (isactionarrear)
		{
			//remove maturiy
			calldates.pop_back();
			fixingbasedates.pop_back();
			
			if (!isfixingarrear)
			{
				if (!isSameStartDates(legs))
				{
					//error
					LAString msg = "StartDate must be same in each leg";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
				}
				
				//startDate of trade
				dh = &(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL));
				const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
				
				fixingbasedates.pop_back();	
				fixingbasedates.insert(fixingbasedates.begin(), 
									LAPriceCFGenUtility::getDate(startdate_trade, leg.get(), 
									CALIBRATION_DATA_SLIDINGRULE, CALIBRATION_DATA_CALENDAR)); 
			}
		}
	

	}
	else if (schtype == DETAIL) // case of detail input for call schedule
	{
		
		////////////////////////////////////////////////
		// calendar and sliding rule for schedule calc//
		////////////////////////////////////////////////
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(callinfo.get(), 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													leg.get(),
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,
													pSrule, pCal);

		dh = &(callinfo.getData(PRICING_DATA_FIXINGTIMING, ISNOTNULL));
		const LAString& fixingtiming = dynamic_cast<const LADataString&>(dh->get()).get(); 
		bool isfixingarrear = LAPriceCFGenUtility::isArrear(fixingtiming);
		dh = &(callinfo.getData(PRICING_DATA_ACTIONTIMING, ISNOTNULL));
		const LAString& actiontiming = dynamic_cast<const LADataString&>(dh->get()).get(); 
		bool isactionarrear = LAPriceCFGenUtility::isArrear(actiontiming);
		if (isfixingarrear == true && isactionarrear == false)
		{
			//error
			LAString msg = "call fixing timing is arrear but action timing is advance";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}			
		
		//startDate
		LADate startdate;
		dh = &(callinfo.getData(PRICING_DATA_STARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		else
		{
			if (!isSameStartDates(legs))
			{
				//error
				LAString msg = "StartDate must be same in each leg";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}
			//startterm
			dh = &(callinfo.getData(PRICING_DATA_STARTTERM, ISNOTNULL));
			const LAString& term = dynamic_cast<const LADataString&>(dh->get()).get();
			//startDate of trade
			dh = &(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL));
			const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
			startdate = LAMathDateCalculations::getDate(startdate_trade, term, true);
		}

		//endDate
		LADate enddate;
		dh = &(callinfo.getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			enddate = dynamic_cast<const LADataDate&>(dh->get()).get();
		else
		{
			if (!isSameEndDates(legs))
			{
				//error
				LAString msg = "EndDate must be same in each leg";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}			
			//endterm
			dh = &(callinfo.getData(PRICING_DATA_ENDTERM, ISNOTNULL));
			const LAString& term = dynamic_cast<const LADataString&>(dh->get()).get();
			//endDate of trade
			dh = &(leg.getData(PRICING_DATA_ENDDATE, ISNOTNULL));
			const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
			enddate = LAMathDateCalculations::getDate(startdate_trade, term, false);
		}
	

		//call schedule
		LAPriceCFGenUtility::generateSchedule(callinfo.get(), startdate, enddate,
							PRICING_DATA_FREQUENCY,
							PRICING_DATA_ACTIONTIMING,
							PRICING_DATA_FIRSTODDDATE,
							PRICING_DATA_LASTODDDATE,
							PRICING_DATA_OBSERVATIONDAY,
							*pSrule,
							pCal,
							calldates,
							dates_unadjust);//dates_unadjust:dummy		

		//fixing schedule
		LAPriceCFGenUtility::generateSchedule(callinfo.get(), startdate, enddate,
							PRICING_DATA_FREQUENCY,
							PRICING_DATA_FIXINGTIMING,
							PRICING_DATA_FIRSTODDDATE,
							PRICING_DATA_LASTODDDATE,
							PRICING_DATA_OBSERVATIONDAY,
							*pSrule,
							pCal,
							fixingbasedates,
							dates_unadjust);//dates_unadjust:dummy
	}
	else
	{
		//error
		LAString msg = PRICING_DATA_INPUTTYPE;
		msg += ": " + schtype;
		msg += " is not support";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
	}
	
	/////////////////////
	//call fixing dates//
	/////////////////////
	fixingdates.resize(calldates.size());
	dh = &(callinfo.getData(PRICING_DATA_OFFSET, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		int offset = dynamic_cast<const LADataInt&>(dh->get()).get();		
		if (offset == 0)
			fixingdates = fixingbasedates;
		else
		{
			//get fixing calendar	
			dh = &(callinfo.getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
			else
			{
				dh = &(callinfo.getData(PRICING_DATA_OBSERVATIONCALENDAR, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
				else
				{
					dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
					pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
				}
			}
			
			for (unsigned int i = 0; i < calldates.size(); i++)
				fixingdates[i] = pCal_fixing->getBusinessDay(fixingbasedates[i], -offset);
		}
	}
	else//special offset date
	{
		// calendar and sliding rule for fixing date//
		dh = &(callinfo.getData(PRICING_DATA_FIXINGSLIDINGRULE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			pSrule_fixing = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
		else if (schtype == DETAIL)
			pSrule_fixing = pSrule;
		else
		{
			dh = &(callinfo.getData(PRICING_DATA_OBSERVATIONSLIDINGRULE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				pSrule_fixing = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
			else
			{
				dh = &(leg.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
				pSrule_fixing = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
			}
		}
		
		if (pSrule_fixing->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
		{
			dh = &(callinfo.getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
			else
			{
				dh = &(callinfo.getData(PRICING_DATA_OBSERVATIONCALENDAR, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
				else
				{
					dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
					pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
				}
			}
		}
			
		dh = &(callinfo.getData(PRICING_DATA_FIXINGSPECIALOFFSET, ISNOTNULL));
		LAString offset = dynamic_cast<const LADataString&>(dh->get()).get();		
			
		dh = &(callinfo.getData(PRICING_DATA_FIXINGSPECIALDAY, ISNOTNULL));
		const IntArray& specialday = dynamic_cast<const LADataInts&>(dh->get()).get();	
		if (specialday.size() != 12)
		{
			//error
			LAString msg = "fixing special day size must be 12";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}				
		for (unsigned int i = 0; i < calldates.size(); i++)
			fixingdates[i] = LAPriceCFGenUtility::getDate(fixingbasedates[i], offset, specialday, *pSrule_fixing, pCal_fixing);	
	}

	///////////////////
	// set call dates//
	///////////////////
	ah_ = &(callinfo.getData(PRICING_DATA_ACTIONDATES, NOCHECK));
	if (!ah_->isDefined())
		ah_ = &callinfo.add(PRICING_DATA_ACTIONDATES, new LADataDates());
	dynamic_cast<LADataDates&>(ah_->get()).set(calldates);	
	/////////////////////
	// set fixing dates//
	/////////////////////
	ah_ = &(callinfo.getData(PRICING_DATA_EXPIRYDATES, NOCHECK));
	if (!ah_->isDefined())
		ah_ = &callinfo.add(PRICING_DATA_EXPIRYDATES, new LADataDates());
	dynamic_cast<LADataDates&>(ah_->get()).set(fixingdates);	
	/////////////
	// extra cf//
	/////////////
/*	dh = &(callinfo.getData(PRICING_DATA_EXTRACF, NOCHECK));
	if (!dh->isDefined() || dh->isNull()) return; //no extra cf

	double extracf = dynamic_cast<const LADataDouble&>(dh->get()).get();
	
	// set extracf
	DoubleArray extracf_array(1, extracf);
	ah_ = &(callinfo.getData(PRICING_DATA_EXTRACFS, NOCHECK));
	if (!ah_->isDefined())
		ah_ = &callinfo.add(PRICING_DATA_EXTRACFS, new LADataDoubles());
	dynamic_cast<LADataDoubles&>(ah_->get()).set(extracf_array);	
*/	
	dh = &(callinfo.getData(PRICING_DATA_EXTRACFFUNC, NOCHECK));
	if (!dh->isDefined() || dh->isNull()) return; 

	// extra cf offset
	dh = &(callinfo.getData(PRICING_DATA_EXTRACFOFFSET, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		callinfo.remove(PRICING_DATA_EXTRACFDATES);

		int offset_extracf = dynamic_cast<const LADataInt&>(dh->get()).get();

		if (offset_extracf == 0) return;

		if (pCal == NULL)
		{
			dh = &(callinfo.getData(PRICING_DATA_OBSERVATIONCALENDAR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				pCal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
			else
			{
				dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
				pCal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
			}
		}

		DateVector	extracfdates(calldates.size());
		for (unsigned int i = 0; i < calldates.size(); i++)
			extracfdates[i] = pCal->getBusinessDay(calldates[i], offset_extracf); 


		// set extracf dates
		ah_ = &callinfo.add(PRICING_DATA_EXTRACFDATES, new LADataDates());
		dynamic_cast<LADataDates&>(ah_->get()).set(extracfdates);	
	}

}

/*!
	@brief	setup trigger schedule
	@param[in, out] trade trade object

*/
void
LAPriceCashFlowGenerator::setUpTriggerSchedule(LAObject& trade) const
{
	const LADataHolder* dh;
	LADataHolder* ah_;
	
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	
	//trigger info 
	ah_ = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (!ah_->isDefined() || ah_->isNull()) return;	//no trigger information
	LADataMultiReference& triggers = dynamic_cast<LADataMultiReference&>(ah_->get());
	
	for (unsigned int i = 0; i < triggers.getSize(); i++)
	{
		// trigger schedule type
		dh = &(triggers.get(i).getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
		LAString schtype = dynamic_cast<const LADataString&>(dh->get()).get();
		schtype.toUpper();
		if (schtype == MANUAL)
		{
			setUpTriggerCpnChange(trade, triggers.get(i).get());
			continue;//manual input
		}
		else if (schtype != DETAIL)
		{
			//error
			LAString msg = PRICING_DATA_INPUTTYPE;
			msg += ": " + schtype;
			msg += " is not support";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}

		// target leg
		dh = &(triggers.get(i).getData(PRICING_DATA_TARGETLEG, ISNOTNULL));
		LAString target = dynamic_cast<const LADataString&>(dh->get()).get();
		target.toUpper();
		LAObjectHolder* pleg;
		unsigned int legNo = 0;
		if (target == BOTH) pleg = &legs.get(0);
		else
		{
			if (target.size() < 4 ||
				(legNo = (target.subString(3, target.size() - 1)).getIntValue()) == 0
				|| legs.getSize() < legNo)
			{
				//error
				LAString msg = PRICING_DATA_TARGETLEG;
				msg += ": " + target;
				msg += " is a wrong input";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}
			pleg = &legs.get(legNo - 1);
		}


		//startDate of trigger
		LADate startdate;
		dh = &(triggers.get(i).getData(PRICING_DATA_STARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())		
			startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		else
		{
			if (target == BOTH && !isSameStartDates(legs))
			{
				//error
				LAString msg = "StartDate must be same in each leg";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}			
			dh = &(triggers.get(i).getData(PRICING_DATA_STARTTERM, ISNOTNULL));
			//startterm
			const LAString& term = dynamic_cast<const LADataString&>(dh->get()).get();
			//startDate of trade
			dh = &(pleg->getData(PRICING_DATA_STARTDATE, ISNOTNULL));
			const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
			startdate = LAMathDateCalculations::getDate(startdate_trade, term, true);
		}
		
		//endDate of trigger
		LADate enddate;
		dh = &(triggers.get(i).getData(PRICING_DATA_ENDDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())		
			enddate = dynamic_cast<const LADataDate&>(dh->get()).get();
		else
		{
			if (target == BOTH && !isSameEndDates(legs))
			{
				//error
				LAString msg = "EndDate must be same in each leg";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}					
			dh = &(triggers.get(i).getData(PRICING_DATA_ENDTERM, ISNOTNULL));
			//endtermstr
			const LAString& term = dynamic_cast<const LADataString&>(dh->get()).get();
			//endDate of trade
			dh = &(pleg->getData(PRICING_DATA_ENDDATE, ISNOTNULL));
			const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
			enddate = LAMathDateCalculations::getDate(startdate_trade, term, false);
		}
		
		//BY IKEDA 20061226
		LADate startdate_cmp = startdate;
		LADate enddate_cmp = enddate;
	
		////////////////////
		//trigger schedule//
		////////////////////
		DateVector triggerdates, triggerdates_unadjust, 
					actiondates, actiondates_unadjust,
					fixingdates, actionfixingdates;

		////////////////////////////////////////////////
		// calendar and sliding rule for schedule calc//
		////////////////////////////////////////////////
		const LAPriceDataSlidingRule* pSrule = NULL;
		const LAPriceDataCalendar*	pCal = NULL;
		const LAPriceDataSlidingRule* pSrule_fixing = NULL;
		const LAPriceDataCalendar*	pCal_fixing = NULL;
		
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(triggers.get(i).get(), 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													pleg->get(),
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,
													pSrule, pCal);


		LAPriceCFGenUtility::generateSchedule(triggers.get(i).get(), startdate, enddate,
							PRICING_DATA_OBSERVATIONFREQUENCY,
							PRICING_DATA_FIXINGTIMING,
							PRICING_DATA_FIRSTODDDATE,
							PRICING_DATA_LASTODDDATE,
							PRICING_DATA_OBSERVATIONDAY,
							*pSrule,
							pCal,
							triggerdates,
							triggerdates_unadjust);		


		////////////////////////
		//trigger fixing dates//
		////////////////////////
		fixingdates.resize(triggerdates.size());
		dh = &(triggers.get(i).getData(PRICING_DATA_OFFSET, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			int offset = dynamic_cast<const LADataInt&>(dh->get()).get();
			if (offset == 0)
				fixingdates = triggerdates;
			else
			{
				//get fixing calendar	
				dh = &(triggers.get(i).getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
				else
					pCal_fixing = pCal;
			
				for (unsigned int j = 0; j < triggerdates.size(); j++)
					fixingdates[j] = pCal_fixing->getBusinessDay(triggerdates[j], -offset);
			}
		}
		else//special offset date
		{
			// calendar and sliding rule for fixing date//
			dh = &(triggers.get(i).getData(PRICING_DATA_FIXINGSLIDINGRULE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				pSrule_fixing = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
			else 
				pSrule_fixing = pSrule;
			if (pSrule_fixing->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
			{
				dh = &(triggers.get(i).getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
				else
					pCal_fixing = pCal;
			}

	
			dh = &(triggers.get(i).getData(PRICING_DATA_FIXINGSPECIALOFFSET, ISNOTNULL));
			LAString offset = dynamic_cast<const LADataString&>(dh->get()).get();		
				
			dh = &(triggers.get(i).getData(PRICING_DATA_FIXINGSPECIALDAY, ISNOTNULL));
			const IntArray& specialday = dynamic_cast<const LADataInts&>(dh->get()).get();	
			if (specialday.size() != 12)
			{
				//error
				LAString msg = "fixing special day size must be 12";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}		
			for (unsigned int j = 0; j < triggerdates.size(); j++)
				fixingdates[j] = LAPriceCFGenUtility::getDate(triggerdates[j], offset, specialday, *pSrule_fixing, pCal_fixing);	
		}

		///////////////////
		//action schedule//
		///////////////////
		dh = &(triggers.get(i).getData(PRICING_DATA_ISACTIONDETAIL, ISNOTNULL));
		bool isdetail = dynamic_cast<const LADataBool&>(dh->get()).get();	
		if (isdetail)
		{
	
			//startDate of trigger action
			dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONSTARTDATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
			else
			{
				dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONSTARTTERM, ISNOTNULL));
				//starttermstr
				const LAString& term = dynamic_cast<const LADataString&>(dh->get()).get();
				//startDate of trade
				dh = &(pleg->getData(PRICING_DATA_STARTDATE, ISNOTNULL));		//PRICING_DATA_ACTIONSTARTDATE, ISNOTNULL));
				const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
				startdate = LAMathDateCalculations::getDate(startdate_trade, term, true);
			}
	
			//endDate of trigger action
			dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONENDDATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				enddate = dynamic_cast<const LADataDate&>(dh->get()).get();
			else
			{
				dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONENDTERM, NOCHECK));
				//endtermstr
				const LAString& term = dynamic_cast<const LADataString&>(dh->get()).get();
				//endDate of trade
				dh = &(pleg->getData(PRICING_DATA_ENDDATE, ISNOTNULL));			//PRICING_DATA_ACTIONENDDATE, ISNOTNULL));
				const LADate& startdate_trade = dynamic_cast<const LADataDate&>(dh->get()).get();
				enddate = LAMathDateCalculations::getDate(startdate_trade, term, false);
			}
			
			//BY IKEDA 20061226
			if (startdate_cmp > startdate) //(startdate_cmp > startdate) || (enddate_cmp > enddate))
			{
				//error
				LAString msg = "Date of trigger action is not fitted Date of trigger";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}
			
			////////////////////////////////////////////////
			// calendar and sliding rule for schedule calc//
			////////////////////////////////////////////////
			LAPriceCFGenUtility::getBusDayRuleAndCalendar(triggers.get(i).get(), 
														PRICING_DATA_ACTIONSLIDINGRULE,
														PRICING_DATA_ACTIONCALENDAR,
														pleg->get(),
														CALIBRATION_DATA_SLIDINGRULE,
														CALIBRATION_DATA_CALENDAR,
														pSrule, pCal);
			
		
			LAPriceCFGenUtility::generateSchedule(triggers.get(i).get(), startdate, enddate,
								PRICING_DATA_ACTIONFREQUENCY,
								PRICING_DATA_ACTIONTIMING,
								PRICING_DATA_ACTIONFIRSTODDDATE,
								PRICING_DATA_ACTIONLASTODDDATE,
								PRICING_DATA_ACTIONDAY,
								*pSrule,
								pCal,
								actiondates,
								actiondates_unadjust);		


			///////////////////////
			//action fixing dates//
			///////////////////////
			DateVector tmpdates(actiondates.size());
			dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONOFFSET, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				int offset = dynamic_cast<const LADataInt&>(dh->get()).get();
				if (offset == 0)
					tmpdates = actiondates;
				else
				{
					//get fixing calendar	
					dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONFIXINGCALENDAR, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
						pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
					else
						pCal_fixing = pCal;
				
					for (unsigned int j = 0; j < actiondates.size(); j++)
						tmpdates[j] = pCal_fixing->getBusinessDay(actiondates[j], -offset);
				}
			}
			else//special offset date
			{
				// calendar and sliding rule for fixing date//
				dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONFIXINGSLIDINGRULE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					pSrule_fixing = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
				else 
					pSrule_fixing = pSrule;
				if (pSrule_fixing->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				{
					dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONFIXINGCALENDAR, NOCHECK));
					if (dh->isDefined() && !dh->isNull())
						pCal_fixing = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());	
					else
						pCal_fixing = pCal;
				}				
					
				dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONFIXINGSPECIALOFFSET, ISNOTNULL));
				LAString offset = dynamic_cast<const LADataString&>(dh->get()).get();		
					
				dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONFIXINGSPECIALDAY, ISNOTNULL));
				const IntArray& specialday = dynamic_cast<const LADataInts&>(dh->get()).get();	
				if (specialday.size() != 12)
				{
					//error
					LAString msg = "fixing special day size must be 12";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
				}				

				for (unsigned int j = 0; j < triggerdates.size(); j++)
					tmpdates[j] = LAPriceCFGenUtility::getDate(actiondates[j], offset, specialday, *pSrule_fixing, pCal_fixing);	
			}

			if (fixingdates.size() == tmpdates.size())
				actionfixingdates = tmpdates;
			else
			{
				actionfixingdates.resize(fixingdates.size());
				unsigned int pos;
				for (unsigned int j = 0; j < actionfixingdates.size(); j++)
				{
					LAAlgorithm::locate<DateVector, LADate>(tmpdates, fixingdates[j], tmpdates.size(), pos);
					if (pos == tmpdates.size())
					{
						//error
						LAString msg = "Trigger fixing date:" + LADataDate(fixingdates[j]).convertToString();
						msg += " corresponds to no trigger action date.";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					actionfixingdates[j] = tmpdates.at(pos);
				}
			}
		}
		else
		{
			dh = &(triggers.get(i).getData(PRICING_DATA_FIXINGTIMING, ISNOTNULL));
			const LAString& fixingtiming = dynamic_cast<const LADataString&>(dh->get()).get(); 
			bool isfixingarrear = LAPriceCFGenUtility::isArrear(fixingtiming);
			dh = &(triggers.get(i).getData(PRICING_DATA_ACTIONTIMING, ISNOTNULL));
			const LAString& actiontiming = dynamic_cast<const LADataString&>(dh->get()).get(); 
			bool isactionarrear = LAPriceCFGenUtility::isArrear(actiontiming);
			if (isfixingarrear == true && isactionarrear == false)
			{
				//error
				LAString msg = "trigger fixing timing is arrear but action timing is advance";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}

			//action schedule
			LAPriceCFGenUtility::generateSchedule(triggers.get(i).get(), startdate, enddate,
								PRICING_DATA_OBSERVATIONFREQUENCY,
								PRICING_DATA_ACTIONTIMING,
								PRICING_DATA_FIRSTODDDATE,
								PRICING_DATA_LASTODDDATE,
								PRICING_DATA_OBSERVATIONDAY,
								*pSrule,
								pCal,
								actionfixingdates,
								actiondates_unadjust);			
			
		}


		/////////////////////
		// set action dates//
		/////////////////////
		ah_ = &(triggers.get(i).getData(PRICING_DATA_ACTIONDATES, NOCHECK));
		if (!ah_->isDefined())
			ah_ = &triggers.get(i).add(PRICING_DATA_ACTIONDATES, new LADataDates());
		dynamic_cast<LADataDates&>(ah_->get()).set(actionfixingdates);	
		/////////////////////
		// set fixing dates//
		/////////////////////
		ah_ = &(triggers.get(i).getData(PRICING_DATA_EXPIRYDATES, NOCHECK));
		if (!ah_->isDefined())
			ah_ = &triggers.get(i).add(PRICING_DATA_EXPIRYDATES, new LADataDates());
		dynamic_cast<LADataDates&>(ah_->get()).set(fixingdates);		


		/////////////////////////////////
		//set up for trigger cpn change//
		/////////////////////////////////
		setUpTriggerCpnChange(trade, triggers.get(i).get());
	}
}


/*!
	@brief	setup trigger coupon change type
	@param[in, out] trade trade object
	@param[in, out] trigger trigger info object

*/
void
LAPriceCashFlowGenerator::setUpTriggerCpnChange(LAObject& trade, LAObject& trigger) const
{
	LADataHolder* dh;

	//cpn change info
	dh = &(trigger.getData(PRICING_DATA_COUPONCHANGEINFO, NOCHECK));	
	if (!dh->isDefined() || dh->isNull()) return;
	const LADataReference& cpninfo = dynamic_cast<const LADataReference&>(dh->get());
	// name of this trigger info
	dh = &(trigger.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
	const LAString& trigger_name = dynamic_cast<const LADataString&>(dh->get()).get(); 
	LAObjectPool& objPool = trade.getDataInstance()->getObjectPool();

	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());

	// target leg
	dh = &(trigger.getData(PRICING_DATA_TARGETLEG, ISNOTNULL));
	LAString target = dynamic_cast<const LADataString&>(dh->get()).get();
	target.toUpper();
	unsigned int legNo = 0;
	if (target != BOTH)
	{
		if (target.size() < 4 ||
			(legNo = (target.subString(3, target.size() - 1)).getIntValue()) == 0
			|| legs.getSize() < legNo)
		{
			//error
			LAString msg = PRICING_DATA_TARGETLEG;
			msg += ": " + target;
			msg += " is a wrong input";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}

	}

	dh = &(trigger.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
	const DateVector& actiondates = dynamic_cast<LADataDates&>(dh->get()).get();	



	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		if (target != BOTH && legNo != i + 1) continue;	

		//input type
		dh = &(legs.get(i).getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
		LAString schtype = dynamic_cast<const LADataString&>(dh->get()).get();
		schtype.toUpper();
		if (schtype == MANUAL) //manual input
		{
			trigger.getData(PRICING_DATA_COUPONINFOS + LADataInt(i + 1).convertToString(), ISNOTNULL); 
			continue;
		}
		
		// Calculete payment dates and interest calculation dates
		DateVector paymentdates, paymentdates_unadjust, startdates, enddates;
        const LAString& freq = dynamic_cast<const LADataString&>(legs.get(i).getData(PRICING_DATA_FREQUENCY, ISNOTNULL).get()).get();
        const LADate* first_odd_date = (dh=&legs.get(i).getData(PRICING_DATA_FIRSTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const LADataDate&>(dh->get()).get() : NULL;
        const LADate* last_odd_date = (dh=&legs.get(i).getData(PRICING_DATA_LASTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const LADataDate&>(dh->get()).get() : NULL;
        const LADate& start_date = dynamic_cast<const LADataDate&>(legs.get(i).getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
        const LADate& end_date = dynamic_cast<const LADataDate&>(legs.get(i).getData(PRICING_DATA_ENDDATE, ISNOTNULL).get()).get();
		DateVector startdates_unadjust, enddates_unadjust;
		calcPaymentDates(legs.get(i).get(), paymentdates, paymentdates_unadjust, startdates, startdates_unadjust, enddates, 
			enddates_unadjust, start_date, end_date, freq, first_odd_date, last_odd_date);
		
		// isArrear payment
		dh = &(legs.get(i).getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
		const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
		bool isarrear = LAPriceCFGenUtility::isArrear(timing);
		// isNotionalChangeAtStat
		dh = &(legs.get(i).getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, ISNOTNULL));
		bool isnotionalchangestart = dynamic_cast<const LADataBool&>(dh->get()).get();
		// isNotionalChangeAtEnd
		dh = &(legs.get(i).getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
		bool isnotionalchangeend = dynamic_cast<const LADataBool&>(dh->get()).get();		

		dh = &(trigger.getData(PRICING_DATA_COUPONINFOS + LADataInt(i + 1).convertToString(), NOCHECK));
		if (dh->isDefined()) trigger.remove(PRICING_DATA_COUPONINFOS + LADataInt(i + 1).convertToString());
		
		
		LAString couponinfo_str;
		LAObjectHolder objHolder;
		unsigned int k = 0;//counter of coupon payment
		for (unsigned int j = 0; j < paymentdates.size(); j++)
		{
			if (j == 0 && isarrear && isnotionalchangestart) continue;// notinoal exchange only
			if (j == paymentdates.size() - 1 && !isarrear && isnotionalchangeend) continue; // notinoal exchange only
		

			if (isarrear && actiondates.at(0) >= paymentdates[j]) {k++; continue;}
			if (!isarrear && actiondates.at(0) > paymentdates[j]) {k++; continue;}

			LAString couponinfo_name = trigger_name + "_" + LEG_S + LADataInt(i + 1).convertToString()
					+ "_" + COUPONINFO + LADataInt(j + 1).convertToString();
						
			LADate advancepaymentdate;
			if (!isarrear) advancepaymentdate = paymentdates[j];
			else if (j > 0) advancepaymentdate = paymentdates[j - 1];
			else
			{
				dh = &(legs.get(j).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
				const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
				
				advancepaymentdate = LAPriceCFGenUtility::getDate(startdate, legs.get(i).get(), 
									CALIBRATION_DATA_SLIDINGRULE, CALIBRATION_DATA_CALENDAR);
			}
		
			DateVector tmpdates;
			objHolder.setEntity(createCouponInfo(couponinfo_name, 
				advancepaymentdate, paymentdates[j], paymentdates_unadjust[j], 
				startdates[k], enddates[k], cpninfo.get().get(), legs.get(i).get(),
				tmpdates,tmpdates,tmpdates), true);
			
			objPool.remove(couponinfo_name);//if already exist remove it.
			objPool.set(couponinfo_name, &objHolder.get());
			objHolder.setDeleteFlag(false);
				
			couponinfo_str += couponinfo_name;
			if (k != startdates.size() - 1) couponinfo_str += ":";
			k++;
		}

		dh = &trigger.add(PRICING_DATA_COUPONINFOS + LADataInt(i + 1).convertToString(), new LADataMultiReference());
		dh->convertFromString(couponinfo_str);		
	}
}


/*!
	@brief	check start dates are same between both legs
	@param[in] legs reference to leg entities
	
	@return ture:same, false:not same

*/
bool
LAPriceCashFlowGenerator::isSameStartDates(const LADataMultiReference& legs) const
{
	const LADataHolder* dh = &(legs.get(0).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
	const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
	for (unsigned int i = 1; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
		if (startdate != dynamic_cast<const LADataDate&>(dh->get()).get()) return false;		
	}
	return true;
}
/*!
	@brief	check end dates are same between both legs
	@param[in] legs reference to leg entities
	
	@return ture:same, false:not same

*/
bool
LAPriceCashFlowGenerator::isSameEndDates(const LADataMultiReference& legs) const
{
	const LADataHolder* dh = &(legs.get(0).getData(PRICING_DATA_ENDDATE, ISNOTNULL));
	const LADate& enddate = dynamic_cast<const LADataDate&>(dh->get()).get();
	for (unsigned int i = 1; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_ENDDATE, ISNOTNULL));
		if (enddate != dynamic_cast<const LADataDate&>(dh->get()).get()) return false;		
	}
	return true;
}


/*!
	@brief	setup funding change 
	@param[in, out] trade trade object

*/
void
LAPriceCashFlowGenerator::setUpFundingChange(LAObject& trade) const
{
	// get funding change info 
	LADataHolder* dh = &(trade.getData(PRICING_DATA_FUNDINGCHANGEINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		return;
	}
	LAObject& fginfo = dynamic_cast<LADataReference &>(dh->get()).get().get();

	// get leg entities
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
	if (legs.getSize() != 2)
		throw LACoreInvalidData("The number of legs must be 2",__FILE__,__LINE__);

	// get funding target leg (default setting is LEG2)
	LAObject& strleg = legs.get(0).get();
	LAObject& fdleg = legs.get(1).get();
	dh = &(fginfo.getData(PRICING_DATA_FUNDINGTARGETLEG, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString fdlegstr = dynamic_cast<LADataString &>(dh->get()).get();
		if (fdlegstr.toUpper() == "LEG1")
		{
			strleg = legs.get(1).get();
			fdleg = legs.get(0).get();
		}
		else if(!(fdlegstr.toUpper() == "LEG2"))
		{
			throw LACoreInvalidData("FundingTargetLeg must be LEG1 or LEG2", __FILE__, __LINE__);
		}
	}
	
	// create attrubutes that are needed for funding change
	dh = &(trade.getData(PRICING_DATA_ISCHANGEDBYFUNDING, NOCHECK));
	bool ischgattribute = false;
	if (dh->isDefined() && !dh->isNull())
	{
		ischgattribute = dynamic_cast<LADataBool &>(dh->get()).get();
	}
	if (!ischgattribute)
	{
		createFundingChangeAttribute(trade, fdleg, strleg, fginfo);
	}

	// cashlets of the funding leg
	LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference&>((&(fdleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL)))->get());
	unsigned int cashSize = cashlets.getSize();
	DoubleVector tmpvec(2, 0.0);
	DoubleMatrix tmpspread(cashSize, tmpvec);
	DoubleVector tmpfixedrate(cashSize);
	createFundingSpread(tmpspread, tmpfixedrate, trade, fdleg, strleg, fginfo);
	DoubleVector tmpfixedrate_;
	for (size_t i = 0; i < tmpspread.size(); ++i)
	{
		tmpfixedrate_.push_back(tmpspread[i][1]);
	}

	// get modified notional, currency, daycount conventions, calendar, sliding rule
	DoubleVector mdfynotionals = dynamic_cast<LADataDoubles &>((&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGNOTIONALS, ISNOTNULL)))->get()).get();
	const LAString mdycur = ((LAString)(dynamic_cast<LADataString &>((&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGCURRENCY, ISNOTNULL)))->get()).get())).toUpper();
	const LAString mdydc = (&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, ISNOTNULL)))->get().convertToString();
	const LAString mdyindexdc = (&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, ISNOTNULL)))->get().convertToString();
	const LAString mdycal = (&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, ISNOTNULL)))->get().convertToString();
	const LAString mdysrule = (&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, ISNOTNULL)))->get().convertToString();
	DoubleVector mdyadjustratios = dynamic_cast<LADataDoubles &>((&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGSETTLEMENTADJUSTRATIOS, ISNOTNULL)))->get()).get();

	// get modified forecast curve name
	LAString strcurvefname(STD);
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGFORECASTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		strcurvefname = dynamic_cast<LADataString &>(dh->get()).get();

	for (unsigned int i = 0; i < cashSize; i++)
	{
		// change cashlet dataValues
		LAObject& fdcash = cashlets.get(i).get();
		dynamic_cast<LADataDouble &>((&(fdcash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL)))->get()).set(mdfynotionals[i]);
		(&(fdcash.getData(PRICING_DATA_CURRENCY, ISNOTNULL)))->get().convertFromString(mdycur);
		dh = &(fdcash.getData(PRICING_DATA_DAYCOUNT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			dh->get().convertFromString(mdydc);
		}
		fdcash.remove(PRICING_DATA_ISMODIFIEDBYFUNDING);
		fdcash.add(PRICING_DATA_ISMODIFIEDBYFUNDING, new LADataBool(true));

		// change settlement adjust ratio
		fdcash.remove(PRICING_DATA_SETTLEMENTADJUSTRATIO);
		fdcash.add(PRICING_DATA_SETTLEMENTADJUSTRATIO, new LADataDouble(mdyadjustratios[i]));

		// get funding coupon
		dh = &(fdcash.getData(PRICING_DATA_COUPONINFOS,NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			continue;
		}
		LADataMultiReference& coupons = dynamic_cast<LADataMultiReference &>(dh->get());
		if (coupons.getSize() != 1)
		{
			throw LACoreInvalidData("Funding Coupon Size must be 1", __FILE__, __LINE__);
		}
		LAObject& fdcoupon = coupons.get(0).get();
		
		// change coupon attribues
		(&(fdcoupon.getData(PRICING_DATA_CURRENCY, ISNOTNULL)))->get().convertFromString(mdycur);
		fdcoupon.remove(PRICING_DATA_ISMODIFIEDBYFUNDING);
		fdcoupon.add(PRICING_DATA_ISMODIFIEDBYFUNDING, new LADataBool(true));

		// get coupon index 
		LADataMultiReference& indexs = dynamic_cast<LADataMultiReference &>((dh = &(fdcoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL)))->get());
		if (indexs.getSize() != 1)
		{
			throw LACoreInvalidData("Funding Index Size must be 1", __FILE__, __LINE__);
		}
		LAObject& fdindex = indexs.get(0).get();

		// change coupon coefficients
		dynamic_cast<LADataDoubles &>((&(fdcoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL)))->get()).set(tmpspread[i]);

		// check indextype
		LAString indextype = ((LAString)(dynamic_cast<LADataString &>((&(fdindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL)))->get()).get())).toUpper();
		if (indextype == FIXEDRATE)
		{
			dh = &(fdindex.getData(PRICING_DATA_FIXEDRATE, ISNOTNULL));
			dynamic_cast<LADataDouble &>(dh->get()).set(tmpfixedrate[i]);
			continue;
		}
		else if(indextype != LIBOR)
			throw LACoreInvalidData("Funding Index must be FixedRate or Libor",__FILE__,__LINE__);

		// change index attribues
		(&(fdindex.getData(PRICING_DATA_CURRENCY, ISNOTNULL)))->get().convertFromString(mdycur);
		(&(fdindex.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL)))->get().convertFromString(mdyindexdc);
		(&(fdindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)))->get().convertFromString(mdycal);
		(&(fdindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)))->get().convertFromString(mdysrule);
		dh = &(fdindex.getData(PRICING_DATA_BASISCURVE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			dh->get().convertFromString(strcurvefname);
		}
		fdindex.remove(PRICING_DATA_ISMODIFIEDBYFUNDING);
		fdindex.add(PRICING_DATA_ISMODIFIEDBYFUNDING, new LADataBool(true));
	}

	// change index of call info
	dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LADataReference& calls = dynamic_cast<LADataReference &>(dh->get());
		LADataMultiReference& callindexs = dynamic_cast<LADataMultiReference &>((&(calls.get().getData(PRICING_DATA_INDEXINFOS, ISNOTNULL)))->get());
		for (unsigned int i = 0; i < callindexs.getSize(); i++)
		{
			LAObject& indexcall = callindexs.get(i).get();
			const LAString indextype = ((LAString)dynamic_cast<LADataString &>((&(indexcall.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL)))->get()).get()).toUpper();
			dh = &(indexcall.getData(PRICING_DATA_CURRENCY, NOCHECK));
			const LAString cur = (dh->isDefined() && !dh->isNull()) ? dynamic_cast<LADataString &>(dh->get()).get() : LAString("");
			
			if (cur == mdycur || cur == "")
				continue;

			if (indextype != LIBOR)
				throw LACoreInvalidData("Call index type not supported by FundingChange", __FILE__, __LINE__);

			// change call index dataValues
			(&(indexcall.getData(PRICING_DATA_CURRENCY, NOCHECK)))->get().convertFromString(mdycur);
			(&(indexcall.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL)))->get().convertFromString(mdyindexdc);
			(&(indexcall.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)))->get().convertFromString(mdycal);
			(&(indexcall.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)))->get().convertFromString(mdysrule);
			dh = &(indexcall.getData(PRICING_DATA_FIXINGCALENDAR, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				dh->get().convertFromString(mdycal);
			}
			dh = &(indexcall.getData(PRICING_DATA_FIXINGSLIDINGRULE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				dh->get().convertFromString(mdysrule);
			}

			// remove call index curve name and interpolation
			dh = &(indexcall.getData(PRICING_DATA_BASISCURVE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				indexcall.remove(PRICING_DATA_BASISCURVE);
			}
			dh = &(indexcall.getData(PRICING_DATA_BASISINTERPOLATION, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				indexcall.remove(PRICING_DATA_BASISINTERPOLATION);
			}

			indexcall.remove(PRICING_DATA_ISMODIFIEDBYFUNDING);
			indexcall.add(PRICING_DATA_ISMODIFIEDBYFUNDING, new LADataBool(true));
		}
	}
}


// create funding spread
/*!
	@brief	setup trigger coupon change type
	@param[in, out] trade trade object
	@param[in, out] trigger trigger info object

*/
void
LAPriceCashFlowGenerator::createFundingSpread(DoubleMatrix &spreadmat, DoubleVector &fixedrates, LAObject& trade, LAObject& fundingleg, LAObject& strleg, LAObject& fginfo) const
{
	// declare variables
	LAString paytiming("ARREAR");
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	std::vector< std::vector<LAString> > indextypes;
	std::vector<LAPriceDataDayCount> cfdcs;

	// get path object
	LADataReference& pathref = dynamic_cast<LADataReference &>((&(trade.getData(IR_MODEL_DATA_PATHENTITY, ISNOTNULL)))->get());
	if (pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
	{
		const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
		/*if (!valuemehod.isTypeOf(FN_IR_NDSSWAPTIONVALUE))
		{
			return;
		}*/
	}
	else if (!pathref.get().get().isTypeOf(ENTITY_PATH))
	{
		throw LACoreInvalidData("Error createFundingSpread: invalid path type", __FILE__, __LINE__);
	}

	// get asofdate, settle date
	const LADate settle = dynamic_cast<LADataDate &>((&(trade.getData(PRICING_DATA_SETTLEDATE, ISNOTNULL)))->get()).get();
	const LADate issuedate = dynamic_cast<LADataDate &>((&(fginfo.getData(PRICING_DATA_ISSUEDATE, ISNOTNULL)))->get()).get();
	const LADate firstfixingdate = dynamic_cast<LADataDate &>((&(fginfo.getData(PRICING_DATA_FIRSTFIXINGDATE, ISNOTNULL)))->get()).get();
	const DateVector fixingdates = dynamic_cast<LADataDates &>((&(fginfo.getData(PRICING_DATA_FIXINGDATES, ISNOTNULL)))->get()).get();
	const DateVector paydates = dynamic_cast<LADataDates &>((&(fginfo.getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)))->get()).get();
	const DateVector cfstartdates = dynamic_cast<LADataDates &>((&(fginfo.getData(PRICING_DATA_CFCALCSTARTDATES, ISNOTNULL)))->get()).get();
	const DateVector cfenddates = dynamic_cast<LADataDates &>((&(fginfo.getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL)))->get()).get();

	// get base attrubues
	const DoubleVector basenotionals = dynamic_cast<LADataDoubles &>((&(fginfo.getData(PRICING_DATA_BASEFUNDINGNOTIONALS, ISNOTNULL)))->get()).get();
	const LAString basecur = ((LAString)(dynamic_cast<LADataString &>((&(fginfo.getData(PRICING_DATA_BASEFUNDINGCURRENCY, ISNOTNULL)))->get()).get())).toUpper();
	const LAPriceDataDayCount basedc = dynamic_cast<LAPriceDataDayCount &>(fginfo.getData(PRICING_DATA_BASEFUNDINGDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataDayCount baseindexdc = dynamic_cast<LAPriceDataDayCount &>(fginfo.getData(PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar basecal = dynamic_cast<LAPriceDataCalendar&>(fginfo.getData(PRICING_DATA_BASEFUNDINGCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule basesld = dynamic_cast<LAPriceDataSlidingRule&>(fginfo.getData(PRICING_DATA_BASEFUNDINGSLIDINGRULE, ISNOTNULL).get());
	const DoubleVector baseadjustratios = dynamic_cast<LADataDoubles &>((&(fginfo.getData(PRICING_DATA_BASEFUNDINGSETTLEMENTADJUSTRATIOS, ISNOTNULL)))->get()).get();

	// get modified dataValues
	const DoubleVector mdfynotionals = dynamic_cast<LADataDoubles &>((&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGNOTIONALS, ISNOTNULL)))->get()).get();
	const LAString mdycur = ((LAString)(dynamic_cast<LADataString &>((&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGCURRENCY, ISNOTNULL)))->get()).get())).toUpper();
	const LAPriceDataDayCount mdydc = dynamic_cast<LAPriceDataDayCount &>(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataDayCount mdyindexdc = dynamic_cast<LAPriceDataDayCount &>(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar mdycal = dynamic_cast<LAPriceDataCalendar&>(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule mdysld = dynamic_cast<LAPriceDataSlidingRule&>(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE, ISNOTNULL).get());
	const DoubleVector mdyadjustratios = dynamic_cast<LADataDoubles &>((&(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGSETTLEMENTADJUSTRATIOS, ISNOTNULL)))->get()).get();

	// get fx object and yield object
	LADate asof;
	LAMathFXEntity* pfx = 0;
	LAMathYieldCurve* pbasecurve = 0;
	LAMathYieldCurve* pmdycurve = 0;
	if (pathref.get().get().isTypeOf(ENTITY_PATH))
	{
		LAMathPathEntity* ppath = &(dynamic_cast<LAMathPathEntity&>(pathref.get().get()));
		asof = dynamic_cast<LADataDate &>((&ppath->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL))->get()).get();
		LADataMultiReference& initialrefs = ppath->getInitialValues();
		const LADataStrings& sdenames = ppath->getSDEAttrNames();
		for (unsigned int i = 0; i < initialrefs.getSize(); i++)
		{
			const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(ppath->getData(sdenames[i]).get());

			if (sde.getSDEPathType() == IR)
			{
				const LAString &ccy = sde.getCurrency();

				if (ccy == basecur)
					pbasecurve = dynamic_cast<LAMathYieldCurve *>(&initialrefs.get(i).get());
				else if (ccy == mdycur)
					pmdycurve = dynamic_cast<LAMathYieldCurve *>(&initialrefs.get(i).get());

				continue;
			}

			if (sde.getSDEPathType() == FX)
			{
				// specifying currency pair is not necessary because all FX SDEs share the common initial value
				pfx = dynamic_cast<LAMathFXEntity *>(&initialrefs.get(i).get());
				break;
			}
		}
	}
	else
	{
		LAMathPlainVanillaEntity& path = dynamic_cast<LAMathPlainVanillaEntity&>(pathref.get().get());
		asof = path.getAsOfDate();
		pbasecurve = &path.getIRCurve(basecur);
		pmdycurve = &path.getIRCurve(mdycur);
		pfx = dynamic_cast<LAMathFXEntity *>(&path.getFXEntity().get().get());
	}
	const double fxspot = pfx->getRate(basecur, mdycur, 0.0);

	// get base curve names and interpolation
	LAString fundingcurvefname, strcurvefname;
	LADataHolder* dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGFORECASTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		fundingcurvefname = dynamic_cast<LADataString &>(dh->get()).get();
	}
	const LAInterpolationBase* pInterbase = 0;
	LAString bfbasecurvename = pbasecurve->getCurveType();
	if (fundingcurvefname.size() > 0 && bfbasecurvename != fundingcurvefname)
		pInterbase = &(pbasecurve->getDFInterpolation(&fundingcurvefname));
	else
		pInterbase = &(pbasecurve->getDFInterpolation(&bfbasecurvename));

	// get modified curve names and interpolation
	dh = &(fginfo.getData(PRICING_DATA_MODIFIEDFUNDINGFORECASTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		strcurvefname = dynamic_cast<LADataString &>(dh->get()).get();
	}
	const LAInterpolationBase* pIntermdfy = 0;
	LAString bfmdycurvename = pmdycurve->getCurveType();
	if (strcurvefname.size() > 0 && bfmdycurvename != strcurvefname)
		pIntermdfy = &(pmdycurve->getDFInterpolation(&strcurvefname));
	else
		pIntermdfy = &(pmdycurve->getDFInterpolation(&bfmdycurvename));

	// cashlets of the funding leg
	dh = &(fundingleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference &>(dh->get());

	unsigned int insertpos = numeric_limits<unsigned int>::max();
	unsigned int cashSize = cashlets.getSize();
	for (unsigned int i = 0; i < cashSize; i++)
	{
		// get cashlet
		LAObject& fdcash = cashlets.get(i).get();

		// continue if paymentdate <= asofdate
		dh = &(fdcash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const LADate paymentdate = dynamic_cast<LADataDate &>(dh->get()).get();
		if (asof >= paymentdate)
			continue;

		dh = &(fdcash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			continue;
		}
		LADataMultiReference& coupons = dynamic_cast<LADataMultiReference &>(dh->get());
		if (coupons.getSize() != 1)
			throw LACoreInvalidData("Funding Coupon size must be 1",__FILE__,__LINE__);

		// update insert position 
		insertpos = (i < insertpos) ? i : insertpos;

		// coupon object
		LAObject& fdcoupon = coupons.get(0).get();
		dh = &(fdcoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		LADataMultiReference& indexs = dynamic_cast<LADataMultiReference &>(dh->get());
		if (indexs.getSize() != 1)
			throw LACoreInvalidData("Funding Index size must be 1",__FILE__,__LINE__);

		#pragma region PreparationFromCashlets 
		
		// original cashflow daycount
		LAPriceDataDayCount cfdc = dynamic_cast<LAPriceDataDayCount &>((&(fdcash.getData(PRICING_DATA_BASEFUNDINGDAYCOUNT, ISNOTNULL)))->get());
		cfdcs.push_back(cfdc);

		#pragma endregion PreparationFromCashlets 

		#pragma region PreparationFromIndexes

		// index object
		LAObject& fdindex = indexs.get(0).get();

		// index type
		std::vector<LAString> indextype(2);
		dh = &(fdindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
		indextype[0] = dynamic_cast<LADataString &>(dh->get()).get();
		indextype[0].toUpper();

		// index accesory
		dh = &(fdindex.getData(PRICING_DATA_ACCESSORY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			indextype[1] = dynamic_cast<LADataString &>(dh->get()).get();
			indextype[1].toUpper();
		}
		
		indextypes.push_back(indextype);

		#pragma endregion PreparationFromIndexes
	}

	// get end date of the funding leg
	dh = &(cashlets.get(cashSize - 1).get().getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
	LADate finalExDate(dynamic_cast<LADataDate &>(dh->get()).get());

	// get PaymentTerm 
	unsigned int vecSize = paydates.size();
	DoubleVector payterms(vecSize), baseDFs(vecSize), mdyDFs(vecSize), baseLibors(vecSize), mdyLibors(vecSize),
				cfstartterms(vecSize), cfendterms(vecSize), basetaus(vecSize), mdytaus(vecSize), retspread(vecSize);

	LAString bStrname = pbasecurve->getCurveType();
	LAString fStrname = pmdycurve->getCurveType();
	pbasecurve->setCurveType("STD");
	pmdycurve->setCurveType("STD");

	// initial value of bootstrap
	double sumbase = 0.0;
	double summdy = 0.0;
	double betaplus = 0.0;
	
	// check whether initial notional exchange is already exercised.
	double baseDFs_old = 0.0;
	double mdyDFs_old = 0.0;
	if(settle < issuedate)
	{
		double firstterm = dc_act365.getTerm(asof,issuedate);
		baseDFs_old = pbasecurve->getBasisDF(firstterm);
		mdyDFs_old = pmdycurve->getBasisDF(firstterm);
	}

	// get spreads
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGSPREADS, ISNOTNULL));
	DoubleVector basespreads = dynamic_cast<LADataDoubles &>(dh->get()).get();
	if (basespreads.size() != vecSize)
		throw LACoreInvalidData("baseSpread size must be the same as vecSize",__FILE__,__LINE__);

	// get fixed rates
	dh = &(fginfo.getData(PRICING_DATA_BASEFUNDINGRATES, ISNOTNULL));
	DoubleVector basefixedrates = dynamic_cast<LADataDoubles &>(dh->get()).get();
	if (basefixedrates.size() != vecSize)
		throw LACoreInvalidData("baseFixedRate size must be the same as vecSize",__FILE__,__LINE__);

	// find insert position
	if (spreadmat.size() < vecSize)
		throw LACoreInvalidData("Error Pos",__FILE__,__LINE__);

	// calculate funding spreads
	for (unsigned int i = 0; i < vecSize; i++)
	{
		payterms[i]	= dc_act365.getTerm(asof, paydates[i]);
		basetaus[i] = cfdcs[i].getTerm(cfstartdates[i], cfenddates[i]);
		mdytaus[i] = mdydc.getTerm(cfstartdates[i], cfenddates[i]);
		if(basetaus[i] == 0. || mdytaus[i] == 0.)
			throw LACoreInvalidData("Invalid Cashflow: CF start date is same as end date.",__FILE__,__LINE__);

		// Libor (L(T_{i-1},T_i), paied at T_i) Calculation
		// These calculation logic for Libor accrual is based on following implementation:
		// Start date: LAPricePayOff::SetUpEntityOfIndex(LADate&, MBentity&, LAObject&, DateVector*, bool)
		// End date: LAMathIndexEntity::setUpForIR(void)
		// And variable SPOTLAG must be consistent with definition in LAMathIndexEntity.cpp
		if (indextypes[i][0] == LIBOR)
		{
			LADate baseIdxAccDate_s = (fixingdates[i] < asof) ? asof :  basecal.getBusinessDay(fixingdates[i], SPOTLAG);
			LADate mdyIdxAccDate_s = (fixingdates[i] < asof) ? asof : mdycal.getBusinessDay(fixingdates[i], SPOTLAG);

			LADate baseIdxAccDate_e = LAMathDateCalculations::getDate(baseIdxAccDate_s, indextypes[i][1], basesld, &basecal, true);
			LADate mdyIdxAccDate_e = LAMathDateCalculations::getDate(mdyIdxAccDate_s, indextypes[i][1], mdysld, &mdycal, true);

			double indexTerm_s, indexTerm_e, indexAccrualTerm;
			
			indexTerm_s = dc_act365.getTerm(asof, baseIdxAccDate_s);
			indexTerm_e = dc_act365.getTerm(asof, baseIdxAccDate_e);
			indexAccrualTerm = baseindexdc.getTerm(baseIdxAccDate_s, baseIdxAccDate_e, false);
			if (!(LAMath::abs(indexAccrualTerm) > 0.))
				throw LACoreInvalidData("Libor accrual term was evaluated to 0.",__FILE__,__LINE__);
			baseLibors[i] = 1.0/indexAccrualTerm*(pInterbase->value(indexTerm_s)/pInterbase->value(indexTerm_e) - 1.0);

			indexTerm_s = dc_act365.getTerm(asof, mdyIdxAccDate_s);
			indexTerm_e = dc_act365.getTerm(asof, mdyIdxAccDate_e);
			indexAccrualTerm = mdyindexdc.getTerm(mdyIdxAccDate_s, mdyIdxAccDate_e, false);
			if (!(LAMath::abs(indexAccrualTerm) > 0.))
				throw LACoreInvalidData("Libor accrual term was evaluated to 0.",__FILE__,__LINE__);
			mdyLibors[i] = 1.0/indexAccrualTerm*(pIntermdfy->value(indexTerm_s)/pIntermdfy->value(indexTerm_e) - 1.0);
		}
		else if (indextypes[i][0] == FIXEDRATE)
		{
			baseLibors[i] = basefixedrates[i];
			mdyLibors[i] = 0.;
		}
		else
			throw LACoreInvalidData("IndexType error: IndexType must be Libor or FixedRate for funding change",__FILE__,__LINE__);

		// calculation of par spread
		double exTerm = (i < vecSize - 1) ? payterms[i] : dc_act365.getTerm(asof, finalExDate);

		baseDFs[i] = pbasecurve->getBasisDF(payterms[i]);
		double exBaseDF = pbasecurve->getBasisDF(exTerm);
		// if a payment has been already settled, an artificial exchange value should be excluded   
		if(paydates[i] <= settle)
		{
			exBaseDF = 0.;
		}

		const unsigned int nextpos = (i == vecSize - 1) ? (vecSize - 1 + insertpos) : (i + insertpos + 1);
		const unsigned int currentpos = i + insertpos;
		const unsigned int previouspos = (i + insertpos - 1 <= 0) ? 0 : i + insertpos - 1;

		double baseleg = 
			baseadjustratios[currentpos] * basenotionals[nextpos] * exBaseDF
			+ baseadjustratios[currentpos] * basenotionals[currentpos] * (baseLibors[i] + basespreads[i]) * basetaus[i] * baseDFs[i]
			- baseadjustratios[previouspos] * basenotionals[currentpos] * baseDFs_old;
		sumbase += (baseleg * fxspot);
		
		mdyDFs[i] = pmdycurve->getBasisDF(payterms[i]);
		double exMdyDF = pmdycurve->getBasisDF(exTerm);
		// if a payment has been already settled, an artificial exchange value should be excluded   
		if(paydates[i] <= settle)
		{
			exMdyDF = 0.;
		}

		double mdyleg = 
			mdyadjustratios[currentpos] * mdfynotionals[nextpos] * exMdyDF
			+ mdyadjustratios[currentpos] * mdfynotionals[currentpos] * mdyLibors[i] * mdytaus[i] * mdyDFs[i]
			- mdyadjustratios[previouspos] * mdfynotionals[currentpos] * mdyDFs_old;
		summdy += mdyleg;
		
		retspread[i] = (sumbase-summdy-betaplus)/mdytaus[i]/mdyDFs[i]/mdfynotionals[currentpos]/mdyadjustratios[currentpos];
		betaplus += mdyadjustratios[currentpos] * mdfynotionals[currentpos] * retspread[i] * mdytaus[i] * mdyDFs[i];

		spreadmat[currentpos][0] = 1.;
		spreadmat[currentpos][1] = (indextypes[i][0] == LIBOR ? retspread[i] : 0.);
		fixedrates[currentpos] = (indextypes[i][0] == FIXEDRATE ? retspread[i] : 0.);

		baseDFs_old = exBaseDF;
		mdyDFs_old = exMdyDF;
	}

	//add atribute
	fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGSPREADS);
	fginfo.add(PRICING_DATA_MODIFIEDFUNDINGSPREADS, new LADataDoubles(retspread));

	return;
}


// create funding change data
/*!
	@brief	setup trigger coupon change type
	@param[in, out] trade trade object
	@param[in, out] trigger trigger info object

*/
void
LAPriceCashFlowGenerator::createFundingChangeAttribute(LAObject& trade, LAObject& fundingleg, LAObject& strleg, LAObject& fginfo) const
{
	// get path object
	LADataHolder* dh = &(trade.getData(IR_MODEL_DATA_PATHENTITY, ISNOTNULL));
	LADataReference& pathref = dynamic_cast<LADataReference &>(dh->get());
	LADate asofDate;
	if (pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
	{
		const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());
		//if (valuemehod.isTypeOf(FN_IR_NDSSWAPTIONVALUE))
		//{
		//	const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(pathref.get().get());
		//	asofDate = path.getAsOfDate();
		//}
		//else
		//{
			return;
		//}
	}
	else if (!pathref.get().get().isTypeOf(ENTITY_PATH))
	{
		throw LACoreInvalidData("Error createFundingChangeAttribute: invalid path type", __FILE__, __LINE__);
	}
	else
	{
		const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(pathref.get().get());
		asofDate = path.getAsOfDate();
	}

	// set IsNotionalExchange from TRUE to FALSE
	dh = &(fundingleg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, ISNOTNULL));
	if (!dynamic_cast<LADataBool &>(dh->get()).get())
		throw LACoreInvalidData("Initial exchange must be true when Funding Change",__FILE__,__LINE__);
	dynamic_cast<LADataBool &>(dh->get()).set(false);
	
	dh = &(fundingleg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
	if (!dynamic_cast<LADataBool &>(dh->get()).get())
		throw LACoreInvalidData("Final exchange must be true when Funding Change",__FILE__,__LINE__);
	dynamic_cast<LADataBool &>(dh->get()).set(false);
	
	dh = &(strleg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, ISNOTNULL));
	if (!dynamic_cast<LADataBool &>(dh->get()).get())
		throw LACoreInvalidData("Initial exchange must be true when Funding Change",__FILE__,__LINE__);
	dynamic_cast<LADataBool &>(dh->get()).set(false);
	
	dh = &(strleg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
	if (!dynamic_cast<LADataBool &>(dh->get()).get())
		throw LACoreInvalidData("Final exchange must be true when Funding Change",__FILE__,__LINE__);
	dynamic_cast<LADataBool &>(dh->get()).set(false);

	// get first funding cashlet
	LADataMultiReference& fdcashlets = dynamic_cast<LADataMultiReference &>((&(fundingleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL)))->get());
	LAObject& fdcash = fdcashlets.get(0).get();

	// set base currency
	const LAString basecur = ((LAString)dynamic_cast<LADataString &>((&(fdcash.getData(PRICING_DATA_CURRENCY, ISNOTNULL)))->get()).get()).toUpper();
	fginfo.remove(PRICING_DATA_BASEFUNDINGCURRENCY);
	fginfo.add(PRICING_DATA_BASEFUNDINGCURRENCY, new LADataString()).convertFromString(basecur);

	// set IsChangedByFunding true
	bool IsChangedByFunding = false;
	dh = &(trade.getData(PRICING_DATA_ISCHANGEDBYFUNDING, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		IsChangedByFunding = dynamic_cast<LADataBool &>(dh->get()).get();
	}

	// 
	LADate issuedate("99991231");
	LADate firstfixingdate("99991231");
	DoubleVector spreads;
	DoubleVector fixedrates;
	DateVector paymentdates;
	DateVector startdates;
	DateVector enddates;
	DateVector fixingdates;
	DoubleVector basenotionals;

	for (unsigned int i = 0; i < fdcashlets.getSize(); i++)
	{
		// get cashlet
		LAObject& fdcash = fdcashlets.get(i).get();

		// save original notional amounts
		if (!IsChangedByFunding)
		{
			const double notional = LAMath::abs(dynamic_cast<LADataDouble &>((&(fdcash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL)))->get()).get());
			basenotionals.push_back(notional);
		}

		// remove notional cashflow
		dh = &(fdcash.getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			fdcash.remove(PRICING_CALIBRATION_DATAOTIONALCF);

		// cashflow start date
		dh = &(fdcash.getData(PRICING_DATA_CFCALCSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LADate startdate = dynamic_cast<LADataDate &>(dh->get()).get();
			issuedate = (startdate < issuedate) ? startdate : issuedate;
		}

		// continue if paymentdate <= asofdate
		const LADate paymentdate = dynamic_cast<LADataDate &>((&(fdcash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL)))->get()).get();
		issuedate = (paymentdate < issuedate) ? paymentdate : issuedate;
		if (asofDate >= paymentdate)
			continue;

		// continue if there is no coupon infos
		dh = &(fdcash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		// throw an error if the payment timing is not ARREAR
		LAString paytiming = ((LAString)dynamic_cast<LADataString &>((&(fdcash.getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL)))->get()).get()).toUpper();
		if (paytiming != "ARREAR")
			throw LACoreInvalidData("Invalid payment timing: only ARREAR is supported", __FILE__, __LINE__);

		// save original daycount conventions for calculating funding spreads later
		if (!IsChangedByFunding)
		{
			const LAPriceDataDayCount& daycount = dynamic_cast<LAPriceDataDayCount &>((&(fdcash.getData(PRICING_DATA_DAYCOUNT, ISNOTNULL)))->get());
			fdcash.remove(PRICING_DATA_BASEFUNDINGDAYCOUNT);
			fdcash.add(PRICING_DATA_BASEFUNDINGDAYCOUNT, new LAPriceDataDayCount(daycount));
		}

		// get coupon object
		LADataMultiReference& fdcoupons = dynamic_cast<LADataMultiReference &>((&(fdcash.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL)))->get());
		if (fdcoupons.getSize() != 1)
		{
			throw LACoreInvalidData("Funding Coupon size must be 1", __FILE__, __LINE__);
		}
		LAObject& fdcoupon = fdcoupons.get(0).get();

		// get index object and index type
		LADataMultiReference& fdindexs = dynamic_cast<LADataMultiReference &>((&(fdcoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL)))->get());
		if (fdindexs.getSize() != 1)
		{
			throw LACoreInvalidData("Funding Index size must be 1", __FILE__, __LINE__);
		}
		LAObject& fdindex = fdindexs.get(0).get();
		const LAString indextype = ((LAString)dynamic_cast<LADataString &>((&(fdindex.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL)))->get()).get()).toUpper();

		// get base spread
		DoubleVector coeff = dynamic_cast<LADataDoubles &>((&(fdcoupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL)))->get()).get();
		if (coeff.size() != 2 || coeff[0] != 1.0)
		{
			throw LACoreInvalidData("Wrong Funding Spread Set", __FILE__, __LINE__);
		}
		spreads.push_back(coeff[1]);

		// get fixed rates
		if (indextype == FIXEDRATE)
		{
			const double rate = dynamic_cast<LADataDouble &>((&(fdindex.getData(PRICING_DATA_FIXEDRATE, ISNOTNULL)))->get()).get();
			fixedrates.push_back(rate);
		}
		else if(indextype == LIBOR)
		{
			fixedrates.push_back(0.);
		}

		// fixing date
		dh = &(fdindex.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LADate fixingdate = dynamic_cast<LADataDate &>(dh->get()).get();
			fixingdates.push_back(fixingdate);
			firstfixingdate = (fixingdate <= firstfixingdate) ? fixingdate : firstfixingdate;
		}
		else
		{
			if (indextype != FIXEDRATE)
			{
				LAString msg = LAString("Data [") + PRICING_DATA_FIXINGDATE + "] is necessary for the calculation of index.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			fixingdates.push_back(LADate());
		}

		paymentdates.push_back(paymentdate);
		const LADate startdate = dynamic_cast<LADataDate &>((&(fdcash.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL)))->get()).get();
		startdates.push_back(startdate);
		const LADate enddate = dynamic_cast<LADataDate &>((&(fdcash.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL)))->get()).get();
		enddates.push_back(enddate);
	}

	// set base spreads and fixed rates
	fginfo.remove(PRICING_DATA_BASEFUNDINGSPREADS);
	fginfo.add(PRICING_DATA_BASEFUNDINGSPREADS, new LADataDoubles(spreads));
	fginfo.remove(PRICING_DATA_BASEFUNDINGRATES);
	fginfo.add(PRICING_DATA_BASEFUNDINGRATES, new LADataDoubles(fixedrates));
	fginfo.remove(PRICING_DATA_ISSUEDATE);
	fginfo.add(PRICING_DATA_ISSUEDATE, new LADataDate(issuedate));
	fginfo.remove(PRICING_DATA_FIRSTFIXINGDATE);
	fginfo.add(PRICING_DATA_FIRSTFIXINGDATE, new LADataDate(firstfixingdate));
	fginfo.remove(PRICING_DATA_FIXINGDATES);
	fginfo.add(PRICING_DATA_FIXINGDATES, new LADataDates(fixingdates));
	fginfo.remove(PRICING_DATA_PAYMENTDATES);
	fginfo.add(PRICING_DATA_PAYMENTDATES, new LADataDates(paymentdates));
	fginfo.remove(PRICING_DATA_CFCALCSTARTDATES);
	fginfo.add(PRICING_DATA_CFCALCSTARTDATES, new LADataDates(startdates));
	fginfo.remove(PRICING_DATA_CFCALCENDDATES);
	fginfo.add(PRICING_DATA_CFCALCENDDATES, new LADataDates(enddates));
	fginfo.remove(PRICING_DATA_BASEFUNDINGNOTIONALS);
	fginfo.add(PRICING_DATA_BASEFUNDINGNOTIONALS, new LADataDoubles(basenotionals));

	// set structure cashlet
	LADataMultiReference& strcashlets = dynamic_cast<LADataMultiReference &>((&(strleg.getData(PRICING_DATA_CASHLETS, ISNOTNULL)))->get());
	LAObject& strcash = strcashlets.get(0).get();

	// set modified currency
	const LAString mdycur = ((LAString)dynamic_cast<LADataString &>((&(strcash.getData(PRICING_DATA_CURRENCY, ISNOTNULL)))->get()).get()).toUpper();
	fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGCURRENCY);
	fginfo.add(PRICING_DATA_MODIFIEDFUNDINGCURRENCY, new LADataString()).convertFromString(mdycur);

	// get adjust ratios, which represents FX(fix)/FX(pay), only relevant to non-deliverable exotic products
	DoubleVector base_adjust_ratios;
	DoubleVector mdy_adjust_ratios;
	for (unsigned int i = 0; i < fdcashlets.getSize(); ++i)
	{
		LAObject& fdcash = fdcashlets.get(i).get();
		dh = &(fdcash.getData(PRICING_DATA_SETTLEMENTADJUSTRATIO, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			base_adjust_ratios.push_back(dynamic_cast<LADataDouble &>(dh->get()).get());
		else
			base_adjust_ratios.push_back(1.0);

		const LADate fdpaydate = dynamic_cast<LADataDate &>((&(fdcash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL)))->get()).get();
		double mdy_adjust_ratio = 1.0;
		for (unsigned int j = 0; j < strcashlets.getSize(); ++j)
		{
			LAObject& strcash = strcashlets.get(j).get();
			const LADate strpaydate = dynamic_cast<LADataDate &>((&(strcash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL)))->get()).get();
			if (fdpaydate == strpaydate)
			{
				dh = &(strcash.getData(PRICING_DATA_SETTLEMENTADJUSTRATIO, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					mdy_adjust_ratio = dynamic_cast<LADataDouble &>(dh->get()).get();

				break;
			}
		}
		mdy_adjust_ratios.push_back(mdy_adjust_ratio);
	}
	fginfo.remove(PRICING_DATA_BASEFUNDINGSETTLEMENTADJUSTRATIOS);
	fginfo.add(PRICING_DATA_BASEFUNDINGSETTLEMENTADJUSTRATIOS, new LADataDoubles(base_adjust_ratios));
	fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGSETTLEMENTADJUSTRATIOS);
	fginfo.add(PRICING_DATA_MODIFIEDFUNDINGSETTLEMENTADJUSTRATIOS, new LADataDoubles(mdy_adjust_ratios));

	// set modified notional amounts
	DoubleVector mdynotionals;
	const double mdynotional_at_start = LAMath::abs(dynamic_cast<LADataDouble &>((&(strcash.getData(PRICING_CALIBRATION_DATAOTIONALCF, ISNOTNULL)))->get()).get());
	for (unsigned int i = 0; i < fdcashlets.getSize(); ++i)
	{
		const double mdynotional = mdynotional_at_start * (basenotionals[i] / basenotionals[0]);
		mdynotionals.push_back(mdynotional);
	}
	fginfo.remove(PRICING_DATA_MODIFIEDFUNDINGNOTIONALS);
	fginfo.add(PRICING_DATA_MODIFIEDFUNDINGNOTIONALS, new LADataDoubles(mdynotionals));

	// remove notional cashflow
	for (unsigned int i = 0; i < strcashlets.getSize(); i++)
	{
		LAObject& strcash = strcashlets.get(i).get();
		dh = &(strcash.getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			strcash.remove(PRICING_CALIBRATION_DATAOTIONALCF);
	}

	// set flag
	trade.remove(PRICING_DATA_ISCHANGEDBYFUNDING);
	trade.add(PRICING_DATA_ISCHANGEDBYFUNDING, new LADataBool(true));

	// for bond funding leg
	bool is_bond = false;
	dh = &(fundingleg.getData(PRICING_DATA_ISBONDFUNDINGLEG, NOCHECK));
	if(dh->isDefined() && !dh->isNull())
	{
		is_bond = dynamic_cast<LADataBool&>(dh->get()).get();
	}
	if (!is_bond)
	{
		dh = &(fundingleg.getData(PRICING_DATA_CURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LAString chkcur = dynamic_cast<LADataString &>(dh->get()).get();
			if (chkcur == mdycur)
				throw LACoreInvalidData("Currency Convert Error",__FILE__,__LINE__);
			dynamic_cast<LADataString &>(dh->get()).set(mdycur);
		}
	}

	return;
}

/*!
	@brief	setup renotional 
	@param[in] trade trade object
*/
void
LAPriceCashFlowGenerator::setUpRenotional(LAObject& trade) const
{
	
	// ! Check whether renotional is needed

	LADataHolder* dh =  &( trade.getData( CALIBRATION_DATA_UNDERLYINGS, NOCHECK ) );
	
	if ( !dh->isDefined() || dh->isNull() )
	{
		return;
	}
	
	LADataMultiReference* mr = dynamic_cast< LADataMultiReference* >( &( dh->get() ) );
	
	for ( size_t i = 0; i < mr->getSize(); i++ )
	{
		dh = &(mr->get(i).getData(PRICING_DATA_INPUTTYPE, ISNOTNULL));
		LAString inputtype = dynamic_cast<const LADataString &>(dh->get());
		if (DETAIL == inputtype.toUpper())
			continue;

		LADataHolder * ahRenotional = &( mr->get( i ).getData( PRICING_DATA_ISRENOTIONAL, NOCHECK ) );

		if ( !ahRenotional->isDefined() || ahRenotional->isNull() )
		{
			continue;
		}
		else
		{
			bool isRenotional = dynamic_cast< LADataBool& >( ahRenotional->get() ).get();
			if ( isRenotional )
			{
				// ! Renotional 
				trade.remove(CALIBRATION_DATA_ISRENOTIONALSETUPFINISHED);
				createRenotional( trade, mr->get( i ).get() );
				trade.add(CALIBRATION_DATA_ISRENOTIONALSETUPFINISHED, new LADataBool(true));
			}
			else
			{
				continue;
			}
		}
	}


}


//
//----------
//

/*!
	@brief	create renotional 
	@param[in] trade trade object
	@param[in] leg leg object
*/

void
LAPriceCashFlowGenerator::createRenotional( LAObject& trade, LAObject& leg ) const
{
	// ! Get PathEntity
	LADataHolder* dh = &( trade.getData( PRICING_DATA_PATHENTITY, ISNOTNULL ) );
	LADataReference& arPath = dynamic_cast< LADataReference& >( dh->get() );
	
	LAObject* pathEntity = NULL;

	if ( arPath.get().get().isTypeOf( ENTITY_PATH ) )
	{
		pathEntity = &( dynamic_cast< LAMathPathEntity& >( arPath.get().get() ) );
		
	}
	else if ( arPath.get().get().isTypeOf( ENTITY_PLAINVANILLA ) )
	{
		pathEntity = &( dynamic_cast< LAMathPlainVanillaEntity& >( arPath.get().get() ) );
	}
	else
	{
		LAString msg = "pathEntity type is wrong! LAPriceCashFlowGenerator::createRenotional";
		throw LACoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
	}

	// ! Get asofDate
	dh = &( pathEntity->getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ) );
	LADate asofDate = dynamic_cast< LADataDate& >( dh->get() ).get();

	// ! Get leg currency
	dh = &( leg.getData( PRICING_DATA_CURRENCY, ISNOTNULL ) );
	LAString targetCcy = dynamic_cast< LADataString& >( dh->get() ).get();

	// ! Get base currency and base notional
	LAString baseCcy = "";
	double baseNotional;

	dh = &( trade.getData( CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL ) );
	const LADataMultiReference& mr = dynamic_cast< LADataMultiReference& >( dh->get() );
	
	for (size_t i = 0; i < mr.getSize(); i++  )
	{
		dh = & ( mr.get( i ).getData( PRICING_DATA_CURRENCY, ISNOTNULL ) );
		LAString tmpCcy = dynamic_cast< LADataString& >( dh->get() ).get();
		if ( tmpCcy != targetCcy )
		{
			// ! Get base currency
			baseCcy = tmpCcy;
			
			// ! Get base notional
			dh = &( mr.get( i ).getData( PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL ) );
			baseNotional = dynamic_cast< LADataDouble& >( dh->get() ).get();

			break;
		}
		else
		{
			continue;
		}	
	}
	
	if (baseCcy == LAString(""))
		throw LACoreInvalidData("Base currency does not exist in renotinal.",__FILE__,__LINE__);

	// ! Get cashlets 
	dh = &( leg.getData( PRICING_DATA_CASHLETS, ISNOTNULL ) );
	const LADataMultiReference& mrCashlets = dynamic_cast< LADataMultiReference& >( dh->get() );
    
    for(unsigned int i = 0; i < mrCashlets.getSize(); i++){
        dh = &mrCashlets.get(i).getData(PRICING_CALIBRATION_DATAOTIONALCFISFIXED);
        if(dh->isDefined() && !dh->isNull()) continue;
        dh = &mrCashlets.get(i).getData(PRICING_CALIBRATION_DATAOTIONALCF);
        const bool is_fixed = dh->isDefined() && !dh->isNull();
        mrCashlets.get(i).add(PRICING_CALIBRATION_DATAOTIONALCFISFIXED, new LADataBool(is_fixed));
    }

	//calc adjust value
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	LAInterpolationBase* pInter_adjust = NULL;
	if (pathEntity->isTypeOf(ENTITY_PLAINVANILLA))
	{
		LAMathYieldCurve& yc_base = dynamic_cast< LAMathPlainVanillaEntity *>(pathEntity)->getIRCurve(baseCcy);
		//xccybasis adjust
		const LADataHolder* ah_const = &(yc_base.getYieldData().get().get().getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK));
		if (ah_const->isDefined() && !ah_const->isNull())
		{
			const DoubleArray& adjustValue_term = dynamic_cast<const LADataDoubles&> (ah_const->get()).get();
			const DoubleArray& adjustValue = dynamic_cast<const LADataDoubles&> 
				((yc_base.getYieldData().get().get().getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL)).get()).get();
			pInter_adjust = dynamic_cast<LAInterpolationBase*> ((dynamic_cast<const LAPriceDataInterpolation&> 
				(yc_base.getYieldData().get().get().getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get())).getMethod().clone());
			pInter_adjust->set(adjustValue_term,adjustValue);
		}
	}

	// ! Get forward fx
	dh = &( pathEntity->getData( IR_MODEL_DATA_FXRATE, ISNOTNULL ) );
	LADataReference* ar = dynamic_cast< LADataReference* >( &( dh->get() ) );
	LAMathFXEntity* fxEntity = dynamic_cast< LAMathFXEntity* >( &( ar->get().get() ) );

	//FX Calendar
	const LAPriceDataCalendar* cal = 0;
	dh = &(leg.getData(PRICING_DATA_RENOTIONALCALENDAR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		cal = &(dynamic_cast<const LAPriceDataCalendar&>(dh->get()));
	//FX spot lag
	int offset = -1;
	dh = &(leg.getData(PRICING_DATA_RENOTIONALOFFSET, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		offset = dynamic_cast<const LADataInt&>(dh->get()).get();

	bool isforwardbase = true;
	LADate spotDate;
	if (pathEntity->isTypeOf(ENTITY_PLAINVANILLA))
	{	
		dh = &(leg.getData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			isforwardbase = dynamic_cast<const LADataBool &>(dh->get()).get();

		if (cal != 0 && !cal->isNull() && offset >= 0)
			spotDate = cal->getBusinessDay(asofDate, offset);
		else
			spotDate = fxEntity->getSpotDate(targetCcy, baseCcy, asofDate);
	}	
	
	size_t cashSize = mrCashlets.getSize();	
    const LADataHolder *cah; dh = NULL;
	for ( size_t i = 1; i < cashSize; i++ )
	{
		LAObject& current_cashlet = mrCashlets.get(i).get();
		LAObject& last_cashlet = mrCashlets.get(i-1).get();

		const bool last_is_fixed = dynamic_cast<const LADataBool&>(last_cashlet.getData(PRICING_CALIBRATION_DATAOTIONALCFISFIXED, ISNOTNULL).get()).get();
		if(last_is_fixed) continue;

		const LADate last_fixing = dynamic_cast<const LADataDate&>(last_cashlet.getData(PRICING_DATA_RENOTIONALFIXINGDATE, ISNOTNULL).get()).get();
		double forward = 0.0;
		double adjustValue = 1.;
		LADate paymentDate;
		if (cal != 0 && !cal->isNull() && offset >= 0)
			paymentDate= cal->getBusinessDay(last_fixing, offset);
		else
			paymentDate = fxEntity->getSpotDate(targetCcy, baseCcy, last_fixing);
		if (isforwardbase){
			if (spotDate > paymentDate){
				paymentDate = spotDate;
			}
			forward = fxEntity->getForwardRate( targetCcy, baseCcy, spotDate, paymentDate );
			double paymentTerm = dc_act365.getTerm(asofDate, paymentDate);
			if (pInter_adjust) adjustValue = pInter_adjust->value(paymentTerm);
		}
        else{
			forward = fxEntity->getForwardRate( targetCcy, baseCcy, spotDate, spotDate );
        }

        const double last_notional    = dynamic_cast<const LADataDouble&>(last_cashlet.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
        const double current_notional = baseNotional / forward * adjustValue;
        const double last_notional_cf = last_notional - current_notional;

        last_cashlet.remove(PRICING_CALIBRATION_DATAOTIONALCF);
        last_cashlet.LAObject::add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(last_notional_cf));
        dynamic_cast<LADataDouble&>(current_cashlet.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(current_notional);
	}
    LAObject& final_cashlet = mrCashlets.get(cashSize-1).get();
    const double final_notional = dynamic_cast<LADataDouble&>(final_cashlet.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
    final_cashlet.remove(PRICING_CALIBRATION_DATAOTIONALCF);
    final_cashlet.LAObject::add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(final_notional));

	if (pInter_adjust) delete pInter_adjust;
}



/*!
	@brief	setup funding change 
	@param[in, out] trade trade object

*/
void
LAPriceCashFlowGenerator::createOddTradeEntity(LAObject& trade) const
{

	//leg object
	//LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	//LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
	////check stub indextype
	//LAString oddindextype;
	////bool isoddtrade =false;
	//for (unsigned int i = 0; i < legs.getSize(); i++)
	//{
	//	LAObject& eleg = legs.get(i).get();
	//	dh = &(eleg.getData(PRICING_DATA_FIRSTODDDATE,NOCHECK));
	//	if (dh->isDefined() && !dh->isNull())
	//	{
	//		dh = &(eleg.getData(PRICING_DATA_FIRSTODDINDEXTYPE, NOCHECK));
	//		if (dh->isDefined() && !dh->isNull())
	//			oddindextype = dynamic_cast<LADataString &>(dh->get());
	//		if (oddindextype.size() > 0 && oddindextype != CURRENTINDEX)
	//		{
	//			//isoddtrade = true;
	//			trade.remove(PRICING_DATA_ISODDTRADE);
	//			trade.add(PRICING_DATA_ISODDTRADE, new LADataBool(true));
	//		}
	//	}

	//	dh = &(eleg.getData(PRICING_DATA_LASTODDDATE,NOCHECK));
	//	if (dh->isDefined() && !dh->isNull())
	//	{
	//		dh = &(eleg.getData(PRICING_DATA_LASTODDINDEXTYPE, NOCHECK));
	//		if (dh->isDefined() && !dh->isNull())
	//			oddindextype = dynamic_cast<LADataString &>(dh->get());
	//		if (oddindextype.size() > 0 && oddindextype != CURRENTINDEX)
	//		{
	//			//isoddtrade = true;
	//			trade.remove(PRICING_DATA_ISODDTRADE);
	//			trade.add(PRICING_DATA_ISODDTRADE, new LADataBool(true));
	//		}
	//	}
	//}

	//for stub modify
	trade.remove(PRICING_DATA_ISODDTRADE);
	trade.add(PRICING_DATA_ISODDTRADE, new LADataBool(true));

	
}

/*!
	@brief setUpEquivalentStrike

	@param[in, out] trade object
*/
void
LAPriceCashFlowGenerator::setUpEquivalentStrike(LAObject& trade) const
{
	LADataHolder* dh;
	
	//this function is called when input type is detail
	//&& ( this function is called when function is swaption
	//		|| when function is convergencevalue && subvalue is swaption)

	//in case data has already been set
	dh = &(trade.getData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		if (dynamic_cast<const LADataBool &>(dh->get()).get())
			return;
	}

	//input type
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference &>(dh->get());
	for (unsigned int i = 0; i < legs.getSize();i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_INPUTTYPE, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			return;
		
		LAString schtype = dynamic_cast<const LADataString&>(dh->get()).get();
		schtype.toUpper();
		if (schtype == MANUAL) return;//manual input
	}

	dh = &(trade.getData(CALIBRATION_DATA_VALUE, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
		return;

	const LADataValuation& val = dynamic_cast<const LADataValuation &>(dh->get());
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	if (val.isTypeOf(FN_IR_SWAPTIONVALUEFROMCASHFLOW))
	{

		trade.remove(PRICING_DATA_ISCALCEQUIVALENTSTRIKE);
		trade.add(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, new LADataBool(true));
		return;
	}
#else
	if (true)
	{
		
	}
#endif
	else if (val.isTypeOf(FN_IR_CONVERGENCEVALUE))
	{
		dh = &(trade.getData(PRICING_DATA_SUBVALUE, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			return;

		const LADataValuation& subval = dynamic_cast<const LADataValuation &>(dh->get());
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		if (subval.isTypeOf(FN_IR_SWAPTIONVALUEFROMCASHFLOW))
		{
			trade.remove(PRICING_DATA_ISCALCEQUIVALENTSTRIKE);
			trade.add(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, new LADataBool(true));
			return;
		}
#endif
	}

	//const LADataValuation& val = dynamic_cast<const LADataValuation &>(dh->get());
	////this function includes also fn_ir_convergencevalue
	////if (!val.isTypeOf(FN_IR_SWAPTIONVALUEFROMCASHFLOW))
	////	return;

	////in case data has already been set
	//dh = &(trade.getData(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, NOCHECK));
	//if (dh->isDefined() && !dh->isNull())
	//	return;

	////input type
	//dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
	//if (!dh->isDefined() || dh->isNull())
	//	return;

	//const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference &>(dh->get());
	//for (unsigned int i = 0; i < legs.getSize();i++)
	//{
	//	dh = &(legs.get(i).getData(PRICING_DATA_INPUTTYPE, NOCHECK));
	//	if (!dh->isDefined() || dh->isNull())
	//		return;
	//	
	//	LAString schtype = dynamic_cast<const LADataString&>(dh->get()).get();
	//	schtype.toUpper();
	//	if (schtype == MANUAL) return;//manual input

	//	dh = &(legs.get(i).getData(PRICING_DATA_ISAMORTIZE, ISNOTNULL));
	//	bool isamortize = dynamic_cast<const LADataBool &>(dh->get());
	//	if (isamortize)
	//	{
	//		trade.add(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, new LADataBool(true));
	//		return;
	//	}

	//	dh = &(legs.get(i).getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
	//	const LADataMultiReference& coupons = dynamic_cast<const LADataMultiReference &>(dh->get());
	//	if (coupons.getSize() != 1)
	//		throw LACoreInvalidData("Coupon of SwaptionValueFromCashFlow must be 1",__FILE__,__LINE__);

	//	dh = &(coupons.get(0).getData(PRICING_DATA_OPERATOR, ISNOTNULL));
	//	const LAPriceDataFunction& couponope = dynamic_cast<const LAPriceDataFunction &>(dh->get());
	//	if (!couponope.isTypeOf(FN_LINEAR))
	//		throw LACoreInvalidData("Operator of SwaptionValueFromCashFlow must be fn_linear",__FILE__,__LINE__);

	//	dh = &(coupons.get(0).getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
	//	const DoubleVector& coeffs = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	//	if(coeffs.back() != 0.0)
	//	{
	//		dh = &(coupons.get(0).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
	//		if (dh->isDefined() && !dh->isNull())
	//		{
	//			const LADataMultiReference &indexs = dynamic_cast<const LADataMultiReference &>(dh->get());
	//			dh = &(indexs.get(0).getData(PRICING_DATA_INDEXTYPE, NOCHECK));
	//			if (dh->isDefined() && !dh->isNull())
	//			{
	//				LAString indextype = dynamic_cast<const LADataString &>(dh->get());
	//				if (indextype.toUpper() != FIXED_RATE)
	//				{
	//					trade.remove(PRICING_DATA_ISCALCEQUIVALENTSTRIKE);
	//					trade.add(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, new LADataBool(true));
	//					return;
	//				}
	//			}
	//		}
	//	}
	//}

	////if not return, then false
	//trade.remove(PRICING_DATA_ISCALCEQUIVALENTSTRIKE);
	//trade.add(PRICING_DATA_ISCALCEQUIVALENTSTRIKE, new LADataBool(false));
	//return;
}

void
LAPriceCashFlowGenerator::createRenotionalInfo(const LADate& advancepaymentdate,
										const LADate& paymentdate,
										const LADate& paymentdate_unadjust, 
										const LADate& startdate,
										const LADate& enddate,
										LAObject& cashlet, LAObject& leg) const
{
	LADataHolder* dh;
	LADate fixingbasedate;
	LADate fixingdate;

	// isArrear
	dh = &(leg.getData(PRICING_DATA_RENOTIONALTIMING, ISNOTNULL));
	const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
	bool isarrear = LAPriceCFGenUtility::isArrear(timing);
	if (isarrear)
	{
		LAString msg = "arrear is not supported in renotional timing.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}
	//if (isarrear && advancepaymentdate == paymentdate)//index fixing timing is arrear but payment timing is advance
	//{
	//	//error
	//	LAString msg = "Index fixing timing is arrear but payment timing is advance";
	//		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	//}
	dh = &(leg.getData(PRICING_DATA_ISFIXINGPAYMENTDATEBASE, NOCHECK));
	if (dh->isDefined() && !dh->isNull()
		&& dynamic_cast<const LADataBool&>(dh->get()).get() == true)
		fixingbasedate = paymentdate;
	else
		fixingbasedate = enddate;
	

	dh = &(leg.getData(PRICING_DATA_RENOTIONALOFFSET, ISNOTNULL));
	int offset = dynamic_cast<const LADataInt&>(dh->get()).get();		
	//leg.remove(PRICING_DATA_RENOTIONALOFFSET);
	if (offset == 0) 
		fixingdate = fixingbasedate;
	else
	{
		//calendar
		LAPriceDataCalendar cal;
		dh = &(leg.getData(PRICING_DATA_RENOTIONALCALENDAR, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		}
		else
		{
			dh = &(leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
			cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		}
		fixingdate = cal.getBusinessDay(fixingbasedate, -offset);
	}
	
	cashlet.remove(PRICING_DATA_RENOTIONALFIXINGDATE);
	cashlet.add(PRICING_DATA_RENOTIONALFIXINGDATE, new LADataDate(fixingdate));
	
	
	return;
}


// consistency check for frquency
bool
LAPriceCashFlowGenerator::isConsistentFrequencyPair(const LAString& rollfreq, const LAString& payfreq) const
{
	bool ret = false;
	if (rollfreq == BUSINESS_DAYS || rollfreq == DAILY)
		ret = true;
	else if (rollfreq == WEEKLY)
	{
		if (payfreq == WEEKLY || payfreq == MONTHLY || payfreq == QUARTERLY 
			|| payfreq == SEMI_ANNUAL || payfreq == ANNUAL)
			ret = true;
		else
			ret = false;
	}
	else if (rollfreq == MONTHLY)
	{
		if (payfreq == MONTHLY || payfreq == QUARTERLY 
			|| payfreq == SEMI_ANNUAL || payfreq == ANNUAL)
			ret = true;
		else
			ret = false;
	}
	else if (rollfreq == QUARTERLY)
	{
		if (payfreq == QUARTERLY || payfreq == SEMI_ANNUAL || payfreq == ANNUAL)
			ret = true;
		else
			ret = false;
	}
	else if (rollfreq == SEMI_ANNUAL)
	{
		if (payfreq == SEMI_ANNUAL || payfreq == ANNUAL)
			ret = true;
		else
			ret = false;
	}
	else if (rollfreq == ANNUAL)
	{
		if (payfreq == ANNUAL)
			ret = true;
		else
			ret = false;
	}
	else
		ret = false;

	return ret;
}

void
LAPriceCashFlowGenerator::modifyLegInfo(LAObject& leg) const
{
    LADataHolder* dh;
    dh = &leg.getData(PRICING_DATA_COUPONINFOS);
    if(!dh->isDefined() || dh->isNull()) return;
    LAObject& coupon = dynamic_cast<LADataMultiReference&>(dh->get()).get(0).get();
    dh = &coupon.getData(PRICING_DATA_INDEXINFOS);
    if(!dh->isDefined() || dh->isNull()) return;
    LAObject& index = dynamic_cast<LADataMultiReference&>(dh->get()).get(0).get();
    LAString index_type = dynamic_cast<const LADataString&>(index.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
    index_type.toUpper();

    if(index_type!="OIS") return;


    dh = &leg.getData(PRICING_DATA_INDEXNAME);
    if(!dh->isDefined() || dh->isNull()) return;
    LAString index_name = dynamic_cast<const LADataString&>(dh->get()).get();
    index_name.toUpper();



    if(!(dh=&leg.getData(PRICING_DATA_ISCOMPOUNDINGCOUPON))->isDefined() || dh->isNull()){
        leg.add(PRICING_DATA_ISCOMPOUNDINGCOUPON, new LADataBool(true));
    }
    if(index_name == "USD_FF"){
        if(!(dh=&leg.getData(PRICING_DATA_COMPOUND_ON_ALL_DAYS))->isDefined() || dh->isNull()){
            leg.add(PRICING_DATA_COMPOUND_ON_ALL_DAYS, new LADataBool(true));
        }
    }
    if(!(dh=&leg.getData(PRICING_DATA_ROLLFREQUENCY))->isDefined() || dh->isNull()){
        leg.add(PRICING_DATA_ROLLFREQUENCY, new LADataString(BUSINESS_DAYS));
    }
    if(!(dh=&leg.getData(PRICING_DATA_COMPOUNDING_FUNCTION))->isDefined() || dh->isNull()){
        dh = &leg.add(PRICING_DATA_COMPOUNDING_FUNCTION, new LAPriceDataFunction());
        if(index_name == "USD_FF"){
            dh->convertFromString(FN_COMPOUNDING10_STR);
        }
        else{
            dh->convertFromString(FN_COMPOUNDING1_STR);
        }
    }
}

void 
LAPriceCashFlowGenerator::calcPaymentDates(LAObject& leg,
                                        DateVector& payment_dates,
                                        DateVector& payment_dates_unadjust,
                                        DateVector& payment_start_dates,
                                        DateVector& payment_end_dates,
                                        DateVector& roll_dates,
                                        DateVector& roll_dates_unadjust,
                                        DateVector& roll_start_dates,
                                        DateVector& roll_end_dates,
										bool& is_first_stub,
										bool& is_last_stub) const
{
    LADataHolder* dh;
    const bool is_comp = (dh=&leg.getData(PRICING_DATA_ISCOMPOUNDINGCOUPON))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    
    const LAString& freq         = dynamic_cast<const LADataString&>(leg.getData(PRICING_DATA_FREQUENCY, ISNOTNULL).get()).get();
    const LADate* first_odd_date = (dh=&leg.getData(PRICING_DATA_FIRSTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const LADataDate&>(dh->get()).get() : NULL;
    const LADate* last_odd_date  = (dh=&leg.getData(PRICING_DATA_LASTODDDATE))->isDefined() && ! dh->isNull() ? &dynamic_cast<const LADataDate&>(dh->get()).get() : NULL;
    const LADate& start_date     = dynamic_cast<const LADataDate&>(leg.getData(PRICING_DATA_STARTDATE, ISNOTNULL).get()).get();
    const LADate& end_date       = dynamic_cast<const LADataDate&>(leg.getData(PRICING_DATA_ENDDATE, ISNOTNULL).get()).get();





    DateVector payment_start_dates_unadjust, payment_end_dates_unadjust;
    calcPaymentDates(leg, 
                     payment_dates, payment_dates_unadjust, 
                     payment_start_dates, payment_start_dates_unadjust, 
                     payment_end_dates, payment_end_dates_unadjust, 
                     start_date, end_date, 
                     freq, 
                     first_odd_date, last_odd_date);

	


    if(!is_comp)
	{
		/////////////////////////
		//identify stub payment//
		/////////////////////////

		//isforwardroll
		bool isforwardroll = true;
		dh = &(leg.getData(PRICING_DATA_ISFORWARDROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			isforwardroll = dynamic_cast<const LADataBool&>(dh->get()).get();

		detectStubCoupon(is_first_stub,
						 is_last_stub,
						 freq,
						 start_date,
						 end_date,
						 first_odd_date,
						 last_odd_date,
						 isforwardroll,
						 payment_dates_unadjust);
		return;
	}




    const LAString& roll_freq = dynamic_cast<const LADataString&>(leg.getData(PRICING_DATA_ROLLFREQUENCY, ISNOTNULL).get()).get();
    if(!isConsistentFrequencyPair(roll_freq, freq)){
        LAString msg;
        msg += "Roll frequency is not payment frequency(";
        msg += freq + ", " + roll_freq + ").";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }




    const bool is_forward_roll_orig = (dh=&leg.getData(PRICING_DATA_ISFORWARDROLL))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    leg.remove(PRICING_DATA_ISFORWARDROLL);
    leg.add(PRICING_DATA_ISFORWARDROLL, new LADataBool(is_forward_roll_orig));

    for(size_t i = 0; i < payment_start_dates.size(); i++){
        const bool is_forward_roll = 
            i==0 ? false : 
            (i== payment_start_dates.size()-1 ? true : is_forward_roll_orig);
        dynamic_cast<LADataBool&>(leg.getData(PRICING_DATA_ISFORWARDROLL).get()).set(is_forward_roll);



		LADate temp_start, temp_end;
		if (roll_freq == BUSINESS_DAYS || roll_freq == DAILY)
		{
			temp_start = payment_start_dates[i];
			temp_end   = payment_end_dates[i];
		}
		else{
			temp_start = payment_start_dates_unadjust[i];
			temp_end   = payment_end_dates_unadjust[i];
		}
        const LADate* temp_fodd = NULL;
        const LADate* temp_lodd = NULL;
        DateVector temp_dates, temp_dates_unadjust, 
                   temp_start_dates, temp_end_dates, 
                   temp_start_dates_unadjust, temp_end_dates_unadjust;
        calcPaymentDates(leg, 
                         temp_dates, temp_dates_unadjust,
                         temp_start_dates, temp_start_dates_unadjust, 
                         temp_end_dates, temp_end_dates_unadjust, 
                         temp_start, temp_end, 
                         roll_freq, 
                         temp_fodd, temp_lodd);
		if (!roll_dates.size() == 0 && roll_dates[roll_dates.size() - 1] == temp_dates[0])
			roll_dates.erase(roll_dates.end() - 1);
		if (!roll_dates_unadjust.size() == 0 && roll_dates_unadjust[roll_dates_unadjust.size() - 1] == temp_dates_unadjust[0])
			roll_dates_unadjust.erase(roll_dates_unadjust.end() - 1);
        for(size_t j = 0; j < temp_dates.size(); j++){
            roll_dates.push_back(temp_dates[j]);
            roll_dates_unadjust.push_back(temp_dates_unadjust[j]);
        }
		for (size_t j = 0; j < temp_start_dates.size(); j++) {
			roll_start_dates.push_back(temp_start_dates[j]);
			roll_end_dates.push_back(temp_end_dates[j]);
		}
    }

    leg.getData(PRICING_DATA_ISFORWARDROLL).convertFromString(is_forward_roll_orig ? "TRUE" : "FALSE");

	/////////////////////////
	//identify stub payment//
	/////////////////////////

	detectStubCoupon(is_first_stub,
					 is_last_stub,
					 roll_freq,
					 start_date,
					 end_date,
					 first_odd_date,
					 last_odd_date,
					 is_forward_roll_orig,
					 roll_dates_unadjust);

}


LAObject*
LAPriceCashFlowGenerator::createCouponInfo(const LAString name,
                 const LADate& advancepaymentdate,
                 const LADate& paymentdate,
                 const LADate& paymentdate_unadjust,
                 const LADate& startdate,
                 const LADate& enddate,
                 const LAObject& couponinfo,
                 const LAObject& leg,
                 LAString stubindextype,
				 const double fixedrate,
				 const bool is_digital
				 ) const
{
    const DateVector rolldates;
    const DateVector rollstartdates;
    const DateVector rollenddates;
    return createCouponInfo(name,
        advancepaymentdate,
        paymentdate,
        paymentdate_unadjust,
        startdate,
        enddate,
        couponinfo,
        leg,
        rolldates,
        rollstartdates,
        rollenddates,
        stubindextype,
		fixedrate,
		is_digital);
}


void
LAPriceCashFlowGenerator::detectStubCoupon(bool& is_first_stub,
                                        bool& is_last_stub,
	                                    const LAString& freq,
										const LADate& start_date,
										const LADate& end_date,
										const LADate* first_odd_date,
										const LADate* last_odd_date,
										const bool isforwardroll,
										const DateVector& payment_dates_unadjust) const
{
	DateVector rollDates = payment_dates_unadjust;
	if (start_date < *min_element(payment_dates_unadjust.begin(), payment_dates_unadjust.end()))
	{
		rollDates.insert(rollDates.begin(), start_date);
	}
	if (*max_element(payment_dates_unadjust.begin(), payment_dates_unadjust.end()) < end_date)
	{
		rollDates.insert(rollDates.end(), end_date);
	}

	//convert frequency to term string
	LAString term;
	if (freq == NONE)
	{
		term = NONE;
	}
	else if (freq == BUSINESS_DAYS || freq == DAILY)
	{
		term = "1D";
	}
	else if (freq == WEEKLY)
	{
		term = "1W";
	}
	else
	{
		term = LAString(LAMathDateCalculations::getPeriodFrequencyInMonths(freq)) + "M";
	}

	//detect stub payment date 
	is_first_stub = false;
	if(first_odd_date != 0)
	{
		is_first_stub = true;
	}
	else
	{
		if (!isforwardroll && term != NONE)
		{
			DateVector::const_iterator it = upper_bound(rollDates.cbegin(), rollDates.cend(), start_date);
			if (it == rollDates.cend())
			{
				throw LACoreInvalidData("Faild to find the first payment date", __FILE__, __LINE__);
			}
			LADate nonStubFirstPaymentDate = LAMathDateCalculations::getDate(*it, term, LAPriceDataSlidingRule(), 0, false);
			is_first_stub = (nonStubFirstPaymentDate != start_date);
		}
	}

	is_last_stub = false;
	if(last_odd_date != 0)
	{
		is_last_stub = true;
	}
	else
	{
		if (isforwardroll && term != NONE)
		{
			DateVector::const_iterator it = lower_bound(rollDates.cbegin(), rollDates.cend(), end_date);
			if (it == rollDates.cbegin() || it == rollDates.cend())
			{
				throw LACoreInvalidData("Faild to find the last payment date", __FILE__, __LINE__);
			}
			LADate nonStubLastPaymentDate = LAMathDateCalculations::getDate(*(it - 1), term, LAPriceDataSlidingRule(), 0, true);
			is_last_stub = (nonStubLastPaymentDate != end_date);
		}
	}
}


void
LAPriceCashFlowGenerator::setUpBondFundingLeg(LAObject& trade) const
{
	LADataHolder* dh;

	// ! Get Leg Info
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());

	vector<unsigned int> bond_funding_leg_pos;
	for (unsigned int i = 0; i < legs.getSize(); ++i)
	{
		dh = &legs.get(i).get().getData(PRICING_DATA_ISBONDFUNDINGLEG, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			bond_funding_leg_pos.push_back(i);
		}
	}

	if(bond_funding_leg_pos.size() == 0)
		return;

	if (legs.getSize() != 2 || bond_funding_leg_pos.size() > 1)
		throw LACoreInvalidData("size of leg is inappropreate as structured bond.",__FILE__,__LINE__);


	LAObjectPool& objPool = trade.getDataInstance()->getObjectPool();

	// ! Get PathEntity
	dh = &( trade.getData( PRICING_DATA_PATHENTITY, ISNOTNULL ) );
	LADataReference& arPath = dynamic_cast< LADataReference& >( dh->get() );
	
	LAObject* pathEntity = NULL;

	if ( arPath.get().get().isTypeOf( ENTITY_PATH ) )
	{
		pathEntity = &( dynamic_cast< LAMathPathEntity& >( arPath.get().get() ) );
		
	}
	else if ( arPath.get().get().isTypeOf( ENTITY_PLAINVANILLA ) )
	{
		pathEntity = &( dynamic_cast< LAMathPlainVanillaEntity& >( arPath.get().get() ) );
	}
	else
	{
		LAString msg = "pathEntity type is wrong! LAPriceCashFlowGenerator::createRenotional";
		throw LACoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
	}

	// ! Get asofDate
	dh = &( pathEntity->getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ) );
	LADate asofDate = dynamic_cast< LADataDate& >( dh->get() ).get();


	// Coupon CF FX rate value
	LADate start_date = dynamic_cast<const LADataDate&>(legs.get(bond_funding_leg_pos[0]).getData(PRICING_DATA_STARTDATE, ISNOTNULL).get());
	LAString base_ccy = dynamic_cast<const LADataString&>(legs.get(bond_funding_leg_pos[0]).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get());
	base_ccy.toUpper();
	LAString for_ccy = dynamic_cast<const LADataString&>(legs.get(bond_funding_leg_pos[0]).getData(PRICING_DATA_BONDFUNDINGCURRENCY, ISNOTNULL).get());
	for_ccy.toUpper();

	const LAMathFXEntity* pFwd = nullptr;
	if (pathEntity->isTypeOf(ENTITY_PATH))
	{
		LAMathPathEntity& path = *(dynamic_cast<LAMathPathEntity *>(pathEntity));
		LADataMultiReference& initialrefs = path.getInitialValues();
		const LADataStrings& sdenames = path.getSDEAttrNames();
		unsigned int initSize = sdenames.getSize();
		for (unsigned int i = 0; i < initSize; i++)
		{
			const LAMathAttrSDE &sde = dynamic_cast<const LAMathAttrSDE &>(path.getData(sdenames[i]).get());
			LAString ccy_sde = sde.getCurrency();
			ccy_sde.toUpper();

			if (-1 == ccy_sde.findString('/'))
			{
				continue;
			}

			if (-1 != ccy_sde.findString(for_ccy) || -1 != ccy_sde.findString(base_ccy))
			{
				pFwd = dynamic_cast<LAMathFXEntity *>(&initialrefs.get(i).get());
				break;
			}
		}
	}
	else if(pathEntity->isTypeOf(ENTITY_PLAINVANILLA))
	{
		LAMathPlainVanillaEntity& path = *(dynamic_cast<LAMathPlainVanillaEntity *>(pathEntity));
		pFwd = dynamic_cast<LAMathFXEntity *>(&path.getFXEntity().get().get());
	}
	
	if (pFwd == nullptr && base_ccy != for_ccy)
		throw LACoreInvalidData("Error setUpBondFunding",__FILE__,__LINE__);

		
	double fxRate = base_ccy != for_ccy ? pFwd->getForwardRate(base_ccy, for_ccy, asofDate, max(start_date, asofDate)) : 1.0;
	double notional = dynamic_cast<const LADataDouble&>(legs.get(bond_funding_leg_pos[0]).getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get());
	notional *= fxRate;

	// Update Notional
	LADataMultiReference& cashs = dynamic_cast<LADataMultiReference &>((legs.get(bond_funding_leg_pos[0]).get()).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
	for(unsigned int i = 0; i < cashs.getSize(); ++i)
	{
		LAObject& targetcash = cashs.get(i).get();
		dynamic_cast<LADataString&>(targetcash.getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(for_ccy);
		dynamic_cast<LADataDouble&>(targetcash.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).set(notional);
		dh = &targetcash.getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK);
		if(dh->isDefined() && !dh->isNull())
		{
			dynamic_cast<LADataDouble&>(dh->get()).set(notional);
		}
	}

	dh = &legs.get(bond_funding_leg_pos[0]).getData(PRICING_DATA_ISNOTIONALEXCHANGEATSTART, ISNOTNULL);
	if(dynamic_cast<LADataBool&>(dh->get()).get())
	{
		LAObject& targetcash = cashs.get(0).get();
		dynamic_cast<LADataDouble&>(targetcash.getData(PRICING_CALIBRATION_DATAOTIONALCF, ISNOTNULL).get()).set(-notional);
	}

	dh = &legs.get(bond_funding_leg_pos[0]).getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL);
	if(dynamic_cast<LADataBool&>(dh->get()).get())
	{
		LAObject& targetcash = cashs.get(cashs.getSize() - 1).get();
		dynamic_cast<LADataDouble&>(targetcash.getData(PRICING_CALIBRATION_DATAOTIONALCF, ISNOTNULL).get()).set(notional);
	}

	dh = &(trade.getData(PRICING_DATA_FUNDINGCHANGEINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAObject& fginfo = dynamic_cast<LADataReference &>(dh->get()).get().get();
		dh = &fginfo.getData(PRICING_DATA_BASEFUNDINGNOTIONAL, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			dynamic_cast<LADataDouble&>(dh->get()).set(notional);
		}
	}




	// Bond funding special treatment:
	// Insert notional cf at the end of funding leg when there exists notional cf at the end of strucutured leg.
	vector<bool> is_exsit_final_notionalcf(2, false);
	for (unsigned int i = 0; i < 2; ++i)
	{
		LADataMultiReference& cashs = dynamic_cast<LADataMultiReference &>((legs.get(i).get()).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
		LAObject& cash_last = cashs.get(cashs.getSize() - 1).get();

		dh = &(cash_last.getData(PRICING_DATA_ISREDEMPTION, NOCHECK));
		bool cond1 = (dh->isDefined() && !dh->isNull()) ? dynamic_cast<const LADataBool&>(dh->get()).get() : false;
		dh = &(cash_last.getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK));
		bool cond2 = (dh->isDefined() && !dh->isNull());
		dh = &(cash_last.getData(PRICING_CALIBRATION_DATAOTIONALCFS, NOCHECK));
		bool cond3 = (dh->isDefined() && !dh->isNull());
		dh = &(cash_last.getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, NOCHECK));
		bool cond4 = (dh->isDefined() && !dh->isNull());

		is_exsit_final_notionalcf[i] = (cond1 || cond2 || cond3 || cond4);
	}

	vector<bool> is_need_inserted(2, false);
	if (!is_exsit_final_notionalcf[0] && !is_exsit_final_notionalcf[1])
	{
		for (unsigned int i = 0; i < 2; ++i)
		{
			dh = &legs.get(i).getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL);
			is_need_inserted[i] = dynamic_cast<LADataBool&>(dh->get()).get();
		}
	}
	else
	{
		for (unsigned int i = 0; i < 2; ++i)
		{
			is_need_inserted[i] = !is_exsit_final_notionalcf[i];
		}
	}

	for (unsigned int i = 0; i < 2; ++i)
	{
		if(!is_need_inserted[i])
			continue;

		LADataMultiReference& cashs = dynamic_cast<LADataMultiReference &>((legs.get(i).get()).getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
		LAObject& cash_last = cashs.get(cashs.getSize() - 1).get();

		dh = &(cash_last.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL));
		double notional_mod = dynamic_cast<const LADataDouble&>(dh->get());
		cash_last.remove(PRICING_CALIBRATION_DATAOTIONALCF);
		cash_last.add(PRICING_CALIBRATION_DATAOTIONALCF, new LADataDouble(notional_mod));
	}

}

/*!
@brief	setup for exotic products with non-deliverable currencies: 
		1) change currencies of extra cashflows, settlement amounts of which are already fixed, into the simulated currency
		2) calculate adjust ratios FX(fix)/FX(pay) for cashflows whose settlement amounts are not yet fixed

@param	[in, out] trade trade object

*/
void
LAPriceCashFlowGenerator::setUpNonDeliverable(LAObject& trade) const
{
	LADataHolder* dh = &(trade.getData(PRICING_DATA_PATHENTITY, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		return;
	}
	LADataReference& attrPath = dynamic_cast<LADataReference&>(dh->get());
	LAMathFXEntity* pFX = 0;
	LADate asofdate;
	LAString simccy;
	if (attrPath.get().get().isTypeOf(ENTITY_PLAINVANILLA)) // vanilla
	{
		LAMathPlainVanillaEntity* pPath = &(dynamic_cast< LAMathPlainVanillaEntity&>(attrPath.get().get()));
		if (pPath->getFXEntity().isNull())
		{
			return;
		}
		pFX = dynamic_cast<LAMathFXEntity *>(&pPath->getFXEntity().get().get());
		asofdate = dynamic_cast<LADataDate &>((&(pPath->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)))->get()).get();
	}
	else if (attrPath.get().get().isTypeOf(ENTITY_PATH)) // exo
	{
		LAMathPathEntity* pPath = &dynamic_cast<LAMathPathEntity&>(attrPath.get().get());
		const LAStringVector sdenames = pPath->getSDEAttrNames().get();
		const LAStringVector simsdenames = pPath->getSimulationSDEAttrNames().get();
		if (simsdenames.size() != 1)
		{
			return;
		}
		else if (sdenames.size() == 1)
		{
			return;
		}

		dh = &pPath->getData(simsdenames[0], ISNOTNULL);
		simccy = dynamic_cast<LAMathAttrSDE&>(dh->get()).getCurrency();

		const LADataMultiReference& initialrefs = pPath->getInitialValues();
		for (unsigned int i = 0; i < sdenames.size(); ++i)
		{
			const LAMathAttrSDE& sde = dynamic_cast<const LAMathAttrSDE&>(pPath->getData(sdenames[i], ISNOTNULL).get());
			if (sde.getSDEPathType() == FX)
			{
				pFX = dynamic_cast<LAMathFXEntity *>(&initialrefs.get(i).get());
			}
		}
		asofdate = dynamic_cast<LADataDate &>((&(pPath->getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)))->get()).get();
	}
	else
	{
		throw LACoreInvalidData("Invalid path type.", __FILE__, __LINE__);
	}

	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); ++i)
	{
		dh = &(legs.get(i).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int j = 0; j < cashlets.getSize(); ++j)
		{
			LAObject& cashlet = cashlets.get(j).get();

			// only for exo
			if (attrPath.get().get().isTypeOf(ENTITY_PATH))
			{
				// change currencies of extra cashflows, settlement amounts of which are already fixed, into the simulated currency
				dh = &cashlet.getData(PRICING_DATA_EXTRACF, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const double original_extracf = dynamic_cast<LADataDouble &>(dh->get()).get();
					dh = &cashlet.getData(PRICING_DATA_EXTRACFCURRENCY, NOCHECK);
					if (!dh->isDefined() || dh->isNull())
						dh = &cashlet.getData(PRICING_DATA_CURRENCY, ISNOTNULL);
					const LAString original_ccy = dh->convertToString().exchange("\"", "");

					if (original_ccy == simccy)
						continue;

					dh = &(cashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
					const LADate paymentdate = dynamic_cast<LADataDate &>(dh->get()).get();

					const double fwdfx = pFX->getRate(original_ccy, simccy, paymentdate);
					const double modified_extracf = original_extracf * fwdfx;

					dh = &cashlet.getData(PRICING_DATA_EXTRACF, NOCHECK);
					dynamic_cast<LADataDouble &>(dh->get()).set(modified_extracf);

					dh = &cashlet.getData(PRICING_DATA_EXTRACFCURRENCY, ISNOTNULL);
					dh->get().convertFromString(simccy);
				}
			}

			// calculate adjust ratios FX(fix)/FX(pay) for cashflows whose settlement amounts are not yet fixed 
			dh = &cashlet.getData(PRICING_DATA_SETTLEMENTCURRENCY, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				const LAString settle_ccy = dynamic_cast<LADataString &>(dh->get()).get();
				const LAString original_ccy = dynamic_cast<LADataString &>((&cashlet.getData(PRICING_DATA_CURRENCY, ISNOTNULL))->get()).get();

				const LADate paydate = dynamic_cast<LADataDate &>((&(cashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL)))->get()).get();
				const double fwdfx_pay = pFX->getRate(original_ccy, settle_ccy, paydate < asofdate ? asofdate : paydate);

				dh = &cashlet.getData(PRICING_DATA_SETTLEMENTFIXINGDATE, NOCHECK);
				const LADate fixdate = (dh->isDefined() && !dh->isNull()) ? 
					dynamic_cast<LADataDate &>(dh->get()).get() : 
					LAMathDateCalculations::getDate(paydate, "-5D", SLIDING_RULE_FOLLOWING, &LAPriceDataCalendar(pFX->getCalendarNames().get()), true);
				const LADate spotdate = pFX->getSpotDate(original_ccy, settle_ccy, fixdate);
				const double fwdfx_fix = pFX->getRate(original_ccy, settle_ccy, spotdate < asofdate ? asofdate : spotdate);

				const double adjust_ratio = fwdfx_fix / fwdfx_pay;
				cashlet.remove(PRICING_DATA_SETTLEMENTADJUSTRATIO);
				cashlet.add(PRICING_DATA_SETTLEMENTADJUSTRATIO, new LADataDouble(adjust_ratio));
			}
		}

		dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			LAObjectHolder& callinfo = dynamic_cast<LADataReference&>(dh->get()).get();
			dh = &(callinfo.getData(PRICING_DATA_SETTLEMENTCURRENCY, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const LAString settle_ccy = dynamic_cast<LADataString &>((&callinfo.getData(PRICING_DATA_SETTLEMENTCURRENCY, ISNOTNULL))->get()).get();
				const LAString original_ccy = dynamic_cast<LADataString &>((&callinfo.getData(PRICING_DATA_EXTRACFCURRENCY, ISNOTNULL))->get()).get();

				const DateVector fixdates = dynamic_cast<LADataDates&>((&(callinfo.getData(PRICING_DATA_SETTLEMENTFIXINGDATES, ISNOTNULL)))->get()).get();
				const DateVector paydates = dynamic_cast<LADataDates&>((&(callinfo.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL)))->get()).get();

				DoubleVector adjust_ratios;
				for (unsigned int k = 0; k < fixdates.size(); ++k)
				{
					const LADate spotdate = pFX->getSpotDate(original_ccy, settle_ccy, fixdates[k]);
					const double fwdfx_pay = pFX->getRate(original_ccy, settle_ccy, paydates[k] < asofdate ? asofdate : paydates[k]);
					const double fwdfx_fix = pFX->getRate(original_ccy, settle_ccy, spotdate < asofdate ? asofdate : spotdate);
					adjust_ratios.push_back(fwdfx_fix / fwdfx_pay);
				}
				callinfo.remove(PRICING_DATA_SETTLEMENTADJUSTRATIOS);
				callinfo.add(PRICING_DATA_SETTLEMENTADJUSTRATIOS, new LADataDoubles(adjust_ratios));
			}
		}
	}
}
