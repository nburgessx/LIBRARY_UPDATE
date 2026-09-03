// LAMathYieldCurvePro.h

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

const double INFINITESIMAL = 1E-7;

#include "AQLMathDefine.h"
#include "AQLBasic.h"
#include "AQLNl2sol.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataManager.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLDataMultiReference.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataFunction.h"
#include "AQLDataInstance.h"
#include "AQLCoreUtil.h"
#include "AQLOptimumBrent.h"
#include "LAPriceYieldGenerator.h"
#include "LAMathDateCalculations.h"
#include "LAMathDateUtilities.h"
#include "AQLLinearInterpolation.h"
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "AQLAlgorithm.h"
#include "AQLMatrix.h"
#include "LAMathFXEntity.h"
#include "ConstantDeclarations.h"
#include "AQLObjectHolder.h"
#include "LAMathYieldCurvePro.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"

using namespace std;

//====================================================================
/*!
    @brief constructor

	@param[in] dataInstance pointer of AQLDataInstance

*/
LAMathYieldCurvePro::LAMathYieldCurvePro(AQLDataInstance* dataInstance) : 
				LAMathYieldCurve(dataInstance)//,mCurveVersion(0)
{
	setDataInstance(dataInstance);
	AQLPriceDataManager& dm = dataInstance->getDataMaster();

	dm.setData(CALIBRATION_DATA_ASOFDATE,					DATA_DATE			);
	dm.setData(CALIBRATION_DATA_CURVEGENERATOR,				DATA_PROCEDURE		);
	dm.setData(CALIBRATION_DATA_MARKETDATA,					DATA_MULTIREFERENCE	);
	dm.setData(IR_CALIBRATION_DATA_BASISRATES,				DATA_DOUBLE_MATRIX	);
	dm.setData(IR_CALIBRATION_DATA_BASISDATA,				DATA_MULTIREFERENCE	);
	dm.setData(IR_CALIBRATION_DATA_ISFUTUREUSE,				DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_BASEYIELDCURVE,			DATA_REFERENCE		);
	dm.setData(IR_CALIBRATION_DATA_CURRENCY,				DATA_STRING			);
	dm.setData(IR_CALIBRATION_DATA_INTERPOLATIONYG,			DATA_INTERPOLATION	);
	dm.setData(IR_CALIBRATION_DATA_INTERPOLATIONFW,			DATA_INTERPOLATION	);
	dm.setData(IR_CALIBRATION_DATA_INTERPOLATIONBS,			DATA_INTERPOLATION	);
	dm.setData(IR_CALIBRATION_DATA_BASISFUNCTION,			DATA_FUNCTION		);
	dm.setData(PRICING_DATA_RATEPRIORITY,					DATA_STRINGS		);
	dm.setData(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX,			DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_ISDISCOUNTCURVE,			DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_GENERATEDFS,				DATA_STRINGS		);
	dm.setData(IR_CALIBRATION_DATA_ISARBFREE,				DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_ISFRAUSE,				DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST,		DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_FOREIGNYIELDDATA,		DATA_REFERENCE		);
	dm.setData(IR_CALIBRATION_DATA_FXENTITY,				DATA_REFERENCE		);
	dm.setData(IR_CALIBRATION_DATA_COLYIELDDATA,			DATA_REFERENCE		);
	dm.setData(IR_CALIBRATION_DATA_ISFWDFX,					DATA_BOOL			);
	dm.setData(IR_CALIBRATION_DATA_OPTIMIZEMETHOD,			DATA_STRING			);
	dm.setData(IR_CALIBRATION_DATA_COMPOUNDING_FUNCTION,	DATA_FUNCTION		);

	mpAsOfDate			= &add(CALIBRATION_DATA_ASOFDATE);
	mpProcedure			= &add(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData		= &add(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates		= &add(IR_CALIBRATION_DATA_BASISRATES);		
	mpBasisData			= &add(IR_CALIBRATION_DATA_BASISDATA);		
	mpIsFutureUse		= &add(IR_CALIBRATION_DATA_ISFUTUREUSE);		
	mpBaseYieldCurve	= &add(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency			= &add(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG			= &add(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW			= &add(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS			= &add(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction		= &add(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority		= &add(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree			= &add(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse			= &add(IR_CALIBRATION_DATA_ISFRAUSE);	
	mpIsSwapTenorAdjust = &add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);	
	mpForeignYieldData	= &add(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);	
	mpFXEntity			= &add(IR_CALIBRATION_DATA_FXENTITY);
	mpColYieldData		= &add(IR_CALIBRATION_DATA_COLYIELDDATA);	
	mpOptimizeMethod	= &add(IR_CALIBRATION_DATA_OPTIMIZEMETHOD);
	mpCompoundFunction  = &add(IR_CALIBRATION_DATA_COMPOUNDING_FUNCTION);
}
/*!
    @brief copy constructor

	@param[in] curve original object
*/
LAMathYieldCurvePro::LAMathYieldCurvePro(
	const LAMathYieldCurvePro& curve) : 
	LAMathYieldCurve(curve)//,mCurveVersion(0)
{
	mpAsOfDate			= &getData(CALIBRATION_DATA_ASOFDATE);
	mpProcedure			= &getData(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData		= &getData(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates		= &getData(IR_CALIBRATION_DATA_BASISRATES);		
	mpBasisData			= &getData(IR_CALIBRATION_DATA_BASISDATA);		
	mpIsFutureUse		= &getData(IR_CALIBRATION_DATA_ISFUTUREUSE);		
	mpBaseYieldCurve	= &getData(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency			= &getData(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG			= &getData(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW			= &getData(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS			= &getData(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction		= &getData(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority		= &getData(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree			= &getData(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse			= &getData(IR_CALIBRATION_DATA_ISFRAUSE);
	mpIsSwapTenorAdjust	= &getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);
	mpForeignYieldData	= &getData(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);
	mpFXEntity			= &getData(IR_CALIBRATION_DATA_FXENTITY);
	mpColYieldData		= &getData(IR_CALIBRATION_DATA_COLYIELDDATA);
	mpOptimizeMethod	= &getData(IR_CALIBRATION_DATA_OPTIMIZEMETHOD);
	mpCompoundFunction  = &getData(IR_CALIBRATION_DATA_COMPOUNDING_FUNCTION);
}
/*!
    @brief destructor	
*/
LAMathYieldCurvePro::~LAMathYieldCurvePro()
{
}

// QUERY
/*!
    @brief get EntityType
	
	@return EntityType
*/
object_t	
LAMathYieldCurvePro::getType(void) const
{
	return ENTITY_IRYCPRO;
}
/*!
    @brief this object have specified object or not
	@param[in] id Object type
	@return true  :this object have specified id object
			false :this object don't have specified id object
*/
bool
LAMathYieldCurvePro::isTypeOf(object_t id) const
{
	return (id == ENTITY_IRYCPRO ? true : LAMathYieldCurve::isTypeOf(id));
}

/*!
    @brief get AsOfDate
	@return AsOfDate
*/
const AQLDataDate&  
LAMathYieldCurvePro::getAsOfDate(void) const
{
	return dynamic_cast<const AQLDataDate&>(mpAsOfDate->get());
}
/*!
    @brief get AsOfDate
	@return AsOfDate
*/
AQLDataDate&  
LAMathYieldCurvePro::getAsOfDate(void)
{
	return dynamic_cast<AQLDataDate&>(mpAsOfDate->get());
}

/*!
    @brief get IsArbFree
	@return IsArbFree
*/
const AQLDataBool&  
LAMathYieldCurvePro::getIsArbFree(void) const
{
	return dynamic_cast<const AQLDataBool&>(mpIsArbFree->get());
}
/*!
    @brief get IsArbFree
	@return IsArbFree
*/
AQLDataBool&  
LAMathYieldCurvePro::getIsArbFree(void)
{
	return dynamic_cast<AQLDataBool&>(mpIsArbFree->get());
}

/*!
    @brief get MarketData
			
	@return MarketData
*/
const AQLDataMultiReference&
LAMathYieldCurvePro::getMarketData() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpMarketData->get());
}
/*!
    @brief get MarketData
			
	@return MarketData
*/
AQLDataMultiReference&
LAMathYieldCurvePro::getMarketData()
{
	return dynamic_cast<AQLDataMultiReference&>(mpMarketData->get());
}
/*!
    @brief get BasisRates
			
	@return BasisRates
*/

const AQLDataDoubleMatrix&
LAMathYieldCurvePro::getBasisRates() const	
{
	return dynamic_cast<const AQLDataDoubleMatrix&>(mpBasisRates->get());
}

/*!
    @brief get BasisRates and set BasisRates
			
	@return BasisRates
*/
AQLDataDoubleMatrix&
LAMathYieldCurvePro::getBasisRates() 
{
	return dynamic_cast<AQLDataDoubleMatrix&>(mpBasisRates->get());
}

/*!
    @brief get BasisData
			
	@return BasisData
*/

const AQLDataMultiReference&
LAMathYieldCurvePro::getBasisData() const	
{
	return dynamic_cast<const AQLDataMultiReference&>(mpBasisData->get());
}

/*!
    @brief get BasisData and set BasisData
			
	@return BasisData
*/
AQLDataMultiReference&
LAMathYieldCurvePro::getBasisData() 
{
	return dynamic_cast<AQLDataMultiReference&>(mpBasisData->get());
}


/*!
    @brief get Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/

const AQLDataBool&
LAMathYieldCurvePro::getIsFutureUse() const	
{
	return dynamic_cast<const AQLDataBool&>(mpIsFutureUse->get());
}

/*!
    @brief get Use FutureRate or not and set Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/
AQLDataBool&
LAMathYieldCurvePro::getIsFutureUse() 
{
	return dynamic_cast<AQLDataBool&>(mpIsFutureUse->get());
}

/*!
    @brief get Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/

const AQLDataBool&
LAMathYieldCurvePro::getIsFRAUse() const	
{
	return dynamic_cast<const AQLDataBool&>(mpIsFRAUse->get());
}

/*!
    @brief get Use FutureRate or not and set Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/
AQLDataBool&
LAMathYieldCurvePro::getIsFRAUse() 
{
	return dynamic_cast<AQLDataBool&>(mpIsFRAUse->get());
}

/*!
    @brief get BaseYieldCurve
			
	@return BaseYieldCurve
*/

const AQLDataReference&
LAMathYieldCurvePro::getBaseYieldCurve() const	
{
	return dynamic_cast<const AQLDataReference&>(mpBaseYieldCurve->get());
}

/*!
    @brief get BaseYieldCurve and set BaseYieldCurve
			
	@return BaseYieldCurve
*/
AQLDataReference&
LAMathYieldCurvePro::getBaseYieldCurve() 
{
	return dynamic_cast<AQLDataReference&>(mpBaseYieldCurve->get());
}

/*!
    @brief get Interpolation to generate yield curve
			
	@return Interpolation to generate yield curve
*/
const AQLPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_yg() const	
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInterYG->get());
}

/*!
    @brief get Interpolation to generate yield curve
			
	@return Interpolation to generate yield curve
*/
AQLPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_yg() 
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInterYG->get());
}

/*!
    @brief get Interpolation to use future
			
	@return Interpolation to use future
*/
const AQLPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_fw() const	
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInterFW->get());
}

/*!
    @brief get Interpolation to use future
			
	@return Interpolation to use future
*/
AQLPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_fw() 
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInterFW->get());
}

/*!
    @brief get Interpolation to set basis rates
			
	@return Interpolation to set basis rates
*/
const AQLPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_bs() const	
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInterBS->get());
}

/*!
    @brief get Interpolation to set basis rates
			
	@return Interpolation to set basis rates
*/
AQLPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_bs() 
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInterBS->get());
}

/*!
    @brief get Basis Function
			
	@return Basis Function
*/
const AQLPriceDataFunction&
LAMathYieldCurvePro::getBasisFunction() const	
{
	return dynamic_cast<const AQLPriceDataFunction&>(mpBasisFunction->get());
}

/*!
    @brief get Basis Function
			
	@return Basis Function
*/
AQLPriceDataFunction&
LAMathYieldCurvePro::getBasisFunction() 
{
	return dynamic_cast<AQLPriceDataFunction&>(mpBasisFunction->get());
}

AQLDataStrings&
LAMathYieldCurvePro::getRatePriority() 
{
	return dynamic_cast<AQLDataStrings&>(mpRatePriority->get());
}

const AQLDataStrings&
LAMathYieldCurvePro::getRatePriority() const 
{
	return dynamic_cast<AQLDataStrings&>(mpRatePriority->get());
}

/*!
    @brief get FXEntity
			
	@return FXEntity
*/
const AQLDataReference&
LAMathYieldCurvePro::getFXEntity() const	
{
	return dynamic_cast<const AQLDataReference&>(mpFXEntity->get());
}

/*!
    @brief get FXEntity and set FXEntity
			
	@return FXEntity
*/
AQLDataReference&
LAMathYieldCurvePro::getFXEntity() 
{
	return dynamic_cast<AQLDataReference&>(mpFXEntity->get());
}

/*!
    @brief get OptimizeMethod
			
	@return OptimizeMethod
*/
const AQLDataString&
LAMathYieldCurvePro::getOptimizeMethod() const	
{
	return dynamic_cast<const AQLDataString&>(mpOptimizeMethod->get());
}

/*!
    @brief get OptimizeMethod and set OptimizeMethod
			
	@return OptimizeMethod
*/
AQLDataString&
LAMathYieldCurvePro::getOptimizeMethod() 
{
	return dynamic_cast<AQLDataString&>(mpOptimizeMethod->get());
}

/*!
	@brief get CompoundingFunction

	@return 
*/
const AQLPriceDataFunction&
LAMathYieldCurvePro::getCompoundingFunction() const
{
	return dynamic_cast<const AQLPriceDataFunction&>(mpCompoundFunction->get());
}

/*!
	@brief get CompoundingFunction and set CompoundingFunction

	@return CompoundingFunction
*/
AQLPriceDataFunction&
LAMathYieldCurvePro::getCompoundingFunction()
{
	return dynamic_cast<AQLPriceDataFunction&>(mpCompoundFunction->get());
}


// OPERATION
/*!
    @brief set Interpolation

	@param[in] a	pointer of Interpolation function
	@param[in] name name of Interpolation 
*/
void
LAMathYieldCurvePro::setInterpolation(
	AQLInterpolationBase* a, const AQLString& name)
{
	dynamic_cast<AQLPriceDataInterpolation*>(&(
		getData(CALIBRATION_DATA_INTERPOLATION).get()))->setMethod(a, name);
}
/*!
    @brief set Interpolation

	@param[in] name name of Interpolation 
*/
void
LAMathYieldCurvePro::setInterpolation(const AQLString& name)
{
	dynamic_cast<AQLPriceDataInterpolation*>(&(
		getData(CALIBRATION_DATA_INTERPOLATION).get()))->setMethod(name);
}
 
/*!
    @brief set Generator class

	@param[in] a	pointer of generate class
	@param[in] name name of generate class
*/
void
LAMathYieldCurvePro::setDFGenerator(const AQLCoreProcedure* a,
										const AQLString& name)
{
	dynamic_cast<AQLDataProcedure*>(&(
		getData(CALIBRATION_DATA_CURVEGENERATOR).get()))->setMethod(a, name);
}

/*!
    @brief set Generator class

	@param[in] name name of generate class
*/
void
LAMathYieldCurvePro::setDFGenerator(const AQLString& name)
{
	dynamic_cast<AQLDataProcedure*>(&(
		getData(CALIBRATION_DATA_CURVEGENERATOR).get()))->setMethod(name);
}

/*!
    @brief clone this class

	@return	pointer of this class
*/
AQLObject* 
LAMathYieldCurvePro::clone() const
{
    try {
    	LAMathYieldCurvePro*	pCurve = new LAMathYieldCurvePro(*this);
    	return pCurve;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
void                
LAMathYieldCurvePro::remove(
	const AQLString& dataName)
{
	if (dataName == CALIBRATION_DATA_ASOFDATE
		|| dataName == CALIBRATION_DATA_CURVEGENERATOR
		|| dataName == CALIBRATION_DATA_MARKETDATA
		|| dataName == IR_CALIBRATION_DATA_BASISRATES		
		|| dataName == IR_CALIBRATION_DATA_BASISDATA		
		|| dataName == IR_CALIBRATION_DATA_ISFUTUREUSE		
		|| dataName == IR_CALIBRATION_DATA_BASEYIELDCURVE
		|| dataName == IR_CALIBRATION_DATA_CURRENCY
		|| dataName == IR_CALIBRATION_DATA_INTERPOLATIONYG
		|| dataName == IR_CALIBRATION_DATA_INTERPOLATIONFW
		|| dataName == IR_CALIBRATION_DATA_INTERPOLATIONBS
		|| dataName == IR_CALIBRATION_DATA_BASISFUNCTION
		|| dataName == PRICING_DATA_RATEPRIORITY
		|| dataName == IR_CALIBRATION_DATA_ISARBFREE
		|| dataName == IR_CALIBRATION_DATA_ISFRAUSE
		|| dataName == IR_CALIBRATION_DATA_ISSWAPTENORADJUST
		|| dataName == IR_CALIBRATION_DATA_FOREIGNYIELDDATA
		|| dataName == IR_CALIBRATION_DATA_COLYIELDDATA)
	{
		return; 
	}
	LAMathYieldCurve::remove(dataName);
}

void               
LAMathYieldCurvePro::reset(void)
{
	LAMathYieldCurve::reset();
	mpAsOfDate			= &add(CALIBRATION_DATA_ASOFDATE);
	mpProcedure			= &add(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData		= &add(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates		= &add(IR_CALIBRATION_DATA_BASISRATES);		
	mpBasisData			= &add(IR_CALIBRATION_DATA_BASISDATA);		
	mpIsFutureUse		= &add(IR_CALIBRATION_DATA_ISFUTUREUSE);		
	mpBaseYieldCurve	= &add(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency			= &add(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG			= &add(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW			= &add(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS			= &add(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction		= &add(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority		= &add(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree			= &add(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse			= &add(IR_CALIBRATION_DATA_ISFRAUSE);	
	mpIsSwapTenorAdjust = &add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);	
	mpForeignYieldData	= &add(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);	
	mpColYieldData		= &add(IR_CALIBRATION_DATA_COLYIELDDATA);	
}

/////////////// PROTECTED METHODS /////////////////////
/*!
    @brief copy object

	@param[in] e copy object
	@return Reference of this object
*/
AQLObject&
LAMathYieldCurvePro::copy(
	const AQLObject& e)
{
	if (this == &e) return *this;

	LAMathYieldCurve::copy(e);
	if (!e.isTypeOf(ENTITY_IRYCPRO))
	{
		AQLString err = "Assignement error for LAMathYieldCurvePro : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mpAsOfDate			= &getData(CALIBRATION_DATA_ASOFDATE);
	mpProcedure			= &getData(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData		= &getData(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates		= &getData(IR_CALIBRATION_DATA_BASISRATES);		
	mpBasisData			= &getData(IR_CALIBRATION_DATA_BASISDATA);		
	mpIsFutureUse		= &getData(IR_CALIBRATION_DATA_ISFUTUREUSE);		
	mpBaseYieldCurve	= &getData(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency			= &getData(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG			= &getData(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW			= &getData(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS			= &getData(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction		= &getData(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority		= &getData(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree			= &getData(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse			= &getData(IR_CALIBRATION_DATA_ISFRAUSE);	
	mpIsSwapTenorAdjust = &getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);	
	mpForeignYieldData	= &getData(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);	
	mpColYieldData		= &getData(IR_CALIBRATION_DATA_COLYIELDDATA);	

	return *this;
}

/*!
    @brief Set data by name

	@param[in] name name of Data

	@return Reference of AQLDataHolder include this Data

*/
AQLDataHolder&
LAMathYieldCurvePro::add(const AQLString& name)
{
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::add(name, dh);
}

/*!
    @brief Set data by name ( Remove then Add )

	@param[in] name name of Data

	@return Reference of AQLDataHolder include this Data

*/
AQLDataHolder&
LAMathYieldCurvePro::reset(const AQLString& name)
{
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::reset(name, dh);
}

/*!
    @brief culc DF from marketData

	@param[in] asof As of Date
*/

void
LAMathYieldCurvePro::calcDiscountFactor(const AQLDate& asof)
{
	try
	{
		if(getAsOfDate().get() != asof) getAsOfDate() = asof;
		dynamic_cast<AQLDataProcedure&>(mpProcedure->get()).calibrateModel(asof);
	}
	catch(AQLCoreError& e)
	{
		AQLString msg("DiscountFactor Calculation Error at ");
		msg += getName();
		AQLCoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
		err += e;
		throw err;
	}
	catch (...)
	{
		AQLString msg("DiscountFactor Calculation at ");
		msg += getName().get();
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}
/*!
    @brief set DF By ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setDFByShiftZero(double width, unsigned int pos, const UintArray& grids)
{
	unsigned int i, j, i_l, i_r;

	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject& YieldData = objHolder.get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get());
	
	AQLDataDoubles rates;
	rates.resize(terms.getSize());
	for (i = 0; i < terms.getSize() ; i++)
		rates.set(getZeroRate(terms[i]), i);
	
	if (pos > grids.size() - 1 || grids[grids.size() - 1] > terms.getSize() - 1)
	{
		// error
		AQLString msg = getName();
		msg += " : Input pos or grids are something wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	
	i = grids[pos];
	rates.set(rates[i] + width, i);

	if (pos == 0)
	{
		for (j = 0; j < i; j++)
			rates.set(rates[j] + width, j);
		
		if(grids.size() == 1)
		{
			for (j = i + 1; j < rates.getSize(); j++)
					rates.set(rates[j] + width, j);
		}
		else
		{
			i_r = grids[pos + 1];
			for (j = i + 1; j < i_r; j++)
				rates.set(rates[j] + width * (terms[i_r] - terms[j]) / (terms[i_r] - terms[i]), j);
		}
	}
	else if(pos == grids.size() - 1)
	{
		for(j = i + 1; j < rates.getSize(); j++)
			rates.set(rates[j] + width, j);
		
		i_l = grids[pos - 1];
		for(j = i_l + 1; j < i; j++)
			rates.set(rates[j] + width * (terms[j] - terms[i_l]) / (terms[i] - terms[i_l]), j);
	}
	else
	{
		i_l = grids[pos - 1];
		i_r = grids[pos + 1];
		for(j = i_l + 1; j < i; j++)
			rates.set(rates[j] + width * (terms[j] - terms[i_l]) / (terms[i] - terms[i_l]), j);
		for(j = i+1;j<i_r;j++)
			rates.set(rates[j] + width * (terms[i_r] - terms[j]) / (terms[i_r] - terms[i]), j);
	}
	
	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	for (i = 0; i < terms.getSize() ; i++)
		rates.set(1 / AQLPriceDataConvention::rateToRet(rates[i], terms[i], conv) ,i);
	
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new AQLDataDoubles(rates));
}

/*!
    @brief set DF By parallel ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setDFByShiftZero(double width, FloorType type, double floor)
{
	unsigned int i;

	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject& YieldData = objHolder.get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get());
	
	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	
	AQLDataDoubles rates;
	rates.resize(terms.getSize());
	
	for (i = 0; i < terms.getSize() ; i++)
	{
		rates.set(getZeroRate(terms[i]), i);
	}
	for(i = 0; i < rates.getSize(); i++)
	{
		double rnew;
		switch (type)
		{
		case NOFLOOR:
			rnew = rates[i] + width;
			break;		
		case NORMAL:
			rnew = AQLMath::max(rates[i] + width, floor); 
			if(width < 0 && rnew > rates[i]) rnew = rates[i];
			break;
		case SYMMETRIC:
            if (width < 0)
				rnew = AQLMath::min(rates[i], AQLMath::max(rates[i] + width, floor));
			else
			{
				rnew = AQLMath::min(rates[i], AQLMath::max(rates[i] - width, floor));
				rnew = 2 * rates[i] - rnew; 
			}
			break;
		default:
			throw AQLCoreInvalidData("Not Supported Floor Type", __FILE__, __LINE__);		
		}
		rates.set(1 / AQLPriceDataConvention::rateToRet(rnew, terms[i], conv) ,i);
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new AQLDataDoubles(rates));
}

/*!
    @brief set BasisDF By ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setBasisDFByShiftZero(double width, unsigned int pos, const UintArray& grids)
{
	AQLObjectHolder &eh_y = getYieldData().get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> (eh_y.getData(CALIBRATION_DATA_TERMS + mCurveSuffix , ISNOTNULL).get());
	
	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);

	const unsigned int termSize = terms.getSize();
	AQLDataDoubles rates;
	rates.resize(termSize);
	for (unsigned int i = 0; i < termSize ; ++i)
	{
		if (terms[i] >= EPS)
		{
			// calc basis zero rate
			const double dff = 1.0;
			const double dft = getBasisDF(terms[i]);
			rates.set(AQLPriceDataConvention::retToRate(dff / dft, terms[i], conv), i);
		}
		else
		{
			rates.set(0.0, i);
		}
	}
	
	if (pos > grids.size() - 1 || grids[grids.size() - 1] > terms.getSize() - 1)
	{
		// error
		AQLString msg = getName();
		msg += " : Input pos or grids are something wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	
	unsigned int center = grids[pos];
	rates.set(rates[center] + width, center);
	if (pos == 0)
	{
		for (unsigned int j = 0; j < center; ++j)
		{
			rates.set(rates[j] + width, j);
		}
		
		if(grids.size() == 1)
		{
			for (unsigned int j = center + 1; j < termSize; ++j)
			{
				rates.set(rates[j] + width, j);
			}
		}
		else
		{
			const unsigned int center_r = grids[pos + 1];
			for (unsigned int j = center + 1; j < center_r; ++j)
			{
				rates.set(rates[j] + width * (terms[center_r] - terms[j]) / (terms[center_r] - terms[center]), j);
			}
		}
	}
	else if (pos == grids.size() - 1)
	{

		for(unsigned int j = center + 1; j < termSize; ++j)
		{
			rates.set(rates[j] + width, j);
		}
		
		const unsigned int center_l = grids[pos - 1];
		for(unsigned int j = center_l + 1; j < center; ++j)
		{
			rates.set(rates[j] + width * (terms[j] - terms[center_l]) / (terms[center] - terms[center_l]), j);
		}
	}
	else
	{
		unsigned int center_l = grids[pos - 1];
		unsigned int center_r = grids[pos + 1];
		for (unsigned int j = center_l + 1; j < center; ++j)
		{
			rates.set(rates[j] + width * (terms[j] - terms[center_l]) / (terms[center] - terms[center_l]), j);
		}
		for (unsigned int j = center + 1; j < center_r; ++j)
		{
			rates.set(rates[j] + width * (terms[center_r] - terms[j]) / (terms[center_r] - terms[center]), j);
		}
	}
	
	for (unsigned int i = 0; i < termSize ; i++)
	{
		// calc df 
		const double val = AQLPriceDataConvention::rateToRet(rates[i], terms[i], conv);
		/*if (val < 1.0)
		{
			throw AQLCoreInvalidData("DF is over one.", __FILE__, __LINE__);
		}*/
		rates.set(1.0 / val, i);
	}
	
	eh_y.remove(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix);
	eh_y.add(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix, new AQLDataDoubles(rates));
}

/*!
    @brief set BasisDF By parallel ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setBasisDFByShiftZero(double width, FloorType type, double floor)
{
	AQLObjectHolder &eh_y = getYieldData().get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> (eh_y.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL).get());
	
	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	
	const unsigned int termSize = terms.getSize();
	AQLDataDoubles rates;
	rates.resize(terms.getSize());
	
	for (unsigned int i = 0; i < termSize ; ++i)
	{
		if (terms[i] >= EPS)
		{
			// calc basis zero rate
			const double dff = 1.0;
			const double dft = getBasisDF(terms[i]);
			rates.set(AQLPriceDataConvention::retToRate(dff / dft, terms[i], conv), i);
		}
		else
		{
			rates.set(0.0, i);
		}
	}
	for(unsigned int i = 0; i < termSize; ++i)
	{
		double rnew = 0.0;
		switch (type)
		{
		case NOFLOOR:
			rnew = rates[i] + width;
			break;		
		case NORMAL:
			rnew = AQLMath::max(rates[i] + width, floor); 
			if (width < 0.0 && rnew > rates[i]) 
			{
				rnew = rates[i];
			}
			break;
		case SYMMETRIC:
            if (width < 0.0)
			{
				rnew = AQLMath::min(rates[i], AQLMath::max(rates[i] + width, floor));
			}
			else
			{
				rnew = AQLMath::min(rates[i], AQLMath::max(rates[i] - width, floor));
				rnew = 2.0 * rates[i] - rnew; 
			}
			break;
		default:
			throw AQLCoreInvalidData("Not Supported Floor Type", __FILE__, __LINE__);		
		}
		const double val = AQLPriceDataConvention::rateToRet(rnew, terms[i], conv);
		/*if (val < 1.0)
		{
			throw AQLCoreInvalidData("DF is over one.", __FILE__, __LINE__);
		}*/

		rates.set(1.0 / val, i);
	}
	eh_y.remove(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix);
	eh_y.add(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix, new AQLDataDoubles(rates));
}

/*!
    @brief set basis rates to data

	@param[in] values

*/
void 
LAMathYieldCurvePro::setBasisRates(const DoubleMatrix& values)
{	
	AQLDataDoubleMatrix& attrvalue = dynamic_cast<AQLDataDoubleMatrix&>(mpBasisRates->get()); 
	attrvalue.set(values);
}

/*!
    @brief calc basis rates
	@param[in] curveType

*/
void
LAMathYieldCurvePro::setBasisRates(const AQLString &curveType)
{
	AQLDataHolder *dh = 0;
	dh = &getData(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS,NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLStringVector& nonvec = dynamic_cast<AQLDataStrings &>(dh->get()).get();
		if (nonvec.end() != std::find(nonvec.begin(),nonvec.end(),curveType))
		{	
			// set generate flag true
			for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
			{
				if (it->second == mAssignedCurveMktMap[curveType])
				{
					const AQLString& curveName = it->first;
					AQLString suffix = "_" + curveName;
					mBCurveGenMap[curveName] = true;
				}
			}
			return;
		}
	}
	
	if (mBCurveGenMap.find(curveType) != mBCurveGenMap.end())
	{
		return;
	}
	
	if (mArbFreeCurveGenMap.find(curveType) != mArbFreeCurveGenMap.end())
	{
		return;
	}

	AQLStringVector gCurveNames;
	dh = &getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
	}

	AQLString suffix = "_" + mAssignedCurveMktMap[curveType];
	// get market data
	const AQLDataMultiReference* mr_ = &getMarketDataRef(mAssignedCurveMktMap[curveType]);
	vector<AQLObject*> data_;
	vector<AQLObject*> data_fwd;
	for(unsigned int i = 0; i < mr_->getSize(); i++)
	{
		// check use grid
		const AQLDataHolder *dh = &mr_->get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;
		
        const AQLString &dataType = dynamic_cast<const AQLDataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		AQLString tmpType = dataType;
		tmpType.toUpper();
		if (tmpType.findString(BASIS) == 0) 
		{
			data_.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(FWDFX) == 0) 
		{
			data_fwd.push_back(&mr_->get(i).get());
		}
	}

	// 
	std::vector<AQLString> addtionalCalibGrid;
	dh = &this->getData(IR_CALIBRATION_DATA_ADDITIONALCALIBGRID + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		addtionalCalibGrid = dynamic_cast<const AQLDataStrings&>(dh->get()).get();
	}
	const AQLInterpolationBase* pSpreadInter;
	dh = &this->AQLObject::getData(IR_CALIBRATION_DATA_INTERPOLATIONBS + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		pSpreadInter = &dynamic_cast<const AQLPriceDataInterpolation&>(dh->get()).getMethod();
	}

	// generate addtional market data through interpolating basis spreads
	std::vector<AQLObject*> data_addtional;
	if (addtionalCalibGrid.size() > 0)
	{
		const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &c_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
		const AQLString c_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();

		bool eom = false;
		dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		AQLString roll_conv_ts = ROLLCONV_NORMAL;
		if (c_freq == LUNAR) roll_conv_ts = ROLLCONV_LUNAR;
		else if (eom) roll_conv_ts = ROLLCONV_EOM;

		const bool isTimeInter = dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL).get()).get();

		for (unsigned int i = 0; i < addtionalCalibGrid.size(); ++i)
		{
			data_addtional.push_back(data_[0]->clone());
			data_addtional[i]->remove(IR_CALIBRATION_DATA_TERM);
			data_addtional[i]->add(IR_CALIBRATION_DATA_TERM, new AQLDataString(addtionalCalibGrid[i]));
			
			AQLDate tmpDate = LAMathDateCalculations::getDate(c_spotdate, addtionalCalibGrid[i], c_sld, &c_cal, true, &roll_conv_ts);
			double term_basis = c_dc.getTerm(c_spotdate, tmpDate);				
			double rate;
			if (isTimeInter)
			{
				rate = pSpreadInter->value(term_basis) / term_basis;
			}
			else
			{
				rate = pSpreadInter->value(term_basis);
			}
			data_addtional[i]->remove(CALIBRATION_DATA_RATE);
			data_addtional[i]->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));

			data_.push_back(data_addtional[i]);
		}
	}

	int fwd_size = data_fwd.size();
	bool fwd_isonly = false;
	// data exist check
	if (data_.empty() && data_fwd.empty())
	{
		AQLString msg = "CurveName = "+ curveType + ", basis/fwdfx data is not set.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	else if (data_.empty() && !data_fwd.empty()) // only fwdfx case
	{
		data_ = data_fwd;
		fwd_isonly = true;
	}

	// sort
	Comp_term comp;
	sort(data_.begin(), data_.end(), comp);
	if (!data_fwd.empty())
	{
		sort(data_fwd.begin(), data_fwd.end(), comp);
		if (!fwd_isonly && comp(data_.front(), data_fwd.back()))
		{
			throw AQLCoreInvalidData("ForwardFX Term must be smaller than CCS Term", __FILE__, __LINE__);
		}
	}

	// foreign currency flag
	bool isForeignCcyLeg = false;
	dh =  &getData(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isForeignCcyLeg = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// 2-swap?
	dh = &data_[0]->getData(IR_CALIBRATION_DATA_IS2SWAP, NOCHECK);
	const bool is2Swap = (dh->isDefined() && !dh->isNull()) && dynamic_cast<const AQLDataBool &>(dh->get()).get();

	// get first element val
	// curve info
	// setBasisRates by recursive
	
	bool isBasisEnabled = false;
	dh = &getData(IR_CALIBRATION_DATA_ENABLECALCULATION, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		isBasisEnabled = dynamic_cast<const AQLDataBool &>(dh->get()).get();

	const AQLString &fCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_FORECAST, ISNOTNULL).get()).get();
	if ((fCurve != STD) && (fCurve != DUMMY) && (fCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), fCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		setBasisRates(fCurve);
#endif
	}
	const AQLString &dCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_DISCOUNT, ISNOTNULL).get()).get();
	if ((dCurve != STD) && (dCurve != DUMMY) && (dCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), dCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		setBasisRates(dCurve);
#endif
	}
	const AQLString &a_fCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST , ISNOTNULL).get()).get();
	if ((a_fCurve != STD) && (a_fCurve != DUMMY) && (a_fCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), a_fCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		if (!isForeignCcyLeg) setBasisRates(a_fCurve);
#endif
	}
	const AQLString &a_dCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL).get()).get();
	if ((a_dCurve != STD) && (a_dCurve != DUMMY) && (a_dCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), a_dCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		if (!isForeignCcyLeg) setBasisRates(a_dCurve);
#endif
	}
	const AQLString *f_dCurve = 0;
	if (is2Swap)
	{
		f_dCurve = &dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_DFCURVENAME, ISNOTNULL).get()).get();
	}
	// check Dummy
	if (fCurve == DUMMY || dCurve == DUMMY || (f_dCurve && (*f_dCurve == DUMMY)))
	{
		throw AQLCoreInvalidData("Dummy curve must be used for against curve only.", __FILE__, __LINE__);
	}
	if ((a_fCurve == DUMMY && a_dCurve != DUMMY) || (a_fCurve != DUMMY && a_dCurve == DUMMY))
	{
		throw AQLCoreInvalidData("If DUMMY curve is used, both forecast and discount must be DUMMY curve.", __FILE__, __LINE__);
	}
	//// check FixedRate
	//if (a_fCurve == FIXEDRATE || a_dCurve == FIXEDRATE && (f_dCurve && (*f_dCurve == FIXEDRATE)))
	//{
	//	throw AQLCoreInvalidData("FIXEDRATE must be used for target curve only.", __FILE__, __LINE__);
	//}
	//if ((fCurve == FIXEDRATE && dCurve != FIXEDRATE) || (fCurve != FIXEDRATE && dCurve == FIXEDRATE))
	//{
	//	throw AQLCoreInvalidData("If FIXEDRATE is used, both forecast and discount must be FIXEDRATE.", __FILE__, __LINE__);
	//}

	const bool isDiscount =  dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
	const bool isTimeInter  = dynamic_cast<const AQLDataBool &>((data_[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL)).get()).get();
	const bool isAgtSpread =  dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();
	// market convention info
	// cashlet
	const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &c_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
	const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
	AQLString c_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
	const int c_lag  = dynamic_cast<const AQLDataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());
	// index
	const AQLPriceDataDayCount &i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
	const AQLPriceDataCalendar &i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &i_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
	const int i_lag  = dynamic_cast<const AQLDataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
	AQLString i_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	AQLString i_accessary = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

	if (!isDiscount)
	{
		// if forecast only
		i_freq.toUpper();
		if (i_freq != SIMPLE)
		{
			throw AQLCoreInvalidData("If forecast mode, only simple is possible in frequency.", __FILE__, __LINE__);
		}
		// if ForeignCcy
		if (isForeignCcyLeg)
		{
			throw AQLCoreInvalidData("If it has foreign ccy leg, target must be discount curve.", __FILE__, __LINE__);
		}
		//// if FixedRate
		//if (fCurve == FIXEDRATE)
		//{
		//	throw AQLCoreInvalidData("If FIXEDRATE is used, target must be discount curve.", __FILE__, __LINE__);
		//}
	}

	// against cashlet
	const AQLPriceDataDayCount &a_c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &a_c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &a_c_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, ISNOTNULL).get());
	const AQLDate &a_c_spotdate = dynamic_cast<const AQLDataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, ISNOTNULL).get());
	AQLString a_c_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL).get()).get();
	const bool c_isBackward = dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETISBACKWARD, ISNOTNULL).get());
	// against index
	const AQLPriceDataDayCount &a_i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &a_i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, ISNOTNULL).get());
	const AQLPriceDataCalendar &a_i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &a_i_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, ISNOTNULL).get());
	const int a_i_lag  = dynamic_cast<const AQLDataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, ISNOTNULL).get());
	AQLString a_i_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, ISNOTNULL).get()).get();
	AQLString a_i_accessary = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, ISNOTNULL).get()).get();
	const bool a_c_isBackward = dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETISBACKWARD, ISNOTNULL).get());

	// simultaneous equation
	bool isSimuEq = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSimuEq = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	// issamegridindex
	bool isSameGridIndex = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSameGridIndex = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	// isoddtermfrnindex
	bool isOddTermFRNIndex = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISODDTERMFRNINDEX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isOddTermFRNIndex = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	// isyieldspreadcalc
	bool isYieldSpreadCalc = true;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISYIELDSPREADCALC, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isYieldSpreadCalc = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	if (isForeignCcyLeg && isYieldSpreadCalc)
	{
		throw AQLCoreInvalidData("yield spread calc flag must be false.", __FILE__, __LINE__);
	}
	//iseomroll
	bool eom = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	// isfwdinterpolation
	bool isFWDInter = false;
	AQLPriceDataInterpolation *a_fwdInter = 0;
	AQLPriceDataInterpolation *fwdInter = 0;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isFWDInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		a_fwdInter = &dynamic_cast<AQLPriceDataInterpolation &>(data_[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		if (isDiscount)
			fwdInter = dynamic_cast<AQLPriceDataInterpolation *>(a_fwdInter->clone());
	}
	// compounding	
	AQLString c_freq_cpd(c_freq), a_c_freq_cpd(a_c_freq);	
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, NOCHECK));	
	if (dh->isDefined() && !dh->isNull())	
	{	
		c_freq_cpd = dynamic_cast<const AQLDataString &>(dh->get()).get();
		AQLDate period_reset(LAMathDateCalculations::getDate(c_spotdate, FrequencyToTerm(c_freq), true));
		AQLDate period_payment(LAMathDateCalculations::getDate(c_spotdate, FrequencyToTerm(c_freq_cpd), true));
		if (period_reset > period_payment)
		{
			throw AQLCoreInvalidData("Payment frequency must be wider than reset frequency.", __FILE__, __LINE__);
		}
	}	
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, NOCHECK));	
	if (dh->isDefined() && !dh->isNull())	
	{	
		a_c_freq_cpd = dynamic_cast<const AQLDataString &>(dh->get()).get();
		AQLDate period_reset(LAMathDateCalculations::getDate(a_c_spotdate, FrequencyToTerm(a_c_freq), true));
		AQLDate period_payment(LAMathDateCalculations::getDate(a_c_spotdate, FrequencyToTerm(a_c_freq_cpd), true));
		if (period_reset > period_payment)
		{
			throw AQLCoreInvalidData("Payment frequency must be wider than reset frequency.", __FILE__, __LINE__);
		}
	}
	if (!mpCompoundFunction->isDefined() || mpCompoundFunction->isNull())
	{
		throw AQLCoreInvalidData("no valid compounding function is set.", __FILE__, __LINE__);
	}

	// get max term
	const AQLString &termMax = dynamic_cast<const AQLDataString &>((data_.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	// calc term (apply to month)
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	c_freq_cpd.toUpper();
	unsigned int mUnit;
	if (isDiscount)
	{
		mUnit = LAMathDateCalculations::getPeriodFrequencyInMonths(c_freq_cpd);
	}
	else
	{
		mUnit = LAMathDateCalculations::getPeriodFrequencyInMonths(c_freq);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		AQLString msg = "frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	a_c_freq_cpd.toUpper();
	unsigned int a_mUnit = LAMathDateCalculations::getPeriodFrequencyInMonths(a_c_freq_cpd);

	if ((c_freq == LUNAR && a_c_freq != LUNAR) || (c_freq != LUNAR && a_c_freq == LUNAR))
	{
		AQLString msg = "if the one frequency is LUNAR, the other must be LUNAR";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int step = 1;
	unsigned int a_step = 1;
	if (mUnit < a_mUnit)
	{
		step = a_mUnit / mUnit;
	}
	else
	{
		a_step = mUnit / a_mUnit;
	}

	// set roll convention
	AQLString roll_conv(LAPriceYieldGenerator::deduceRollConvention(c_freq, eom));

	// fixed cashlet in 2-swap
	const AQLPriceDataCalendar *f_cal = 0;
	const AQLDate *f_spotdate = 0;
	bool f_isBackward = false;
	bool f_eom = false;
	if (is2Swap)
	{
		f_cal = dynamic_cast<const AQLPriceDataCalendar *>(&data_[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		f_spotdate = &dynamic_cast<const AQLDataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get()).get();
		f_eom = dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, ISNOTNULL).get()).get();
		if (isAgtSpread)
		{
			f_isBackward = a_c_isBackward;
		}
		else
		{
			f_isBackward = c_isBackward;
		}
	}

	AQLObjectHolder &objHolder = getYieldData().get();
	AQLObject &yieldData = objHolder.get();

	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());	

	// is xccy swap marked to market or not
	bool isXccyMarkedToMarket = dynamic_cast<const AQLDataBool& > ((getData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST,
															               IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, ISNOTNULL)).get()); // Alias Method: First Parameter Takes Priority
	bool isUSD = false;
	AQLString currency = dynamic_cast<AQLDataString&> ((yieldData.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
	if (currency.toUpper() == "USD") isUSD = true;

	AQLPriceDataDayCount dc_act(ACT_365_ISDA);
	DoubleArray b_t(1, 0.0);
	DoubleArray df_mod(1, 1.0);
	DoubleArray _terms;
	DoubleArray std_terms;
	DoubleArray std_dfs;
	bool is_std_JoinDateExists = false;
	double std_linearSplineJoinDateAsDouble = 0.;
	if (fCurve == STD || dCurve == STD 
		|| !isForeignCcyLeg && (a_fCurve == STD || a_fCurve == DUMMY || a_fCurve == STD || a_fCurve == DUMMY))
	{
		std_terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
		std_dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL)).get()).get();
		const AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			is_std_JoinDateExists = true;
			std_linearSplineJoinDateAsDouble = dynamic_cast<const AQLDataDouble&>(dh->get()).get();
		}
	}
	AQLPriceDataInterpolation f_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (fCurve == STD)
	{
		if (is_std_JoinDateExists)
		{
			f_inter.setJoinDateAsDouble(std_linearSplineJoinDateAsDouble);
		}
		f_inter.set(std_terms, std_dfs);
		if (!isDiscount)
		{
			_terms = std_terms;
		}
	}
	else if (fCurve == FIXEDRATE)
	{
		const DoubleArray terms{ 0., m * 12. };
		const DoubleArray dfs{ 1., 1. };
		f_inter.set(terms, dfs);
	}
	else
	{
		const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + fCurve, ISNOTNULL)).get()).get();
		const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + fCurve, ISNOTNULL)).get()).get();
		const AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + AQLString("_") + fCurve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			f_inter.setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
		}
		f_inter.set(terms, dfs);
		if (!isDiscount)
		{
			_terms = terms;
		}
	}
	AQLPriceDataInterpolation d_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (dCurve == STD)
	{
		if (is_std_JoinDateExists)
		{
			d_inter.setJoinDateAsDouble(std_linearSplineJoinDateAsDouble);
		}
		d_inter.set(std_terms, std_dfs);
		if (isDiscount)
		{
			_terms = std_terms;
		}
	}
	else if (dCurve == FIXEDRATE)
	{
		const DoubleArray terms{ 0., m * 12. };
		const DoubleArray dfs{ 1., 1. };
		d_inter.set(terms, dfs);
	}
	else
	{
		const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + dCurve, ISNOTNULL)).get()).get();
		const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + dCurve, ISNOTNULL)).get()).get();
		const AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + AQLString("_") + dCurve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			d_inter.setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
		}
		d_inter.set(terms, dfs);
		if (isDiscount)
		{
			_terms = terms;
		}
	}

	// against side
	AQLPriceDataInterpolation a_f_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (isForeignCcyLeg)
	{
		AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{	
			throw AQLCoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			AQLObject &fYieldData = eh_fy.get();
			AQLString suffix;
			if (a_fCurve != STD) suffix = AQLString("_") +  a_fCurve;
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
			const AQLDataHolder* dh = &fYieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_f_inter.setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			}
			a_f_inter.set(terms, dfs);
		}		
	}
	else
	{
		if (a_fCurve == STD || a_fCurve == DUMMY)
		{
			if (is_std_JoinDateExists)
			{
				a_f_inter.setJoinDateAsDouble(std_linearSplineJoinDateAsDouble);
			}
			a_f_inter.set(std_terms, std_dfs);
		}
		else if (a_fCurve == FIXEDRATE)
		{
			const DoubleArray terms{ 0, m * 12. };
			const DoubleArray dfs{ 1., 1. };
			a_f_inter.set(terms, dfs);
		}
		else
		{
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") +  a_fCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") +  a_fCurve, ISNOTNULL)).get()).get();
			const AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + AQLString("_") + a_fCurve, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_f_inter.setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			}
			a_f_inter.set(terms, dfs);
		}
	}
	AQLPriceDataInterpolation a_d_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (isForeignCcyLeg)
	{
		AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{	
			throw AQLCoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			AQLObject &fYieldData = eh_fy.get();
			AQLString suffix;
			if (a_dCurve != STD) suffix = AQLString("_") +  a_dCurve;
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
			const AQLDataHolder* dh = &fYieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_d_inter.setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			}
			a_d_inter.set(terms, dfs);
		}		
	}
	else
	{
		if (a_dCurve == STD || a_dCurve == DUMMY)
		{
			if (is_std_JoinDateExists)
			{
				a_d_inter.setJoinDateAsDouble(std_linearSplineJoinDateAsDouble);
			}
			a_d_inter.set(std_terms, std_dfs);
		}
		else
		{
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") +  a_dCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + a_dCurve, ISNOTNULL)).get()).get();
			const AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + AQLString("_") + a_dCurve, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_d_inter.setJoinDateAsDouble(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			}
			a_d_inter.set(terms, dfs);
		}
	}
	std::shared_ptr<AQLPriceDataInterpolation> f_d_inter;
	if (is2Swap)
	{
		f_d_inter.reset(new AQLPriceDataInterpolation(dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())));
		if (*f_dCurve == STD)
		{
			f_d_inter->set(std_terms, std_dfs);
		}
		else
		{
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + *f_dCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + *f_dCurve, ISNOTNULL)).get()).get();
			f_d_inter->set(terms, dfs);
		}
	}

	const unsigned int max = m / mUnit + 1;
	AQLDate fdate = c_spotdate;
	AQLDate a_fdate = a_c_spotdate;
	AQLString strUnit = AQLString(static_cast<int>(mUnit)) + AQLString("M");

	const unsigned int MAX_LOOP = 100;

	const double spotTerm = dc_act.getTerm(asof, c_spotdate);
	const double d_spotdf = d_inter.value(spotTerm);
	const double f_spotdf = f_inter.value(spotTerm);
	const double a_spotTerm = dc_act.getTerm(asof, a_c_spotdate);
	const double a_d_spotdf = a_d_inter.value(a_spotTerm); 

	// if spotTerm is different between legs, adjust against leg df.
	double a_d_df_adjust = 1.0;
	if (isDiscount && (spotTerm != a_spotTerm)) a_d_df_adjust = a_d_spotdf / a_d_inter.value(spotTerm);

	// calc extrapolation terms
	AQLString maxTerm, maxFreq;
	DoubleArray extra_terms;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		maxTerm = dynamic_cast<AQLDataString&>(dh->get()).get();
		maxTerm += "Y";
		maxFreq = dynamic_cast<AQLDataString&>(objHolder.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
		const AQLDate& maxDate = LAMathDateCalculations::getDate(c_spotdate, maxTerm, c_sld, &c_cal, true, &roll_conv);
		DateVector tmp_dates; DoubleArray tmp_taus;
		LAPriceYieldGenerator::getPaymentDates(c_spotdate, maxDate, maxFreq, c_cal, c_sld, dc_act, tmp_dates, extra_terms, tmp_taus, eom);
	}

	// calc DF from FwdFX
	double fwd_spotTerm = 0.0;
	double fwd_spotdf = 1.0;
	int fwd_size_mm = 0;
	DoubleArray fwd_terms; 
	DoubleArray fwd_dfs;
	if (fwd_size > 0)
	{
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		// Here the getData method has been overridden to accept an alias if the first name is missing
		const bool isFXForwardQuotedAsOutright = dynamic_cast<const AQLDataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISRATIO, IR_CALIBRATION_DATA_ISFXOUTRIGHT, ISNOTNULL).get()).get();
		const AQLPriceDataCalendar &fwd_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_fwd[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &fwd_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data_fwd[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const AQLDate &fwd_spotdate = dynamic_cast<const AQLDataDate &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
		const int fwd_spotlag = dynamic_cast<const AQLDataInt &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get()).get();
		fwd_spotTerm = dc_act.getTerm(asof, fwd_spotdate);
		fwd_spotdf = d_inter.value(fwd_spotTerm);
		AQLString fwd_freq = dynamic_cast<const AQLDataString &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
		const bool fwd_eom = dynamic_cast<const AQLDataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL).get()).get();

		// set roll convention
		AQLString fwd_roll_conv("");
		if (fwd_freq == LUNAR) fwd_roll_conv = ROLLCONV_LUNAR;
		else if (fwd_eom) fwd_roll_conv = ROLLCONV_EOM;
		else fwd_roll_conv = ROLLCONV_NORMAL;

		// calc extrapolation terms for fwdfx
		if (fwd_isonly && extra_terms.size() > 0)
		{
			extra_terms.clear();
			const AQLDate& maxDate = LAMathDateCalculations::getDate(fwd_spotdate, maxTerm, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
			DateVector tmp_dates; DoubleArray tmp_taus;
			LAPriceYieldGenerator::getPaymentDates(fwd_spotdate, maxDate, maxFreq, fwd_cal, fwd_sld, dc_act, tmp_dates, extra_terms, tmp_taus, fwd_eom);
		}

		double term = 0.0;
		double df = 1.0;
		AQLDate end;

		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if ( isFXForwardQuotedAsOutright )
		{
			double fwd_ratio_pow = 1.0;
			for (int i = 0; i < fwd_size; i++)
			{
				const double fwd_ratio = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const AQLString &fwd_termStr  = dynamic_cast<const AQLDataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				if (fwd_termStr == "ON")
				{
					if (fwd_spotlag < 1) continue;
					else
					{
						fwd_ratio_pow *= fwd_ratio;
						end = LAMathDateCalculations::getDate(asof, fwd_termStr, SLIDING_RULE_FOLLOWING, &fwd_cal, true);
						term = dc_act.getTerm(asof, end);
						df = a_d_inter.value(term) / fwd_ratio_pow;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else if (fwd_termStr == "TN")
				{
					if (fwd_spotlag < 2) continue;
					else
					{
						fwd_ratio_pow *= fwd_ratio;
						term = dc_act.getTerm(asof, fwd_spotdate); //term is asof to spot if spotlag != 2
						df = a_d_inter.value(term) / fwd_ratio_pow;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else
				{
					end = LAMathDateCalculations::getDate(fwd_spotdate, fwd_termStr, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
					term = dc_act.getTerm(asof, end);
					df =a_d_inter.value(term) / (fwd_ratio * fwd_ratio_pow);
				}

				if (!fwd_terms.size() || fwd_terms.back() < term)
				{
					fwd_terms.push_back(term);
					fwd_dfs.push_back(df);
				}
			}
		}
		else
		{
			const bool fwd_ispriceccy = dynamic_cast<const AQLDataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISPRICECCY,
																							  IR_CALIBRATION_DATA_ISDOMESTICCURRENCY, ISNOTNULL).get()).get();
			//get spot fx
			const LAMathFXEntity& fx = dynamic_cast<const LAMathFXEntity&>(getFXEntity().get().get());
			const AQLStringVector spotccys = fx.getCurrencys().get();
			const DoubleArray spotfxs = fx.getSpotRates().get();
			double spotfx_unitccy = 0.0;
			double spotfx_usd_unitccy = 0.0;
			for (unsigned int i = 0; i < spotccys.size(); i++)
			{
				if (spotccys[i] == currency)
					spotfx_unitccy  = spotfxs[i];
				else if (spotccys[i] == "USD")
					spotfx_usd_unitccy  = spotfxs[i];
			}
			if (spotfx_unitccy == 0.0 || spotfx_usd_unitccy == 0.0)
			{
				throw AQLCoreInvalidData("SpotFX rates do not exist!", __FILE__, __LINE__); 
			}
			const double spotfx = fwd_ispriceccy ? spotfx_usd_unitccy / spotfx_unitccy : spotfx_unitccy / spotfx_usd_unitccy;
			//search ON&TN fwd spread
			double fwd_spread_on = 0.0;
			double fwd_spread_tn = 0.0;
			for (int i = 0; i < fwd_size; i++)
			{
				const AQLString &termStr = dynamic_cast<const AQLDataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				if (termStr == "ON")
					fwd_spread_on = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				else if (termStr == "TN")
					fwd_spread_tn = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				else if (fwd_spread_on != 0.0 && fwd_spread_tn != 0.0)
					break;
			}
			const double fwd_fx_on = fwd_spotlag != 1 ? spotfx - fwd_spread_tn - fwd_spread_on : spotfx - fwd_spread_on;
			const double fwd_fx_tn = spotfx - fwd_spread_tn;
			for (int i = 0; i < fwd_size; i++)
			{
				const AQLString &fwd_termStr  = dynamic_cast<const AQLDataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				double fwd_ratio = 1.0;
				if (fwd_termStr == "ON")
				{
					if (fwd_spotlag < 1) continue;
					else
					{
						if (fwd_spotlag == 1)
							fwd_ratio = fwd_ispriceccy ? fwd_fx_on / spotfx : spotfx / fwd_fx_on;
						else
							fwd_ratio = fwd_ispriceccy ? fwd_fx_on / fwd_fx_tn : fwd_fx_tn / fwd_fx_on;
						end = LAMathDateCalculations::getDate(asof, fwd_termStr, SLIDING_RULE_FOLLOWING, &fwd_cal, true);
						term = dc_act.getTerm(asof, end);
						df = a_d_inter.value(term) * fwd_ratio;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else if (fwd_termStr == "TN")
				{
					if (fwd_spotlag < 2) continue;
					else
					{
						fwd_ratio = fwd_ispriceccy ? fwd_fx_on / spotfx : spotfx / fwd_fx_on;
						term = dc_act.getTerm(asof, fwd_spotdate); //term is asof to spot if spotlag != 2
						df = a_d_inter.value(term) * fwd_ratio;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else
				{
					const double fwd_spread = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
					const double fwd_fx = spotfx + fwd_spread;
					if (fwd_spotlag == 0)
						fwd_ratio = fwd_ispriceccy ? spotfx / fwd_fx : fwd_fx / spotfx;
					else
						fwd_ratio = fwd_ispriceccy ? fwd_fx_on / fwd_fx : fwd_fx / fwd_fx_on;
					end = LAMathDateCalculations::getDate(fwd_spotdate, fwd_termStr, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
					term = dc_act.getTerm(fwd_spotdate, end) + fwd_spotTerm;
					df = a_d_inter.value(term) * fwd_ratio;
				}

				if (!fwd_terms.size() || fwd_terms.back() < term)
				{
					fwd_terms.push_back(term);
					fwd_dfs.push_back(df);
				}
			}
		}
		fwd_size = fwd_terms.size();
	}

	if (fwd_isonly)
	{
		DoubleVector terms(fwd_terms.begin(), fwd_terms.end());
		DoubleVector dfs(fwd_dfs.begin(), fwd_dfs.end());

		// insert front data
		DoubleMatrix termsmtx_fwd(2);
		termsmtx_fwd[0] = terms;
		termsmtx_fwd[0].erase(termsmtx_fwd[0].end() - 1);
		termsmtx_fwd[1] = terms;
		termsmtx_fwd[1].erase(termsmtx_fwd[1].begin());
		if (fwd_spotTerm == 0.0 && !termsmtx_fwd[0].empty() && termsmtx_fwd[0].front() != 0.0)
		{
			termsmtx_fwd[0].insert(termsmtx_fwd[0].begin(), 0.0);
			termsmtx_fwd[1].insert(termsmtx_fwd[1].begin(), terms.front());
		}

		terms.insert(terms.begin(), 0.0);
		dfs.insert(dfs.begin(), 1.0);

		// insert extrapolation terms
		if (extra_terms.size() > 0 && extra_terms.back() + fwd_spotTerm > terms.back())
		{
			for (int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + fwd_spotTerm;
				if (term <= terms.back())
					continue;
				terms.push_back(term);
				dfs.push_back(dfs.back());
				if (!termsmtx_fwd[0].empty())
				{
					termsmtx_fwd[0].push_back(termsmtx_fwd[1].back());
					termsmtx_fwd[1].push_back(term);
				}
			}
		}

		saveBasisCurve(curveType, terms, termsmtx_fwd, dfs, data_);

		return;
	}

	//analytic for CalcDatesForDVZero
	DateVector dvzeroDates;
	DoubleArray yield_mod;
	DoubleMatrix termsmtx_fwd(2);
	if (isSimuEq)
	{
		const double EPS_PV = 1.0e-10;
		const double MIN_VAL = 1.0e-10;

		unsigned int b_size = data_.size();
		if (!b_size)
		{
			throw AQLCoreInvalidData("Basis data is empty.", __FILE__, __LINE__);
		}
		DoubleArray a_targetPVVec(b_size, 0.0);
		DoubleArray spreadVec(b_size, 0.0);
		vector<DateVector> datesVec(b_size);
		vector<DoubleArray> gridVec(b_size);
		vector<DoubleArray> tauVec(b_size);
		vector<DoubleArray> dfsVec(b_size);
		vector<DoubleMatrix> i_gridMatVec(b_size);
		vector<DoubleMatrix> i_termMatVec(b_size);
		vector<IntArray> cpd_timesVec(b_size);
		vector<DoubleArray> a_gridVec(b_size);
		vector<DoubleArray> a_tauVec(b_size);
		vector<DoubleMatrix> a_i_gridMatVec(b_size);
		vector<DoubleMatrix> a_i_termMatVec(b_size);
		vector<IntArray> a_cpd_timesVec(b_size);
		vector<DoubleMatrix> b_yieldTimeMatVec(b_size);

		AQLPriceDataInterpolation spread_time_inter = getInterpolation_yg();
		DoubleArray grid_spread_time((fwd_size - fwd_size_mm) + b_size, 0.0);
		DoubleArray spread_timeVec((fwd_size - fwd_size_mm) + b_size, 0.0);
		double linearSplineJoinDateAsDouble_spread_time_inter = 0.;
		spread_time_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble_spread_time_inter);

		if (fwd_terms.size() > 0 && fwd_spotTerm != spotTerm)
		{
			AQLPriceDataInterpolation tmp_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
			tmp_inter.set(fwd_terms, fwd_dfs);
			fwd_spotdf = tmp_inter.value(spotTerm);//to adjust for xccy basis spot
		}
		for (int i = fwd_size_mm; i < fwd_size; i++)
		{
			grid_spread_time[i - fwd_size_mm] = fwd_terms[i] - spotTerm;
			spread_timeVec[i - fwd_size_mm] = -AQLMath::log(fwd_dfs[i] / fwd_spotdf);
		}

		AQLString a_fMarket = getMarketForCurve(a_fCurve);
		AQLString a_dMarket = getMarketForCurve(a_dCurve);
		bool isSameMarket = false;
		if (a_fMarket == a_dMarket)
		{
			if (a_fMarket == IR_NO_DATA)
				isSameMarket = a_fCurve == a_dCurve;
			else
				isSameMarket = true;
		}
		if (isFWDInter)
		{
			AQLPriceDataDayCount dc;
			AQLPriceDataCalendar cal;
			AQLPriceDataSlidingRule sld;
			AQLString accessory;
			AQLString freq;
			DoubleArray terms;
			DoubleArray fwds;
			DoubleArray taus;
			DoubleMatrix termsMat;
			double linearSplineJoinDateAsDouble = 0.;
			// target curve
			if (isDiscount && fCurve != DUMMY)
			{
				getForwardConvention(fCurve, dc, sld, cal, accessory);
				getDayCount(fCurve) = dc;
				getBaseForwardRate(fCurve, terms, termsMat, taus, fwds, linearSplineJoinDateAsDouble);
				fwdInter->setJoinDateAsDouble(linearSplineJoinDateAsDouble);
				fwdInter->set(terms, fwds);
			}
			// against curve;
			if (a_fCurve != DUMMY)
			{
				if (isForeignCcyLeg)
				{
					AQLString fYieldDataName = dynamic_cast<const AQLDataString &>(getForeignYieldData().get().get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					AQLDataInstance* dataInstance = getDataInstance();
					LAMathYieldCurve fYC(dataInstance);
					fYC.getYieldData().convertFromString(fYieldDataName);
					fYC.getCurveConvention(freq, cal, sld, dc, accessory, a_fCurve);
					fYC.getDayCount(a_fCurve) = dc;
					fYC.setInterpolation(a_fwdInter->convertToString());
					fYC.getBaseForwardRate(a_fCurve, terms, termsMat, taus, fwds, linearSplineJoinDateAsDouble);
				}	
				else
				{
					getForwardConvention(a_fCurve, dc, sld, cal, accessory);
					getDayCount(a_fCurve) = dc;
					getBaseForwardRate(a_fCurve, terms, termsMat, taus, fwds, linearSplineJoinDateAsDouble);
				}
				a_fwdInter->setJoinDateAsDouble(linearSplineJoinDateAsDouble);
				a_fwdInter->set(terms, fwds);
			}
		}
		for (unsigned int i = 0; i < b_size; ++i)
		{
			double spread = dynamic_cast<const AQLDataDouble &>((data_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();


			const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

			if (is2Swap)
			{
				const AQLPriceDataDayCount *f_dc = dynamic_cast<const AQLPriceDataDayCount *>(&data_[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
				const AQLString *f_freq = &dynamic_cast<const AQLDataString &>(data_[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
				const AQLPriceDataSlidingRule *f_sld = dynamic_cast<const AQLPriceDataSlidingRule *>(&data_[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());

				// Target (no 2-swap-based spread)
				double t_annuity = 0.0;
				if (isAgtSpread)
				{
					const AQLString a_roll_conv = LAPriceYieldGenerator::deduceRollConvention(a_c_freq, eom);
					DateVector a_dates;
					DoubleArray a_terms_grid;
					DoubleArray a_terms_interval;
					IntArray a_cpd_times;
					deducePaymentDatesAndTerms(a_c_spotdate, strTerm, a_c_sld, a_c_cal, a_roll_conv, a_c_freq, a_c_freq_cpd, a_c_dc, eom, a_c_isBackward, a_dates, a_terms_grid, a_terms_interval, a_cpd_times);
					t_annuity = calcAnnuity(a_spotTerm, a_terms_grid, a_terms_interval, a_d_inter);
				}
				else
				{
					DateVector dates;
					DoubleArray terms_grid;
					DoubleArray terms_interval;
					IntArray cpd_times;
					deducePaymentDatesAndTermsThisSide(strTerm, roll_conv, eom, isSameGridIndex,
						c_spotdate, c_sld, c_cal, c_freq, c_freq_cpd, c_dc, c_isBackward, a_c_spotdate,
						dates, terms_grid, terms_interval, cpd_times);
					t_annuity = calcAnnuity(spotTerm, terms_grid, terms_interval, d_inter);
				}

				// Original (2-swap-based spread)
				const AQLString o_roll_conv = LAPriceYieldGenerator::deduceRollConvention(*f_freq, f_eom);
				DateVector o_dates;
				DoubleArray o_terms_grid;
				DoubleArray o_terms_interval;
				IntArray o_cpd_times;
				deducePaymentDatesAndTerms(*f_spotdate, strTerm, *f_sld, *f_cal, o_roll_conv, *f_freq, *f_freq, *f_dc, eom, f_isBackward, o_dates, o_terms_grid, o_terms_interval, o_cpd_times);
				const double o_annuity = calcAnnuity(spotTerm, o_terms_grid, o_terms_interval, *f_d_inter);

				// Convert spread from original to target
				spread *= o_annuity / t_annuity;
			}

			spreadVec[i] = spread;
			// set isOddTerm
			int y_Term, m_Term, d_Term, w_Term;
			LAMathDateCalculations::termStrtoYMDW(strTerm, y_Term, m_Term, d_Term, w_Term);
			bool a_isOddTerm = (12 * y_Term + m_Term) % a_mUnit != 0;
			bool isOddTerm = (12 * y_Term + m_Term) % mUnit != 0;

			if (isAgtSpread || (a_fCurve != DUMMY  && (!isSameGridIndex || !isSameMarket || isFWDInter)))
			{
				DateVector a_dates;
				DoubleArray a_terms_grid;
				DoubleArray a_terms_interval;
				IntArray a_cpd_times;
				deducePaymentDatesAndTerms(a_c_spotdate, strTerm, a_c_sld, a_c_cal, roll_conv, a_c_freq, a_c_freq_cpd, a_c_dc, eom, a_c_isBackward, a_dates, a_terms_grid, a_terms_interval, a_cpd_times);
				DoubleMatrix a_i_gridMat(a_dates.size());
				DoubleMatrix a_i_termMat(a_dates.size());				
				double targetPV = 0.0;
				double df = 1.0;
				AQLDate fDate = a_c_spotdate;

				for (unsigned int j = 0; j < a_dates.size(); ++j)
				{
					DoubleArray a_i_gridVec;
					DoubleArray a_i_termVec;
					if (isSameGridIndex)
					{
						if (a_isOddTerm && isOddTermFRNIndex && a_c_isBackward && j == 0)
						{
							calcIndexGrid(asof, a_c_spotdate, a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
						}
						else
						{
						a_i_gridVec.push_back(dc_act.getTerm(asof, fDate));
						a_i_gridVec.push_back(dc_act.getTerm(asof, a_dates[j]));
						a_i_termVec.push_back(a_i_dc.getTerm(fDate, a_dates[j]));
						}
						fDate =  a_dates[j];
					}
					else
					{
						if (j == 0)
						{
							calcIndexGrid(asof, a_c_spotdate, a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
						}
						else
						{
							calcIndexGrid(asof, a_dates[j -1], a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
						}
					}
					a_i_gridMat[j] = a_i_gridVec;
					a_i_termMat[j] = a_i_termVec;
				}
				a_gridVec[i] = a_terms_grid;
				a_tauVec[i] = a_terms_interval;
				a_i_gridMatVec[i] = a_i_gridMat;
				a_i_termMatVec[i] = a_i_termMat;
				a_cpd_timesVec[i] = a_cpd_times;

				if (isDiscount && isXccyMarkedToMarket && !isUSD) // isXccyMarkedToMarket
				{
					//calc after (at calcBasisCF(...))
					//a_targetPVVec[i] = calcAgainstPV(isFWDInter, a_f_inter, a_fwdInter, a_d_inter, spread_time_inter, isAgtSpread ? spread : 0.0, a_c_cpd_times, 
					//					a_spotTerm, spotTerm, a_terms_grid, a_terms_interval, a_i_gridMat, a_i_termMat) * a_d_df_adjust;
				}
				else // spot renotional
				{
					a_targetPVVec[i] = calcAgainstPV(isFWDInter, a_f_inter, a_fwdInter, a_d_inter, isAgtSpread ? spread : 0.0, a_cpd_times, 
										a_spotTerm, a_terms_grid, a_terms_interval, a_i_gridMat, a_i_termMat, a_c_isBackward) * a_d_df_adjust;
				}
			}

			DateVector dates;
			DoubleArray terms_grid;
			DoubleArray terms_interval;
			IntArray cpd_times;
			deducePaymentDatesAndTermsThisSide(strTerm, roll_conv, eom, isSameGridIndex,
				c_spotdate, c_sld, c_cal, c_freq, c_freq_cpd, c_dc, c_isBackward, a_c_spotdate,
				dates, terms_grid, terms_interval, cpd_times);

			// calc basis r * t
			DoubleArray dfs(dates.size());
			DoubleMatrix i_gridMat(dates.size());
			DoubleMatrix i_termMat(dates.size());
			DoubleMatrix b_yieldTimeMat(dates.size());
			AQLDate fDate = c_spotdate;
			for (unsigned int j = 0; j < dates.size(); ++j)
			{
				
				DoubleArray i_gridVec;
				DoubleArray i_termVec;
				if (isSameGridIndex)
				{
					if (isOddTerm && isOddTermFRNIndex && c_isBackward && j == 0)
					{
						calcIndexGrid(asof, c_spotdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
					}
					else
					{
					i_gridVec.push_back(dc_act.getTerm(asof, fDate));
					i_gridVec.push_back(dc_act.getTerm(asof, dates[j]));
					i_termVec.push_back(i_dc.getTerm(fDate, dates[j]));
					}
					fDate =  dates[j];
				}
				else
				{
					if (j == 0)
					{
						calcIndexGrid(asof, c_spotdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
					}
					else
					{
						calcIndexGrid(asof, dates[j - 1], i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
					}
				}
				i_gridMat[j] = i_gridVec;
				i_termMat[j] = i_termVec;

				if (isDiscount)
				{
					if (isYieldSpreadCalc)
					{
						b_yieldTimeMat[j].resize(2);
						b_yieldTimeMat[j][0] = j > 0 ? b_yieldTimeMat[j - 1][1] : 0.;
						b_yieldTimeMat[j][1] = -AQLMath::log(d_inter.value(terms_grid[j] + spotTerm) / d_spotdf);
					}
					else
					{
						b_yieldTimeMat[j] = DoubleArray(1, 0.0);
					}
				}
				else
				{
					DoubleArray b_yieldTimeVec(i_gridVec.size());
					if (isYieldSpreadCalc)
					{
						for (unsigned int k = 1; k < i_gridVec.size(); ++k)
						{
							const double df = f_inter.value(i_gridVec[k]);
							b_yieldTimeVec[k] = -AQLMath::log(df / f_spotdf);
						}
						const double df1 = f_inter.value(i_gridVec[0]);
						b_yieldTimeVec[0] = -AQLMath::log(df1 / f_spotdf) ;
					}
					b_yieldTimeMat[j] = b_yieldTimeVec;
				}

				dfs[j] = d_inter.value(terms_grid[j] + spotTerm) / d_spotdf;
			}
		
			i_gridMatVec[i] = i_gridMat;
			i_termMatVec[i] = i_termMat;
			cpd_timesVec[i] = cpd_times;
			b_yieldTimeMatVec[i] = b_yieldTimeMat;

			gridVec[i] = terms_grid;
			tauVec[i] = terms_interval;
			datesVec[i] = dates;
			dfsVec[i] = dfs;
			grid_spread_time[(fwd_size - fwd_size_mm) + i] = terms_grid.back();
			spread_timeVec[(fwd_size - fwd_size_mm) + i] = -AQLMath::log(dfs.back());
		}

		if (!isDiscount)
		{
			dh = &(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATETERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const AQLString &strTerm_s = dynamic_cast<const AQLDataString &>(dh->get()).get();
				const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				AQLDate date_s = LAMathDateCalculations::getDate(asof, strTerm_s, true);
				AQLDate date = LAMathDateCalculations::getDate(asof, strTerm, true);
				
				if (date_s < date)
				{
					// create data for spotrate
					const DoubleArray &i_gridVec = i_gridMatVec.back().front();
					const DoubleArray &i_termVec = i_termMatVec.back().front();

					DoubleMatrix i_gridMat;
					DoubleMatrix i_termMat;
					i_gridMat.push_back(i_gridVec);
					i_termMat.push_back(i_termVec);

					DoubleArray b_yieldTimeVec(i_gridVec.size());

					if (isYieldSpreadCalc)
					{
						for (unsigned int i = 0; i < spread_timeVec.size(); i++)
						{
							spread_timeVec[i] = 0.;
						}

						for (unsigned int k = 1; k < i_gridVec.size(); ++k)
						{
							const double df = f_inter.value(i_gridVec[k]);
							b_yieldTimeVec[k] = -AQLMath::log(df / f_spotdf);
						}
						const double df1 = f_inter.value(i_gridVec[0]);
						b_yieldTimeVec[0] = -AQLMath::log(df1 / f_spotdf);
					}
					
					DoubleMatrix b_yieldTimeMat;
					b_yieldTimeMat.push_back(b_yieldTimeVec);

					const double term = gridVec.back().front();
					const double tau = tauVec.back().front();
					const double df = d_inter.value(term + spotTerm) / d_spotdf;

					const double spotRate = dynamic_cast<const AQLDataDouble &>(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATE, ISNOTNULL).get()).get();
					const double targetPV = (1.0 + spotRate * tau) * df - 1.0;

					i_gridMatVec.insert(i_gridMatVec.begin(), i_gridMat);
					i_termMatVec.insert(i_termMatVec.begin(), i_termMat);
					cpd_timesVec.insert(cpd_timesVec.begin(), IntArray(1, 1));
					b_yieldTimeMatVec.insert(b_yieldTimeMatVec.begin(), b_yieldTimeMat);

					gridVec.insert(gridVec.begin(), DoubleArray(1, term));
					tauVec.insert(tauVec.begin(), DoubleArray(1, tau));
					datesVec.insert(datesVec.begin(), DateVector(1, datesVec.back().front()));
					dfsVec.insert(dfsVec.begin(),  DoubleArray(1, df));
					grid_spread_time.insert(grid_spread_time.begin(),  term);
					spread_timeVec.insert(spread_timeVec.begin(), isYieldSpreadCalc ? 0. : -AQLMath::log(df));

					spreadVec.insert(spreadVec.begin(), 0.0);
					a_targetPVVec.insert(a_targetPVVec.begin(), targetPV);

					++b_size;
				}
				else if (date_s == date)
				{
					if (gridVec[0].size() != 1)
					{
						throw AQLCoreInvalidData("basis first term, cashlet size must be one", __FILE__, __LINE__);
					}
					const double term = gridVec[0][0];
					const double tau = tauVec[0][0];
					const double df = d_inter.value(term + spotTerm) / d_spotdf;

					const double spotRate = dynamic_cast<const AQLDataDouble &>(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATE, ISNOTNULL).get()).get();
					const double targetPV = (1.0 + spotRate * tau) * df - 1.0;

					if (isAgtSpread)
					{
						DateVector a_dates;
						DoubleArray a_terms_grid;
						DoubleArray a_terms_interval;
						IntArray a_cpd_times;
						deducePaymentDatesAndTerms(a_c_spotdate, strTerm, a_c_sld, a_c_cal, roll_conv, a_c_freq, a_c_freq_cpd, a_c_dc, eom, a_c_isBackward, a_dates, a_terms_grid, a_terms_interval, a_cpd_times);

						DoubleMatrix a_i_gridMat(a_dates.size());
						DoubleMatrix a_i_termMat(a_dates.size());				
						AQLDate fDate = a_c_spotdate;

						for (unsigned int i = 0; i < a_dates.size(); ++i)
						{
							DoubleArray a_i_gridVec;
							DoubleArray a_i_termVec;
							if (isSameGridIndex)
							{
								a_i_gridVec.push_back(dc_act.getTerm(asof, fDate));
								a_i_gridVec.push_back(dc_act.getTerm(asof, a_dates[i]));
								a_i_termVec.push_back(a_i_dc.getTerm(fDate, a_dates[i]));
								fDate =  a_dates[i];
							}
							else
							{
								if (i == 0)
								{
									calcIndexGrid(asof, a_c_spotdate, a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
								}
								else
								{
									calcIndexGrid(asof, a_dates[i -1], a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
								}
							}
							a_i_gridMat[i] = a_i_gridVec;
							a_i_termMat[i] = a_i_termVec;
						}
						double val0 = targetPV - a_targetPVVec[0];
						double spread0 = spreadVec[0];
						double spread1 = spread0 + 0.01;
						double val1 = targetPV - calcAgainstPV(isFWDInter, a_f_inter, a_fwdInter, a_d_inter, spread1, a_cpd_times, 
															a_spotTerm, a_terms_grid, a_terms_interval, a_i_gridMat, a_i_termMat, a_c_isBackward) * a_d_df_adjust;

						int loopNum = MAX_LOOP;
						while (loopNum--)
						{
							if (AQLMath::abs(val0 - val1) < EPS_PV)
							{
								break;
							}
							double dval = (val1 - val0) / (spread1 - spread0);
							double spread2 = spread1 - val1 / dval;
							spread0 = spread1;
							spread1 = spread2;
							val0 = val1;
							val1 = targetPV - calcAgainstPV(isFWDInter, a_f_inter, a_fwdInter, a_d_inter, spread1, a_cpd_times, 
														a_spotTerm, a_terms_grid, a_terms_interval, a_i_gridMat, a_i_termMat, a_c_isBackward) * a_d_df_adjust;

						}
						spreadVec[0] = spread1;
						a_targetPVVec[0] = targetPV;
					}
					else
					{
						const double diff_targetPV = a_targetPVVec[0] - targetPV;
						const double spread = diff_targetPV / (tau * df);
						spreadVec[0] = spread;
					}

					dynamic_cast<AQLDataDouble &>(data_[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).set(spreadVec[0]);
					data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATE);
					data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATETERM);
				}
				else
				{
					AQLString msg = "spotrateterm is after basis first term. spotrateterm = " + strTerm_s + ", basis first term = " + strTerm;
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}
		}
		//// for analytic
		//dvzeroDates = datesVec.back();
		//if (!dvzeroDates.empty() && dvzeroDates[0] == c_spotdate)
		//{
		//	dvzeroDates.erase(dvzeroDates.begin());
		//}

		grid_spread_time.insert(grid_spread_time.begin(), 0.0);
		spread_timeVec.insert(spread_timeVec.begin(), 0.0);
		spread_time_inter.set(grid_spread_time, spread_timeVec);

		//get convexity adjust interpolation
		AQLPriceDataInterpolation* adjust_inter = NULL;
		if (isDiscount && isXccyMarkedToMarket)
		{
			dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DoubleArray& adjustValue_term = dynamic_cast<const AQLDataDoubles&> (dh->get()).get();
				const DoubleArray& adjustValue
					= dynamic_cast<const AQLDataDoubles&> ((data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL)).get()).get();
				adjust_inter = &(dynamic_cast<AQLPriceDataInterpolation&> (data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get()));
				adjust_inter->set(adjustValue_term, adjustValue);
			}
		}

		AQLString OptimizationMethod = dynamic_cast<AQLDataString&>(mpOptimizeMethod->get()).get();
		OptimizationMethod.toUpper();

		if (OptimizationMethod == NR)
		{
#pragma region NEWTONRAPHSON
		DoubleArray targetValVec0(b_size, 0.0);
		calcBasisCF(spread_time_inter, isDiscount, isXccyMarkedToMarket, isUSD, isFWDInter, f_inter, fwdInter, dfsVec, a_f_inter, a_fwdInter, a_d_inter, a_d_df_adjust, adjust_inter, isAgtSpread, spreadVec, 
			cpd_timesVec, a_cpd_timesVec, spotTerm, a_spotTerm, gridVec, tauVec, i_gridMatVec, i_termMatVec, a_gridVec, a_tauVec, a_i_gridMatVec, a_i_termMatVec, 
			b_yieldTimeMatVec, a_targetPVVec, targetValVec0, true);

		// initial perturbation: add 1bps to initial zero rate.
		transform(grid_spread_time.cbegin() + (fwd_size - fwd_size_mm) + 1, grid_spread_time.cend(), 
			spread_timeVec.cbegin() + (fwd_size - fwd_size_mm) + 1, spread_timeVec.begin() + (fwd_size - fwd_size_mm) + 1, [](const double& grid, const double& spread_time) {return spread_time + 0.0001 * grid; });
		spread_time_inter.set(grid_spread_time, spread_timeVec);

		DoubleArray targetValVec1(b_size, 0.0);
		calcBasisCF(spread_time_inter, isDiscount, isXccyMarkedToMarket, isUSD, isFWDInter, f_inter, fwdInter, dfsVec, a_f_inter, a_fwdInter, a_d_inter, a_d_df_adjust, adjust_inter, isAgtSpread, spreadVec, 
			cpd_timesVec, a_cpd_timesVec, spotTerm, a_spotTerm, gridVec, tauVec, i_gridMatVec, i_termMatVec, a_gridVec, a_tauVec, a_i_gridMatVec, a_i_termMatVec, 
			b_yieldTimeMatVec, a_targetPVVec, targetValVec1, true);

		int loopNum = MAX_LOOP;
		while (loopNum--)
		{
			bool isEnd = true;
			for (unsigned int i = 0; i < b_size; ++i)
			{
				if (AQLMath::abs(targetValVec0[i] - targetValVec1[i]) >= EPS_PV)
				{
					isEnd = false;
					break;
				}
			}
			if (isEnd)
			{
				break;
			}

			AQLMatrix divMat(b_size, b_size);
			// create divMat
			for (unsigned int i = 0; i < b_size; ++i)
			{
				DoubleArray spread_timeVec_t = spread_timeVec;
				spread_timeVec_t[(fwd_size - fwd_size_mm) + i + 1] += MIN_VAL;
				spread_time_inter.set(grid_spread_time, spread_timeVec_t);
				DoubleArray vals;
				calcBasisCF(spread_time_inter, isDiscount, isXccyMarkedToMarket, isUSD, isFWDInter, f_inter, fwdInter, dfsVec, a_f_inter, a_fwdInter, a_d_inter, a_d_df_adjust, adjust_inter, isAgtSpread, spreadVec, 
					cpd_timesVec, a_cpd_timesVec, spotTerm, a_spotTerm, gridVec, tauVec, i_gridMatVec, i_termMatVec, a_gridVec, a_tauVec, a_i_gridMatVec, a_i_termMatVec, 
					b_yieldTimeMatVec, a_targetPVVec, vals, false);
				for (unsigned int j = 0; j < b_size; ++j)
				{
					if (vals.size() != b_size )
					{
						throw AQLCoreInvalidData("size error!", __FILE__, __LINE__);
					}
					const double divVal = (vals[j] + targetValVec1[j]) / MIN_VAL;
					divMat.setValue(j, i, divVal);
				}
			}

			AQLMatrix invMat = divMat.inverseMatrix();
			AQLMatrix valMat(targetValVec1);
			AQLMatrix deltaMat = invMat * valMat;
			// plus delta
			for (unsigned int i = 0; i < b_size; ++i)
			{
				spread_timeVec[(fwd_size - fwd_size_mm) + i + 1] += deltaMat.getValue(i, 0);
			}
			spread_time_inter.set(grid_spread_time, spread_timeVec);
			targetValVec0 = targetValVec1;
			calcBasisCF(spread_time_inter, isDiscount, isXccyMarkedToMarket, isUSD, isFWDInter, f_inter, fwdInter, dfsVec, a_f_inter, a_fwdInter, a_d_inter, a_d_df_adjust, adjust_inter, isAgtSpread, spreadVec, 
				cpd_timesVec, a_cpd_timesVec, spotTerm, a_spotTerm, gridVec, tauVec, i_gridMatVec, i_termMatVec, a_gridVec, a_tauVec, a_i_gridMatVec, a_i_termMatVec, 
				b_yieldTimeMatVec, a_targetPVVec, targetValVec1, true);

		}
		if (loopNum < 0)
		{
			for (unsigned int i = 0; i < b_size; ++i)
			{
				// check error is within 0.00001bp
				if (AQLMath::abs(targetValVec1[i]) > 1.0E-9)
				{
					throw AQLCoreInvalidData("Convergence error in DF calc (Newton Raphson)", __FILE__, __LINE__); 
				}
			}
		}
#pragma endregion NEWTONRAPHSON
		}
		else if (OptimizationMethod == NL2SOLALGO)
		{
			DoubleArray targetValVec0(b_size, 0.0);
			//eliminate starting value because it is not optimized (fixed to zero).
			DoubleArray val(spread_timeVec.begin() + 1, spread_timeVec.end());

			//set cost method
			CalibrationCostFunctionBasis costFunc(*this, 
                                                  grid_spread_time, 
												  spread_time_inter, 
												  isDiscount, 
												  isXccyMarkedToMarket, 
												  isUSD, 
												  isFWDInter, 
												  f_inter, 
												  fwdInter, 
												  dfsVec, 
												  a_f_inter, 
												  a_fwdInter, 
												  a_d_inter, 
												  a_d_df_adjust, 
												  adjust_inter, 
												  isAgtSpread, 
												  spreadVec, 
												  cpd_timesVec, 
												  a_cpd_timesVec, 
												  spotTerm, 
												  a_spotTerm, 
												  gridVec, 
												  tauVec, 
												  i_gridMatVec, 
												  i_termMatVec, 
												  a_gridVec,
												  a_tauVec, 
												  a_i_gridMatVec, 
												  a_i_termMatVec, 
												  b_yieldTimeMatVec, 
												  a_targetPVVec, 
												  targetValVec0, 
												  true,
												  MAX_LOOP);

			// calibration
			NL2SOL prob(costFunc);
			long iter = prob.tryToSolve(val);

			if (static_cast<unsigned int>(iter) == MAX_LOOP)
			{
				throw AQLCoreInvalidData("Convergence error in DF calc (NL2SOL)", __FILE__, __LINE__); 
			}

			spread_timeVec.clear();
			spread_timeVec.push_back(0.);
			spread_timeVec.insert(spread_timeVec.end(), val.begin(), val.end());
			spread_time_inter.set(grid_spread_time, spread_timeVec);

		}
		else
		{
			AQLString msg = AQLString("Optimize method ") + OptimizationMethod + " is not supported.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__); 
		}

		AQLPriceDataInterpolation *p_base_inter = 0;
		double b_spotdf = 1.0;
		if (isDiscount)
		{
			p_base_inter = &d_inter;
			b_spotdf = d_spotdf;
		}
		else
		{
			p_base_inter = &f_inter;
			b_spotdf = f_spotdf;
		}

		DoubleArray terms_mod;
		if (isDiscount)
		{
			for (unsigned int i = 0; i < cpd_timesVec.back().size(); ++i)
			{
				int payment_pos = static_cast<int>(std::accumulate(cpd_timesVec.back().cbegin(), cpd_timesVec.back().cbegin() + i + 1, 0.)) - 1;
				if (payment_pos >= gridVec.back().size())
				{
					throw AQLCoreInvalidData("coupon reset time is not consistent to fixing timing.", __FILE__, __LINE__);
				}
				terms_mod.push_back(gridVec.back()[payment_pos]);
			}
		}
		else
		{
			terms_mod = gridVec.back();
		}
		if (terms_mod.empty())
		{
			throw AQLCoreInvalidData("term is empty", __FILE__, __LINE__); 
		}

		DoubleArray terms_mod_fwdmatend = terms_mod;
		for (unsigned int i = 0; i < data_.size(); ++i)
		{
			const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			// set isOddTerm
			int y_Term, m_Term, d_Term, w_Term;
			LAMathDateCalculations::termStrtoYMDW(strTerm, y_Term, m_Term, d_Term, w_Term);
			if ((12 * y_Term + m_Term) % mUnit == 0)
			{
				continue;
			}

			if (isOddTermFRNIndex && !isDiscount)
			{
				int strTerm_int = 12 * y_Term + m_Term + mUnit;
				AQLString strTerm_add(strTerm_int);
				strTerm_add += AQLString("M");

				DateVector dates;
				DoubleArray terms_grid;
				DoubleArray terms_interval;
				IntArray num_reset;
				deducePaymentDatesAndTerms(a_c_spotdate, strTerm_add, c_sld, c_cal, roll_conv, c_freq, c_freq_cpd, c_dc, eom, c_isBackward, dates, terms_grid, terms_interval, num_reset);

				for (unsigned int j = 0; j < terms_grid.size() - 1; ++j)
				{
					unsigned int k = std::distance(terms_mod.cbegin(), std::lower_bound(terms_mod.cbegin(), terms_mod.cend(), terms_grid[j]));
					if (terms_mod.size() <= k || eq(terms_mod[k], terms_grid[j], 1./365.))
					{
						continue;
					}
					terms_mod.insert(terms_mod.begin() + k, terms_grid[j]);
					terms_mod_fwdmatend.insert(terms_mod_fwdmatend.begin() + k + 1, terms_grid[j + 1]);
				}
			}
			else if (isDiscount)
			{
				int strTerm_int = 12 * y_Term + m_Term;
				AQLString strTerm_add(strTerm_int);
				strTerm_add += AQLString("M");

				DateVector dates;
				DoubleArray terms_grid;
				DoubleArray terms_interval;
				IntArray num_reset;
				deducePaymentDatesAndTerms(a_c_spotdate, strTerm_add, c_sld, c_cal, roll_conv, c_freq_cpd, c_freq_cpd, c_dc, eom, c_isBackward, dates, terms_grid, terms_interval, num_reset);

				for (unsigned int j = 0; j < terms_grid.size(); ++j)
				{
					unsigned int k = std::distance(terms_mod.cbegin(), std::lower_bound(terms_mod.cbegin(), terms_mod.cend(), terms_grid[j]));
					if (terms_mod.size() <= k || eq(terms_mod[k], terms_grid[j], 1./365.))
					{
						continue;
					}
					terms_mod.insert(terms_mod.begin() + k, terms_grid[j]);
					terms_mod_fwdmatend.insert(terms_mod_fwdmatend.begin() + k + 1, terms_grid[j] + mUnit / 12.);
				}
			}
		}

		DoubleArray dfs(terms_mod.size());
		if (isYieldSpreadCalc)
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				const double y_spread_time = spread_time_inter.value(terms_mod[i]);
				const double b_spread_time = -AQLMath::log(p_base_inter->value(terms_mod[i] + spotTerm) / b_spotdf);
				dfs[i] = AQLMath::exp(-(b_spread_time + y_spread_time));
			}
		}
		else
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				dfs[i] = AQLMath::exp(-spread_time_inter.value(terms_mod[i]));
			}
		}

		yield_mod.resize(terms_mod.size(), 0.0);

		// set spot -> asof
		if (spotTerm > 0.0)
		{
			double spotdf = 1.0;
			double yield_spot = 0.0;

			if (isDiscount)
			{
				if (fwd_size > 0)
				{
					spotdf = fwd_spotdf;
				}
				else if (isYieldSpreadCalc)
				{
					spotdf = AQLMath::exp((-spread_timeVec[1] / grid_spread_time[1]) * spotTerm) * b_spotdf;
				}
				else
				{
					spotdf = AQLMath::exp((-spread_timeVec[1] / grid_spread_time[1]) * spotTerm);
				}
				yield_spot = -AQLMath::log(spotdf) / spotTerm;
			}
			else
			{
				AQLPriceDataInterpolation tmp_inter = *p_base_inter;
				DoubleArray tmp_yields(dfs.size());
				for (unsigned int i = 0; i < terms_mod.size(); ++i)
				{
					tmp_yields[i] = -AQLMath::log(dfs[i]) / terms_mod[i];
				}
				tmp_inter.set(terms_mod, tmp_yields);
				yield_spot = tmp_inter.value(spotTerm);
				spotdf = AQLMath::exp(-yield_spot * spotTerm);
			}
			
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				terms_mod[i] += spotTerm;
				terms_mod_fwdmatend[i] += spotTerm;
				dfs[i] *= spotdf;
				yield_mod[i] =  -AQLMath::log(dfs[i]) / terms_mod[i];
			}
			
			// insert short term curve		
			if (fwd_size > 0)
			{
				/*for (int i = fwd_size - 1; i >= 0; --i)
				{
					if (fwd_terms[i]+INFINITESIMAL < terms_mod.front())
					{
						terms_mod.insert(terms_mod.begin(), fwd_terms[i]);
						terms_mod_fwdmatend.insert(terms_mod_fwdmatend.begin(), fwd_terms[i]);
						dfs.insert(dfs.begin(), fwd_dfs[i]);
						yield_mod.insert(yield_mod.begin(), -AQLMath::log(fwd_dfs[i]) / fwd_terms[i]);
					}
				}*/

				for (unsigned int i = 0; i < fwd_terms.size(); ++i)
				{
					for (unsigned int k = 0; k < terms_mod.size(); ++k)
					{
						if (k == 0)
						{
							if (fwd_terms[i] + INFINITESIMAL < terms_mod[0])
							{
								terms_mod.insert(terms_mod.begin(), fwd_terms[i]);
								terms_mod_fwdmatend.insert(terms_mod_fwdmatend.begin(), fwd_terms[i]);
								dfs.insert(dfs.begin(), fwd_dfs[i]);
								yield_mod.insert(yield_mod.begin(), -AQLMath::log(fwd_dfs[i]) / fwd_terms[i]);
								break;
							}
						}
						else
						{
							if (terms_mod[k - 1] < fwd_terms[i] - INFINITESIMAL && fwd_terms[i] + INFINITESIMAL < terms_mod[k])
							{
								terms_mod.insert(terms_mod.begin() + k, fwd_terms[i]);
								terms_mod_fwdmatend.insert(terms_mod_fwdmatend.begin() + k, fwd_terms[i]);
								dfs.insert(dfs.begin() + k, fwd_dfs[i]);
								yield_mod.insert(yield_mod.begin() + k, -AQLMath::log(fwd_dfs[i]) / fwd_terms[i]);
								break;
							}
						}
					}
				}
			}
			else
			{
				terms_mod.insert(terms_mod.begin(), spotTerm);
				terms_mod_fwdmatend.insert(terms_mod_fwdmatend.begin(), spotTerm);
				dfs.insert(dfs.begin(), spotdf);
				yield_mod.insert(yield_mod.begin(), yield_spot);
			}
		}
		else
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				yield_mod[i] =  -AQLMath::log(dfs[i]) / terms_mod[i];
			}
		}

		termsmtx_fwd[0] = terms_mod;
		termsmtx_fwd[0].erase(termsmtx_fwd[0].end() - 1);
		termsmtx_fwd[1] = terms_mod_fwdmatend;
		termsmtx_fwd[1].erase(termsmtx_fwd[1].begin());

		if (spotTerm == 0.0 && !termsmtx_fwd[0].empty() && termsmtx_fwd[0].front() != 0.0)
		{
			termsmtx_fwd[0].insert(termsmtx_fwd[0].begin(), 0.0);
			termsmtx_fwd[1].insert(termsmtx_fwd[1].begin(), terms_mod_fwdmatend.front());
		}

		terms_mod.insert(terms_mod.begin(), 0.0);
		dfs.insert(dfs.begin(), 1.0);
		yield_mod.insert(yield_mod.begin(), 0.0);

		// insert extrapolation terms
		if (extra_terms.size() > 0 && extra_terms.back() + spotTerm > terms_mod.back())
		{
			for (int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + spotTerm;
				if (term <= terms_mod.back())
					continue;
				terms_mod.push_back(term);
				dfs.push_back(dfs.back());
				yield_mod.push_back(yield_mod.back());
				if (!termsmtx_fwd[0].empty())
				{
					termsmtx_fwd[0].push_back(termsmtx_fwd[1].back());
					termsmtx_fwd[1].push_back(term);
				}
			}
		}

		_terms = terms_mod;
	}
	else
	{
		throw AQLCoreInvalidData("We support Simultaneous-Equation method only.", __FILE__, __LINE__); 
	}
	//{
	//	double sum1 = 0.0;
	//	double sum2 = 0.0;
	//	double a_sum1 = 0.0;
	//	double a_sum2 = 0.0;
	//	double lt = 0.0;
	//	double ldf = 1.0;
	//
	//	if (!isDiscount)
	//	{
	//		DoubleArray tmp_termVec;
	//		DoubleArray tmp_gridVec;
	//		calcIndexGrid(asof, c_spotdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, tmp_gridVec, tmp_termVec);
	//		lt = tmp_gridVec[0];
	//		ldf = f_inter.value(lt);
	//	}

	//	if (!yieldData.getData(CALIBRATION_DATA_TERMS).isDefined())
	//		throw AQLCoreInvalidData("terms error", __FILE__,__LINE__);
	//	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	//	// maxterm for calc
	//	const double tmax = terms.back();

	//	// get interpolation method
	//	AQLPriceDataInterpolation &b_inter = getInterpolation_bs();
	//	DoubleArray b_t_grid(1, 0.0);
	//	DoubleArray b_termstruct_grid(1, 0.0);
	//	for (unsigned int i = 0; i < data_.size(); i++)
	//	{
	//		const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//		double rate = dynamic_cast<const AQLDataDouble &>((data_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	//		double term = 0.0;
	//		if (isAgtSpread)
	//		{
	//			AQLDate tmpDate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm, a_c_sld, &a_c_cal, true, &a_c_roll_conv);
	//			term = a_c_dc.getTerm(a_c_spotdate, tmpDate);
	//		}
	//		else
	//		{
	//			AQLDate tmpDate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &c_roll_conv);
	//			term = c_dc.getTerm(c_spotdate, tmpDate);
	//		}

	//		b_t_grid.push_back(term);
	//		if (isTimeInter)
	//		{
	//			b_termstruct_grid.push_back(rate * term);
	//		}
	//		else
	//		{
	//			b_termstruct_grid.push_back(rate);
	//		}
	//	}
	//	b_inter.set(b_t_grid, b_termstruct_grid);

	//	const double EPS_PV = 1.0e-15;
	//	DoubleArray p_time(1, 0.0);
	//	for (unsigned int i = 1; i < max; i += step )
	//	{
	//		if (lt > tmax)
	//		{
	//			break;
	//		}
	//		AQLString strTerm = AQLString(static_cast<int>(mUnit * (i - 1))) + AQLString("M");
	//
	//		double target_pv = 1.0;
	//		if (a_fCurve != DUMMY)
	//		{
	//			double spread = 0.0;
	//			if (isAgtSpread)
	//			{
	//				AQLString a_dfstrTerm = AQLString(static_cast<int>(mUnit * (i - 1) + a_mUnit * a_step)) + AQLString("M");
	//				AQLDate a_dfdate = LAMathDateCalculations::getDate(a_c_spotdate, a_dfstrTerm, a_c_sld, &a_c_cal, true);
	//				const double s_lterm_ = c_dc.getTerm(a_c_spotdate, a_dfdate);
	//				if (isTimeInter)
	//				{
	//					spread = b_inter.value(s_lterm_) / s_lterm_;
	//				}
	//				else
	//				{
	//					spread = b_inter.value(s_lterm_);
	//				}				
	//			}
	//
	//			AQLDate a_fdate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm, a_c_sld, &a_c_cal, true);
	//			AQLDate a_ldate;
	//			double a_lterm = 0.0;
	//			double a_d_df = 1.0;
	//			for (unsigned int j = 0; j < a_step; ++j)
	//			{
	//				// calc cash flow
	//				AQLString a_strTerm_ = AQLString(static_cast<int>(mUnit * (i - 1) + a_mUnit * (j + 1))) + AQLString("M");
	//				a_ldate = LAMathDateCalculations::getDate(a_c_spotdate, a_strTerm_, a_c_sld, &a_c_cal, true);
	//				const double a_delta = a_c_dc.getTerm(a_fdate, a_ldate);
	//				a_lterm = dc_act.getTerm(asof, a_ldate);

	//				DoubleArray a_i_gridVec;
	//				DoubleArray a_i_termVec;
	//				if (isSameGridIndex)
	//				{
	//					a_i_gridVec.push_back(dc_act.getTerm(asof, a_fdate));
	//					a_i_gridVec.push_back(dc_act.getTerm(asof, a_ldate));
	//					a_i_termVec.push_back(a_i_dc.getTerm(a_fdate, a_ldate));
	//				}
	//				else
	//				{
	//					calcIndexGrid(asof, a_fdate, a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
	//				}
	//				// calc index rate
	//				double ret = 0.0;
	//				for (unsigned int k = 1; k < a_i_gridVec.size(); ++k)
	//				{
	//					ret += a_i_termVec[k - 1] * a_f_inter.value(a_i_gridVec[k]);
	//				}
	//				const double a_rate = (a_f_inter.value(a_i_gridVec[0]) -  a_f_inter.value(a_i_gridVec.back())) / ret;
	//				
	//				a_d_df = a_d_inter.value(a_lterm);
	//				a_sum1 += a_rate * a_delta * a_d_df;
	//				a_sum2 += a_delta * a_d_df;
	//
	//				a_fdate = a_ldate;
	//			}
	//			// add notional
	//			target_pv = a_sum1 + a_sum2 * spread + 1.0 * a_d_df;
	//			// set pv asof -> spot
	//			target_pv /= a_d_spotdf;
	//		}
	//	
	//		// newton method
	//		if (c_spotdate < a_c_spotdate)
	//		{
	//			fdate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm, c_sld, &c_cal, true);
	//		}
	//		else
	//		{
	//			fdate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true);
	//		}
	//		AQLDate ldate;		
	//		double val0 = 0.0;
	//		// for discount df
	//		if (isDiscount)
	//		{
	//			AQLString dfstrTerm = AQLString(static_cast<int>(mUnit * (i - 1) + mUnit * step)) + AQLString("M");
	//			AQLDate dfdate;
	//			if (c_spotdate < a_c_spotdate)
	//			{
	//				dfdate = LAMathDateCalculations::getDate(a_c_spotdate, dfstrTerm, c_sld, &c_cal, true);
	//			}
	//			else
	//			{
	//				dfdate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true);
	//			}
	//
	//			double dfTerm = dc_act.getTerm(c_spotdate, dfdate);
	//			double df0 = ldf;
	//			double d_df = ldf;
	//			double spread = 0.0;
	//			if (!isAgtSpread)
	//			{
	//				AQLDate dfdate_ = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true);
	//				const double s_lterm_ = c_dc.getTerm(c_spotdate, dfdate_);
	//				if (isTimeInter && s_lterm_ != 0.0)
	//				{
	//					spread = b_inter.value(s_lterm_) / s_lterm_;
	//				}
	//				else
	//				{
	//					spread = b_inter.value(s_lterm_);
	//				}
	//			}
	//
	//			DoubleVector rate_deltaVec(step, 0.0);
	//			DoubleVector deltaVec(step, 0.0);
	//			DoubleVector alphaVec(step, 0.0);
	//			double orig_target_pv = target_pv;
	//			double sum1_ = sum1;
	//			double sum2_ = sum2;
	//			int add = 0;
	//			for (unsigned int j = 0; j < step; ++j)
	//			{
	//				DoubleArray i_gridVec;
	//				DoubleArray i_termVec;
	//				if (i == 1 && c_spotdate < a_c_spotdate)
	//				{
	//					rate_deltaVec.resize(step + 1, 0.0);
	//					deltaVec.resize(step + 1, 0.0);
	//					alphaVec.resize(step + 1, 0.0);
	//					const double delta = c_dc.getTerm(c_spotdate, a_c_spotdate);
	//					const double s_lterm = dc_act.getTerm(c_spotdate, a_c_spotdate);

	//					if (isSameGridIndex)
	//					{
	//						i_gridVec.push_back(dc_act.getTerm(asof, c_spotdate));
	//						i_gridVec.push_back(dc_act.getTerm(asof, a_c_spotdate));
	//						i_termVec.push_back(i_dc.getTerm(c_spotdate, a_c_spotdate));
	//					}
	//					else
	//					{
	//						calcIndexGrid(asof, c_spotdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
	//					}
	//					// calc index rate
	//					double ret = 0.0;
	//					for (unsigned int k = 1; k < i_gridVec.size(); ++k)
	//					{
	//						ret += i_termVec[k - 1] * f_inter.value(i_gridVec[k]);
	//					}
	//					const double rate = (f_inter.value(i_gridVec[0]) -  f_inter.value(i_gridVec.back())) / ret;

	//					rate_deltaVec[0] = rate * delta;
	//					deltaVec[0] = delta;
	//					alphaVec[0] = (s_lterm - lt) / (dfTerm - lt);
	//					d_df = ldf + alphaVec[0] * (df0 - ldf);
	//					sum1_ += rate_deltaVec[0] * d_df;
	//					sum2_ += deltaVec[0] * d_df;
	//					fdate = a_c_spotdate;
	//					i_gridVec.clear();
	//					i_termVec.clear();
	//					++add;
	//				}
	//
	//				// calc cash flow
	//				AQLString strTerm_ = AQLString(static_cast<int>(mUnit * (i - 1) + mUnit * (j + 1))) + AQLString("M");
	//				if (c_spotdate < a_c_spotdate)
	//				{
	//					ldate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm_, c_sld, &c_cal, true);
	//				}
	//				else
	//				{
	//					ldate = LAMathDateCalculations::getDate(c_spotdate, strTerm_, c_sld, &c_cal, true);
	//				}
	//				//analytic for CalcDatesForDVZero
	//				dvzeroDates.push_back(ldate);

	//				if (isSameGridIndex)
	//				{
	//					i_gridVec.push_back(dc_act.getTerm(asof, fdate));
	//					i_gridVec.push_back(dc_act.getTerm(asof, ldate));
	//					i_termVec.push_back(i_dc.getTerm(fdate, ldate));
	//				}
	//				else
	//				{
	//					calcIndexGrid(asof, fdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
	//				}
	//				// calc index rate
	//				double ret = 0.0;
	//				for (unsigned int k = 1; k < i_gridVec.size(); ++k)
	//				{
	//					ret += i_termVec[k - 1] * f_inter.value(i_gridVec[k]);
	//				}
	//				const double rate = (f_inter.value(i_gridVec[0]) -  f_inter.value(i_gridVec.back())) / ret;
	//				const double delta = c_dc.getTerm(fdate, ldate);
	//				const double s_lterm = dc_act.getTerm(c_spotdate, ldate);
	//				rate_deltaVec[j + add] = rate * delta;
	//				deltaVec[j + add] = delta;
	//				alphaVec[j + add] = (s_lterm - lt) / (dfTerm - lt);
	//				d_df = ldf + alphaVec[j + add] * (df0 - ldf);
	//
	//				sum1_ += rate_deltaVec[j + add] * d_df;
	//				sum2_ += deltaVec[j + add] * d_df;
	//
	//				b_t.push_back(s_lterm);
	//				fdate = ldate;
	//			}
	//			val0 = sum1_ + sum2_ * spread + 1.0 * d_df;
	//			val0 -= target_pv;
	//
	//			double df1 = df0 + 1.0e-5;
	//			sum1_ = sum1;
	//			sum2_ = sum2;
	//			target_pv = orig_target_pv;
	//
	//			for (unsigned int j = 0; j < step; ++j)
	//			{
	//				if (i == 1 && c_spotdate < a_c_spotdate)
	//				{
	//					d_df = ldf + alphaVec[0] * (df1 - ldf);
	//					sum1_ += rate_deltaVec[0] * d_df;
	//					sum2_ += deltaVec[0] * d_df;
	//				}
	//				d_df = ldf + alphaVec[j + add] * (df1 - ldf);
	//				sum1_ += rate_deltaVec[j + add] * d_df;
	//				sum2_ += deltaVec[j + add] * d_df;
	//			}
	//			double val1 = sum1_ + sum2_ * spread + 1.0 * d_df;
	//			val1 -= target_pv;
	//
	//			unsigned int loopNum = MAX_LOOP;
	//			while (loopNum--)
	//			{
	//				if (AQLMath::abs(val0 - val1) < EPS_PV)
	//				{
	//					break;
	//				}
	//				target_pv = orig_target_pv;
	//				double dval = (val1 - val0) / (df1 - df0);
	//				double df2 = df1 - val1 / dval;
	//				df0 = df1;
	//				df1 = df2;
	//				val0 = val1;
	//				sum1_ = sum1;
	//				sum2_ = sum2;
	//				for (unsigned int j = 0; j < step; ++j)
	//				{
	//					if (i == 1 && c_spotdate < a_c_spotdate)
	//					{
	//						d_df = ldf + alphaVec[0] * (df1 - ldf);
	//						sum1_ += rate_deltaVec[0] * d_df;
	//						sum2_ += deltaVec[0] * d_df;
	//					}
	//					d_df = ldf + alphaVec[j + add] * (df1 - ldf);
	//					sum1_ += rate_deltaVec[j + add] * d_df;
	//					sum2_ += deltaVec[j + add] * d_df;
	//				}
	//				val1 = sum1_ + sum2_ * spread + 1.0 * d_df;
	//				val1 -= target_pv;
	//			}
	//			if (loopNum < 0)
	//			{
	//				throw AQLCoreInvalidData("Convergence error in Basis calc (Newton Raphson)", __FILE__, __LINE__); 
	//			}
	//			for (unsigned int j = 0; j < step; ++j)
	//			{
	//				if (i == 1 && c_spotdate < a_c_spotdate)
	//				{
	//					d_df = ldf + alphaVec[0] * (df1 - ldf);
	//					sum1 += rate_deltaVec[0] * d_df;
	//					sum2 += deltaVec[0] * d_df;
	//				}
	//				d_df = ldf + alphaVec[j + add] * (df1 - ldf);
	//				df_mod.push_back(d_df);
	//
	//				sum1 += rate_deltaVec[j + add] * d_df;
	//				sum2 += deltaVec[j + add] * d_df;
	//			}
	//			lt = b_t.back();
	//			ldf = df_mod.back();
	//		}
	//		else
	//		{
	//			AQLLinearInterpolation tmpInter;
	//			DoubleVector delta_dfVec;
	//			double d_ldf = 1.0;
	//			DoubleMatrix i_gridMtx;
	//			DoubleMatrix i_termMtx;
	//			DoubleMatrix alphaMtx;
	//
	//			AQLString dfstrTerm = AQLString(static_cast<int>(mUnit * (i - 1) + mUnit * (step - 1))) + AQLString("M");
	//			AQLDate dfdate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true);
	//
	//			DoubleVector tmp_termVec;
	//			DoubleVector tmp_gridVec;
	//			calcIndexGrid(asof, dfdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, tmp_gridVec, tmp_termVec);
	//			const double dfTerm = tmp_gridVec.back();
	//			double df0 = f_inter.value(dfTerm);
	//			DoubleVector b_t_ = b_t;
	//			DoubleVector df_mod_ = df_mod;
	//			b_t_.push_back(dfTerm);
	//			df_mod_.push_back(df0);
	//			tmpInter.set(b_t_, df_mod_);
	//
	//			double spread = 0.0;
	//			if (!isAgtSpread)
	//			{
	//				AQLString dfstrTerm_ = AQLString(static_cast<int>(mUnit * (i - 1) + mUnit * step)) + AQLString("M");
	//				AQLDate dfdate_ = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm_, c_sld, &c_cal, true);
	//				const double s_lterm_ = c_dc.getTerm(c_spotdate, dfdate_);
	//				if (isTimeInter && s_lterm_ != 0.0)
	//				{
	//					spread = b_inter.value(s_lterm_) / s_lterm_;
	//				}
	//				else
	//				{
	//					spread = b_inter.value(s_lterm_);
	//				}
	//			}
	//
	//			// calc term info
	//			for (unsigned int j = 0; j < step; ++j)
	//			{	
	//				// calc cash flow
	//				AQLString strTerm_ = AQLString(static_cast<int>(mUnit * (i - 1) + mUnit * (j + 1))) + AQLString("M");
	//				ldate = LAMathDateCalculations::getDate(c_spotdate, strTerm_, c_sld, &c_cal, true);
	//				//analytic for CalcDatesForDVZero
	//				dvzeroDates.push_back(ldate);
	//				DoubleArray i_gridVec;
	//				DoubleArray i_termVec;
	//				if (isSameGridIndex)
	//				{
	//					i_gridVec.push_back(dc_act.getTerm(asof, fdate));
	//					i_gridVec.push_back(dc_act.getTerm(asof, ldate));
	//					i_termVec.push_back(i_dc.getTerm(fdate, ldate));
	//				}
	//				else
	//				{
	//					calcIndexGrid(asof, fdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
	//				}
	//				i_gridMtx.push_back(i_gridVec);
	//				i_termMtx.push_back(i_termVec);	
	//
	//				const double delta = a_c_dc.getTerm(fdate, ldate);
	//				const double lterm = dc_act.getTerm(asof, ldate);
	//				d_ldf = d_inter.value(lterm);
	//				delta_dfVec.push_back(d_ldf * delta);
	//				p_time.push_back(lterm);
	//
	//				fdate = ldate;
	//			}
	//
	//			double sum1_ = sum1;
	//			double sum2_ = sum2;
	//			for (unsigned int j = 0; j < step; ++j)
	//			{
	//				// calc index rate
	//				double ret = 0.0;
	//				for (unsigned int k = 1; k < i_gridMtx[j].size(); ++k)
	//				{
	//					ret += i_termMtx[j][k - 1] * tmpInter.value(i_gridMtx[j][k]);
	//				}
	//				const double rate = (tmpInter.value(i_gridMtx[j][0]) - tmpInter.value(i_gridMtx[j].back())) / ret;
	//				sum1_ += rate * delta_dfVec[j];
	//				sum2_ += delta_dfVec[j];
	//			}
	//			double cnstVal = sum2_ * spread + 1.0 * d_ldf;
	//			val0 = sum1_ + cnstVal;
	//			// set pv asof -> spot
	//			val0 /= d_spotdf;
	//			val0 -= target_pv;
	//
	//			sum1_ = sum1;
	//			double df1 = df0 + 1.0e-5;
	//			for (unsigned int j = 0; j < step; ++j)
	//			{
	//				df_mod_.back() = df1;
	//				tmpInter.set(b_t_, df_mod_);
	//				// calc index rate
	//				double ret = 0.0;
	//				for (unsigned int k = 1; k < i_gridMtx[j].size(); ++k)
	//				{
	//					ret += i_termMtx[j][k - 1] * tmpInter.value(i_gridMtx[j][k]);
	//				}
	//				const double rate = (tmpInter.value(i_gridMtx[j][0]) - tmpInter.value(i_gridMtx[j].back())) / ret;				
	//				sum1_ += rate * delta_dfVec[j];
	//			}
	//			double val1 = sum1_ + cnstVal;
	//			// set pv asof -> spot
	//			val1 /= d_spotdf;
	//			val1 -= target_pv;
	//
	//			unsigned int loopNum = MAX_LOOP;
	//			while (loopNum--)
	//			{
	//				if (AQLMath::abs(val0 - val1) < EPS_PV)
	//				{
	//					break;
	//				}
	//				double dval = (val1 - val0) / (df1 - df0);
	//				double df2 = df1 - val1 / dval;
	//				df0 = df1;
	//				df1 = df2;
	//				val0 = val1;
	//
	//				sum1_ = sum1;
	//				for (unsigned int j = 0; j < step; ++j)
	//				{
	//					df_mod_.back() = df1;
	//					tmpInter.set(b_t_, df_mod_);
	//					// calc index rate
	//					double ret = 0.0;
	//					for (unsigned int k = 1; k < i_gridMtx[j].size(); ++k)
	//					{
	//						ret += i_termMtx[j][k - 1] * tmpInter.value(i_gridMtx[j][k]);
	//					}
	//					const double rate = (tmpInter.value(i_gridMtx[j][0]) - tmpInter.value(i_gridMtx[j].back())) / ret;
	//					sum1_ += rate * delta_dfVec[j];
	//				}
	//				val1 = sum1_ + cnstVal;
	//				// set pv asof -> spot
	//				val1 /= d_spotdf;
	//				val1 -= target_pv;
	//			}
	//			if (loopNum < 0)
	//			{
	//				throw AQLCoreInvalidData("Convergence error in Basis calc (Newton Raphson)", __FILE__, __LINE__); 
	//			}
	//			// set forecast df map
	//			map<double, double> f_dfmap;
	//			for (unsigned int j = 0; j < step; ++j)
	//			{
	//				double ret = 0.0;
	//				for (unsigned int k = 1; k < i_gridMtx[j].size(); ++k)
	//				{
	//					const double df = tmpInter.value(i_gridMtx[j][k]);
	//					ret += i_termMtx[j][k - 1] * df;
	//					f_dfmap[i_gridMtx[j][k]] = df;
	//				}
	//				const double rate = (tmpInter.value(i_gridMtx[j][0]) - tmpInter.value(i_gridMtx[j].back())) / ret;
	//				sum1 += rate * delta_dfVec[j];
	//			}
	//			sum2 = sum2_;
	//
	//			map<double, double>::const_iterator it = f_dfmap.begin();
	//			while (it != f_dfmap.end())
	//			{
	//				b_t.push_back(it->first);
	//				df_mod.push_back(it->second);
	//				++it;
	//			}
	//			lt = b_t.back();
	//			ldf = df_mod.back();
	//		}
	//	}
	//	DoubleArray b_t_2(1, 0.0);
	//	DoubleArray df_mod_2(1, 1.0);
	//	double f_yieldSpread = 0.0;
	//	double firstTerm = 0.0;
	//	if (isDiscount)
	//	{
	//		const double f_yield = -AQLMath::log(df_mod[1]) / b_t[1];
	//		f_yieldSpread = f_yield - (-AQLMath::log(d_inter.value(b_t[1] + spotTerm) / d_spotdf) / b_t[1]);
	//		const double d_spotdf_ = AQLMath::exp(-f_yieldSpread * spotTerm) * d_spotdf;
	//		firstTerm = spotTerm + b_t[1];
	//		if (spotTerm != 0.0)
	//		{
	//			for (unsigned int i = 0; i < b_t.size(); i++)
	//			{
	//				b_t_2.push_back(b_t[i] + spotTerm);
	//				df_mod_2.push_back(df_mod[i] * d_spotdf_);
	//			}
	//		}
	//		else
	//		{
	//			b_t_2 = b_t;
	//			df_mod_2 = df_mod;
	//		}
	//		p_time = b_t;
	//	}
	//	else
	//	{
	//		const double f_yield = -AQLMath::log(df_mod[1]) / b_t[1];
	//		f_yieldSpread = f_yield - (-AQLMath::log(f_inter.value(b_t[1])) / b_t[1]);
	//		firstTerm = b_t[1];
	//		b_t_2 = b_t;
	//		df_mod_2 = df_mod;
	//	}
	//	AQLPriceDataInterpolation *p_base_inter = 0;
	//	if (isDiscount)
	//	{
	//		p_base_inter = &d_inter;
	//	}
	//	else
	//	{
	//		p_base_inter = &f_inter;
	//	}
	//	// calc yieldspread
	//	b_inter.set(b_t_2, df_mod_2);
	//	DoubleArray yieldSpread(p_time.size());
	//	for (unsigned int i = 1; i < p_time.size(); ++i)
	//	{
	//		const double yield = -AQLMath::log(b_inter.value(p_time[i])) / p_time[i];
	//		yieldSpread[i] = yield - (-AQLMath::log(p_base_inter->value(p_time[i])) / p_time[i]);
	//	}
	//	// merge term
	//	if (dvzeroDates.empty())
	//	{
	//		throw AQLCoreInvalidData("dvzeroDates is empty", __FILE__, __LINE__);
	//	}
	//	const AQLDate &firstDate = dvzeroDates[0];
	//	AQLDate shortDate = firstDate; 
	//	shortDate.addDays(-10);// 10days before first date
	//	const double shortTerm = firstTerm + dc_act.getTerm(firstDate, shortDate);
	//	DoubleArray tmp(1, 0.0);
	//	for (unsigned int i = 1; i < _terms.size(); ++i)
	//	{
	//		if (_terms[i] >= shortTerm)
	//		{
	//			break;
	//		}
	//		tmp.push_back(_terms[i]);
	//	}
	//	unsigned int f_pos;
	//	AQLAlgorithm::locate<DoubleVector, double>(b_t_2, tmp.back(), b_t_2.size(), f_pos);
	//	const double EPS_TERM = 1.0E-4;
	//	if (b_t_2[f_pos] - tmp.back() < EPS_TERM)
	//	{
	//		f_pos++;
	//	}
	//	tmp.insert(tmp.end(), b_t_2.begin() + f_pos, b_t_2.end());
	//	_terms = tmp;
	//	// calc yield
	//	yield_mod.resize(_terms.size(), 0.0);
	//	AQLPriceDataInterpolation tmp_inter = b_inter;
	//	tmp_inter.set(p_time, yieldSpread);
	//	for (unsigned int i = 1; i < _terms.size(); i++)
	//	{
	//		if (f_yieldSpread != 0.0 && _terms[i] < firstTerm)
	//		{
	//			double df = p_base_inter->value(_terms[i]);
	//			yield_mod[i] = -AQLMath::log(df) / _terms[i];
	//			yield_mod[i] += f_yieldSpread;
	//		}
	//		else
	//		{
	//			const double df = p_base_inter->value(_terms[i]);
	//			if (df <= 0.0)
	//			{
	//				throw AQLCoreInvalidData("Df is below zero.", __FILE__, __LINE__);
	//			}
	//			const double yield =  -AQLMath::log(df) / _terms[i];
	//			const double y_spread = tmp_inter.value(_terms[i]);
	//			yield_mod[i] = yield + y_spread;
	//		}
	//	}
	//}
	//delete a_fwdInter;
	
	// delete artificial market data
	for (unsigned int i = 0; i < data_addtional.size(); ++i)
	{
		const AQLString& term_additional = dynamic_cast<AQLDataString&>(data_addtional[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
		std::vector<AQLObject*>::iterator itr = data_.begin();
		while (itr != data_.end())
		{
			const AQLString& term = dynamic_cast<AQLDataString&>((*itr)->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
			if(term == term_additional)
			{
				itr = data_.erase(itr);
			}
			else
			{
				++itr;
			}
		}
		delete data_addtional[i];
	}

	// calc modify df
	DoubleArray dfs_mod(_terms.size(), 0.0);
	for (unsigned int i = 0; i < yield_mod.size(); i++)
	{
		dfs_mod[i] = AQLMath::exp(-_terms[i] * yield_mod[i]);
	}

	DoubleArray::const_iterator min_it = min_element(_terms.begin(), _terms.end());
	if (!_terms.empty() && *min_it < 0.0)
	{
		AQLString msg = curveType + " terms, term must be positive.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	min_it = min_element(termsmtx_fwd[0].begin(), termsmtx_fwd[0].end());
	if (!termsmtx_fwd[0].empty() && *min_it < 0.0)
	{
		AQLString msg = curveType + " terms_fwd, term must be positive.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//for (unsigned int i = 0; i < data_.size(); i++)
	//{
	//	DateVector ret;

	//	const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//	if (strTerm == "ON" || strTerm == "TN") continue;
	//	AQLDate matudate;
	//	if (isDiscount || isSimuEq)
	//	{
	//		if (c_spotdate < a_c_spotdate && !isSameGridIndex)
	//		{
	//			ret.push_back(a_c_spotdate);
	//			matudate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
	//		}
	//		else
	//		{
	//			ret.push_back(c_spotdate);
	//			matudate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
	//		}
	//	}
	//	else
	//	{
	//		ret.push_back(c_spotdate);
	//		matudate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
	//	}

	//	//find dvzeroDates
	//	unsigned int pos=0;
	//	if (!AQLAlgorithm::find<DateVector,AQLDate>(dvzeroDates,matudate,0,dvzeroDates.size()-1 ,pos))
	//	{
	//		//this means basis grid e.x. 100y is longer than DF1 max grid, which ofcourse can not be calculated.
	//		if (matudate > dvzeroDates.back())
	//			continue;
	//		else
	//			throw AQLCoreInvalidData("BasisGrid Search Error",__FILE__,__LINE__);
	//	}

	//	ret.insert(ret.end(),dvzeroDates.begin(),dvzeroDates.begin()+pos+1);
	//	if(!data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
	//	{
	//		data_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(ret));
	//	}
	//	else
	//	{
	//		dynamic_cast<AQLDataDates&>(data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(ret);
	//	}
	//}

	saveBasisCurve(curveType, _terms, termsmtx_fwd, dfs_mod, data_);

}

/*!
	@brief calc fwdfx constant curve

*/
void
LAMathYieldCurvePro::calcFwdFXConstantCurve(void)
{
	AQLString curveType;
	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); ++it)
	{
		AQLString market = it->second;
		if (market.toUpper() == FWDFXCONST)
		{
			curveType = it->first;
			break;
		}
	}
	if (curveType.size() > 0)
	{
		const AQLDataHolder *dh;
		bool isCheckCurves = false;
		dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		bool isCurveAttrExist = checkCurveAttr(FWDFXCONST);
		if (!isCheckCurves || !isCurveAttrExist)
		{
			calcFwdFXConstantCurve(curveType);
		}
	}
}

/*!
	@brief calc fwdfx constant curve

*/
void
LAMathYieldCurvePro::calcFwdFXConstantCurve(const AQLString &curveType)
{
	// get yield data
	AQLObject &yData = getYieldData().get().get(); 
	AQLObject &colYData = getColYieldData().get().get();
	// get market data
	const AQLString& market = mAssignedCurveMktMap[curveType];
	AQLObjectHolder& mktData = getMarketDataRef(market).get(0);

	// get forecast curve
	const AQLString &fCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_FORECAST).get()).get();
	AQLPriceDataInterpolation f_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (f_inter.isHybrid())
	{
		double linearSplineJoinDateAsDouble = 0.;
		f_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	setCurveInterpolation(fCurve, yData, f_inter);
	// get against forecast curve
	const AQLString &a_fCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_AGTFORECAST).get()).get();
	AQLPriceDataInterpolation a_f_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (a_f_inter.isHybrid() )
	{
		double linearSplineJoinDateAsDouble = 0.;
		a_f_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	setCurveInterpolation(a_fCurve, colYData, a_f_inter);
	// get against discount curve
	const AQLString &a_dCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_AGTDISCOUNT).get()).get();
	AQLPriceDataInterpolation a_d_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (a_d_inter.isHybrid())
	{
		double linearSplineJoinDateAsDouble = 0.;
		a_d_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	setCurveInterpolation(a_dCurve, colYData, a_d_inter);

	// calc discount curve
	const AQLString &dCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_DISCOUNT).get()).get();
	const AQLString &suffix = dCurve == STD ? AQLString("") : AQLString("_") + dCurve;
	const DoubleArray &_terms = dynamic_cast<const AQLDataDoubles&> ((yData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
	DoubleVector _dfs(_terms.size());
	for (unsigned int i = 0; i < _terms.size(); ++i)
	{
		_dfs[i] = a_d_inter.value(_terms[i]) * f_inter.value(_terms[i]) / a_f_inter.value(_terms[i]);
	}

	// save curves
	AQLObjectHolder &objHolder = getYieldData().get();
	for (map<AQLString, AQLString>::const_iterator it = getAssignedCurveMktMap().begin(); it != getAssignedCurveMktMap().end(); ++it)
	{
		if (it->second == market)
		{
			const AQLString& curveName = it->first;
			const AQLString suffix = "_" + curveName;
			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(_terms));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new AQLDataDoubles(_dfs));

			//mBCurveGenMap[curveName] = true;
			mGCurveGenMap[curveName] = true;
		}
	}

	// set df2
	AQLDataHolder* dh = &getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		setDF2();
	}
}

/*!
	@brief get market data reference

	@param[in] market name
*/
const AQLDataMultiReference&
LAMathYieldCurvePro::getMarketDataRef(const AQLString& mktName) const
{
	AQLString suffix;
	AQLStringVector tmpMktNames = mktName.toToken('_');
	if (tmpMktNames.size() == 2)
	{
		const AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{
			throw AQLCoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			suffix = "_" +tmpMktNames[1];
			return dynamic_cast<const AQLDataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
		}
	}
	else
	{
		suffix = "_" +tmpMktNames[0];
		return dynamic_cast<const AQLDataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
	}
}

/*!
	@brief save basis curve

	@param[in] curveType
	@param[in] terms
	@param[in] termsmtx_fwd
	@param[in] dfs
	@param[out] data
*/
void
LAMathYieldCurvePro::saveBasisCurve(const AQLString& curveType, const DoubleArray& terms, const DoubleMatrix& termsmtx_fwd, const DoubleArray& dfs, std::vector<AQLObject*>& data)
{
	AQLObjectHolder &objHolder = getYieldData().get();
	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (it->second == mAssignedCurveMktMap[curveType])
		{
			const AQLString& curveName = it->first;
			AQLString suffix = "_" + curveName;
			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(terms));
			objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, new AQLDataDoubleMatrix(termsmtx_fwd));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new AQLDataDoubles(dfs));
			// set generate flag true
			mBCurveGenMap[curveName] = true;
			setCurveConvention(objHolder, data, curveName);
			//set STD curve
			const AQLDataHolder& dh = objHolder.getData(CALIBRATION_DATA_TERMS);
			const AQLDataHolder& ah2 = objHolder.getData(IR_CALIBRATION_DATA_ISFWDFX);
			bool isfwdfx = false;
			if(ah2.isDefined() && !ah2.isNull())
				isfwdfx = dynamic_cast<const AQLDataBool&>(ah2.get()).get();
			
			if (!dh.isDefined() || dh.isNull() || isfwdfx)
			{
				objHolder.remove(IR_CALIBRATION_DATA_ISFWDFX);
				objHolder.add(IR_CALIBRATION_DATA_ISFWDFX, new AQLDataBool(true));
				objHolder.remove(CALIBRATION_DATA_TERMS);
				objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(terms));
				objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX);
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(termsmtx_fwd));
				objHolder.remove(IR_CALIBRATION_DATA_DFS);
				objHolder.add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(dfs));
				mGCurveGenMap[STD] = true;
				setCurveConvention(objHolder, data, STD);
			}
		}
	}
}

