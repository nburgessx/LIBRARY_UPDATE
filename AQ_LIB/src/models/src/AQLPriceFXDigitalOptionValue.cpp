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
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLMathDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceFXDigitalOptionValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLLinearRatesOptionValueDataProvider.h"

using namespace std;

AQLPriceFXDigitalOptionValue::AQLPriceFXDigitalOptionValue()
: AQLPriceFXOptionValue()
{}

//AQLPriceFXDigitalOptionValue::AQLPriceFXDigitalOptionValue(AQLPriceFXDigitalOptionValue& v)
//: AQLLinearRatesOptionValue(v)
//{}

AQLPriceFXDigitalOptionValue::~AQLPriceFXDigitalOptionValue()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLPriceFXDigitalOptionValue::getType() const
{
	return FN_FXDIGITALOPTIONVALUE;
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceFXDigitalOptionValue::isTypeOf(function_t id) const
{
	return (id == FN_FXDIGITALOPTIONVALUE ? true : AQLPriceFXOptionValue::isTypeOf(id));
}

//hishida vannavolga
/*
    @brief get option method name
     @return option method name
*/
AQLString 
AQLPriceFXDigitalOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceFXDigitalOptionValue::registerData(AQLPriceDataManager& dm) const
{

	AQLLinearRatesOptionValue::registerData(dm);

	AQLPriceFXOptionValue::registerData(dm);
	
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
}

AQLCoreFunctionBase*
AQLPriceFXDigitalOptionValue::clone() const
{
    try 
	{
		return new AQLPriceFXDigitalOptionValue(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}





/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLPriceFXDigitalOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	AQLPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<AQLPriceFXOptionValueDataProvider *>(AQLPriceFXOptionValue::setUpDataProvider(basedate,object,att));

	AnalyticDGParam* pm = dynamic_cast<AnalyticDGParam *>(dataProvider->mParam[0][0]);

	dh = &(object.getData(PRICING_DATA_DIGITALPAYOFF, ISNOTNULL));
	pm->Dig = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	return dataProvider;
}

// calc payoff after maturity
double				
AQLPriceFXDigitalOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	
	//dataProvider;
	AQLPriceFXOptionValueDataProvider* dp =  dynamic_cast<AQLPriceFXOptionValueDataProvider*>(dataProvider);
	//AQLLinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<AQLLinearRatesOptionValueDataProvider &>(att.getDataProvider());
	
	AQLDataHolder* dh = &(e.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	
	double ret = 0.0;
	AnalyticDGParam* dgparam = dynamic_cast<AnalyticDGParam* >(dp->mParam[0][0]);
	if (AQLString("CALL") == optiontype && dgparam->S > dgparam->K)
	{
		ret = dgparam->Dig;
	}
	else if(AQLString("PUT") == optiontype && dgparam->K > dgparam->S)
	{
		ret = dgparam->Dig;
	}
	else
		ret = 0.0;
	return ret;
}


std::vector< std::vector<AnalyticParam*> >
AQLPriceFXDigitalOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const
{
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticDGParam();
	return ret;
}



