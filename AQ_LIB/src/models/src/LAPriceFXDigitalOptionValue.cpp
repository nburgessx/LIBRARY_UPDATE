//  2009, AlgoQuantHub.
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
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAMathDateCalculations.h"
#include "LACoreComponentManager.h"
#include "LAPriceFXDigitalOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LALinearRatesOptionValueDataProvider.h"

using namespace std;

LAPriceFXDigitalOptionValue::LAPriceFXDigitalOptionValue()
: LAPriceFXOptionValue()
{}

//LAPriceFXDigitalOptionValue::LAPriceFXDigitalOptionValue(LAPriceFXDigitalOptionValue& v)
//: LALinearRatesOptionValue(v)
//{}

LAPriceFXDigitalOptionValue::~LAPriceFXDigitalOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAPriceFXDigitalOptionValue::getType() const
{
	return FN_FXDIGITALOPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXDigitalOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXDIGITALOPTIONVALUE ? true : LAPriceFXOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
LAString 
LAPriceFXDigitalOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDigitalOptionValue::registerData(LAPriceDataManager& dm) const
{

	LALinearRatesOptionValue::registerData(dm);

	LAPriceFXOptionValue::registerData(dm);
	
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
}

LACoreFunctionBase*
LAPriceFXDigitalOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXDigitalOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att LADataValuation class that this valuation class is setted

	@return cashe class
	
*/
LADataProvider*					
LAPriceFXDigitalOptionValue::setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const
{
	
	LADataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXOptionValue::setUpDataProvider(basedate,object,att));

	AnalyticDGParam* pm = dynamic_cast<AnalyticDGParam *>(dataProvider->mParam[0][0]);

	dh = &(object.getData(PRICING_DATA_DIGITALPAYOFF, ISNOTNULL));
	pm->Dig = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	return dataProvider;
}

// calc payoff after maturity
double				
LAPriceFXDigitalOptionValue::calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const
{
	
	//dataProvider;
	LAPriceFXOptionValueDataProvider* dp =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dataProvider);
	//LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());
	
	LADataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	LAString optiontype = dynamic_cast<LADataString &>(dh->get()).get();
	optiontype.toUpper();
	
	double ret = 0.0;
	AnalyticDGParam* dgparam = dynamic_cast<AnalyticDGParam* >(dp->mParam[0][0]);
	if (LAString("CALL") == optiontype && dgparam->S > dgparam->K)
	{
		ret = dgparam->Dig;
	}
	else if(LAString("PUT") == optiontype && dgparam->K > dgparam->S)
	{
		ret = dgparam->Dig;
	}
	else
		ret = 0.0;
	return ret;
}


std::vector< std::vector<AnalyticParam*> >
LAPriceFXDigitalOptionValue::createAnalyticParam(LAObject& object, LADataProvider* dataProvider) const
{
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticDGParam();
	return ret;
}