/*!
	@brief set curve interpolation

	@param[in] curveName
	@param[in] yieldData
	@param[out] inter
*/
void
LAMathYieldCurvePro::setCurveInterpolation(const AQLString& curveName, const AQLObject &yieldData, AQLPriceDataInterpolation& inter) const
{
	const AQLString &suffix = curveName == STD ? AQLString("") : AQLString("_") + curveName;
	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
	const DoubleArray &dfs   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
	inter.set(terms, dfs);
}

/*!
    @brief calc index grid

	@param[in] asofdate	asof date
	@param[in] date		end date
	@param[in] resetLag index reset lag
	@param[in] dc		index daycount
	@param[in] fixcal	index fixing calendar
	@param[in] paycal	index payment calendar
	@param[in] sld		index sliding rule
	@param[in] freq		index frequency (almost all is simple)
	@param[in] accessary index accessary
	@param[out] gridVec index grid (from asofdate, daycount = ACT/365_ISDA)
	@param[out] termVec index span (daycount = dc)

*/
void
LAMathYieldCurvePro::calcIndexGrid(const AQLDate &asofdate, const AQLDate &date, int resetLag, const AQLPriceDataDayCount &dc, 
								 const AQLPriceDataCalendar &fixcal, const AQLPriceDataCalendar &paycal, const AQLPriceDataSlidingRule &sld, 
								 const AQLString &freq, const AQLString &accessary, DoubleVector &gridVec, DoubleVector &termVec)
{
	gridVec.clear();
	termVec.clear();

	AQLString tmpFreq = freq;
	tmpFreq.toUpper();
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(accessary, y, m, d, w);

	unsigned int size = 0;
	unsigned int addmonth = 0;
	if (d != 0)
	{
		throw AQLCoreInvalidData("d != 0 is not support", __FILE__, __LINE__);	
	}
	if(0 == y && 0 == m)
	{
		throw AQLCoreInvalidData("This Accessory input supports only y,m,d",__FILE__,__LINE__);
	}

	if (freq == SIMPLE)
	{
		size = 2;
		addmonth = y * 12 + m;
	}
	else if (freq == MONTHLY)
	{
		size = y * 12 + m + 1;
		addmonth = 1;
	}
	else if (freq == QUARTERLY)
	{
		if (m % 3 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}
		size = y * 4 + m / 3 + 1;
		addmonth = 3;

	}
	else if (freq == SEMI_ANNUAL)
	{
		if (m % 6 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}	
		size = y * 2 + m / 6 + 1;
		addmonth = 6;

	}
	else if (freq == ANNUAL)
	{
		if (m % 12 != 0)
		{
			throw AQLCoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		size = y + m / 12 + 1;
		addmonth = 12;
	}
	else
	{
		//error
		AQLString err = "Frequency: ";
		err += freq;
		err += " is not support";
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	AQLPriceDataDayCount dc_act(ACT_365_ISDA);
	gridVec.resize(size);
	termVec.resize(size - 1);
	AQLDate fixingdate = fixcal.getBusinessDay(date, -1 * resetLag);
	AQLDate paymentdate = paycal.getBusinessDay(fixingdate, resetLag);
	AQLDate noadjdate = paymentdate;
	gridVec[0] = dc_act.getTerm(asofdate, paymentdate);

	for (unsigned int i = 1; i < size; i++)
	{
		AQLDate olddate = paymentdate;
		noadjdate.addMonths(addmonth);
		paymentdate = sld.getDate(noadjdate, paycal);
		gridVec[i] = dc_act.getTerm(asofdate, paymentdate);		
		termVec[i - 1] = dc.getTerm(olddate, paymentdate, false);
	}

}

/*!
    @brief calc basis cashflow

	@param[in] inter : inter of target spread*t
	@param[in] isDiscount : for generate Discount Curve or not
	@param[in] isFwdRen : is forwardfx renotional or not (spotfx renotional)
	@param[in] f_inter : inter of target forecast DF
	@param[in] dfsVec : target DF
	@param[in] a_d_inter : inter of against discount DF
	@param[in] a_d_df_adjust : base date adjustment for against discount DF
	@param[in] adjust_inter : inter of convexity adjust
	@param[in] isAgtSpread : spread is on against leg or not
	@param[in] spreadVec : basis spread
	@param[in] cpd_times : compounding times
	@param[in] spotTerm : cashlet spot term
	@param[in] gridVec : cashlet grid
	@param[in] tauVec : cashlet tau
	@param[in] i_gridMatVec : index grid
	@param[in] i_termMatVec : index tau
	@param[in] b_yieldTimeMatVec : forecast r*t
	@param[in] a_targetPVVec : against target leg PV
	@param[out] out : output of result PV
	@param[in] isNegative : is negative or not
	@param[in] c_isBackward : backward flag of target leg
*/
void
LAMathYieldCurvePro::calcBasisCF(const AQLPriceDataInterpolation &s_inter, const bool isDiscount, const bool isFwdRen, const bool isUSD, const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, 
							   const std::vector<DoubleArray> &dfsVec, const AQLPriceDataInterpolation &a_f_inter, const AQLPriceDataInterpolation *a_fwd_inter, const AQLPriceDataInterpolation &a_d_inter, const double a_d_df_adjust, const AQLPriceDataInterpolation *adjust_inter, 
							   const bool isAgtSpread, const DoubleArray &spreadVec, const std::vector<IntArray> &cpd_timesVec, const std::vector<IntArray> &a_cpd_timesVec, const double spotTerm, const double a_spotTerm, 
							   const std::vector<DoubleArray> &gridVec, const std::vector<DoubleArray> &tauVec, const std::vector<DoubleMatrix> &i_gridMatVec, const std::vector<DoubleMatrix> &i_termMatVec, 
							   const std::vector<DoubleArray> &a_gridVec, const std::vector<DoubleArray> &a_tauVec, const std::vector<DoubleMatrix> &a_i_gridMatVec, const std::vector<DoubleMatrix> &a_i_termMatVec, 
							   const std::vector<DoubleMatrix> &b_yieldTimeMatVec, DoubleArray &a_targetPVVec, DoubleArray &out, const bool isNegative)
{
	unsigned int b_size = spreadVec.size();
	if (gridVec.size() != b_size || tauVec.size() != b_size || dfsVec.size() != b_size ||
		i_gridMatVec.size() != b_size || i_termMatVec.size() != b_size || b_yieldTimeMatVec.size() != b_size || a_targetPVVec.size() != b_size)
	{
		throw AQLCoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
	}

	for (unsigned int i = 0; i < b_size; ++i)
	{
		const unsigned int gridSize = gridVec[i].size();
		if (i_gridMatVec[i].size() != gridSize || tauVec[i].size() != gridSize || dfsVec[i].size() != gridSize || b_yieldTimeMatVec[i].size() != gridSize)
		{
			throw AQLCoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
		}
	}

	DoubleArray tmpSpreadVec(b_size, 0.0);
	if (!isAgtSpread) tmpSpreadVec = spreadVec;
	DoubleArray targetPVVec(b_size, 0.0);
	if (isDiscount) // for discount curve
	{
		if (isFwdRen) // fwd renotional
		{
			if (isUSD) // target renotional
			{
				for (unsigned int i = 0; i < b_size; ++i)
					targetPVVec[i] = calcTargetPV(s_inter, isFWDInter, f_inter, fwd_inter, a_d_inter, adjust_inter, tmpSpreadVec[i], cpd_timesVec[i], spotTerm, gridVec[i], 
											tauVec[i], i_gridMatVec[i], i_termMatVec[i], b_yieldTimeMatVec[i]);
			}
			else // against renotional
			{
				for (unsigned int i = 0; i < b_size; ++i)
					targetPVVec[i] = calcTargetPV(s_inter, isFWDInter, f_inter, fwd_inter, tmpSpreadVec[i], cpd_timesVec[i], spotTerm, gridVec[i], tauVec[i],	
											i_gridMatVec[i], i_termMatVec[i], b_yieldTimeMatVec[i]);
				a_targetPVVec.clear();
				a_targetPVVec.resize(b_size);
				DoubleArray tmpAgtSpreadVec(b_size, 0.0);
				if (isAgtSpread) tmpAgtSpreadVec = spreadVec;
				for (unsigned int i = 0; i < b_size; ++i)
					a_targetPVVec[i] = calcAgainstPV(isFWDInter, a_f_inter, a_fwd_inter, a_d_inter, s_inter, adjust_inter, tmpAgtSpreadVec[i], a_cpd_timesVec[i], 
											a_spotTerm, spotTerm, a_gridVec[i], a_tauVec[i], a_i_gridMatVec[i], a_i_termMatVec[i]) * a_d_df_adjust;
			}
		}
		else // spot renotional
		{
			for (unsigned int i = 0; i < b_size; ++i)
				targetPVVec[i] = calcTargetPV(s_inter, isFWDInter, f_inter, fwd_inter, tmpSpreadVec[i], cpd_timesVec[i], spotTerm, gridVec[i], tauVec[i],	
										i_gridMatVec[i], i_termMatVec[i], b_yieldTimeMatVec[i]);
		}
	}
	else // for forecast curve
	{
		for (unsigned int i = 0; i < b_size; ++i)
			targetPVVec[i] = calcTargetPV(s_inter, dfsVec[i], tmpSpreadVec[i], cpd_timesVec[i], spotTerm, gridVec[i], 
									tauVec[i], i_gridMatVec[i], i_termMatVec[i], b_yieldTimeMatVec[i]);
	}

	out.clear();
	out.resize(b_size);
	const double sign = isNegative ? -1.0 : 1.0;
	for (unsigned int i = 0; i < b_size; ++i)
		out[i] = sign * (a_targetPVVec[i] - targetPVVec[i]);
}

/*!
	@brief calc against PV (spot renotional)
    
	@param[in] isFWDInter : use inter of forward rate or not
	@param[in] f_inter : inter of forecast DF
	@param[in] fwd_inter : inter of forcast forward rate
	@param[in] d_inter : inter of discount DF
	@param[in] spread : basis spread
	@param[in] cpd_times : compounding times
	@param[in] term_spot : term of spot date
	@param[in] terms_grid : cashlet grid
	@param[in] terms_interval : cashlet tau
	@param[in] i_gridMat : index grid
	@param[in] i_termMat : index tau
	@param[in] isBackward : backward flag

	@return against PV
*/
double		
LAMathYieldCurvePro::calcAgainstPV(const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, const AQLPriceDataInterpolation &d_inter, 
 const double spread, const IntArray& cpd_times, const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const bool isBackward)
{
	AQLPriceDataInterpolation dummy_interpolation;
	return calcAgainstPV(isFWDInter, f_inter, fwd_inter, d_inter, dummy_interpolation, NULL, spread, cpd_times, term_spot, term_spot, terms_grid, terms_interval, i_gridMat, i_termMat);
}

/*!
	@brief calc against PV (forward renotional)
	    
	@param[in] isFWDInter : use inter of forward rate or not
	@param[in] f_inter : inter of forecast DF
	@param[in] fwd_inter : inter of forcast forward rate
	@param[in] d_inter : inter of discount DF
	@param[in] a_s_inter : inter of against spread*t
	@param[in] adjust_inter : inter of convexity adjust
	@param[in] spread : basis spread
	@param[in] cpd_times : compounding times
	@param[in] term_spot : term of spot date
	@param[in] a_term_spot : term of against spot date
	@param[in] terms_grid : cashlet grid
	@param[in] terms_interval : cashlet tau
	@param[in] i_gridMat : index grid
	@param[in] i_termMat : index tau

	@return against PV
*/
double		
LAMathYieldCurvePro::calcAgainstPV(const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, const AQLPriceDataInterpolation &d_inter, const AQLPriceDataInterpolation &a_s_inter, const AQLPriceDataInterpolation *adjust_inter, 
 const double spread, const IntArray& cpd_times, const double term_spot, const double a_term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat)
{
	if (terms_grid.empty())
	{
		return 0.;
	}

	DoubleArray fwdVec(i_gridMat.size());
	for (unsigned int j = 0; j < i_gridMat.size(); ++j)
	{
		if (isFWDInter)
		{
			fwdVec[j] = fwd_inter->value(i_gridMat[j].front());
		}
		else
		{
			double ant = 0.0;
			for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
			{
				ant += i_termMat[j][k - 1] * f_inter.value(i_gridMat[j][k]);
			}
			fwdVec[j] = (f_inter.value(i_gridMat[j].front()) - f_inter.value(i_gridMat[j].back())) / ant;
		}
	}

	DoubleMatrix dfsMat(terms_grid.size());
	const double spot_df = d_inter.value(term_spot);
	dfsMat[0] = { 1.0, d_inter.value(terms_grid.front() + term_spot) / spot_df };
	for (unsigned int j = 1; j < terms_grid.size(); ++j)
	{
		dfsMat[j] = { dfsMat[j - 1].back(), d_inter.value(terms_grid[j] + term_spot) / spot_df };
	}

	DoubleArray a_dfsVec(0);
	if (!a_s_inter.isNull())
	{
		const double adj_term_spot = term_spot - a_term_spot;
		a_dfsVec.resize(terms_grid.size());
		a_dfsVec[0] = 1.;
		for (unsigned int j = 1; j < terms_grid.size(); ++j)
		{
			a_dfsVec[j] = AQLMath::exp(-a_s_inter.value(terms_grid[j - 1] + adj_term_spot));
		}
	}

	return calcLegPV(fwdVec, dfsMat, a_dfsVec, adjust_inter, spread, cpd_times, terms_grid, terms_interval, i_gridMat);
}

/*
	@brief calc target PV for Newton-Raphson method to generate Discount Curve (spot renotional)
    
	@param[in] s_inter : inter of target spread*t
	@param[in] f_inter : inter of forecast DF
	@param[in] spread : basis spread
	@param[in] cpd_times : compounding times
	@param[in] terms_grid : cashlet grid	
	@param[in] terms_interval : cashlet tau	
	@param[in] i_gridMat : index grid	
	@param[in] i_termMat : index tau	
	@param[in] b_yieldTimeMat : forecast r*t
	
	@return target PV	
*/	
double
LAMathYieldCurvePro::calcTargetPV(const AQLPriceDataInterpolation &s_inter, const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, 
	const double spread, const IntArray& cpd_times, double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
	const DoubleMatrix &b_yieldTimeMat)
{
	AQLPriceDataInterpolation dummy_interpolation;
	return calcTargetPV(s_inter, isFWDInter, f_inter, fwd_inter, dummy_interpolation, NULL, spread, cpd_times, term_spot, terms_grid, terms_interval, i_gridMat, i_termMat, b_yieldTimeMat);
}

/*
	@brief calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)
    
	@param[in] s_inter : inter of target spread*t
	@param[in] f_inter : inter of forecast DF
	@param[in] a_d_inter : inter of against discount DF
	@param[in] adjust_inter : inter of convexity adjust
	@param[in] spread : basis spread
	@param[in] cpd_times : compounding times
	@param[in] term_spot : cashlet spot term
	@param[in] terms_grid : cashlet grid	
	@param[in] terms_interval : cashlet tau	
	@param[in] term_spot : cashlet spot term		
	@param[in] i_gridMat : index grid	
	@param[in] i_termMat : index tau	
	@param[in] b_yieldTimeMat : base curve r*t	
	
	@return target PV	
*/	
double
LAMathYieldCurvePro::calcTargetPV(const AQLPriceDataInterpolation &s_inter, const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, 
	const AQLPriceDataInterpolation &a_d_inter, const AQLPriceDataInterpolation *adjust_inter, const double spread, const IntArray& cpd_times, const double term_spot, 
	const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const DoubleMatrix &b_yieldTimeMat) 
{
	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw AQLCoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
		}
	}

	if (terms_grid.empty())
	{
		return 0.;
	}

	DoubleArray fwdsVec(i_gridMat.size());
	for (unsigned int j = 0; j < i_gridMat.size(); ++j)
	{
		if (isFWDInter)
		{
			fwdsVec[j] = fwd_inter->value(i_gridMat[j].front());
		}
		else
		{
			double ant = 0.0;
			for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
			{
				ant += i_termMat[j][k - 1] * f_inter.value(i_gridMat[j][k]);
			}
			fwdsVec[j] = (f_inter.value(i_gridMat[j].front()) - f_inter.value(i_gridMat[j].back())) / ant;
		}
	}


	DoubleMatrix dfsMat(terms_grid.size(), DoubleArray(2));
	DoubleArray yieldTimes;
	DoubleArray b_yieldTimes;
	yieldTimes = { 1.0, s_inter.value(terms_grid.front()) };
	b_yieldTimes = { b_yieldTimeMat.front().front(), b_yieldTimeMat.front().back() };
	transform(yieldTimes.cbegin(), yieldTimes.cend(), b_yieldTimes.cbegin(), dfsMat.front().begin(),
		[](const double& yieldTime, const double& b_yieldTime) { return AQLMath::exp(-(yieldTime + b_yieldTime)); });
	for (unsigned int j = 1; j < terms_grid.size(); ++j)
	{
		yieldTimes = { yieldTimes.back(), s_inter.value(terms_grid[j]) };	// yieldTimes.back() must be s_inter.value(terms_grid[j - 1]) on right hand side
		b_yieldTimes = { b_yieldTimeMat[j].front(), b_yieldTimeMat[j].back() };
		transform(yieldTimes.cbegin(), yieldTimes.cend(), b_yieldTimes.cbegin(), dfsMat[j].begin(),
			[](const double& yieldTime, const double& b_yieldTime) { return AQLMath::exp(-(yieldTime + b_yieldTime)); });
	}



	DoubleArray a_dfsVec(0);
	if (!a_d_inter.isNull())
	{
		a_dfsVec.resize(terms_grid.size());
		const double a_spot_df = a_d_inter.value(term_spot);
		a_dfsVec.front() = 1.;
		for (unsigned int j = 1; j < terms_grid.size(); ++j)
		{
			a_dfsVec[j] = a_d_inter.value(terms_grid[j - 1] + term_spot) / a_spot_df;
		}
	}

	return calcLegPV(fwdsVec, dfsMat, a_dfsVec, adjust_inter, spread, cpd_times, terms_grid, terms_interval, i_gridMat);
}

