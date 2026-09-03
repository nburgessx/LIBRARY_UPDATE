/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include <numeric>
#include <set>

#include "LALinearRatesSwapTradeValue.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAPricePortfolioValue.h"
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
#include "LAMathIndexEntity.h"
#include "AQLPriceDataFunction.h"
#include "LAMathFXEntity.h"
#include "AQLAlgorithm.h"
#include "LAPricePayOff.h"
#include "LAPricePayOffTool.h"
#include "LAPriceAccruedInterest.h"
#include "LAMathPlainVanillaEntity.h"
#include "AQLBasic.h"
#include "LAMathYieldCurve.h"
#include "AQLPriceDataInterpolation.h"

#include "LAPriceCouponTool.h"
#include "LAMathYieldCurvePro.h"
#include "AQLMatrix.h"
#include "AQLLinearFunc.h"
#include "AQLLinearInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "LAPriceCouponForDigital2.h"
#include "LAMathDateUtilities.h"
#include "LACoreUtility.h"

#include "LALinearRatesOptionValue.h"
#include "AQLPriceDataInterpolation.h"

using namespace std;

#define RCV	"RCV"
#define PAY	"PAY"
#define STARTABLE	"STARTABLE"
#define CANCELABLE	"CANCELABLE"
#define CPN "CPN"
#define CPNCF "CPNCF"
#define DF2 "DF2"
#define STD     "STD"

#define YES_FIXINGFLAG "Y"
#define NO_FIXINGFLAG "N"

#ifndef	CURRENTINDEX
#define CURRENTINDEX		"CURRENTINDEX"
#endif
#ifndef	CLOSESTINDEX
#define CLOSESTINDEX		"CLOSESTINDEX"
#endif
#ifndef	NEXTINDEX
#define NEXTINDEX			"NEXTINDEX"
#endif
#ifndef	PREVIOUSINDEX
#define PREVIOUSINDEX		"PREVIOUSINDEX"
#endif
#ifndef	STARTENDDATEMETHOD
#define STARTENDDATEMETHOD	"STARTENDDATEMETHOD"
#endif
#ifndef	SPOTRATERATIOMETHOD
#define SPOTRATERATIOMETHOD "SPOTRATERATIOMETHOD"
#endif
#ifndef	GRIDTERMRATIOMETHOD
#define GRIDTERMRATIOMETHOD "GRIDTERMRATIOMETHOD"
#endif

#define ANALYTIC "ANALYTIC"
#define SEMIANALYTIC "SEMIANALYTIC"

#define FX_RATE	"FXRATE"

