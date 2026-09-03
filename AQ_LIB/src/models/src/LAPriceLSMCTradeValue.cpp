/*! @file
    @brief Source code for class to evaluate trade.

*/
//  2007, Mizuho International London..
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceLSMCTradeValue.h"
#include "LAPriceAccruedInterest.h"

#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADate.h"
#include "LADataValuation.h"
#include "LADataProcedure.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAObjectHolder.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataFunction.h"
#include "LAMathValuableEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathAttrSDE.h"
#include "LAMathIndexEntity.h"

#include "LARatesSDEBase.h"
#include "LAPolynomialBase.h"
#include "LAPolyFitBase.h"
#include "LAPolyFitLS.h"

#include "LAPricePayOff.h"
#include "LABasic.h"
#include <numeric>
#include <algorithm>
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"

using namespace std;


/*!
    @brief constructor
	@param[in] polyfit pointer to polynomial fitting class
	@param[in] pacc pointer to accured interest calculation class
*/
LAPriceLSMCTradeValue::LAPriceLSMCTradeValue(LAPolyFitBase* polyfit, LAPriceAccruedInterest* pacc) :
LAPriceTradeValue(pacc), mpPolyFit(polyfit)
{
}

/*!
    @brief copy constructor
	@param[in] v copy source 
*/
LAPriceLSMCTradeValue::LAPriceLSMCTradeValue(const LAPriceLSMCTradeValue& v)
: LAPriceTradeValue(v), mpPolyFit(0)
{
	if (v.mpPolyFit != 0)
		mpPolyFit = dynamic_cast<LAPolyFitBase*>(v.mpPolyFit->clone());
}