/*!
	@brief calc target PV for Newton-Raphson method to generate Forecast Curve

	@param[in] s_inter : inter of target spread*t
	@param[in] dfsVec : cashlet DF
	@param[in] spread : basis spread
	@param[in] cpd_times : compounding times
	@param[in] term_spot : term of spot date
	@param[in] terms_grid : cashlet grid
	@param[in] terms_interval : cashlet tau
	@param[in] i_gridMat : index grid
	@param[in] i_termMat : index tau
	@param[in] b_yieldTimeMat : forecast r*t	
	@param[in] isBackward : backward flag

	@return target PV
*/
double	
LAMathYieldCurvePro::calcTargetPV(const AQLPriceDataInterpolation &s_inter, const DoubleArray &dfsVec, const double spread, const IntArray& cpd_times,
	 const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
	 const DoubleMatrix &b_yieldTimeMat)
{
	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw AQLCoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
		}
	}

	if (terms_grid.empty())
	{
		return 0.;
	}

	DoubleArray fwdsVec(i_gridMat.size());
	for (unsigned int j = 0; j < i_gridMat.size(); ++j)
	{
		double df = 1.0;
		double ant = 0.0;
		for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
		{
			const double term = i_gridMat[j][k] - term_spot;
			const double y_spread_time = s_inter.value(term);
			const double yield_time = y_spread_time + b_yieldTimeMat[j][k];
			df = AQLMath::exp(-yield_time);
			ant += i_termMat[j][k - 1] * df;
		}

		const double term1 = i_gridMat[j][0] - term_spot;
		const double y_spread_time1 = s_inter.value(term1);
		const double yield_time1 = y_spread_time1 + b_yieldTimeMat[j][0];
		const double df1 = AQLMath::exp(-yield_time1);
		fwdsVec[j] = (df1 - df) / ant;
	}

	DoubleMatrix dfsMat(terms_grid.size());
	dfsMat.front() = { 1., dfsVec.front() };
	for (unsigned int j = 1; j < terms_grid.size(); ++j)
	{
		dfsMat[j] = { dfsVec[j - 1], dfsVec[j] };
	}

	DoubleArray a_dfVec(0);

	return calcLegPV(fwdsVec, dfsMat, a_dfVec, NULL, spread, cpd_times, terms_grid, terms_interval, i_gridMat);
}


