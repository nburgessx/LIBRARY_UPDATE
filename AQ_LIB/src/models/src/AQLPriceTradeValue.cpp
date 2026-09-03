/*! @file
    @brief Source code for class to evaluate trade.

*/
//  2007, AlgoQuantHub..
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

const unsigned int MAX_IDX_SIZE = 250; //50Y x Quarterly + buffa

#include <functional>
#include <algorithm>
#include <numeric>
#include "AQLPriceTradeValue.h"
#include "AQLPricePortfolioValue.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLDate.h"
#include "AQLDataValuation.h"
#include "AQLDataProcedure.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectHolder.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLMathValuableEntity.h"
#include "AQLMathPathEntity.h"
#include "AQLMathAttrSDE.h"
#include "AQLMathIndexEntity.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathFXEntity.h"

#include "AQLRatesSDEBase.h"
#include "AQLPolynomialBase.h"
#include "AQLAlgorithm.h"

#include "AQLRatesNumeraireBankAccountHW.h"

#include "AQLPricePayOff.h"
#include "AQLPricePayOffTool.h"
#include "AQLPriceAccruedInterest.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLInterpolationBase.h"
#include "AQLPriceDataInterpolation.h"


using namespace std;

#define RCV	"RCV"
#define PAY	"PAY"
#define STARTABLE	"STARTABLE"
#define CANCELABLE	"CANCELABLE"
#define CPN "CPN"
#define CPNCF "CPNCF"
#ifndef BASIS
#define BASIS	"BASISRATE"
#endif

