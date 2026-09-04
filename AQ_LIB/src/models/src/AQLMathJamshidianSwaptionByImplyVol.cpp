#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathJamshidianSwaptionByImplyVol.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
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
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLMathDateCalculations.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLModelDynamicsHW1FCurve.h"

#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLCoreComponentManager.h"

const double VOLINFBYBLACK = 0.0001;
const double VOLSUPBYBLACK = 2.5;
const double POSITIVERATIORANGE = 2.5;
const double NEGATIVERATIORANGE = -0.5;

using namespace std;

AQLMathJamshidianSwaptionByImplyVol::AQLMathJamshidianSwaptionByImplyVol()
: AQLMathJamshidianSwaption()
{}

AQLMathJamshidianSwaptionByImplyVol::~AQLMathJamshidianSwaptionByImplyVol()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLMathJamshidianSwaptionByImplyVol::getType() const
{
	return FN_JAMSHIDIANSWAPTIONBYIMPLYVOL;
}




AQLCoreFunctionBase*
AQLMathJamshidianSwaptionByImplyVol::clone() const
{
    try 
	{
		return new AQLMathJamshidianSwaptionByImplyVol(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return swaption prem
	
*/
double
AQLMathJamshidianSwaptionByImplyVol::value(const AQLDate& basedate, AQLObject& object,
					const AQLDataValuation& att) const
{
	AQLMathJamshidianSwaptionByImplyVolDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		dataProvider = dynamic_cast<AQLMathJamshidianSwaptionByImplyVolDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dataProvider = &dynamic_cast<AQLMathJamshidianSwaptionByImplyVolDataProvider&>(att.getDataProvider());

	double ret = AQLMathJamshidianSwaption::value(basedate,object,att);
	//Volatility fitting caluculation is only when mPrem and mSimprem is at some range
	//dataProvider->mPrem == 0.0  && dataProvider->mVol ==0.0 exists for case of getting imply vol by hwmodel not for case of calibration
	double ratio = dataProvider->mSimPrem / AQLMath::max( dataProvider->mPrem,0.00001) -1.0 ;
		
	if( NEGATIVERATIORANGE <= ratio && ratio <= POSITIVERATIORANGE  || dataProvider->mPrem == 0.0 && dataProvider->mVol ==0.0)
	{
		AnalyticBKParam& bk = dataProvider->mBKParam;
		AnalyticAFFParam& pm = dataProvider->mAFFParam;

		bk.K = dataProvider->mStrike;
		bk.Te = dataProvider->mGrids[0]-dataProvider->mAdjTerm;
		bk.Nu = dataProvider->mAnnuity/dataProvider->mAdjDF;
		bk.F = (pm.Poptm-pm.Pbondm)/AQLMath::max(0.000001,dataProvider->mAnnuity);
		double val = AQLMathIRVanillaFuncUtility::optimize(VOLINFBYBLACK, VOLSUPBYBLACK, dataProvider->mSimPrem, bk, dataProvider->mAnalyticMethod2, dataProvider->mAnalyticMethod3);

		ret = 10000 * AQLMath::abs(val - dataProvider->mVol) * dataProvider->mWeight;
	}
	return ret;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLMathJamshidianSwaptionByImplyVol::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	AQLMathJamshidianSwaptionByImplyVolDataProvider* dataProvider = dynamic_cast<AQLMathJamshidianSwaptionByImplyVolDataProvider*>(AQLMathJamshidianSwaption::setUpDataProvider(basedate,object,att));
	
	//mAnalyticMethod
	dh = &(object.getData(PRICING_DATA_OPTIONTYPE,ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	if("PAYERS" == optiontype)
		optiontype= "PUT";
	else if("RECEIVERS" == optiontype)
		optiontype = "CALL";
	else
		throw AQLCoreInvalidData("Error or OptonType",__FILE__,__LINE__);
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	
	AQLString bscomponent = AQLString(BK) + AQLString(PREM) + optiontype;
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	dataProvider->mAnalyticMethod2 = it->second;
	
	bscomponent = AQLString(BK) + AQLString(VEGA) + optiontype;
	it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	dataProvider->mAnalyticMethod3 = it->second;

	return dataProvider;
}


/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLMathJamshidianSwaptionByImplyVol::createNewDataProvider() const
{
	AQLMathJamshidianSwaptionByImplyVolDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLMathJamshidianSwaptionByImplyVolDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

AQLMathJamshidianSwaptionByImplyVol::AQLMathJamshidianSwaptionByImplyVolDataProvider::AQLMathJamshidianSwaptionByImplyVolDataProvider()
: AQLMathJamshidianSwaptionDataProvider()
,mBKParam(),mAnalyticMethod2(0),mAnalyticMethod3(0)
{	
}

AQLMathJamshidianSwaptionByImplyVol::AQLMathJamshidianSwaptionByImplyVolDataProvider::~AQLMathJamshidianSwaptionByImplyVolDataProvider()
{
}