/*
@brief calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)

	@param[in] fwdsVec :  forward rates at accrual starting (fixing) time for each reset.
	@param[in] dfsMat : discount factors as of SPOT DATE at accrual starting (fixing) and ending (payement) time for each reset.
	@param[in] a_dfsMat : discount factors as of SPOT DATE for against leg at accrual starting (fixing) time for each reset.
	@param[in] adjust_inter : interpolation objects for convexity adjustments on resetting FX. 
	@param[in] spread : basis spread
	@param[in] cpd_times : compounding times
	@param[in] terms_grid : cashlet grid
	@param[in] terms_interval : cashlet tau
	@param[in] i_gridMat : index grid

@return target PV
*/
double
LAMathYieldCurvePro::calcLegPV(const DoubleArray &fwdsVec, const DoubleMatrix &dfsMat, const DoubleArray &a_dfsVec, const AQLPriceDataInterpolation *adjust_inter,
	const double spread, const IntArray& cpd_times, const DoubleArray &terms_grid, const DoubleArray& terms_interval, const DoubleMatrix &i_gridMat)
{
	double ret = 0.0;
	if (dfsMat.empty())
	{
		return ret;
	}

	const bool is_notional_reset = !a_dfsVec.empty();

	// terms_grid: compound grid, which length is same as number of fixing rates
	// cpd_times: number of fixing for each payments, which length is same as number of fixing rates
	// we have to check if terms_grid.size() != accumulate(cpd_times.cbegin(), cpd_times.cend()) before entering this function.
	unsigned int i = 0; // index number for retvieval of infomration 
	for (unsigned int l = 0; l < cpd_times.size(); ++l)
	{
		// set to container
		DoubleArray x(3 * cpd_times[l]);
		for (unsigned int j = 0; j < cpd_times[l]; ++j)
		{
			x[j] = fwdsVec[i];
			x[cpd_times[l] + j] = terms_interval[i];
			x[2 * cpd_times[l] + j] = spread;
			++i;
		}
		const double cpd = getCompoundingFunction().operator()(x);

		if (is_notional_reset)
		{
			const double df_0 = dfsMat[i - cpd_times[l]].front();
			const double df_0_agt = a_dfsVec[i - cpd_times[l]];
			const double fwdfx = df_0_agt / df_0; // ForwardFX(domestic/foreign) = DF_foregin / DF_domestic

			double adjust_value = 1.0;
			if (adjust_inter) adjust_value *= adjust_inter->value(i_gridMat[i - cpd_times[l]].front());

			ret += ((1.0 + cpd) * dfsMat[i - 1].back() - df_0) * fwdfx * adjust_value;
		}
		else
		{
			ret += cpd * dfsMat[i - 1].back();
		}
	}

	if (is_notional_reset)
	{
		return ret;
	}
	else
	{
		return ret + 1.0 * dfsMat.back().back() - 1.0;
	}

}