/*!
    @brief constructor
	@param[in] pacc pointer to accured interest calculation class
*/
AQLPriceTradeValue::AQLPriceTradeValue(AQLPriceAccruedInterest* pacc) :
AQLCoreValuation(), mpAcc(pacc)
{
}
/*!
    @brief copy constructor
	@param[in] v copy source 
*/
AQLPriceTradeValue::AQLPriceTradeValue(const AQLPriceTradeValue& v)
: AQLCoreValuation(v), mpAcc(0)
{
	if (v.mpAcc != 0)
		mpAcc = dynamic_cast<AQLPriceAccruedInterest*>(v.mpAcc->clone());
}
/*!
    @brief destructor

*/
AQLPriceTradeValue::~AQLPriceTradeValue()
{
	delete mpAcc;
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
AQLPriceTradeValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRADEVALUE ? true : AQLCoreValuation::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceTradeValue::clone() const
{
    try 
	{
  		return new AQLPriceTradeValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Return this class type

	@return this class type
*/
function_t			
AQLPriceTradeValue::getType() const
{
	return FN_IR_TRADEVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceTradeValue::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_ISCALCRISK, DATA_BOOL);
	dm.setData(PRICING_DATA_TODAY, DATA_DATE);
	dm.setData(PRICING_DATA_SETTLEDATE, DATA_DATE);
	dm.setData(PRICING_DATA_SPOTLAG, DATA_INT);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_ISDETAILOUTPUT, DATA_BOOL);
	dm.setData(PRICING_DATA_CALLINFO, DATA_REFERENCE);
	dm.setData(PRICING_DATA_TRIGGERINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_PATHENTITY, DATA_REFERENCE);
	dm.setData(PRICING_DATA_CFGENERATOR, DATA_PROCEDURE);
	dm.setData(PRICING_DATA_MCNUM, DATA_INT);
	dm.setData(PRICING_DATA_CLEANPRICE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_DIRTYPRICE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_ACCRUEDINTEREST, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CLEANPRICESQUARE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CLEANPRICEDEVIATION, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CALLTRIGGERVALUE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER, DATA_DOUBLE);
	dm.setData(PRICING_DATA_ISSPOTACCRUEDINTEREST, DATA_BOOL);
	dm.setData(PRICING_DATA_ACTIONPROBABILITIES, DATA_DOUBLES);
	dm.setData(PRICING_DATA_AVERAGELIFE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CURRENCY, DATA_STRING);
	dm.setData(CALIBRATION_DATA_UNDERLYINGS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_SELECTSIDE, DATA_STRING);
	dm.setData(PRICING_DATA_SELECTCALLTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_LSMCNUM, DATA_INT);
	dm.setData(PRICING_DATA_POLYNOMIAL, DATA_FUNCTION);
	dm.setData(PRICING_DATA_PRODUCT, DATA_STRING);
	dm.setData(PRICING_DATA_ISOPTIONHOLDER, DATA_BOOL);
	dm.setData(PRICING_DATA_ISLSMCDETAILOUTPUT, DATA_BOOL);
	dm.setData(PRICING_DATA_LSMCREGCOEFFICIENTS, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCREGCOEFFICIENTS2, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCEXPLANATORYVARIABLES, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCEXPLAINEDVARIABLES, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCCALLPOSTJUDGE, DATA_BOOLS);
	dm.setData(PRICING_DATA_LSMCCALLJUDGE, DATA_BOOLS);
	dm.setData(PRICING_DATA_COEFFICIENTS2, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_CALLREBATE, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_ISPATHDETAILOUTPUT, DATA_BOOL);
    dm.setData(PRICING_DATA_ISEXCLUDEOUTLIER_Y, DATA_BOOL);
    dm.setData(PRICING_DATA_ISEXCLUDEOUTLIER_X, DATA_BOOL);
    dm.setData(PRICING_DATA_ISEXCLUDEOUTLIER_E, DATA_BOOL);
    dm.setData(PRICING_DATA_CRITERIA_Y, DATA_DOUBLE);
    dm.setData(PRICING_DATA_CRITERIA_X, DATA_DOUBLE);
    dm.setData(PRICING_DATA_CRITERIA_E, DATA_DOUBLE);
    dm.setData(PRICING_DATA_SVDTOLERANCE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_ISSHIFT_Y, DATA_BOOL);
	dm.setData(PRICING_DATA_ISSHIFT_X, DATA_BOOL);
	dm.setData(PRICING_DATA_ISSCALE_Y, DATA_BOOL);
	dm.setData(PRICING_DATA_ISSCALE_X, DATA_BOOL);
	dm.setData(PRICING_DATA_ISCONVERT_XY, DATA_BOOLS);
	dm.setData(PRICING_DATA_SHIFT_Y, DATA_DOUBLES);
	dm.setData(PRICING_DATA_SHIFT_X, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_SCALE_Y, DATA_DOUBLES);
	dm.setData(PRICING_DATA_SCALE_X, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_ISCONVERT_XY2, DATA_BOOLS);
	dm.setData(PRICING_DATA_SHIFT_Y2, DATA_DOUBLES);
	dm.setData(PRICING_DATA_SHIFT_X2, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_SCALE_Y2, DATA_DOUBLES);
	dm.setData(PRICING_DATA_SCALE_X2, DATA_DOUBLE_MATRIX);
    dm.setData(PRICING_DATA_VALUEDATE, DATA_DATE);
	dm.setData(PRICING_DATA_DISCOUNTCURVE, DATA_STRING);
	dm.setData(PRICING_DATA_ISLEGBASECURRENCY, DATA_BOOL);
	dm.setData(PRICING_DATA_ISSETUPPAYOFF, DATA_BOOL);
	dm.setData(PRICING_DATA_ISRECALCTRADEDATA, DATA_BOOL);
	dm.setData(PRICING_DATA_ISRESULTOUTPUT, DATA_BOOL );
	dm.setData(PRICING_DATA_PV_LEG1, DATA_DOUBLE);
	dm.setData(PRICING_DATA_PV_LEG2, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CURRENCY_LEG1, DATA_STRING);
	dm.setData(PRICING_DATA_CURRENCY_LEG2, DATA_STRING);
	dm.setData(PRICING_DATA_ODDINDEXTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_ISODDTRADE, DATA_BOOL);
	dm.setData(PRICING_DATA_ISCALCVANILLA, DATA_BOOL);
	dm.setData(PRICING_DATA_YIELDPRONAME, DATA_STRING);
	dm.setData(PRICING_DATA_ISADDFWDPREMPV, DATA_BOOL);
	dm.setData(CALIBRATION_DATA_FEEAMOUNTS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_FEEPAYMENTDATES, DATA_DATES);
	dm.setData(CALIBRATION_DATA_FEECURRENCIES, DATA_STRINGS);
	dm.setData(CALIBRATION_DATA_FEEDISCOUNTCURVES, DATA_STRINGS);

	AQLPricePayOff payoff;
	payoff.registerData(dm);

}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
#ifndef VISUAL_STUDIO_2010_ANALYTICS
double
AQLPriceTradeValue::value(const AQLDate& basedate, AQLObject& object,
					const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	//original currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	AQLString currency_ori = dynamic_cast<AQLDataString &>(dh->get()).get();

	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	if (!iscalcrisk) att.setDataProvider(NULL);
	
	bool istraderecalc = false;
	dh = &object.getData(PRICING_DATA_ISRECALCTRADEDATA, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		istraderecalc = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	
	bool isfundingchg = false;
	dh = &(object.getData(PRICING_DATA_FUNDINGCHANGEINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		isfundingchg = true;

	bool issetuppayoff = false;
	dh = &(object.getData(PRICING_DATA_ISSETUPPAYOFF, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		issetuppayoff = dynamic_cast<const AQLDataBool&>(dh->get()).get();

	if (!iscalcrisk || istraderecalc || isfundingchg || issetuppayoff) att.setDataProvider(NULL);
	
	AQLPriceTradeValueDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		dataProvider = dynamic_cast<AQLPriceTradeValueDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dataProvider = &dynamic_cast<AQLPriceTradeValueDataProvider&>(att.getDataProvider());

	dataProvider->pPath->setUpforMC();
	set<AQLMathIndexEntity*>::iterator it;
	for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
		(*it)->setUpforMC();

	double ret = value(basedate, object, dataProvider, 0);
	//return original currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dynamic_cast<AQLDataString&>(dh->get()).set(currency_ori);

	return ret;
	
}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] dataProvider cashe class
	@param[in] startpathnum start path number

	@return clean price
*/
double
AQLPriceTradeValue::value(const AQLDate& basedate, 
						AQLObject& object, 
						AQLDataProvider* dp,
						unsigned int startpathnum) const
{
	(void)basedate;
	AQLDataHolder* dh;
	AQLPriceTradeValueDataProvider* dataProvider = dynamic_cast<AQLPriceTradeValueDataProvider*>(dp);

	// base currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	AQLString currency_ori = dynamic_cast<AQLDataString &>(dh->get()).get();
	dynamic_cast<AQLDataString&>(dh->get()).set(dataProvider->basecur);
	
	//check the order of simcurrency
	const AQLDataStrings& simsdenames = (!dataProvider->pPath->getSimulationSDEAttrNames().isNull()) ? dataProvider->pPath->getSimulationSDEAttrNames() : dataProvider->pPath->getSDEAttrNames(); //Condition branch: SimulationSDECcy is not created when setting up HW with Excel pricer
	unsigned int simsdeSize = simsdenames.getSize();
	if (simsdeSize > 1)
	{
		AQLDataStrings simccys_IR, simccys_FX;
		for (unsigned int i = 0; i < simsdeSize; i++)
		{
			const AQLMathAttrSDE& sde = dynamic_cast<const AQLMathAttrSDE& >(dataProvider->pPath->getData(simsdenames[i], ISNOTNULL).get());
			if (sde.getSDEPathType() == IR)
				simccys_IR.push_back(sde.getCurrency());
			else if (sde.getSDEPathType() == FX)
				simccys_FX.push_back(sde.getCurrency());
		}
		if (simccys_IR.getSize() == simccys_FX.getSize() + 1)
		{
			for (unsigned int i =0; i < simccys_FX.getSize(); i++)
			{
				if (simccys_FX[i].findString(simccys_IR[0]) < 0 || simccys_FX[i].findString(simccys_IR[i+1]) < 0)
					throw AQLCoreInvalidData("the order of SimulationSDEcurrency is wrong",__FILE__,__LINE__);
			}
		}
		else
			throw AQLCoreInvalidData("the number of SimulationSDEcurrency is wrong",__FILE__,__LINE__);
	}

	// calcuate risk mode or not
	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const AQLDataBool&>(dh->get()).get();

	set<AQLMathIndexEntity*>::iterator it;

	//get sumulation curve name
	const AQLString& originalNumeraire = dataProvider->pNumeraire->getBasisName();
	//set basis spread of numeraire
	if (!dataProvider->basisgrid_payoff.empty())
		dataProvider->pNumeraire->setBasisSpread(dataProvider->basisname_payoff, dataProvider->basisgrid_payoff, dataProvider->basisspread_payoff);

	if (iscalcrisk)
	{
		double pv1 = 0.0, pv2 = 0.0;
		double pv1_tmp = 0.0, pv2_tmp = 0.0;
		double accruedint = 0.0;
		DoubleMatrix time1, cf1, time2, cf2;
		vector<pair<unsigned int, AQLDate> > triggerhit;
		DoubleArray rebate(dataProvider->expirytimes.size());
		DoubleMatrix explanatory(dataProvider->expirytimes.size());

		//mc
		for (int i = startpathnum; i < dataProvider->mcnum; i++)
		{
			dataProvider->pNumeraire->setBasisName(originalNumeraire);

			pv1_tmp = 0.0, pv2_tmp = 0.0;
			dataProvider->pPath->setNextPath();
			
			for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
				(*it)->setNextIndex();
			
			if (!dataProvider->basisgrid_payoff.empty())
				dataProvider->pNumeraire->setBasisName(dataProvider->basisname_payoff);

			if (dataProvider->acc.isDefined() && !dataProvider->isstartable) //calculate accurued interest
			{
				const AQLPriceAccruedInterest& acc = dynamic_cast<const AQLPriceAccruedInterest&>(dataProvider->acc.get());
				const vector<PayOffToolHolderVector>& payoff_vec = dataProvider->payoff.getPayOff();
				double adjust;
				if (dataProvider->settle == dataProvider->baseterm)
					adjust = 1.0;
				else
					adjust = (*dataProvider->pNumeraire)(dataProvider->baseterm) / (*dataProvider->pNumeraire)(dataProvider->settle);
				
				
				//back to previous version for accrue interest
				//for (unsigned int j = 0; j < payoff_vec.size(); j++)
				//{
				//	if (dataProvider->rcvpay[j]) accruedint += adjust * acc.calcAccruedInterest(dataProvider->settledate, j);
				//	else accruedint -= adjust * acc.calcAccruedInterest(dataProvider->settledate, j);
				//}
				
				if (dataProvider->rcvpay[0]) accruedint += adjust * acc.calcAccruedInterest(dataProvider->settledate, 0);
					else accruedint -= adjust * acc.calcAccruedInterest(dataProvider->settledate, 0);

			}			
			if (!dataProvider->iscall || dataProvider->actiondates.size() == 0)
			{
				dataProvider->payoff.calcPayOff(time1, cf1, true, triggerhit, true);
				for (unsigned int j = 0; j < time1.size(); j++)
					for (unsigned int k = 0; k < time1[j].size(); k++)
						if (time1[j][k] > dataProvider->settle)
						{
							if (dataProvider->rcvpay[j])	pv1_tmp += cf1[j][k] / (*dataProvider->pNumeraire)(time1[j][k]);
							else pv1_tmp -= cf1[j][k] / (*dataProvider->pNumeraire)(time1[j][k]);					
						}
			}
			else
			{
				if(dataProvider->iscalcswap) dataProvider->payoff.calcPayOff(time1, cf1, false, triggerhit, false);
				dataProvider->payoff.calcPayOff(time1, cf1, true, triggerhit, /*false, */true, &rebate, &explanatory);
				DoubleVector callval;
				pv1_tmp = value_backward(time1, cf1, triggerhit, rebate, explanatory, dataProvider, callval);
			}
			pv1_tmp *= (*dataProvider->pNumeraire)(dataProvider->baseterm);
			pv1 += pv1_tmp;

			if (!dataProvider->isstartable) continue; 
			
			dataProvider->payoff.calcPayOff(time2, cf2, true, triggerhit, false);
			for (unsigned int j = 0; j < time2.size(); j++)
				for (unsigned int k = 0; k < time2[j].size(); k++)
					if (time2[j][k] > dataProvider->settle)
					{
						if (dataProvider->rcvpay[j])	pv2_tmp += cf2[j][k] / (*dataProvider->pNumeraire)(time2[j][k]);
						else pv2_tmp -= cf2[j][k] / (*dataProvider->pNumeraire)(time2[j][k]);
					}			
			
			pv2_tmp *= (*dataProvider->pNumeraire)(dataProvider->baseterm);
			pv2 += pv2_tmp;		
		}
		double pv = 0.0;
		if (!dataProvider->isstartable)
			pv = pv1 / double(dataProvider->mcnum - startpathnum);
		else 
			pv = (-pv1 + pv2) / double(dataProvider->mcnum - startpathnum);

		// valuedate calc
		double ratio = calcValueTermRatio(*dataProvider);
		pv *= ratio;
		if (!dataProvider->isoptionholder) pv *= -1.0;

		if (dataProvider->basecur != dataProvider->numerairecur)
			pv *= getFXEntity(object).getRate(dataProvider->numerairecur, dataProvider->basecur, dataProvider->baseterm);

		dynamic_cast<AQLDataString&>(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(currency_ori);
		// Calculate Fee Value
		double pvFee = calcFeeValueExo(object, dataProvider);
		return pv + pvFee;
	}

	// call
	AQLObject* pcallinfo = NULL;
	if (dataProvider->iscall)
	{
		dh = &(object.getData(PRICING_DATA_CALLINFO, ISNOTNULL));
		AQLDataReference& attr = dynamic_cast<AQLDataReference&>(dh->get());		
		pcallinfo = &attr.get().get();
	}


	// trigger
	bool istrigger = false;
	vector<AQLObject*> triggerinfos;
	dh = &(object.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		istrigger = true;
		AQLDataMultiReference& attr = dynamic_cast<AQLDataMultiReference&>(dh->get());		
		triggerinfos.resize(attr.getSize());
		for (unsigned int i = 0; i < attr.getSize(); i++)
			triggerinfos[i] = &attr.get(i).get();
	}

	// isdetail output
	bool isdetailoutput = false;
	if (istrigger || dataProvider->iscall)
	{
		dh = &(object.getData(PRICING_DATA_ISDETAILOUTPUT, ISNOTNULL));
		isdetailoutput = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	}
	// islsmcdetail output
	bool islsmcdetailoutput = false;
	if (dataProvider->iscall && dataProvider->actiondates.size() != 0)
	{
		dh = &(object.getData(PRICING_DATA_ISLSMCDETAILOUTPUT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			islsmcdetailoutput = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	}
	// isdistribution output
	bool ispathdetailoutput = false;
	dh = &(object.getData(PRICING_DATA_ISPATHDETAILOUTPUT, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		ispathdetailoutput = dynamic_cast<const AQLDataBool&>(dh->get()).get();

	BoolVector postjudge, judge, postjudge_, judge_;
	DoubleMatrix explained_, explanatory_, rebate_;
	DoubleArray explained;
	unsigned int num_explanatory = 0;

	double pv1_tmp = 0.0, pv2_tmp = 0.0, pv3_tmp = 0.0;
	double pv1 = 0.0, pv2 = 0.0, pv3 = 0.0;
	double accruedint = 0.0;
	double accruedint2 = 0.0;
	double pv1_square = 0.0;
	DoubleMatrix time1, time2, time3, cf1, cf2, cf3;
	vector<pair<unsigned int, AQLDate> > triggerhit;
	vector<map<AQLDate, unsigned int> > counter_action;
	DoubleArray rebate(dataProvider->expirytimes.size());
	DoubleMatrix explanatory(dataProvider->expirytimes.size());
	vector<UintArray> extracfpos;
	
	if (istrigger || dataProvider->iscall)
	{
		unsigned int size = triggerinfos.size();
		size++;
		counter_action.resize(size);
		if (dataProvider->iscall)
		{
			dh = &(pcallinfo->getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
			const DateVector& actiondates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			for (unsigned int i = 0; i < actiondates.size(); i++)
				counter_action[0][actiondates[i]] = 0;
		}
		
		for (unsigned int i = 0; i < triggerinfos.size(); i++)
		{
			dh = &(triggerinfos[i]->getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
			const DateVector& actiondates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
			for (unsigned int j = 0; j < actiondates.size(); j++)
				counter_action[1 + i][actiondates[j]] = 0;
		}
	}

	// param for each cashlet
	unsigned int legnum = dataProvider->paytimes.size();
	DoubleVector tmpval(dataProvider->maxcfnum, 0.0);
	DoubleMatrix avecf1(legnum, tmpval), avepv1(legnum, tmpval); // cf and pv for each cashlet
	DoubleMatrix avecf2(legnum, tmpval), avepv2(legnum, tmpval); // underlying cf and pv for each cashlet
	vector<map<double, double> > extracf_cfmap(legnum, map<double, double>()), extracf_pvmap(legnum, map<double, double>()); // extracf val
	// param for call value
	const unsigned int expiryNum = dataProvider->expirytimes.size();
	DoubleVector avecall(expiryNum, 0.0);
	// param for exposure
	DoubleVector expTimes = dataProvider->iscall && dataProvider->actiondates.size() != 0 ? dataProvider->expirytimes : dataProvider->paytimes[0];
	const unsigned int expNum = expTimes.size();
	DoubleVector aveExpNet(expNum, 0.0), aveExpPos(expNum, 0.0), aveExpNeg(expNum, 0.0);

	// ! Get data for resultOut
	bool isResultOut = false;
	dh = &( object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK ) );
	if ( dh->isDefined() && !dh->isNull() ) 
		isResultOut = dynamic_cast< AQLDataBool& >( dh->get() ).get();
	
	vector< DateVector > paymentDates;
	vector< DateVector > cfCalcStartDates;
	vector< DateVector > cfCalcEndDates;

	if ( isResultOut )
	{
		const vector< PayOffToolHolderVector >& payOff = dataProvider->payoff.getPayOff();
		paymentDates.resize( payOff.size() );
		cfCalcStartDates.resize( payOff.size() );
		cfCalcEndDates.resize( payOff.size() );
		for ( size_t k = 0; k < payOff.size(); k++ )
		{
			paymentDates[k].resize( payOff[k].size() );
			cfCalcStartDates[k].resize( payOff[k].size() );
			cfCalcEndDates[k].resize( payOff[k].size() );

			for ( size_t j = 0; j < payOff[k].size(); j++ )
			{
				paymentDates[k][j] = payOff[k][j].getPayOff().mPaymentDate;  
				cfCalcStartDates[k][j] = payOff[k][j].getPayOff().mStart;
				cfCalcEndDates[k][j] = payOff[k][j].getPayOff().mEnd;
			}
		}
	}

	std::map<AQLString, AQLStringVector> idxPathMap;
	// mc 
	for (int i = startpathnum; i < dataProvider->mcnum; i++)
	{
		dataProvider->pNumeraire->setBasisName(originalNumeraire);

		dataProvider->pPath->setNextPath();
		for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
		{
			(*it)->setNextIndex();
			if (ispathdetailoutput)
			{
				const AQLDataDoubles index((*it)->getIndex());
				if (index.getSize() < MAX_IDX_SIZE)
				{
					const AQLString idxname = (*it)->getCurrency().get()+(*it)->getIndexType().get()+(*it)->getAccessory().get();
					idxPathMap[idxname].push_back(index.convertToString());
				}
			}
		}
		if (!dataProvider->basisgrid_payoff.empty())
			dataProvider->pNumeraire->setBasisName(dataProvider->basisname_payoff);

		double accruedint_tmp = 0.0;
		double accruedint2_tmp = 0.0;
		if (dataProvider->acc.isDefined() && !dataProvider->isstartable) //calculate accurued interest
		{
			
			const AQLPriceAccruedInterest& acc = dynamic_cast<const AQLPriceAccruedInterest&>(dataProvider->acc.get());
			const vector<PayOffToolHolderVector>& payoff_vec = dataProvider->payoff.getPayOff();

			
			//back to previous version for accrue interest
			//for (unsigned int j = 0; j < payoff_vec.size(); j++)
			//{
			//	if (dataProvider->rcvpay[j]) accruedint_tmp += acc.calcAccruedInterest(dataProvider->settledate, j);
			//	else accruedint_tmp -= acc.calcAccruedInterest(dataProvider->settledate, j);
			//}
			
			
			if (dataProvider->rcvpay[0]) accruedint_tmp += acc.calcAccruedInterest(dataProvider->settledate, 0);
				else accruedint_tmp -= acc.calcAccruedInterest(dataProvider->settledate, 0);

			accruedint += accruedint_tmp;
		}
		// base date numeraire
		double numeraire_b = (*dataProvider->pNumeraire)(dataProvider->baseterm);
		// calc underlying value
		dataProvider->payoff.calcPayOff(time2, cf2, false, triggerhit, false);
		pv2_tmp = 0.0;
		for (unsigned int j = 0; j < time2.size(); j++)
		{
			for (unsigned int k = 0; k < time2[j].size(); k++)
			{
				if (time2[j][k] > dataProvider->settle)
				{
					double numeraire = (*dataProvider->pNumeraire)(time2[j][k]);
					if (dataProvider->rcvpay[j])	
					{
						pv2_tmp += cf2[j][k] / numeraire;
						avecf2[j][k] += cf2[j][k];
						avepv2[j][k] += (numeraire_b * cf2[j][k]) / numeraire;
					}
					else 
					{
						pv2_tmp -= cf2[j][k] / numeraire;
						avecf2[j][k] -= cf2[j][k];
						avepv2[j][k] -= (numeraire_b * cf2[j][k]) / numeraire;
					}
				}
			}
		}
		pv2_tmp *= numeraire_b;
		pv2 += pv2_tmp;
		if (!dataProvider->iscall || dataProvider->actiondates.size() == 0)
		{
			dataProvider->payoff.calcPayOff(time1, cf1, true, triggerhit, /*true, */false, 0, 0, &extracfpos);
			pv1_tmp = 0.0;
			DoubleArray payoffPv(expNum, 0.0);
			for (unsigned int j = 0; j < time1.size(); j++)
			{
				double sign = 1.0;
				if (!dataProvider->rcvpay[j])
				{
					sign = -1.0;
				}
				if (extracfpos[j].empty())
				{
					for (unsigned int k = 0; k < time1[j].size(); k++)
					{
						if (time1[j][k] > dataProvider->settle)
						{
							double numeraire = (*dataProvider->pNumeraire)(time1[j][k]);
							pv1_tmp +=  sign * cf1[j][k] / numeraire;
							avecf1[j][k] += sign * cf1[j][k];
							avepv1[j][k] += (sign * numeraire_b * cf1[j][k]) / numeraire;
							unsigned int pos = 0;
							AQLAlgorithm::locate(expTimes, time1[j][k], expNum, pos);
							if ( pos < expNum) payoffPv[pos] += (sign * numeraire_b * cf1[j][k]) / numeraire;
						}
					}
				}
				else
				{
					unsigned int cf_index = 0;
					for (unsigned int k = 0; k < time1[j].size(); k++)
					{
						if (time1[j][k] > dataProvider->settle)
						{
							double numeraire = (*dataProvider->pNumeraire)(time1[j][k]);
							pv1_tmp += sign * cf1[j][k] / numeraire;
							unsigned int expos;
							if (!AQLAlgorithm::find<UintArray, unsigned int>(extracfpos[j], k, 0, extracfpos[j].size() - 1, expos))
							{
								avecf1[j][cf_index] += sign * cf1[j][k];
								avepv1[j][cf_index] += (sign * numeraire_b * cf1[j][k]) / numeraire;
								++cf_index;
							}
							else
							{
								double paytime = time1[j][k];
								extracf_cfmap[j][paytime] += sign * cf1[j][k];
								extracf_pvmap[j][paytime] += (sign * numeraire_b * cf1[j][k]) / numeraire;
							}
							unsigned int pos = 0;
							AQLAlgorithm::locate(expTimes, time1[j][k], expNum, pos);
							if ( pos < expNum) payoffPv[pos] += (sign * numeraire_b * cf1[j][k]) / numeraire;
						}
					}
				}
			}
			double remainPv = 0.0;
			for (int j = expNum - 1; j >= 0; j--)
			{
				aveExpNet[j] += remainPv;
				if (aveExpNet[j] > 0) aveExpPos[j] += remainPv;
				else				  aveExpNeg[j] += remainPv;
				remainPv += payoffPv[j];
			}
		}
		else
		{	
			dataProvider->payoff.calcPayOff(time1, cf1, true, triggerhit, /*false, */true, &rebate, &explanatory, &extracfpos);
			for (unsigned int j = 0; j < time1.size(); j++)
			{
				double sign = 1.0;
				if (!dataProvider->rcvpay[j])
				{
					sign = -1.0;
				}
				if (extracfpos[j].empty())
				{
					for (unsigned int k = 0; k < time1[j].size(); k++)
					{
						if (time1[j][k] > dataProvider->settle)
						{
							double numeraire = (*dataProvider->pNumeraire)(time1[j][k]);
							avecf1[j][k] += sign * cf1[j][k];
							avepv1[j][k] += (sign * numeraire_b * cf1[j][k]) / numeraire;
						}
					}
				}
				else
				{
					unsigned int cf_index = 0;
					for (unsigned int k = 0; k < time1[j].size(); k++)
					{
						if (time1[j][k] > dataProvider->settle)
						{
							double numeraire = (*dataProvider->pNumeraire)(time1[j][k]);
							unsigned int expos;
							if (!AQLAlgorithm::find<UintArray, unsigned int>(extracfpos[j], k, 0, extracfpos[j].size() - 1, expos))
							{
								avecf1[j][cf_index] += sign * cf1[j][k];
								avepv1[j][cf_index] += (sign * numeraire_b * cf1[j][k]) / numeraire;
								++cf_index;
							}
							else
							{
								double paytime = time1[j][k];
								extracf_cfmap[j][paytime] += sign * cf1[j][k];
								extracf_pvmap[j][paytime] += (sign * numeraire_b * cf1[j][k]) / numeraire;
							}
						}
					}
				}

			}
			
			DoubleVector callval;
			if (islsmcdetailoutput)
			{
				if (i == static_cast<int>(startpathnum))
				{
					unsigned int size = dataProvider->mcnum - startpathnum;
					postjudge_.resize(dataProvider->actiontimes.size() * size, false);
					judge_.resize(dataProvider->actiontimes.size() * size, false);
					explained_.resize(dataProvider->actiontimes.size(), DoubleArray(size));
					postjudge.resize(dataProvider->actiontimes.size(), false);
					judge.resize(dataProvider->actiontimes.size(), false);
					explained.resize(dataProvider->actiontimes.size());
					rebate_.resize(dataProvider->actiontimes.size(), DoubleArray(size));
				}

				
				pv1_tmp = value_backward(time1, cf1, triggerhit, rebate, explanatory, dataProvider, callval, &postjudge, &judge, &explained);
				
				unsigned int size_path = dataProvider->mcnum - startpathnum;
				unsigned int size_action = dataProvider->actiontimes.size();
				for (unsigned int j = 0; j < size_action; j++)
				{
					postjudge_[j * size_path + i] = postjudge[j];
					judge_[j * size_path + i] = judge[j];
					explained_[j][i] = explained[j];
					rebate_[j][i] = rebate[j];
				}

				if (explanatory_.size() == 0 && (num_explanatory = explanatory[0].size()) != 0)
				{
					explanatory_.resize(dataProvider->actiontimes.size(), DoubleArray((dataProvider->mcnum - startpathnum) * num_explanatory));
				}
				if (num_explanatory != 0)
					for (unsigned int j = 0; j < size_action; j++)
						for (unsigned int k = 0; k < explanatory[j].size(); k++)
							explanatory_[j][num_explanatory * i + k] = explanatory[j][k];		
						

			}			
			else
				pv1_tmp = value_backward(time1, cf1, triggerhit, rebate, explanatory, dataProvider, callval);
			
			if (expiryNum != callval.size())
			{
				throw AQLCoreInvalidData("callval size must be equal to expirly times", __FILE__, __LINE__);
			}
			for(unsigned int k = 0; k < expiryNum; k++)
			{
				const double val = numeraire_b * callval[k];
				avecall[k] += val;
				aveExpNet[k] += val;
				if (val > 0) aveExpPos[k] += val;
				else		 aveExpNeg[k] += val;
			}
		}
		pv1_tmp *= numeraire_b;
		pv1 += pv1_tmp;	

		if (!dataProvider->isstartable)
		{
			if (dataProvider->settle == dataProvider->baseterm)
				accruedint2_tmp = accruedint_tmp;
			else
				accruedint2_tmp = accruedint_tmp * (*dataProvider->pNumeraire)(dataProvider->baseterm) / (*dataProvider->pNumeraire)(dataProvider->settle);
			pv1_square += (pv1_tmp - accruedint2_tmp) * (pv1_tmp - accruedint2_tmp);
			accruedint2 += accruedint2_tmp;
		}		

		if (istrigger || dataProvider->iscall)
		{
        	for (unsigned int j = 0; j < triggerhit.size(); j++)
				counter_action[triggerhit[j].first][triggerhit[j].second] += 1;
		}

		if (dataProvider->isstartable)
		{
			if (istrigger || !isdetailoutput)
			{
				dataProvider->payoff.calcPayOff(time3, cf3, true, triggerhit, false);
				pv3_tmp = 0.0;
				for (unsigned int j = 0; j < time3.size(); j++)
					for (unsigned int k = 0; k < time3[j].size(); k++)
						if (time3[j][k] > dataProvider->settle)
						{
							if (dataProvider->rcvpay[j])	pv3_tmp += cf3[j][k] / (*dataProvider->pNumeraire)(time3[j][k]);
							else pv3_tmp -= cf3[j][k] / (*dataProvider->pNumeraire)(time3[j][k]);
						}
				pv3_tmp *= (*dataProvider->pNumeraire)(dataProvider->baseterm);
				
			}
			else
				pv3_tmp = pv2_tmp;

			pv3 += pv3_tmp;
		}

		if (dataProvider->isstartable) pv1_square += (-pv1_tmp + pv3_tmp) * (-pv1_tmp + pv3_tmp); 

	}

	// divide by mc num
	const int mcnum = dataProvider->mcnum - startpathnum;
	DoubleVector div_vec(dataProvider->maxcfnum, mcnum);
	for (unsigned int i= 0; i < legnum; i++)
	{
		transform(avecf1[i].begin(), avecf1[i].end(), div_vec.begin(), avecf1[i].begin(), divides<double>());
		transform(avepv1[i].begin(), avepv1[i].end(), div_vec.begin(), avepv1[i].begin(), divides<double>());
		transform(avecf2[i].begin(), avecf2[i].end(), div_vec.begin(), avecf2[i].begin(), divides<double>());
		transform(avepv2[i].begin(), avepv2[i].end(), div_vec.begin(), avepv2[i].begin(), divides<double>());
	}
	div_vec.resize(avecall.size(), mcnum);
	transform(avecall.begin(), avecall.end(), div_vec.begin(), avecall.begin(), divides<double>());
	div_vec.resize(expNum, mcnum);
	transform(aveExpNet.begin(), aveExpNet.end(), div_vec.begin(), aveExpNet.begin(), divides<double>());
	transform(aveExpPos.begin(), aveExpPos.end(), div_vec.begin(), aveExpPos.begin(), divides<double>());
	transform(aveExpNeg.begin(), aveExpNeg.end(), div_vec.begin(), aveExpNeg.begin(), divides<double>());
		
	// Calculate Fee Value
	double pvFee = calcFeeValueExo(object, dataProvider);

	//////////
	//output//
	//////////
	pv1 /= double(dataProvider->mcnum - startpathnum);
	pv2 /= double(dataProvider->mcnum - startpathnum);
	pv3 /= double(dataProvider->mcnum - startpathnum);
	pv1_square /= double(dataProvider->mcnum - startpathnum);
	accruedint /= double(dataProvider->mcnum - startpathnum);
	accruedint2 /= double(dataProvider->mcnum - startpathnum);

	// valuedate calc
	double ratio = calcValueTermRatio(*dataProvider);
	pv1 *= ratio;
	pv2 *= ratio;
	pv3 *= ratio;
	pv1_square *= ratio * ratio;
	DoubleVector valueRatioVec(dataProvider->maxcfnum, ratio);
	for (unsigned int i= 0; i < legnum; i++)
	{
		transform(avecf1[i].begin(), avecf1[i].end(), valueRatioVec.begin(), avecf1[i].begin(), multiplies<double>());
		transform(avepv1[i].begin(), avepv1[i].end(), valueRatioVec.begin(), avepv1[i].begin(), multiplies<double>());
		transform(avecf2[i].begin(), avecf2[i].end(), valueRatioVec.begin(), avecf2[i].begin(), multiplies<double>());
		transform(avepv2[i].begin(), avepv2[i].end(), valueRatioVec.begin(), avepv2[i].begin(), multiplies<double>());
	}

	double cleanprice1, cleanprice2, dirtyprice1;
	if (dataProvider->isstartable)
	{
		cleanprice1 = -pv1 + pv3;
		cleanprice2 = pv2;
		dirtyprice1 = cleanprice1;
	}
	else
	{
		cleanprice1 = pv1 - accruedint2;
		cleanprice2 = pv2 - accruedint2;
		dirtyprice1 = pv1;
	}
	double fxrate = 1.0;
	if (dataProvider->basecur != dataProvider->numerairecur)
	{
		fxrate = getFXEntity(object).getRate(dataProvider->numerairecur, dataProvider->basecur, dataProvider->baseterm);
		cleanprice1 *= fxrate;
		cleanprice2 *= fxrate;	
		dirtyprice1 *= fxrate;
		pv1_square *= fxrate * fxrate;	
		accruedint *= fxrate;
		accruedint2 *= fxrate;
		for (unsigned int i= 0; i < legnum; i++)
		{
			DoubleArray fx_cf_multiple(avecf1[i].size(), fxrate);
			transform(avecf1[i].begin(), avecf1[i].end(), fx_cf_multiple.begin(), avecf1[i].begin(), multiplies<double>());
			transform(avepv1[i].begin(), avepv1[i].end(), fx_cf_multiple.begin(), avepv1[i].begin(), multiplies<double>());
			transform(avecf2[i].begin(), avecf2[i].end(), fx_cf_multiple.begin(), avecf2[i].begin(), multiplies<double>());
			transform(avepv2[i].begin(), avepv2[i].end(), fx_cf_multiple.begin(), avepv2[i].begin(), multiplies<double>());
		}
	}

	double pv1_deviation = AQLMath::sqrt(AQLMath::max((pv1_square - cleanprice1 * cleanprice1), 0.0) / dataProvider->mcnum);

	// calc cash
	map<AQLString, double> cashMap;
	for (unsigned int i = 0; i < dataProvider->cashvec.size(); i++)
	{
		double sign = 1.0;
		if (!dataProvider->rcvpay[i])
		{
			sign = -1.0;
		}

		const unsigned int cfnum = dataProvider->cashvec[i].size();
		for (unsigned int j = 0; j < cfnum; j++)
		{
			// get coupon and calc
			const AQLPricePayOffTool &pt = dataProvider->cashvec[i][j].getPayOff();
			pt.calcPayOff();
			// notional cf when notional exchange occurred
			if (pt.mNotionalCFCur != "" && pt.mNotionalCur != pt.mNotionalCFCur)
			{
				cashMap[pt.mNotionalCFCur] += sign * pt.mNotional * pt.mNotionalCFExchangeRate;
			}
			// notional exchange
			cashMap[pt.mNotionalCur] += sign * pt.mNotionalCF;
			// extracf
			cashMap[pt.mExtraCFCur] += sign * pt.mExtraCF;
			// accrued interest
			if (pt.mAccruedInt != 0.0 && !pt.mCouponsCur.empty())
			{
				cashMap[pt.mCouponsCur[pt.mSelectedCpnPos]] += sign * pt.mAccruedInt;
			}
			// coupon
			if (pt.isCouponPayment())
			{
				cashMap[pt.mCouponsCur[pt.mSelectedCpnPos]] += sign * pt.mCpnPayOffbyPayOffCur;
			}
		}
	}
	// check option holder
	if (!dataProvider->isoptionholder)
	{
		cleanprice1 *= -1.0;
		cleanprice2 *= -1.0;
		dirtyprice1 *= -1.0;
		accruedint *= -1.0;
		transform(avecall.begin(), avecall.end(), avecall.begin(), negate<double>());
		transform(aveExpNet.begin(), aveExpNet.end(), aveExpNet.begin(), negate<double>());
		transform(aveExpPos.begin(), aveExpPos.end(), aveExpPos.begin(), negate<double>());
		transform(aveExpNeg.begin(), aveExpNeg.end(), aveExpNeg.begin(), negate<double>());
		map<AQLString, double>::iterator cIt = cashMap.begin();
		while (cIt !=  cashMap.end())
		{
			cIt->second *= -1.0;
			++cIt;
		}
	}
	double feeExcludedPV = dirtyprice1;
	// add fee pv
	cleanprice1 += pvFee;
	cleanprice2 += pvFee;
	dirtyprice1 += pvFee;


	object.remove(PRICING_DATA_CLEANPRICE);
	object.remove(PRICING_DATA_DIRTYPRICE);
	object.remove(PRICING_DATA_ACCRUEDINTEREST);
	object.remove(PRICING_DATA_CLEANPRICESQUARE);
	object.remove(PRICING_DATA_CLEANPRICEDEVIATION);
	object.remove(PRICING_DATA_CALLTRIGGERVALUE);
	object.remove(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER);
	object.remove(PRICING_DATA_PVCURRENCY);
	object.remove(PRICING_DATA_FEE_EXCLUDED_PV);

	
	object.add(PRICING_DATA_CLEANPRICE, new AQLDataDouble(cleanprice1));
	object.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(dirtyprice1));
	object.add(PRICING_DATA_ACCRUEDINTEREST, new AQLDataDouble(accruedint));
	object.add(PRICING_DATA_CLEANPRICESQUARE, new AQLDataDouble(pv1_square));
	object.add(PRICING_DATA_CLEANPRICEDEVIATION, new AQLDataDouble(pv1_deviation));
	object.add(PRICING_DATA_PVCURRENCY, new AQLDataString(dataProvider->basecur));
	object.add(PRICING_DATA_FEE_EXCLUDED_PV, new AQLDataDouble(feeExcludedPV));

	// set cash attr
	map<AQLString, double>::const_iterator cash_it = cashMap.begin();
	while (cash_it != cashMap.end())
	{
		AQLString cash_attr = cash_it->first + "_" + PRICING_DATA_CASH;
		object.remove(cash_attr);
		object.add(cash_attr, new AQLDataDouble(cash_it->second));
		++cash_it;
	}
//#ifdef XLLAPLI

	if ( isResultOut )
	{
		for (unsigned int i = 0; i < legnum; i++)
		{
			// set underlying value(for each cachlet)
			AQLString cfname = PRICING_DATA_CASHLETVALUE_LEG + AQLDataInt(i + 1).convertToString();
			AQLString time = PRICING_DATA_CASHLETVALUETIME_LEG + AQLDataInt(i + 1).convertToString();
			AQLString pvname = PRICING_DATA_PVVALUE_LEG + AQLDataInt(i + 1).convertToString();
			object.remove(cfname);
			object.add(cfname, new AQLDataDoubles(avecf2[i]));
			object.remove(time);
			object.add(time, new AQLDataDoubles(dataProvider->paytimes[i]));
			object.remove(pvname);
			object.add(pvname, new AQLDataDoubles(avepv2[i]));

			AQLString payDateName = PRICING_DATA_PAYMENTDATE_LEG + AQLDataInt(i + 1).convertToString();
			object.remove( payDateName );
			if ( i < paymentDates.size() )
				object.add( payDateName, new AQLDataDates( paymentDates[i] ) );
			AQLString startDateName = PRICING_DATA_CFCALCSTARTDATE_LEG + AQLDataInt(i + 1).convertToString();
			object.remove( startDateName );
			if ( i < cfCalcStartDates.size() )
				object.add( startDateName, new AQLDataDates( cfCalcStartDates[i] ) );
			AQLString endDateName = PRICING_DATA_CFCALCENDDATE_LEG + AQLDataInt(i + 1).convertToString();
			object.remove( endDateName );
			if ( i < cfCalcEndDates.size() )
				object.add( endDateName, new AQLDataDates( cfCalcEndDates[i] ) );


			if (isdetailoutput)
			{
				// trigger pv
				DoubleVector cf_trigger	= avecf1[i];			
				DoubleVector pv_trigger	= avepv1[i];
				// minus underlying value
				transform(cf_trigger.begin(), cf_trigger.end(), avecf2[i].begin(), cf_trigger.begin(), minus<double>());
				transform(pv_trigger.begin(), pv_trigger.end(), avepv2[i].begin(), pv_trigger.begin(), minus<double>());
				
				AQLString triggername = PRICING_DATA_TRIGGERVALUE_LEG + AQLDataInt(i + 1).convertToString();
				AQLString triggertime = PRICING_DATA_TRIGGERVALUETIME_LEG + AQLDataInt(i + 1).convertToString();
				AQLString triggerpvname = PRICING_DATA_TRIGGERPVVALUE_LEG + AQLDataInt(i + 1).convertToString();
				object.remove(triggername);
				object.add(triggername, new AQLDataDoubles(cf_trigger));
				object.remove(triggertime);
				object.add(triggertime, new AQLDataDoubles(dataProvider->paytimes[i]));
				object.remove(triggerpvname);
				object.add(triggerpvname, new AQLDataDoubles(pv_trigger));

				if (!extracf_cfmap[i].empty())
				{
					const unsigned int excf_size = extracf_cfmap[i].size();
					if (excf_size != extracf_pvmap[i].size())
					{
						throw AQLCoreInvalidData("extra cf calc index error ", __FILE__, __LINE__);
					}

					// param for output data
					DoubleVector time_extra;
					DoubleVector cf_extra;
					DoubleVector pv_extra;

					map<double, double>::const_iterator it_cf = extracf_cfmap[i].begin();
					map<double, double>::const_iterator it_pv = extracf_pvmap[i].begin();
					while (it_cf != extracf_cfmap[i].end())
					{
						time_extra.push_back(it_cf->first);
						cf_extra.push_back(it_cf->second * fxrate / mcnum);
						pv_extra.push_back(it_pv->second * fxrate / mcnum);
						++it_cf, ++it_pv;
					}

					AQLString extracf_cfname = PRICING_DATA_EXTRACFVALUE_LEG + AQLDataInt(i + 1).convertToString();
					AQLString extracf_time = PRICING_DATA_EXTRACFVALUETIME_LEG + AQLDataInt(i + 1).convertToString();
					AQLString extracf_pvname = PRICING_DATA_EXTRACFPVVALUE_LEG + AQLDataInt(i + 1).convertToString();
					object.remove(extracf_cfname);
					object.add(extracf_cfname, new AQLDataDoubles(cf_extra));
					object.remove(extracf_time);
					object.add(extracf_time, new AQLDataDoubles(time_extra));
					object.remove(extracf_pvname);
					object.add(extracf_pvname, new AQLDataDoubles(pv_extra));
				}
			}
		}
		if (isdetailoutput)
		{
			//call pv
			AQLString calltime = "CallValueTime";
			AQLString callname = "CallValue";
			object.remove(calltime);
			object.add(calltime, new AQLDataDoubles(dataProvider->expirytimes));
			object.remove(callname);
			object.add(callname, new AQLDataDoubles(avecall));
		}
		//exposure
		AQLString expTime = "ExposureTime";
		AQLString expNameNet = "NetExposure";
		AQLString expNamePos = "PositiveExposure";
		AQLString expNameNeg = "NegativeExposure";
		object.remove(expTime);
		object.add(expTime, new AQLDataDoubles(expTimes));
		object.remove(expNameNet);
		object.add(expNameNet, new AQLDataDoubles(aveExpNet));
		object.remove(expNamePos);
		object.add(expNamePos, new AQLDataDoubles(aveExpPos));
		object.remove(expNameNeg);
		object.add(expNameNeg, new AQLDataDoubles(aveExpNeg));
		if (ispathdetailoutput)
		{
			std::map<AQLString, AQLString> idxGridMap;
			for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
			{
				const AQLString idxname = (*it)->getCurrency().get()+(*it)->getIndexType().get()+(*it)->getAccessory().get();
				const AQLDataDoubles grid((*it)->getTimeGrid());
				idxGridMap[idxname] = grid.convertToString();
			}
			AQLStringMatrix outMtx;
			for (std::map<AQLString, AQLStringVector>::const_iterator it = idxPathMap.begin(); it != idxPathMap.end(); ++it)
			{
				AQLStringVector index = it->second;
				index.insert(index.begin(), idxGridMap[it->first]);
				index.insert(index.begin(), it->first);
				outMtx.push_back(index);
			}
			object.remove(PRICING_DATA_PATHDETAIL);
			object.add(PRICING_DATA_PATHDETAIL, new AQLDataStringMatrix(outMtx));
			idxGridMap.clear();
			idxPathMap.clear();
		}
	}

//#endif

	if (isdetailoutput)
	{
		object.add(PRICING_DATA_CALLTRIGGERVALUE, new AQLDataDouble(cleanprice1 - cleanprice2));
		object.add(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER, new AQLDataDouble(cleanprice2));
	}
	//action probabilities
	//average life 
	double aveLife = 0.;
	double prob_sum = 0.;
	if (dataProvider->iscall)
	{
		DoubleArray probs(counter_action[0].size());
		map<AQLDate, unsigned int>::const_iterator it;
		unsigned int i = 0;
		for (it = counter_action[0].begin(); it != counter_action[0].end(); it++, i++)
			probs[i] = (double)it->second / double(dataProvider->mcnum - startpathnum);
		
		pcallinfo->remove(PRICING_DATA_ACTIONPROBABILITIES);
		pcallinfo->add(PRICING_DATA_ACTIONPROBABILITIES, new AQLDataDoubles(probs));
		
		for (it = counter_action[0].begin(); it != counter_action[0].end(); it++)
		{
			prob_sum += (double)it->second / double(dataProvider->mcnum - startpathnum);
			aveLife += dataProvider->pPath->getDayCount().getTerm(basedate, it->first) * (double)it->second / double(dataProvider->mcnum - startpathnum);
		}
		
		if (islsmcdetailoutput)
		{
			pcallinfo->remove(PRICING_DATA_LSMCREGCOEFFICIENTS);
			DoubleMatrix lsmcparam(dataProvider->coefficient->get1DSize(), DoubleArray(dataProvider->coefficient->getSize(0) - 1));
			for (unsigned int i = 0; i < lsmcparam.size(); i++)
			{
				for (unsigned int j = 0; j < lsmcparam[i].size(); j++)
				{
					lsmcparam[i][j] = -dataProvider->coefficient->get()[i][j];
				}
			}
			pcallinfo->add(PRICING_DATA_LSMCREGCOEFFICIENTS, new AQLDataDoubleMatrix(lsmcparam));

			pcallinfo->remove(PRICING_DATA_LSMCREGCOEFFICIENTS2);
			DoubleMatrix lsmcparam_rebate(dataProvider->coefficient_rebate->get1DSize(), DoubleArray(dataProvider->coefficient_rebate->getSize(0) - 1));
			for (unsigned int i = 0; i < lsmcparam_rebate.size(); i++)
			{
				for (unsigned int j = 0; j < lsmcparam_rebate[i].size(); j++)
				{
					lsmcparam_rebate[i][j] = -dataProvider->coefficient_rebate->get()[i][j];
				}
			}
			pcallinfo->add(PRICING_DATA_LSMCREGCOEFFICIENTS2, new AQLDataDoubleMatrix(lsmcparam_rebate));

			if (startpathnum == 0)
			{
				//lsmc detail
				pcallinfo->remove(PRICING_DATA_LSMCEXPLANATORYVARIABLES);
				pcallinfo->remove(PRICING_DATA_LSMCEXPLAINEDVARIABLES);
				pcallinfo->remove(PRICING_DATA_LSMCCALLPOSTJUDGE);
				pcallinfo->remove(PRICING_DATA_LSMCCALLJUDGE);
				pcallinfo->remove(PRICING_DATA_CALLREBATE);
				pcallinfo->add(PRICING_DATA_LSMCEXPLANATORYVARIABLES, new AQLDataDoubleMatrix(explanatory_));
				pcallinfo->add(PRICING_DATA_LSMCEXPLAINEDVARIABLES, new AQLDataDoubleMatrix(explained_));
				pcallinfo->add(PRICING_DATA_LSMCCALLPOSTJUDGE, new AQLDataBools(postjudge_));
				pcallinfo->add(PRICING_DATA_LSMCCALLJUDGE, new AQLDataBools(judge_));
				pcallinfo->add(PRICING_DATA_CALLREBATE, new AQLDataDoubleMatrix(rebate_));
			}
			else
			{
				dh = &pcallinfo->getData(PRICING_DATA_LSMCEXPLAINEDVARIABLES, ISNOTNULL);
				AQLDataDoubleMatrix& data_explanatory = dynamic_cast<AQLDataDoubleMatrix&>(dh->get());
				const DoubleMatrix& explanatory__ = data_explanatory.get();
				double size_action = explanatory_.size();
				for (unsigned int i = 0; i < size_action; i++)
					explanatory_[i].insert(explanatory_[i].begin(), explanatory__[i].begin(), explanatory__[i].end()); 
				data_explanatory.set(explanatory_);


				dh = &pcallinfo->getData(PRICING_DATA_LSMCEXPLAINEDVARIABLES, ISNOTNULL);
				AQLDataDoubleMatrix& data_explained = dynamic_cast<AQLDataDoubleMatrix&>(dh->get());
				const DoubleMatrix& explained__ = data_explained.get();
				for (unsigned int i = 0; i < size_action; i++)
					explained_[i].insert(explained_[i].begin(), explained__[i].begin(), explained__[i].end()); 
				data_explained.set(explained_);				

				dh = &pcallinfo->getData(PRICING_DATA_LSMCCALLPOSTJUDGE, ISNOTNULL);
				AQLDataBools& data_postjudge = dynamic_cast<AQLDataBools&>(dh->get());
				const BoolVector& postjudge__ = data_postjudge.get();
				postjudge_.insert(postjudge_.begin(), postjudge__.begin(), postjudge__.end()); 
				data_postjudge.set(postjudge_);				

				dh = &pcallinfo->getData(PRICING_DATA_LSMCCALLJUDGE, ISNOTNULL);
				AQLDataBools& data_judge = dynamic_cast<AQLDataBools&>(dh->get());
				const BoolVector& judge__ = data_judge.get();
				postjudge_.insert(judge_.begin(), judge__.begin(), judge__.end()); 
				data_judge.set(judge_);				

			}		
		
		}
	}
	if (istrigger)
	{
		for (unsigned int i = 0; i < triggerinfos.size(); i++)
		{
			DoubleArray probs(counter_action[1 + i].size());
			map<AQLDate, unsigned int>::const_iterator it;
			unsigned int j = 0;
			for (it = counter_action[1 + i].begin(); it != counter_action[1 + i].end(); it++, j++)
				probs[j] = (double)it->second / double(dataProvider->mcnum - startpathnum);

			for (it = counter_action[1 + i].begin(); it != counter_action[1 + i].end(); ++it)
			{
				prob_sum += (double)it->second / double(dataProvider->mcnum - startpathnum);
				aveLife += dataProvider->pPath->getDayCount().getTerm(basedate, it->first) * (double)it->second / double(dataProvider->mcnum - startpathnum);
			}
			
			triggerinfos[i]->remove(PRICING_DATA_ACTIONPROBABILITIES);
			triggerinfos[i]->add(PRICING_DATA_ACTIONPROBABILITIES, new AQLDataDoubles(probs));
		}
	}

	DoubleVector tmpLastPayment;
	for (unsigned int i = 0; i < dataProvider->paytimes.size(); ++i)
	{
		if (!dataProvider->paytimes[i].empty())
		{
			tmpLastPayment.push_back(dataProvider->paytimes[i].back());
		}
	}
	if (!tmpLastPayment.empty())
	{
		double lastPaymentTerm = *max_element(tmpLastPayment.begin(), tmpLastPayment.end());
		aveLife += lastPaymentTerm * (1. - prob_sum);
	}

	object.remove(PRICING_DATA_AVERAGELIFE);
	object.add(PRICING_DATA_AVERAGELIFE, new AQLDataDouble(aveLife));

	dynamic_cast<AQLDataString&>(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).set(currency_ori);
	return dirtyprice1;

}

#endif
/*!
	@brief get reference index entities

	@param[in] trade trade

	@return reference index entities
	
*/
set<AQLMathIndexEntity*>
AQLPriceTradeValue::getReferenceIndex(AQLObject& trade) const
{
	set<AQLMathIndexEntity*> indexs;
	AQLDataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		//cashlets
		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;
		AQLDataMultiReference& cashlets = dynamic_cast<AQLDataMultiReference&>(dh->get());
		
		for (unsigned int j = 0; j < cashlets.getSize(); j++)
		{
			// range accrue index info
			dh = &(cashlets.get(j).getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				if (dynamic_cast<const AQLDataBool &>(dh->get()).get())
				{
					dh = &(cashlets.get(j).getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK));
					if(dh->isDefined() && !dh->isNull()) // multi index case
					{
						const AQLDataMultiReference& rainfos = dynamic_cast<const AQLDataMultiReference &>(dh->get());
						for (int l = 0; l < rainfos.getSize(); l++)
						{
							dh = &(rainfos.get(l).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								AQLDataMultiReference& raindexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
								for (unsigned int k = 0; k < raindexinfos.getSize(); k++)
								{
									//range accrue index object
									dh = &(raindexinfos.get(k).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
									if (!dh->isDefined() || dh->isNull()) continue;
									AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
									indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
								}
							}

							dh = &(rainfos.get(l).getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								AQLDataMultiReference& rabindexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
								for (unsigned int k = 0; k < rabindexinfos.getSize(); k++)
								{
									//range accrue index object
									dh = &(rabindexinfos.get(k).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
									if (!dh->isDefined() || dh->isNull()) continue;
									AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
									indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
								}
							}
						}
					}
					else // single index case
					{
						dh = &(cashlets.get(j).getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							AQLDataMultiReference& raindexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
							for (unsigned int k = 0; k < raindexinfos.getSize(); k++)
							{
								//range accrue index object
								dh = &(raindexinfos.get(k).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
								if (!dh->isDefined() || dh->isNull()) continue;
								AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
								indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
							}
						}

						dh = &(cashlets.get(j).getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK));
						if (dh->isDefined() && !dh->isNull())
						{
							AQLDataMultiReference& rabindexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
							for (unsigned int k = 0; k < rabindexinfos.getSize(); k++)
							{
								//range accrue index object
								dh = &(rabindexinfos.get(k).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
								if (!dh->isDefined() || dh->isNull()) continue;
								AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
								indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
							}
						}
					}
				}
			}
			//coupon info
			dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				AQLDataMultiReference& couponinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());

				for (unsigned int k = 0; k < couponinfos.getSize(); k++)
				{
					//index info
					dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;

					AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
					for (unsigned int l = 0; l < indexinfos.getSize(); l++)
					{
						//index object
						dh = &(indexinfos.get(l).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
						if (!dh->isDefined() || dh->isNull()) continue;
						AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
						indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
					}
				}
			}

			//notional coupon info
			dh = &(cashlets.get(j).getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				AQLDataMultiReference& couponinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());

				for (unsigned int k = 0; k < couponinfos.getSize(); k++)
				{
					//index info
					dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;

					AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
					for (unsigned int l = 0; l < indexinfos.getSize(); l++)
					{
						//index object
						dh = &(indexinfos.get(l).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
						if (!dh->isDefined() || dh->isNull()) continue;
						AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
						indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
					}
				}
			}

            //// when compounding coupon, cashlet may have first/last stub coupon
            //if((dh=&cashlets.get(j).getData(PRICING_DATA_FIRSTSTUBCOUPON))->isDefined() && !dh->isNull()){
            //    AQLObject& first_stub_coupon = dynamic_cast<AQLDataReference&>(dh->get()).get().get();
            //    if((dh=&first_stub_coupon.getData(PRICING_DATA_INDEXINFOS))->isDefined() && !dh->isNull()){
            //        AQLDataMultiReference& first_stub_indices = dynamic_cast<AQLDataMultiReference&>(dh->get());
            //        for(size_t k = 0; k < first_stub_indices.getSize(); k++){
            //            if((dh=&first_stub_indices.get(k).get().getData(PRICING_DATA_INDEXENTITY))->isDefined() && !dh->isNull()){
            //                indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(dynamic_cast<AQLDataReference&>(dh->get()).get().get()));
            //            }
            //        }
            //    }
            //}
            //if((dh=&cashlets.get(j).getData(PRICING_DATA_LASTSTUBCOUPON))->isDefined() && !dh->isNull()){
            //    AQLObject& last_stub_coupon = dynamic_cast<AQLDataReference&>(dh->get()).get().get();
            //    if((dh=&last_stub_coupon.getData(PRICING_DATA_INDEXINFOS))->isDefined() && !dh->isNull()){
            //        AQLDataMultiReference& last_stub_indices = dynamic_cast<AQLDataMultiReference&>(dh->get());
            //        for(size_t k = 0; k < last_stub_indices.getSize(); k++){
            //            if((dh=&last_stub_indices.get(k).get().getData(PRICING_DATA_INDEXENTITY))->isDefined() && !dh->isNull()){
            //                indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(dynamic_cast<AQLDataReference&>(dh->get()).get().get()));
            //            }
            //        }
            //    }            
            //}
		}
	}
	//trigger
	dh = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		AQLDataMultiReference& triggers = dynamic_cast<AQLDataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < triggers.getSize(); i++)
		{
			//index info(trigger reference)
			dh = &(triggers.get(i).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull()) 
			{
				AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
				for (unsigned int j = 0; j < indexinfos.getSize(); j++)
					{
						//index object
						dh = &(indexinfos.get(j).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
						if (!dh->isDefined() || dh->isNull()) continue;
						AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
						indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
						
					}
			}
			//extracf index info
			dh = &(triggers.get(i).getData(PRICING_DATA_EXTRACFINDEXINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull()) 
			{
				AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
				for (unsigned int j = 0; j < indexinfos.getSize(); j++)
				{
					//index object
					dh = &(indexinfos.get(j).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;
					AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
					indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));	
				}
			}
			dh = &(triggers.get(i).getData(PRICING_DATA_TRIGGERTARGETS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const AQLStringVector& targets = dynamic_cast<const AQLDataStrings&>(dh->get()).get();	
				//object pool
				AQLObjectPool& objPool = triggers.get(i).getDataInstance()->getObjectPool();
				for (unsigned int j = 0; j < targets.size(); j++)
				{
					AQLObjectHolder objHolder = objPool.getObject(targets[j], ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined()) continue;
					dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;// not indexinfo
					dh = &(objHolder.getData(PRICING_DATA_INDEXENTITY, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;
					AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
					indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
				}
			}					

			//cpn change case			
			dh = &(triggers.get(i).getData(PRICING_DATA_COUPONCHANGEINFO, NOCHECK));	
			if (!dh->isDefined() || dh->isNull()) continue;			
			
			for (unsigned int j = 0; j < legs.getSize(); j++)
			{				
				//coupon info
				dh = &(triggers.get(i).getData(PRICING_DATA_COUPONINFOS + AQLDataInt(j + 1).convertToString(), NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				
				AQLDataMultiReference& couponinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
				for (unsigned int k = 0; k < couponinfos.getSize(); k++)
				{
					//index info
					dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;

					AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
					for (unsigned int l = 0; l < indexinfos.getSize(); l++)
					{
						//index object
						dh = &(indexinfos.get(l).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
						if (!dh->isDefined() || dh->isNull()) continue;
						AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
						indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
						
					}
				}
			}
		}
	}
	//call
	dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		AQLDataReference& call = dynamic_cast<AQLDataReference&>(dh->get());	
		dh = &(call.get().getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) 
		{
			AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
			for (unsigned int i = 0; i < indexinfos.getSize(); i++)
			{
				//index object
				dh = &(indexinfos.get(i).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
				indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
			}
		}
		// extracf index info
		dh = &(call.get().getData(PRICING_DATA_EXTRACFINDEXINFOS, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) 
		{
			AQLDataMultiReference& indexinfos = dynamic_cast<AQLDataMultiReference&>(dh->get());
			for (unsigned int i = 0; i < indexinfos.getSize(); i++)
			{
				//index object
				dh = &(indexinfos.get(i).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
				indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));				
			}
		}
		dh = &(call.get().getData(PRICING_DATA_LSMCVARIABLES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLStringVector& targets = dynamic_cast<const AQLDataStrings&>(dh->get()).get();	
			//object pool
			AQLObjectPool& objPool = call.get().getDataInstance()->getObjectPool();
			for (unsigned int i = 0; i < targets.size(); i++)
			{
				AQLObjectHolder objHolder = objPool.getObject(targets[i], ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined()) continue;
				dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;// not indexinfo
				dh = &(objHolder.getData(PRICING_DATA_INDEXENTITY, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
				indexs.insert(&dynamic_cast<AQLMathIndexEntity&>(ref.get().get()));
			}
		}
	}
	return indexs;
}

#ifndef VISUAL_STUDIO_2010_ANALYTICS

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*
AQLPriceTradeValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
								const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	AQLPriceTradeValueDataProvider* dataProvider = dynamic_cast<AQLPriceTradeValueDataProvider*>(createNewDataProvider());
	att.setDataProvider(dataProvider);
	
	// mc num
	dh = &(object.getData(PRICING_DATA_MCNUM, ISNOTNULL));
	dataProvider->mcnum = dynamic_cast<const AQLDataInt&>(dh->get()).get();		
	
	// base currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dataProvider->basecur = dynamic_cast<const AQLDataString&>(dh->get()).get();
	dataProvider->credit_ccy = dynamic_cast<const AQLDataString&>(dh->get()).get();

	// valuation currency
	dh = &(object.getData(PRICING_DATA_VALUATIONCURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->basecur = dynamic_cast<const AQLDataString&>(dh->get()).get();

	// path object
	dh = &(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	AQLDataReference& attr = dynamic_cast<AQLDataReference&>(dh->get());
	dataProvider->pPath = &dynamic_cast<AQLMathPathEntity&>(attr.get().get());
	AQLDate asofdate = dataProvider->pPath->getAsOfDate();
	// numeraire of base currency
	dataProvider->pNumeraire = NULL;
	AQLStringVector names = dataProvider->pPath->getSimulationSDEAttrNames().get();
	if (names.size() == 0)
		names = dataProvider->pPath->getSDEAttrNames().get();

	for (unsigned int i = 0; i < names.size(); i++)
	{
		dh = &dataProvider->pPath->getData(names[i], ISNOTNULL);
		AQLMathAttrSDE& attr = dynamic_cast<AQLMathAttrSDE&>(dh->get());
		if (attr.getSDEPathType() == IR)
		{		
			dataProvider->pNumeraire = attr.getSDE().getNumeraire();
			dataProvider->numerairecur = attr.getCurrency();
			break;
		}
	}

	if (dataProvider->pNumeraire == NULL)
	{
		//error
		throw AQLCoreInvalidData("Numeraire does not exist", __FILE__, __LINE__);	
	}

	//dataProvider->pFX = NULL;
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dynamic_cast<AQLDataString&>(dh->get()).set(dataProvider->numerairecur);
	
	// today
	dh = &(object.getData(PRICING_DATA_TODAY, ISNOTNULL));
	const AQLDate& today = dynamic_cast<const AQLDataDate&>(dh->get()).get();
	if (today > basedate)
	{
		//error
		throw AQLCoreInvalidData("basedate must be same or after today", __FILE__, __LINE__);	
	}

	// settle date
	AQLDate settledate;
	dh = &(object.getData(PRICING_DATA_SETTLEDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		settledate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
	else
	{
		dh = &(object.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
		int spotlag = dynamic_cast<const AQLDataInt&>(dh->get()).get();
		dh = &(object.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
		settledate = cal.getBusinessDay(today, spotlag);
	}
	if (today > settledate)
	{
		//error
		throw AQLCoreInvalidData("settledate must be same or after today", __FILE__, __LINE__);	
	}
	if (basedate > settledate)
	{
		//error
		throw AQLCoreInvalidData("settledate must be same or after basedate", __FILE__, __LINE__);	
	}
	dataProvider->settledate = settledate;
	// settle
	dataProvider->settle = dataProvider->pPath->getDayCount().getTerm(today, settledate);
	// baseterm
	if (today < basedate)
		dataProvider->baseterm = dataProvider->pPath->getDayCount().getTerm(today, basedate);
	else 
		dataProvider->baseterm = 0.0;

	// value date
	dh = &(object.getData(PRICING_DATA_VALUEDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDate& date = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		if (date < asofdate)
		{
			dataProvider->valueDate = asofdate;
		}
		else
		{
			dataProvider->valueDate = date;
		}
	}
	else
	{
		//dataProvider->valueDate = today;
		dataProvider->valueDate = asofdate;
	}

	//leg object
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference&>(dh->get());
	dataProvider->rcvpay.resize(legs.getSize());
	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_SELECTSIDE, ISNOTNULL));
		AQLString sltside = dynamic_cast<const AQLDataString&>(dh->get()).get();
		sltside.toUpper();
		if (sltside == RCV) dataProvider->rcvpay[i] = true;
		else if (sltside == PAY) dataProvider->rcvpay[i] = false;
		else
		{
			//error
			AQLString msg = PRICING_DATA_SELECTSIDE;
			msg += ": " + sltside;
			msg += " is a wrong input";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}

	}
	// option holder
	dataProvider->isoptionholder = true;
	dh = &(object.getData(PRICING_DATA_ISOPTIONHOLDER, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->isoptionholder = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	}

	// isstartable
	dataProvider->isstartable = false;
	dataProvider->iscall = false;
	dataProvider->iscalcswap = false;
	dh = &(object.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		dataProvider->iscall = true;
		AQLDataReference& attr = dynamic_cast<AQLDataReference&>(dh->get());		
		dh = &attr.get().getData(PRICING_DATA_SELECTCALLTYPE, ISNOTNULL);
		AQLString calltype = dynamic_cast<const AQLDataString&>(dh->get()).get();
		calltype.toUpper();
		if (calltype == STARTABLE) dataProvider->isstartable = true;
		else if (calltype == CANCELABLE) dataProvider->isstartable = false;
		else
		{
			//error
			AQLString msg = PRICING_DATA_SELECTCALLTYPE;
			msg += ": " + calltype;
			msg += " is a wrong input";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}
		dataProvider->payoff.setLSMCMode(true);

		dh = &attr.get().getData(PRICING_DATA_INDEXINFOS, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) 
		{
			const AQLDataMultiReference &indexinfos = dynamic_cast<const AQLDataMultiReference &>(dh->get());
			for (unsigned int i = 0; i < indexinfos.getSize(); i++) 
			{ 
				//index object 
				AQLString indexType = dynamic_cast<const AQLDataString &>(indexinfos.get(i).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get(); 
				indexType.toUpper();
				if (indexType == CPN || indexType == CPNCF)
				{
					dh = &indexinfos.get(i).getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK);
					if (dh->isDefined() && !dh->isNull()) 
					{
						const AQLString &observationEndTerm = dynamic_cast<const AQLDataString &>(dh->get()).get();
						if (observationEndTerm.findString('-') == 0)
						{
							dataProvider->iscalcswap = true;
						}
					}
				} 
			}
		}
		dh = &attr.get().getData(PRICING_DATA_LSMCVARIABLES, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) 
		{ 
			const AQLStringVector &targets = dynamic_cast<const AQLDataStrings &>(dh->get()).get();	
			//get from object pool
			AQLObjectPool &objPool = attr.get().getDataInstance()->getObjectPool();
			for (unsigned int i = 0; i < targets.size(); i++)
			{
				AQLObjectHolder objHolder = objPool.getObject(targets[i], ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					continue;
				}
				dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
				if (dh->isDefined() || !dh->isNull())
				{
					AQLString indexType = dynamic_cast<const AQLDataString &>(dh->get()).get();
					indexType.toUpper();
					if (indexType == CPN || indexType == CPNCF)
					{
						dh = &objHolder.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK);
						if (dh->isDefined() && !dh->isNull())
						{
							const AQLString &observationEndTerm = dynamic_cast<const AQLDataString &>(dh->get()).get();
							if (observationEndTerm.findString('-') == 0)
							{
								dataProvider->iscalcswap = true;
							}
						}
					}
				} 
			}
		}
	}
	// cashflow generator
	dh = &object.getData(PRICING_DATA_CFGENERATOR, ISNOTNULL);
	AQLDataProcedure& modelDataObj = dynamic_cast<AQLDataProcedure&>(dh->get());
	modelDataObj.calibrateModel(asofdate);

	// setup
	dataProvider->payoff.setUp(asofdate, object);

	// calc maxcfnum and paytime
	const vector<PayOffToolHolderVector> &payvec = dataProvider->payoff.getPayOff();
	dataProvider->paytimes.resize(payvec.size());
	dataProvider->maxcfnum = 0;
	const unsigned int legnum = dataProvider->paytimes.size();
	for (unsigned int i = 0; i < legnum; i++)
	{
		const unsigned int cfnum = payvec[i].size();
		if (dataProvider->maxcfnum < static_cast<int>(cfnum)) dataProvider->maxcfnum = cfnum;
		dataProvider->paytimes[i].resize(cfnum, 0.0);
		for (unsigned int j = 0; j < cfnum; j++)
		{
			dataProvider->paytimes[i][j] = payvec[i][j].getPayOff().mPaymentTime;
		}
	}

	// cash setup
	dataProvider->cashvec.resize(legs.getSize());
    for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		//cashlets
		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined()) continue;
		AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference&>(dh->get());

		//in case of all cashflows has passed through asofdata. 
		if (payvec[i].size() == 0)
			continue;

		unsigned int k = 0;
		for (unsigned int j = 0; j < cashlets.getSize(); j++)
		{
			//paymentdate
			dh = &(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			const AQLDate &date = dynamic_cast<const AQLDataDate&>(dh->get()).get();
			if (date != today) continue;
			
			AQLPricePayOffToolHolder h;
			h.setPayOff(new AQLPricePayOffTool());
			dataProvider->cashvec[i].push_back(h);
			h.getPayOff().setUp(basedate, object, i, cashlets.get(j).get(), dataProvider->payoff, k++);
		}
	}

	// reference index
	dataProvider->indexs = getReferenceIndex(object);
	
	// set up funding spread
	dataProvider->basisgrid_payoff.clear();
	dataProvider->basisspread_payoff.clear();
	dataProvider->basisname_payoff = "";
	dh = &(object.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		//this is important flag : create isDataProvidergagerecalc flag
		object.remove(PRICING_DATA_ISRECALCTRADEDATA);
		object.add(PRICING_DATA_ISRECALCTRADEDATA,new AQLDataBool(true));
		// get discount curve name
		const AQLString& dfCurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
		// get funding spread
		AQLObject& fudingSpreadEntity = dynamic_cast<AQLDataReference& > (object.getData(PRICING_DATA_FUNDINGSPREADENTITY, ISNOTNULL).get()).get().get();
		double spread = dynamic_cast<AQLDataDouble &>(fudingSpreadEntity.getData(PRICING_DATA_FUNDINGSPREAD, ISNOTNULL).get()).get();
		
		const AQLString& yieldProName = dynamic_cast<const AQLDataString&> ((object.getData(PRICING_DATA_YIELDPRONAME, ISNOTNULL)).get()).get();
		AQLMathYieldCurvePro& ycPro = dynamic_cast<AQLMathYieldCurvePro &>(object.getDataInstance()->getObjectPool().getObject(yieldProName, ENCHKTYPE_ISDEFINED).get());
		AQLString suffix_mkt = AQLString("_") + ycPro.getMarketForCurve(dfCurveName);
		//suffix_mkt.toLower();
		// get market data
		const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&> (ycPro.getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());
		bool isCreditCurveExist = true;
		for	(unsigned int i = 0; i < mr.getSize(); ++i)
		{
			//check market type
			AQLString dataType = dynamic_cast<const AQLDataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			dataType.toUpper();
			if (dataType != BASIS)
				throw AQLCoreInvalidData("Market for generating credit curve must be basis!", __FILE__, __LINE__);
			//check spread
			//double rate = dynamic_cast<AQLDataDouble&> ((mr.get(i).getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			//if (rate != spread)
			//{
			AQLDataDouble &attrRate = dynamic_cast<AQLDataDouble &>(mr.get(i).getData(CALIBRATION_DATA_RATE, ISNOTNULL).get());
			attrRate.set(spread);
			//	isCreditCurveExist = false;
			//}
		}
		//generate credit curve
		const AQLStringVector& names = dataProvider->pPath->getSDEAttrNames().get();
		int num = -1;
		for (unsigned int i = 0; i < names.size(); i++)
		{
			const AQLMathAttrSDE& sde = dynamic_cast<const AQLMathAttrSDE&>(dataProvider->pPath->getData(names[i], ISNOTNULL).get());
			AQLString ccy = sde.getCurrency();
			AQLString credit_ccy = dataProvider->credit_ccy;
			if (ccy.toUpper() == credit_ccy.toUpper())
			{
				num = i;
				break;
			}
		}
		if (num == -1)
		{
			throw AQLCoreInvalidData("sdeccy is not found!", __FILE__, __LINE__);
		}

		AQLObject& yieldData =  dynamic_cast<AQLMathYieldCurve& >(dataProvider->pPath->getInitialValues().get(num).get()).getYieldData().get().get();
		const AQLString yieldDataName = dynamic_cast<const AQLDataString &> (yieldData.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		ycPro.getYieldData().convertFromString(yieldDataName);
		//if (!isCreditCurveExist)
		//{		
		yieldData.remove(CALIBRATION_DATA_TERMS + AQLString("_") + dfCurveName);
		yieldData.remove(IR_CALIBRATION_DATA_DFS + AQLString("_") + dfCurveName);
		ycPro.AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
		ycPro.AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(dfCurveName));
		ycPro.setBasisRates();
		yieldData.AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
		//}
		//calculate credit spread
		const AQLInterpolationBase* pInter = &ycPro.getDFInterpolation(&dfCurveName);
		if (dataProvider->credit_ccy == dataProvider->numerairecur)
		{
			dataProvider->basisgrid_payoff = dynamic_cast<const AQLDataDoubles&> (yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
			const DoubleArray& df_basecurve = dynamic_cast<const AQLDataDoubles&> (yieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();
			dataProvider->basisspread_payoff.push_back(0.0);
			for (unsigned int i = 1; i < dataProvider->basisgrid_payoff.size(); ++i)
			{
				double df_credit = pInter->value(dataProvider->basisgrid_payoff[i]);
				if (df_credit < 0. || df_basecurve[i] < 0.)
					throw AQLCoreInvalidData("discount factor is negative!", __FILE__, __LINE__);
				dataProvider->basisspread_payoff.push_back(AQLMath::log(df_basecurve[i] / df_credit) / dataProvider->basisgrid_payoff[i]);
			}
		}
		else //calculate spreads of numeraire currency when it is different from trade currency
		{
			//const AQLStringVector& names = dataProvider->pPath->getSDEAttrNames().get();
			num = -1;
			for (unsigned int i = 0; i < names.size(); i++)
			{
				const AQLMathAttrSDE& sde = dynamic_cast<const AQLMathAttrSDE&>(dataProvider->pPath->getData(names[i], ISNOTNULL).get());
				AQLString ccy = sde.getCurrency();
				AQLString numerairecur = dataProvider->numerairecur;
				if (ccy.toUpper() == numerairecur.toUpper())
				{
					num = i;
					break;
				}
			}
			if (num == -1)
			{
				throw AQLCoreInvalidData("sdeccy is not found!", __FILE__, __LINE__);
			}
			const AQLMathYieldCurve& yield_numeraire = dynamic_cast<const AQLMathYieldCurve& >(dataProvider->pPath->getInitialValues().get(num).get());
			dataProvider->basisgrid_payoff = dynamic_cast<const AQLDataDoubles &>(yield_numeraire.getYieldData().get().getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
			const DoubleArray& df_basecurve = dynamic_cast<const AQLDataDoubles&> (yield_numeraire.getYieldData().get().getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();
			const AQLInterpolationBase* pInter_df_basecur = &ycPro.getBasisDFInterpolation();
			const AQLInterpolationBase* pInter_df_numerairecur = &yield_numeraire.getBasisDFInterpolation();
			dataProvider->basisspread_payoff.push_back(0.0);
			double max_term_credit_ccy = dynamic_cast<const AQLDataDoubles&> (yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + dfCurveName, ISNOTNULL).get()).get().back();
			for (unsigned int i = 1; i < dataProvider->basisgrid_payoff.size(); ++i)
			{
				if (max_term_credit_ccy < dataProvider->basisgrid_payoff[i])
				{
					dataProvider->basisspread_payoff.push_back(0.);
				}
				else
				{
					double df_credit = pInter->value(dataProvider->basisgrid_payoff[i]);
					double df_basecur = pInter_df_basecur->value(dataProvider->basisgrid_payoff[i]);
					double df_numerairecur = pInter_df_numerairecur->value(dataProvider->basisgrid_payoff[i]);
					if (df_credit < 0. || df_basecurve[i] < 0. || df_basecur < 0. || df_numerairecur < 0.)
						throw AQLCoreInvalidData("discount factor is negative!", __FILE__, __LINE__);
					dataProvider->basisspread_payoff.push_back(AQLMath::log(df_basecurve[i] / df_credit * df_basecur / df_numerairecur) / dataProvider->basisgrid_payoff[i]);
				}
			}
		}

		dataProvider->basisname_payoff = PAYOFFBASIS;
	}

	// accrued interest function
	if (mpAcc != NULL)
	{
		mpAcc->setUp(basedate, object, att);
		dataProvider->acc.set(mpAcc->clone(), true);
	}

	if (!dataProvider->iscall)
	{
		clearLSMCPointers(*dataProvider);
		dataProvider->lsmcnum = 0;
		return dataProvider;
	}
	
	bool isTradeValue = (getType() == FN_IR_TRADEVALUE);
	
	dh = &(object.getData(PRICING_DATA_CALLINFO, ISNOTNULL));
	AQLDataReference& call = dynamic_cast<AQLDataReference&>(dh->get());
	AQLObjectHolder& objHolder = call.get();

	//expirytimes actiontimes;
	dh = &(objHolder.getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
	const DateVector& expirydates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
	
	dh = &(objHolder.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
	const DateVector& actiondates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
	
	for (unsigned int i = 0; i < expirydates.size(); i++)
	{
		
		//if (expirydates[i] > today)
		//{
		//	dataProvider->expirytimes.push_back(dataProvider->pPath->getDayCount().getTerm(today, expirydates[i]));
		//	dataProvider->actiontimes.push_back(dataProvider->pPath->getDayCount().getTerm(today, actiondates[i]));
		//	dataProvider->actiondates.push_back(actiondates[i]);
		//}
		if (expirydates[i] > asofdate)
		{
			dataProvider->expirytimes.push_back(dataProvider->pPath->getDayCount().getTerm(asofdate, expirydates[i]));
			dataProvider->actiontimes.push_back(dataProvider->pPath->getDayCount().getTerm(asofdate, actiondates[i]));
			dataProvider->actiondates.push_back(actiondates[i]);
		}
	}

	if (dataProvider->actiondates.size() == 0)
	{
		clearLSMCPointers(*dataProvider);
		dataProvider->lsmcnum = 0;
		return dataProvider;	
	}

	//LSMC num
	dh = &(object.getData(PRICING_DATA_LSMCNUM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->lsmcnum = dynamic_cast<const AQLDataInt&>(dh->get()).get();
		if (dataProvider->mcnum != 0 && dataProvider->lsmcnum > dataProvider->mcnum)
		{
			//error
			throw AQLCoreInvalidData("LSMCNum must be same or less than MCNum", __FILE__, __LINE__);	
		}
	}
	else
		dataProvider->lsmcnum = 0;
	
	if (dataProvider->lsmcnum == 0 || isTradeValue)
	{
		//coefficient
		dh = &objHolder.getData(PRICING_DATA_COEFFICIENTS, ISNOTNULL);
		dataProvider->coefficient = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());
//		dataProvider->payoff.setLSMCCoefficient(dataProvider->coefficient->get());
		dh = &objHolder.getData(PRICING_DATA_COEFFICIENTS2, ISNOTNULL);
		dataProvider->coefficient_rebate = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

		// standardization
		dh = &objHolder.getData(PRICING_DATA_ISCONVERT_XY, ISNOTNULL);
		dataProvider->isconvert_xy = &dynamic_cast<AQLDataBools&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_Y, ISNOTNULL);
		dataProvider->shift_y = &dynamic_cast<AQLDataDoubles&>(dh->get());
		dh = &objHolder.getData(PRICING_DATA_SCALE_Y, ISNOTNULL);
		dataProvider->scale_y = &dynamic_cast<AQLDataDoubles&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_X, ISNOTNULL);
		dataProvider->shift_x = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());
		dh = &objHolder.getData(PRICING_DATA_SCALE_X, ISNOTNULL);
		dataProvider->scale_x = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_ISCONVERT_XY2, ISNOTNULL);
		dataProvider->isconvert_xy_rebate = &dynamic_cast<AQLDataBools&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_Y2, ISNOTNULL);
		dataProvider->shift_y_rebate = &dynamic_cast<AQLDataDoubles&>(dh->get());
		dh = &objHolder.getData(PRICING_DATA_SCALE_Y2, ISNOTNULL);
		dataProvider->scale_y_rebate = &dynamic_cast<AQLDataDoubles&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_X2, ISNOTNULL);
		dataProvider->shift_x_rebate = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());
		dh = &objHolder.getData(PRICING_DATA_SCALE_X2, ISNOTNULL);
		dataProvider->scale_x_rebate = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

	}
	else
	{
		//coefficient
		dh = &objHolder.getData(PRICING_DATA_COEFFICIENTS, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_COEFFICIENTS, new AQLDataDoubleMatrix());
		dataProvider->coefficient = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());
		
		dh = &objHolder.getData(PRICING_DATA_COEFFICIENTS2, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_COEFFICIENTS2, new AQLDataDoubleMatrix());
		dataProvider->coefficient_rebate = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

		// standardization
		dh = &objHolder.getData(PRICING_DATA_ISCONVERT_XY, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_ISCONVERT_XY, new AQLDataBools());
		dataProvider->isconvert_xy = &dynamic_cast<AQLDataBools&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_Y, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SHIFT_Y, new AQLDataDoubles());
		dataProvider->shift_y = &dynamic_cast<AQLDataDoubles&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SCALE_Y, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SCALE_Y, new AQLDataDoubles());
		dataProvider->scale_y = &dynamic_cast<AQLDataDoubles&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_X, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SHIFT_X, new AQLDataDoubleMatrix());
		dataProvider->shift_x = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SCALE_X, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SCALE_X, new AQLDataDoubleMatrix());
		dataProvider->scale_x = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

		// rebate
		dh = &objHolder.getData(PRICING_DATA_ISCONVERT_XY2, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_ISCONVERT_XY2, new AQLDataBools());
		dataProvider->isconvert_xy_rebate = &dynamic_cast<AQLDataBools&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_Y2, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SHIFT_Y2, new AQLDataDoubles());
		dataProvider->shift_y_rebate = &dynamic_cast<AQLDataDoubles&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SCALE_Y2, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SCALE_Y2, new AQLDataDoubles());
		dataProvider->scale_y_rebate = &dynamic_cast<AQLDataDoubles&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SHIFT_X2, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SHIFT_X2, new AQLDataDoubleMatrix());
		dataProvider->shift_x_rebate = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

		dh = &objHolder.getData(PRICING_DATA_SCALE_X2, NOCHECK);
		if (!dh->isDefined())
			dh = &objHolder.add(PRICING_DATA_SCALE_X2, new AQLDataDoubleMatrix());
		dataProvider->scale_x_rebate = &dynamic_cast<AQLDataDoubleMatrix&>(dh->get());

	}

	//polynomial
	dh = &(objHolder.getData(PRICING_DATA_POLYNOMIAL, ISNOTNULL));
	AQLFunctionBase& poly = dynamic_cast<AQLPriceDataFunction&>(dh->get()).getFunction();
	if (!poly.isTypeOf(FN_POLYNOMIALBASE))
	{
		//error
		throw AQLCoreInvalidData("Not Polynomial function", __FILE__, __LINE__);	
	}
	dataProvider->poly = &dynamic_cast<AQLPolynomialBase&>(poly);

	dataProvider->polies.resize(dataProvider->actiondates.size());
	dataProvider->polies_rebate.resize(dataProvider->actiondates.size());
	DoubleArray param;
	for (unsigned int i = 0; i < dataProvider->polies.size(); i++)
	{
		dataProvider->polies[i] = dynamic_cast<AQLPolynomialBase*>(dataProvider->poly->clone());
		dataProvider->polies_rebate[i] = dynamic_cast<AQLPolynomialBase*>(dataProvider->poly->clone());
		
		if (dataProvider->lsmcnum == 0 || isTradeValue)
		{
			param = dataProvider->coefficient->get()[i];
			param.resize(param.size() - 1);
			for (unsigned int j = 0; j < param.size(); j++)
				param[j] = -param[j];
			dataProvider->polies[i]->setParam(param);

			param = dataProvider->coefficient_rebate->get()[i];
			param.resize(param.size() - 1);
			for (unsigned int j = 0; j < param.size(); j++)
				param[j] = -param[j];
			dataProvider->polies_rebate[i]->setParam(param);

			// standardization
			dataProvider->polies[i]->setConvertFlag(dataProvider->isconvert_xy->get()[i]);
			const double shift_y = dataProvider->shift_y->get()[i];
			const double scale_y = dataProvider->scale_y->get()[i];
			const DoubleArray *shift_x = &(dataProvider->shift_x->get()[i]);
			const DoubleArray *scale_x = &(dataProvider->scale_x->get()[i]);
			dataProvider->polies[i]->setStandardizationParam(shift_y, scale_y, *shift_x, *scale_x);

			dataProvider->polies_rebate[i]->setConvertFlag(dataProvider->isconvert_xy_rebate->get()[i]);
			const double shift_y_rebate = dataProvider->shift_y_rebate->get()[i];
			const double scale_y_rebate = dataProvider->scale_y_rebate->get()[i];
			const DoubleArray *shift_x_rebate = &(dataProvider->shift_x_rebate->get()[i]);
			const DoubleArray *scale_x_rebate = &(dataProvider->scale_x_rebate->get()[i]);
			dataProvider->polies_rebate[i]->setStandardizationParam(shift_y_rebate, scale_y_rebate, *shift_x_rebate, *scale_x_rebate);
		}
	}

    //exclude outlier
    dh = &(object.getData(PRICING_DATA_ISEXCLUDEOUTLIER_Y, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isexcludeoutlier_y = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isexcludeoutlier_y = false;

    dh = &(object.getData(PRICING_DATA_ISEXCLUDEOUTLIER_X, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isexcludeoutlier_x = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isexcludeoutlier_x = false;

    dh = &(object.getData(PRICING_DATA_ISEXCLUDEOUTLIER_E, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isexcludeoutlier_e = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isexcludeoutlier_e = false;

    dh = &(object.getData(PRICING_DATA_CRITERIA_Y, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->criteria_y = dynamic_cast<AQLDataDouble&>(dh->get()).get();
    else
        dataProvider->criteria_y = 0.;

    dh = &(object.getData(PRICING_DATA_CRITERIA_X, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->criteria_x = dynamic_cast<AQLDataDouble&>(dh->get()).get();
    else
        dataProvider->criteria_x = 0.;

    dh = &(object.getData(PRICING_DATA_CRITERIA_E, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->criteria_e = dynamic_cast<AQLDataDouble&>(dh->get()).get();
    else
        dataProvider->criteria_e = 0.;
	//shift
	dh = &(objHolder.getData(PRICING_DATA_ISSHIFT_Y, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isshift_y = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isshift_y = false;

    dh = &(objHolder.getData(PRICING_DATA_ISSHIFT_X, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isshift_x = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isshift_x = false;
    //scale
	dh = &(objHolder.getData(PRICING_DATA_ISSCALE_Y, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isscale_y = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isscale_y = false;

    dh = &(objHolder.getData(PRICING_DATA_ISSCALE_X, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	    dataProvider->isscale_x = dynamic_cast<AQLDataBool&>(dh->get()).get();
    else
        dataProvider->isscale_x = false;
	//SVD Tolerance (a tolerance of calculated eigen value in SVD decomposition) 
	dh = &(objHolder.getData(PRICING_DATA_SVDTOLERANCE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		double tolerance = dynamic_cast<AQLDataDouble&>(dh->get()).get();
		if (tolerance < 1.E-13)
			throw AQLCoreInvalidData("SVD Tolerance is too small.", __FILE__, __LINE__);	
	    dataProvider->svdTolerance = tolerance;
	}
    else
        dataProvider->svdTolerance = 1.E-13;

	return dataProvider;	
}

#endif
/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLPriceTradeValue::createNewDataProvider() const
{
	AQLPriceTradeValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLPriceTradeValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}


#ifndef VISUAL_STUDIO_2010_ANALYTICS

/*!
	@brief value by backward

	@param[in] time cf time of non call case
	@param[in] cf cf of non call case
	@param[in, out] triggerhit trigger call hit information
	@param[in] rebate rebate
	@param[in] explanatory explanatory of LSMC
	@param[in] dataProvider cache object
	@param[out] call value for each action times

	@return dirty price	
*/
double
AQLPriceTradeValue::value_backward(const DoubleMatrix& time,
								const DoubleMatrix& cf,
								vector<pair<unsigned int, AQLDate> >& triggerhit,
								const DoubleArray& rebate,
								const DoubleMatrix& explanatory,
								const AQLDataProvider* dp,
								DoubleVector &callval,
								BoolVector* afterjudge,
								BoolVector* judge,
								DoubleVector* explainedvar) const
{
	callval.clear();
	const AQLPriceTradeValueDataProvider* dataProvider = dynamic_cast<const AQLPriceTradeValueDataProvider*>(dp);

	vector<DoubleArray::const_iterator> it_time(time.size()), it_cf(cf.size());
	for (unsigned int k = 0; k < time.size(); k++)
	{
		it_time[k] = time[k].end();	
		it_cf[k] = cf[k].end();
//		it_time[k]--;
//		it_cf[k]--;
	}				
	
	double pv = 0.0, pv_predict = 0.0, rebate_predict = 0.0;
	DoubleArray explained(time.size(), 0);
	DoubleArray pv_vec(time.size());
	DoubleArray nu_vec(time.size(), 1.0);
	bool is_ITM;
	int calltiming = -1;
	

	int i = dataProvider->actiontimes.size() - 1;
	callval.resize(i + 1, 0.0);
	for (; i >= 0; i--)//i:action time suffix
	{
		double numeraire_expiry = (*dataProvider->pNumeraire)(dataProvider->expirytimes[i]);
		
		if (i != (int)dataProvider->actiontimes.size() - 1)
		{
			double numeraire = numeraire_expiry / (*dataProvider->pNumeraire)(dataProvider->expirytimes[i + 1]);
			pv *= numeraire;
			callval[i] = callval[i + 1] * numeraire;
			
			pv_predict *= numeraire; 
		}

		for (unsigned int k = 0; k < time.size(); k++)//k:leg suffix
		{
			pv_vec[k] = 0;
			for(; it_time[k] != time[k].begin() && *(it_time[k] - 1) > dataProvider->actiontimes[i]; it_time[k]--)
			{
				it_cf[k]--;
				pv_vec[k] = pv_vec[k] * (*dataProvider->pNumeraire)(*(it_time[k] - 1)) / nu_vec[k] + (*it_cf[k]);
				nu_vec[k] = (*dataProvider->pNumeraire)(*(it_time[k] - 1));				
			}
			pv_vec[k] *= numeraire_expiry / nu_vec[k];
			nu_vec[k] = numeraire_expiry;

			if (dataProvider->rcvpay[k] && !dataProvider->isstartable) { pv += pv_vec[k]; pv_predict += pv_vec[k]; callval[i] += pv_vec[k]; }
			else if (!dataProvider->rcvpay[k] && dataProvider->isstartable) { pv += pv_vec[k]; pv_predict += pv_vec[k]; callval[i] += pv_vec[k]; }
			else if (dataProvider->rcvpay[k] && dataProvider->isstartable) { pv -= pv_vec[k]; pv_predict -= pv_vec[k]; callval[i] -= pv_vec[k]; }
			else { pv -= pv_vec[k]; pv_predict -= pv_vec[k]; callval[i] -= pv_vec[k]; }
		}
		
#ifdef __ITM_CHECK__
		if (!dataProvider->isstartable && pv_predict >= rebate[i])
		{
			is_ITM = false;
			if (explainedvar != 0) (*explainedvar)[i] = rebate[i];
		}
		else if (dataProvider->isstartable && pv_predict >= -rebate[i])
		{
			is_ITM = false;
			if (explainedvar != 0) (*explainedvar)[i] = -rebate[i];
		}
		else
		{
			is_ITM = true;
			if (explainedvar != 0) (*explainedvar)[i] = pv;
		}
#else
			is_ITM = true;
			if (explainedvar != 0) (*explainedvar)[i] = pv;
#endif

		if (explanatory[i].size() == 0)
		{
			pv_predict = 0.0;
			rebate_predict = rebate[i];
		}
		else
		{
			dataProvider->polies[i]->setNumVar(explanatory[i].size());
			dataProvider->polies_rebate[i]->setNumVar(explanatory[i].size());
			pv_predict = (*dataProvider->polies[i])(explanatory[i]);
			rebate_predict = (*dataProvider->polies_rebate[i])(explanatory[i]);
		}

		if (judge != 0) (*judge)[i] = false;
		if (afterjudge != 0) (*afterjudge)[i] = false;
		if (is_ITM)
		{
			
			if (afterjudge != 0)
			{
				if ((!dataProvider->isstartable && pv < rebate[i])
					|| (dataProvider->isstartable && pv < -rebate[i]))
					 (*afterjudge)[i] = true;			
			}			
					
			
			if (!dataProvider->isstartable && pv_predict < rebate_predict)
			{
				pv = rebate[i];
				pv_predict = pv;
				calltiming = i;
				if (judge != 0) (*judge)[i] = true;
			}
			else if (dataProvider->isstartable && pv_predict < -rebate_predict)
			{
				pv = -rebate[i];
				pv_predict = pv;
				calltiming = i;
				if (judge != 0) (*judge)[i] = true;
			}
	
			callval[i] = pv / numeraire_expiry;
		}
	}

	// distribute accumulated call value to each action times
	if (dataProvider->expirytimes.size() != 0)
	{
		pv /= (*dataProvider->pNumeraire)(dataProvider->expirytimes[0]);
		//int acSize = dataProvider->actiontimes.size();
		//DoubleVector tmp(acSize, 0.0);
		//for (int i = 0; i < acSize; i++)//i:action time suffix
		//	tmp[i] = (callval[i] - callval[i + 1]);
		//callval = tmp;
	}

	for (unsigned int k = 0; k < time.size(); k++)//k:leg suffix
	{
		pv_vec[k] = 0;
		for(; it_time[k] != time[k].begin() && *(it_time[k] - 1) > dataProvider->settle; it_time[k]-- )
		{
			it_cf[k]--;
			double numeraire = (*dataProvider->pNumeraire)(*(it_time[k] - 1));
			pv_vec[k] = pv_vec[k] * numeraire / nu_vec[k] + (*it_cf[k]);
			nu_vec[k] = numeraire;
		}
		pv_vec[k] /= nu_vec[k];
		if (dataProvider->rcvpay[k] && !dataProvider->isstartable) { pv += pv_vec[k]; callval[0] += pv_vec[k]; }
		else if (!dataProvider->rcvpay[k] && dataProvider->isstartable) { pv += pv_vec[k]; callval[0] += pv_vec[k]; }
		else if (dataProvider->rcvpay[k] && dataProvider->isstartable) { pv -= pv_vec[k]; callval[0] -= pv_vec[k]; }
		else { pv -= pv_vec[k]; callval[0] -= pv_vec[k]; }
	}			
    if (dataProvider->isstartable) pv = -pv;
	
	if (calltiming != -1)
	{
		vector<pair<unsigned int, AQLDate> >::iterator it = triggerhit.begin();
		while (it != triggerhit.end())
		{
			if (it->second > dataProvider->actiondates[calltiming])
				it = triggerhit.erase(it);
			else
				it++;
		}
		triggerhit.push_back(pair<int, AQLDate>(0, dataProvider->actiondates[calltiming]));
	}
	
	return pv;
}
/*!
	@brief calc value term ratio

	@param[in] dataProvider
	@return BasisDF Ratio(ValueTerm against BaseTerm)
*/
double
AQLPriceTradeValue::calcValueTermRatio(const AQLPriceTradeValueDataProvider &dataProvider) const
{
	// valuedate calc
	double valueTerm = dataProvider.pPath->getDayCount().getTerm(dataProvider.pPath->getAsOfDate().get(), dataProvider.valueDate);
	const AQLStringVector& names = dataProvider.pPath->getSDEAttrNames().get();
	int num = -1;
	for (unsigned int i = 0; i < names.size(); i++)
	{
		const AQLMathAttrSDE& sde = dynamic_cast<const AQLMathAttrSDE&>(dataProvider.pPath->getData(names[i], ISNOTNULL).get());
		AQLString ccy = sde.getCurrency();
		AQLString basecur = dataProvider.basecur;
		if (ccy.toUpper() == basecur.toUpper())
		{
			num = i;
			break;
		}
	}
	if (num == -1)
	{
		throw AQLCoreInvalidData("sdeccy is not found!", __FILE__, __LINE__);
	}
	const AQLMathYieldCurve& yield = dynamic_cast<const AQLMathYieldCurve& >(dataProvider.pPath->getInitialValues().get(num).get());
	return yield.getBasisDF(dataProvider.baseterm) / yield.getBasisDF(valueTerm);
}

/*!
	@brief get FXEntity

	@param[in] object
	@return FXEntity Ref
*/
const AQLMathFXEntity &
AQLPriceTradeValue::getFXEntity(AQLObject &object) const
{
	AQLDataReference &pathref = dynamic_cast<AQLDataReference &>(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
	AQLMathPathEntity& path = dynamic_cast<AQLMathPathEntity &>(pathref.get().get());
	
	const AQLDataMultiReference& initialrefs = path.getInitialValues();
	const AQLDataStrings& sdenames = path.getSDEAttrNames();
	unsigned int initSize = sdenames.getSize();
	for (unsigned int i = 0; i < initSize; i++)
	{
		const AQLMathAttrSDE &sde = dynamic_cast<const AQLMathAttrSDE &>(path.getData(sdenames[i]).get());
		const AQLString &ccy = sde.getCurrency();

		if (-1 != ccy.findString('/'))
		{
			return dynamic_cast<AQLMathFXEntity &>(initialrefs.get(i).get());
			
		}
	}

	throw AQLCoreInvalidData("Initial FX does not exist",__FILE__,__LINE__);

	

}

/*!
	@brief clear LSMC pointers
	@param[in, out] AQLPriceTradeValueDataProvider
*/
void AQLPriceTradeValue::clearLSMCPointers(AQLPriceTradeValueDataProvider &dataProvider) const
{
	dataProvider.poly = 0;
	dataProvider.coefficient = 0;
	dataProvider.coefficient_rebate = 0;
	dataProvider.shift_y = 0;
	dataProvider.scale_y = 0;
	dataProvider.shift_x = 0;
	dataProvider.scale_x = 0;
	dataProvider.shift_y_rebate = 0;
	dataProvider.scale_y_rebate = 0;
	dataProvider.shift_x_rebate = 0;
	dataProvider.scale_x_rebate = 0;
}

/*!
@brief calculate fee value

@param[in] tradeEntity
@param[out] feePV
@side-effect adding data 	PRICING_DATA_PV_FEE, PRICING_DATA_PVVALUE_FEE, PRICING_DATA_DF_FEE
and stores fee pv(summary) , fee pv (cashflow wise), discount factor used for calculation of fee.

*/
double
AQLPriceTradeValue::calcFeeValueExo(AQLObject &tradeEntity, AQLPriceTradeValueDataProvider* dataProvider) const
{
	double pvFee = 0.;
	AQLDataHolder* dh;
	dh = &tradeEntity.getData(CALIBRATION_DATA_FEEAMOUNTS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const DoubleArray& feeAmounts = dynamic_cast<AQLDataDoubles&>(dh->get()).get();
		size_t feeSize = feeAmounts.size();

		tradeEntity.remove(PRICING_DATA_PV_FEE);
		tradeEntity.add(PRICING_DATA_PV_FEE, new AQLDataDouble(DBL_MAX));

		// ! First temporary input Fail Value.
		// ! PV Values
		tradeEntity.remove(PRICING_DATA_PVVALUE_FEE);
		tradeEntity.add(PRICING_DATA_PVVALUE_FEE, new AQLDataDoubles(DoubleArray(feeSize, DBL_MAX)));
		// ! DF
		tradeEntity.remove(PRICING_DATA_DF_FEE);
		tradeEntity.add(PRICING_DATA_DF_FEE, new AQLDataDoubles(DoubleArray(feeSize, DBL_MAX)));
		// ! Today FX
		tradeEntity.remove(PRICING_DATA_TODAYFX_FEE);
		tradeEntity.add(PRICING_DATA_TODAYFX_FEE, new AQLDataDoubles(DoubleArray(feeSize, DBL_MAX)));

		dh = &tradeEntity.getData(CALIBRATION_DATA_FEEPAYMENTDATES, ISNOTNULL);
		const DateVector& feePaymentDates = dynamic_cast<AQLDataDates&>(dh->get()).get();
		dh = &tradeEntity.getData(CALIBRATION_DATA_FEECURRENCIES, NOCHECK);
		const AQLStringVector& feeCurrencies = dynamic_cast<AQLDataStrings&>(dh->get()).get();

		// size check
		if (!(feePaymentDates.size() == feeSize &&  feeCurrencies.size() == feeSize))
		{
			throw AQLCoreInvalidData("[calcFeeValueVanilla] Size of Data: feeAmounts, feePaymentDates, feeCurrencies, feeDiscountCurves are not the same.", __FILE__, __LINE__);
		}

		DoubleArray feeValues(feeAmounts.size(), 0.);
		DoubleArray discountFactors(feeAmounts.size());
		DoubleArray todayFXRates(feeAmounts.size(), 1.);

		for (size_t i = 0; i < feeAmounts.size(); i++)
		{
			// Set up fee discount curve
			const AQLMathYieldCurve* yieldCurve;
			const AQLDataMultiReference& initialRefs = dataProvider->pPath->getInitialValues();
			const AQLDataStrings& sdeNames = dataProvider->pPath->getSDEAttrNames();
			for (unsigned int j = 0; j < sdeNames.getSize(); j++)
			{
				const AQLMathAttrSDE &sde = dynamic_cast<const AQLMathAttrSDE &>(dataProvider->pPath->getData(sdeNames[j]).get());
				const AQLString &ccy = sde.getCurrency();

				if (ccy == feeCurrencies[i])
				{
					yieldCurve = dynamic_cast<const AQLMathYieldCurve*>(&initialRefs.get(j).get());
				}
			}

			double baseTerm = yieldCurve->getDayCount().getTerm(dataProvider->valueDate, feePaymentDates[i]);
			discountFactors[i] = yieldCurve->getBasisDF(baseTerm);
			feeValues[i] += discountFactors[i] * feeAmounts[i];

			// convert to base currency
			if (feeCurrencies[i] != dataProvider->basecur)
			{
				const AQLMathFXEntity& fxEntity = getFXEntity(tradeEntity);
				double todayFxRate = fxEntity.getRate(feeCurrencies[i], dataProvider->basecur, 0.);

				todayFXRates[i] = todayFxRate;
			}
		}

		// ! Store PVs for printing out
		// ! PV
		pvFee = 0.;
		for (size_t i = 0; i < feeValues.size(); ++i)
		{
			pvFee += todayFXRates[i] * feeValues[i];
		}
		tradeEntity.remove(PRICING_DATA_PV_FEE);
		tradeEntity.add(PRICING_DATA_PV_FEE, new AQLDataDouble(pvFee));

		// ! PV Values
		tradeEntity.remove(PRICING_DATA_PVVALUE_FEE);
		tradeEntity.add(PRICING_DATA_PVVALUE_FEE, new AQLDataDoubles(feeValues));

		// ! DF
		tradeEntity.remove(PRICING_DATA_DF_FEE);
		tradeEntity.add(PRICING_DATA_DF_FEE, new AQLDataDoubles(discountFactors));

		// ! Today FX
		tradeEntity.remove(PRICING_DATA_TODAYFX_FEE);
		tradeEntity.add(PRICING_DATA_TODAYFX_FEE, new AQLDataDoubles(todayFXRates));

	}
	return pvFee;
}

#else
double
AQLPriceTradeValue::value(const AQLDate& basedate, AQLObject& object,
					const AQLDataValuation& att) const
{
	return 0.0;
}

double
AQLPriceTradeValue::value_backward(const DoubleMatrix& time,
								const DoubleMatrix& cf,
								vector<pair<unsigned int, AQLDate> >& triggerhit,
								const DoubleArray& rebate,
								const DoubleMatrix& explanatory,
								const AQLDataProvider* dp,
								DoubleVector &callval,
								BoolVector* afterjudge,
								BoolVector* judge,
								DoubleVector* explainedvar) const
{
	return 0.0;
}

double
AQLPriceTradeValue::value(const AQLDate& basedate, 
						AQLObject& object, 
						AQLDataProvider* dp,
						unsigned int startpathnum) const
{
	return 0.0;
}

double
AQLPriceTradeValue::calcFeeValueExo(AQLObject &tradeEntity, AQLPriceTradeValueDataProvider* dp) const
{
	return 0.0;
}

#endif
AQLPriceTradeValue::AQLPriceTradeValueDataProvider::~AQLPriceTradeValueDataProvider()
{
	for (unsigned int i = 0; i < polies.size(); i++)
	{
		delete polies[i];
		delete polies_rebate[i];
	}
}