/*!
    @brief destructor

*/
LAPriceLSMCTradeValue::~LAPriceLSMCTradeValue()
{
	delete 	mpPolyFit;
}
/*!
    @brief  Check function for this class ID

	@param[in] id this class type(function_t type)
	@return true or false
*/
bool
LAPriceLSMCTradeValue::isTypeOf(function_t id) const
{
	return (id == FN_IR_LSMCTRADEVALUE ? true : LAPriceTradeValue::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

	@return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceLSMCTradeValue::clone() const
{
    try 
	{
    	return new LAPriceLSMCTradeValue(*this);	
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
LAPriceLSMCTradeValue::getType() const
{
	return FN_IR_LSMCTRADEVALUE;
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceLSMCTradeValue::registerData(LAPriceDataManager& dm) const
{
	LAPriceTradeValue::registerData(dm);

	dm.setData(PRICING_DATA_ISLSMCDETAILOUTPUT, DATA_BOOL);
	dm.setData(PRICING_DATA_LSMCREGCOEFFICIENTS, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCEXPLANATORYVARIABLES, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCEXPLAINEDVARIABLES, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_LSMCCALLPOSTJUDGE, DATA_BOOLS);
	dm.setData(PRICING_DATA_LSMCCALLJUDGE, DATA_BOOLS);


}
/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return clean price
	
*/
double
LAPriceLSMCTradeValue::value(const LADate& basedate, LAObject& object,
					const LADataValuation& att) const
{
	LADataHolder* dh;
	// call
	bool iscall = false;
	LAObject* pcallinfo = NULL;
	dh = &(object.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		iscall = true;
		LADataReference& attr = dynamic_cast<LADataReference&>(dh->get());		
		pcallinfo = &attr.get().get();
	}	
	else
		return LAPriceTradeValue::value(basedate, object, att);	

	
	//oeprator
	dh = &(pcallinfo->getData(PRICING_DATA_OPERATOR, NOCHECK));
	if (!dh->isDefined())
		dh = &pcallinfo->add(PRICING_DATA_OPERATOR, new LAPriceDataFunction());
	if (dh->isNull())
	{
		dynamic_cast<LAPriceDataFunction&>(dh->get()).setFunction(new LAPriceLSMCOperator(), "lsmcoperator");
		LAFunctionBase& method = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

		dh = &(pcallinfo->getData(PRICING_DATA_POLYNOMIAL, ISNOTNULL));
		LAFunctionBase& poly = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();

		dynamic_cast<LAPriceLSMCOperator&>(method).mPoly = &dynamic_cast<LAPolynomialBase&>(poly);

	}

	// setup dataProvider
	bool iscalcrisk = false;
	dh = &object.getData(PRICING_DATA_ISCALCRISK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		iscalcrisk = dynamic_cast<const LADataBool&>(dh->get()).get();
	att.setDataProvider(NULL);
	

	LAPriceTradeValueDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
	{
		//original currency
		dh = &(object.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
		LAString currency_ori = dynamic_cast<LADataString &>(dh->get()).get();
		dataProvider = dynamic_cast<LAPriceTradeValueDataProvider*>(setUpDataProvider(basedate, object, att));
		//return original currency
		dynamic_cast<LADataString&>(dh->get()).set(currency_ori);
	}
	else
		dataProvider = &dynamic_cast<LAPriceTradeValueDataProvider&>(att.getDataProvider());

	// setup
	dataProvider->pPath->setUpforMC();
	set<LAMathIndexEntity*>::iterator it;
	for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
		(*it)->setUpforMC();	

	if (dataProvider->lsmcnum == 0 || dataProvider->actiontimes.size() == 0)
		return LAPriceTradeValue::value(basedate, object, dataProvider, 0);	

	
	// prepare variables
	DoubleMatrix _explanatory(dataProvider->expirytimes.size());
	DoubleMatrix rebate(dataProvider->lsmcnum);
	DoubleMatrix numeraire_expirytime(dataProvider->lsmcnum);
	for (unsigned int i = 0; i < rebate.size(); i++)
	{
		rebate[i].resize(dataProvider->expirytimes.size());
		numeraire_expirytime[i].resize(dataProvider->expirytimes.size());
	}
	vector<DoubleMatrix> time1(dataProvider->lsmcnum), cf1(dataProvider->lsmcnum);
	vector<DoubleMatrix> numeraire_cftime(dataProvider->lsmcnum);
	vector<DoubleMatrix> explanatory(dataProvider->expirytimes.size()), transformed_explanatory;
	for (unsigned int i = 0; i < explanatory.size(); i++)
		explanatory[i].resize(dataProvider->lsmcnum);

	DoubleMatrix coefficient(dataProvider->expirytimes.size()), coefficient_rebate(dataProvider->expirytimes.size());
	BoolVector isconvert_xy(dataProvider->expirytimes.size(), false);
	vector<DoubleArray> standardization_y(2, DoubleArray(dataProvider->expirytimes.size(), 0.0));
	vector<DoubleMatrix> standardization_x(2, DoubleMatrix(dataProvider->expirytimes.size(), DoubleArray(0.0)));

	BoolVector isconvert_xy_rebate(dataProvider->expirytimes.size(), false);
	vector<DoubleArray> standardization_y_rebate(2, DoubleArray(dataProvider->expirytimes.size(), 0.0));
	vector<DoubleMatrix> standardization_x_rebate(2, DoubleMatrix(dataProvider->expirytimes.size(), DoubleArray(0.0)));
	DoubleArray numeraire_base(dataProvider->lsmcnum);
	IntArray calltiming;


    //setup lsmc exclude outlier parameter
    dynamic_cast<LAPolyFitLS*>(mpPolyFit)->setRegParam(dataProvider->isexcludeoutlier_y,
                                                       dataProvider->criteria_y,
                                                       dataProvider->isexcludeoutlier_x,
                                                       dataProvider->criteria_x,
                                                       dataProvider->isexcludeoutlier_e,
                                                       dataProvider->criteria_e,
													   dataProvider->isshift_y,
													   dataProvider->isscale_y,
													   dataProvider->isshift_x,
													   dataProvider->isscale_x,
													   dataProvider->svdTolerance
                                                      );

	//get sumulation curve name
	const LAString& originalNumeraire = dataProvider->pNumeraire->getBasisName();
	//set basis spread of numeraire
	if (!dataProvider->basisgrid_payoff.empty())
		dataProvider->pNumeraire->setBasisSpread(dataProvider->basisname_payoff, dataProvider->basisgrid_payoff, dataProvider->basisspread_payoff);
	/////////////////////////
	if (iscalcrisk)//calculating risk index case
	{
		double pv1 = 0.0, pv2 = 0.0;
		double pv2_tmp = 0.0;
		double accruedint_sum = 0.0;
		double accruedint_tmp = 0.0;
		DoubleMatrix time2, cf2;
		vector<pair<unsigned int, LADate> > triggerhit;
		
//		if (dataProvider->isstartable) reverseRcvPaySide(object);		
		//mc
		for (int i = 0; i < dataProvider->lsmcnum; i++)
		{
			dataProvider->pNumeraire->setBasisName(originalNumeraire);

			pv2_tmp = 0.0;
			accruedint_tmp = 0.0;
			dataProvider->pPath->setNextPath();
			
			for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
				(*it)->setNextIndex();
			
			if (!dataProvider->basisgrid_payoff.empty())
				dataProvider->pNumeraire->setBasisName(dataProvider->basisname_payoff);

			if (dataProvider->acc.isDefined() && !dataProvider->isstartable) //calculate accurued interest
			{
				const LAPriceAccruedInterest& acc = dynamic_cast<const LAPriceAccruedInterest&>(dataProvider->acc.get());
				const vector<PayOffToolHolderVector>& payoff_vec = dataProvider->payoff.getPayOff();
				for (unsigned int j = 0; j < payoff_vec.size(); j++)
				{
					if (dataProvider->rcvpay[j]) accruedint_tmp += acc.calcAccruedInterest(dataProvider->settledate, j);
					else accruedint_tmp -= acc.calcAccruedInterest(dataProvider->settledate, j);
				}
				if (dataProvider->baseterm == dataProvider->settle) accruedint_sum += accruedint_tmp;
				else accruedint_sum += accruedint_tmp * (*dataProvider->pNumeraire)(dataProvider->baseterm) / (*dataProvider->pNumeraire)(dataProvider->settle);
			}			
			
			if (dataProvider->iscalcswap) dataProvider->payoff.calcPayOff(time1[i], cf1[i], false, triggerhit, false);
			dataProvider->payoff.calcPayOff(time1[i], cf1[i], true, triggerhit, /*false, */true, &rebate[i], &_explanatory);
			for (unsigned int j = 0; j < _explanatory.size(); j++)
				explanatory[j][i] = _explanatory[j];


			numeraire_cftime[i].resize(time1[i].size());
			for (unsigned int j = 0; j < time1[i].size(); j++)
			{
				numeraire_cftime[i][j].resize(time1[i][j].size());
				for (unsigned int k = 0; k < time1[i][j].size(); k++)
					numeraire_cftime[i][j][k] =  (*dataProvider->pNumeraire)(time1[i][j][k]);
			}
		
			for (unsigned int j = 0; j < numeraire_expirytime[i].size(); j++)
				numeraire_expirytime[i][j] =  (*dataProvider->pNumeraire)(dataProvider->expirytimes[j]);
			numeraire_base[i] = (*dataProvider->pNumeraire)(dataProvider->baseterm);

			if (!dataProvider->isstartable) continue; 


			for (unsigned int j = 0; j < time1[i].size(); j++)
				for (unsigned int k = 0; k < time1[i][j].size(); k++)
					if (time1[i][j][k] > dataProvider->settle)
					{
						if (dataProvider->rcvpay[j])	pv2_tmp += cf1[i][j][k] / (*dataProvider->pNumeraire)(time1[i][j][k]);
						else pv2_tmp -= cf1[i][j][k] / (*dataProvider->pNumeraire)(time1[i][j][k]);
					}

			pv2_tmp *= (*dataProvider->pNumeraire)(dataProvider->baseterm);
			pv2 += pv2_tmp;
		}
		
		pv1 = value_backward(time1, cf1, numeraire_cftime, rebate, numeraire_expirytime, 
					explanatory, numeraire_base, dataProvider, coefficient, coefficient_rebate, 
					isconvert_xy, standardization_y, standardization_x, isconvert_xy_rebate, standardization_y_rebate, standardization_x_rebate, calltiming, dataProvider->isstartable);
		dataProvider->coefficient->set(coefficient);
		dataProvider->coefficient_rebate->set(coefficient_rebate);
		setStandardizationParam(*dataProvider, isconvert_xy, standardization_y, standardization_x, isconvert_xy_rebate, standardization_y_rebate, standardization_x_rebate);

		if (dataProvider->mcnum == 0) return 0.0;

		double pv;
		if (!dataProvider->isstartable)
			pv = pv1;
		else 
			pv = -pv1 + pv2 / double(dataProvider->lsmcnum);

		// value term
		double ratio = calcValueTermRatio(*dataProvider);
		pv *= ratio;

		if (dataProvider->basecur != dataProvider->numerairecur)
			pv *= getFXEntity(object).getRate(dataProvider->numerairecur, dataProvider->basecur, dataProvider->baseterm);
	
		dataProvider->pNumeraire->setBasisName(originalNumeraire);
		if (dataProvider->actiontimes[0] <= dataProvider->settle)
		{
			dataProvider->pPath->setUpforMC();
			set<LAMathIndexEntity*>::iterator it;
			for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
				(*it)->setUpforMC();				
			return LAPriceTradeValue::value(basedate, object, dataProvider, 0);
		}
		else
		{
			if (!dataProvider->isoptionholder) pv *= -1.0;
			if (dataProvider->lsmcnum < dataProvider->mcnum)
			{
				pv = pv * double(dataProvider->lsmcnum)
					+ LAPriceTradeValue::value(basedate, object, dataProvider, dataProvider->lsmcnum) * double(dataProvider->mcnum - dataProvider->lsmcnum);
				pv /= double(dataProvider->mcnum);
			}
			return pv;
		}
	}
	//////////////////////////////////


	// trigger
	bool istrigger = false;
	dh = &(object.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
		istrigger = true;
	
	// isdetail output
	bool isdetailoutput = false;
	dh = &(object.getData(PRICING_DATA_ISDETAILOUTPUT, ISNOTNULL));
	isdetailoutput = dynamic_cast<const LADataBool&>(dh->get()).get();
	// islsmcdetail output
	bool islsmcdetailoutput = false;
	dh = &(object.getData(PRICING_DATA_ISLSMCDETAILOUTPUT, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		islsmcdetailoutput = dynamic_cast<const LADataBool&>(dh->get()).get();
	if (dataProvider->actiontimes[0] <= dataProvider->settle || istrigger || dataProvider->lsmcnum < dataProvider->mcnum)
		islsmcdetailoutput = false;
	
	BoolVector postjudge, judge;
	DoubleMatrix lsmcparam, explained, lsmcparam_rebate, transformed_explained;

	DoubleArray pv1_vec(dataProvider->lsmcnum), pv2_vec, pv3_vec;
	if (isdetailoutput || dataProvider->iscalcswap) pv2_vec.resize(dataProvider->lsmcnum);
	if (dataProvider->isstartable) pv3_vec.resize(dataProvider->lsmcnum);
	double pv1 = 0.0, pv2 = 0.0, pv3 = 0.0;
	double pv1_square = 0.0;
	double accruedint_sum = 0.0;	
	double accruedint_sum2 = 0.0;	
	DoubleArray accruedint(dataProvider->lsmcnum, 0);
	DoubleMatrix time2, cf2;
	vector<pair<unsigned int, LADate> > triggerhit;
	vector<map<LADate, unsigned int> > counter_action(1);

	dh = &(pcallinfo->getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
	const DateVector& callactiondates = dynamic_cast<const LADataDates&>(dh->get()).get();
	for (unsigned int i = 0; i < callactiondates.size(); i++)
		counter_action[0][callactiondates[i]] = 0;

	// mc 
	for (int i = 0; i < dataProvider->lsmcnum; i++)
	{
		dataProvider->pNumeraire->setBasisName(originalNumeraire);
		dataProvider->pPath->setNextPath();
		for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
			(*it)->setNextIndex();

		if (!dataProvider->basisgrid_payoff.empty())
			dataProvider->pNumeraire->setBasisName(dataProvider->basisname_payoff);

		if (dataProvider->acc.isDefined() && !dataProvider->isstartable) //calculate accurued interest
		{
			
			const LAPriceAccruedInterest& acc = dynamic_cast<const LAPriceAccruedInterest&>(dataProvider->acc.get());
			const vector<PayOffToolHolderVector>& payoff_vec = dataProvider->payoff.getPayOff();
			for (unsigned int j = 0; j < payoff_vec.size(); j++)
			{
				if (dataProvider->rcvpay[j]) accruedint[i] += acc.calcAccruedInterest(dataProvider->settledate, j);
				else accruedint[i] -= acc.calcAccruedInterest(dataProvider->settledate, j);
			}
			accruedint_sum += accruedint[i];
			if (dataProvider->baseterm != dataProvider->settle) 
			{
				accruedint[i] *= (*dataProvider->pNumeraire)(dataProvider->baseterm) / (*dataProvider->pNumeraire)(dataProvider->settle);	
				accruedint_sum2 += accruedint[i];
			}
			else
				accruedint_sum2 = accruedint_sum;
		}
		if (isdetailoutput || dataProvider->iscalcswap)
		{
			dataProvider->payoff.calcPayOff(time2, cf2, false, triggerhit, false);
			for (unsigned int j = 0; j < time2.size(); j++)
				for (unsigned int k = 0; k < time2[j].size(); k++)
					if (time2[j][k] > dataProvider->settle)
					{
						if (dataProvider->rcvpay[j])	pv2_vec[i] += cf2[j][k] / (*dataProvider->pNumeraire)(time2[j][k]);
						else pv2_vec[i] -= cf2[j][k] / (*dataProvider->pNumeraire)(time2[j][k]);
					}
			pv2_vec[i] *= (*dataProvider->pNumeraire)(dataProvider->baseterm);
			pv2 += pv2_vec[i];
		}

		dataProvider->payoff.calcPayOff(time1[i], cf1[i], true, triggerhit, /*false, */true, &rebate[i], &_explanatory);
		
		for (unsigned int j = 0; j < _explanatory.size(); j++)
			explanatory[j][i] = _explanatory[j];

		numeraire_cftime[i].resize(time1[i].size());
		for (unsigned int j = 0; j < time1[i].size(); j++)
		{
			numeraire_cftime[i][j].resize(time1[i][j].size());
			for (unsigned int k = 0; k < time1[i][j].size(); k++)
				numeraire_cftime[i][j][k] =  (*dataProvider->pNumeraire)(time1[i][j][k]);
		}
		for (unsigned int j = 0; j < numeraire_expirytime[i].size(); j++)
			numeraire_expirytime[i][j] =  (*dataProvider->pNumeraire)(dataProvider->expirytimes[j]);
		numeraire_base[i] = (*dataProvider->pNumeraire)(dataProvider->baseterm);
	
	
		if (!isdetailoutput && !dataProvider->isstartable) continue;

		if (dataProvider->isstartable)
		{
			if (!isdetailoutput || istrigger)
			{		
				for (unsigned int j = 0; j < time1[i].size(); j++)
					for (unsigned int k = 0; k < time1[i][j].size(); k++)
						if (time1[i][j][k] > dataProvider->settle)
						{
							if (dataProvider->rcvpay[j])	pv3_vec[i] += cf1[i][j][k] / (*dataProvider->pNumeraire)(time1[i][j][k]);
							else pv3_vec[i] -= cf1[i][j][k] / (*dataProvider->pNumeraire)(time1[i][j][k]);
						}
				pv3_vec[i] *= (*dataProvider->pNumeraire)(dataProvider->baseterm);
			}
			else 
				pv3_vec[i] = pv2_vec[i];
				
			pv3 += pv3_vec[i];
		}
	}

	if (islsmcdetailoutput)
	{
		postjudge.resize(dataProvider->actiontimes.size() * dataProvider->lsmcnum, false);
		judge.resize(dataProvider->actiontimes.size() * dataProvider->lsmcnum, false);
		lsmcparam.resize(dataProvider->actiontimes.size());
		explained.resize(dataProvider->actiontimes.size());
		lsmcparam_rebate.resize(dataProvider->actiontimes.size());
		transformed_explained.resize(dataProvider->actiontimes.size());
		transformed_explanatory.resize(dataProvider->actiontimes.size());
		pv1 = value_backward(time1, cf1, numeraire_cftime, rebate, numeraire_expirytime, 
			explanatory, numeraire_base, dataProvider, coefficient, coefficient_rebate, isconvert_xy, standardization_y, standardization_x, isconvert_xy_rebate, standardization_y_rebate, standardization_x_rebate, calltiming, 
			dataProvider->isstartable, false, &pv1_vec, &postjudge, &judge, &lsmcparam, &explained, &lsmcparam_rebate, &transformed_explained, &transformed_explanatory);
	}
	else
		pv1 = value_backward(time1, cf1, numeraire_cftime, rebate, numeraire_expirytime, 
			explanatory, numeraire_base, dataProvider, coefficient, coefficient_rebate, isconvert_xy, standardization_y, standardization_x, isconvert_xy_rebate, standardization_y_rebate, standardization_x_rebate, calltiming, dataProvider->isstartable, false, &pv1_vec);
	
	
	dataProvider->coefficient->set(coefficient);
	dataProvider->coefficient_rebate->set(coefficient_rebate);
	setStandardizationParam(*dataProvider, isconvert_xy, standardization_y, standardization_x, isconvert_xy_rebate, standardization_y_rebate, standardization_x_rebate);

	if (dataProvider->mcnum == 0) return 0.0;

	for (int i = 0; i < dataProvider->lsmcnum; i++)
	{
		if (!dataProvider->isstartable) pv1_square += (pv1_vec[i] - accruedint[i]) * (pv1_vec[i] - accruedint[i]);
		else pv1_square += (-pv1_vec[i] + pv3_vec[i]) * (-pv1_vec[i] + pv3_vec[i]);
	}
	
	
	for (unsigned int i = 0; i < calltiming.size(); i++)
	{
		//if (calltiming[i] != -1) counter_action[0][callactiondates[calltiming[i]]] += 1;
		if (calltiming[i] != -1) counter_action[0][dataProvider->actiondates[calltiming[i]]] += 1;
	}

	if (dataProvider->actiontimes[0] <= dataProvider->settle || istrigger || dataProvider->lsmcnum < dataProvider->mcnum)
	{
		dataProvider->pPath->setUpforMC();
		set<LAMathIndexEntity*>::iterator it;
		for (it = dataProvider->indexs.begin(); it != dataProvider->indexs.end(); it++)
			(*it)->setUpforMC();				
		dataProvider->pNumeraire->setBasisName(originalNumeraire);
		return LAPriceTradeValue::value(basedate, object, dataProvider, 0);		
	}

	// Calculate Fee Value
	double pvFee = calcFeeValueExo(object, dataProvider);

	//////////
	//output//
	//////////
	pv2 /= double(dataProvider->lsmcnum);
	pv3 /= double(dataProvider->lsmcnum);
	pv1_square /= double(dataProvider->lsmcnum);

	// value term
	double ratio = calcValueTermRatio(*dataProvider);
	pv1 *= ratio;
	pv2 *= ratio;
	pv3 *= ratio;
	pv1_square *= ratio * ratio;

	double cleanprice1, cleanprice2, dirtyprice1;
	if (dataProvider->isstartable)
	{
		cleanprice1 = -pv1 + pv3;
		cleanprice2 = pv2;
		dirtyprice1 = cleanprice1;
	}
	else
	{
		cleanprice1 = pv1 - accruedint_sum2 / double(dataProvider->lsmcnum);
		cleanprice2 = pv2 - accruedint_sum2 / double(dataProvider->lsmcnum);
		dirtyprice1 = pv1;
	}

	if (dataProvider->basecur != dataProvider->numerairecur)
	{
		double fxrate = getFXEntity(object).getRate(dataProvider->numerairecur, dataProvider->basecur, dataProvider->baseterm);
		cleanprice1 *= fxrate;
		cleanprice2 *= fxrate;
		dirtyprice1 *= fxrate;
		accruedint_sum *= fxrate;
		pv1_square *= fxrate * fxrate;
		
		
	}
	
	double pv1_deviation = LAMath::sqrt(LAMath::max((pv1_square - cleanprice1 * cleanprice1), 0.0) / dataProvider->lsmcnum);

	// check option holder
	if (!dataProvider->isoptionholder)
	{
		cleanprice1 *= -1.0;
		cleanprice2 *= -1.0;
		dirtyprice1 *= -1.0;
		accruedint_sum *= -1.0;
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
	
	object.add(PRICING_DATA_CLEANPRICE, new LADataDouble(cleanprice1));
	object.add(PRICING_DATA_DIRTYPRICE, new LADataDouble(dirtyprice1));
	object.add(PRICING_DATA_ACCRUEDINTEREST, new LADataDouble(accruedint_sum / double(dataProvider->lsmcnum)));
	object.add(PRICING_DATA_CLEANPRICESQUARE, new LADataDouble(pv1_square));
	object.add(PRICING_DATA_CLEANPRICEDEVIATION, new LADataDouble(pv1_deviation));
	object.add(PRICING_DATA_PVCURRENCY, new LADataString(dataProvider->basecur));
	object.add(PRICING_DATA_FEE_EXCLUDED_PV, new LADataDouble(feeExcludedPV));

	if (isdetailoutput)
	{
		object.add(PRICING_DATA_CALLTRIGGERVALUE, new LADataDouble(cleanprice1 - cleanprice2));
		object.add(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER, new LADataDouble(cleanprice2));
	}
	//action probabilities
	DoubleArray probs(counter_action[0].size());
	map<LADate, unsigned int>::const_iterator it2;
	unsigned int count = 0;
	for (it2 = counter_action[0].begin(); it2 != counter_action[0].end(); it2++)
		probs[count++] = (double)it2->second / (double)dataProvider->lsmcnum;
		
	pcallinfo->remove(PRICING_DATA_ACTIONPROBABILITIES);
	pcallinfo->add(PRICING_DATA_ACTIONPROBABILITIES, new LADataDoubles(probs));
	//average life 
	double aveLife = 0.;
	double prob_sum = 0.;
	//call
	for (it2 = counter_action[0].begin(); it2 != counter_action[0].end(); it2++)
	{
		prob_sum += (double)it2->second / (double)dataProvider->lsmcnum;
		aveLife += dataProvider->pPath->getDayCount().getTerm(basedate, it2->first) * (double)it2->second / (double)dataProvider->lsmcnum;
	}
	//trigger
	for (unsigned int i = 1; i < counter_action.size(); i++)
	{
		for (it2 = counter_action[1 + i].begin(); it2 != counter_action[1 + i].end(); ++it2)
		{
			prob_sum += (double)it2->second / (double)dataProvider->lsmcnum;
			aveLife += dataProvider->pPath->getDayCount().getTerm(basedate, it2->first) * (double)it2->second / (double)dataProvider->lsmcnum;
		}
	}
	//last payment
	DoubleVector tmpLastPayment;
	for (unsigned int i = 0; i < dataProvider->paytimes.size(); ++i)
	{
		tmpLastPayment.push_back(dataProvider->paytimes[i].back());
	}
	double lastPaymentTerm = *max_element(tmpLastPayment.begin(), tmpLastPayment.end());
	aveLife += lastPaymentTerm * (1. - prob_sum);
	object.remove(PRICING_DATA_AVERAGELIFE);
	object.add(PRICING_DATA_AVERAGELIFE, new LADataDouble(aveLife));
	//lsmc detail
	pcallinfo->remove(PRICING_DATA_LSMCREGCOEFFICIENTS);
	pcallinfo->remove(PRICING_DATA_LSMCEXPLANATORYVARIABLES);
	pcallinfo->remove(PRICING_DATA_LSMCEXPLAINEDVARIABLES);
	pcallinfo->remove(PRICING_DATA_LSMCCALLPOSTJUDGE);
	pcallinfo->remove(PRICING_DATA_LSMCCALLJUDGE);
	pcallinfo->remove(PRICING_DATA_LSMCREGCOEFFICIENTS2);
	pcallinfo->remove(PRICING_DATA_CALLREBATE);
	if (islsmcdetailoutput)
	{
		pcallinfo->add(PRICING_DATA_LSMCREGCOEFFICIENTS, new LADataDoubleMatrix(lsmcparam));
		pcallinfo->add(PRICING_DATA_LSMCREGCOEFFICIENTS2, new LADataDoubleMatrix(lsmcparam_rebate));

		const DoubleMatrix *p_explained = 0;
		const vector<DoubleMatrix> *p_explanatory = 0;
		if (dataProvider->isshift_y || dataProvider->isshift_x)
		{
			p_explained = &transformed_explained;
			p_explanatory = &transformed_explanatory;
		}
		else
		{
			p_explained = &explained;
			p_explanatory = &explanatory;
		}
		unsigned int size1 = p_explanatory->size();
		unsigned int size2 = p_explanatory->begin()->size();
		unsigned int size3 = 0;
		for (unsigned int i = 0; i < size1; i++)
		{
			for (unsigned int j = 0; j < size2; j++)
			{
				size3 = (*p_explanatory)[i][j].size();
				if (size3 != 0) break;
			}
			if (size3 != 0) break;
		}
		DoubleMatrix exvar(size1, DoubleArray(size2 * size3));
		for (unsigned int i = 0; i < size1; i++)
			for (unsigned int j = 0; j < size2; j++)
				for (unsigned int k = 0; k < size3; k++)
					exvar[i][j * size3 + k] = (*p_explanatory)[i][j][k];

		size1 = rebate[0].size();
		size2 = rebate.size();
		DoubleMatrix rebate_trans(size1, DoubleArray(size2));
		for (unsigned int i = 0; i < size1; i++)
			for (unsigned int j = 0; j < size2; j++)
				rebate_trans[i][j] = rebate[j][i];

		pcallinfo->add(PRICING_DATA_LSMCEXPLANATORYVARIABLES, new LADataDoubleMatrix(exvar));
		pcallinfo->add(PRICING_DATA_LSMCEXPLAINEDVARIABLES, new LADataDoubleMatrix(*p_explained));
		pcallinfo->add(PRICING_DATA_LSMCCALLPOSTJUDGE, new LADataBools(postjudge));
		pcallinfo->add(PRICING_DATA_LSMCCALLJUDGE, new LADataBools(judge));
		pcallinfo->add(PRICING_DATA_CALLREBATE, new LADataDoubleMatrix(rebate_trans));
	}


	return dirtyprice1;

}

/*!
	@brief value by backward

	@param[in] time cf time of non call case
	@param[in] cf cf of non call case
	@param[in] numeraire_cftime numeraire value of cf time
	@param[in] rebate rebate
	@param[in] numeraire_expirytime numeraire value of cf call expirytime
	@param[in] explanatory explanatory of LSMC
	@param[in] numeraire_base numeraire value of basetime
	@param[in] dataProvider cache object
	@param[out] coefficient LSMC regression coefficient
	@param[out] isconvert_xy
	@param[out] standardization_y
	@param[out] standardization_x
	@param[out] isconvert_xy_rebate
	@param[out] standardization_y_rebate
	@param[out] standardization_x_rebate
	@param[out] calltiming call timing
	@param[in] isstartalble flag of startalble option or cancelable option
	@param[in] iscalcrisk flag of calulating risk index or not
	@param[out] pvs pv of each path
	@param[out] afterjudge 
	@param[out] judge 
	@param[out] lsmcparam 
	@param[out] explainedvar 

	@return dirty price	
*/
double
LAPriceLSMCTradeValue::value_backward(const vector<DoubleMatrix>& time,
									const vector<DoubleMatrix>& cf,
									const vector<DoubleMatrix>& numeraire_cftime,
									const DoubleMatrix& rebate,
									const DoubleMatrix& numeraire_expirytime,
									const vector<DoubleMatrix>& explanatory,
									const DoubleArray& numeraire_base,
									const LADataProvider* dataProvider,
									DoubleMatrix& coefficient,
									DoubleMatrix& coefficient_rebate,
									BoolVector &isconvert_xy,
									std::vector<DoubleArray>& standardization_y,
									std::vector<DoubleMatrix>& standardization_x,
									BoolVector &isconvert_xy_rebate,
									std::vector<DoubleArray>& standardization_y_rebate,
									std::vector<DoubleMatrix>& standardization_x_rebate,
									IntArray& calltiming,
									bool isstartable,
									bool iscalcrisk,									
									DoubleArray* pvs,
									BoolVector* afterjudge,
									BoolVector* judge,
									DoubleMatrix* lsmcparam,
									DoubleMatrix* explainedvar,
									DoubleMatrix* lsmcparam_rebate,
									DoubleMatrix* transformed_explainedvar,
									std::vector<DoubleMatrix>* transformed_explanatory) const
{
	const LAPriceTradeValueDataProvider* dataProvider_ = dynamic_cast<const LAPriceTradeValueDataProvider*>(dataProvider);

	vector<vector<DoubleArray::const_iterator> > it_time(time.size()), it_cf(cf.size()), it_numeraire(numeraire_cftime.size());
	for (unsigned int j = 0; j < time.size(); j++)
	{
		it_time[j].resize(time[j].size());
		it_cf[j].resize(cf[j].size());
		it_numeraire[j].resize(numeraire_cftime[j].size());
		for (unsigned int k = 0; k < time[j].size(); k++)
		{
			it_time[j][k] = time[j][k].end();	
			it_cf[j][k] = cf[j][k].end();
			it_numeraire[j][k] = numeraire_cftime[j][k].end();
//			it_time[j][k]--;
//			it_cf[j][k]--;
//			it_numeraire[j][k]--;
		}				
	}

	calltiming.resize(time.size(), -1);//mc path number

	DoubleArray pv_vec(time.size(), 0);
	DoubleArray pv_vec_predict(time.size(), 0);
	DoubleArray rebate_predict(time.size(), 0);
	DoubleArray explained(time.size(), 0);
	DoubleArray rebate_vec(time.size(), 0);
	DoubleMatrix pv_mat(time.size());
	DoubleMatrix nu_mat(time.size());
	BoolVector is_ITM(time.size());
	for (unsigned int j = 0; j < time.size(); j++)
	{
		pv_mat[j].resize(time[j].size(), 0);
		nu_mat[j].resize(time[j].size(), 1);
	}	

	const unsigned int actionSize = dataProvider_->actiontimes.size();
	int i = static_cast<int>(actionSize) - 1;
	const unsigned int pathnum = time.size();
	if (isconvert_xy.size() != actionSize || isconvert_xy_rebate.size() != actionSize)
	{
		throw LACoreInvalidData("isconvert_xy vector size must be same as action size.", __FILE__, __LINE__);
	}

	if (standardization_y.size() != 2 || standardization_x.size() != 2 ||
		standardization_y_rebate.size() != 2 || standardization_x_rebate.size() != 2)
	{
		throw LACoreInvalidData("Standardization vector size must be 2. First element is shiftvalue. Seconde element is scalevalue.", __FILE__, __LINE__);
	}
	if (standardization_y[0].size() != actionSize || standardization_y[1].size() != actionSize ||  
		standardization_x[0].size() != actionSize || standardization_x[1].size() != actionSize ||
		standardization_y_rebate[0].size() != actionSize || standardization_y_rebate[1].size() != actionSize ||
		standardization_x_rebate[0].size() != actionSize || standardization_x_rebate[1].size() != actionSize)
	{
		throw LACoreInvalidData("Each standardization vector size must be same as action size.", __FILE__, __LINE__);
	}

	for (; i >= 0; i--)//i:action time suffix
	{
		bool isRebate = false;
		for (unsigned int j = 0; j < pathnum; j++)//j:mc path suffix
		{
			if (i != static_cast<int>(actionSize) - 1)
			{
				pv_vec[j] *= numeraire_expirytime[j][i] / numeraire_expirytime[j][i + 1];
#ifdef __ITM_CHECK__			
				pv_vec_predict[j] *= numeraire_expirytime[j][i] / numeraire_expirytime[j][i + 1];
#endif
			}
			for (unsigned int k = 0; k < time[j].size(); k++)//k:leg suffix
			{
				pv_mat[j][k] = 0;
				for(; it_time[j][k] != time[j][k].begin() && *(it_time[j][k] - 1) > dataProvider_->actiontimes[i]; it_time[j][k]--)
				{
					it_cf[j][k]--;
					it_numeraire[j][k]--;
					pv_mat[j][k] = pv_mat[j][k] * (*it_numeraire[j][k]) / nu_mat[j][k] + (*it_cf[j][k]);
					nu_mat[j][k] = (*it_numeraire[j][k]);
				}
				pv_mat[j][k] *= numeraire_expirytime[j][i] / nu_mat[j][k];
				nu_mat[j][k] = numeraire_expirytime[j][i];
#ifdef __ITM_CHECK__
				if (dataProvider_->rcvpay[k] && !isstartable) {pv_vec[j] += pv_mat[j][k]; pv_vec_predict[j] += pv_mat[j][k];}
				else if (!dataProvider_->rcvpay[k] && isstartable) {pv_vec[j] += pv_mat[j][k]; pv_vec_predict[j] += pv_mat[j][k];}
				else if (dataProvider_->rcvpay[k] && isstartable) {pv_vec[j] -= pv_mat[j][k]; pv_vec_predict[j] -= pv_mat[j][k];}
				else {pv_vec[j] -= pv_mat[j][k]; pv_vec_predict[j] -= pv_mat[j][k];}
#else
				if (dataProvider_->rcvpay[k] && !isstartable) {pv_vec[j] += pv_mat[j][k];}
				else if (!dataProvider_->rcvpay[k] && isstartable) {pv_vec[j] += pv_mat[j][k];}
				else if (dataProvider_->rcvpay[k] && isstartable) {pv_vec[j] -= pv_mat[j][k];}
				else {pv_vec[j] -= pv_mat[j][k]; pv_vec_predict[j] -= pv_mat[j][k];}
#endif

			}
#ifdef __ITM_CHECK__			
			if (!isstartable && pv_vec_predict[j] >= rebate[j][i])
			{
				is_ITM[j] = false;
				explained[j] = rebate[j][i];
			}
			else if (isstartable && pv_vec_predict[j] >= -rebate[j][i])
			{
				is_ITM[j] = false;
				explained[j] = -rebate[j][i];
			}
			else
			{
				explained[j] = pv_vec[j];
				is_ITM[j] = true;
			}
#else
			explained[j] = pv_vec[j];
			is_ITM[j] = true;
#endif
			if ((rebate_vec[j] = rebate[j][i]) != 0.0) isRebate = true;
		}

		//regress
		if (transformed_explainedvar && transformed_explanatory)
		{
			mpPolyFit->regress(explained, explanatory[i], *dataProvider_->polies[i], &(*transformed_explainedvar)[i], &(*transformed_explanatory)[i]);
		}
		else
		{
			mpPolyFit->regress(explained, explanatory[i], *dataProvider_->polies[i]);
		}
		
		const DoubleArray& param = dataProvider_->polies[i]->getParam();
		if (lsmcparam != 0) (*lsmcparam)[i] = param;
		if (explainedvar != 0) (*explainedvar)[i] = explained;

		coefficient[i].resize(param.size() + 1);
		for (unsigned int j = 0; j < coefficient[i].size() -1; j++)
			coefficient[i][j] = -param[j];
		if (isstartable) coefficient[i].back() = -1;
		else coefficient[i].back() = 1;
		dataProvider_->polies[i]->getStandardizationParam(standardization_y[0][i], standardization_y[1][i], standardization_x[0][i], standardization_x[1][i]);
		isconvert_xy[i] = dataProvider_->polies[i]->isConvert();

		if (isRebate)
			mpPolyFit->regress(rebate_vec, explanatory[i], *dataProvider_->polies_rebate[i]);
		else
		{
			DoubleArray param_tmp(param.size(), 0.0);
			dataProvider_->polies_rebate[i]->setParam(param_tmp);
			for (unsigned int j = 0; j < explanatory[i].size(); j++)
			{
				if (explanatory[i][j].size() == 0) continue;
				dataProvider_->polies_rebate[i]->setNumVar(explanatory[i][j].size());
				break;
			}
		}
		const DoubleArray& param_rebate = dataProvider_->polies_rebate[i]->getParam();
		if (lsmcparam_rebate != 0)  (*lsmcparam_rebate)[i] = param_rebate;
		coefficient_rebate[i].resize(param_rebate.size() + 1);
		for (unsigned int j = 0; j < coefficient_rebate[i].size() -1; j++)
			coefficient_rebate[i][j] = -param_rebate[j];
		if (isstartable) coefficient_rebate[i].back() = -1;
		else coefficient_rebate[i].back() = 1;
		dataProvider_->polies_rebate[i]->getStandardizationParam(standardization_y_rebate[0][i], standardization_y_rebate[1][i], standardization_x_rebate[0][i], standardization_x_rebate[1][i]);
		isconvert_xy_rebate[i] = dataProvider_->polies_rebate[i]->isConvert();

		for (unsigned int j = 0; j < pathnum; j++)//j:mc path suffix
		{
			if (explanatory[i][j].size() == 0)
			{
				pv_vec_predict[j] = 0.0;
				continue;
			}			
			if (afterjudge != 0)
			{
				if (is_ITM[j] &&
					((!isstartable && pv_vec[j] < rebate[j][i])
					|| (isstartable && pv_vec[j] < -rebate[j][i])))
					(*afterjudge)[i * pathnum + j] = true;							
			}			
			pv_vec_predict[j] = (*dataProvider_->polies[i])(explanatory[i][j]);
			rebate_predict[j] = (*dataProvider_->polies_rebate[i])(explanatory[i][j]);

			if (is_ITM[j] &&
				((!isstartable && pv_vec_predict[j] < rebate_predict[j])
				|| (isstartable && pv_vec_predict[j] < -rebate_predict[j]))) 
			{
				if (isstartable) pv_vec[j] = -rebate[j][i];			
				else pv_vec[j] = rebate[j][i];	
				pv_vec_predict[j] = pv_vec[j];
				//record call timing
				calltiming[j] = i;
				if (judge != 0) (*judge)[i * pathnum + j] = true;
			}
		}
	}


	if (numeraire_expirytime[0].size() != 0)
		for (unsigned int j = 0; j < time.size(); j++)
			pv_vec[j] *= numeraire_base[j] / numeraire_expirytime[j][0];
			
	for (unsigned int j = 0; j < time.size(); j++)//j:mc path suffix
	{
		for (unsigned int k = 0; k < time[j].size(); k++)//k:leg suffix
		{
			pv_mat[j][k] = 0;
			for(; it_time[j][k] != time[j][k].begin() && *(it_time[j][k] - 1) > dataProvider_->settle; it_time[j][k]--)
			{
				it_cf[j][k]--;
				it_numeraire[j][k]--;
				pv_mat[j][k] = pv_mat[j][k] * (*it_numeraire[j][k]) / nu_mat[j][k] + (*it_cf[j][k]);
				nu_mat[j][k] = (*it_numeraire[j][k]);
			}
			pv_mat[j][k] *= numeraire_base[j] / nu_mat[j][k];

			if (dataProvider_->rcvpay[k] && !isstartable) pv_vec[j] += pv_mat[j][k];
			else if (!dataProvider_->rcvpay[k] && isstartable) pv_vec[j] += pv_mat[j][k];
			else if (dataProvider_->rcvpay[k] && isstartable) pv_vec[j] -= pv_mat[j][k];
			else pv_vec[j] -= pv_mat[j][k];

		}			
	}

	double pv = 0;
	for (unsigned int j = 0; j < time.size(); j++)
	{
        if (isstartable) pv_vec[j] = -pv_vec[j];
		pv += pv_vec[j];
		if (!iscalcrisk) (*pvs)[j] = pv_vec[j];
	}

	return pv / (double)time.size();
}



/*!
	@brief setup cache class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cache class
	
*/
/*LADataProvider*
LAPriceLSMCTradeValue::setUpDataProvider(const LADate& basedate, LAObject& object, 
								const LADataValuation& att) const
{
	LADataHolder* dh;

	LAPriceTradeValue::setUpDataProvider(basedate, object, att);
	LAPriceTradeValueDataProvider* dataProvider = &dynamic_cast<LAPriceTradeValueDataProvider&>(att.getDataProvider());
	
	//call info
	dh = &(object.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		dataProvider->payoff.setLSMCMode(false);
		return dataProvider;
	}
	
	LADataReference& call = dynamic_cast<LADataReference&>(dh->get());
	LAObjectHolder& objHolder = call.get();

	//LSMC num
	dh = &(object.getData(PRICING_DATA_LSMCNUM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dataProvider->lsmcnum = dynamic_cast<const LADataInt&>(dh->get()).get();
		if (dataProvider->mcnum != 0 && dataProvider->lsmcnum > dataProvider->mcnum)
		{
			//error
			throw LACoreInvalidData("LSMCNum must be same or less than MCNum", __FILE__, __LINE__);	
		}
	}
	else
		dataProvider->lsmcnum = 0;
	

	if (dataProvider->lsmcnum == 0)
	{
		//coefficient
		dh = &objHolder.getData(PRICING_DATA_COEFFICIENTS, ISNOTNULL);
		dataProvider->coefficient = &dynamic_cast<LADataDoubleMatrix&>(dh->get());
		dataProvider->payoff.setLSMCCoefficient(dataProvider->coefficient->get());
		dataProvider->payoff.setLSMCMode(false);
		return dataProvider;
	}

	// today
	dh = &(object.getData(PRICING_DATA_TODAY, ISNOTNULL));
	const LADate& today = dynamic_cast<const LADataDate&>(dh->get()).get();

	
	//expirytimes actiontimes;
	dh = &(objHolder.getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
	const DateVector& expirydates = dynamic_cast<const LADataDates&>(dh->get()).get();
	
	dh = &(objHolder.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
	const DateVector& actiondates = dynamic_cast<const LADataDates&>(dh->get()).get();

	for (unsigned int i = 0; i < expirydates.size(); i++)
	{
		if (expirydates[i] > today)
		{
			dataProvider->expirytimes.push_back(dataProvider->pPath->getDayCount().getTerm(today, expirydates[i]));
			dataProvider->actiontimes.push_back(dataProvider->pPath->getDayCount().getTerm(today, actiondates[i]));
		}
	}

	//polynomial
	dh = &(objHolder.getData(PRICING_DATA_POLYNOMIAL, ISNOTNULL));
	LAFunctionBase& poly = dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();
	if (!poly.isTypeOf(FN_POLYNOMIALBASE))
	{
		//error
		throw LACoreInvalidData("Not Polynomial function", __FILE__, __LINE__);	
	}
	dataProvider->poly = &dynamic_cast<LAPolynomialBase&>(poly);

	//coefficient
	objHolder.remove(PRICING_DATA_COEFFICIENTS);
	dh = &objHolder.add(PRICING_DATA_COEFFICIENTS, new LADataDoubleMatrix());
	dataProvider->coefficient = &dynamic_cast<LADataDoubleMatrix&>(dh->get());
	

	return dataProvider;
	 
}
*/
	
/*!
	@brief create new cache class
	@return cache class
*/
/*LADataProvider*
LAPriceLSMCTradeValue::createNewDataProvider() const
{
	LAPriceTradeValueDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LAPriceTradeValueDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	dataProvider->payoff.setLSMCMode(true);
    return dataProvider;
}*/

/*!
    @brief Make copy(clone) of this object.
    @return pointer of this object.
*/
LACoreFunctionBase*
LAPriceLSMCTradeValue::LAPriceLSMCOperator::clone() const
{
    try 
	{
        return new LAPriceLSMCOperator(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }

}


/*!
    @brief return function value
    @return function value
*/
double
LAPriceLSMCTradeValue::LAPriceLSMCOperator::operator()(const DoubleArray& x) const
{
	DoubleArray xx = x;
	xx.resize(x.size() - 1);
	DoubleArray basis_func;
	mPoly->setNumVar(xx.size());
	mPoly->calcBasisFunc(xx, basis_func);
    return inner_product(basis_func.begin(), basis_func.end(), mParam.begin(), 0.0) + mParam.back() * x.back();
	
}

/*!
	@brief set standardization param

	@param[in,out] dataProvider
	@param[in] isconvert_xy
	@param[in] standardization_y
	@param[in] standardization_x
	@param[in] isconvert_xy_rebate
	@param[in] standardization_y_rebate
	@param[in] standardization_x_rebate

*/
void
LAPriceLSMCTradeValue::setStandardizationParam(LAPriceTradeValueDataProvider &dataProvider, const BoolVector &isconvert_xy, const vector<DoubleArray>& standardization_y, const vector<DoubleMatrix>& standardization_x,
											const BoolVector &isconvert_xy_rebate, const vector<DoubleArray>& standardization_y_rebate, const vector<DoubleMatrix>& standardization_x_rebate) const
{
	dataProvider.isconvert_xy->set(isconvert_xy);
	dataProvider.shift_y->set(standardization_y[0]);
	dataProvider.scale_y->set(standardization_y[1]);
	dataProvider.shift_x->set(standardization_x[0]);
	dataProvider.scale_x->set(standardization_x[1]);
	// rebate
	dataProvider.isconvert_xy_rebate->set(isconvert_xy_rebate);
	dataProvider.shift_y_rebate->set(standardization_y_rebate[0]);
	dataProvider.scale_y_rebate->set(standardization_y_rebate[1]);
	dataProvider.shift_x_rebate->set(standardization_x_rebate[0]);
	dataProvider.scale_x_rebate->set(standardization_x_rebate[1]);
}