/*!
    @brief calc basis rates and add DF2 data as modified DF

*/
void
LAMathYieldCurvePro::setBasisRates(void)
{
	mBCurveGenMap.clear();
	AQLDataHolder *dh = 0;
	// calc basis dfs
	dh = &getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLStringVector &basisDFs = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
		AQLStringVector::const_iterator it = basisDFs.begin();
		while (it != basisDFs.end())
		{
			// data check
			bool isCheckCurves = false;
			dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
			if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			bool isCurveAttrExist = checkCurveAttr(getMarketForCurve(*it));
			if (!isCheckCurves || !isCurveAttrExist)
			{
				dh = &getData(IR_CALIBRATION_DATA_BASISTARGETDF, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const AQLString &targetDF = dynamic_cast<const AQLDataString &>(dh->get()).get();
					if (targetDF == *it) setBasisRates(*it);
					else mBCurveGenMap[*it] = true;
				}
				else
				{
					setBasisRates(*it);
				}
			}
			else
			{
				mBCurveGenMap[*it] = true;
			}
			++it;
		}
	}
	
	dh = &getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		setDF2();
	}
	else
	{
		setBasisRates_old();
	}
}

/*!
    @brief a function for setting DF2

*/
void 
LAMathYieldCurvePro::setDF2(void)
{
	const AQLString &mainCName = dynamic_cast<const AQLDataString &>(getData(IR_CALIBRATION_DATA_MAINBASISDF, ISNOTNULL).get()).get();
	if (mGCurveGenMap.find(mainCName) == mGCurveGenMap.end() && 
		mBCurveGenMap.find(mainCName) == mBCurveGenMap.end() && 
		mArbFreeCurveGenMap.find(mainCName) == mArbFreeCurveGenMap.end()) return;

	AQLObjectHolder &objHolder = getYieldData().get();
	DoubleArray dfs_mod;
	DoubleArray yield_mod;

	//const AQLString &mainCName = dynamic_cast<const AQLDataString &>(getData(IR_CALIBRATION_DATA_MAINBASISDF, ISNOTNULL).get()).get();
		
	AQLString suffix = "";
	if (mainCName != STD)
	{
		suffix = "_" + mainCName;
	}
	//const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
	DoubleArray terms_mod = dynamic_cast<const AQLDataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get();
	DoubleArray dfs_mod_ = dynamic_cast<const AQLDataDoubles &>(objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).get();
	DoubleArray terms = terms_mod;
	AQLDataHolder* dh;
	dh = &objHolder.getData(CALIBRATION_DATA_TERMS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		terms = dynamic_cast<AQLDataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS, NOCHECK).get()).get();
	// adjust terms
	AQLPriceDataInterpolation inter = getInterpolation();
	dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		inter.setJoinDateAsDouble(dynamic_cast<AQLDataDouble&>(dh->get()).get());
	}
	else if (inter.isHybrid())
	{
		double linearSplineJoinDateAsDouble = 0.;
		inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	inter.set(terms_mod, dfs_mod_);
	dfs_mod.resize(terms.size());
	for (unsigned int i = 0; i < terms.size(); ++i)
	{
		dfs_mod[i] = inter.value(terms[i]);
	}
	// set DF2, grid is same with attr CALIBRATION_DATA_TERMS
	objHolder.remove(IR_CALIBRATION_DATA_DFS2);
	objHolder.add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(dfs_mod));
}