/*!
    @brief constructor
	@param[in] pacc pointer to accured interest calculation class
*/
LALinearRatesSwapTradeValue::LALinearRatesSwapTradeValue(LAPriceAccruedInterest* pacc) :
LAPriceTradeValue(pacc)
{
}
/*!
    @brief copy constructor
	@param[in] v copy source 
*/
LALinearRatesSwapTradeValue::LALinearRatesSwapTradeValue(const LALinearRatesSwapTradeValue& v)
: LAPriceTradeValue(v)
{
}
/*!
    @brief destructor

*/
LALinearRatesSwapTradeValue::~LALinearRatesSwapTradeValue()
{
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
LALinearRatesSwapTradeValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_PLAINVANILLASWAPTRADEVALUE ? true : LAPriceTradeValue::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
AQLCoreFunctionBase*
LALinearRatesSwapTradeValue::clone() const
{
    try 
	{
  		return new LALinearRatesSwapTradeValue(*this);
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
LALinearRatesSwapTradeValue::getType() const
{
	return FN_IR_PLAINVANILLASWAPTRADEVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LALinearRatesSwapTradeValue::registerData(AQLPriceDataManager& dm) const
{
	LAPriceTradeValue::registerData(dm);
	dm.setData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, DATA_BOOL);
}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
LALinearRatesSwapTradeValue::do_value(const AQLDate& basedate, AQLObject& object,
                                         const AQLDataValuation& att) const
{
	AQLDataHolder* dh;

	bool isanalytic = false;
	dh = &(object.getData(PRICING_DATA_ISANALYTIC, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		isanalytic =  dynamic_cast<const AQLDataBool&>(dh->get()).get();
	bool lsanalyticalreadycalced = false;
	dh = &(object.getData(PRICING_DATA_ISANALYTICALREADYCALCED, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		lsanalyticalreadycalced =  dynamic_cast<const AQLDataBool&>(dh->get()).get();
	//analytic mode and already calced then results have already existed
	if (isanalytic && lsanalyticalreadycalced)
		return 0.0;


	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const AQLDataBool&>(dh->get()).get();

	// check nocalc 
	bool isnocalc = false;
    dh = &object.getData(PRICING_DATA_ZEROCALC, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
    	isnocalc = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	if (isnocalc)
	{
		double noCalcResult = 0;
		if (!iscalcrisk)
		{
			object.remove(PRICING_DATA_CLEANPRICE);
			object.remove(PRICING_DATA_DIRTYPRICE);
			object.remove(PRICING_DATA_ACCRUEDINTEREST);
			object.remove(PRICING_DATA_CLEANPRICESQUARE);
			object.remove(PRICING_DATA_CLEANPRICEDEVIATION);
			object.remove(PRICING_DATA_CALLTRIGGERVALUE);
			object.remove(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER);
			object.remove(PRICING_DATA_PV_LEG1);
			object.remove(PRICING_DATA_PV_LEG2);
			object.remove(PRICING_DATA_CURRENCY_LEG1);
			object.remove(PRICING_DATA_CURRENCY_LEG2);
			object.remove(PRICING_DATA_DIRTYPRICE_LEG1CCY);
			object.remove(PRICING_DATA_DIRTYPRICE_LEG2CCY);
			object.remove(PRICING_DATA_TODAYFX_LEG1CCY);
			object.remove(PRICING_DATA_TODAYFX_LEG2CCY);

			object.add(PRICING_DATA_CLEANPRICE, new AQLDataDouble(noCalcResult));
			object.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(noCalcResult));
			object.add(PRICING_DATA_ACCRUEDINTEREST, new AQLDataDouble(noCalcResult));
		}
		return noCalcResult;
	}

	
	bool istraderecalc = false;
	dh = &object.getData(PRICING_DATA_ISRECALCTRADEDATA, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		istraderecalc = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	
	bool issetuppayoff = false;
	dh = &(object.getData(PRICING_DATA_ISSETUPPAYOFF, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		issetuppayoff = dynamic_cast<const AQLDataBool&>(dh->get()).get();

	bool omitnotionalexposure = false;
	dh = &(object.getData(PRICING_DATA_OMITNOTIONALEXPOSURE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		omitnotionalexposure = dynamic_cast<const AQLDataBool&>(dh->get()).get();

	// ! Check renotional
	bool isRenotional = false;
	dh = &( object.getData( CALIBRATION_DATA_UNDERLYINGS, NOCHECK ) );
	if ( dh->isDefined() && !dh->isNull() )
	{
		const AQLDataMultiReference& mr = dynamic_cast< AQLDataMultiReference& >( dh->get() );
		for ( size_t i = 0; i < mr.getSize(); i++ )
		{
			dh = &( mr.get( i ).getData( PRICING_DATA_ISRENOTIONAL, NOCHECK ) );
			if ( dh->isDefined() && !dh->isNull() )
			{
				bool tmpIsRenotional = dynamic_cast< AQLDataBool& >( dh->get() ).get();
				if ( tmpIsRenotional )
				{
					isRenotional = true;
					break;
				}
			}
		}
	}
	if (istraderecalc || issetuppayoff || isRenotional || isanalytic || omitnotionalexposure)
	{
		att.setDataProvider(NULL);
	}

	
	LALinearRatesSwapTradeValueDataProvider* dp = NULL;
	if (att.isNullDataProvider())
		dp = dynamic_cast<LALinearRatesSwapTradeValueDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dp = &dynamic_cast<LALinearRatesSwapTradeValueDataProvider&>(att.getDataProvider());

	dh = &(object.getData(PRICING_DATA_VALUATIONCURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dp->basecur = dynamic_cast<const AQLDataString&>(dh->get()).get();

	LALinearRatesSwapTradeValueDataProvider* dataProvider = dp;
	
	set<LAMathIndexEntity*>::iterator it;
	for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
	{
		// FX object must be setup for each risk scenario
		if(!iscalcrisk || (*it)->getIndexType().get() == FX_RATE)
			(*it)->setUpforPlainVanilla();
	}

	for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
		(*it)->calcIndexforPlainVanilla();

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
	if (istrigger)
	{
		dh = &(object.getData(PRICING_DATA_ISDETAILOUTPUT, ISNOTNULL));
		isdetailoutput = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	}


	
	double pv1 = 0.0;
	double recpv1 = 0.0;
	double paypv1 = 0.0;
	double accruedint1 = 0.0;
	DoubleMatrix time1,cf1;
	vector<pair<unsigned int, AQLDate> > triggerhit;
	DoubleMatrix explanatory(dataProvider->expirytimes.size());
	vector<UintArray> extracfpos;
	double annuity = 0.0;

	bool isMovePrefixingCF = true;
	double prefixingPV = 0.0;

	// param for each cashlet
	unsigned int legnum = dataProvider->paytimes.size();
	DoubleVector tmpval(dataProvider->maxcfnum, 0.0);
	DoubleMatrix avecf1(legnum, tmpval), avepv1(legnum, tmpval); // cf and pv for each cashlet
	double recPrefixingPV1 = 0.0;
	double payPrefixingPV1 = 0.0;

	vector<map<double, double> > extracf_cfmap(legnum, map<double, double>()), extracf_pvmap(legnum, map<double, double>()); // extracf val	
	double numerarireforirr = 1.0;
	//AQLPriceDataInterpolation recDCInter = dataProvider->pVanilla->getIRCurve(dataProvider->reccur).getInterpolation();
	//AQLPriceDataInterpolation payDCInter = dataProvider->pVanilla->getIRCurve(dataProvider->paycur).getInterpolation();
	//const AQLObjectHolder &recYData = dataProvider->pVanilla->getIRCurve(dataProvider->reccur).getYieldData().get();
	//const AQLObjectHolder &payYData = dataProvider->pVanilla->getIRCurve(dataProvider->paycur).getYieldData().get();

	const AQLInterpolationBase *pRecDCInter = 0;
	const AQLInterpolationBase *pPayDCInter = 0;
	const AQLDataHolder *c_ah = 0;
	const AQLString curvestdname = STD; 
	// set numeraire curve
	//if (dataProvider->recDCurve == DF2)
	//{
	//	const AQLObjectHolder &recYData = dataProvider->pVanilla->getIRCurve(dataProvider->reccur).getYieldData().get();
	//	c_ah = &(recYData.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK));
	//	if (c_ah->isDefined() && !c_ah->isNull())
	//	{
	//		pRecDCInter = &(dataProvider->pVanilla->getIRCurve(dataProvider->reccur).getBasisDFInterpolation());
	//	}
	//	else
	//	{
	//		// if DF2 is not exist get DF
	//		pRecDCInter = &(dataProvider->pVanilla->getIRCurve(dataProvider->reccur).getDFInterpolation(&curvestdname));
	//	}
	//}
	//else
	//{
	pRecDCInter = &(dataProvider->pVanilla->getIRCurve(dataProvider->reccur).getDFInterpolation(&dataProvider->recDCurve));
	//}
	//if (dataProvider->payDCurve == DF2)
	//{
	//	const AQLObjectHolder &payYData = dataProvider->pVanilla->getIRCurve(dataProvider->paycur).getYieldData().get();
	//	c_ah = &(payYData.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK));
	//	if (c_ah->isDefined() && !c_ah->isNull())
	//	{
	//		pPayDCInter = &(dataProvider->pVanilla->getIRCurve(dataProvider->paycur).getBasisDFInterpolation());
	//	}
	//	else
	//	{
	//		// if DF2 is not exist get DF
	//		pPayDCInter = &(dataProvider->pVanilla->getIRCurve(dataProvider->paycur).getDFInterpolation(&curvestdname));

	//	}

	//}
	//else
	//{
	pPayDCInter = &(dataProvider->pVanilla->getIRCurve(dataProvider->paycur).getDFInterpolation(&dataProvider->payDCurve));
	//}

	// base date numeraire
	const double recNumeraire_b = pRecDCInter->value(dataProvider->baseterm);
	const double payNumeraire_b = pPayDCInter->value(dataProvider->baseterm);
	//double numeraire_b = numcurve.getBasisDF(dataProvider->baseterm);

	//from now value class
	if (dataProvider->acc.isDefined()) //calculate accurued interest
	{
		
		const LAPriceAccruedInterest& acc = dynamic_cast<const LAPriceAccruedInterest&>(dataProvider->acc.get());
		
		
		if (dataProvider->rcvpay[0]) accruedint1 = acc.calcAccruedInterest(dataProvider->settledate, 0);
			else accruedint1 = -acc.calcAccruedInterest(dataProvider->settledate, 0);


		if (dataProvider->settle != dataProvider->baseterm)
		{
			if (dataProvider->rcvpay[0])
			{
				accruedint1 *= pRecDCInter->value(dataProvider->baseterm) / pRecDCInter->value(dataProvider->settle);
			}
			else
			{
				accruedint1 *= pPayDCInter->value(dataProvider->baseterm) / pPayDCInter->value(dataProvider->settle);
			}
		}

		if (dataProvider->rcvpay[0])
		{
			if (dataProvider->basecur != dataProvider->reccur)
			{
				double fxrate = getFXEntity(object).getRate(dataProvider->reccur, dataProvider->basecur, dataProvider->baseterm);
				accruedint1 *= fxrate;
			}
		}
		else
		{
			if (dataProvider->basecur != dataProvider->paycur)
			{
				double fxrate = getFXEntity(object).getRate(dataProvider->paycur, dataProvider->basecur, dataProvider->baseterm);
				accruedint1 *= fxrate;
			}
		}
		//if (dataProvider->settle != dataProvider->baseterm)
		//	accruedint1 *= numcurve.getBasisDF(dataProvider->baseterm) / numcurve.getBasisDF(dataProvider->settle);
	}

	// calc underlying value
	
	
	dataProvider->payoff.calcPayOff(time1, cf1, true, triggerhit, /*true, */false, 0, 0, &extracfpos);
	//this type is <JPY_STD,term,diffvalue>
	
	std::map<AQLString, std::map<double,double> >mapdfdzerolibor;
	if (dataProvider->isdifferentiation)
	{
		BoolVector isrecVec(time1.size());
		std::vector<const AQLInterpolationBase*> pInterpVec(time1.size());
		
		for (unsigned int j =0; j < time1.size();j++)
		{
			isrecVec[j] = dataProvider->rcvpay[j];
			pInterpVec[j] = (dataProvider->rcvpay[j]) ? pRecDCInter : pPayDCInter; 
		}
		dataProvider->payoff.calcDerivationOfLibor(mapdfdzerolibor, pInterpVec, isrecVec);
	}

	const vector<PayOffToolHolderVector> &payOff = dataProvider->payoff.getPayOff();


	for (unsigned int j = 0; j < time1.size(); j++)
	{
		double sign = 1.0;
		double numeraire_b = recNumeraire_b;
		const AQLInterpolationBase *pDCInter = pRecDCInter;
		double *p_pv1 = &recpv1;
		double *p_prefixingPV1 = &recPrefixingPV1;
		if (!dataProvider->rcvpay[j])
		{
			sign = -1.0;
			numeraire_b = payNumeraire_b;
			pDCInter = pPayDCInter;
			p_pv1 = &paypv1;
			p_prefixingPV1 = &payPrefixingPV1;

		}

	
		//get leg object
		BoolVector isFixedCF = BoolVector(time1[j].size(), true);
		AQLDataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
		AQLDataMultiReference* legs = dynamic_cast< AQLDataMultiReference* >(&(dh->get()));

		AQLObject& leg = legs->get(j).get();
		dh = &leg.getData(PRICING_DATA_IS_FIXED_CF, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			isFixedCF = dynamic_cast<AQLDataBools&>(dh->get()).get();
			if (isFixedCF.size() != time1[j].size())
			{
				throw AQLCoreInvalidData("PRICING_DATA_IS_FIXED_CF size is different from that of cashflows", __FILE__, __LINE__);
			}
		}


		std::map<AQLString, double>* pcurvemap=0;
		AQLString diffkey;
		AQLString diffcur;
		if (dataProvider->isdifferentiation) 
		{

			AQLString curvetype = (dataProvider->rcvpay[j]) ? dataProvider->recDCurve : dataProvider->payDCurve;
			diffcur = (dataProvider->rcvpay[j]) ? dataProvider->reccur : dataProvider->paycur;
			//be careful about name;
			/*if (curvetype == "DF2")
			{
				const AQLObjectHolder &ydata = dataProvider->pVanilla->getIRCurve(diffcur).getYieldData().get();
				const AQLDataHolder& chkattr = ydata.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
				if (chkattr.isDefined() && !chkattr.isNull())
					curvetype = dynamic_cast<AQLDataString &>(dataProvider->pVanilla->getIRCurvePro(diffcur).getData(IR_CALIBRATION_DATA_MAINBASISDF,ISNOTNULL).get());
				else
					curvetype = "STD";
			}*/
			//pcurvemap = &(dataProvider->pVanilla->getIRCurvePro(diffcur).getCurveDependeny(curvetype));
			
		}

		//for irr swaption
		bool isfirstcf = true;
		if (extracfpos[j].empty())
		{
			for (unsigned int k = 0; k < time1[j].size(); k++)
			{
				
				if (time1[j][k] > dataProvider->settle)
				{
					//double numeraire = numcurve.getBasisDF(time1[j][k]);
					double numeraire = pDCInter->value(time1[j][k]);
					//pv1 +=  sign * cf1[j][k] * numeraire;
					*p_pv1 +=  sign * cf1[j][k] * numeraire;
					if (!isFixedCF[k])
					{
						*p_prefixingPV1 += sign * cf1[j][k] * numeraire;
					}
					else  if (k + 1 < time1[j].size() && !isFixedCF[k + 1])
					{
						// first renotional case
						double next_notional = payOff[j][k + 1].getPayOff().mNotional;
						*p_prefixingPV1 -= sign * next_notional * numeraire;
					}

					avecf1[j][k] = sign * cf1[j][k];
					avepv1[j][k] = (sign / numeraire_b * cf1[j][k]) * numeraire;

					if (dataProvider->isannuitycalc && j == dataProvider->annuityLegNo)
					{
						std::map<unsigned int, double >::iterator it = dataProvider->annuitymap.find(k);
						if (it != dataProvider->annuitymap.end())
							annuity += numeraire / numeraire_b * it->second;

						//irr swaption
						if (dataProvider->isirrmodel && isfirstcf)
						{
							numerarireforirr = pDCInter->value(dataProvider->fwdswapterm);
							numerarireforirr /= numeraire_b; 
							isfirstcf = false;
						}
					}
					if (dataProvider->isdifferentiation)
					{
						double paytime = time1[j][k];
						std::map<AQLString, double>::iterator itcmap;
						for (itcmap = pcurvemap->begin(); itcmap != pcurvemap->end(); ++itcmap)
						{
							diffkey = diffcur.toUpper() + "_" + itcmap->first;
							double direction = itcmap->second;
							mapdfdzerolibor[diffkey][paytime] += (-paytime) * avepv1[j][k] * direction;
						}
					}

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
					//double numeraire = numcurve.getBasisDF(time1[j][k]);
					double numeraire = pDCInter->value(time1[j][k]);
					//pv1 += sign * cf1[j][k] * numeraire;
					*p_pv1 += sign * cf1[j][k] * numeraire;
					if (!isFixedCF[k])
					{
						*p_prefixingPV1 += sign * cf1[j][k] * numeraire;
					}
					else  if (k + 1 < time1[j].size() && !isFixedCF[k + 1])
					{
						// first renotional case
						double next_notional = payOff[j][k + 1].getPayOff().mNotional;
						*p_prefixingPV1 -= sign * next_notional * numeraire;
					}

					unsigned int expos;
					if (!AQLAlgorithm::find<UintArray, unsigned int>(extracfpos[j], k, 0, extracfpos[j].size() - 1, expos))
					{
						avecf1[j][cf_index] = sign * cf1[j][k];
						avepv1[j][cf_index] = (sign / numeraire_b * cf1[j][k]) * numeraire;
						++cf_index;
					}
					else
					{
						double paytime = time1[j][k];
						extracf_cfmap[j][paytime] = sign * cf1[j][k];
						extracf_pvmap[j][paytime] = (sign / numeraire_b * cf1[j][k]) * numeraire;
					}

					if (dataProvider->isannuitycalc && j == dataProvider->annuityLegNo)
					{
						std::map<unsigned int, double >::iterator it = dataProvider->annuitymap.find(k);
						if (it != dataProvider->annuitymap.end())
							annuity += numeraire / numeraire_b * it->second;

						//irr swaption
						if (dataProvider->isirrmodel && isfirstcf)
						{
							numerarireforirr = pDCInter->value(dataProvider->fwdswapterm);
							numerarireforirr /= numeraire_b; 
							isfirstcf = false;
						}
					}
					
					if (dataProvider->isdifferentiation)
					{
						double paytime = time1[j][k];
						std::map<AQLString, double>::iterator itcmap;
						for (itcmap = pcurvemap->begin(); itcmap != pcurvemap->end(); ++itcmap)
						{
							diffkey = diffcur.toUpper() + "_" + itcmap->first;
							double direction = itcmap->second;
							mapdfdzerolibor[diffkey][paytime] += (-paytime) * (sign / numeraire_b * cf1[j][k]) * numeraire * direction;
						}
					}
				}
			}
		}
	}
	recpv1 /= recNumeraire_b;
	paypv1 /= payNumeraire_b;
	recPrefixingPV1 /= recNumeraire_b;
	payPrefixingPV1 /= payNumeraire_b;

	//leg pv
	double leg1val = (dataProvider->rcvpay[0]) ? recpv1 : paypv1;
	double leg2val = (dataProvider->rcvpay[0]) ? paypv1 : recpv1;
	double leg1valPrefixingCFAdjustment, leg2valPrefixingCFAdjustment;

	AQLString leg1cur = (dataProvider->rcvpay[0]) ? dataProvider->reccur : dataProvider->paycur;
	AQLString leg2cur = (dataProvider->rcvpay[0]) ? dataProvider->paycur : dataProvider->reccur;

	double ccy1todayfx = 1.0;
	double ccy2todayfx = 1.0;
	double rectodayfx = 1.0;
	double paytodayfx = 1.0;

	if (dataProvider->basecur != dataProvider->reccur)
	{
		rectodayfx = getFXEntity(object).getRate(dataProvider->reccur, dataProvider->basecur, dataProvider->baseterm);
		recpv1 *= rectodayfx;
		recPrefixingPV1 *= rectodayfx;
		if (leg1cur == dataProvider->reccur)
		{
			ccy1todayfx = rectodayfx;
		}
		if (leg2cur == dataProvider->reccur)
		{
			ccy2todayfx = rectodayfx;
		}
		for (unsigned int i= 0; i < legnum; i++)
		{
			if (dataProvider->rcvpay[i])
			{
				/*DoubleArray fx_cf_multiple(avecf1[i].size(), fxrate);
				transform(avecf1[i].begin(), avecf1[i].end(), fx_cf_multiple.begin(), avecf1[i].begin(), multiplies<double>());
				transform(avepv1[i].begin(), avepv1[i].end(), fx_cf_multiple.begin(), avepv1[i].begin(), multiplies<double>());*/
			}
		}
	}
	if (dataProvider->basecur != dataProvider->paycur)
	{
		paytodayfx = getFXEntity(object).getRate(dataProvider->paycur, dataProvider->basecur, dataProvider->baseterm);
		paypv1 *= paytodayfx;
		payPrefixingPV1 *= paytodayfx;
		if (leg1cur == dataProvider->paycur)
		{
			ccy1todayfx = paytodayfx;
		}
		if (leg2cur == dataProvider->paycur)
		{
			ccy2todayfx = paytodayfx;
		}
		for (unsigned int i= 0; i < legnum; i++)
		{
			if (!dataProvider->rcvpay[i])
			{
				/*DoubleArray fx_cf_multiple(avecf1[i].size(), fxrate);
				transform(avecf1[i].begin(), avecf1[i].end(), fx_cf_multiple.begin(), avecf1[i].begin(), multiplies<double>());
				transform(avepv1[i].begin(), avepv1[i].end(), fx_cf_multiple.begin(), avepv1[i].begin(), multiplies<double>());*/
			}
		}
	}

	// Calculate Fee Value
	double pvFee = calcFeeValueVanilla(object, dataProvider->basecur);
	
	// Adjust prefixing CF
	if (isMovePrefixingCF)
	{
		double adjustmentInBaseCcy = -recPrefixingPV1 + payPrefixingPV1;
		leg1valPrefixingCFAdjustment = (dataProvider->rcvpay[0]) ? (adjustmentInBaseCcy) / rectodayfx : (-adjustmentInBaseCcy) / paytodayfx;
		leg2valPrefixingCFAdjustment = (dataProvider->rcvpay[0]) ? (-adjustmentInBaseCcy) / paytodayfx : (adjustmentInBaseCcy) / rectodayfx;
	}

	pv1 = recpv1 + paypv1;
	//pv1 /= numeraire_b;
	//////////
	//output//
	//////////
	double cleanprice1 = pv1 - accruedint1;
	double dirtyprice1 = pv1;

	// check option holder
	if (!dataProvider->isoptionholder)
	{
		cleanprice1 *= -1.0;
		dirtyprice1 *= -1.0;
	}

	double feeExcludedPV = dirtyprice1;
	cleanprice1 += pvFee;
	dirtyprice1 += pvFee;

	double ccy1dirtyprice = dirtyprice1;
	double ccy2dirtyprice = dirtyprice1;
	if (leg1cur != dataProvider->basecur)
	{
		double fxrate = getFXEntity(object).getRate(dataProvider->basecur, leg1cur, dataProvider->baseterm);
		ccy1dirtyprice *= fxrate;
	}
	
	if (leg2cur != dataProvider->basecur)
	{
		double fxrate = getFXEntity(object).getRate(dataProvider->basecur, leg2cur, dataProvider->baseterm);
		ccy2dirtyprice *= fxrate;
	}

	// output currency change for NDS
	double fx_settle_nondeliv;
	if(dataProvider->isnondeliverableLeg.size() == 2)
	{
		if(dataProvider->isnondeliverableLeg[0])
		{
			fx_settle_nondeliv = getFXEntity(object).getRate(leg1cur, dataProvider->originalcur, dataProvider->baseterm);
			leg1val *= fx_settle_nondeliv;
			leg1cur = dataProvider->originalcur;
			ccy1todayfx = getFXEntity(object).getRate(leg1cur, dataProvider->basecur, dataProvider->baseterm);
			ccy1dirtyprice = dirtyprice1 * getFXEntity(object).getRate(dataProvider->basecur, leg1cur, dataProvider->baseterm);
		}
		else if(dataProvider->isnondeliverableLeg[1])
		{
			fx_settle_nondeliv = getFXEntity(object).getRate(leg2cur, dataProvider->originalcur, dataProvider->baseterm);
			leg2val *= fx_settle_nondeliv;
			leg2cur = dataProvider->originalcur;
			ccy2todayfx = getFXEntity(object).getRate(leg2cur, dataProvider->basecur, dataProvider->baseterm);
			ccy2dirtyprice = dirtyprice1 * getFXEntity(object).getRate(dataProvider->basecur, leg2cur, dataProvider->baseterm);
		}
	}

	if (!iscalcrisk)
	{
		object.remove(PRICING_DATA_CLEANPRICE);
		object.remove(PRICING_DATA_DIRTYPRICE);
		object.remove(PRICING_DATA_ACCRUEDINTEREST);
		object.remove(PRICING_DATA_CLEANPRICESQUARE);
		object.remove(PRICING_DATA_CLEANPRICEDEVIATION);
		object.remove(PRICING_DATA_CALLTRIGGERVALUE);
		object.remove(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER);
		object.remove(PRICING_DATA_PV_LEG1);
		object.remove(PRICING_DATA_PV_LEG2);
		object.remove(PRICING_DATA_CURRENCY_LEG1);
		object.remove(PRICING_DATA_CURRENCY_LEG2);
		object.remove(PRICING_DATA_DIRTYPRICE_LEG1CCY);
		object.remove(PRICING_DATA_DIRTYPRICE_LEG2CCY);
		object.remove(PRICING_DATA_TODAYFX_LEG1CCY);
		object.remove(PRICING_DATA_TODAYFX_LEG2CCY);
		object.remove(PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT);
		object.remove(PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT);
		object.remove(PRICING_DATA_PV);
		object.add(PRICING_DATA_PV, new AQLDataDouble(dirtyprice1));
		object.remove(PRICING_DATA_FEE_EXCLUDED_PV);
		object.add(PRICING_DATA_FEE_EXCLUDED_PV, new AQLDataDouble(feeExcludedPV));

		object.add(PRICING_DATA_CLEANPRICE, new AQLDataDouble(cleanprice1));
		object.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(dirtyprice1));
		object.add(PRICING_DATA_ACCRUEDINTEREST, new AQLDataDouble(accruedint1));
		object.add(PRICING_DATA_PV_LEG1, new AQLDataDouble(leg1val));
		object.add(PRICING_DATA_PV_LEG2, new AQLDataDouble(leg2val));
		object.add(PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT, new AQLDataDouble(leg1valPrefixingCFAdjustment));
		object.add(PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT, new AQLDataDouble(leg2valPrefixingCFAdjustment));

		object.add(PRICING_DATA_CURRENCY_LEG1, new AQLDataString(leg1cur));
		object.add(PRICING_DATA_CURRENCY_LEG2, new AQLDataString(leg2cur));
		object.add(PRICING_DATA_TODAYFX_LEG1CCY, new AQLDataDouble(ccy1todayfx));
		object.add(PRICING_DATA_TODAYFX_LEG2CCY, new AQLDataDouble(ccy2todayfx));
		object.add(PRICING_DATA_DIRTYPRICE_LEG1CCY, new AQLDataDouble(ccy1dirtyprice));
		object.add(PRICING_DATA_DIRTYPRICE_LEG2CCY, new AQLDataDouble(ccy2dirtyprice));
	}

	if (dataProvider->isannuitycalc)
	{
		object.remove(PRICING_DATA_ANNUITYVALUE);
		object.add(PRICING_DATA_ANNUITYVALUE, new AQLDataDouble(annuity));
		object.remove(PRICING_DATA_FIRSTNUMERAIREFORIRR);
		object.add(PRICING_DATA_FIRSTNUMERAIREFORIRR, new AQLDataDouble(numerarireforirr));

	}

	// we stopped giving the analytic risk function on 2012/11/26 for improving performance of the linepricer
	//if (dataProvider->isdifferentiation)
	//{
	//	setAnatlyticIRRiskIntoGrid(object,dataProvider,mapdfdzerolibor);
	//}


//#ifdef XLLAPLI
	bool isResultOut = false;
	dh = &( object.getData( PRICING_DATA_ISRESULTOUTPUT, NOCHECK ) );
	if ( dh->isDefined() && !dh->isNull() )
	{
		isResultOut = dynamic_cast< AQLDataBool& >( dh->get() ).get();
	}

	if ( isResultOut )
	{
		if (!iscalcrisk)
		{
			// ! Get Forwards
			DoubleMatrix gearings( payOff.size() );
			DoubleMatrix forwards( payOff.size() );
			DoubleMatrix spreads( payOff.size() );
			DoubleMatrix fradfs( payOff.size() );
			vector< DateVector > fixingDates( payOff.size() );
			vector< AQLStringVector > fixingFlags( payOff.size() );
			// for EQ-Flows
			DoubleMatrix boundary1( payOff.size() );
			DoubleMatrix boundary2( payOff.size() );
			DoubleMatrix boundary3( payOff.size() );
			DoubleMatrix boundary4( payOff.size() );
			DoubleMatrix couponRate0( payOff.size() );
			DoubleMatrix couponRate1( payOff.size() );
			DoubleMatrix couponRate2( payOff.size() );
			DoubleMatrix couponRate3( payOff.size() );
			DoubleMatrix couponRate4( payOff.size() );
	
			bool isFra = false;
			for ( size_t i = 0; i < payOff.size(); i++ )
			{	
				gearings[i].resize( payOff[i].size() );
				forwards[i].resize( payOff[i].size() );
				spreads[i].resize( payOff[i].size() );
				fradfs[i].resize( payOff[i].size() ); 
				fixingDates[i].resize( payOff[i].size() );
				fixingFlags[i].resize( payOff[i].size() );
				boundary1[i].resize( payOff[i].size() );
				boundary2[i].resize( payOff[i].size() );
				boundary3[i].resize( payOff[i].size() );
				boundary4[i].resize( payOff[i].size() );
				couponRate0[i].resize( payOff[i].size() );
				couponRate1[i].resize( payOff[i].size() );
				couponRate2[i].resize( payOff[i].size() );
				couponRate3[i].resize( payOff[i].size() );
				couponRate4[i].resize( payOff[i].size() );
	
				for ( size_t j = 0; j < payOff[i].size(); j++ )
				{
                    if(payOff[i][j].getPayOff().isCompounding()){
                        fradfs[i][j] = 0;
                        const LAPricePayOffToolCompound* temp = dynamic_cast<const LAPricePayOffToolCompound*>(&payOff[i][j].getPayOff());
                        if(temp==NULL){
                            AQLString msg;
                            msg += "CAST ERROR:";
                            msg += "payoff tool cannot be casted to LAPriceCouponToolCompound";
                            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
                        }
                        temp->setCashflow(gearings[i][j], forwards[i][j], spreads[i][j], fixingDates[i][j], fixingFlags[i][j]);
                        continue;
                    }
					// ! Get index with assumption of no digital coupon.
					vector<LAPriceCouponTool *> coupons = payOff[i][j].getPayOff().mpCoupons;

					if (coupons.size() == 1)
					{
						vector<LAPriceIndexToolBase *> indexs = (*coupons[0]).getIndexs();
						const AQLFunctionBase* fBase = payOff[i][j].getPayOff().mpCoupons[0]->getOperator();
						DoubleArray paramArray = fBase->getParam();

						//gearing
						gearings[i][j] = 0.;
						for (size_t k = 0; k < paramArray.size() - 1; ++k)
						{
							gearings[i][j] += paramArray[k];
						}
						if (indexs.size() == 1)
						{
							if (indexs[0]->isTypeOf(INDEX_TOOL))
							{
								const UintArray &indexPos = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexPos();
								if (!indexPos.empty())
								{
									// get last index value
									forwards[i][j] = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getIndex()[indexPos.back()];
									fixingDates[i][j] = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getGrid()[indexPos.back()];
									fixingFlags[i][j] = NO_FIXINGFLAG;
								}
								else
								{
									forwards[i][j] = 0.;
								}
							}
							else if (indexs[0]->isTypeOf(INDEX_TOOLFIXED))
							{
								forwards[i][j] = dynamic_cast<LAPriceIndexToolFixed *>(indexs[0])->getFixedRate();
								AQLDate* pFixingDate = dynamic_cast<LAPriceIndexToolFixed *>(indexs[0])->getFixingDate();
								if (pFixingDate) 
								{
									fixingDates[i][j] = *pFixingDate;
									fixingFlags[i][j] = YES_FIXINGFLAG;
								}
							}
							else
							{
								forwards[i][j] = 0.;
							}
						}
						else
						{
							if (paramArray.size() != indexs.size() + 1)
							{
								throw AQLCoreInvalidData("Coefficient size and indices size are not same!", __FILE__, __LINE__);		
							}

							if (gearings[i][j] == 0.)
							{
								throw AQLCoreInvalidData("Gearing is 0!", __FILE__, __LINE__);		
							}
							forwards[i][j] = 0.;
							for (size_t k = 0; k < paramArray.size() - 1; k++)
							{
								forwards[i][j] += paramArray[k] * indexs[k]->calcIndex() / gearings[i][j];
							}

							//stubcase
							if (indexs[0]->isTypeOf(INDEX_TOOL))
							{
								const UintArray &indexPos = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexPos();
								if (!indexPos.empty())
								{
									fixingDates[i][j]  = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getGrid()[indexPos.back()];
									fixingFlags[i][j] = NO_FIXINGFLAG;
								}
							}
						}

						if ( paramArray.size() >= 2 )
						{
							spreads[i][j] = paramArray.back();
						}
						else
						{
							spreads[i][j] = 0.;
						}

						if (!indexs.empty())
						{
							if (indexs.back()->isDFRatioUpdate())
							{
								isFra = true;
								fradfs[i][j] = indexs.back()->getDFRatio();
							}
						}
					}
					else
					{
						gearings[i][j] = 0.;
						forwards[i][j] = 0.;
						spreads[i][j] = 0.;
						
						// for digital
						LAPriceCouponBase* opb = payOff[i][j].getPayOff().mpCpnSelectOperator;
						if (coupons.size() == 6 && opb->isTypeOf(FN_CPNSLTOPERATORFORDIGITAL2))
						{
							LAPriceCouponForDigital2* op = dynamic_cast<LAPriceCouponForDigital2*>(opb);
							boundary1[i][j] = op->getParam()[0];
							boundary2[i][j] = op->getParam()[1];
							boundary3[i][j] = op->getParam()[2];
							boundary4[i][j] = op->getParam()[3];
							couponRate0[i][j] = coupons[1]->calcCoupon();
							couponRate1[i][j] = coupons[2]->calcCoupon();
							couponRate2[i][j] = coupons[3]->calcCoupon();
							couponRate3[i][j] = coupons[4]->calcCoupon();
							couponRate4[i][j] = coupons[5]->calcCoupon();

							vector<LAPriceIndexToolBase *> indexs = (*coupons[0]).getIndexs();
							if (indexs.size() == 1)
							{
								if (indexs[0]->isTypeOf(INDEX_TOOL))
								{
									const UintArray &indexPos = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexPos();
									if (!indexPos.empty())
									{
										// get last index value
										fixingDates[i][j] = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getGrid()[indexPos.back()];
										fixingFlags[i][j] = NO_FIXINGFLAG;
									}
								}
								else if (indexs[0]->isTypeOf(INDEX_TOOLFIXED))
								{
									AQLDate* pFixingDate = dynamic_cast<LAPriceIndexToolFixed *>(indexs[0])->getFixingDate();
									if (pFixingDate) 
									{
										fixingDates[i][j] = *pFixingDate;
										fixingFlags[i][j] = YES_FIXINGFLAG;
									}
								}
							}
							else
							{
								//stubcase
								if (indexs[0]->isTypeOf(INDEX_TOOL))
								{
									const UintArray &indexPos = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexPos();
									if (!indexPos.empty())
									{
										fixingDates[i][j]  = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getGrid()[indexPos.back()];
										fixingFlags[i][j] = NO_FIXINGFLAG;
									}
								}
							}
						}
					}
				}
			}


			// ! Get Notionals
			DoubleMatrix notionals( payOff.size() );
			DoubleMatrix notionalcfs( payOff.size() );
			std::vector<DateVector> renotionalfixingdates( payOff.size() );

			for ( size_t i = 0; i < payOff.size(); i++ )
			{
				notionals[i].resize( payOff[i].size() );
				notionalcfs[i].resize( payOff[i].size() );
				renotionalfixingdates[i].resize( payOff[i].size() );

				for( size_t j = 0; j < payOff[i].size(); j++ )
				{
					notionals[i][j] = payOff[i][j].getPayOff().mNotional;
					notionalcfs[i][j] = payOff[i][j].getPayOff().mNotionalCF;
					renotionalfixingdates[i][j] = payOff[i][j].getPayOff().mRenotionalFixingDate;
				}
			}

			// for NDS after modification of ODMS mapping
			BoolVector isNonDeliverable(payOff.size(), false);
			std::vector<DateVector> settlefixingdates(payOff.size());
			DoubleMatrix settleadjustratios(payOff.size());
			for (size_t i = 0; i < payOff.size(); i++)
			{
				settlefixingdates[i].resize(payOff[i].size());
				settleadjustratios[i].resize(payOff[i].size(), 1.0);
				for (size_t j = 0; j < payOff[i].size(); j++)
				{
					if (payOff[i][j].getPayOff().mIsNonDeliverable)
					{
						isNonDeliverable[i] = true;
						settlefixingdates[i][j] = payOff[i][j].getPayOff().mSettlementFixingDate;
						settleadjustratios[i][j] = payOff[i][j].getPayOff().mSettlementAdjustRatio;
					}
				}
			}


			// ! Get DFs
			DoubleMatrix dfs(time1.size());
			for ( size_t i = 0; i < time1.size(); i++ )
			{
				dfs[i].resize(time1[i].size());
				for ( size_t j = 0; j < time1[i].size(); j++ )
				{
					double numer , numer_b;
					if( dataProvider->rcvpay[i] )
					{
						numer = pRecDCInter->value( time1[i][j] );
						numer_b = recNumeraire_b;
					}
					else
					{
						numer = pPayDCInter->value( time1[i][j] );
						numer_b = payNumeraire_b;
					}
	
					dfs[i][j] = numer / numer_b;
				}
			}
		
			// ! Get accrual days, cfCalcStartDates, cfCalcEndDates
			DoubleMatrix accrualDays( payOff.size() );
			DoubleMatrix accrualTerms( payOff.size() );
			vector< DateVector > cfCalcStartDates( payOff.size() );
			vector< DateVector > cfCalcEndDates( payOff.size() );

			for ( size_t i = 0; i < payOff.size(); i++ )
			{
				accrualDays[i].resize( payOff[i].size() );
				accrualTerms[i].resize( payOff[i].size() );
				cfCalcStartDates[i].resize( payOff[i].size() );
				cfCalcEndDates[i].resize( payOff[i].size() );

				for ( size_t j = 0; j < payOff[i].size(); j++ )
				{
					AQLDate accStartDate = payOff[i][j].getPayOff().mStart;
					AQLDate accEndDate = payOff[i][j].getPayOff().mEnd;
	
					accrualDays[i][j] = accStartDate.intervalDays( accEndDate );
					accrualTerms[i][j] = payOff[i][j].getPayOff().mTerm;
					cfCalcStartDates[i][j] = accStartDate;
					cfCalcEndDates[i][j] = accEndDate;
				}
			}

			// ! Get PaymentDates, PaymentTimes
			vector< DateVector > paymentDates( payOff.size() );
			DoubleMatrix paymentTimes( payOff.size() );

			for ( size_t i = 0; i < payOff.size(); i++ )
			{
				paymentDates[i].resize( payOff[i].size() );
				paymentTimes[i].resize( payOff[i].size() );
			
				for ( size_t j = 0; j < payOff[i].size(); j++ )
				{
					paymentDates[i][j] = payOff[i][j].getPayOff().mPaymentDate;
					paymentTimes[i][j] = payOff[i][j].getPayOff().mPaymentTime;
				}
			}



			// ! convexity adjust
			DoubleMatrix convexityAdjusts( payOff.size() );
			DoubleMatrix convexityAdjustVols( payOff.size() );
			bool convexityAdjustFlag = false;
			for ( size_t i = 0; i < payOff.size(); i++ )
			{	
				convexityAdjusts[i].resize( payOff[i].size() );
				convexityAdjustVols[i].resize( payOff[i].size() ); 
	
				for ( size_t j = 0; j < payOff[i].size(); j++ )
				{
					// ! Get index with assumption of no digital coupon, no more than 2 indices.
					vector<LAPriceCouponTool *> coupons = payOff[i][j].getPayOff().mpCoupons;

					if (coupons.size() == 1)
					{
						vector<LAPriceIndexToolBase *> indexs = (*coupons[0]).getIndexs();
						if (indexs.size() == 1)
						{
							if (indexs[0]->isTypeOf(INDEX_TOOL))
							{
								const UintArray &indexPos = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexPos();
								if (!indexPos.empty())
								{
									// get last index value
									AQLString indextype = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getIndexType();
									bool isDelayed = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->isDelayedConvexityAdjusted();
									if (indextype == "CMS" || isDelayed)
									{
										convexityAdjusts[i][j] = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getConvexityAdjust()[indexPos.back()];
										convexityAdjustVols[i][j] = dynamic_cast<LAPriceIndexTool *>(indexs[0])->getIndexEntity()->getConvexityAdjustVol()[indexPos.back()];
										convexityAdjustFlag = true;
									}
								}
								else
								{
									convexityAdjusts[i][j] = 0.;
									convexityAdjustVols[i][j] = 0.;
								}
							}
							else
							{
								convexityAdjusts[i][j] = 0.;
								convexityAdjustVols[i][j] = 0.;
							}
						}
						else
						{
							convexityAdjusts[i][j] = 0.;
							convexityAdjustVols[i][j] = 0.;
						}
					}
					else
					{
						convexityAdjusts[i][j] = 0.;
						convexityAdjustVols[i][j] = 0.;
					}
				}
			}

			DoubleMatrix cfmat = avecf1;
			DoubleMatrix pvmat = avepv1;
			DoubleMatrix ptimemat = dataProvider->paytimes;

			// add the past info if exists
			for (unsigned int i = 0 ; i < legnum; i++)
			{

				dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
				AQLDataMultiReference& unders = dynamic_cast<AQLDataMultiReference& >(dh->get());
				AQLObject & leg = unders.get(i).get();

				// For cases where a leg has a fee but not cashflows.
                dh = &(leg.getData(PRICING_DATA_CASHLETS, NOCHECK));
                if(!dh->isDefined() || dh->isNull()) continue;
				

				dh = &(leg.getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
				/*if (!dh->isDefined() || dh->isNull())
					continue;
				DateVector pastpaydates = dynamic_cast<AQLDataDates &>(dh->get()).get();*/
				DateVector pastpaydates;
				if (!dh->isDefined() || dh->isNull())
				{
					dh = &(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
					AQLDataMultiReference & cashlets = dynamic_cast<AQLDataMultiReference& >(dh->get());
					for (unsigned int j = 0 ; j < cashlets.getSize(); j++)
					{
						AQLObject& eCashlet = cashlets.get(j).get();
						dh = &(eCashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
						AQLDate paymentDate = dynamic_cast<AQLDataDate &>(dh->get()).get();
						if (paymentDate <= basedate) pastpaydates.push_back(paymentDate);
						else break;
					}
				}
				else
				{
					pastpaydates = dynamic_cast<AQLDataDates &>(dh->get()).get();
					/*dh = &(leg.getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL));
					pastcfs = dynamic_cast<AQLDataDoubles &>(dh->get()).get();*/

					/*if (pastpaydates.size() != pastcfs.size())
						throw AQLCoreInvalidData("PastCoupons Error",__FILE__,__LINE__);*/

				}
				
				unsigned int pastSize = pastpaydates.size();

				DateVector pastcfStartdates(pastSize);
				DateVector pastcfEnddates(pastSize);
				DoubleVector pastNotionals(pastSize);
				DoubleVector pastNotionalcfs(pastSize);
				DateVector pastRenotionalFixingDates(pastSize);
				DoubleVector pastGearings(pastSize);
				DoubleVector pastForwards(pastSize);
				DoubleVector pastSpreads(pastSize);
				DoubleVector pastcfs(pastSize);
				DoubleVector pastAccrualDays(pastSize);
				DateVector pastFixingDates(pastSize);
				AQLStringVector pastFixingFlags(pastSize);
				DateVector pastSettleFixingDates(pastSize);
				DoubleVector pastSettleAdjustRatios(pastSize, 1.0);

				dh = &(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
				AQLDataMultiReference & cashlets = dynamic_cast<AQLDataMultiReference& >(dh->get());
				if (cashlets.getSize() < pastSize)
					throw AQLCoreInvalidData("PastCashlets Error",__FILE__,__LINE__);

				dh = &(leg.getData(PRICING_DATA_SELECTSIDE, ISNOTNULL));
				AQLString sltside = dynamic_cast<const AQLDataString&>(dh->get()).get();
				sltside.toUpper();

				unsigned int k = 0;
				for (unsigned int j = 0; j < pastSize; j++)
				{
					//AQLObject& cashlet
					//AQLDate chkpaydate = pastpaydates[j];
					
					while (k < cashlets.getSize())
					{
						AQLObject& ecash = cashlets.get(k).get();
						dh = &(ecash.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
						AQLDate targetdate = dynamic_cast<AQLDataDate &>(dh->get()).get();
						if (pastpaydates[j] == targetdate)
						{
							dh = &(ecash.getData(PRICING_DATA_CFCALCSTARTDATE, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								pastcfStartdates[j] = dynamic_cast<AQLDataDate &>(dh->get()).get();
							}

							dh = &(ecash.getData(PRICING_DATA_CFCALCENDDATE, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								pastcfEnddates[j] = dynamic_cast<AQLDataDate &>(dh->get()).get();
							}

							dh = &(ecash.getData(PRICING_CALIBRATION_DATAOTIONAL, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								pastNotionals[j] = dynamic_cast<AQLDataDouble &>(dh->get()).get();
							}

							dh = &(ecash.getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								pastNotionalcfs[j] = dynamic_cast<AQLDataDouble &>(dh->get()).get();
							}

							dh = &(ecash.getData(PRICING_DATA_RENOTIONALFIXINGDATE, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								pastRenotionalFixingDates[j] = dynamic_cast<AQLDataDate &>(dh->get()).get();
							}

							dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
							if (dh->isDefined() && !dh->isNull())
							{
								AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
								if (coupons.getSize() < 1)
									throw AQLCoreInvalidData("A coupon of a past cashflow is more than one!", __FILE__, __LINE__);
								AQLObject &coupon = coupons.get(0).get();

								dh = &(coupon.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
								if (dh->isDefined() && !dh->isNull())
								{
									AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(dh->get());
									if (indexs.getSize() < 1) 
										throw AQLCoreInvalidData("A index of a past coupon is more than one!", __FILE__, __LINE__);
									AQLObject &index = indexs.get(0).get();

									dh = &(index.getData(PRICING_DATA_FIXEDRATE, NOCHECK));
									if (dh->isDefined() && !dh->isNull())
									{
										pastForwards[j] = dynamic_cast<AQLDataDouble &>(dh->get()).get();	
									}

									dh = &(coupon.getData(PRICING_DATA_OPERATOR, NOCHECK));
									if (dh->isDefined() && !dh->isNull())
									{
										const DoubleArray& coeff = dynamic_cast<const AQLDataDoubles&>(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get()).get();
										//spread
										pastSpreads[j] = coeff.back();
										//gearing
										for (size_t i_coeff = 0; i_coeff < coeff.size() - 1; ++i_coeff)
										{
											pastGearings[j] += coeff[i_coeff];
										}
										AQLPriceDataFunction &method = dynamic_cast<AQLPriceDataFunction &>(dh->get());
										if (method.isTypeOf(FN_LINEAR))
										{
											dh = &(ecash.getData(PRICING_DATA_DAYCOUNT, NOCHECK));
											if (dh->isDefined() && !dh->isNull())
											{
												AQLPriceDataDayCount &dc = dynamic_cast<AQLPriceDataDayCount&>(dh->get());
												if (dc.getDayCount() == ACT_ACT_ICMA)
												{
													const AQLString frequency = LACoreUtility::removeQuotation(leg.getData(PRICING_DATA_FREQUENCY, ISNOTNULL).convertToString());
													const AQLString calendar = leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).convertToString();
													const AQLString slidingrule = leg.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).convertToString();

													DateVector pastcfStartdate_vec(1), pastcfEnddate_vec(1);
													pastcfStartdate_vec[0] = pastcfStartdates[j];
													pastcfEnddate_vec[0] = pastcfEnddates[j];

													DateMatrix regular_startenddates = LAMathDateUtilities::calcRegularDates(frequency, calendar, slidingrule, pastcfStartdate_vec, pastcfEnddate_vec);
													dc.setCouponsInYear(12 / LAMathDateCalculations::getPeriodFrequencyInMonths(frequency));
													dc.setCouponStartDates(regular_startenddates[0]);
													dc.setCouponEndDates(regular_startenddates[1]);
												}
												double accrualTerm = dc.getTerm(pastcfStartdates[j], pastcfEnddates[j], false);
												pastcfs[j] = (pastGearings[j] * pastForwards[j] + pastSpreads[j]) * accrualTerm * pastNotionals[j];
											}

											pastcfs[j] += pastNotionalcfs[j];
											if (sltside == PAY) pastcfs[j] *= -1;
										}
									}

									dh = &(index.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
									if (dh->isDefined() && !dh->isNull())
									{
										pastFixingDates[j] = dynamic_cast<AQLDataDate &>(dh->get()).get();
										pastFixingFlags[j] = YES_FIXINGFLAG;
									}
								}
							}

							pastAccrualDays[j] = pastcfStartdates[j].intervalDays( pastcfEnddates[j] );

							// for NDS after modification of ODMS mapping
							dh = &(ecash.getData(PRICING_DATA_SETTLEMENTCURRENCY, NOCHECK));
							{
								if (dh->isDefined() && !dh->isNull())
								{
									pastSettleFixingDates[j] = dynamic_cast<AQLDataDate &>((&(ecash.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL)))->get()).get();
									pastSettleAdjustRatios[j] = 1.0;
								}
							}

							k++;
							break;
						}
						else if (pastpaydates[j] > targetdate) 
						{
							throw AQLCoreInvalidData("PastPaymentDateError",__FILE__,__LINE__);
						}
						
						k++;
					}
				}

				//insert the past info
				DoubleVector tmpinsertvec(pastSize, 0.0);
				DateVector tmpInsertDateVec(pastSize);

				cfCalcStartDates[i].insert(cfCalcStartDates[i].begin(), pastcfStartdates.begin(),pastcfStartdates.end());
				cfCalcEndDates[i].insert(cfCalcEndDates[i].begin(), pastcfEnddates.begin(),pastcfEnddates.end());
				paymentDates[i].insert(paymentDates[i].begin(), pastpaydates.begin(),pastpaydates.end());
				notionals[i].insert(notionals[i].begin(), pastNotionals.begin(),pastNotionals.end());
				notionalcfs[i].insert(notionalcfs[i].begin(), pastNotionalcfs.begin(),pastNotionalcfs.end());
				renotionalfixingdates[i].insert(renotionalfixingdates[i].begin(), pastRenotionalFixingDates.begin(),pastRenotionalFixingDates.end());
				gearings[i].insert(gearings[i].begin(), pastGearings.begin(), pastGearings.end());
				forwards[i].insert(forwards[i].begin(), pastForwards.begin(),pastForwards.end());
				spreads[i].insert(spreads[i].begin(), pastSpreads.begin(),pastSpreads.end());
				accrualDays[i].insert(accrualDays[i].begin(), pastAccrualDays.begin(),pastAccrualDays.end());
				cfmat[i].insert(cfmat[i].begin(), pastcfs.begin(),pastcfs.end());
				fixingDates[i].insert(fixingDates[i].begin(), pastFixingDates.begin(),pastFixingDates.end());
				fixingFlags[i].insert(fixingFlags[i].begin(), pastFixingFlags.begin(),pastFixingFlags.end());
				settlefixingdates[i].insert(settlefixingdates[i].begin(), pastSettleFixingDates.begin(), pastSettleFixingDates.end());
				settleadjustratios[i].insert(settleadjustratios[i].begin(), pastSettleAdjustRatios.begin(), pastSettleAdjustRatios.end());
				//pvmat[i].insert(pvmat[i].begin(), pastcfs.begin(),pastcfs.end());


				pvmat[i].insert(pvmat[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
				dfs[i].insert(dfs[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
				//forwards[i].insert(forwards[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
				//gearings[i].insert(gearings[i].begin(), tmpinsertvec_one.begin(),tmpinsertvec_one.end());
				//spreads[i].insert(spreads[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
				//accrualDays[i].insert(accrualDays[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
					

				ptimemat[i].insert(ptimemat[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());

				if (convexityAdjustFlag)
				{
					convexityAdjusts[i].insert(convexityAdjusts[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
					convexityAdjustVols[i].insert(convexityAdjustVols[i].begin(), tmpinsertvec.begin(),tmpinsertvec.end());
				}

			}
			//////////////////////////////////////////////

			// today fx converting from settlement currency to non-deliverable currency (used only for NDS)
			DoubleVector fx_settle_nondeliv_vec;
			if(dataProvider->isnondeliverableLeg.size() == 2)
			{
				if(dataProvider->isnondeliverableLeg[0])
					fx_settle_nondeliv_vec.resize(cfmat[0].size(),fx_settle_nondeliv);
				else if(dataProvider->isnondeliverableLeg[1])
					fx_settle_nondeliv_vec.resize(cfmat[1].size(),fx_settle_nondeliv);
			}

			//check is renotional or not
			dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
			AQLDataMultiReference& unders = dynamic_cast<AQLDataMultiReference& >(dh->get());
			

			setCompoundedRate(object, dataProvider->payoff);

			for (unsigned int i = 0; i < legnum; i++)
			{
				const int num = i + 1;
				// set underlying value(for each cachlet)
				// ! CFCalcStartDate
				AQLString cfCalcStartDateName = PRICING_DATA_CFCALCSTARTDATE_LEG + AQLString( num );
				object.remove( cfCalcStartDateName );
				object.add( cfCalcStartDateName, new AQLDataDates( cfCalcStartDates[i] ) );
				
				// ! CFCalcEndDate
				AQLString cfCalcEndDateName = PRICING_DATA_CFCALCENDDATE_LEG + AQLString( num );
				object.remove( cfCalcEndDateName );
				object.add( cfCalcEndDateName, new AQLDataDates( cfCalcEndDates[i] ) );

				// ! PaymentDate
				AQLString paymentDateName = PRICING_DATA_PAYMENTDATE_LEG + AQLString( num );
				object.remove( paymentDateName );
				object.add( paymentDateName, new AQLDataDates( paymentDates[i] ) );

				// ! PaymentTime	
				AQLString time = PRICING_DATA_CASHLETVALUETIME_LEG + AQLString( num );
				object.remove(time);
				object.add(time, new AQLDataDoubles(ptimemat[i]));

				// ! Cashflow
				AQLString cfname = PRICING_DATA_CASHLETVALUE_LEG + AQLString( num );
				object.remove(cfname);
				object.add(cfname, new AQLDataDoubles(cfmat[i]));

				// ! PV
				AQLString pvname = PRICING_DATA_PVVALUE_LEG + AQLString( num );
				object.remove(pvname);
				object.add(pvname, new AQLDataDoubles(pvmat[i]));

				// ! DF
				AQLString dfName = PRICING_DATA_DF_LEG + AQLString( num );
				object.remove( dfName );
				object.add( dfName, new AQLDataDoubles( dfs[i] ) );

				// ! Gearing
				AQLString gearingame = PRICING_DATA_GEARING_LEG + AQLString( num );
				object.remove( gearingame );
				object.add( gearingame, new AQLDataDoubles( gearings[i] ) );

				// ! Forward
				AQLString forwardName = PRICING_DATA_FORWARD_LEG + AQLString( num );
				object.remove( forwardName );
				object.add( forwardName, new AQLDataDoubles( forwards[i] ) );

				// ! Spreads
				AQLString spreadName = PRICING_DATA_SPREAD_LEG + AQLString( num );
				object.remove( spreadName );
				object.add( spreadName, new AQLDataDoubles( spreads[i] ) );

				// ! AccrualDays
				AQLString accDaysName = PRICING_DATA_ACCRUALDAYS_LEG + AQLString( num );
				object.remove( accDaysName );
				object.add( accDaysName, new AQLDataDoubles( accrualDays[i] ) );

				// ! Notional
				AQLString notionalName = PRICING_CALIBRATION_DATAOTIONAL_LEG + AQLString( num );
				object.remove( notionalName );
				object.add( notionalName, new AQLDataDoubles( notionals[i] ) );

				// ! FixingDate
				AQLString fixingDateName = PRICING_DATA_FIXINGDATE_LEG + AQLString( num );
				object.remove( fixingDateName );
				object.add( fixingDateName, new AQLDataDates( fixingDates[i] ) );

				// ! FixingFlag
				AQLString fixingFlagName = PRICING_DATA_FIXINGFLAG_LEG + AQLString( num );
				object.remove( fixingFlagName );
				object.add( fixingFlagName, new AQLDataStrings( fixingFlags[i] ) );

				// ! FX rate for NDS
				if(dataProvider->isnondeliverableLeg[i])
				{
					AQLString fx_nondeliv_settle_Name = DATAN_N_MV_IR_TODAYFX_SETTLEMENTTONONDELIVERABLE + AQLString( num );
					object.remove(fx_nondeliv_settle_Name);
					object.add(fx_nondeliv_settle_Name, new AQLDataDoubles(fx_settle_nondeliv_vec));
				}

				// for NDS after modification of ODMS mapping
				if (isNonDeliverable[i])
				{
					AQLString settleFixingDateName = PRICING_DATA_SETTLEMENTFIXINGDATE_LEG + AQLString(num);
					object.remove(settleFixingDateName);
					object.add(settleFixingDateName, new AQLDataDates(settlefixingdates[i]));

					AQLString settleAdjustRatioName = PRICING_DATA_SETTLEMENTADJUSTRATIO_LEG + AQLString(num);
					object.remove(settleAdjustRatioName);
					object.add(settleAdjustRatioName, new AQLDataDoubles(settleadjustratios[i]));
				}

				bool isrenotional = false;
				bool isamortize = false;
				bool isnotionalexatend = false;
				AQLObject & leg = unders.get(i).get();
				dh = &(leg.getData(PRICING_DATA_ISRENOTIONAL, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					isrenotional = dynamic_cast<AQLDataBool &>(dh->get()).get();
				dh = &(leg.getData(PRICING_DATA_ISAMORTIZE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					isamortize = dynamic_cast<AQLDataBool &>(dh->get()).get();
				dh = &(leg.getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					isnotionalexatend = dynamic_cast<AQLDataBool &>(dh->get()).get();
				if (isrenotional || (isamortize && isnotionalexatend))
				{
					// ! Notionalcv
					AQLString notionalcfName = PRICING_CALIBRATION_DATAOTIONALCF_LEG + AQLString( num );
					object.remove( notionalcfName );
					object.add( notionalcfName, new AQLDataDoubles( notionalcfs[i] ) );
				}
				if (isrenotional)
				{
					// ! RenotionalFixingDate
					AQLString renotionalfixingDateName = PRICING_DATA_RENOTIONALFIXINGDATE_LEG + AQLString( num );
					object.remove( renotionalfixingDateName );
					object.add( renotionalfixingDateName, new AQLDataDates( renotionalfixingdates[i] ) );
				}

				if (isFra)
				{
					// ! FRADF
					AQLString dfName = PRICING_DATA_FRADF_LEG + AQLString( num );
					object.remove( dfName );
					object.add( dfName, new AQLDataDoubles( fradfs[i] ) );
				}

				// ! ConvexityAdjust
				if (convexityAdjustFlag)
				{
					AQLString convexityAdjustName = PRICING_DATA_CONVEXITYADJUST_LEG + AQLString( num );
					object.remove( convexityAdjustName );
					object.add( convexityAdjustName, new AQLDataDoubles( convexityAdjusts[i] ) );

					AQLString convexityAdjustVolName = PRICING_DATA_CONVEXITYADJUSTVOL_LEG + AQLString( num );
					object.remove( convexityAdjustVolName );
					object.add( convexityAdjustVolName, new AQLDataDoubles( convexityAdjustVols[i] ) );
				}

				if (isdetailoutput)
				{
					// trigger pv
					DoubleVector cf_trigger	= avecf1[i];			
					DoubleVector pv_trigger	= avepv1[i];
					// minus underlying value
					transform(cf_trigger.begin(), cf_trigger.end(), avecf1[i].begin(), cf_trigger.begin(), minus<double>());
					transform(pv_trigger.begin(), pv_trigger.end(), avepv1[i].begin(), pv_trigger.begin(), minus<double>());
				
					AQLString triggername = "TriggerValue_Leg" + AQLDataInt(i + 1).convertToString();
					AQLString triggertime = "TriggerValueTime_Leg" + AQLDataInt(i + 1).convertToString();
					AQLString triggerpvname = "TriggerPVValue_Leg" + AQLDataInt(i + 1).convertToString();
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
						
						double fxrate = 1.0;
						if (dataProvider->rcvpay[i] && dataProvider->basecur != dataProvider->reccur)
						{
							fxrate = getFXEntity(object).getRate(dataProvider->reccur, dataProvider->basecur, dataProvider->baseterm);
						}
						else if (!dataProvider->rcvpay[i] && dataProvider->basecur != dataProvider->paycur)
						{
							fxrate = getFXEntity(object).getRate(dataProvider->paycur, dataProvider->basecur, dataProvider->baseterm);
						}
						map<double, double>::const_iterator it_cf = extracf_cfmap[i].begin();
						map<double, double>::const_iterator it_pv = extracf_pvmap[i].begin();
						while (it_cf != extracf_cfmap[i].end())
						{
							time_extra.push_back(it_cf->first);
							cf_extra.push_back(it_cf->second * fxrate);
							pv_extra.push_back(it_pv->second * fxrate);
							++it_cf, ++it_pv;
						}
	
						AQLString extracf_cfname = "ExtraCFValue_Leg" + AQLDataInt(i + 1).convertToString();
						AQLString extracf_time = "ExtraCFValueTime_Leg" + AQLDataInt(i + 1).convertToString();
						AQLString extracf_pvname = "ExtraCFPVValue_Leg" + AQLDataInt(i + 1).convertToString();
						object.remove(extracf_cfname);
						object.add(extracf_cfname, new AQLDataDoubles(cf_extra));
						object.remove(extracf_time);
						object.add(extracf_time, new AQLDataDoubles(time_extra));
						object.remove(extracf_pvname);
						object.add(extracf_pvname, new AQLDataDoubles(pv_extra));
					}
				}

				// for EQ-Flows
				AQLString dataName;
				// ! AccrualTerms
				dataName = PRICING_DATA_ACCRUALTERM_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( accrualTerms[i] ) );
				// ! Boundary1
				dataName = PRICING_DATA_BOUNDARY1_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( boundary1[i] ) );
				// ! Boundary2
				dataName = PRICING_DATA_BOUNDARY2_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( boundary2[i] ) );
				// ! Boundary3
				dataName = PRICING_DATA_BOUNDARY3_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( boundary3[i] ) );
				// ! Boundary4
				dataName = PRICING_DATA_BOUNDARY4_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( boundary4[i] ) );
				// ! CouponRate0
				dataName = PRICING_DATA_COUPONRATE0_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( couponRate0[i] ) );
				// ! CouponRate1
				dataName = PRICING_DATA_COUPONRATE1_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( couponRate1[i] ) );
				// ! CouponRate2
				dataName = PRICING_DATA_COUPONRATE2_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( couponRate2[i] ) );
				// ! CouponRate1
				dataName = PRICING_DATA_COUPONRATE3_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( couponRate3[i] ) );
				// ! CouponRate1
				dataName = PRICING_DATA_COUPONRATE4_LEG + AQLString( num );
				object.remove( dataName );
				object.add( dataName, new AQLDataDoubles( couponRate4[i] ) );

			}
		}
	}
//#endif
	
	return dirtyprice1;
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
LALinearRatesSwapTradeValue::value(const AQLDate& basedate, 
						AQLObject& object, 
						AQLDataProvider* dp,
						unsigned int startpathnum) const
{
	(void)basedate;(void)object;(void)dp;(void)startpathnum;
	return 0.0;
}



/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*
LALinearRatesSwapTradeValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
								const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	LALinearRatesSwapTradeValueDataProvider* dataProvider = dynamic_cast<LALinearRatesSwapTradeValueDataProvider*>(createNewDataProvider());
	att.setDataProvider(dataProvider);

	// base currency
	dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	dataProvider->basecur = dynamic_cast<const AQLDataString&>(dh->get()).get();
	
	// path object
	dh = &(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	AQLDataReference& attr = dynamic_cast<AQLDataReference&>(dh->get());
	dataProvider->pVanilla = &dynamic_cast<LAMathPlainVanillaEntity&>(attr.get().get());

	// today
	dh = &(object.getData(PRICING_DATA_TODAY));
    if(!dh->isDefined() || dh->isNull()){
        dh = &object.getData(CALIBRATION_DATA_ASOFDATE);
    }
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
	dataProvider->settle = dataProvider->pVanilla->getDayCount().getTerm(today, settledate);
	// baseterm
	AQLDate valueDate;
	dh = &(object.getData(PRICING_DATA_VALUEDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		valueDate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
	}
	else
	{
		valueDate = basedate;
	}
	if (today < valueDate)
		dataProvider->baseterm = dataProvider->pVanilla->getDayCount().getTerm(today, valueDate);
	else 
		dataProvider->baseterm = 0.0;
	/*if (today < basedate)
		dataProvider->baseterm = dataProvider->pVanilla->getDayCount().getTerm(today, basedate);
	else 
		dataProvider->baseterm = 0.0;*/

	//leg object
	dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference&>(dh->get());
	dataProvider->rcvpay.resize(legs.getSize());
	dataProvider->payDCurve = STD;
	dataProvider->recDCurve = STD;
	dataProvider->isnondeliverableLeg.resize(legs.getSize());
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
		// discount curve;
		dh = &(legs.get(i).getData(PRICING_DATA_DISCOUNTCURVE, ISNOTNULL));
		/*if (dh->isDefined() && !dh->isNull())
		{*/
			const AQLString &dCurveName = dynamic_cast<const AQLDataString&>(dh->get()).get();
			if (dataProvider->rcvpay[i])
			{
				dataProvider->recDCurve = dCurveName;
			}
			else
			{
				dataProvider->payDCurve = dCurveName;
			}
		//}
		dh = &(legs.get(i).getData(PRICING_DATA_CURRENCY, NOCHECK));
		if (  legs.getSize() > 1  && dh->isDefined() && !dh->isNull() )
		{
			const AQLString &legCur = dynamic_cast<const AQLDataString &>(dh->get()).get();
			if (dataProvider->rcvpay[i])
			{
				dataProvider->reccur = legCur;
			}
			else
			{
				dataProvider->paycur = legCur;
			}
		}
		else if (legs.getSize() == 1)
		{
			dataProvider->reccur = dataProvider->basecur;
			dataProvider->paycur = dataProvider->basecur;
		}
		else
		{
			if (dataProvider->rcvpay[i])
			{
				dataProvider->reccur = dataProvider->basecur;
			}
			else
			{
				dataProvider->paycur = dataProvider->basecur;
			}
		}

		// for NDS
		dh = &(legs.get(i).getData(PRICING_DATA_ISNONDELIVERABLELEG, NOCHECK));
		if(dh->isDefined() && !dh->isNull())
		{
			if(dynamic_cast<const AQLDataBool &>(dh->get()).get())
			{
				dataProvider->isnondeliverableLeg[i] = true;
				dh = &(legs.get(i).getData(PRICING_DATA_ORIGINALCURRENCY, ISNOTNULL));
				dataProvider->originalcur = dynamic_cast<const AQLDataString&>(dh->get()).get();
			}
			else
				dataProvider->isnondeliverableLeg[i] = false;
		}
		else
			dataProvider->isnondeliverableLeg[i] = false;
	}
	// option holder
	dataProvider->isoptionholder = true;
	dh = &(object.getData(PRICING_DATA_ISOPTIONHOLDER, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->isoptionholder = dynamic_cast<const AQLDataBool&>(dh->get()).get();
	}



	//dataProvider->pFX = NULL;
	if ( (dataProvider->basecur != dataProvider->paycur || dataProvider->basecur != dataProvider->reccur) && legs.getSize() > 1 )
	{
		object.remove(PRICING_DATA_ISLEGBASECURRENCY);
		object.add(PRICING_DATA_ISLEGBASECURRENCY, new AQLDataBool(true));
		//dh = &(object.getData(IR_MODEL_DATA_FXRATE, ISNOTNULL));
		//AQLDataReference& fx = dynamic_cast<AQLDataReference&>(dh->get());
		//dataProvider->pFX = &dynamic_cast<LAMathFXEntity&>(fx.get().get());
	}

	// cashflow generator
	dh = &object.getData(PRICING_DATA_CFGENERATOR, ISNOTNULL);
	AQLDataProcedure& modelDataObj = dynamic_cast<AQLDataProcedure&>(dh->get());
	modelDataObj.calibrateModel(today);
	
	AQLDate asofdate = dataProvider->pVanilla->getAsOfDate();

	//for stub swap
	dh = &(object.getData(PRICING_DATA_ISODDTRADE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		bool isoddtrade = dynamic_cast<AQLDataBool &>(dh->get());
		if (isoddtrade)
		{
			dataProvider->pVanilla->setUpLiborRateMap();
			setUpStubCoefficient(object, dataProvider);
		}

	}

	// remove notional exposure
	dh = &object.getData(PRICING_DATA_OMITNOTIONALEXPOSURE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		bool omitnotionalexposure = dynamic_cast<AQLDataBool &>(dh->get());
		if (omitnotionalexposure)
		{
			removeNotionalExposure(object, dataProvider);
		}
	}


	// setup
	//dataProvider->payoff.setUp(today, object);
	dataProvider->payoff.setUp(asofdate, object);

	//irrswaption
	dataProvider->isirrmodel = false;
	dataProvider->isannuitycalc = false;
	dh = &object.getData(PRICING_DATA_ISANNUITYCALC, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		dataProvider->isannuitycalc = dynamic_cast<AQLDataBool&>(dh->get()).get();
	if (dataProvider->isannuitycalc)
		setUpForAnnuityCalc(object,dataProvider);

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

	// reference index
	dataProvider->indexs = getReferenceIndex(object);
	
	// accrued interest function
	if (mpAcc != NULL)
	{
		//mpAcc->setUp(basedate, object, att);
		mpAcc->setUp(asofdate, object, att);
		dataProvider->acc.set(mpAcc->clone(), true);
	}

	dataProvider->isdifferentiation = false;
	dh = &object.getData(PRICING_DATA_ISANALYTIC, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		dataProvider->isdifferentiation = dynamic_cast<AQLDataBool&>(dh->get()).get();
	
	return dataProvider;	
}


double
LALinearRatesSwapTradeValue::value_backward(const DoubleMatrix& time,
											const DoubleMatrix& cf,
											std::vector<std::pair<unsigned int, AQLDate> >& triggerhit,
											const DoubleArray& rebate,
											const DoubleMatrix& explanatory,
											const AQLDataProvider* dp,
											DoubleVector &callval,
											BoolVector* afterjudge,
											BoolVector* judge,
											DoubleVector* explainedvar) const
{
	(void)time;(void)cf;(void)triggerhit;
	(void)rebate;(void)explanatory;(void)dp;
	(void)callval;(void)afterjudge;(void)judge;
	(void)explainedvar;
	return 0.0;
}

/*!
	@brief get FXEntity

	@param[in] object
	@return FXEntity Ref
*/
const LAMathFXEntity &
LALinearRatesSwapTradeValue::getFXEntity(AQLObject &object) const
{
	const AQLDataReference &paramref = dynamic_cast<const AQLDataReference &>(object.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
	const LAMathPlainVanillaEntity& pvanilla = dynamic_cast<const LAMathPlainVanillaEntity &>(paramref.get().get());
	return dynamic_cast<const LAMathFXEntity& >(pvanilla.getFXEntity().get().get());
}



/*!
	@brief get setUpForAnnuityCalc

	@param[in] object
	@param[in] dataProvider
	
*/
void						
LALinearRatesSwapTradeValue::setUpForAnnuityCalc(AQLObject& object, AQLDataProvider* dp) const
{
	LALinearRatesSwapTradeValueDataProvider* dataProvider = dynamic_cast<LALinearRatesSwapTradeValueDataProvider*>(dp);

	const AQLDataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& refs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	if (refs.getSize() != 2)
		throw AQLCoreInvalidData("setUpForAnnuityCalc error",__FILE__,__LINE__);
	const AQLObject& leg1 = refs.get(0).get();

	//search floater leg
	dataProvider->annuityLegNo = 0;

	dh = &(leg1.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference &>(dh->get());
	unsigned int cSize = cashlets.getSize();
	for (unsigned int i = 0; i < cSize; i++)
	{
		const AQLObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;
			

		const AQLDataMultiReference& coupons = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		const AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
		const AQLDataMultiReference& indexs = dynamic_cast<const AQLDataMultiReference &>(dh->get());
		const AQLObject& eindex = indexs.get(0).get();

		AQLString indextype;
		dh = &(eindex.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			indextype = dynamic_cast<const AQLDataString &>(dh->get()).get();
			indextype.toUpper();
		}

		if (indextype !=  "FIXEDRATE" && indextype != "")
		{
			dataProvider->annuityLegNo = 1;
			break;
		}
	}
	

	//map to the annuitymap
	// calc maxcfnum and paytime
	DoubleVector termvec;
	const vector<PayOffToolHolderVector> &payvec = dataProvider->payoff.getPayOff();
	unsigned int annuitycSize = payvec[dataProvider->annuityLegNo].size();
	for (unsigned int i = 0; i < annuitycSize; i++)
	{
		dataProvider->annuitymap.insert(std::make_pair(i ,payvec[dataProvider->annuityLegNo][i].getPayOff().mTerm));
		termvec.push_back(payvec[dataProvider->annuityLegNo][i].getPayOff().mTerm);

	}
	
	//irr swaption
	dh = &(object.getData(PRICING_DATA_ISIRRMODEL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		dataProvider->isirrmodel = dynamic_cast<const AQLDataBool &>(dh->get()).get();

	if (dataProvider->isirrmodel)
	{
		const AQLObject& fixedleg = refs.get(dataProvider->annuityLegNo).get(); 
		dh = &(fixedleg.getData(PRICING_DATA_STARTDATE, ISNOTNULL));
		const AQLDate& fwdswpsdate = dynamic_cast<const AQLDataDate &>(dh->get()).get();
		// baseterm
		const AQLDate& basedate = dataProvider->pVanilla->getAsOfDate().get();
	
		if (basedate < fwdswpsdate)
			dataProvider->fwdswapterm = dataProvider->pVanilla->getDayCount().getTerm(basedate, fwdswpsdate);
		else 
			dataProvider->fwdswapterm = 0.0;
	}


	object.remove(PRICING_DATA_ANNUITYTERMS);
	object.add(PRICING_DATA_ANNUITYTERMS, new AQLDataDoubles(termvec));

	return;
}


//for stub swap
void
LALinearRatesSwapTradeValue::setUpStubCoefficient(AQLObject& trade, AQLDataProvider* dp) const
{
	AQLDataHolder* dh;
	dh = &(trade.getData(PRICING_DATA_ISSTUBSETCOMPLETE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		bool isstubset = dynamic_cast<AQLDataBool &>(dh->get());
		if (isstubset)
			return;
	}

	LALinearRatesSwapTradeValueDataProvider* dataProvider = dynamic_cast<LALinearRatesSwapTradeValueDataProvider*>(dp);


	LAMathPlainVanillaEntity *pvanilla = dataProvider->pVanilla;
	if (!pvanilla)
	{
		throw AQLCoreInvalidData("Plain Vanilla object is not set in entitiy pool", __FILE__, __LINE__);
	}

	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference &legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	unsigned int legSize = legs.getSize();

	for (unsigned int i = 0; i < legSize; i++)
	{
		AQLObject &eleg = legs.get(i).get();
		//get cashlet
		dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
		unsigned int cashletSize = cashlets.getSize();
		for (unsigned int j = 0; j < cashletSize; j++)
		{
			AQLObject &ecashlet = cashlets.get(j).get();
			//get firstcoupon
			dh = &(ecashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
				continue;

			AQLDataMultiReference &coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
            for(size_t idx_coupon = 0; idx_coupon < coupons.getSize(); idx_coupon++)
            {
                AQLObject &temp_coupon = coupons.get(idx_coupon).get();
				dh = &(temp_coupon.getData(PRICING_DATA_ODDINDEXTYPE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
					doSetUpStubCoefficient(temp_coupon, ecashlet, pvanilla);
            }
		}
	}


	trade.remove(PRICING_DATA_ISSTUBSETCOMPLETE);
	trade.add(PRICING_DATA_ISSTUBSETCOMPLETE, new AQLDataBool(true));
	
}




/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
LALinearRatesSwapTradeValue::createNewDataProvider() const
{
	LALinearRatesSwapTradeValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LALinearRatesSwapTradeValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

LALinearRatesSwapTradeValue::LALinearRatesSwapTradeValueDataProvider::~LALinearRatesSwapTradeValueDataProvider()
{
}

void LALinearRatesSwapTradeValue::setCompoundedRate(AQLObject& trade, const LAPricePayOff& payoff) const{
	const AQLDataHolder* dh = &trade.getData(PRICING_DATA_COMPOUNDED_RATE_OUT, NOCHECK);
	if(!dh->isDefined() || dh->isNull() || !dynamic_cast<const AQLDataBool&>(dh->get()).get()){
		return;
	}
		

	for(size_t i = 0; i < payoff.getPayOff().size(); i++){
		trade.remove(AQLString(PRICING_DATA_COMPOUNDING_STARTDATE) + "_Leg_" + AQLString((int)(i+1)));
		trade.remove(AQLString(PRICING_DATA_COMPOUNDING_ENDDATE)   + "_Leg_" + AQLString((int)(i+1)));
		trade.remove(AQLString(PRICING_DATA_COMPOUNDING_FIXDATE)   + "_Leg_" + AQLString((int)(i+1)));
		trade.remove(AQLString(PRICING_DATA_COMPOUNDING_FIXFLAG)   + "_Leg_" + AQLString((int)(i+1)));
		trade.remove(AQLString(PRICING_DATA_COMPOUNDEDTERM)        + "_Leg_" + AQLString((int)(i+1)));
		trade.remove(AQLString(PRICING_DATA_COMPOUNDEDRATE)        + "_Leg_" + AQLString((int)(i+1)));

		DateVector start, end, fixd;
        AQLStringVector fixf;
		DoubleVector term, rate;
		payoff.getCompoundedRateInfo(i, start, end, term, fixd, fixf, rate);
		
        if(start.size()==0) continue;

		trade.add(AQLString(PRICING_DATA_COMPOUNDING_STARTDATE) + "_Leg_" + AQLString((int)(i+1)), new AQLDataDates(start));
		trade.add(AQLString(PRICING_DATA_COMPOUNDING_ENDDATE)   + "_Leg_" + AQLString((int)(i+1)), new AQLDataDates(end));
		trade.add(AQLString(PRICING_DATA_COMPOUNDING_FIXDATE)   + "_Leg_" + AQLString((int)(i+1)), new AQLDataDates(fixd));
		trade.add(AQLString(PRICING_DATA_COMPOUNDING_FIXFLAG)   + "_Leg_" + AQLString((int)(i+1)), new AQLDataStrings(fixf));
		trade.add(AQLString(PRICING_DATA_COMPOUNDEDTERM)        + "_Leg_" + AQLString((int)(i+1)), new AQLDataDoubles(term));
		trade.add(AQLString(PRICING_DATA_COMPOUNDEDRATE)        + "_Leg_" + AQLString((int)(i+1)), new AQLDataDoubles(rate));
	}
}

void LALinearRatesSwapTradeValue::doSetUpStubCoefficient(AQLObject& coupon, AQLObject& cashlet, LAMathPlainVanillaEntity* pvanilla) const
{
    AQLDataHolder* dh;
	AQLDate asOf = pvanilla->getAsOfDate().get();

    

    ////check fn_linear or not
    //dh = &(coupon.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
    //AQLPriceDataFunction &method = dynamic_cast<AQLPriceDataFunction &>(dh->get());
    //if (!method.isTypeOf(FN_LINEAR)) return;

    dh = &(coupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
    AQLDataMultiReference &indexs = dynamic_cast<AQLDataMultiReference &>(dh->get());
    unsigned int indexSize = indexs.getSize();

    dh = &(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
    AQLDataDoubles &coeffs = dynamic_cast<AQLDataDoubles &>(dh->get());
    DoubleVector coefvec = coeffs.get();

    // in case of stub setting but index is 1 
    //if (coefvec.size() == 2 && coefvec[0] == -9999.0 && indexSize == 1)
    //{
    //	coefvec[0] = 1.0;
    //	//set
    //	coeffs.set(coefvec);
    //	continue;
    //}

    ////index must be more than 2
    //if (coefvec.size() != indexSize + 1 || indexSize < 2)
    //	continue;

    //bool isstubcoupon = true;
    //for (unsigned k = 0; k < coefvec.size() - 1; k++)
    //{
    //    if (coefvec[k] != -9999.0)
    //    {	
    //        isstubcoupon = false;
    //        break;
    //    }
    //}
    //if (isstubcoupon)
    //{
	const AQLDate& start_date = (dh=&coupon.getData(PRICING_DATA_CFCALCSTARTDATE))->isDefined() && !dh->isNull() ? 
		dynamic_cast<const AQLDataDate&>(dh->get()).get() :
		dynamic_cast<const AQLDataDate&>(cashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
	const AQLDate& end_date = (dh=&coupon.getData(PRICING_DATA_CFCALCENDDATE))->isDefined() && !dh->isNull() ?
		dynamic_cast<const AQLDataDate&>(dh->get()).get() :
		dynamic_cast<const AQLDataDate&>(cashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get();

	//for (unsigned k = 0; k < coefvec.size() - 1; k++)
	//	coefvec[k] = 0.0;

	dh = &(coupon.getData(PRICING_DATA_ODDINDEXTYPE, ISNOTNULL));
	AQLString stubtype =  dynamic_cast<AQLDataString &>(dh->get()).get();
	stubtype.toUpper();

	if (stubtype == CURRENTINDEX)
	{
		//first index is main index
		for (unsigned k = 1; k < coefvec.size() - 1; k++)
			coefvec[k] = 0.0;
		coeffs.set(coefvec);
		return;
	}
	else if (stubtype == STARTENDDATEMETHOD)
	{
		//first index is main index
		DateVector datevec(2);
		datevec[0] = start_date;
		datevec[1] = end_date;

		AQLObject &eindex = indexs.get(0).get();
		dh = &(eindex.getData(PRICING_DATA_DATESFORINDEXGENERATE, NOCHECK));

		if (dh->isDefined() && !dh->isNull())
		{
			dynamic_cast<AQLDataDates &>(dh->get()).set(datevec);
		}
		else
		{
			eindex.add(PRICING_DATA_DATESFORINDEXGENERATE, new AQLDataDates(datevec));
		}

		//coefvec[0] = 1.0;
		for (unsigned k = 1; k < coefvec.size() - 1; k++)
			coefvec[k] = 0.0;
		coeffs.set(coefvec);
		return;
	}

	// in case of stub setting but index is 1 --> this is fixed rate case!!
	if (coefvec.size() == 2 && indexSize == 1)
	{
		//coefvec[0] = 1.0;
		for (unsigned k = 1; k < coefvec.size() - 1; k++)
			coefvec[k] = 0.0;
		//set
		coeffs.set(coefvec);
		return;
	}

	//get currency
	dh = &(coupon.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL));
	AQLString ccy = dynamic_cast<AQLDataString &>(dh->get()).get();
	//exchange stubcoupon
    	
	AQLPriceDataDayCount dc(ACT_365_ISDA);
	double term = dc.getTerm(start_date, end_date, false);
	if (stubtype == SPOTRATERATIOMETHOD)
	{
		std::map<AQLString, AQLInterpolationBase*>::iterator it = pvanilla->getLiborRateMap().find(ccy);
		if (it == pvanilla->getLiborRateMap().end())
			throw AQLCoreInvalidData("Odd Coefficient Error",__FILE__,__LINE__);

		AQLLinearInterpolation* inter = dynamic_cast<AQLLinearInterpolation* >(it->second);
		double targetrate = inter->value(term);

		//change ratio
		set<pair<double, pair<double, unsigned int> > > term_rate_pos;
		for (unsigned l = 0 ; l < indexSize; l++)
		{
			AQLObject &eindex = indexs.get(l).get();
			dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
			AQLString termStr = dynamic_cast<AQLDataString &>(dh->get()).get();

			dh = &(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
			int spotlag = dynamic_cast<AQLDataInt &>(dh->get()).get();

			dh = &(eindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
			AQLPriceDataSlidingRule &sld = dynamic_cast<AQLPriceDataSlidingRule &>(dh->get());

			dh = &(eindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
			AQLPriceDataCalendar &cal = dynamic_cast<AQLPriceDataCalendar &>(dh->get());

			AQLDate spotdate = cal.getBusinessDay(asOf,spotlag);
			AQLDate enddate = LAMathDateCalculations::getDate(spotdate, termStr, true);
			enddate = (!sld.isNull() && !cal.isNull()) ? sld.getDate(enddate, cal) : enddate;

			double temp_term = dc.getTerm(spotdate,enddate,false);
			double rate = inter->value(temp_term);

			term_rate_pos.insert(make_pair(temp_term, make_pair(rate, l)));
		}

		set<pair<double, pair<double, unsigned int> > >::const_iterator temp_it;
		for(temp_it = term_rate_pos.begin(); temp_it != term_rate_pos.end(); ++temp_it){
			if(temp_it->first >= term) break;
		}


		double rate1, rate2;
		unsigned int pos1, pos2;
		if(temp_it == term_rate_pos.begin()){
			rate1 = temp_it->second.first;
			pos1 = temp_it->second.second;
			temp_it++;
			rate2 = temp_it->second.first;
			pos2 = temp_it->second.second;
		}
		else if(temp_it == term_rate_pos.end()){
			temp_it--;
			rate2 = temp_it->second.first;
			pos2 = temp_it->second.second;
			temp_it--;
			rate1 = temp_it->second.first;
			pos1 = temp_it->second.second;
		}
		else{
			set<pair<double, pair<double, unsigned int> > > ::const_iterator it1, it2;
			it1 = it2 = temp_it; it1--;
			rate1 = it1->second.first;
			rate2 = it2->second.first;
			pos1 = it1->second.second;
			pos2 = it2->second.second;
		}

		double tolerance;
		bool isToleranceExist = false;
		dh = &(coupon.getData(PRICING_DATA_TOLERANCEFORSPOTRATERATIOMETHOD, NOCHECK));
		if(dh->isDefined() && !dh->isNull())
		{
			tolerance = dynamic_cast<AQLDataDouble &>(dh->get()).get();
			isToleranceExist = true;
		}

		if(isToleranceExist ? AQLMath::abs(rate1 - rate2) < tolerance : rate1==rate2)
		{
			for (unsigned k = 0; k < coefvec.size() - 1; k++){
				if(k == pos1 || k == pos2){
					coefvec[k] *= 0.5;
				}
				else{
					coefvec[k] = 0.0;
				}
			}
		}
		else{
			for (unsigned k = 0; k < coefvec.size() - 1; k++){
				if(k == pos1){
					coefvec[k] *= (rate2 - targetrate) / (rate2 - rate1);
				}
				else if(k == pos2){
					coefvec[k] *= (targetrate - rate1) / (rate2 - rate1);
				}
				else{
					coefvec[k] = 0.0;
				}
			}
		}
	}
	else
	{
		double targetterm = 0.0;
		std::map<AQLString, DoubleVector>::iterator itvec = pvanilla->getLiborGridTermMap().find(ccy);
		if (itvec == pvanilla->getLiborGridTermMap().end())
			throw AQLCoreInvalidData("Odd Coefficient Error",__FILE__,__LINE__);
		DoubleVector termvec = itvec->second;
		DoubleVector::iterator itTargetTerm = std::upper_bound(termvec.begin(),termvec.end(),term);
		if (itTargetTerm == termvec.end())
			--itTargetTerm;

		if (stubtype == NEXTINDEX)
		{
			targetterm = *itTargetTerm;
		}
		else if(stubtype == PREVIOUSINDEX)
		{
			if (itTargetTerm == termvec.begin())
				targetterm = *itTargetTerm;
			else
				targetterm = *(--itTargetTerm);
		}
		else if (stubtype == CLOSESTINDEX)
		{
			if (itTargetTerm == termvec.begin())
				targetterm = *itTargetTerm;
			else
			{
				double afterm = *itTargetTerm;
				double bfterm = *(--itTargetTerm);

				targetterm = (afterm - term > term - bfterm) ? bfterm : afterm;
			}
		}
		else if (stubtype == GRIDTERMRATIOMETHOD)
		{
			targetterm = term;
		}
		else
			throw AQLCoreInvalidData("Stub Type Error",__FILE__,__LINE__);

		//change ratio
		map<double, unsigned int> mapTermPos;
		for (unsigned l = 0 ; l < indexSize; l++)
		{
			AQLObject &eindex = indexs.get(l).get();
			dh = &(eindex.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
			AQLString termStr = dynamic_cast<AQLDataString &>(dh->get()).get();

			dh = &(eindex.getData(PRICING_DATA_SPOTLAG, ISNOTNULL));
			int spotlag = dynamic_cast<AQLDataInt &>(dh->get()).get();

			dh = &(eindex.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
			AQLPriceDataSlidingRule &sld = dynamic_cast<AQLPriceDataSlidingRule &>(dh->get());

			dh = &(eindex.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
			AQLPriceDataCalendar &cal = dynamic_cast<AQLPriceDataCalendar &>(dh->get());

			AQLDate spotdate = cal.getBusinessDay(asOf,spotlag);
			AQLDate enddate = LAMathDateCalculations::getDate(spotdate, termStr, true);
			enddate = (!sld.isNull() && !cal.isNull()) ? sld.getDate(enddate, cal) : enddate;

			double term = dc.getTerm(spotdate,enddate,false);
			mapTermPos.insert(std::make_pair(term,l));
		}

		map<double, unsigned int>::iterator itTerm = mapTermPos.lower_bound(targetterm);
		double smallterm,bigterm;
		unsigned int smallpos, bigpos;
		if (itTerm == mapTermPos.begin())
		{
			smallterm = itTerm->first;
			smallpos = itTerm->second;

			bigterm = (++itTerm)->first;
			bigpos = itTerm->second;
		}
		else if (itTerm == mapTermPos.end())
		{
			bigterm = (--itTerm)->first;
			bigpos = itTerm->second;

			smallterm = (--itTerm)->first;
			smallpos = itTerm->second;
		}
		else
		{
			bigterm = itTerm->first;
			bigpos = itTerm->second;

			smallterm = (--itTerm)->first;
			smallpos = itTerm->second;
		}
		for (unsigned k = 0; k < coefvec.size() - 1; k++){
			if(k == smallpos){
				coefvec[k] *= (bigterm-targetterm)/(bigterm-smallterm);
			}
			else if(k == bigpos){
				coefvec[k] *= (targetterm-smallterm)/(bigterm-smallterm);
			}
			else{
				coefvec[k] = 0.0;
			}
		}
	}
	//set
	coeffs.set(coefvec);
//}
}

bool LALinearRatesSwapTradeValue::hasCashflow(const AQLObject& trade) const
{
    const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    for(size_t i = 0; i < legs.getSize(); i++){
        const AQLDataHolder* dh = &legs.get(i).getData(PRICING_DATA_CASHLETS);
        if(!dh->isDefined() || dh->isNull()) return false;
    }
    return true;
}

double
LALinearRatesSwapTradeValue::value(const AQLDate& basedate, AQLObject& object,
                                      const AQLDataValuation& att) const
{
    if(hasCashflow(object)){
        return do_value(basedate, object, att);
    }
    else{
        const double pv = 0;
        object.remove(PRICING_DATA_DIRTYPRICE);
        object.add(PRICING_DATA_DIRTYPRICE, new AQLDataDouble(pv));
        object.remove(PRICING_DATA_CLEANPRICE);
        object.add(PRICING_DATA_CLEANPRICE, new AQLDataDouble(pv));
		return pv;
    }
}

void
LALinearRatesSwapTradeValue::removeNotionalExposure(AQLObject& trade, AQLDataProvider* dp) const
{
	AQLDataHolder* dh = 0;

	dh = &trade.getData( CALIBRATION_DATA_UNDERLYINGS, NOCHECK);	
	if (!dh->isDefined() || dh->isNull())
	{
		return;
	}		

	AQLDataMultiReference* pLegs = dynamic_cast< AQLDataMultiReference* >(&dh->get());
	size_t sizeLeg = pLegs->getSize();
	std::vector<std::vector<double> > adjNotionalCF(sizeLeg);

	//check if MTM trade
	bool isRenotional = false;
	size_t renotionalLeg = 0;
	for ( size_t i = 0; i < pLegs->getSize(); i++ )
	{
		dh = &pLegs->get(i).getData(PRICING_DATA_ISRENOTIONAL, NOCHECK);
		if (!dh->isDefined() || dh->isNull())
		{
			continue;
		}
		else
		{
			if (dynamic_cast<AQLDataBool&>(dh->get()).get())
			{
				isRenotional = true;
				renotionalLeg = i;
			}
		}
	}

	// Modify notional CF by cashlets
	if (isRenotional)
	{
		// MtM case:
		// 1. remove all fixed MtM CF ( and first notional exchange nevertheless its fixing date)
		// 2. convert next coming MtM CF to a notional exchange at forward rate
		if (!dynamic_cast<AQLDataBool &>(trade.getData(CALIBRATION_DATA_ISRENOTIONALSETUPFINISHED, ISNOTNULL).get()).get())
		{
			throw AQLCoreInvalidData("Renotional CF Modification is not finished.",__FILE__,__LINE__);
		}

		const AQLDate asofDate = dynamic_cast<LALinearRatesSwapTradeValueDataProvider*>(dp)->pVanilla->getAsOfDate().get(); 

		// for renotional leg
		dh = &pLegs->get(renotionalLeg).getData(PRICING_DATA_CASHLETS, ISNOTNULL);
		AQLDataMultiReference* pCashlets = dynamic_cast< AQLDataMultiReference* >(&dh->get());
		size_t sizeCashlets = pCashlets->getSize();

		AQLDate newInitialExDate = dynamic_cast<AQLDataDate&>(pCashlets->get(sizeCashlets - 1).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
		dynamic_cast<AQLDataDouble&>(pCashlets->get(0).getData(PRICING_CALIBRATION_DATAOTIONALCF, ISNOTNULL).get()).set(0.);
		for(unsigned int j = 1; j < sizeCashlets; ++j)
		{
			AQLDate fixingDate = dynamic_cast<AQLDataDate&>(pCashlets->get(j).getData(PRICING_DATA_RENOTIONALFIXINGDATE, ISNOTNULL).get()).get();
			if (fixingDate <= asofDate)
			{
				// remove fixed MtM CF
				dh = &pCashlets->get(j).getData(PRICING_CALIBRATION_DATAOTIONALCF, ISNOTNULL);
				dynamic_cast<AQLDataDouble&>(dh->get()).set(0.);
			}
			else
			{
				// convert next MtM CF to notional exchange
				double notional = dynamic_cast<AQLDataDouble&>(pCashlets->get(j).getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
				dh = &pCashlets->get(j).getData(PRICING_CALIBRATION_DATAOTIONALCF, ISNOTNULL);
				double exValue = dynamic_cast<AQLDataDouble&>(dh->get()).get() - notional;
				dynamic_cast<AQLDataDouble&>(dh->get()).set(exValue);
				newInitialExDate = dynamic_cast<AQLDataDate&>(pCashlets->get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
				break;
			}
		}

		// for non renotional leg
		for (size_t i = 0; i < pLegs->getSize(); ++i)
		{
			if (i == renotionalLeg)
			{
				continue;
			}

			dh = &pLegs->get(i).getData(PRICING_DATA_CASHLETS, ISNOTNULL);
			pCashlets = dynamic_cast< AQLDataMultiReference* >(&dh->get());
			sizeCashlets = sizeCashlets = pCashlets->getSize();

			// first notional exchange nevertheless its fixing date
			dh = &pCashlets->get(0).getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				dynamic_cast<AQLDataDouble&>(dh->get()).set(0.);
			}

			bool isModified = false;
			unsigned int j = 1;
			while (!isModified && j < sizeCashlets)
			{
				AQLDate paymentDate = dynamic_cast<AQLDataDate&>(pCashlets->get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
				dh = &pCashlets->get(j).getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK);
				if(paymentDate == newInitialExDate)
				{
					double notional = dynamic_cast<AQLDataDouble&>(pCashlets->get(j).getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL).get()).get();
					double exValue = -notional;

					//20180529-Fixed: Fixed A Bug On The Last Notional CF.
					AQLDate lastPaymentDate = dynamic_cast<AQLDataDate&>(pCashlets->get(sizeCashlets - 1).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL).get()).get();
					if (lastPaymentDate == newInitialExDate)
					{
						exValue = 0;
					}

					pCashlets->get(j).get().remove(PRICING_CALIBRATION_DATAOTIONALCF);
					pCashlets->get(j).get().add(PRICING_CALIBRATION_DATAOTIONALCF, new AQLDataDouble(exValue));
					isModified = true;
				}
				else
				{
					if (dh->isDefined() && !dh->isNull())
					{
						throw AQLCoreInvalidData("Cannot determine wheather this MtM CF is already fixed.",__FILE__,__LINE__);
					}
				}
				++j;
			}
			if (!isModified)
			{
				throw AQLCoreInvalidData("Could not find the cashlets for notional CF modification.",__FILE__,__LINE__);
			}
		}
	}
	else
	{
		//Non-MtM case: remove all of Notional CF
		for (size_t i = 0; i < sizeLeg; ++i)
		{
			dh = &pLegs->get(i).getData(PRICING_DATA_CASHLETS, ISNOTNULL);
			AQLDataMultiReference* pCashlets = dynamic_cast< AQLDataMultiReference* >(&dh->get());
			
			//20170830-Fixed: Remove Notional CF for Cashlet0
			//for(size_t j = 1; j < pCashlets->getSize(); ++j)
			for(size_t j = 0; j < pCashlets->getSize(); ++j)						
			{
				dh = &pCashlets->get(j).getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					dynamic_cast<AQLDataDouble&>(dh->get()).set(0.);
				}
			}
		}
	}

}


/*!
@brief calculate fee value

@param[in] tradeEntity
@param[out] feePV
@side-effect adding data 	PRICING_DATA_PV_FEE, PRICING_DATA_PVVALUE_FEE, PRICING_DATA_DF_FEE, PRICING_DATA_TODAYFX_FEE
and stores to fee pv(summary) , fee pv (cashflow wise), discount factor used for calculation of fee, today FX from Fee currency to base currency.

*/
double 
LALinearRatesSwapTradeValue::calcFeeValueVanilla(AQLObject &tradeEntity,const AQLString& baseCurrency)
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
		dh = &tradeEntity.getData(CALIBRATION_DATA_FEEDISCOUNTCURVES, NOCHECK);
		const AQLStringVector& feeDiscountCurves = dynamic_cast<AQLDataStrings&>(dh->get()).get();

		// size check
		if (!(feePaymentDates.size() == feeSize &&  feeCurrencies.size() == feeSize && feeDiscountCurves.size() == feeSize))
		{
			throw AQLCoreInvalidData("[calcFeeValueVanilla] Size of Data: feeAmounts, feePaymentDates, feeCurrencies, feeDiscountCurves are not the same.", __FILE__, __LINE__);
		}

		DoubleArray feeValues(feeAmounts.size(), 0.);
		DoubleArray discountFactors(feeAmounts.size());
		DoubleArray todayFXRates(feeAmounts.size(), 1.);

		for (size_t i = 0; i < feeAmounts.size(); i++)
		{
			//Curve Set Up
			const AQLDataReference* pathEntity = NULL;
			// ! Check whether data is PathEntity or MarketParam		
			if (tradeEntity.getData(PRICING_DATA_PATHENTITY, NOCHECK).isDefined())
			{
				pathEntity = dynamic_cast<AQLDataReference *>(&tradeEntity.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
			}
			else if (tradeEntity.getData(PRICING_DATA_MARKETPARAM, NOCHECK).isDefined())
			{
				pathEntity = dynamic_cast<AQLDataReference *>(&tradeEntity.getData(PRICING_DATA_MARKETPARAM, ISNOTNULL).get());
			}
			else
			{
				throw AQLCoreInvalidData("[calcFeeValueVanilla] PathEntity is not set! MExoGridUtility::getCashflowStream", __FILE__, __LINE__);
			}

			const LAMathPlainVanillaEntity& vanillaEntity = dynamic_cast<const LAMathPlainVanillaEntity&>(pathEntity->get().get());

			//check fee is paid in the past.
			if (feePaymentDates[i] <= vanillaEntity.getAsOfDate().get())
			{
				continue;
			}
			const AQLInterpolationBase& discountCurve = vanillaEntity.getIRCurve(feeCurrencies[i]).getDFInterpolation(&feeDiscountCurves[i]);
			double baseTerm = vanillaEntity.getDayCount().getTerm(vanillaEntity.getAsOfDate().get(), feePaymentDates[i]);
			discountFactors[i] = discountCurve.value(baseTerm);

			feeValues[i] += discountFactors[i] * feeAmounts[i];

			// base currency

			//FX set Up
			if (feeCurrencies[i] != baseCurrency)
			{
				const AQLDataReference& fXEntityRef = vanillaEntity.getFXEntity();
				if (fXEntityRef.isNull())
				{
					throw AQLCoreInvalidData("[calcFeeValueVanilla] FXEntity is not defined.", __FILE__, __LINE__);
				}
				const LAMathFXEntity& fxEntity = dynamic_cast<const LAMathFXEntity&>(fXEntityRef.get().get());
				double todayFxRate = fxEntity.getRate(feeCurrencies[i], baseCurrency, 0.);

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


