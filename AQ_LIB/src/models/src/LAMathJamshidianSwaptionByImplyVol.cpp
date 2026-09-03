//  2008, Mizuho International London.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAObject.h"
#include "LADataProcedure.h"
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
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAModelDynamicsHW1FCurve.h"

#include "LAMathIRVanillaFuncUtility.h"
#include "LACoreComponentManager.h"

const double VOLINFBYBLACK = 0.0001;
const double VOLSUPBYBLACK = 2.5;
const double POSITIVERATIORANGE = 2.5;
const double NEGATIVERATIORANGE = -0.5;

using namespace std;

LAMathJamshidianSwaptionByImplyVol::LAMathJamshidianSwaptionByImplyVol()
: LAMathJamshidianSwaption()
{}

LAMathJamshidianSwaptionByImplyVol::~LAMathJamshidianSwaptionByImplyVol()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAMathJamshidianSwaptionByImplyVol::getType() const
{
	return FN_JAMSHIDIANSWAPTIONBYIMPLYVOL;
}




LACoreFunctionBase*
LAMathJamshidianSwaptionByImplyVol::clone() const
{
    try 
	{
		return new LAMathJamshidianSwaptionByImplyVol(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to LAMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return swaption prem
	
*/
double
LAMathJamshidianSwaptionByImplyVol::value(const LADate& basedate, LAObject& object,
					const LADataValuation& att) const
{
	LAMathJamshidianSwaptionByImplyVolDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		dataProvider = dynamic_cast<LAMathJamshidianSwaptionByImplyVolDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dataProvider = &dynamic_cast<LAMathJamshidianSwaptionByImplyVolDataProvider&>(att.getDataProvider());

	double ret = LAMathJamshidianSwaption::value(basedate,object,att);
	//Volatility fitting caluculation is only when mPrem and mSimprem is at some range
	//dataProvider->mPrem == 0.0  && dataProvider->mVol ==0.0 exists for case of getting imply vol by hwmodel not for case of calibration
	double ratio = dataProvider->mSimPrem / LAMath::max( dataProvider->mPrem,0.00001) -1.0 ;
		
	if( NEGATIVERATIORANGE <= ratio && ratio <= POSITIVERATIORANGE  || dataProvider->mPrem == 0.0 && dataProvider->mVol ==0.0)
	{
		AnalyticBKParam& bk = dataProvider->mBKParam;
		AnalyticAFFParam& pm = dataProvider->mAFFParam;

		bk.K = dataProvider->mStrike;
		bk.Te = dataProvider->mGrids[0]-dataProvider->mAdjTerm;
		bk.Nu = dataProvider->mAnnuity/dataProvider->mAdjDF;
		bk.F = (pm.Poptm-pm.Pbondm)/LAMath::max(0.000001,dataProvider->mAnnuity);
		double val = LAMathIRVanillaFuncUtility::optimize(VOLINFBYBLACK, VOLSUPBYBLACK, dataProvider->mSimPrem, bk, dataProvider->mAnalyticMethod2, dataProvider->mAnalyticMethod3);

		ret = 10000 * LAMath::abs(val - dataProvider->mVol) * dataProvider->mWeight;
	}
	return ret;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAMathJamshidianSwaptionByImplyVol::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	LADataHolder* dh;
	LAMathJamshidianSwaptionByImplyVolDataProvider* dataProvider = dynamic_cast<LAMathJamshidianSwaptionByImplyVolDataProvider*>(LAMathJamshidianSwaption::setUpDataProvider(basedate,object,att));
	
	//mAnalyticMethod
	dh = &(object.getData(PRICING_DATA_OPTIONTYPE,ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();
	if("PAYERS" == optiontype)
		optiontype= "PUT";
	else if("RECEIVERS" == optiontype)
		optiontype = "CALL";
	else
		throw LACoreInvalidData("Error or OptonType",__FILE__,__LINE__);
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	
	LAString bscomponent = LAString(BK) + LAString(PREM) + optiontype;
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	dataProvider->mAnalyticMethod2 = it->second;
	
	bscomponent = LAString(BK) + LAString(VEGA) + optiontype;
	it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	dataProvider->mAnalyticMethod3 = it->second;

	return dataProvider;
}


/*!
	@brief create new cache class
	@return cache class
*/
LADataProvider*
LAMathJamshidianSwaptionByImplyVol::createNewDataProvider() const
{
	LAMathJamshidianSwaptionByImplyVolDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new LAMathJamshidianSwaptionByImplyVolDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

LAMathJamshidianSwaptionByImplyVol::LAMathJamshidianSwaptionByImplyVolDataProvider::LAMathJamshidianSwaptionByImplyVolDataProvider()
: LAMathJamshidianSwaptionDataProvider()
,mBKParam(),mAnalyticMethod2(0),mAnalyticMethod3(0)
{	
}

LAMathJamshidianSwaptionByImplyVol::LAMathJamshidianSwaptionByImplyVolDataProvider::~LAMathJamshidianSwaptionByImplyVolDataProvider()
{
}