/*!
    @brief calc basis rates and add DF2 data as modified DF

*/
void
LAMathYieldCurvePro::setBasisRates_old(void)
{
	vector<const AQLObject *> data_basis;
	// get market data
	const AQLDataMultiReference &mr = getMarketData();
	for (unsigned int i = 0; i < mr.getSize(); i++)
	{
        AQLString term = dynamic_cast<const AQLDataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		term.toUpper();
		if (term == BASIS) 
			data_basis.push_back(&mr.get(i).get());
	}
	// data exist check
	if (data_basis.empty())
	{
		return;
		//throw AQLCoreInvalidData("basis data is not set.", __FILE__, __LINE__);
	}
	// sort
	sort(data_basis.begin(), data_basis.end(), Comp_term());

	// get spotdate
	const AQLDate &spotdate = dynamic_cast<const AQLDataDate &> ((data_basis[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
	// asof and spot check
	if (asof > spotdate)
	{
		throw AQLCoreInvalidData("asofdate > spotdate, cannnot calc basis DF", __FILE__, __LINE__);
	}

	// get first element val
	const AQLPriceDataDayCount &dcbs = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNTBASE, ISNOTNULL)).get());
	const AQLPriceDataDayCount &dc = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const AQLPriceDataCalendar &calbs = dynamic_cast<const AQLPriceDataCalendar &>((data_basis[0]->getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule &sldbs  = dynamic_cast<const AQLPriceDataSlidingRule &>((data_basis[0]->getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
	//iseomroll
	bool eombs = false;
	AQLDataHolder dh = data_basis[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
	if (dh.isDefined() && !dh.isNull())
	{
		eombs = dynamic_cast<const AQLDataBool &>(dh.get()).get();
	}
	AQLString freqbs = dynamic_cast<const AQLDataString &>((data_basis[0]->getData(IR_CALIBRATION_DATA_FREQUENCYBASE, ISNOTNULL)).get()).get();
	freqbs.toUpper();
	// set roll convention
	AQLString roll_conv_bs("");
	if (freqbs == LUNAR) roll_conv_bs = ROLLCONV_LUNAR;
	else if (eombs) roll_conv_bs = ROLLCONV_EOM;
	else roll_conv_bs = ROLLCONV_NORMAL;
	
	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	for (unsigned int i = 0; i < data_basis.size(); i++)
	{
		const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		double rate = dynamic_cast<const AQLDataDouble &>((data_basis[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLDate tmpDate = LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
		double term = dcbs.getTerm(spotdate, tmpDate);

		b_t_grid.push_back(term);
		b_termstruct_grid.push_back(rate);	
	}
	// get max term
	const AQLString &termMax = dynamic_cast<const AQLDataString &>((data_basis.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	// get interpolation method
	AQLPriceDataInterpolation &inter = getInterpolation_bs();
	inter.set(b_t_grid, b_termstruct_grid);

	// calc term (apply to month)
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	unsigned int mUnit = 0;
	if (freqbs == ANNUAL)			  mUnit = 12;
	else if (freqbs == SEMI_ANNUAL) mUnit = 6;
	else if (freqbs == QUARTERLY)	  mUnit = 3;
	else if (freqbs == MONTHLY || freqbs == LUNAR)	  mUnit = 1;
	else 
	{
		AQLString msg = "frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		AQLString msg = "frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject &yieldData = objHolder.get(); 
	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms.back();

	DoubleArray b_t(1, 0.0);
	DoubleArray df_mod(1, 1.0);
	double sum = 0.0;
	double sum1 = 0.0;
	double sum2 = 0.0;
	double b_delta = 0.0;
	double b_libor = 0.0;
	AQLDate fdate = spotdate;
	const unsigned int max = m / mUnit + 1;
	for (unsigned int i = 1; i < max; i++)
	{
		AQLString strTerm = AQLString(static_cast<int>(mUnit * i));
		strTerm += "M";

		AQLDate ldate =  LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
		//term is term under basis convention.
		const double term = dcbs.getTerm(spotdate, ldate);
		//termaa is term act/act.
		const double termaa = dc.getTerm(spotdate, ldate);
		// check max
		if (tmax <= termaa)
		{
			break;
		}
		// basis is interpolated basisrate.
		const double basis = inter.value(term);

		//making sum of (L + s)*term*DF which tenor is j*i
		AQLString strTerm_ = AQLString(static_cast<int>(mUnit * i));
		strTerm_ += "M";
		ldate = LAMathDateCalculations::getDate(spotdate, strTerm_, sldbs, &calbs, true, &roll_conv_bs);
		const double delta = dcbs.getTerm(fdate, ldate);
		const double dff = getDF(asof, fdate);
		const double dfl = getDF(asof, ldate);
		const double libor = 1.0 / delta * (dff / dfl - 1.0);
		if (i != 1)
		{
			double temp_val = b_delta * df_mod[i - 1];
			sum2 += temp_val;
			sum1 += b_libor * temp_val;
			sum = sum1 + sum2 * basis;
		}
		b_libor = libor;
		b_delta = delta;
		fdate = ldate;
		//DFs included basisrates.
		const double df = (1.0 - sum) / (1.0 + (libor + basis) * delta);
		if (df <= 0.0)
		{
			throw AQLCoreInvalidData("Df is below zero.", __FILE__, __LINE__);
		}
		df_mod.push_back(df);
		b_t.push_back(termaa);
	}
	//change DFs from T+2 to T+0. 
	//DFs from T+2 to  T+0 is non modified.
	const double asofterm = dc.getTerm(asof, spotdate);
	const double asofdf = getDF(asof, spotdate);
	if (asofdf <= 0.0)
	{
		throw AQLCoreInvalidData("Df from asofdate to spotdate is below zero.", __FILE__, __LINE__);
	}

	DoubleArray b_t_2(1, 0.0);
	DoubleArray df_mod_2(1, 1.0);
	if (asofterm != 0.0)
	{
		for (unsigned int i = 0; i < b_t.size(); i++)
		{
			b_t_2.push_back(b_t[i] + asofterm);
			df_mod_2.push_back(df_mod[i] * asofdf);
		}
	}
	else
	{
		// no need to adjust
		b_t_2 = b_t;
		df_mod_2 = df_mod;
	}
	
	inter.set(b_t_2, df_mod_2);
	DoubleArray b_mod(terms.size(), 0.0);
	for (unsigned int i = 1; i < terms.size(); i++)
	{
		const double df = inter.value(terms[i]);
		if (df <= 0.0)
		{
			throw AQLCoreInvalidData("Df is below zero.", __FILE__, __LINE__);
		}
		b_mod[i] = -AQLMath::log(df) / terms[i];
	}
	
	DoubleMatrix b1(1, b_mod);
	//set zerobasisrates 
	setBasisRates(b1);
	
	//add mod_DFs
	const DoubleMatrix &d_matrix = getBasisRates().get();
	DoubleArray dfs_mod;
	for (unsigned int i = 0; i < d_matrix.size(); i++)
	{
		for (unsigned int j = 0; j < d_matrix[i].size(); j++)
		{
			const double term = terms[j];
			const double bRate = d_matrix[i][j];
			dfs_mod.push_back(AQLMath::exp(-term * bRate));
		}
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS2);
	objHolder.add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(dfs_mod));
}

/*!
    @brief calc basis rates and set new curve 

*/
void
LAMathYieldCurvePro::setBasisRates2(const AQLString& basisCurveID)
{
	vector<const AQLObject *> data_basis;
	// get market data
	const AQLDataMultiReference &mr = getMarketData();
	for (unsigned int i = 0; i < mr.getSize(); i++)
	{
        AQLString term = dynamic_cast<const AQLDataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		term.toUpper();
		if (term == BASIS) 
			data_basis.push_back(&mr.get(i).get());
	}
	// data exist check
	if (data_basis.empty())
	{
		throw AQLCoreInvalidData("basis data is not set.", __FILE__, __LINE__);
	}
	// sort
	sort(data_basis.begin(), data_basis.end(), Comp_term());

	// get spotdate
	const AQLDate &spotdate = dynamic_cast<const AQLDataDate &> ((data_basis[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
	// asof and spot check
	if (asof > spotdate)
	{
		throw AQLCoreInvalidData("asofdate > spotdate, cannnot calc basis DF", __FILE__, __LINE__);
	}

	// get first element val
	const AQLPriceDataDayCount &dcbs = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNTBASE, ISNOTNULL)).get());
	const AQLPriceDataDayCount &dc = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const AQLPriceDataCalendar &calbs = dynamic_cast<const AQLPriceDataCalendar &>((data_basis[0]->getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule &sldbs  = dynamic_cast<const AQLPriceDataSlidingRule &>((data_basis[0]->getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
	//iseomroll
	bool eombs = false;
	AQLDataHolder dh = data_basis[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
	if (dh.isDefined() && !dh.isNull())
	{
		eombs = dynamic_cast<const AQLDataBool &>(dh.get()).get();
	}
	AQLString freqbs = dynamic_cast<const AQLDataString &>((data_basis[0]->getData(IR_CALIBRATION_DATA_FREQUENCYBASE, ISNOTNULL)).get()).get();
	freqbs.toUpper();
	// set roll convention
	AQLString roll_conv_bs("");
	if (freqbs == LUNAR) roll_conv_bs = ROLLCONV_LUNAR;
	else if (eombs) roll_conv_bs = ROLLCONV_EOM;
	else roll_conv_bs = ROLLCONV_NORMAL;

	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	for (unsigned int i = 0; i < data_basis.size(); i++)
	{
		const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data_basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		double rate = dynamic_cast<const AQLDataDouble &>((data_basis[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLDate tmpDate = LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
		double term = dcbs.getTerm(spotdate, tmpDate);

		b_t_grid.push_back(term);
		b_termstruct_grid.push_back(rate);	
	}
	// get max term
	const AQLString &termMax = dynamic_cast<const AQLDataString &>((data_basis.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	// get interpolation method
	AQLPriceDataInterpolation &inter = getInterpolation_bs();
	inter.set(b_t_grid, b_termstruct_grid);

	// calc term (apply to month)
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	unsigned int mUnit = 0;
	if (freqbs == ANNUAL)			  mUnit = 12;
	else if (freqbs == SEMI_ANNUAL) mUnit = 6;
	else if (freqbs == QUARTERLY)	  mUnit = 3;
	else if (freqbs == MONTHLY || freqbs == LUNAR)	  mUnit = 1;
	else 
	{
		AQLString msg = "frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		AQLString msg = "frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject &yieldData = objHolder.get(); 
	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms.back();

	DoubleArray b_t(1, 0.0);
	DoubleArray df_mod(1, 1.0);
	AQLDate fdate = spotdate;
	const unsigned int max = m / mUnit + 1;
	for (unsigned int i = 1; i < max; i++)
	{
		AQLString strTerm = AQLString(static_cast<int>(mUnit * i));
		strTerm += "M";

		AQLDate ldate = LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
		//term is term under basis convention.
		const double term = dcbs.getTerm(spotdate, ldate);
		//termaa is term act/act.
		const double termaa = dc.getTerm(spotdate, ldate);
		// basis is interpolated basisrate.
		const double basis = inter.value(term);

		double delta = 0.0;
		double temp_val = 0.0;
		double sum = 0.0;
		//making sum of (L + s)*term*DF which tenor is j*i 
		for (unsigned int j = 1; j < i + 1; j++)
		{
			// calc ldate which is never same with fdate
			AQLString strTerm_ = AQLString(static_cast<int>(mUnit * j));
			strTerm_ += "M";
			ldate = LAMathDateCalculations::getDate(spotdate, strTerm_, sldbs, &calbs, true, &roll_conv_bs);
			
			delta = dcbs.getTerm(fdate, ldate);
			const double dff = getDF(asof, fdate);
			const double dfl = getDF(asof, ldate);
			temp_val = 1.0 / delta * (dff / dfl - 1.0);
			if (j < i)
			{
				sum += (temp_val + basis) * delta * df_mod[j]; 
			}
			fdate = ldate;
		}

		//DFs included basisrates.
		const double df = (1.0 - sum) / (1.0 + (temp_val + basis) * delta);
		if (df <= 0.0)
		{
			throw AQLCoreInvalidData("Df is below zero.", __FILE__, __LINE__);
		}
		df_mod.push_back(df);
		b_t.push_back(termaa);

		// check max
		if (tmax <= termaa)
		{
			break;
		}
		fdate = spotdate;
	}
	//change DFs from T+2 to T+0. 
	//DFs from T+2 to  T+0 is non modified.
	const double asofterm = dc.getTerm(asof, spotdate);
	const double asofdf = getDF(asof, spotdate);
	if (asofdf <= 0.0)
	{
		throw AQLCoreInvalidData("Df from asofdate to spotdate is below zero.", __FILE__, __LINE__);
	}

	DoubleArray b_t_2(1, 0.0);
	DoubleArray df_mod_2(1, 1.0);
	if (asofterm != 0.0)
	{
		for (unsigned int i = 0; i < b_t.size(); i++)
		{
			b_t_2.push_back(b_t[i] + asofterm);
			df_mod_2.push_back(df_mod[i] * asofdf);
		}
	}
	else
	{
		// no need to adjust
		b_t_2 = b_t;
		df_mod_2 = df_mod;
	}
	
	inter.set(b_t_2, df_mod_2);
	DoubleArray b_mod(terms.size(), 0.0);
	for (unsigned int i = 1; i < terms.size(); i++)
	{
		const double df = inter.value(terms[i]);
		if (df <= 0.0)
		{
			throw AQLCoreInvalidData("Df is below zero.", __FILE__, __LINE__);
		}
		b_mod[i] = -AQLMath::log(df) / terms[i];
	}
	
	DoubleMatrix b1(1, b_mod);
	//set zerobasisrates 
	setBasisRates(b1);
	
	//add mod_DFs
	const DoubleMatrix &d_matrix = getBasisRates().get();
	DoubleArray dfs_mod;
	for (unsigned int i = 0; i < d_matrix.size(); i++)
	{
		for (unsigned int j = 0; j < d_matrix[i].size(); j++)
		{
			const double term = terms[j];
			const double bRate = d_matrix[i][j];
			dfs_mod.push_back(AQLMath::exp(-term * bRate));
		}
	}
    
    AQLDataInstance* dataInstance = getDataInstance();
    AQLObjectPool &objPool = dataInstance->getObjectPool();
	objHolder = objPool.getObject(basisCurveID,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add("Name", new AQLDataString()).convertFromString(basisCurveID);
		e->add("AsOfDate", new AQLDataDate(asof));
		e->add("Terms",	new AQLDataDoubles(terms));
		e->add("DiscountFactors",new AQLDataDoubles(dfs_mod));
		objPool.set(basisCurveID,e);
	}
	else if(objHolder.isDefined())
	{
		AQLDataHolder* dh;
		dh = &objHolder.getData("AsOfDate",ISDEFINED);
		AQLDataDate& date = dynamic_cast<AQLDataDate &>(dh->get());
		date.set(asof);

		dh = &objHolder.getData("Terms",ISDEFINED);
		AQLDataDoubles& terms_att = dynamic_cast<AQLDataDoubles& >(dh->get());
		terms_att.set(terms);
		dh = &objHolder.getData("DiscountFactors",ISDEFINED);
		AQLDataDoubles& dfs = dynamic_cast<AQLDataDoubles& >(dh->get());
		dfs.set(dfs_mod);
	}
}

/*!
    @brief add basis rates

*/
void
LAMathYieldCurvePro::addBasisRates(void)
{	
	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject& YieldData = objHolder.get();
	const DoubleArray& dfs   = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS + mCurveSuffix, ISNOTNULL)).get()).get();
	DoubleArray dfs_mod(dfs.size());
	
	const DoubleMatrix& d_matrix = getBasisRates().get();
	
	for(unsigned int i = 0; i < d_matrix.size(); i++)
	{
		for(unsigned int j = 0; j < d_matrix[i].size(); j++)
			dfs_mod[j] = AQLMath::exp(-d_matrix[i][j]/* * terms[j]*/) * dfs[j];
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new AQLDataDoubles(dfs_mod));
}

/*!
    @brief set arb free flag and curve name
*/
void
LAMathYieldCurvePro::setArbFreeCurveName(const AQLStringVector& curveNames_6ML, 
									   const AQLStringVector& curveNames_DF, 
									   const AQLStringVector& curveNames_3ML)
{
	for (size_t i=0; i<curveNames_6ML.size(); i++)
	{
		if(curveNames_6ML[i] != STD)
		{
			mArbFreeCurveGenMap[curveNames_6ML[i]] = true;
			mAssignedCurveMktMap[curveNames_6ML[i]] = AF6ML;
		}
	}
	for (size_t i=0; i<curveNames_DF.size(); i++)
	{
		if(curveNames_DF[i] != STD)
		{
			mArbFreeCurveGenMap[curveNames_DF[i]] = true;
			mAssignedCurveMktMap[curveNames_DF[i]] = AFDF;
		}
	}
	for (size_t i=0; i<curveNames_3ML.size(); i++)
	{
		if(curveNames_3ML[i] != STD)
		{
			mArbFreeCurveGenMap[curveNames_3ML[i]] = true;
			mAssignedCurveMktMap[curveNames_3ML[i]] = AF3ML;
		}
	}
}

/*!
    @brief default constructor for calibration cost function inner class
*/
LAMathYieldCurvePro::CalibrationCostFunction::CalibrationCostFunction(LAMathYieldCurvePro& outerObject,
                                                                    const DoubleArray grid_spread_time,
                                                                    AQLPriceDataInterpolation& spread_time_inter,
	                                                                const DoubleVector targetPV,
																	unsigned int maxIterationNum
	                                                               )
: mParent(outerObject), 
  mGrid_spread_time(grid_spread_time),
  mSpread_time_inter(spread_time_inter),
  mTargetPV(targetPV),
  mMaxIterationNum(maxIterationNum)
{
}

/*!
    @brief copy constructor for calibration cost function inner class
*/
LAMathYieldCurvePro::CalibrationCostFunction::CalibrationCostFunction(const CalibrationCostFunction& rhs)
: mParent(rhs.mParent), 
  mGrid_spread_time(rhs.mGrid_spread_time),
  mSpread_time_inter(rhs.mSpread_time_inter),
  mTargetPV(rhs.mTargetPV),
  mMaxIterationNum(rhs.mMaxIterationNum)
{
}

/*!
    @brief destructor for calibration cost function inner class
*/
LAMathYieldCurvePro::CalibrationCostFunction::~CalibrationCostFunction()
{
}

/*!
    @brief constraint for calibration cost function inner class
*/
bool
LAMathYieldCurvePro::CalibrationCostFunction::constraintsAreViolated(const DoubleArray& x)
{
	//no constraint
	return false;
}


/*!
    @brief default constructor for calibration cost function inner class
*/
LAMathYieldCurvePro::CalibrationCostFunctionBasis::CalibrationCostFunctionBasis(LAMathYieldCurvePro& outerObject,
                                                                              const DoubleArray grid_spread_time,
                                                                              AQLPriceDataInterpolation &s_inter,
                                                                              const bool isDiscount, 
                                                                              const bool isFwdRen, 
                                                                              const bool isUSD, 
                                                                              const bool isFWDInter, 
                                                                              const AQLPriceDataInterpolation &f_inter, 
                                                                              const AQLPriceDataInterpolation *fwd_inter, 
                                                                              const std::vector<DoubleArray> &dfsVec, 
	                                                                          const AQLPriceDataInterpolation &a_f_inter, 
                                                                              const AQLPriceDataInterpolation *a_fwd_inter, 
                                                                              const AQLPriceDataInterpolation &a_d_inter, 
                                                                              const double a_d_df_adjust, 
                                                                              const AQLPriceDataInterpolation *adjust_inter, 
	                                                                          const bool isAgtSpread, 
                                                                              const DoubleArray &spreadVec, 
                                                                              const std::vector<IntArray> &cpd_times, 
                                                                              const std::vector<IntArray> &a_cpd_times, 
                                                                              const double spotTerm, 
                                                                              const double a_spotTerm, 
	                                                                          const std::vector<DoubleArray> &gridVec, 
                                                                              const std::vector<DoubleArray> &tauVec, 
                                                                              const std::vector<DoubleMatrix> &i_gridMatVec, 
                                                                              const std::vector<DoubleMatrix> &i_termMatVec, 
	                                                                          const std::vector<DoubleArray> &a_gridVec, 
                                                                              const std::vector<DoubleArray> &a_tauVec, 
                                                                              const std::vector<DoubleMatrix> &a_i_gridMatVec, 
                                                                              const std::vector<DoubleMatrix> &a_i_termMatVec, 
	                                                                          const std::vector<DoubleMatrix> &b_yieldTimeMatVec, 
                                                                              DoubleArray &a_targetPVVec, 
                                                                              DoubleArray &targetPV, 
                                                                              const bool isNegative,
																			  unsigned int maxIterationNum
									                                         )
: CalibrationCostFunction(outerObject, grid_spread_time, s_inter, targetPV, maxIterationNum), 
  mIsDiscount(isDiscount),
  mIsFwdRen(isFwdRen),
  mIsUSD(isUSD),
  mIsFWDInter(isFWDInter),
  mF_inter(f_inter),
  mpFwd_inter(fwd_inter),
  mDfsVec(dfsVec),
  mA_f_inter(a_f_inter),
  mpA_fwd_inter(a_fwd_inter),
  mA_d_inter(a_d_inter),
  mA_d_df_adjust(a_d_df_adjust),
  mpAdjust_inter(adjust_inter),
  mIsAgtSpread(isAgtSpread),
  mSpreadVec(spreadVec),
  mCpd_times(cpd_times),
  mA_cpd_times(a_cpd_times),
  mSpotTerm(spotTerm),
  mA_spotTerm(a_spotTerm),
  mGridVec(gridVec),
  mTauVec(tauVec),
  mI_gridMatVec(i_gridMatVec),
  mI_termMatVec(i_termMatVec),
  mA_gridVec(a_gridVec),
  mA_tauVec(a_tauVec),
  mA_i_gridMatVec(a_i_gridMatVec),
  mA_i_termMatVec(a_i_termMatVec),
  mB_yieldTimeMatVec(b_yieldTimeMatVec),
  mA_targetPVVec(a_targetPVVec),
  mIsNegative(isNegative)
{
}

/*!
    @brief copy constructor for calibration cost function inner class
*/
LAMathYieldCurvePro::CalibrationCostFunctionBasis::CalibrationCostFunctionBasis(const CalibrationCostFunctionBasis& rhs)
: CalibrationCostFunction(rhs), 
  mIsDiscount(rhs.mIsDiscount),
  mIsFwdRen(rhs.mIsFwdRen),
  mIsUSD(rhs.mIsUSD),
  mIsFWDInter(rhs.mIsFWDInter),
  mF_inter(rhs.mF_inter),
  mpFwd_inter(rhs.mpFwd_inter),
  mDfsVec(rhs.mDfsVec),
  mA_f_inter(rhs.mA_f_inter),
  mpA_fwd_inter(rhs.mpA_fwd_inter),
  mA_d_inter(rhs.mA_d_inter),
  mA_d_df_adjust(rhs.mA_d_df_adjust),
  mpAdjust_inter(rhs.mpAdjust_inter),
  mIsAgtSpread(rhs.mIsAgtSpread),
  mSpreadVec(rhs.mSpreadVec),
  mCpd_times(rhs.mCpd_times),
  mA_cpd_times(rhs.mA_cpd_times),
  mSpotTerm(rhs.mSpotTerm),
  mA_spotTerm(rhs.mA_spotTerm),
  mGridVec(rhs.mGridVec),
  mTauVec(rhs.mTauVec),
  mI_gridMatVec(rhs.mI_gridMatVec),
  mI_termMatVec(rhs.mI_termMatVec),
  mA_gridVec(rhs.mA_gridVec),
  mA_tauVec(rhs.mA_tauVec),
  mA_i_gridMatVec(rhs.mA_i_gridMatVec),
  mA_i_termMatVec(rhs.mA_i_termMatVec),
  mB_yieldTimeMatVec(rhs.mB_yieldTimeMatVec),
  mA_targetPVVec(rhs.mA_targetPVVec),
  mIsNegative(rhs.mIsNegative)
{
}

/*!
    @brief destructor for calibration cost function inner class
*/
LAMathYieldCurvePro::CalibrationCostFunctionBasis::~CalibrationCostFunctionBasis()
{
}



/*!
    @brief constraint for calibration cost function inner class
*/

void
LAMathYieldCurvePro::CalibrationCostFunctionBasis::operator()(DoubleArray& f, const DoubleArray& x)
{
	DoubleArray x_;
	x_.push_back(0.0);
	x_.insert(x_.end(), x.begin(), x.end());
	mSpread_time_inter.set(mGrid_spread_time, x_);
	DoubleArray ret;
	ret.resize(mTargetPV.size());
	mParent.calcBasisCF(mSpread_time_inter, 
                        mIsDiscount, 
				        mIsFwdRen, 
				        mIsUSD, 
				        mIsFWDInter, 
				        mF_inter, 
				        mpFwd_inter, 
				        mDfsVec, 
                        mA_f_inter, 
				        mpA_fwd_inter, 
				        mA_d_inter, 
				        mA_d_df_adjust, 
				        mpAdjust_inter, 
				        mIsAgtSpread, 
				        mSpreadVec, 
				        mCpd_times, 
				        mA_cpd_times, 
				        mSpotTerm, 
				        mA_spotTerm, 
				        mGridVec, 
				        mTauVec, 
				        mI_gridMatVec, 
				        mI_termMatVec, 
				        mA_gridVec, 
				        mA_tauVec, 
				        mA_i_gridMatVec, 
				        mA_i_termMatVec, 
				        mB_yieldTimeMatVec, 
				        mA_targetPVVec, 
				        ret, 
				        mIsNegative);
	f = ret;
}


/*!
    @brief constructor
*/
LAMathBasisFunction::LAMathBasisFunction()
{
}

/*!
    @brief destructor	
*/
LAMathBasisFunction::~LAMathBasisFunction()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAMathBasisFunction::getType() const
{
	return FN_BASISFUNC1;
}

/*!
    @brief clone this class

	@return	pointer of this class
*/
AQLCoreFunctionBase*
LAMathBasisFunction::clone() const
{
    try 
	{
		return new LAMathBasisFunction(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathBasisFunction::operator()(const DoubleArray& x) const
{
	if(x.size() == 1)
		return operator()(x[0]);
	throw AQLCoreInvalidData("parameter size must be one", __FILE__, __LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathBasisFunction::operator()(const double& x) const
{
	double ret = 0.0;
	double dfn = mParam[mParam.size()-6];
	double vminusv = mParam[mParam.size()-4];
	double sum = 0.0;

	for (unsigned int i = 0; i < mParam.size() -6; i += 2)
	{
		sum += mParam[i + 1] * x;
		ret += mParam[i] * AQLMath::exp(sum);
	}
	ret += dfn * AQLMath::exp(sum) + vminusv ;
	return AQLMath::sqr(ret);
}

/*!
    @brief constructor
*/
LAMathBasisFunction2::LAMathBasisFunction2()
{
}

/*!
    @brief destructor	
*/
LAMathBasisFunction2::~LAMathBasisFunction2()
{
}

/*!
    @brief clone this class

	@return	pointer of this class
*/
AQLCoreFunctionBase*
LAMathBasisFunction2::clone() const
{
    try 
	{
		return new LAMathBasisFunction2(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathBasisFunction2::operator()(const DoubleArray& x) const
{
	if(x.size() == 1)
		return operator()(x[0]);
	throw AQLCoreInvalidData("parameter size must be one", __FILE__, __LINE__);
}
/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathBasisFunction2::operator()(const double& x) const
{

	double ret = 0.0;
	double t = 0.0;
	double dfn     = mParam[mParam.size() - 6];
	double vminusv = mParam[mParam.size() - 4];
	double b       = mParam[mParam.size() - 2];
	double sum = 0.0;

	for (unsigned int i = 0; i < mParam.size() -6; i += 2)
	{
		t += - mParam[i + 1];
		sum += mParam[i + 1] * (t * x + b);
		ret += mParam[i] * AQLMath::exp(sum);
	}

	ret += dfn*AQLMath::exp(sum) + vminusv ;
	return AQLMath::sqr(ret);
}

void
LAMathYieldCurvePro::setdNPVdm(const AQLString &curveType)
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	//check whethor Matrix has been made or not
	AQLDataHolder *dh = 0;

	dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix));
	if (dh->isDefined() && !dh->isNull())
		return;

	// get market data
	AQLString mktName;
	if (curveType == STD)
	{
		mktName = SWAP;
	}
	else
	{
		mktName = mAssignedCurveMktMap[curveType];
	}
	AQLString suffix_mkt = "";
	if (mktName != SWAP)
		suffix_mkt = "_" + mktName;
	const AQLDataMultiReference* mr_;
	AQLStringVector tmpMktNames = mktName.toToken('_');
	if (tmpMktNames.size() == 2)
	{
		AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{	
			throw AQLCoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			mr_ = &dynamic_cast<const AQLDataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1], ISNOTNULL).get());
		}
	}
	else
	{
		mr_ = &dynamic_cast<const AQLDataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());
	}
	vector<AQLObject*> dataall;
	IntArray omitvec;
	for(unsigned int i = 0; i < mr_->getSize(); i++)
	{
       const AQLString &dataType = dynamic_cast<const AQLDataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();

	   //grid long case whether we can omit or not
	   dh = &(mr_->get(i).getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK));
	   bool isexist = (dh->isDefined() && !dh->isNull());
	   if (!isexist)
	   {
		   omitvec.push_back(i);
		   continue;
	   }

		AQLString tmpType = dataType;
		tmpType.toUpper();
		if (tmpType == BASIS) 
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if(tmpType == PAR)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if(tmpType == ZERO)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == O_N || tmpType == T_N)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if(tmpType == FUTURE)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == BOJ || tmpType == FEDFUNDRATE)
		{
			dataall.push_back(&mr_->get(i).get());	
		}
		else if(tmpType == FRA6M || tmpType == FRA3M)
		{
			dataall.push_back(&mr_->get(i).get());
		}
	}

	AQLStringVector typevec(dataall.size());
	std::vector<DateVector> dateMat(dataall.size());
	DoubleVector gridtermvec(dataall.size());
	AQLStringVector gridstrvec(dataall.size());
	DoubleVector mratevec(dataall.size());

	DoubleVector dNPVdm(dataall.size());
	DoubleMatrix dNPVdzero(dataall.size());
	double dnpvdm = 0.0;
	const AQLDate& asOfDate = getAsOfDate().get();
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA); 
	
	const AQLInterpolationBase& pInter = getDFInterpolation(&curveType);
	dh = &getData(IR_CALIBRATION_DATA_DFCURVENAME + suffix, NOCHECK);
	AQLString dfcurveType = curveType;
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString keyname = dynamic_cast<AQLDataString &>(dh->get());
		if (keyname == ITSELF)
			dfcurveType = curveType;
		else
			dfcurveType = keyname;
	}
	const AQLInterpolationBase& pInterSTD = getDFInterpolation(&dfcurveType);


	//note that cashflow is always payers
	unsigned int sizeAll = dataall.size();
	bool isdiscount,isagtspd;
	for (unsigned int i = 0; i < sizeAll; i++)
	{
		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL));
		typevec[i] = dynamic_cast<const AQLDataString &>(dh->get());

		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, ISNOTNULL));
		dateMat[i] = dynamic_cast<const AQLDataDates &>(dh->get()).get();
		dNPVdzero[i].resize(dateMat[i].size());

		//termvec
		gridtermvec[i] = dc_act365.getTerm(asOfDate, dateMat[i].back());
		//termstrgrid
		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_TERM, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL));
			gridstrvec[i]= dynamic_cast<const AQLDataString &>(dh->get());
		}
		else
			gridstrvec[i]= dynamic_cast<const AQLDataString &>(dh->get());
		

		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL));
		}
		const AQLPriceDataDayCount& dcCalc = dynamic_cast<const AQLPriceDataDayCount &>(dh->get());

		//Libor, O_N or T_N case
		if (typevec[i] == ZERO || typevec[i] == O_N || typevec[i] == T_N || typevec[i] == BOJ || typevec[i] == FEDFUNDRATE)
		{
			if (dateMat[i].size() != 2)
				throw AQLCoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			double mrate = dynamic_cast<const AQLDataDouble &>(dh->get());

			double calcterm = dcCalc.getTerm(dateMat[i][0],dateMat[i][1]);
			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dfterm2 = dc_act365.getTerm(asOfDate, dateMat[i][1]);

			double df1 = pInter.value(dfterm1);
			double df2 = pInter.value(dfterm2);
			
			dNPVdm[i] = calcterm * df2;
			dNPVdzero[i][0] = -dfterm1 * df1;
			dNPVdzero[i][1] = dfterm2 * (1.0 + calcterm * mrate) * df2;
			mratevec[i] = mrate;
		}//swap case 
		else if(typevec[i] == PAR)
		{
			if (dateMat[i].size() < 2)
				throw AQLCoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dftermLast = dc_act365.getTerm(asOfDate, dateMat[i].back());

			double df1 = pInter.value(dfterm1);
			double dfLast =  pInter.value(dftermLast);

			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			double mrate = dynamic_cast<const AQLDataDouble &>(dh->get());
			if (AQLMath::abs(mrate) < EPS)
			{
				//throw AQLCoreInvalidData("Market rate 0", __FILE__,__LINE__);
				mrate = EPS;
				dNPVdm[i] = EPS;
			}
			else
				dNPVdm[i] = - (dfLast - df1) / mrate;
			dNPVdzero[i][0] = -dfterm1 * df1;

			double dfterm = 0.0;
			double calcterm = 0.0;
			double dfj =0.0;
			for (unsigned int j =1; j < dNPVdzero[i].size(); j++)
			{
				dfterm = dc_act365.getTerm(asOfDate, dateMat[i][j]);
				calcterm = dcCalc.getTerm(dateMat[i][j-1],dateMat[i][j]);
				dfj = pInter.value(dfterm);
				dNPVdzero[i][j] = dfterm * calcterm * mrate * dfj;
			}
			dNPVdzero[i].back() +=  dfterm * dfj;
			mratevec[i] = mrate;
		
		}//basis case
		else if (typevec[i] == BASIS)
		{
			// in case of basis there are 3cases, discountcase, forecastAndagainstspreadcase,
			//and forecastAndnotspreadcase
			if (dateMat[i].size() < 2)
				throw AQLCoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			if (0==i)
			{
				dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL));
				isdiscount = dynamic_cast<AQLDataBool &>(dh->get());

				dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL));
				isagtspd = dynamic_cast<AQLDataBool &>(dh->get());
			}

			if (isdiscount)//XCCYBasisCase
			{
				double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
				double basisdf1 = pInter.value(dfterm1);
				double dfterm = 0.0;
				double calcterm = 0.0;
				double dfj =0.0;
				double dfj_1 = pInterSTD.value(dfterm1);
				double baisdfj=0.0;
				double spreadj = 0.0;

				dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
				double brate = dynamic_cast<const AQLDataDouble &>(dh->get());
				dNPVdzero[i][0] = -dfterm1 * basisdf1;
				for (unsigned int j =1; j < dNPVdzero[i].size(); j++)
				{
					dfterm = dc_act365.getTerm(asOfDate, dateMat[i][j]);
					calcterm = dcCalc.getTerm(dateMat[i][j-1],dateMat[i][j]);
					dfj = pInterSTD.value(dfterm);
					baisdfj = pInter.value(dfterm);
					spreadj = baisdfj / dfj;
					
					dNPVdzero[i][j] = dfterm * spreadj * (dfj_1 + (brate * calcterm - 1.0) * dfj);
					dNPVdm[i] += calcterm * baisdfj;

					dfj_1 = dfj;
				}
				dNPVdzero[i].back() +=  dfterm * pInter.value(dfterm);
				mratevec[i] = brate;
			}
			else //3M-6MBasis
			{
				double sgn = (isagtspd) ? 1.0 : -1.0;
				
				double dfterm = dc_act365.getTerm(asOfDate, dateMat[i][0]);
				double calcterm = dcCalc.getTerm(dateMat[i][0],dateMat[i][1]);
				double baisdfj=pInter.value(dfterm);
				double divspreadj = pInterSTD.value(dfterm) / baisdfj;

				double bfbasisdfj =0.0;
				double bfdivspreadj = 0.0;

				double afdfterm = dc_act365.getTerm(asOfDate, dateMat[i][1]);
				double afdivspreadj = pInterSTD.value(afdfterm) / pInter.value(afdfterm);
				dNPVdzero[i][0] = dfterm * baisdfj * divspreadj;
				for (unsigned int j =1; j < dNPVdzero[i].size() - 1; j++)
				{
					double bfbasisdfj = baisdfj;
					double bfdivspreadj = divspreadj;
					
					dfterm = dc_act365.getTerm(asOfDate, dateMat[i][j]);
					calcterm = dcCalc.getTerm(dateMat[i][j-1],dateMat[i][j]);
					baisdfj=pInter.value(dfterm);
					divspreadj = pInterSTD.value(dfterm) / baisdfj;

					afdfterm = dc_act365.getTerm(asOfDate, dateMat[i][j+1]);
					afdivspreadj = pInterSTD.value(afdfterm) / pInter.value(afdfterm); 
					dNPVdzero[i][j] = dfterm * bfbasisdfj * divspreadj - dfterm * baisdfj * afdivspreadj;
					dNPVdzero[i][j] *= sgn;
					
					dNPVdm[i] += calcterm * baisdfj;

				}
				dNPVdzero[i].back() +=  sgn * afdfterm * baisdfj * afdivspreadj;
				dNPVdm[i] += calcterm * baisdfj;
				dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
				double brate = dynamic_cast<const AQLDataDouble &>(dh->get());
				mratevec[i] = brate;
			}
		}//future case
		else if (typevec[i] == FUTURE)
		{
			if (dateMat[i].size() != 2)
				throw AQLCoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			double mrate = 0.0;
			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				mrate = dynamic_cast<const AQLDataDouble &>(dh->get());
			else
			{
				dh = &(dataall[i]->getData(PRICING_DATA_PRICE, ISNOTNULL));
				mrate = 1.0- 0.01 *  dynamic_cast<const AQLDataDouble &>(dh->get());
			}

			double calcterm = dcCalc.getTerm(dateMat[i][0],dateMat[i][1]);
			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dfterm2 = dc_act365.getTerm(asOfDate, dateMat[i][1]);
			double df1 = pInter.value(dfterm1);
			double df2 = pInter.value(dfterm2);
			
			dNPVdm[i] = calcterm * df2;
			dNPVdzero[i][0] = -dfterm1 * df1;
			dNPVdzero[i][1] = dfterm2 * (1.0 + calcterm * mrate) * df2;
			mratevec[i] = mrate;
		}
		else if (typevec[i] == BASIS)
		{
			throw AQLCoreInvalidData("Not Support Now",__FILE__,__LINE__);
		}
		else if (typevec[i] == FRA3M || typevec[i] == FRA6M)
		{
			if (dateMat[i].size() != 2)
				throw AQLCoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			double mrate = 0.0;
			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			mrate = dynamic_cast<const AQLDataDouble &>(dh->get());

			double calcterm = dcCalc.getTerm(dateMat[i][0],dateMat[i][1]);
			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dfterm2 = dc_act365.getTerm(asOfDate, dateMat[i][1]);
			double df1 = pInter.value(dfterm1);
			double df2 = pInter.value(dfterm2);
			
			dNPVdm[i] = calcterm * df2;
			dNPVdzero[i][0] = -dfterm1 * df1;
			dNPVdzero[i][1] = dfterm2 * (1.0 + calcterm * mrate) * df2;			
			mratevec[i] = mrate;
		}	
	}

	//deconmpose into matrix
	DateVector gridvec(sizeAll);
	for (unsigned int i = 0; i < sizeAll; i++)
		gridvec[i] = dateMat[i].back();

	unsigned int gridpos = 0;
	//decomposematrix
	DoubleMatrix decompMat(sizeAll, DoubleVector(sizeAll,0.0));
	for (unsigned int i = 0; i < sizeAll; i++)
	{
		//decompose dNPVdzero
		for (unsigned int j =0; j < dateMat[i].size(); j++)
		{
			AQLDate targetdate = dateMat[i][j];
			AQLAlgorithm::locate<DateVector, AQLDate>(gridvec,targetdate,gridvec.size(),gridpos);

			if (gridpos == 0)
			{
				decompMat[i][0] += dNPVdzero[i][j];
			}
			else if (gridpos == gridvec.size())
			{
				decompMat[i].back() += dNPVdzero[i][j];
			}
			else
			{
				double diffD = static_cast<double>(gridvec[gridpos-1].intervalDays(gridvec[gridpos]));
				if (diffD == 0.0)
					throw AQLCoreInvalidData("DateMatrix Error",__FILE__,__LINE__);

				double ratio1 = targetdate.intervalDays(gridvec[gridpos]) / diffD;

				decompMat[i][gridpos-1] += dNPVdzero[i][j] * ratio1;
				decompMat[i][gridpos] += dNPVdzero[i][j] * (1.0 - ratio1);
			}
		}
	}

	//dMdZ
	for (unsigned int i= 0; i < sizeAll; i++)
	{
		if (dNPVdm[i] == 0.0)
			throw AQLCoreInvalidData("dNPVdm 0 Error",__FILE__,__LINE__);
		DoubleVector div_vec(sizeAll, dNPVdm[i]);
		transform(decompMat[i].begin(), decompMat[i].end(), div_vec.begin(), decompMat[i].begin(), divides<double>());
	}

	//inverse
	AQLMatrix matobj(decompMat);
	const AQLMatrix& invmat = matobj.inverseMatrix();
	const AQLMatrix& transmat = invmat.transpose();

	DoubleMatrix ret(sizeAll, DoubleVector(sizeAll,0.0));
	for (unsigned int i = 0; i < sizeAll; i++)
		for (unsigned int j = 0; j < sizeAll; j++)
			ret[i][j] = transmat.getValue(i,j);

	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix, new AQLDataDoubleMatrix(ret));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix, new AQLDataDoubles(gridtermvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix, new AQLDataInts(omitvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES + suffix, new AQLDataStrings(gridstrvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS + suffix, new AQLDataDoubles(mratevec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES + suffix, new AQLDataStrings(typevec));

	return;
}


const AQLDataDoubleMatrix&
LAMathYieldCurvePro::getConversionMatrix(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataDoubleMatrix &>(dh->get());
}

const AQLDataDoubles&	
LAMathYieldCurvePro::getConversionMatrixTerm(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataDoubles &>(dh->get());
}

const AQLDataDoubles&	
LAMathYieldCurvePro::getConversionMarketRates(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataDoubles &>(dh->get());
}


const AQLDataInts&
LAMathYieldCurvePro::getConversionOmitGrids(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataInts &>(dh->get());
}

const AQLDataStrings&
LAMathYieldCurvePro::getConversionMatrixTermTypes(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataStrings &>(dh->get());
}

const AQLDataStrings&
LAMathYieldCurvePro::getConversionMarketTypes(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataStrings &>(dh->get());
}


bool 
LAMathYieldCurvePro::isOmitGridsExist(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix));
	return (dh->isDefined() && !dh->isNull());

}
void
LAMathYieldCurvePro::setCurveDependencyMap(void)
{
	//bcurve = STD + alpha then map[bcurve][Base][+alpha]
	std::map<AQLString, bool>::iterator itbgen = mBCurveGenMap.begin();
	for (itbgen = mBCurveGenMap.begin(); itbgen != mBCurveGenMap.end(); ++itbgen)
	{
		AQLString curveType = itbgen->first;

		AQLString suffix = "";
		if (curveType != STD)
			suffix = "_" + curveType;
		//	// get market data
		AQLString suffix_mkt = "";
		if (curveType != STD)
			suffix_mkt = "_" + mAssignedCurveMktMap[curveType];
		const AQLDataMultiReference* mr_;
		AQLStringVector tmpMktNames = mAssignedCurveMktMap[curveType].toToken('_');
		if (tmpMktNames.size() == 2)
		{
			AQLObjectHolder &eh_fy = getForeignYieldData().get();
			if(!eh_fy.isDefined())
			{	
				throw AQLCoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
			}
			else
			{
				mr_ = &dynamic_cast<const AQLDataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + tmpMktNames[1], ISNOTNULL).get());
			}
		}
		else
		{
			mr_ = &dynamic_cast<const AQLDataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());
		}
		//find basis object
		unsigned int bpos =  mr_->getSize();
		for (unsigned int i = 0; i < mr_->getSize(); i++)
		{
			const AQLString &dataType = dynamic_cast<const AQLDataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			if (dataType == BASIS)
			{	
				bpos = i;
				break;
			}
		}
		if (bpos == mr_->getSize())
			throw AQLCoreInvalidData("Not found DataType = BASIS",__FILE__,__LINE__);

		AQLObject& ebasis = mr_->get(bpos).get();
		bool isdiscount = dynamic_cast<const AQLDataBool &>(ebasis.getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
		bool isagtspd =  dynamic_cast<const AQLDataBool &>(ebasis.getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();
		double spdval = 0.0;
		AQLString targetCurve;
		//forecasttype
		if (!isdiscount)
		{ 
			//now under construnction
			spdval = 1.0;
			targetCurve = dynamic_cast<const AQLDataString &>(ebasis.getData(IR_CALIBRATION_DATA_AGTFORECAST , ISNOTNULL).get()).get();
		}
		else //discount
		{
			//now under construnction
			spdval = 1.0;
			if (isagtspd)
				targetCurve = dynamic_cast<const AQLDataString &>(ebasis.getData(IR_CALIBRATION_DATA_AGTFORECAST , ISNOTNULL).get()).get();
			else
				targetCurve = dynamic_cast<const AQLDataString &>(ebasis.getData(IR_CALIBRATION_DATA_FORECAST , ISNOTNULL).get()).get();
		}

		if (mBCurveGenMap.find(targetCurve) == mBCurveGenMap.end())
		{
			//std, 6mlibor each shift case
			mDpnCurveMap[curveType][targetCurve] = spdval;
		}
		else
		{
			mDpnCurveMap[curveType][targetCurve] = spdval;
		}
	}
	return;

}

