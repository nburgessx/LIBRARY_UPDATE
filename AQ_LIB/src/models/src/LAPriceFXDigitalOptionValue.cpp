//  2009, AlgoQuantHub.
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
#include "LAMathDateCalculations.h"
#include "AQLCoreComponentManager.h"
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
AQLString 
LAPriceFXDigitalOptionValue::getOptionPayoffName() const
{
	return FN_FXDIGITALOPTIONVALUE_STR;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDigitalOptionValue::registerData(AQLPriceDataManager& dm) const
{

	LALinearRatesOptionValue::registerData(dm);

	LAPriceFXOptionValue::registerData(dm);
	
	dm.setData(PRICING_DATA_DIGITALPAYOFF, DATA_DOUBLE);
}

AQLCoreFunctionBase*
LAPriceFXDigitalOptionValue::clone() const
{
    try 
	{
		return new LAPriceFXDigitalOptionValue(*this);
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
LAPriceFXDigitalOptionValue::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	
	AQLDataHolder* dh;
	LAPriceFXOptionValueDataProvider* dataProvider = NULL;
	dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider *>(LAPriceFXOptionValue::setUpDataProvider(basedate,object,att));

	AnalyticDGParam* pm = dynamic_cast<AnalyticDGParam *>(dataProvider->mParam[0][0]);

	dh = &(object.getData(PRICING_DATA_DIGITALPAYOFF, ISNOTNULL));
	pm->Dig = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	return dataProvider;
}

// calc payoff after maturity
double				
LAPriceFXDigitalOptionValue::calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const
{
	
	//dataProvider;
	LAPriceFXOptionValueDataProvider* dp =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dataProvider);
	//LALinearRatesOptionValueDataProvider* dataProvider = &dynamic_cast<LALinearRatesOptionValueDataProvider &>(att.getDataProvider());
	
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
LAPriceFXDigitalOptionValue::createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const
{
	std::vector<AnalyticParam*> retvec(1);
	std::vector< std::vector<AnalyticParam*> > ret(1,retvec);
	ret[0][0] = new AnalyticDGParam();
	return ret;
}