std::map<AQLString, double>&
LAMathYieldCurvePro::getCurveDependeny(const AQLString &curveType) const 
{
	AQLString tmp(curveType);
	if (mBfCurveType == tmp)
		return mBfDpnMap;

	//if xccybasiscurve = STD + (xccySpred) + (- 3M/6MSpread) 
	//then return map, [xccybasiscurve][1], [3M-6Mbasiscurve][-1]
	mBfDpnMap.clear();
	mBfCurveType = AQLString(tmp);

	std::map<AQLString, bool>::const_iterator itbgen = mBCurveGenMap.begin();
	std::map<AQLString, std::map<AQLString, double> >::const_iterator itdpn = mDpnCurveMap.begin();
	std::map<AQLString, bool>::const_iterator itggen = mGCurveGenMap.begin();

	//std, 6mlibor both shift case
	/*for(itggen = mGCurveGenMap.begin(); itggen != mGCurveGenMap.end(); ++itggen)
			mBfDpnMap.insert(std::make_pair(itggen->first,1.0));
		if (mGCurveGenMap.find(curveType) != mGCurveGenMap.end())
		{
			return mBfDpnMap;
		}*/
	//std, 6mlibor both shift case
	
	//std, 6mlibor each shift case
	if (mGCurveGenMap.find(curveType) != mGCurveGenMap.end())
	{
		mBfDpnMap.insert(std::make_pair(curveType,1.0));
		return mBfDpnMap;
	}
	//std, 6mlibor each shift case

	if (mBCurveGenMap.find(curveType) == mBCurveGenMap.end() || 
				mDpnCurveMap.find(curveType) == mDpnCurveMap.end())
		throw AQLCoreInvalidData("Curve Generate Map Error",__FILE__,__LINE__);

	AQLString nameTmp = curveType;
	bool iscomplete = false;
	while (!iscomplete)
	{
		
		itdpn = mDpnCurveMap.find(nameTmp);
		if (itdpn->second.size() != 1)
			throw AQLCoreInvalidData("Not Support Now",__FILE__,__LINE__);
		
		std::map<AQLString, double>::const_iterator itTmp = itdpn->second.begin();
		if (itTmp->first == nameTmp)
			throw AQLCoreInvalidData("Loop Error",__FILE__,__LINE__);
	
		//std, 6mlibor both shift case
		//if (itTmp->first == "GenCurve")
		//{
		//	mBfDpnMap.insert(std::make_pair(nameTmp,itTmp->second));
		//	iscomplete = true;
		//}
		//std, 6mlibor both shift case
		
		//std, 6mlibor each shift case
		if (mGCurveGenMap.find(itTmp->first) != mGCurveGenMap.end())
		{
			mBfDpnMap.insert(std::make_pair(nameTmp,itTmp->second));
			//we register gen curve here
			mBfDpnMap.insert(std::make_pair(itTmp->first,itTmp->second));
			iscomplete = true;
		}
		//std, 6mlibor each shift case
		else
		{
			mBfDpnMap.insert(std::make_pair(nameTmp,itTmp->second));
			nameTmp = itTmp->first;
		}
	}
	
	return mBfDpnMap;
}

void
LAMathYieldCurvePro::changeZeroRiskIntoMarketRisk(AQLString curveType, const DoubleVector& termZeroVals, const DoubleVector& riskZeroVals,
												AQLStringVector& termMarketGrids, DoubleVector& riskMarketVals) const
{

	termMarketGrids = getConversionMatrixTermTypes(curveType).get();
	const DoubleMatrix& mat = getConversionMatrix(curveType).get();
	const DoubleVector& termvec = getConversionMatrixTerm(curveType).get();
	DoubleVector dNPVdzero(termvec.size(),0.0);
	DoubleVector dNPVdm(termvec.size(),0.0);

	riskMarketVals.resize(termvec.size());

	unsigned int gridpos =0;
	//calc dNPVdm and dNPVdm if exist
	for (unsigned int i = 0; i < termZeroVals.size(); i++)
	{
		double term = termZeroVals[i];
		double riskval = riskZeroVals[i];
		AQLAlgorithm::locate<DoubleVector, double>(termvec,term,termvec.size(),gridpos);
		if (gridpos == 0)
		{
			dNPVdzero[0] += riskval;
		}
		else if (gridpos == termvec.size())
		{
			dNPVdzero.back() += riskval;
		}
		else
		{
			double diffD = termvec[gridpos] - termvec[gridpos-1];
			if (diffD == 0.0)
				throw AQLCoreInvalidData("TermVec Error",__FILE__,__LINE__);
			
			double ratio1 = (termvec[gridpos]- term) / diffD;
			dNPVdzero[gridpos-1] +=riskval * ratio1;
			dNPVdzero[gridpos] += riskval * (1.0 - ratio1);
		}
	}
	
	AQLMatrix convertMat(mat);
	AQLMatrix dNPVdzeroMat(dNPVdzero);
	const AQLMatrix& multimat = convertMat * dNPVdzeroMat;
	for (unsigned int l = 0; l < riskMarketVals.size(); l++)
		riskMarketVals[l] = multimat.getValue(l,0);

	/*if(isOmitGridsExist(curveType))
	{
		const IntVector& omitposvec= getConversionOmitGrids(curveType).get();
		unsigned int count =0;
		for (unsigned int i = 0; i < omitposvec.size(); i++, count++)
		{
			unsigned int pos = omitposvec[i];
			dNPVdzero.insert(dNPVdzero.begin()+pos+count,0.0);
			riskMarketVals.insert(riskMarketVals.begin()+pos+count,0.0);
		}
	}*/

	return;
}

/*!
    @brief get IsSwapTenorAdjust
	@return IsSwapTenorAdjust
*/
const AQLDataBool&  
LAMathYieldCurvePro::getIsSwapTenorAdjust(void) const
{
	return dynamic_cast<const AQLDataBool&>(mpIsSwapTenorAdjust->get());
}
/*!
    @brief get IsSwapTenorCahnge
	@return IsSwapTenorCahnge
*/
AQLDataBool&  
LAMathYieldCurvePro::getIsSwapTenorAdjust(void)
{
	return dynamic_cast<AQLDataBool&>(mpIsSwapTenorAdjust->get());
}

/*!
    @brief  

	@return 
*/
const AQLDataReference&
LAMathYieldCurvePro::getForeignYieldData() const
{
	return dynamic_cast<const AQLDataReference&>(mpForeignYieldData->get());
}
/*!
    @brief 
			
	@return 
*/
AQLDataReference&
LAMathYieldCurvePro::getForeignYieldData()
{
	return dynamic_cast<AQLDataReference&>(mpForeignYieldData->get());
}

/*!
    @brief  

	@return 
*/
const AQLDataReference&
LAMathYieldCurvePro::getColYieldData() const
{
	return dynamic_cast<const AQLDataReference&>(mpColYieldData->get());
}

/*!
    @brief 
			
	@return 
*/
AQLDataReference&
LAMathYieldCurvePro::getColYieldData()
{
	return dynamic_cast<AQLDataReference&>(mpColYieldData->get());
}

/*!
    @brief calc floater PV
	@param[in] curveName

*/
void
LAMathYieldCurvePro::setFloater(const AQLString& curveName)
{
	AQLString tmpCurveName = curveName;
	AQLObjectHolder &objHolder = getYieldData().get();
	AQLDataHolder *dh;
	// data check
	bool isCheckCurves = false;
	dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	bool isCurveAttrExist = checkCurveAttr(curveName);
	if (isCheckCurves && isCurveAttrExist) return;

	AQLPriceDataDayCount dc_act(ACT_365_ISDA);
	AQLObject &yieldData = objHolder.get();

	AQLString discountName = 
		dynamic_cast<AQLDataString&> (getData(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpCurveName.toLower(), ISNOTNULL).get());
	AQLString forecastName = 
		dynamic_cast<AQLDataString&> (getData(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpCurveName.toLower(), ISNOTNULL).get());
	AQLString suffix_d = ""; if (discountName != STD) suffix_d = AQLString("_") + discountName;
	AQLString suffix_f = ""; if (forecastName != STD) suffix_f = AQLString("_") + forecastName;
	
	// set forecast curve
	AQLPriceDataInterpolation f_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	const DoubleArray &terms_f = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix_f, ISNOTNULL)).get()).get();
	const DoubleArray &dfs_f   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix_f, ISNOTNULL)).get()).get();
	f_inter.set(terms_f, dfs_f);

	// set discount curve
	AQLPriceDataInterpolation d_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	const DoubleArray &terms_d = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix_d, ISNOTNULL)).get()).get();
	const DoubleArray &dfs_d   = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix_d, ISNOTNULL)).get()).get();
	d_inter.set(terms_d, dfs_d);

	const DoubleArray &terms_std = dynamic_cast<const AQLDataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms_std.back();
	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());	

	DoubleArray dfs; dfs.push_back(1.);
	DoubleArray terms; terms.push_back(0.);

	const double eps_term = 1E-10;
	
	dh = &getData(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpCurveName.toLower(),NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString basisMkt = dynamic_cast<AQLDataString &>(dh->get()).get();
		AQLString suffix = "_" + basisMkt;
		// get market data
		const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&>
			(getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
		vector<AQLObject*> data;
		for(unsigned int i = 0; i < mr.getSize(); i++)
		{
			// check use grid
			const AQLDataHolder *dh = &mr.get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;
			
		   const AQLString &dataType = dynamic_cast<const AQLDataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			AQLString tmpType = dataType;
			tmpType.toUpper();
			if (tmpType.findString(BASIS) == 0) 
			{
				data.push_back(&mr.get(i).get());
			}
		}
		// data exist check
		if (data.empty())
		{
			AQLString msg = "CurveName = "+ basisMkt + ", basis data is not set.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// sort
		sort(data.begin(), data.end(), Comp_term());
		// market convencion info
		const bool isTimeInter  = dynamic_cast<const AQLDataBool &>((data[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL)).get()).get();
		//eom roll
		bool eom = false;
		dh = &data[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) 
			eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		// cashlet
		const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &c_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
		AQLString c_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
		const int c_lag  = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());
		// index
		const AQLPriceDataDayCount &i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &i_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
		const int i_lag  = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
		AQLString i_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
		AQLString i_accessary = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();
		// isfwdinterpolation
		bool isFWDInter = false;
		AQLPriceDataInterpolation *fwdInter = 0;
		dh = &(data[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isFWDInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			fwdInter = &dynamic_cast<AQLPriceDataInterpolation &>(data[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		}
		if (isFWDInter)
		{
			AQLPriceDataDayCount dc;
			AQLPriceDataCalendar cal;
			AQLPriceDataSlidingRule sld;
			AQLString accessory;
			DoubleArray terms;
			DoubleArray fwds;
			DoubleArray taus;
			DoubleMatrix termsMat;
			getForwardConvention(forecastName, dc, sld, cal, accessory);
			getDayCount(forecastName) = dc;
			getBaseForwardRate(forecastName, terms, termsMat, taus, fwds);
			fwdInter->set(terms, fwds);
		}
		// get max term
		const AQLString &termMax = dynamic_cast<const AQLDataString &>((data.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		// calc term (apply to month)
		int y, m, d, w;
		LAMathDateCalculations::termStrtoYMDW(termMax, y, m, d, w);
		m = 12 * y + m;

		c_freq.toUpper();
		unsigned int mUnit = 0;
		if (c_freq == ANNUAL)
		{
			mUnit = 12;
		}
		else if (c_freq == SEMI_ANNUAL)
		{
			mUnit = 6;
		}
		else if (c_freq == QUARTERLY)
		{
			mUnit = 3;
		}
		else if (c_freq == MONTHLY || c_freq == LUNAR)
		{
			mUnit = 1;
		}
		else 
		{
			AQLString msg = "frequency is wrong";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// set roll convention
		AQLString roll_conv("");
		if (c_freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
		else if (eom) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;
	
		// get interpolation method
		AQLPriceDataInterpolation &b_inter = getInterpolation_bs();
		DoubleArray b_t_grid(1, 0.0);
		DoubleArray b_termstruct_grid(1, 0.0);
		AQLStringVector basisTerms;

		for (unsigned int i = 0; i < data.size(); i++)
		{
			const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			basisTerms.push_back(strTerm);

			double rate = dynamic_cast<const AQLDataDouble &>((data[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			double term = 0.0;
			AQLDate tmpDate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			term = c_dc.getTerm(c_spotdate, tmpDate);

			b_t_grid.push_back(term);
			if (isTimeInter)
			{
				b_termstruct_grid.push_back(rate * term);
			}
			else
			{
				b_termstruct_grid.push_back(rate);
			}
		}
		b_inter.set(b_t_grid, b_termstruct_grid);

		const unsigned int max = m / mUnit + 1;

		const double spotTerm = dc_act.getTerm(asof, c_spotdate);
		const double d_spotdf = d_inter.value(spotTerm);
		if (asof != c_spotdate)
		{	
			dfs.push_back(d_spotdf);
			terms.push_back(spotTerm);
		}
		double annuity = 0.;
		double rateTauDF = 0.;
		DateVector dvzeroDates;
		for (unsigned int i = 1; i < max; ++i)
		{
			AQLString strTerm = AQLString(static_cast<int>(mUnit * (i - 1))) + AQLString("M");
			AQLString dfstrTerm = AQLString(static_cast<int>(mUnit * i)) + AQLString("M");
			AQLDate fdate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			AQLDate ldate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true, &roll_conv);
			double dfTerm = dc_act.getTerm(asof, ldate);
			double dfTerm_last = dc_act.getTerm(asof, fdate);
			if (dfTerm > tmax + eps_term)
			{
				break;
			}
			//analytic for CalcDatesForDVZero
			dvzeroDates.push_back(ldate);

			const double dfTerm_dc = c_dc.getTerm(c_spotdate, ldate);
			double spread;
			if (isTimeInter && dfTerm_dc != 0.0)
			{
				spread = b_inter.value(dfTerm_dc) / dfTerm_dc;
			}
			else
			{
				spread = b_inter.value(dfTerm_dc);
			}
			DoubleArray i_gridVec;
			DoubleArray i_termVec;
			calcIndexGrid(asof, fdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
			// calc index rate
			/*double ret = 0.0;
			for (unsigned int k = 1; k < i_gridVec.size(); ++k)
			{
				ret += i_termVec[k - 1] * f_inter.value(i_gridVec[k]);
			}*/
			const double delta = c_dc.getTerm(fdate, ldate);
			double rate = 0.;
			if (isFWDInter)
			{
				rate = fwdInter->value(dfTerm_last);
			}
			else
			{
				double ret = delta * f_inter.value(dfTerm);
				//const double rate = (f_inter.value(i_gridVec[0]) -  f_inter.value(i_gridVec.back())) / ret;
				rate = (f_inter.value(dfTerm_last) -  f_inter.value(dfTerm)) / ret;
			}
			const double df = d_inter.value(dfTerm);
			
			annuity += delta * df;
			rateTauDF += rate * delta * df;
			double floaterPV = rateTauDF + annuity * spread + 1.0 * df;
			dfs.push_back(floaterPV);
			terms.push_back(dfTerm);
		}

		for (unsigned int i = 0; i < data.size(); i++)
		{
			DateVector ret;

			const AQLString &strTerm  = dynamic_cast<const AQLDataString &>((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLDate matudate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			ret.push_back(c_spotdate);

			//find dvzeroDates
			unsigned int pos=0;
			if (!AQLAlgorithm::find<DateVector,AQLDate>(dvzeroDates,matudate,0,dvzeroDates.size()-1 ,pos))
			{
				//this means basis grid e.x. 100y is longer than DF1 max grid, which ofcourse can not be calculated.
				if (matudate > dvzeroDates.back())
					continue;
				else
					throw AQLCoreInvalidData("BasisGrid Search Error",__FILE__,__LINE__);
			}

			ret.insert(ret.end(),dvzeroDates.begin(),dvzeroDates.begin()+pos+1);
			if(!data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
			{
				data[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(ret));
			}
			else
			{
				dynamic_cast<AQLDataDates&>(data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(ret);
			}
		}

		// cashlet
		objHolder.remove(IR_CALIBRATION_DATA_CASHLETCALENDAR + AQLString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_CASHLETCALENDAR + AQLString("_") + curveName, new AQLPriceDataCalendar(c_cal));
		objHolder.remove(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE + AQLString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE + AQLString("_") + curveName, new AQLPriceDataSlidingRule(c_sld));
		objHolder.remove(IR_CALIBRATION_DATA_CASHLETSPOTDATE + AQLString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_CASHLETSPOTDATE + AQLString("_") + curveName, new AQLDataDate(c_spotdate));
		objHolder.remove(IR_CALIBRATION_DATA_XCCYBASISTERM + AQLString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_XCCYBASISTERM + AQLString("_") + curveName, new AQLDataStrings(basisTerms));
		objHolder.remove(IR_CALIBRATION_DATA_ISEOMROLL + AQLString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_ISEOMROLL + AQLString("_") + curveName, new AQLDataBool(eom));

		const AQLDataHolder *dh = &data[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& adjustValue_term = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
			objHolder.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM + AQLString("_") + curveName);
			objHolder.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM + AQLString("_") + curveName, new AQLDataDoubles(adjustValue_term));

			const DoubleArray& adjustValue 
				= dynamic_cast<const AQLDataDoubles&>(data[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL).get()).get();
			objHolder.remove(IR_CALIBRATION_DATA_ADJUSTVALUE + AQLString("_") + curveName);
			objHolder.add(IR_CALIBRATION_DATA_ADJUSTVALUE + AQLString("_") + curveName, new AQLDataDoubles(adjustValue));

			const AQLPriceDataInterpolation& interpAtt
				= dynamic_cast<const AQLPriceDataInterpolation&>(data[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get());
			objHolder.remove(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION + AQLString("_") + curveName);
			objHolder.add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION + AQLString("_") + curveName, new AQLPriceDataInterpolation(interpAtt));
		}
	}
	else
	{
		// get market data
		const AQLDataMultiReference& mr = getMarketData();
		vector<AQLObject*> data;
		for(unsigned int i = 0; i < mr.getSize(); i++)
		{
			// check use grid
			const AQLDataHolder *dh = &mr.get(i).get().getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

			AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(i).get().getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			datatype_str.toUpper();
			if (datatype_str == PAR) data.push_back(&mr.get(i).get());
		}
		// data exist check
		if (data.empty())
		{
			AQLString msg = "swap market is not set.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// sort
		sort(data.begin(), data.end(), Comp_term());
		//eom roll
		bool eom = false;
		dh = &data[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) 
			eom = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		// cashlet
		const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL).get());
		const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &c_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
		AQLString c_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL).get()).get();
		const int c_lag  = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get());
		// index
		const AQLPriceDataDayCount &i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &i_sld  = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const int i_lag  = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get());
		AQLString i_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL).get()).get();
		// isfwdinterpolation
		bool isFWDInter = false;
		AQLPriceDataInterpolation *fwdInter = 0;
		dh = &(data[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isFWDInter = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			fwdInter = &dynamic_cast<AQLPriceDataInterpolation &>(data[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		}
		if (isFWDInter)
		{
			AQLPriceDataDayCount dc;
			AQLPriceDataCalendar cal;
			AQLPriceDataSlidingRule sld;
			AQLString accessory;
			DoubleArray terms;
			DoubleArray fwds;
			DoubleArray taus;
			DoubleMatrix termsMat;
			getForwardConvention(forecastName, dc, sld, cal, accessory);
			getDayCount(forecastName) = dc;
			getBaseForwardRate(forecastName, terms, termsMat, taus, fwds);
			fwdInter->set(terms, fwds);
		}
		// get max term
		const AQLString &termMax = dynamic_cast<const AQLDataString &>((data.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		// calc term (apply to month)
		int y, m, d, w;
		LAMathDateCalculations::termStrtoYMDW(termMax, y, m, d, w);
		m = 12 * y + m;

		c_freq.toUpper();
		unsigned int mUnit = 0;
		if (c_freq == ANNUAL)
		{
			mUnit = 12;
		}
		else if (c_freq == SEMI_ANNUAL)
		{
			mUnit = 6;
		}
		else if (c_freq == QUARTERLY)
		{
			mUnit = 3;
		}
		else if (c_freq == MONTHLY || c_freq == LUNAR)
		{
			mUnit = 1;
		}
		else 
		{
			AQLString msg = "frequency is wrong";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		AQLString i_accessary = AQLString(static_cast<int > (mUnit)) + AQLString("M");
		// set roll convention
		AQLString roll_conv("");
		if (c_freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
		else if (eom) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;

		const unsigned int max = m / mUnit + 1;
		
		const double spotTerm = dc_act.getTerm(asof, c_spotdate);
		const double d_spotdf = d_inter.value(spotTerm);
		if (asof != c_spotdate)
		{	
			dfs.push_back(d_spotdf);
			terms.push_back(spotTerm);
		}

		double rateTauDF = 0.;
		for (unsigned int i = 1; i < max; ++i)
		{
			AQLString strTerm = AQLString(static_cast<int>(mUnit * (i - 1))) + AQLString("M");
			AQLString dfstrTerm = AQLString(static_cast<int>(mUnit * i)) + AQLString("M");
			AQLDate fdate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			AQLDate ldate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true, &roll_conv);
			double dfTerm = dc_act.getTerm(asof, ldate);
			double dfTerm_last = dc_act.getTerm(asof, fdate);
			if (dfTerm > tmax + eps_term)
			{
				break;
			}

			DoubleArray i_gridVec;
			DoubleArray i_termVec;
			calcIndexGrid(asof, fdate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
			// calc index rate
			/*double ret = 0.0;
			for (unsigned int k = 1; k < i_gridVec.size(); ++k)
			{
				ret += i_termVec[k - 1] * f_inter.value(i_gridVec[k]);
			}*/
			const double delta = c_dc.getTerm(fdate, ldate);
			double rate = 0.;
			if (isFWDInter)
			{
				rate = fwdInter->value(dfTerm_last);
			}
			else
			{
				double ret = delta * f_inter.value(dfTerm);
				//const double rate = (f_inter.value(i_gridVec[0]) -  f_inter.value(i_gridVec.back())) / ret;
				rate = (f_inter.value(dfTerm_last) -  f_inter.value(dfTerm)) / ret;
			}
			const double df = d_inter.value(dfTerm);
			rateTauDF += rate * delta * df;
			double floaterPV = rateTauDF + 1.0 * df;
			dfs.push_back(floaterPV);
			terms.push_back(dfTerm);
		}
	}

	objHolder.remove(CALIBRATION_DATA_TERMS + AQLString("_") + curveName);
	objHolder.remove(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveName);
	objHolder.add(CALIBRATION_DATA_TERMS + AQLString("_") + curveName, new AQLDataDoubles(terms));
	objHolder.add(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveName, new AQLDataDoubles(dfs));
}

/*!
    @brief check whether dataValues of curves exist or not

	@param[in] market name
*/
bool 
LAMathYieldCurvePro::checkCurveAttr(const AQLString& mktName) const
{
	bool ret = true;
	const AQLDataHolder *dh;
	const AQLDataReference& ref = getYieldData();
	AQLObjectHolder objHolder = ref.get();
	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (it->second == mktName)
		{
			const AQLString& curveName = it->first;
			AQLString suffix = "";
			if (curveName != STD) suffix = "_" + curveName;
			dh = &(objHolder.getData(CALIBRATION_DATA_TERMS + suffix, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) ret = false;
			dh = &(objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) ret = false;					
		}
	}

	return ret;
}

/*!
    @brief remove all curve data
	@param[out] yieldData
*/
void 
LAMathYieldCurvePro::removeAllCuveData(AQLObject &yieldData) const
{
	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (mNonRemovableMarket.end() != mNonRemovableMarket.find(it->second))
		{
			continue;
		}
		const AQLString& curveName = it->first;
		AQLString suffix = "";
		if (curveName != STD) suffix = "_" + curveName;
		yieldData.remove(CALIBRATION_DATA_TERMS + suffix);
		yieldData.remove(IR_CALIBRATION_DATA_DFS + suffix);		
	}
}

/*!
    @brief remove curve data

	@param[out] yieldData
	@param[in] market name
*/
void 
LAMathYieldCurvePro::removeCuveData(AQLObject &yieldData, const AQLString& mktName) const
{
	if (mNonRemovableMarket.end() != mNonRemovableMarket.find(mktName))
	{
		AQLString msg = "This market cannot be removed. market = " + mktName;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (it->second == mktName)
		{
			const AQLString& curveName = it->first;
			AQLString suffix = "";
			if (curveName != STD) suffix = "_" + curveName;
			yieldData.remove(CALIBRATION_DATA_TERMS + suffix);
			yieldData.remove(IR_CALIBRATION_DATA_DFS + suffix);		
		}
	}
}

/*!
    @brief remove basis curve data

	@param[out] yieldData
	@param[in] market name
*/
void 
LAMathYieldCurvePro::removeBasisCuveData(AQLObject &yieldData) const
{
	const AQLDataHolder *dh = &getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLStringVector &basisDFs = dynamic_cast<const AQLDataStrings &>(dh->get()).get();		
		for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
		{
			if (mNonRemovableMarket.end() != mNonRemovableMarket.find(it->second))
			{
				continue;
			}
			if (basisDFs.end() != std::find(basisDFs.begin(), basisDFs.end(), it->first))
			{
				const AQLString& curveName = it->first;
				AQLString suffix = "";
				if (curveName != STD) suffix = "_" + curveName;
				yieldData.remove(CALIBRATION_DATA_TERMS + suffix);
				yieldData.remove(IR_CALIBRATION_DATA_DFS + suffix);		
			}
		}
	}
}

/*!
    @brief get forward convenction

	@param[in] curvename
	@param[out] daycount
	@param[out] sld
	@param[out] cal
*/
void 
LAMathYieldCurvePro::getForwardConvention(const AQLString &curveName, AQLPriceDataDayCount &dc, AQLPriceDataSlidingRule &sld, AQLPriceDataCalendar &cal, AQLString &accessary) const
{
	const AQLString def_accessary = "1Y";
	const AQLDataHolder *dh = 0;
	if (mBCurveGenMap.find(curveName) != mBCurveGenMap.end())
	{
		AQLString suffix = "_" + getMarketForCurve(curveName);
		dh = &getData(CALIBRATION_DATA_MARKETDATA + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{

			const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&>(dh->get());
			vector<AQLObject*> data_basis;
			for (unsigned int i = 0; i < mr.getSize(); ++i)
			{
				AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				datatype_str.toUpper();
				if (datatype_str == BASIS)
				{
					data_basis.push_back(&mr.get(i).get());
				}
				else if (datatype_str == FWDFX)
				{
					data_basis.push_back(&mr.get(i).get());
				}
			}
			if (data_basis.empty())
			{
				AQLString msg = "Basis Data is empty. curveName = " + curveName;
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			dc = dynamic_cast<const AQLPriceDataDayCount &>(data_basis[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
			sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_basis[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
			// calendar get from cashflow info, because we use frn method.
			cal = dynamic_cast<const AQLPriceDataCalendar &>(data_basis[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
			accessary = dynamic_cast<const AQLDataString &>(data_basis[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();
		}
		else
		{
			accessary = def_accessary;
		}
	}
	else
	{
		AQLString suffix = "";
		const AQLString market = getMarketForCurve(curveName);
		if (!getIsArbFree() && market != SWAP)
		{
			suffix = "_" + market;
		}

		dh = &getData(CALIBRATION_DATA_MARKETDATA + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&>(dh->get());

			vector<AQLObject*> data_libor;
			vector<AQLObject*> data_swap;
			for (unsigned int i = 0; i < mr.getSize(); ++i)
			{
				AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				datatype_str.toUpper();
				if (datatype_str == ZERO)
				{
					data_libor.push_back(&mr.get(i).get());
				}
				else if (datatype_str == PAR)
				{
					data_swap.push_back(&mr.get(i).get());
				}
			}
			if (data_libor.empty() || data_swap.empty())
			{
				accessary = def_accessary;
				return;
				//AQLString msg = "Libor or Swap Data is empty. curveName = " + curveName;
				//throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			dc = dynamic_cast<const AQLPriceDataDayCount &>(data_libor[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
			sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_libor[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
			cal = dynamic_cast<const AQLPriceDataCalendar &>(data_libor[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());

			if (getIsArbFree())
			{
				if (market == AF3ML)
				{
					accessary = "3M";
				}
				else if (market == AF6ML)
				{
					accessary = "6M";
				}
				else
				{
					accessary = def_accessary;
				}
			}
			else
			{
				AQLString freq;
				dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					freq = dynamic_cast<const AQLDataString &>(dh->get()).get();
				}
				else
				{
					dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
					if (dh->isDefined() && !dh->isNull())
					{
						freq = dynamic_cast<const AQLDataString &>(dh->get()).get();
					}
					else
					{
						freq = dynamic_cast<const AQLDataString &>(data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, NOCHECK).get()).get();
					}
				}
				if (freq == ANNUAL)
				{
					accessary = "12M";
				}
				else if (freq == SEMI_ANNUAL)
				{
					accessary = "6M";
				}
				else if (freq == QUARTERLY)
				{
					accessary = "3M";
				}
				else if (freq == MONTHLY || freq == LUNAR)
				{
					accessary = "1M";
				}
				else 
				{
					AQLString msg = "frequency is wrong";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}
		}
		else
		{
			accessary = def_accessary;
		}
	}
}
/*!
    @brief insert non removable market

	@param[in] mktName

*/
void 
LAMathYieldCurvePro::insertNonRemovableMarket(const AQLString& mktName)
{
	if (mNonRemovableMarket.end() == mNonRemovableMarket.find(mktName))
	{
		mNonRemovableMarket.insert(mktName);
	}
}


/*!
    @brief erase non removable market

	@param[in] mktName

*/
void 
LAMathYieldCurvePro::eraseNonRemovableMarket(const AQLString& mktName)
{
	if (mNonRemovableMarket.end() != mNonRemovableMarket.find(mktName))
	{
		mNonRemovableMarket.erase(mktName);
	}
}

/*
    @brief set a rate convention into a curve data object
    
    @param[in] objHolder	object holder
	@param[in] mktData	market object
	@param[in] curveName          
*/
void
LAMathYieldCurvePro::setCurveConvention(AQLObjectHolder& objHolder,
									  std::vector<AQLObject*>& mktData,
									  const AQLString& curveName)
{
	AQLString suffix = "";
	if (curveName != STD) suffix = "_" + curveName;
	
	if (mktData.size() == 0)
	{
		throw AQLCoreInvalidData("Swap size must be more than one", __FILE__, __LINE__);
	}

	AQLString freq = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	const AQLPriceDataCalendar* cal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule* sld  = &dynamic_cast<const AQLPriceDataSlidingRule &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
	const AQLPriceDataDayCount* dc = &dynamic_cast<const AQLPriceDataDayCount &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
	AQLString accessary = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

	objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
	objHolder.remove(CALIBRATION_DATA_CALENDAR + suffix);
	objHolder.remove(CALIBRATION_DATA_SLIDINGRULE + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
	
	objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new AQLDataString(freq));
	objHolder.add(CALIBRATION_DATA_CALENDAR + suffix, new AQLPriceDataCalendar(*cal));
	objHolder.add(CALIBRATION_DATA_SLIDINGRULE + suffix, new AQLPriceDataSlidingRule(*sld));
	objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT + suffix, new AQLPriceDataDayCount(*dc));
	objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new AQLDataString(accessary));
}	

/*
	@Desctiption calculate an annuity

	@param spotTerm [in] spot term
	@param terms_grid [in] terms of payment timings of the annuity
	@param terms_interval [in] terms of calculation periods of the annuity
	@param d_inter [in] discount factor
	@return annuity
*/
double LAMathYieldCurvePro::calcAnnuity(double spotTerm, const DoubleArray& terms_grid, const DoubleArray& terms_interval, const AQLPriceDataInterpolation& d_inter)
{
	double annuity = 0.0;
	for (std::size_t j = 0, je = terms_grid.size(); j < je; ++j)
	{
		annuity += terms_interval[j] * d_inter.value(terms_grid[j] + spotTerm);
	}
	return annuity;
}

/*
	@Desctiption calculate payment dates and terms

	@param spotdate [in] spot date
	@param strTerm [in] string of maturity term
	@param sld [in] sliding rule
	@param cal [in] calendar
	@param roll_conv [in] roll convention
	@param freq [in] frequency of coupon reset
	@param freq_paymenat [in] frequency of payment
	@param dc [in] daycount
	@param eom [in] end-of-month flag
	@param isBackward [in] flag of backward or forward
	@param dates [out] payment dates
	@param terms_grid [out] grids of payment dates
	@param terms_interval [out] terms of calculation periods
	@param num_reset [out] number of reset in accrual periods
	@return void
*/
void LAMathYieldCurvePro::deducePaymentDatesAndTerms(const AQLDate& spotdate, const AQLString& strTerm, const AQLPriceDataSlidingRule& sld, const AQLPriceDataCalendar& cal,
	const AQLString& roll_conv, const AQLString& freq, const AQLString& freq_payment, const AQLPriceDataDayCount& dc, bool eom, bool isBackward,
	DateVector& dates, DoubleArray& terms_grid, DoubleArray& terms_interval, IntArray& num_reset)
{
	AQLDate end;
	if (isBackward)
	{
		end = LAMathDateCalculations::getDate(spotdate, strTerm, true);
	}
	else
	{
		end = LAMathDateCalculations::getDate(spotdate, strTerm, sld, &cal, true, &roll_conv);
	}

	dates.clear();
	terms_grid.clear();
	terms_interval.clear();
	num_reset.clear();
	if (freq == freq_payment)
	{
		LAPriceYieldGenerator::getPaymentDates(spotdate, end, freq, cal, sld, dc, dates, terms_grid, terms_interval, eom, isBackward);
		num_reset.resize(dates.size(), 1);
	}
	else
	{
		AQLDate end_unadjust = LAMathDateCalculations::getDate(spotdate, strTerm, true);
		DateVector reset_dates;
		AQLString roll_conv_payment(LAPriceYieldGenerator::deduceRollConvention(freq_payment, eom));
		int roll_date_payment = spotdate.dayOfMonth();
		if (freq == BUSINESS_DAYS || freq == DAILY)
		{
			LAMathDateCalculations::generateSchedule(spotdate, end, freq_payment, true, NULL, NULL, &roll_date_payment, reset_dates, &sld, &cal, !isBackward, &roll_conv_payment);
		}
		else
		{
			LAMathDateCalculations::generateSchedule(spotdate, end_unadjust, freq_payment, true, NULL, NULL, &roll_date_payment, reset_dates, NULL, NULL, !isBackward, &roll_conv_payment);
		}

		if (reset_dates.size() < 1)
		{
			throw AQLCoreInvalidData("There are no payment date in the target basis swap.", __FILE__, __LINE__);
		}
		LAPriceYieldGenerator::getPaymentDates(spotdate, reset_dates[0], freq, cal, sld, dc, dates, terms_grid, terms_interval, eom, isBackward);
		num_reset.push_back(dates.size());
		for (unsigned int i = 1; i < reset_dates.size(); ++i)
		{
			DateVector index_payment_dates;
			DoubleArray index_payment_grids;
			DoubleArray index_accrual_term;
			LAPriceYieldGenerator::getPaymentDates(spotdate, reset_dates[i - 1], reset_dates[i], freq, cal, sld, dc, index_payment_dates, index_payment_grids, index_accrual_term, eom, isBackward);
			index_accrual_term.front() = dc.getTerm(dates.back(), index_payment_dates.front());
			dates.insert(dates.end(), index_payment_dates.cbegin(), index_payment_dates.cend());
			terms_grid.insert(terms_grid.end(), index_payment_grids.cbegin(), index_payment_grids.cend());
			terms_interval.insert(terms_interval.end(), index_accrual_term.cbegin(), index_accrual_term.cend());
			num_reset.push_back(index_payment_dates.size());
		}
	}

	int total_num_reset = accumulate(num_reset.cbegin(), num_reset.cend(), 0);
	if (dates.size() < total_num_reset || terms_grid.size() < total_num_reset || terms_interval.size() < total_num_reset)
	{
		throw AQLCoreInvalidData("Total number of fixing and total number of coupon reset time must be match.", __FILE__, __LINE__);
	}
}

/*
	@Desctiption calculate payment dates and terms of target leg

	@param strTerm [in] string of maturity term
	@param roll_conv [in] roll convention
	@param eom [in] end-of-month flag
	@param isSameGridIndex [in] flag of same grid(payment grids is used for forward rate calculation) or floating rate grid(index grids is used for forward rate calculation) 
	@param c_spotdate [in] spot date of targte leg
	@param c_sld [in] sliding rule of target leg
	@param c_cal [in] calendar of target leg
	@param freq [in] frequency of coupon reset of target leg
	@param freq_paymenat [in] frequency of payment of target leg
	@param c_dc [in] daycount of target leg
	@param c_isBackward [in] flag of backward or forward of target leg
	@param a_c_spotdate [in] spot date of against leg
	@param dates [out] payment dates
	@param terms_grid [out] grids of payment dates
	@param terms_interval [out] terms of calculation periods
	@param num_reset [out] number of reset in accrual periods
	@return void
*/
void LAMathYieldCurvePro::deducePaymentDatesAndTermsThisSide(const AQLString& strTerm, const AQLString& roll_conv, bool eom, bool isSameGridIndex,
	const AQLDate& c_spotdate, const AQLPriceDataSlidingRule& c_sld, const AQLPriceDataCalendar& c_cal, const AQLString& c_freq, const AQLString& c_freq_payment, const AQLPriceDataDayCount& c_dc, bool c_isBackward, const AQLDate& a_c_spotdate,
	DateVector& dates, DoubleArray& terms_grid, DoubleArray& terms_interval, IntArray& num_reset)
{
	if (c_spotdate < a_c_spotdate && !isSameGridIndex)
	{
		// roll from a_c_spotdate
		deducePaymentDatesAndTerms(a_c_spotdate, strTerm, c_sld, c_cal, roll_conv, c_freq, c_freq_payment, c_dc, eom, c_isBackward, dates, terms_grid, terms_interval, num_reset);
		// calc odd date
		AQLPriceDataDayCount dc_act(ACT_365_ISDA);
		const double odd_term = dc_act.getTerm(c_spotdate, a_c_spotdate);
		const double odd_interval = c_dc.getTerm(c_spotdate, a_c_spotdate, false);
		// term from c_spotdate
		DoubleArray plus_term(terms_grid.size(), odd_term);
		transform(terms_grid.begin(), terms_grid.end(), plus_term.begin(), terms_grid.begin(), plus<double>());
		// insert odd term
		dates.insert(dates.begin(), c_spotdate);
		terms_grid.insert(terms_grid.begin(), odd_term);
		terms_interval.insert(terms_interval.begin(), odd_interval);
		num_reset.push_back(1);
	}
	else
	{
		deducePaymentDatesAndTerms(c_spotdate, strTerm, c_sld, c_cal, roll_conv, c_freq, c_freq_payment, c_dc, eom, c_isBackward, dates, terms_grid, terms_interval, num_reset);
	}
}
