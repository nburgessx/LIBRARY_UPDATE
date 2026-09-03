// LAMathYieldCurvePro.h

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

const double INFINITESIMAL = 1E-7;

#include "LAMathDefine.h"
#include "LABasic.h"
#include "LANl2sol.h"
#include "LAPriceDataConvention.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataManager.h"
#include "LADataProcedure.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataInterpolation.h"
#include "LADataMultiReference.h"
#include "LADataMatrix.h"
#include "LAPriceDataFunction.h"
#include "LADataInstance.h"
#include "LACoreUtil.h"
#include "LAOptimumBrent.h"
#include "LAPriceYieldGenerator.h"
#include "LAMathDateCalculations.h"
#include "LAMathDateUtilities.h"
#include "LALinearInterpolation.h"
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "LAAlgorithm.h"
#include "LAMatrix.h"
#include "LAMathFXEntity.h"
#include "ConstantDeclarations.h"
#include "LAObjectHolder.h"
#include "LAMathYieldCurvePro.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"

using namespace std;

//====================================================================
/*!
    @brief constructor

	@param[in] dataInstance pointer of LADataInstance

*/
LAMathYieldCurvePro::LAMathYieldCurvePro(LADataInstance* dataInstance) : 
				LAMathYieldCurve(dataInstance)//,mCurveVersion(0)
{
	setDataInstance(dataInstance);
	LAPriceDataManager& dm = dataInstance->getDataMaster();

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
const LADataDate&  
LAMathYieldCurvePro::getAsOfDate(void) const
{
	return dynamic_cast<const LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief get AsOfDate
	@return AsOfDate
*/
LADataDate&  
LAMathYieldCurvePro::getAsOfDate(void)
{
	return dynamic_cast<LADataDate&>(mpAsOfDate->get());
}

/*!
    @brief get IsArbFree
	@return IsArbFree
*/
const LADataBool&  
LAMathYieldCurvePro::getIsArbFree(void) const
{
	return dynamic_cast<const LADataBool&>(mpIsArbFree->get());
}
/*!
    @brief get IsArbFree
	@return IsArbFree
*/
LADataBool&  
LAMathYieldCurvePro::getIsArbFree(void)
{
	return dynamic_cast<LADataBool&>(mpIsArbFree->get());
}

/*!
    @brief get MarketData
			
	@return MarketData
*/
const LADataMultiReference&
LAMathYieldCurvePro::getMarketData() const
{
	return dynamic_cast<const LADataMultiReference&>(mpMarketData->get());
}
/*!
    @brief get MarketData
			
	@return MarketData
*/
LADataMultiReference&
LAMathYieldCurvePro::getMarketData()
{
	return dynamic_cast<LADataMultiReference&>(mpMarketData->get());
}
/*!
    @brief get BasisRates
			
	@return BasisRates
*/

const LADataDoubleMatrix&
LAMathYieldCurvePro::getBasisRates() const	
{
	return dynamic_cast<const LADataDoubleMatrix&>(mpBasisRates->get());
}

/*!
    @brief get BasisRates and set BasisRates
			
	@return BasisRates
*/
LADataDoubleMatrix&
LAMathYieldCurvePro::getBasisRates() 
{
	return dynamic_cast<LADataDoubleMatrix&>(mpBasisRates->get());
}

/*!
    @brief get BasisData
			
	@return BasisData
*/

const LADataMultiReference&
LAMathYieldCurvePro::getBasisData() const	
{
	return dynamic_cast<const LADataMultiReference&>(mpBasisData->get());
}

/*!
    @brief get BasisData and set BasisData
			
	@return BasisData
*/
LADataMultiReference&
LAMathYieldCurvePro::getBasisData() 
{
	return dynamic_cast<LADataMultiReference&>(mpBasisData->get());
}


/*!
    @brief get Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/

const LADataBool&
LAMathYieldCurvePro::getIsFutureUse() const	
{
	return dynamic_cast<const LADataBool&>(mpIsFutureUse->get());
}

/*!
    @brief get Use FutureRate or not and set Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/
LADataBool&
LAMathYieldCurvePro::getIsFutureUse() 
{
	return dynamic_cast<LADataBool&>(mpIsFutureUse->get());
}

/*!
    @brief get Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/

const LADataBool&
LAMathYieldCurvePro::getIsFRAUse() const	
{
	return dynamic_cast<const LADataBool&>(mpIsFRAUse->get());
}

/*!
    @brief get Use FutureRate or not and set Use FutureRate or not
			
	@return true : use FutureRate
	        false: don't use FutureRate
*/
LADataBool&
LAMathYieldCurvePro::getIsFRAUse() 
{
	return dynamic_cast<LADataBool&>(mpIsFRAUse->get());
}

/*!
    @brief get BaseYieldCurve
			
	@return BaseYieldCurve
*/

const LADataReference&
LAMathYieldCurvePro::getBaseYieldCurve() const	
{
	return dynamic_cast<const LADataReference&>(mpBaseYieldCurve->get());
}

/*!
    @brief get BaseYieldCurve and set BaseYieldCurve
			
	@return BaseYieldCurve
*/
LADataReference&
LAMathYieldCurvePro::getBaseYieldCurve() 
{
	return dynamic_cast<LADataReference&>(mpBaseYieldCurve->get());
}

/*!
    @brief get Interpolation to generate yield curve
			
	@return Interpolation to generate yield curve
*/
const LAPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_yg() const	
{
	return dynamic_cast<const LAPriceDataInterpolation&>(mpInterYG->get());
}

/*!
    @brief get Interpolation to generate yield curve
			
	@return Interpolation to generate yield curve
*/
LAPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_yg() 
{
	return dynamic_cast<LAPriceDataInterpolation&>(mpInterYG->get());
}

/*!
    @brief get Interpolation to use future
			
	@return Interpolation to use future
*/
const LAPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_fw() const	
{
	return dynamic_cast<const LAPriceDataInterpolation&>(mpInterFW->get());
}

/*!
    @brief get Interpolation to use future
			
	@return Interpolation to use future
*/
LAPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_fw() 
{
	return dynamic_cast<LAPriceDataInterpolation&>(mpInterFW->get());
}

/*!
    @brief get Interpolation to set basis rates
			
	@return Interpolation to set basis rates
*/
const LAPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_bs() const	
{
	return dynamic_cast<const LAPriceDataInterpolation&>(mpInterBS->get());
}

/*!
    @brief get Interpolation to set basis rates
			
	@return Interpolation to set basis rates
*/
LAPriceDataInterpolation&
LAMathYieldCurvePro::getInterpolation_bs() 
{
	return dynamic_cast<LAPriceDataInterpolation&>(mpInterBS->get());
}

/*!
    @brief get Basis Function
			
	@return Basis Function
*/
const LAPriceDataFunction&
LAMathYieldCurvePro::getBasisFunction() const	
{
	return dynamic_cast<const LAPriceDataFunction&>(mpBasisFunction->get());
}

/*!
    @brief get Basis Function
			
	@return Basis Function
*/
LAPriceDataFunction&
LAMathYieldCurvePro::getBasisFunction() 
{
	return dynamic_cast<LAPriceDataFunction&>(mpBasisFunction->get());
}

LADataStrings&
LAMathYieldCurvePro::getRatePriority() 
{
	return dynamic_cast<LADataStrings&>(mpRatePriority->get());
}

const LADataStrings&
LAMathYieldCurvePro::getRatePriority() const 
{
	return dynamic_cast<LADataStrings&>(mpRatePriority->get());
}

/*!
    @brief get FXEntity
			
	@return FXEntity
*/
const LADataReference&
LAMathYieldCurvePro::getFXEntity() const	
{
	return dynamic_cast<const LADataReference&>(mpFXEntity->get());
}

/*!
    @brief get FXEntity and set FXEntity
			
	@return FXEntity
*/
LADataReference&
LAMathYieldCurvePro::getFXEntity() 
{
	return dynamic_cast<LADataReference&>(mpFXEntity->get());
}

/*!
    @brief get OptimizeMethod
			
	@return OptimizeMethod
*/
const LADataString&
LAMathYieldCurvePro::getOptimizeMethod() const	
{
	return dynamic_cast<const LADataString&>(mpOptimizeMethod->get());
}

/*!
    @brief get OptimizeMethod and set OptimizeMethod
			
	@return OptimizeMethod
*/
LADataString&
LAMathYieldCurvePro::getOptimizeMethod() 
{
	return dynamic_cast<LADataString&>(mpOptimizeMethod->get());
}

/*!
	@brief get CompoundingFunction

	@return 
*/
const LAPriceDataFunction&
LAMathYieldCurvePro::getCompoundingFunction() const
{
	return dynamic_cast<const LAPriceDataFunction&>(mpCompoundFunction->get());
}

/*!
	@brief get CompoundingFunction and set CompoundingFunction

	@return CompoundingFunction
*/
LAPriceDataFunction&
LAMathYieldCurvePro::getCompoundingFunction()
{
	return dynamic_cast<LAPriceDataFunction&>(mpCompoundFunction->get());
}


// OPERATION
/*!
    @brief set Interpolation

	@param[in] a	pointer of Interpolation function
	@param[in] name name of Interpolation 
*/
void
LAMathYieldCurvePro::setInterpolation(
	LAInterpolationBase* a, const LAString& name)
{
	dynamic_cast<LAPriceDataInterpolation*>(&(
		getData(CALIBRATION_DATA_INTERPOLATION).get()))->setMethod(a, name);
}
/*!
    @brief set Interpolation

	@param[in] name name of Interpolation 
*/
void
LAMathYieldCurvePro::setInterpolation(const LAString& name)
{
	dynamic_cast<LAPriceDataInterpolation*>(&(
		getData(CALIBRATION_DATA_INTERPOLATION).get()))->setMethod(name);
}
 
/*!
    @brief set Generator class

	@param[in] a	pointer of generate class
	@param[in] name name of generate class
*/
void
LAMathYieldCurvePro::setDFGenerator(const LACoreProcedure* a,
										const LAString& name)
{
	dynamic_cast<LADataProcedure*>(&(
		getData(CALIBRATION_DATA_CURVEGENERATOR).get()))->setMethod(a, name);
}

/*!
    @brief set Generator class

	@param[in] name name of generate class
*/
void
LAMathYieldCurvePro::setDFGenerator(const LAString& name)
{
	dynamic_cast<LADataProcedure*>(&(
		getData(CALIBRATION_DATA_CURVEGENERATOR).get()))->setMethod(name);
}

/*!
    @brief clone this class

	@return	pointer of this class
*/
LAObject* 
LAMathYieldCurvePro::clone() const
{
    try {
    	LAMathYieldCurvePro*	pCurve = new LAMathYieldCurvePro(*this);
    	return pCurve;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
void                
LAMathYieldCurvePro::remove(
	const LAString& dataName)
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
LAObject&
LAMathYieldCurvePro::copy(
	const LAObject& e)
{
	if (this == &e) return *this;

	LAMathYieldCurve::copy(e);
	if (!e.isTypeOf(ENTITY_IRYCPRO))
	{
		LAString err = "Assignement error for LAMathYieldCurvePro : from ";
		err += LAString(e.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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

	@return Reference of LADataHolder include this Data

*/
LADataHolder&
LAMathYieldCurvePro::add(const LAString& name)
{
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::add(name, dh);
}

/*!
    @brief Set data by name ( Remove then Add )

	@param[in] name name of Data

	@return Reference of LADataHolder include this Data

*/
LADataHolder&
LAMathYieldCurvePro::reset(const LAString& name)
{
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::reset(name, dh);
}

/*!
    @brief culc DF from marketData

	@param[in] asof As of Date
*/

void
LAMathYieldCurvePro::calcDiscountFactor(const LADate& asof)
{
	try
	{
		if(getAsOfDate().get() != asof) getAsOfDate() = asof;
		dynamic_cast<LADataProcedure&>(mpProcedure->get()).calibrateModel(asof);
	}
	catch(LACoreError& e)
	{
		LAString msg("DiscountFactor Calculation Error at ");
		msg += getName();
		LACoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
		err += e;
		throw err;
	}
	catch (...)
	{
		LAString msg("DiscountFactor Calculation at ");
		msg += getName().get();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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

	LAObjectHolder objHolder = getYieldData().get();
	const LAObject& YieldData = objHolder.get();
	const LADataDoubles& terms = dynamic_cast<const LADataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get());
	
	LADataDoubles rates;
	rates.resize(terms.getSize());
	for (i = 0; i < terms.getSize() ; i++)
		rates.set(getZeroRate(terms[i]), i);
	
	if (pos > grids.size() - 1 || grids[grids.size() - 1] > terms.getSize() - 1)
	{
		// error
		LAString msg = getName();
		msg += " : Input pos or grids are something wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
	LAPriceDataConvention conv(getDayCount().getDayCount(), rc);
	for (i = 0; i < terms.getSize() ; i++)
		rates.set(1 / LAPriceDataConvention::rateToRet(rates[i], terms[i], conv) ,i);
	
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new LADataDoubles(rates));
}

/*!
    @brief set DF By parallel ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setDFByShiftZero(double width, FloorType type, double floor)
{
	unsigned int i;

	LAObjectHolder objHolder = getYieldData().get();
	const LAObject& YieldData = objHolder.get();
	const LADataDoubles& terms = dynamic_cast<const LADataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get());
	
	RateConvention rc = setRC(getFrequency().get());
	LAPriceDataConvention conv(getDayCount().getDayCount(), rc);
	
	LADataDoubles rates;
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
			rnew = LAMath::max(rates[i] + width, floor); 
			if(width < 0 && rnew > rates[i]) rnew = rates[i];
			break;
		case SYMMETRIC:
            if (width < 0)
				rnew = LAMath::min(rates[i], LAMath::max(rates[i] + width, floor));
			else
			{
				rnew = LAMath::min(rates[i], LAMath::max(rates[i] - width, floor));
				rnew = 2 * rates[i] - rnew; 
			}
			break;
		default:
			throw LACoreInvalidData("Not Supported Floor Type", __FILE__, __LINE__);		
		}
		rates.set(1 / LAPriceDataConvention::rateToRet(rnew, terms[i], conv) ,i);
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new LADataDoubles(rates));
}

/*!
    @brief set BasisDF By ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setBasisDFByShiftZero(double width, unsigned int pos, const UintArray& grids)
{
	LAObjectHolder &eh_y = getYieldData().get();
	const LADataDoubles& terms = dynamic_cast<const LADataDoubles&> (eh_y.getData(CALIBRATION_DATA_TERMS + mCurveSuffix , ISNOTNULL).get());
	
	RateConvention rc = setRC(getFrequency().get());
	LAPriceDataConvention conv(getDayCount().getDayCount(), rc);

	const unsigned int termSize = terms.getSize();
	LADataDoubles rates;
	rates.resize(termSize);
	for (unsigned int i = 0; i < termSize ; ++i)
	{
		if (terms[i] >= EPS)
		{
			// calc basis zero rate
			const double dff = 1.0;
			const double dft = getBasisDF(terms[i]);
			rates.set(LAPriceDataConvention::retToRate(dff / dft, terms[i], conv), i);
		}
		else
		{
			rates.set(0.0, i);
		}
	}
	
	if (pos > grids.size() - 1 || grids[grids.size() - 1] > terms.getSize() - 1)
	{
		// error
		LAString msg = getName();
		msg += " : Input pos or grids are something wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
		const double val = LAPriceDataConvention::rateToRet(rates[i], terms[i], conv);
		/*if (val < 1.0)
		{
			throw LACoreInvalidData("DF is over one.", __FILE__, __LINE__);
		}*/
		rates.set(1.0 / val, i);
	}
	
	eh_y.remove(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix);
	eh_y.add(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix, new LADataDoubles(rates));
}

/*!
    @brief set BasisDF By parallel ShiftZero

	@param[in] width width of Shift Zero

*/
void
LAMathYieldCurvePro::setBasisDFByShiftZero(double width, FloorType type, double floor)
{
	LAObjectHolder &eh_y = getYieldData().get();
	const LADataDoubles& terms = dynamic_cast<const LADataDoubles&> (eh_y.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL).get());
	
	RateConvention rc = setRC(getFrequency().get());
	LAPriceDataConvention conv(getDayCount().getDayCount(), rc);
	
	const unsigned int termSize = terms.getSize();
	LADataDoubles rates;
	rates.resize(terms.getSize());
	
	for (unsigned int i = 0; i < termSize ; ++i)
	{
		if (terms[i] >= EPS)
		{
			// calc basis zero rate
			const double dff = 1.0;
			const double dft = getBasisDF(terms[i]);
			rates.set(LAPriceDataConvention::retToRate(dff / dft, terms[i], conv), i);
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
			rnew = LAMath::max(rates[i] + width, floor); 
			if (width < 0.0 && rnew > rates[i]) 
			{
				rnew = rates[i];
			}
			break;
		case SYMMETRIC:
            if (width < 0.0)
			{
				rnew = LAMath::min(rates[i], LAMath::max(rates[i] + width, floor));
			}
			else
			{
				rnew = LAMath::min(rates[i], LAMath::max(rates[i] - width, floor));
				rnew = 2.0 * rates[i] - rnew; 
			}
			break;
		default:
			throw LACoreInvalidData("Not Supported Floor Type", __FILE__, __LINE__);		
		}
		const double val = LAPriceDataConvention::rateToRet(rnew, terms[i], conv);
		/*if (val < 1.0)
		{
			throw LACoreInvalidData("DF is over one.", __FILE__, __LINE__);
		}*/

		rates.set(1.0 / val, i);
	}
	eh_y.remove(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix);
	eh_y.add(IR_CALIBRATION_DATA_DFS2 + mCurveSuffix, new LADataDoubles(rates));
}

/*!
    @brief set basis rates to data

	@param[in] values

*/
void 
LAMathYieldCurvePro::setBasisRates(const DoubleMatrix& values)
{	
	LADataDoubleMatrix& attrvalue = dynamic_cast<LADataDoubleMatrix&>(mpBasisRates->get()); 
	attrvalue.set(values);
}

/*!
    @brief calc basis rates
	@param[in] curveType

*/
void
LAMathYieldCurvePro::setBasisRates(const LAString &curveType)
{
	LADataHolder *dh = 0;
	dh = &getData(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS,NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector& nonvec = dynamic_cast<LADataStrings &>(dh->get()).get();
		if (nonvec.end() != std::find(nonvec.begin(),nonvec.end(),curveType))
		{	
			// set generate flag true
			for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
			{
				if (it->second == mAssignedCurveMktMap[curveType])
				{
					const LAString& curveName = it->first;
					LAString suffix = "_" + curveName;
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

	LAStringVector gCurveNames;
	dh = &getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	}

	LAString suffix = "_" + mAssignedCurveMktMap[curveType];
	// get market data
	const LADataMultiReference* mr_ = &getMarketDataRef(mAssignedCurveMktMap[curveType]);
	vector<LAObject*> data_;
	vector<LAObject*> data_fwd;
	for(unsigned int i = 0; i < mr_->getSize(); i++)
	{
		// check use grid
		const LADataHolder *dh = &mr_->get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const LADataBool &>(dh->get()).get()) continue;
		
        const LAString &dataType = dynamic_cast<const LADataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		LAString tmpType = dataType;
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
	std::vector<LAString> addtionalCalibGrid;
	dh = &this->getData(IR_CALIBRATION_DATA_ADDITIONALCALIBGRID + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		addtionalCalibGrid = dynamic_cast<const LADataStrings&>(dh->get()).get();
	}
	const LAInterpolationBase* pSpreadInter;
	dh = &this->LAObject::getData(IR_CALIBRATION_DATA_INTERPOLATIONBS + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		pSpreadInter = &dynamic_cast<const LAPriceDataInterpolation&>(dh->get()).getMethod();
	}

	// generate addtional market data through interpolating basis spreads
	std::vector<LAObject*> data_addtional;
	if (addtionalCalibGrid.size() > 0)
	{
		const LAPriceDataDayCount &c_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		const LAPriceDataCalendar &c_cal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		const LAPriceDataSlidingRule &c_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		const LADate &c_spotdate = dynamic_cast<const LADataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
		const LAString c_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();

		bool eom = false;
		dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			eom = dynamic_cast<const LADataBool &>(dh->get()).get();
		}
		LAString roll_conv_ts = ROLLCONV_NORMAL;
		if (c_freq == LUNAR) roll_conv_ts = ROLLCONV_LUNAR;
		else if (eom) roll_conv_ts = ROLLCONV_EOM;

		const bool isTimeInter = dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL).get()).get();

		for (unsigned int i = 0; i < addtionalCalibGrid.size(); ++i)
		{
			data_addtional.push_back(data_[0]->clone());
			data_addtional[i]->remove(IR_CALIBRATION_DATA_TERM);
			data_addtional[i]->add(IR_CALIBRATION_DATA_TERM, new LADataString(addtionalCalibGrid[i]));
			
			LADate tmpDate = LAMathDateCalculations::getDate(c_spotdate, addtionalCalibGrid[i], c_sld, &c_cal, true, &roll_conv_ts);
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
			data_addtional[i]->add(CALIBRATION_DATA_RATE, new LADataDouble(rate));

			data_.push_back(data_addtional[i]);
		}
	}

	int fwd_size = data_fwd.size();
	bool fwd_isonly = false;
	// data exist check
	if (data_.empty() && data_fwd.empty())
	{
		LAString msg = "CurveName = "+ curveType + ", basis/fwdfx data is not set.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
			throw LACoreInvalidData("ForwardFX Term must be smaller than CCS Term", __FILE__, __LINE__);
		}
	}

	// foreign currency flag
	bool isForeignCcyLeg = false;
	dh =  &getData(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isForeignCcyLeg = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	// 2-swap?
	dh = &data_[0]->getData(IR_CALIBRATION_DATA_IS2SWAP, NOCHECK);
	const bool is2Swap = (dh->isDefined() && !dh->isNull()) && dynamic_cast<const LADataBool &>(dh->get()).get();

	// get first element val
	// curve info
	// setBasisRates by recursive
	
	bool isBasisEnabled = false;
	dh = &getData(IR_CALIBRATION_DATA_ENABLECALCULATION, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		isBasisEnabled = dynamic_cast<const LADataBool &>(dh->get()).get();

	const LAString &fCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_FORECAST, ISNOTNULL).get()).get();
	if ((fCurve != STD) && (fCurve != DUMMY) && (fCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), fCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		setBasisRates(fCurve);
#endif
	}
	const LAString &dCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_DISCOUNT, ISNOTNULL).get()).get();
	if ((dCurve != STD) && (dCurve != DUMMY) && (dCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), dCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		setBasisRates(dCurve);
#endif
	}
	const LAString &a_fCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST , ISNOTNULL).get()).get();
	if ((a_fCurve != STD) && (a_fCurve != DUMMY) && (a_fCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), a_fCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		if (!isForeignCcyLeg) setBasisRates(a_fCurve);
#endif
	}
	const LAString &a_dCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL).get()).get();
	if ((a_dCurve != STD) && (a_dCurve != DUMMY) && (a_dCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), a_dCurve)) &&!isBasisEnabled)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		if (!isForeignCcyLeg) setBasisRates(a_dCurve);
#endif
	}
	const LAString *f_dCurve = 0;
	if (is2Swap)
	{
		f_dCurve = &dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_DFCURVENAME, ISNOTNULL).get()).get();
	}
	// check Dummy
	if (fCurve == DUMMY || dCurve == DUMMY || (f_dCurve && (*f_dCurve == DUMMY)))
	{
		throw LACoreInvalidData("Dummy curve must be used for against curve only.", __FILE__, __LINE__);
	}
	if ((a_fCurve == DUMMY && a_dCurve != DUMMY) || (a_fCurve != DUMMY && a_dCurve == DUMMY))
	{
		throw LACoreInvalidData("If DUMMY curve is used, both forecast and discount must be DUMMY curve.", __FILE__, __LINE__);
	}
	//// check FixedRate
	//if (a_fCurve == FIXEDRATE || a_dCurve == FIXEDRATE && (f_dCurve && (*f_dCurve == FIXEDRATE)))
	//{
	//	throw LACoreInvalidData("FIXEDRATE must be used for target curve only.", __FILE__, __LINE__);
	//}
	//if ((fCurve == FIXEDRATE && dCurve != FIXEDRATE) || (fCurve != FIXEDRATE && dCurve == FIXEDRATE))
	//{
	//	throw LACoreInvalidData("If FIXEDRATE is used, both forecast and discount must be FIXEDRATE.", __FILE__, __LINE__);
	//}

	const bool isDiscount =  dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
	const bool isTimeInter  = dynamic_cast<const LADataBool &>((data_[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL)).get()).get();
	const bool isAgtSpread =  dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();
	// market convention info
	// cashlet
	const LAPriceDataDayCount &c_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &c_cal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &c_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
	const LADate &c_spotdate = dynamic_cast<const LADataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
	LAString c_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
	const int c_lag  = dynamic_cast<const LADataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());
	// index
	const LAPriceDataDayCount &i_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &i_fixcal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
	const LAPriceDataCalendar &i_paycal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &i_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
	const int i_lag  = dynamic_cast<const LADataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
	LAString i_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	LAString i_accessary = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

	if (!isDiscount)
	{
		// if forecast only
		i_freq.toUpper();
		if (i_freq != SIMPLE)
		{
			throw LACoreInvalidData("If forecast mode, only simple is possible in frequency.", __FILE__, __LINE__);
		}
		// if ForeignCcy
		if (isForeignCcyLeg)
		{
			throw LACoreInvalidData("If it has foreign ccy leg, target must be discount curve.", __FILE__, __LINE__);
		}
		//// if FixedRate
		//if (fCurve == FIXEDRATE)
		//{
		//	throw LACoreInvalidData("If FIXEDRATE is used, target must be discount curve.", __FILE__, __LINE__);
		//}
	}

	// against cashlet
	const LAPriceDataDayCount &a_c_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &a_c_cal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &a_c_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, ISNOTNULL).get());
	const LADate &a_c_spotdate = dynamic_cast<const LADataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, ISNOTNULL).get());
	LAString a_c_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL).get()).get();
	const bool c_isBackward = dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETISBACKWARD, ISNOTNULL).get());
	// against index
	const LAPriceDataDayCount &a_i_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &a_i_fixcal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, ISNOTNULL).get());
	const LAPriceDataCalendar &a_i_paycal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &a_i_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, ISNOTNULL).get());
	const int a_i_lag  = dynamic_cast<const LADataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, ISNOTNULL).get());
	LAString a_i_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, ISNOTNULL).get()).get();
	LAString a_i_accessary = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, ISNOTNULL).get()).get();
	const bool a_c_isBackward = dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETISBACKWARD, ISNOTNULL).get());

	// simultaneous equation
	bool isSimuEq = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSimuEq = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	// issamegridindex
	bool isSameGridIndex = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSameGridIndex = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	// isoddtermfrnindex
	bool isOddTermFRNIndex = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISODDTERMFRNINDEX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isOddTermFRNIndex = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	// isyieldspreadcalc
	bool isYieldSpreadCalc = true;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISYIELDSPREADCALC, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isYieldSpreadCalc = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	if (isForeignCcyLeg && isYieldSpreadCalc)
	{
		throw LACoreInvalidData("yield spread calc flag must be false.", __FILE__, __LINE__);
	}
	//iseomroll
	bool eom = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eom = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	// isfwdinterpolation
	bool isFWDInter = false;
	LAPriceDataInterpolation *a_fwdInter = 0;
	LAPriceDataInterpolation *fwdInter = 0;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
		a_fwdInter = &dynamic_cast<LAPriceDataInterpolation &>(data_[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		if (isDiscount)
			fwdInter = dynamic_cast<LAPriceDataInterpolation *>(a_fwdInter->clone());
	}
	// compounding	
	LAString c_freq_cpd(c_freq), a_c_freq_cpd(a_c_freq);	
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, NOCHECK));	
	if (dh->isDefined() && !dh->isNull())	
	{	
		c_freq_cpd = dynamic_cast<const LADataString &>(dh->get()).get();
		LADate period_reset(LAMathDateCalculations::getDate(c_spotdate, FrequencyToTerm(c_freq), true));
		LADate period_payment(LAMathDateCalculations::getDate(c_spotdate, FrequencyToTerm(c_freq_cpd), true));
		if (period_reset > period_payment)
		{
			throw LACoreInvalidData("Payment frequency must be wider than reset frequency.", __FILE__, __LINE__);
		}
	}	
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, NOCHECK));	
	if (dh->isDefined() && !dh->isNull())	
	{	
		a_c_freq_cpd = dynamic_cast<const LADataString &>(dh->get()).get();
		LADate period_reset(LAMathDateCalculations::getDate(a_c_spotdate, FrequencyToTerm(a_c_freq), true));
		LADate period_payment(LAMathDateCalculations::getDate(a_c_spotdate, FrequencyToTerm(a_c_freq_cpd), true));
		if (period_reset > period_payment)
		{
			throw LACoreInvalidData("Payment frequency must be wider than reset frequency.", __FILE__, __LINE__);
		}
	}
	if (!mpCompoundFunction->isDefined() || mpCompoundFunction->isNull())
	{
		throw LACoreInvalidData("no valid compounding function is set.", __FILE__, __LINE__);
	}

	// get max term
	const LAString &termMax = dynamic_cast<const LADataString &>((data_.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
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
		LAString msg = "frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	a_c_freq_cpd.toUpper();
	unsigned int a_mUnit = LAMathDateCalculations::getPeriodFrequencyInMonths(a_c_freq_cpd);

	if ((c_freq == LUNAR && a_c_freq != LUNAR) || (c_freq != LUNAR && a_c_freq == LUNAR))
	{
		LAString msg = "if the one frequency is LUNAR, the other must be LUNAR";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
	LAString roll_conv(LAPriceYieldGenerator::deduceRollConvention(c_freq, eom));

	// fixed cashlet in 2-swap
	const LAPriceDataCalendar *f_cal = 0;
	const LADate *f_spotdate = 0;
	bool f_isBackward = false;
	bool f_eom = false;
	if (is2Swap)
	{
		f_cal = dynamic_cast<const LAPriceDataCalendar *>(&data_[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		f_spotdate = &dynamic_cast<const LADataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get()).get();
		f_eom = dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, ISNOTNULL).get()).get();
		if (isAgtSpread)
		{
			f_isBackward = a_c_isBackward;
		}
		else
		{
			f_isBackward = c_isBackward;
		}
	}

	LAObjectHolder &objHolder = getYieldData().get();
	LAObject &yieldData = objHolder.get();

	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const LADate &asof = dynamic_cast<const LADataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());	

	// is xccy swap marked to market or not
	bool isXccyMarkedToMarket = dynamic_cast<const LADataBool& > ((getData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST,
															               IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, ISNOTNULL)).get()); // Alias Method: First Parameter Takes Priority
	bool isUSD = false;
	LAString currency = dynamic_cast<LADataString&> ((yieldData.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
	if (currency.toUpper() == "USD") isUSD = true;

	LAPriceDataDayCount dc_act(ACT_365_ISDA);
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
		std_terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
		std_dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL)).get()).get();
		const LADataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			is_std_JoinDateExists = true;
			std_linearSplineJoinDateAsDouble = dynamic_cast<const LADataDouble&>(dh->get()).get();
		}
	}
	LAPriceDataInterpolation f_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
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
		const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + LAString("_") + fCurve, ISNOTNULL)).get()).get();
		const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + LAString("_") + fCurve, ISNOTNULL)).get()).get();
		const LADataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + LAString("_") + fCurve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			f_inter.setJoinDateAsDouble(dynamic_cast<const LADataDouble&>(dh->get()).get());
		}
		f_inter.set(terms, dfs);
		if (!isDiscount)
		{
			_terms = terms;
		}
	}
	LAPriceDataInterpolation d_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
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
		const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + LAString("_") + dCurve, ISNOTNULL)).get()).get();
		const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + LAString("_") + dCurve, ISNOTNULL)).get()).get();
		const LADataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + LAString("_") + dCurve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			d_inter.setJoinDateAsDouble(dynamic_cast<const LADataDouble&>(dh->get()).get());
		}
		d_inter.set(terms, dfs);
		if (isDiscount)
		{
			_terms = terms;
		}
	}

	// against side
	LAPriceDataInterpolation a_f_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (isForeignCcyLeg)
	{
		LAObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{	
			throw LACoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			LAObject &fYieldData = eh_fy.get();
			LAString suffix;
			if (a_fCurve != STD) suffix = LAString("_") +  a_fCurve;
			const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((fYieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((fYieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
			const LADataHolder* dh = &fYieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_f_inter.setJoinDateAsDouble(dynamic_cast<const LADataDouble&>(dh->get()).get());
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
			const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + LAString("_") +  a_fCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + LAString("_") +  a_fCurve, ISNOTNULL)).get()).get();
			const LADataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + LAString("_") + a_fCurve, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_f_inter.setJoinDateAsDouble(dynamic_cast<const LADataDouble&>(dh->get()).get());
			}
			a_f_inter.set(terms, dfs);
		}
	}
	LAPriceDataInterpolation a_d_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (isForeignCcyLeg)
	{
		LAObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{	
			throw LACoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			LAObject &fYieldData = eh_fy.get();
			LAString suffix;
			if (a_dCurve != STD) suffix = LAString("_") +  a_dCurve;
			const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((fYieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((fYieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
			const LADataHolder* dh = &fYieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_d_inter.setJoinDateAsDouble(dynamic_cast<const LADataDouble&>(dh->get()).get());
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
			const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + LAString("_") +  a_dCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + LAString("_") + a_dCurve, ISNOTNULL)).get()).get();
			const LADataHolder* dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + LAString("_") + a_dCurve, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				a_d_inter.setJoinDateAsDouble(dynamic_cast<const LADataDouble&>(dh->get()).get());
			}
			a_d_inter.set(terms, dfs);
		}
	}
	std::shared_ptr<LAPriceDataInterpolation> f_d_inter;
	if (is2Swap)
	{
		f_d_inter.reset(new LAPriceDataInterpolation(dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())));
		if (*f_dCurve == STD)
		{
			f_d_inter->set(std_terms, std_dfs);
		}
		else
		{
			const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + LAString("_") + *f_dCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + LAString("_") + *f_dCurve, ISNOTNULL)).get()).get();
			f_d_inter->set(terms, dfs);
		}
	}

	const unsigned int max = m / mUnit + 1;
	LADate fdate = c_spotdate;
	LADate a_fdate = a_c_spotdate;
	LAString strUnit = LAString(static_cast<int>(mUnit)) + LAString("M");

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
	LAString maxTerm, maxFreq;
	DoubleArray extra_terms;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		maxTerm = dynamic_cast<LADataString&>(dh->get()).get();
		maxTerm += "Y";
		maxFreq = dynamic_cast<LADataString&>(objHolder.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
		const LADate& maxDate = LAMathDateCalculations::getDate(c_spotdate, maxTerm, c_sld, &c_cal, true, &roll_conv);
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
		const bool isFXForwardQuotedAsOutright = dynamic_cast<const LADataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISRATIO, IR_CALIBRATION_DATA_ISFXOUTRIGHT, ISNOTNULL).get()).get();
		const LAPriceDataCalendar &fwd_cal = dynamic_cast<const LAPriceDataCalendar &>(data_fwd[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const LAPriceDataSlidingRule &fwd_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data_fwd[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const LADate &fwd_spotdate = dynamic_cast<const LADataDate &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
		const int fwd_spotlag = dynamic_cast<const LADataInt &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get()).get();
		fwd_spotTerm = dc_act.getTerm(asof, fwd_spotdate);
		fwd_spotdf = d_inter.value(fwd_spotTerm);
		LAString fwd_freq = dynamic_cast<const LADataString &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
		const bool fwd_eom = dynamic_cast<const LADataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL).get()).get();

		// set roll convention
		LAString fwd_roll_conv("");
		if (fwd_freq == LUNAR) fwd_roll_conv = ROLLCONV_LUNAR;
		else if (fwd_eom) fwd_roll_conv = ROLLCONV_EOM;
		else fwd_roll_conv = ROLLCONV_NORMAL;

		// calc extrapolation terms for fwdfx
		if (fwd_isonly && extra_terms.size() > 0)
		{
			extra_terms.clear();
			const LADate& maxDate = LAMathDateCalculations::getDate(fwd_spotdate, maxTerm, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
			DateVector tmp_dates; DoubleArray tmp_taus;
			LAPriceYieldGenerator::getPaymentDates(fwd_spotdate, maxDate, maxFreq, fwd_cal, fwd_sld, dc_act, tmp_dates, extra_terms, tmp_taus, fwd_eom);
		}

		double term = 0.0;
		double df = 1.0;
		LADate end;

		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if ( isFXForwardQuotedAsOutright )
		{
			double fwd_ratio_pow = 1.0;
			for (int i = 0; i < fwd_size; i++)
			{
				const double fwd_ratio = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const LAString &fwd_termStr  = dynamic_cast<const LADataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
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
			const bool fwd_ispriceccy = dynamic_cast<const LADataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISPRICECCY,
																							  IR_CALIBRATION_DATA_ISDOMESTICCURRENCY, ISNOTNULL).get()).get();
			//get spot fx
			const LAMathFXEntity& fx = dynamic_cast<const LAMathFXEntity&>(getFXEntity().get().get());
			const LAStringVector spotccys = fx.getCurrencys().get();
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
				throw LACoreInvalidData("SpotFX rates do not exist!", __FILE__, __LINE__); 
			}
			const double spotfx = fwd_ispriceccy ? spotfx_usd_unitccy / spotfx_unitccy : spotfx_unitccy / spotfx_usd_unitccy;
			//search ON&TN fwd spread
			double fwd_spread_on = 0.0;
			double fwd_spread_tn = 0.0;
			for (int i = 0; i < fwd_size; i++)
			{
				const LAString &termStr = dynamic_cast<const LADataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				if (termStr == "ON")
					fwd_spread_on = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				else if (termStr == "TN")
					fwd_spread_tn = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				else if (fwd_spread_on != 0.0 && fwd_spread_tn != 0.0)
					break;
			}
			const double fwd_fx_on = fwd_spotlag != 1 ? spotfx - fwd_spread_tn - fwd_spread_on : spotfx - fwd_spread_on;
			const double fwd_fx_tn = spotfx - fwd_spread_tn;
			for (int i = 0; i < fwd_size; i++)
			{
				const LAString &fwd_termStr  = dynamic_cast<const LADataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
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
					const double fwd_spread = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
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
			throw LACoreInvalidData("Basis data is empty.", __FILE__, __LINE__);
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

		LAPriceDataInterpolation spread_time_inter = getInterpolation_yg();
		DoubleArray grid_spread_time((fwd_size - fwd_size_mm) + b_size, 0.0);
		DoubleArray spread_timeVec((fwd_size - fwd_size_mm) + b_size, 0.0);
		double linearSplineJoinDateAsDouble_spread_time_inter = 0.;
		spread_time_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble_spread_time_inter);

		if (fwd_terms.size() > 0 && fwd_spotTerm != spotTerm)
		{
			LAPriceDataInterpolation tmp_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
			tmp_inter.set(fwd_terms, fwd_dfs);
			fwd_spotdf = tmp_inter.value(spotTerm);//to adjust for xccy basis spot
		}
		for (int i = fwd_size_mm; i < fwd_size; i++)
		{
			grid_spread_time[i - fwd_size_mm] = fwd_terms[i] - spotTerm;
			spread_timeVec[i - fwd_size_mm] = -LAMath::log(fwd_dfs[i] / fwd_spotdf);
		}

		LAString a_fMarket = getMarketForCurve(a_fCurve);
		LAString a_dMarket = getMarketForCurve(a_dCurve);
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
			LAPriceDataDayCount dc;
			LAPriceDataCalendar cal;
			LAPriceDataSlidingRule sld;
			LAString accessory;
			LAString freq;
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
					LAString fYieldDataName = dynamic_cast<const LADataString &>(getForeignYieldData().get().get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					LADataInstance* dataInstance = getDataInstance();
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
			double spread = dynamic_cast<const LADataDouble &>((data_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();


			const LAString &strTerm  = dynamic_cast<const LADataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

			if (is2Swap)
			{
				const LAPriceDataDayCount *f_dc = dynamic_cast<const LAPriceDataDayCount *>(&data_[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
				const LAString *f_freq = &dynamic_cast<const LADataString &>(data_[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
				const LAPriceDataSlidingRule *f_sld = dynamic_cast<const LAPriceDataSlidingRule *>(&data_[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());

				// Target (no 2-swap-based spread)
				double t_annuity = 0.0;
				if (isAgtSpread)
				{
					const LAString a_roll_conv = LAPriceYieldGenerator::deduceRollConvention(a_c_freq, eom);
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
				const LAString o_roll_conv = LAPriceYieldGenerator::deduceRollConvention(*f_freq, f_eom);
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
				LADate fDate = a_c_spotdate;

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
			LADate fDate = c_spotdate;
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
						b_yieldTimeMat[j][1] = -LAMath::log(d_inter.value(terms_grid[j] + spotTerm) / d_spotdf);
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
							b_yieldTimeVec[k] = -LAMath::log(df / f_spotdf);
						}
						const double df1 = f_inter.value(i_gridVec[0]);
						b_yieldTimeVec[0] = -LAMath::log(df1 / f_spotdf) ;
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
			spread_timeVec[(fwd_size - fwd_size_mm) + i] = -LAMath::log(dfs.back());
		}

		if (!isDiscount)
		{
			dh = &(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATETERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const LAString &strTerm_s = dynamic_cast<const LADataString &>(dh->get()).get();
				const LAString &strTerm  = dynamic_cast<const LADataString &>((data_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				LADate date_s = LAMathDateCalculations::getDate(asof, strTerm_s, true);
				LADate date = LAMathDateCalculations::getDate(asof, strTerm, true);
				
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
							b_yieldTimeVec[k] = -LAMath::log(df / f_spotdf);
						}
						const double df1 = f_inter.value(i_gridVec[0]);
						b_yieldTimeVec[0] = -LAMath::log(df1 / f_spotdf);
					}
					
					DoubleMatrix b_yieldTimeMat;
					b_yieldTimeMat.push_back(b_yieldTimeVec);

					const double term = gridVec.back().front();
					const double tau = tauVec.back().front();
					const double df = d_inter.value(term + spotTerm) / d_spotdf;

					const double spotRate = dynamic_cast<const LADataDouble &>(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATE, ISNOTNULL).get()).get();
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
					spread_timeVec.insert(spread_timeVec.begin(), isYieldSpreadCalc ? 0. : -LAMath::log(df));

					spreadVec.insert(spreadVec.begin(), 0.0);
					a_targetPVVec.insert(a_targetPVVec.begin(), targetPV);

					++b_size;
				}
				else if (date_s == date)
				{
					if (gridVec[0].size() != 1)
					{
						throw LACoreInvalidData("basis first term, cashlet size must be one", __FILE__, __LINE__);
					}
					const double term = gridVec[0][0];
					const double tau = tauVec[0][0];
					const double df = d_inter.value(term + spotTerm) / d_spotdf;

					const double spotRate = dynamic_cast<const LADataDouble &>(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATE, ISNOTNULL).get()).get();
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
						LADate fDate = a_c_spotdate;

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
							if (LAMath::abs(val0 - val1) < EPS_PV)
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

					dynamic_cast<LADataDouble &>(data_[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).set(spreadVec[0]);
					data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATE);
					data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATETERM);
				}
				else
				{
					LAString msg = "spotrateterm is after basis first term. spotrateterm = " + strTerm_s + ", basis first term = " + strTerm;
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
		LAPriceDataInterpolation* adjust_inter = NULL;
		if (isDiscount && isXccyMarkedToMarket)
		{
			dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DoubleArray& adjustValue_term = dynamic_cast<const LADataDoubles&> (dh->get()).get();
				const DoubleArray& adjustValue
					= dynamic_cast<const LADataDoubles&> ((data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL)).get()).get();
				adjust_inter = &(dynamic_cast<LAPriceDataInterpolation&> (data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get()));
				adjust_inter->set(adjustValue_term, adjustValue);
			}
		}

		LAString OptimizationMethod = dynamic_cast<LADataString&>(mpOptimizeMethod->get()).get();
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
				if (LAMath::abs(targetValVec0[i] - targetValVec1[i]) >= EPS_PV)
				{
					isEnd = false;
					break;
				}
			}
			if (isEnd)
			{
				break;
			}

			LAMatrix divMat(b_size, b_size);
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
						throw LACoreInvalidData("size error!", __FILE__, __LINE__);
					}
					const double divVal = (vals[j] + targetValVec1[j]) / MIN_VAL;
					divMat.setValue(j, i, divVal);
				}
			}

			LAMatrix invMat = divMat.inverseMatrix();
			LAMatrix valMat(targetValVec1);
			LAMatrix deltaMat = invMat * valMat;
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
				if (LAMath::abs(targetValVec1[i]) > 1.0E-9)
				{
					throw LACoreInvalidData("Convergence error in DF calc (Newton Raphson)", __FILE__, __LINE__); 
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
				throw LACoreInvalidData("Convergence error in DF calc (NL2SOL)", __FILE__, __LINE__); 
			}

			spread_timeVec.clear();
			spread_timeVec.push_back(0.);
			spread_timeVec.insert(spread_timeVec.end(), val.begin(), val.end());
			spread_time_inter.set(grid_spread_time, spread_timeVec);

		}
		else
		{
			LAString msg = LAString("Optimize method ") + OptimizationMethod + " is not supported.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__); 
		}

		LAPriceDataInterpolation *p_base_inter = 0;
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
					throw LACoreInvalidData("coupon reset time is not consistent to fixing timing.", __FILE__, __LINE__);
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
			throw LACoreInvalidData("term is empty", __FILE__, __LINE__); 
		}

		DoubleArray terms_mod_fwdmatend = terms_mod;
		for (unsigned int i = 0; i < data_.size(); ++i)
		{
			const LAString &strTerm = dynamic_cast<const LADataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
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
				LAString strTerm_add(strTerm_int);
				strTerm_add += LAString("M");

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
				LAString strTerm_add(strTerm_int);
				strTerm_add += LAString("M");

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
				const double b_spread_time = -LAMath::log(p_base_inter->value(terms_mod[i] + spotTerm) / b_spotdf);
				dfs[i] = LAMath::exp(-(b_spread_time + y_spread_time));
			}
		}
		else
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				dfs[i] = LAMath::exp(-spread_time_inter.value(terms_mod[i]));
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
					spotdf = LAMath::exp((-spread_timeVec[1] / grid_spread_time[1]) * spotTerm) * b_spotdf;
				}
				else
				{
					spotdf = LAMath::exp((-spread_timeVec[1] / grid_spread_time[1]) * spotTerm);
				}
				yield_spot = -LAMath::log(spotdf) / spotTerm;
			}
			else
			{
				LAPriceDataInterpolation tmp_inter = *p_base_inter;
				DoubleArray tmp_yields(dfs.size());
				for (unsigned int i = 0; i < terms_mod.size(); ++i)
				{
					tmp_yields[i] = -LAMath::log(dfs[i]) / terms_mod[i];
				}
				tmp_inter.set(terms_mod, tmp_yields);
				yield_spot = tmp_inter.value(spotTerm);
				spotdf = LAMath::exp(-yield_spot * spotTerm);
			}
			
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				terms_mod[i] += spotTerm;
				terms_mod_fwdmatend[i] += spotTerm;
				dfs[i] *= spotdf;
				yield_mod[i] =  -LAMath::log(dfs[i]) / terms_mod[i];
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
						yield_mod.insert(yield_mod.begin(), -LAMath::log(fwd_dfs[i]) / fwd_terms[i]);
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
								yield_mod.insert(yield_mod.begin(), -LAMath::log(fwd_dfs[i]) / fwd_terms[i]);
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
								yield_mod.insert(yield_mod.begin() + k, -LAMath::log(fwd_dfs[i]) / fwd_terms[i]);
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
				yield_mod[i] =  -LAMath::log(dfs[i]) / terms_mod[i];
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
		throw LACoreInvalidData("We support Simultaneous-Equation method only.", __FILE__, __LINE__); 
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
	//		throw LACoreInvalidData("terms error", __FILE__,__LINE__);
	//	const DoubleArray &terms = dynamic_cast<const LADataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	//	// maxterm for calc
	//	const double tmax = terms.back();

	//	// get interpolation method
	//	LAPriceDataInterpolation &b_inter = getInterpolation_bs();
	//	DoubleArray b_t_grid(1, 0.0);
	//	DoubleArray b_termstruct_grid(1, 0.0);
	//	for (unsigned int i = 0; i < data_.size(); i++)
	//	{
	//		const LAString &strTerm  = dynamic_cast<const LADataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//		double rate = dynamic_cast<const LADataDouble &>((data_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	//		double term = 0.0;
	//		if (isAgtSpread)
	//		{
	//			LADate tmpDate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm, a_c_sld, &a_c_cal, true, &a_c_roll_conv);
	//			term = a_c_dc.getTerm(a_c_spotdate, tmpDate);
	//		}
	//		else
	//		{
	//			LADate tmpDate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &c_roll_conv);
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
	//		LAString strTerm = LAString(static_cast<int>(mUnit * (i - 1))) + LAString("M");
	//
	//		double target_pv = 1.0;
	//		if (a_fCurve != DUMMY)
	//		{
	//			double spread = 0.0;
	//			if (isAgtSpread)
	//			{
	//				LAString a_dfstrTerm = LAString(static_cast<int>(mUnit * (i - 1) + a_mUnit * a_step)) + LAString("M");
	//				LADate a_dfdate = LAMathDateCalculations::getDate(a_c_spotdate, a_dfstrTerm, a_c_sld, &a_c_cal, true);
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
	//			LADate a_fdate = LAMathDateCalculations::getDate(a_c_spotdate, strTerm, a_c_sld, &a_c_cal, true);
	//			LADate a_ldate;
	//			double a_lterm = 0.0;
	//			double a_d_df = 1.0;
	//			for (unsigned int j = 0; j < a_step; ++j)
	//			{
	//				// calc cash flow
	//				LAString a_strTerm_ = LAString(static_cast<int>(mUnit * (i - 1) + a_mUnit * (j + 1))) + LAString("M");
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
	//		LADate ldate;		
	//		double val0 = 0.0;
	//		// for discount df
	//		if (isDiscount)
	//		{
	//			LAString dfstrTerm = LAString(static_cast<int>(mUnit * (i - 1) + mUnit * step)) + LAString("M");
	//			LADate dfdate;
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
	//				LADate dfdate_ = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true);
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
	//				LAString strTerm_ = LAString(static_cast<int>(mUnit * (i - 1) + mUnit * (j + 1))) + LAString("M");
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
	//				if (LAMath::abs(val0 - val1) < EPS_PV)
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
	//				throw LACoreInvalidData("Convergence error in Basis calc (Newton Raphson)", __FILE__, __LINE__); 
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
	//			LALinearInterpolation tmpInter;
	//			DoubleVector delta_dfVec;
	//			double d_ldf = 1.0;
	//			DoubleMatrix i_gridMtx;
	//			DoubleMatrix i_termMtx;
	//			DoubleMatrix alphaMtx;
	//
	//			LAString dfstrTerm = LAString(static_cast<int>(mUnit * (i - 1) + mUnit * (step - 1))) + LAString("M");
	//			LADate dfdate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true);
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
	//				LAString dfstrTerm_ = LAString(static_cast<int>(mUnit * (i - 1) + mUnit * step)) + LAString("M");
	//				LADate dfdate_ = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm_, c_sld, &c_cal, true);
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
	//				LAString strTerm_ = LAString(static_cast<int>(mUnit * (i - 1) + mUnit * (j + 1))) + LAString("M");
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
	//				if (LAMath::abs(val0 - val1) < EPS_PV)
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
	//				throw LACoreInvalidData("Convergence error in Basis calc (Newton Raphson)", __FILE__, __LINE__); 
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
	//		const double f_yield = -LAMath::log(df_mod[1]) / b_t[1];
	//		f_yieldSpread = f_yield - (-LAMath::log(d_inter.value(b_t[1] + spotTerm) / d_spotdf) / b_t[1]);
	//		const double d_spotdf_ = LAMath::exp(-f_yieldSpread * spotTerm) * d_spotdf;
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
	//		const double f_yield = -LAMath::log(df_mod[1]) / b_t[1];
	//		f_yieldSpread = f_yield - (-LAMath::log(f_inter.value(b_t[1])) / b_t[1]);
	//		firstTerm = b_t[1];
	//		b_t_2 = b_t;
	//		df_mod_2 = df_mod;
	//	}
	//	LAPriceDataInterpolation *p_base_inter = 0;
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
	//		const double yield = -LAMath::log(b_inter.value(p_time[i])) / p_time[i];
	//		yieldSpread[i] = yield - (-LAMath::log(p_base_inter->value(p_time[i])) / p_time[i]);
	//	}
	//	// merge term
	//	if (dvzeroDates.empty())
	//	{
	//		throw LACoreInvalidData("dvzeroDates is empty", __FILE__, __LINE__);
	//	}
	//	const LADate &firstDate = dvzeroDates[0];
	//	LADate shortDate = firstDate; 
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
	//	LAAlgorithm::locate<DoubleVector, double>(b_t_2, tmp.back(), b_t_2.size(), f_pos);
	//	const double EPS_TERM = 1.0E-4;
	//	if (b_t_2[f_pos] - tmp.back() < EPS_TERM)
	//	{
	//		f_pos++;
	//	}
	//	tmp.insert(tmp.end(), b_t_2.begin() + f_pos, b_t_2.end());
	//	_terms = tmp;
	//	// calc yield
	//	yield_mod.resize(_terms.size(), 0.0);
	//	LAPriceDataInterpolation tmp_inter = b_inter;
	//	tmp_inter.set(p_time, yieldSpread);
	//	for (unsigned int i = 1; i < _terms.size(); i++)
	//	{
	//		if (f_yieldSpread != 0.0 && _terms[i] < firstTerm)
	//		{
	//			double df = p_base_inter->value(_terms[i]);
	//			yield_mod[i] = -LAMath::log(df) / _terms[i];
	//			yield_mod[i] += f_yieldSpread;
	//		}
	//		else
	//		{
	//			const double df = p_base_inter->value(_terms[i]);
	//			if (df <= 0.0)
	//			{
	//				throw LACoreInvalidData("Df is below zero.", __FILE__, __LINE__);
	//			}
	//			const double yield =  -LAMath::log(df) / _terms[i];
	//			const double y_spread = tmp_inter.value(_terms[i]);
	//			yield_mod[i] = yield + y_spread;
	//		}
	//	}
	//}
	//delete a_fwdInter;
	
	// delete artificial market data
	for (unsigned int i = 0; i < data_addtional.size(); ++i)
	{
		const LAString& term_additional = dynamic_cast<LADataString&>(data_addtional[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
		std::vector<LAObject*>::iterator itr = data_.begin();
		while (itr != data_.end())
		{
			const LAString& term = dynamic_cast<LADataString&>((*itr)->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
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
		dfs_mod[i] = LAMath::exp(-_terms[i] * yield_mod[i]);
	}

	DoubleArray::const_iterator min_it = min_element(_terms.begin(), _terms.end());
	if (!_terms.empty() && *min_it < 0.0)
	{
		LAString msg = curveType + " terms, term must be positive.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	min_it = min_element(termsmtx_fwd[0].begin(), termsmtx_fwd[0].end());
	if (!termsmtx_fwd[0].empty() && *min_it < 0.0)
	{
		LAString msg = curveType + " terms_fwd, term must be positive.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//for (unsigned int i = 0; i < data_.size(); i++)
	//{
	//	DateVector ret;

	//	const LAString &strTerm  = dynamic_cast<const LADataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//	if (strTerm == "ON" || strTerm == "TN") continue;
	//	LADate matudate;
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
	//	if (!LAAlgorithm::find<DateVector,LADate>(dvzeroDates,matudate,0,dvzeroDates.size()-1 ,pos))
	//	{
	//		//this means basis grid e.x. 100y is longer than DF1 max grid, which ofcourse can not be calculated.
	//		if (matudate > dvzeroDates.back())
	//			continue;
	//		else
	//			throw LACoreInvalidData("BasisGrid Search Error",__FILE__,__LINE__);
	//	}

	//	ret.insert(ret.end(),dvzeroDates.begin(),dvzeroDates.begin()+pos+1);
	//	if(!data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
	//	{
	//		data_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(ret));
	//	}
	//	else
	//	{
	//		dynamic_cast<LADataDates&>(data_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(ret);
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
	LAString curveType;
	for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); ++it)
	{
		LAString market = it->second;
		if (market.toUpper() == FWDFXCONST)
		{
			curveType = it->first;
			break;
		}
	}
	if (curveType.size() > 0)
	{
		const LADataHolder *dh;
		bool isCheckCurves = false;
		dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const LADataBool &>(dh->get()).get();
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
LAMathYieldCurvePro::calcFwdFXConstantCurve(const LAString &curveType)
{
	// get yield data
	LAObject &yData = getYieldData().get().get(); 
	LAObject &colYData = getColYieldData().get().get();
	// get market data
	const LAString& market = mAssignedCurveMktMap[curveType];
	LAObjectHolder& mktData = getMarketDataRef(market).get(0);

	// get forecast curve
	const LAString &fCurve = dynamic_cast<const LADataString &>(mktData.getData(IR_CALIBRATION_DATA_FORECAST).get()).get();
	LAPriceDataInterpolation f_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (f_inter.isHybrid())
	{
		double linearSplineJoinDateAsDouble = 0.;
		f_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	setCurveInterpolation(fCurve, yData, f_inter);
	// get against forecast curve
	const LAString &a_fCurve = dynamic_cast<const LADataString &>(mktData.getData(IR_CALIBRATION_DATA_AGTFORECAST).get()).get();
	LAPriceDataInterpolation a_f_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (a_f_inter.isHybrid() )
	{
		double linearSplineJoinDateAsDouble = 0.;
		a_f_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	setCurveInterpolation(a_fCurve, colYData, a_f_inter);
	// get against discount curve
	const LAString &a_dCurve = dynamic_cast<const LADataString &>(mktData.getData(IR_CALIBRATION_DATA_AGTDISCOUNT).get()).get();
	LAPriceDataInterpolation a_d_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	if (a_d_inter.isHybrid())
	{
		double linearSplineJoinDateAsDouble = 0.;
		a_d_inter.setJoinDateAsDouble(linearSplineJoinDateAsDouble);
	}
	setCurveInterpolation(a_dCurve, colYData, a_d_inter);

	// calc discount curve
	const LAString &dCurve = dynamic_cast<const LADataString &>(mktData.getData(IR_CALIBRATION_DATA_DISCOUNT).get()).get();
	const LAString &suffix = dCurve == STD ? LAString("") : LAString("_") + dCurve;
	const DoubleArray &_terms = dynamic_cast<const LADataDoubles&> ((yData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
	DoubleVector _dfs(_terms.size());
	for (unsigned int i = 0; i < _terms.size(); ++i)
	{
		_dfs[i] = a_d_inter.value(_terms[i]) * f_inter.value(_terms[i]) / a_f_inter.value(_terms[i]);
	}

	// save curves
	LAObjectHolder &objHolder = getYieldData().get();
	for (map<LAString, LAString>::const_iterator it = getAssignedCurveMktMap().begin(); it != getAssignedCurveMktMap().end(); ++it)
	{
		if (it->second == market)
		{
			const LAString& curveName = it->first;
			const LAString suffix = "_" + curveName;
			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new LADataDoubles(_terms));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new LADataDoubles(_dfs));

			//mBCurveGenMap[curveName] = true;
			mGCurveGenMap[curveName] = true;
		}
	}

	// set df2
	LADataHolder* dh = &getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		setDF2();
	}
}

/*!
	@brief get market data reference

	@param[in] market name
*/
const LADataMultiReference&
LAMathYieldCurvePro::getMarketDataRef(const LAString& mktName) const
{
	LAString suffix;
	LAStringVector tmpMktNames = mktName.toToken('_');
	if (tmpMktNames.size() == 2)
	{
		const LAObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{
			throw LACoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			suffix = "_" +tmpMktNames[1];
			return dynamic_cast<const LADataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
		}
	}
	else
	{
		suffix = "_" +tmpMktNames[0];
		return dynamic_cast<const LADataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
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
LAMathYieldCurvePro::saveBasisCurve(const LAString& curveType, const DoubleArray& terms, const DoubleMatrix& termsmtx_fwd, const DoubleArray& dfs, std::vector<LAObject*>& data)
{
	LAObjectHolder &objHolder = getYieldData().get();
	for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (it->second == mAssignedCurveMktMap[curveType])
		{
			const LAString& curveName = it->first;
			LAString suffix = "_" + curveName;
			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new LADataDoubles(terms));
			objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, new LADataDoubleMatrix(termsmtx_fwd));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new LADataDoubles(dfs));
			// set generate flag true
			mBCurveGenMap[curveName] = true;
			setCurveConvention(objHolder, data, curveName);
			//set STD curve
			const LADataHolder& dh = objHolder.getData(CALIBRATION_DATA_TERMS);
			const LADataHolder& ah2 = objHolder.getData(IR_CALIBRATION_DATA_ISFWDFX);
			bool isfwdfx = false;
			if(ah2.isDefined() && !ah2.isNull())
				isfwdfx = dynamic_cast<const LADataBool&>(ah2.get()).get();
			
			if (!dh.isDefined() || dh.isNull() || isfwdfx)
			{
				objHolder.remove(IR_CALIBRATION_DATA_ISFWDFX);
				objHolder.add(IR_CALIBRATION_DATA_ISFWDFX, new LADataBool(true));
				objHolder.remove(CALIBRATION_DATA_TERMS);
				objHolder.add(CALIBRATION_DATA_TERMS, new LADataDoubles(terms));
				objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX);
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new LADataDoubleMatrix(termsmtx_fwd));
				objHolder.remove(IR_CALIBRATION_DATA_DFS);
				objHolder.add(IR_CALIBRATION_DATA_DFS, new LADataDoubles(dfs));
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
LAMathYieldCurvePro::setCurveInterpolation(const LAString& curveName, const LAObject &yieldData, LAPriceDataInterpolation& inter) const
{
	const LAString &suffix = curveName == STD ? LAString("") : LAString("_") + curveName;
	const DoubleArray &terms = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
	const DoubleArray &dfs   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
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
LAMathYieldCurvePro::calcIndexGrid(const LADate &asofdate, const LADate &date, int resetLag, const LAPriceDataDayCount &dc, 
								 const LAPriceDataCalendar &fixcal, const LAPriceDataCalendar &paycal, const LAPriceDataSlidingRule &sld, 
								 const LAString &freq, const LAString &accessary, DoubleVector &gridVec, DoubleVector &termVec)
{
	gridVec.clear();
	termVec.clear();

	LAString tmpFreq = freq;
	tmpFreq.toUpper();
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(accessary, y, m, d, w);

	unsigned int size = 0;
	unsigned int addmonth = 0;
	if (d != 0)
	{
		throw LACoreInvalidData("d != 0 is not support", __FILE__, __LINE__);	
	}
	if(0 == y && 0 == m)
	{
		throw LACoreInvalidData("This Accessory input supports only y,m,d",__FILE__,__LINE__);
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
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}
		size = y * 4 + m / 3 + 1;
		addmonth = 3;

	}
	else if (freq == SEMI_ANNUAL)
	{
		if (m % 6 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}	
		size = y * 2 + m / 6 + 1;
		addmonth = 6;

	}
	else if (freq == ANNUAL)
	{
		if (m % 12 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		size = y + m / 12 + 1;
		addmonth = 12;
	}
	else
	{
		//error
		LAString err = "Frequency: ";
		err += freq;
		err += " is not support";
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	LAPriceDataDayCount dc_act(ACT_365_ISDA);
	gridVec.resize(size);
	termVec.resize(size - 1);
	LADate fixingdate = fixcal.getBusinessDay(date, -1 * resetLag);
	LADate paymentdate = paycal.getBusinessDay(fixingdate, resetLag);
	LADate noadjdate = paymentdate;
	gridVec[0] = dc_act.getTerm(asofdate, paymentdate);

	for (unsigned int i = 1; i < size; i++)
	{
		LADate olddate = paymentdate;
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
LAMathYieldCurvePro::calcBasisCF(const LAPriceDataInterpolation &s_inter, const bool isDiscount, const bool isFwdRen, const bool isUSD, const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, 
							   const std::vector<DoubleArray> &dfsVec, const LAPriceDataInterpolation &a_f_inter, const LAPriceDataInterpolation *a_fwd_inter, const LAPriceDataInterpolation &a_d_inter, const double a_d_df_adjust, const LAPriceDataInterpolation *adjust_inter, 
							   const bool isAgtSpread, const DoubleArray &spreadVec, const std::vector<IntArray> &cpd_timesVec, const std::vector<IntArray> &a_cpd_timesVec, const double spotTerm, const double a_spotTerm, 
							   const std::vector<DoubleArray> &gridVec, const std::vector<DoubleArray> &tauVec, const std::vector<DoubleMatrix> &i_gridMatVec, const std::vector<DoubleMatrix> &i_termMatVec, 
							   const std::vector<DoubleArray> &a_gridVec, const std::vector<DoubleArray> &a_tauVec, const std::vector<DoubleMatrix> &a_i_gridMatVec, const std::vector<DoubleMatrix> &a_i_termMatVec, 
							   const std::vector<DoubleMatrix> &b_yieldTimeMatVec, DoubleArray &a_targetPVVec, DoubleArray &out, const bool isNegative)
{
	unsigned int b_size = spreadVec.size();
	if (gridVec.size() != b_size || tauVec.size() != b_size || dfsVec.size() != b_size ||
		i_gridMatVec.size() != b_size || i_termMatVec.size() != b_size || b_yieldTimeMatVec.size() != b_size || a_targetPVVec.size() != b_size)
	{
		throw LACoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
	}

	for (unsigned int i = 0; i < b_size; ++i)
	{
		const unsigned int gridSize = gridVec[i].size();
		if (i_gridMatVec[i].size() != gridSize || tauVec[i].size() != gridSize || dfsVec[i].size() != gridSize || b_yieldTimeMatVec[i].size() != gridSize)
		{
			throw LACoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
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
LAMathYieldCurvePro::calcAgainstPV(const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, const LAPriceDataInterpolation &d_inter, 
 const double spread, const IntArray& cpd_times, const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const bool isBackward)
{
	LAPriceDataInterpolation dummy_interpolation;
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
LAMathYieldCurvePro::calcAgainstPV(const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, const LAPriceDataInterpolation &d_inter, const LAPriceDataInterpolation &a_s_inter, const LAPriceDataInterpolation *adjust_inter, 
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
			a_dfsVec[j] = LAMath::exp(-a_s_inter.value(terms_grid[j - 1] + adj_term_spot));
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
LAMathYieldCurvePro::calcTargetPV(const LAPriceDataInterpolation &s_inter, const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, 
	const double spread, const IntArray& cpd_times, double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
	const DoubleMatrix &b_yieldTimeMat)
{
	LAPriceDataInterpolation dummy_interpolation;
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
LAMathYieldCurvePro::calcTargetPV(const LAPriceDataInterpolation &s_inter, const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, 
	const LAPriceDataInterpolation &a_d_inter, const LAPriceDataInterpolation *adjust_inter, const double spread, const IntArray& cpd_times, const double term_spot, 
	const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const DoubleMatrix &b_yieldTimeMat) 
{
	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw LACoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
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
		[](const double& yieldTime, const double& b_yieldTime) { return LAMath::exp(-(yieldTime + b_yieldTime)); });
	for (unsigned int j = 1; j < terms_grid.size(); ++j)
	{
		yieldTimes = { yieldTimes.back(), s_inter.value(terms_grid[j]) };	// yieldTimes.back() must be s_inter.value(terms_grid[j - 1]) on right hand side
		b_yieldTimes = { b_yieldTimeMat[j].front(), b_yieldTimeMat[j].back() };
		transform(yieldTimes.cbegin(), yieldTimes.cend(), b_yieldTimes.cbegin(), dfsMat[j].begin(),
			[](const double& yieldTime, const double& b_yieldTime) { return LAMath::exp(-(yieldTime + b_yieldTime)); });
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
LAMathYieldCurvePro::calcTargetPV(const LAPriceDataInterpolation &s_inter, const DoubleArray &dfsVec, const double spread, const IntArray& cpd_times,
	 const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
	 const DoubleMatrix &b_yieldTimeMat)
{
	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw LACoreInvalidData(" size is not consistent.", __FILE__, __LINE__);
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
			df = LAMath::exp(-yield_time);
			ant += i_termMat[j][k - 1] * df;
		}

		const double term1 = i_gridMat[j][0] - term_spot;
		const double y_spread_time1 = s_inter.value(term1);
		const double yield_time1 = y_spread_time1 + b_yieldTimeMat[j][0];
		const double df1 = LAMath::exp(-yield_time1);
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
LAMathYieldCurvePro::calcLegPV(const DoubleArray &fwdsVec, const DoubleMatrix &dfsMat, const DoubleArray &a_dfsVec, const LAPriceDataInterpolation *adjust_inter,
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
	LADataHolder *dh = 0;
	// calc basis dfs
	dh = &getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector &basisDFs = dynamic_cast<const LADataStrings &>(dh->get()).get();
		LAStringVector::const_iterator it = basisDFs.begin();
		while (it != basisDFs.end())
		{
			// data check
			bool isCheckCurves = false;
			dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
			if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const LADataBool &>(dh->get()).get();
			bool isCurveAttrExist = checkCurveAttr(getMarketForCurve(*it));
			if (!isCheckCurves || !isCurveAttrExist)
			{
				dh = &getData(IR_CALIBRATION_DATA_BASISTARGETDF, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const LAString &targetDF = dynamic_cast<const LADataString &>(dh->get()).get();
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
	const LAString &mainCName = dynamic_cast<const LADataString &>(getData(IR_CALIBRATION_DATA_MAINBASISDF, ISNOTNULL).get()).get();
	if (mGCurveGenMap.find(mainCName) == mGCurveGenMap.end() && 
		mBCurveGenMap.find(mainCName) == mBCurveGenMap.end() && 
		mArbFreeCurveGenMap.find(mainCName) == mArbFreeCurveGenMap.end()) return;

	LAObjectHolder &objHolder = getYieldData().get();
	DoubleArray dfs_mod;
	DoubleArray yield_mod;

	//const LAString &mainCName = dynamic_cast<const LADataString &>(getData(IR_CALIBRATION_DATA_MAINBASISDF, ISNOTNULL).get()).get();
		
	LAString suffix = "";
	if (mainCName != STD)
	{
		suffix = "_" + mainCName;
	}
	//const DoubleArray &terms = dynamic_cast<const LADataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
	DoubleArray terms_mod = dynamic_cast<const LADataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get();
	DoubleArray dfs_mod_ = dynamic_cast<const LADataDoubles &>(objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).get();
	DoubleArray terms = terms_mod;
	LADataHolder* dh;
	dh = &objHolder.getData(CALIBRATION_DATA_TERMS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		terms = dynamic_cast<LADataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS, NOCHECK).get()).get();
	// adjust terms
	LAPriceDataInterpolation inter = getInterpolation();
	dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		inter.setJoinDateAsDouble(dynamic_cast<LADataDouble&>(dh->get()).get());
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
	objHolder.add(IR_CALIBRATION_DATA_DFS2, new LADataDoubles(dfs_mod));
}

/*!
    @brief calc basis rates and add DF2 data as modified DF

*/
void
LAMathYieldCurvePro::setBasisRates_old(void)
{
	vector<const LAObject *> data_basis;
	// get market data
	const LADataMultiReference &mr = getMarketData();
	for (unsigned int i = 0; i < mr.getSize(); i++)
	{
        LAString term = dynamic_cast<const LADataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		term.toUpper();
		if (term == BASIS) 
			data_basis.push_back(&mr.get(i).get());
	}
	// data exist check
	if (data_basis.empty())
	{
		return;
		//throw LACoreInvalidData("basis data is not set.", __FILE__, __LINE__);
	}
	// sort
	sort(data_basis.begin(), data_basis.end(), Comp_term());

	// get spotdate
	const LADate &spotdate = dynamic_cast<const LADataDate &> ((data_basis[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const LADate &asof = dynamic_cast<const LADataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
	// asof and spot check
	if (asof > spotdate)
	{
		throw LACoreInvalidData("asofdate > spotdate, cannnot calc basis DF", __FILE__, __LINE__);
	}

	// get first element val
	const LAPriceDataDayCount &dcbs = dynamic_cast<const LAPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNTBASE, ISNOTNULL)).get());
	const LAPriceDataDayCount &dc = dynamic_cast<const LAPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const LAPriceDataCalendar &calbs = dynamic_cast<const LAPriceDataCalendar &>((data_basis[0]->getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
	const LAPriceDataSlidingRule &sldbs  = dynamic_cast<const LAPriceDataSlidingRule &>((data_basis[0]->getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
	//iseomroll
	bool eombs = false;
	LADataHolder dh = data_basis[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
	if (dh.isDefined() && !dh.isNull())
	{
		eombs = dynamic_cast<const LADataBool &>(dh.get()).get();
	}
	LAString freqbs = dynamic_cast<const LADataString &>((data_basis[0]->getData(IR_CALIBRATION_DATA_FREQUENCYBASE, ISNOTNULL)).get()).get();
	freqbs.toUpper();
	// set roll convention
	LAString roll_conv_bs("");
	if (freqbs == LUNAR) roll_conv_bs = ROLLCONV_LUNAR;
	else if (eombs) roll_conv_bs = ROLLCONV_EOM;
	else roll_conv_bs = ROLLCONV_NORMAL;
	
	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	for (unsigned int i = 0; i < data_basis.size(); i++)
	{
		const LAString &strTerm  = dynamic_cast<const LADataString &>((data_basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		double rate = dynamic_cast<const LADataDouble &>((data_basis[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		LADate tmpDate = LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
		double term = dcbs.getTerm(spotdate, tmpDate);

		b_t_grid.push_back(term);
		b_termstruct_grid.push_back(rate);	
	}
	// get max term
	const LAString &termMax = dynamic_cast<const LADataString &>((data_basis.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	// get interpolation method
	LAPriceDataInterpolation &inter = getInterpolation_bs();
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
		LAString msg = "frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		LAString msg = "frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	LAObjectHolder objHolder = getYieldData().get();
	const LAObject &yieldData = objHolder.get(); 
	const DoubleArray &terms = dynamic_cast<const LADataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms.back();

	DoubleArray b_t(1, 0.0);
	DoubleArray df_mod(1, 1.0);
	double sum = 0.0;
	double sum1 = 0.0;
	double sum2 = 0.0;
	double b_delta = 0.0;
	double b_libor = 0.0;
	LADate fdate = spotdate;
	const unsigned int max = m / mUnit + 1;
	for (unsigned int i = 1; i < max; i++)
	{
		LAString strTerm = LAString(static_cast<int>(mUnit * i));
		strTerm += "M";

		LADate ldate =  LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
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
		LAString strTerm_ = LAString(static_cast<int>(mUnit * i));
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
			throw LACoreInvalidData("Df is below zero.", __FILE__, __LINE__);
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
		throw LACoreInvalidData("Df from asofdate to spotdate is below zero.", __FILE__, __LINE__);
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
			throw LACoreInvalidData("Df is below zero.", __FILE__, __LINE__);
		}
		b_mod[i] = -LAMath::log(df) / terms[i];
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
			dfs_mod.push_back(LAMath::exp(-term * bRate));
		}
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS2);
	objHolder.add(IR_CALIBRATION_DATA_DFS2, new LADataDoubles(dfs_mod));
}

/*!
    @brief calc basis rates and set new curve 

*/
void
LAMathYieldCurvePro::setBasisRates2(const LAString& basisCurveID)
{
	vector<const LAObject *> data_basis;
	// get market data
	const LADataMultiReference &mr = getMarketData();
	for (unsigned int i = 0; i < mr.getSize(); i++)
	{
        LAString term = dynamic_cast<const LADataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		term.toUpper();
		if (term == BASIS) 
			data_basis.push_back(&mr.get(i).get());
	}
	// data exist check
	if (data_basis.empty())
	{
		throw LACoreInvalidData("basis data is not set.", __FILE__, __LINE__);
	}
	// sort
	sort(data_basis.begin(), data_basis.end(), Comp_term());

	// get spotdate
	const LADate &spotdate = dynamic_cast<const LADataDate &> ((data_basis[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const LADate &asof = dynamic_cast<const LADataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
	// asof and spot check
	if (asof > spotdate)
	{
		throw LACoreInvalidData("asofdate > spotdate, cannnot calc basis DF", __FILE__, __LINE__);
	}

	// get first element val
	const LAPriceDataDayCount &dcbs = dynamic_cast<const LAPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNTBASE, ISNOTNULL)).get());
	const LAPriceDataDayCount &dc = dynamic_cast<const LAPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const LAPriceDataCalendar &calbs = dynamic_cast<const LAPriceDataCalendar &>((data_basis[0]->getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
	const LAPriceDataSlidingRule &sldbs  = dynamic_cast<const LAPriceDataSlidingRule &>((data_basis[0]->getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
	//iseomroll
	bool eombs = false;
	LADataHolder dh = data_basis[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
	if (dh.isDefined() && !dh.isNull())
	{
		eombs = dynamic_cast<const LADataBool &>(dh.get()).get();
	}
	LAString freqbs = dynamic_cast<const LADataString &>((data_basis[0]->getData(IR_CALIBRATION_DATA_FREQUENCYBASE, ISNOTNULL)).get()).get();
	freqbs.toUpper();
	// set roll convention
	LAString roll_conv_bs("");
	if (freqbs == LUNAR) roll_conv_bs = ROLLCONV_LUNAR;
	else if (eombs) roll_conv_bs = ROLLCONV_EOM;
	else roll_conv_bs = ROLLCONV_NORMAL;

	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	for (unsigned int i = 0; i < data_basis.size(); i++)
	{
		const LAString &strTerm  = dynamic_cast<const LADataString &>((data_basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		double rate = dynamic_cast<const LADataDouble &>((data_basis[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		LADate tmpDate = LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
		double term = dcbs.getTerm(spotdate, tmpDate);

		b_t_grid.push_back(term);
		b_termstruct_grid.push_back(rate);	
	}
	// get max term
	const LAString &termMax = dynamic_cast<const LADataString &>((data_basis.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	// get interpolation method
	LAPriceDataInterpolation &inter = getInterpolation_bs();
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
		LAString msg = "frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		LAString msg = "frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	LAObjectHolder objHolder = getYieldData().get();
	const LAObject &yieldData = objHolder.get(); 
	const DoubleArray &terms = dynamic_cast<const LADataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms.back();

	DoubleArray b_t(1, 0.0);
	DoubleArray df_mod(1, 1.0);
	LADate fdate = spotdate;
	const unsigned int max = m / mUnit + 1;
	for (unsigned int i = 1; i < max; i++)
	{
		LAString strTerm = LAString(static_cast<int>(mUnit * i));
		strTerm += "M";

		LADate ldate = LAMathDateCalculations::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
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
			LAString strTerm_ = LAString(static_cast<int>(mUnit * j));
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
			throw LACoreInvalidData("Df is below zero.", __FILE__, __LINE__);
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
		throw LACoreInvalidData("Df from asofdate to spotdate is below zero.", __FILE__, __LINE__);
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
			throw LACoreInvalidData("Df is below zero.", __FILE__, __LINE__);
		}
		b_mod[i] = -LAMath::log(df) / terms[i];
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
			dfs_mod.push_back(LAMath::exp(-term * bRate));
		}
	}
    
    LADataInstance* dataInstance = getDataInstance();
    LAObjectPool &objPool = dataInstance->getObjectPool();
	objHolder = objPool.getObject(basisCurveID,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add("Name", new LADataString()).convertFromString(basisCurveID);
		e->add("AsOfDate", new LADataDate(asof));
		e->add("Terms",	new LADataDoubles(terms));
		e->add("DiscountFactors",new LADataDoubles(dfs_mod));
		objPool.set(basisCurveID,e);
	}
	else if(objHolder.isDefined())
	{
		LADataHolder* dh;
		dh = &objHolder.getData("AsOfDate",ISDEFINED);
		LADataDate& date = dynamic_cast<LADataDate &>(dh->get());
		date.set(asof);

		dh = &objHolder.getData("Terms",ISDEFINED);
		LADataDoubles& terms_att = dynamic_cast<LADataDoubles& >(dh->get());
		terms_att.set(terms);
		dh = &objHolder.getData("DiscountFactors",ISDEFINED);
		LADataDoubles& dfs = dynamic_cast<LADataDoubles& >(dh->get());
		dfs.set(dfs_mod);
	}
}

/*!
    @brief add basis rates

*/
void
LAMathYieldCurvePro::addBasisRates(void)
{	
	LAObjectHolder objHolder = getYieldData().get();
	const LAObject& YieldData = objHolder.get();
	const DoubleArray& dfs   = dynamic_cast<const LADataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS + mCurveSuffix, ISNOTNULL)).get()).get();
	DoubleArray dfs_mod(dfs.size());
	
	const DoubleMatrix& d_matrix = getBasisRates().get();
	
	for(unsigned int i = 0; i < d_matrix.size(); i++)
	{
		for(unsigned int j = 0; j < d_matrix[i].size(); j++)
			dfs_mod[j] = LAMath::exp(-d_matrix[i][j]/* * terms[j]*/) * dfs[j];
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new LADataDoubles(dfs_mod));
}

/*!
    @brief set arb free flag and curve name
*/
void
LAMathYieldCurvePro::setArbFreeCurveName(const LAStringVector& curveNames_6ML, 
									   const LAStringVector& curveNames_DF, 
									   const LAStringVector& curveNames_3ML)
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
                                                                    LAPriceDataInterpolation& spread_time_inter,
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
                                                                              LAPriceDataInterpolation &s_inter,
                                                                              const bool isDiscount, 
                                                                              const bool isFwdRen, 
                                                                              const bool isUSD, 
                                                                              const bool isFWDInter, 
                                                                              const LAPriceDataInterpolation &f_inter, 
                                                                              const LAPriceDataInterpolation *fwd_inter, 
                                                                              const std::vector<DoubleArray> &dfsVec, 
	                                                                          const LAPriceDataInterpolation &a_f_inter, 
                                                                              const LAPriceDataInterpolation *a_fwd_inter, 
                                                                              const LAPriceDataInterpolation &a_d_inter, 
                                                                              const double a_d_df_adjust, 
                                                                              const LAPriceDataInterpolation *adjust_inter, 
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
LACoreFunctionBase*
LAMathBasisFunction::clone() const
{
    try 
	{
		return new LAMathBasisFunction(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
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
	throw LACoreInvalidData("parameter size must be one", __FILE__, __LINE__);
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
		ret += mParam[i] * LAMath::exp(sum);
	}
	ret += dfn * LAMath::exp(sum) + vminusv ;
	return LAMath::sqr(ret);
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
LACoreFunctionBase*
LAMathBasisFunction2::clone() const
{
    try 
	{
		return new LAMathBasisFunction2(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
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
	throw LACoreInvalidData("parameter size must be one", __FILE__, __LINE__);
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
		ret += mParam[i] * LAMath::exp(sum);
	}

	ret += dfn*LAMath::exp(sum) + vminusv ;
	return LAMath::sqr(ret);
}

void
LAMathYieldCurvePro::setdNPVdm(const LAString &curveType)
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	//check whethor Matrix has been made or not
	LADataHolder *dh = 0;

	dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix));
	if (dh->isDefined() && !dh->isNull())
		return;

	// get market data
	LAString mktName;
	if (curveType == STD)
	{
		mktName = SWAP;
	}
	else
	{
		mktName = mAssignedCurveMktMap[curveType];
	}
	LAString suffix_mkt = "";
	if (mktName != SWAP)
		suffix_mkt = "_" + mktName;
	const LADataMultiReference* mr_;
	LAStringVector tmpMktNames = mktName.toToken('_');
	if (tmpMktNames.size() == 2)
	{
		LAObjectHolder &eh_fy = getForeignYieldData().get();
		if(!eh_fy.isDefined())
		{	
			throw LACoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
		}
		else
		{
			mr_ = &dynamic_cast<const LADataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1], ISNOTNULL).get());
		}
	}
	else
	{
		mr_ = &dynamic_cast<const LADataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());
	}
	vector<LAObject*> dataall;
	IntArray omitvec;
	for(unsigned int i = 0; i < mr_->getSize(); i++)
	{
       const LAString &dataType = dynamic_cast<const LADataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();

	   //grid long case whether we can omit or not
	   dh = &(mr_->get(i).getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK));
	   bool isexist = (dh->isDefined() && !dh->isNull());
	   if (!isexist)
	   {
		   omitvec.push_back(i);
		   continue;
	   }

		LAString tmpType = dataType;
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

	LAStringVector typevec(dataall.size());
	std::vector<DateVector> dateMat(dataall.size());
	DoubleVector gridtermvec(dataall.size());
	LAStringVector gridstrvec(dataall.size());
	DoubleVector mratevec(dataall.size());

	DoubleVector dNPVdm(dataall.size());
	DoubleMatrix dNPVdzero(dataall.size());
	double dnpvdm = 0.0;
	const LADate& asOfDate = getAsOfDate().get();
	LAPriceDataDayCount dc_act365(ACT_365_ISDA); 
	
	const LAInterpolationBase& pInter = getDFInterpolation(&curveType);
	dh = &getData(IR_CALIBRATION_DATA_DFCURVENAME + suffix, NOCHECK);
	LAString dfcurveType = curveType;
	if (dh->isDefined() && !dh->isNull())
	{
		LAString keyname = dynamic_cast<LADataString &>(dh->get());
		if (keyname == ITSELF)
			dfcurveType = curveType;
		else
			dfcurveType = keyname;
	}
	const LAInterpolationBase& pInterSTD = getDFInterpolation(&dfcurveType);


	//note that cashflow is always payers
	unsigned int sizeAll = dataall.size();
	bool isdiscount,isagtspd;
	for (unsigned int i = 0; i < sizeAll; i++)
	{
		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL));
		typevec[i] = dynamic_cast<const LADataString &>(dh->get());

		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, ISNOTNULL));
		dateMat[i] = dynamic_cast<const LADataDates &>(dh->get()).get();
		dNPVdzero[i].resize(dateMat[i].size());

		//termvec
		gridtermvec[i] = dc_act365.getTerm(asOfDate, dateMat[i].back());
		//termstrgrid
		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_TERM, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL));
			gridstrvec[i]= dynamic_cast<const LADataString &>(dh->get());
		}
		else
			gridstrvec[i]= dynamic_cast<const LADataString &>(dh->get());
		

		dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
		{
			dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL));
		}
		const LAPriceDataDayCount& dcCalc = dynamic_cast<const LAPriceDataDayCount &>(dh->get());

		//Libor, O_N or T_N case
		if (typevec[i] == ZERO || typevec[i] == O_N || typevec[i] == T_N || typevec[i] == BOJ || typevec[i] == FEDFUNDRATE)
		{
			if (dateMat[i].size() != 2)
				throw LACoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			double mrate = dynamic_cast<const LADataDouble &>(dh->get());

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
				throw LACoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dftermLast = dc_act365.getTerm(asOfDate, dateMat[i].back());

			double df1 = pInter.value(dfterm1);
			double dfLast =  pInter.value(dftermLast);

			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			double mrate = dynamic_cast<const LADataDouble &>(dh->get());
			if (LAMath::abs(mrate) < EPS)
			{
				//throw LACoreInvalidData("Market rate 0", __FILE__,__LINE__);
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
				throw LACoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			if (0==i)
			{
				dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL));
				isdiscount = dynamic_cast<LADataBool &>(dh->get());

				dh = &(dataall[i]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL));
				isagtspd = dynamic_cast<LADataBool &>(dh->get());
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
				double brate = dynamic_cast<const LADataDouble &>(dh->get());
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
				double brate = dynamic_cast<const LADataDouble &>(dh->get());
				mratevec[i] = brate;
			}
		}//future case
		else if (typevec[i] == FUTURE)
		{
			if (dateMat[i].size() != 2)
				throw LACoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			double mrate = 0.0;
			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				mrate = dynamic_cast<const LADataDouble &>(dh->get());
			else
			{
				dh = &(dataall[i]->getData(PRICING_DATA_PRICE, ISNOTNULL));
				mrate = 1.0- 0.01 *  dynamic_cast<const LADataDouble &>(dh->get());
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
			throw LACoreInvalidData("Not Support Now",__FILE__,__LINE__);
		}
		else if (typevec[i] == FRA3M || typevec[i] == FRA6M)
		{
			if (dateMat[i].size() != 2)
				throw LACoreInvalidData("dNPVdzero Error",__FILE__,__LINE__);

			double mrate = 0.0;
			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			mrate = dynamic_cast<const LADataDouble &>(dh->get());

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
			LADate targetdate = dateMat[i][j];
			LAAlgorithm::locate<DateVector, LADate>(gridvec,targetdate,gridvec.size(),gridpos);

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
					throw LACoreInvalidData("DateMatrix Error",__FILE__,__LINE__);

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
			throw LACoreInvalidData("dNPVdm 0 Error",__FILE__,__LINE__);
		DoubleVector div_vec(sizeAll, dNPVdm[i]);
		transform(decompMat[i].begin(), decompMat[i].end(), div_vec.begin(), decompMat[i].begin(), divides<double>());
	}

	//inverse
	LAMatrix matobj(decompMat);
	const LAMatrix& invmat = matobj.inverseMatrix();
	const LAMatrix& transmat = invmat.transpose();

	DoubleMatrix ret(sizeAll, DoubleVector(sizeAll,0.0));
	for (unsigned int i = 0; i < sizeAll; i++)
		for (unsigned int j = 0; j < sizeAll; j++)
			ret[i][j] = transmat.getValue(i,j);

	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix, new LADataDoubleMatrix(ret));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix, new LADataDoubles(gridtermvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix, new LADataInts(omitvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES + suffix, new LADataStrings(gridstrvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS + suffix, new LADataDoubles(mratevec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES + suffix, new LADataStrings(typevec));

	return;
}


const LADataDoubleMatrix&
LAMathYieldCurvePro::getConversionMatrix(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix, ISNOTNULL));
	return dynamic_cast<const LADataDoubleMatrix &>(dh->get());
}

const LADataDoubles&	
LAMathYieldCurvePro::getConversionMatrixTerm(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix, ISNOTNULL));
	return dynamic_cast<const LADataDoubles &>(dh->get());
}

const LADataDoubles&	
LAMathYieldCurvePro::getConversionMarketRates(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS + suffix, ISNOTNULL));
	return dynamic_cast<const LADataDoubles &>(dh->get());
}


const LADataInts&
LAMathYieldCurvePro::getConversionOmitGrids(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix, ISNOTNULL));
	return dynamic_cast<const LADataInts &>(dh->get());
}

const LADataStrings&
LAMathYieldCurvePro::getConversionMatrixTermTypes(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES + suffix, ISNOTNULL));
	return dynamic_cast<const LADataStrings &>(dh->get());
}

const LADataStrings&
LAMathYieldCurvePro::getConversionMarketTypes(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES + suffix, ISNOTNULL));
	return dynamic_cast<const LADataStrings &>(dh->get());
}


bool 
LAMathYieldCurvePro::isOmitGridsExist(const LAString &curveType) const
{
	LAString suffix = "";
	if (curveType != STD)
		suffix = "_" + curveType;

	const LADataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix));
	return (dh->isDefined() && !dh->isNull());

}
void
LAMathYieldCurvePro::setCurveDependencyMap(void)
{
	//bcurve = STD + alpha then map[bcurve][Base][+alpha]
	std::map<LAString, bool>::iterator itbgen = mBCurveGenMap.begin();
	for (itbgen = mBCurveGenMap.begin(); itbgen != mBCurveGenMap.end(); ++itbgen)
	{
		LAString curveType = itbgen->first;

		LAString suffix = "";
		if (curveType != STD)
			suffix = "_" + curveType;
		//	// get market data
		LAString suffix_mkt = "";
		if (curveType != STD)
			suffix_mkt = "_" + mAssignedCurveMktMap[curveType];
		const LADataMultiReference* mr_;
		LAStringVector tmpMktNames = mAssignedCurveMktMap[curveType].toToken('_');
		if (tmpMktNames.size() == 2)
		{
			LAObjectHolder &eh_fy = getForeignYieldData().get();
			if(!eh_fy.isDefined())
			{	
				throw LACoreInvalidData("Foreign Yield Data does not exist!", __FILE__, __LINE__); 
			}
			else
			{
				mr_ = &dynamic_cast<const LADataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + LAString("_") + tmpMktNames[1], ISNOTNULL).get());
			}
		}
		else
		{
			mr_ = &dynamic_cast<const LADataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix_mkt, ISNOTNULL).get());
		}
		//find basis object
		unsigned int bpos =  mr_->getSize();
		for (unsigned int i = 0; i < mr_->getSize(); i++)
		{
			const LAString &dataType = dynamic_cast<const LADataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			if (dataType == BASIS)
			{	
				bpos = i;
				break;
			}
		}
		if (bpos == mr_->getSize())
			throw LACoreInvalidData("Not found DataType = BASIS",__FILE__,__LINE__);

		LAObject& ebasis = mr_->get(bpos).get();
		bool isdiscount = dynamic_cast<const LADataBool &>(ebasis.getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
		bool isagtspd =  dynamic_cast<const LADataBool &>(ebasis.getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();
		double spdval = 0.0;
		LAString targetCurve;
		//forecasttype
		if (!isdiscount)
		{ 
			//now under construnction
			spdval = 1.0;
			targetCurve = dynamic_cast<const LADataString &>(ebasis.getData(IR_CALIBRATION_DATA_AGTFORECAST , ISNOTNULL).get()).get();
		}
		else //discount
		{
			//now under construnction
			spdval = 1.0;
			if (isagtspd)
				targetCurve = dynamic_cast<const LADataString &>(ebasis.getData(IR_CALIBRATION_DATA_AGTFORECAST , ISNOTNULL).get()).get();
			else
				targetCurve = dynamic_cast<const LADataString &>(ebasis.getData(IR_CALIBRATION_DATA_FORECAST , ISNOTNULL).get()).get();
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

std::map<LAString, double>&
LAMathYieldCurvePro::getCurveDependeny(const LAString &curveType) const 
{
	LAString tmp(curveType);
	if (mBfCurveType == tmp)
		return mBfDpnMap;

	//if xccybasiscurve = STD + (xccySpred) + (- 3M/6MSpread) 
	//then return map, [xccybasiscurve][1], [3M-6Mbasiscurve][-1]
	mBfDpnMap.clear();
	mBfCurveType = LAString(tmp);

	std::map<LAString, bool>::const_iterator itbgen = mBCurveGenMap.begin();
	std::map<LAString, std::map<LAString, double> >::const_iterator itdpn = mDpnCurveMap.begin();
	std::map<LAString, bool>::const_iterator itggen = mGCurveGenMap.begin();

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
		throw LACoreInvalidData("Curve Generate Map Error",__FILE__,__LINE__);

	LAString nameTmp = curveType;
	bool iscomplete = false;
	while (!iscomplete)
	{
		
		itdpn = mDpnCurveMap.find(nameTmp);
		if (itdpn->second.size() != 1)
			throw LACoreInvalidData("Not Support Now",__FILE__,__LINE__);
		
		std::map<LAString, double>::const_iterator itTmp = itdpn->second.begin();
		if (itTmp->first == nameTmp)
			throw LACoreInvalidData("Loop Error",__FILE__,__LINE__);
	
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
LAMathYieldCurvePro::changeZeroRiskIntoMarketRisk(LAString curveType, const DoubleVector& termZeroVals, const DoubleVector& riskZeroVals,
												LAStringVector& termMarketGrids, DoubleVector& riskMarketVals) const
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
		LAAlgorithm::locate<DoubleVector, double>(termvec,term,termvec.size(),gridpos);
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
				throw LACoreInvalidData("TermVec Error",__FILE__,__LINE__);
			
			double ratio1 = (termvec[gridpos]- term) / diffD;
			dNPVdzero[gridpos-1] +=riskval * ratio1;
			dNPVdzero[gridpos] += riskval * (1.0 - ratio1);
		}
	}
	
	LAMatrix convertMat(mat);
	LAMatrix dNPVdzeroMat(dNPVdzero);
	const LAMatrix& multimat = convertMat * dNPVdzeroMat;
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
const LADataBool&  
LAMathYieldCurvePro::getIsSwapTenorAdjust(void) const
{
	return dynamic_cast<const LADataBool&>(mpIsSwapTenorAdjust->get());
}
/*!
    @brief get IsSwapTenorCahnge
	@return IsSwapTenorCahnge
*/
LADataBool&  
LAMathYieldCurvePro::getIsSwapTenorAdjust(void)
{
	return dynamic_cast<LADataBool&>(mpIsSwapTenorAdjust->get());
}

/*!
    @brief  

	@return 
*/
const LADataReference&
LAMathYieldCurvePro::getForeignYieldData() const
{
	return dynamic_cast<const LADataReference&>(mpForeignYieldData->get());
}
/*!
    @brief 
			
	@return 
*/
LADataReference&
LAMathYieldCurvePro::getForeignYieldData()
{
	return dynamic_cast<LADataReference&>(mpForeignYieldData->get());
}

/*!
    @brief  

	@return 
*/
const LADataReference&
LAMathYieldCurvePro::getColYieldData() const
{
	return dynamic_cast<const LADataReference&>(mpColYieldData->get());
}

/*!
    @brief 
			
	@return 
*/
LADataReference&
LAMathYieldCurvePro::getColYieldData()
{
	return dynamic_cast<LADataReference&>(mpColYieldData->get());
}

/*!
    @brief calc floater PV
	@param[in] curveName

*/
void
LAMathYieldCurvePro::setFloater(const LAString& curveName)
{
	LAString tmpCurveName = curveName;
	LAObjectHolder &objHolder = getYieldData().get();
	LADataHolder *dh;
	// data check
	bool isCheckCurves = false;
	dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const LADataBool &>(dh->get()).get();
	bool isCurveAttrExist = checkCurveAttr(curveName);
	if (isCheckCurves && isCurveAttrExist) return;

	LAPriceDataDayCount dc_act(ACT_365_ISDA);
	LAObject &yieldData = objHolder.get();

	LAString discountName = 
		dynamic_cast<LADataString&> (getData(IR_CALIBRATION_DATA_DISCOUNT + LAString("_") + tmpCurveName.toLower(), ISNOTNULL).get());
	LAString forecastName = 
		dynamic_cast<LADataString&> (getData(IR_CALIBRATION_DATA_FORECAST + LAString("_") + tmpCurveName.toLower(), ISNOTNULL).get());
	LAString suffix_d = ""; if (discountName != STD) suffix_d = LAString("_") + discountName;
	LAString suffix_f = ""; if (forecastName != STD) suffix_f = LAString("_") + forecastName;
	
	// set forecast curve
	LAPriceDataInterpolation f_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	const DoubleArray &terms_f = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix_f, ISNOTNULL)).get()).get();
	const DoubleArray &dfs_f   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix_f, ISNOTNULL)).get()).get();
	f_inter.set(terms_f, dfs_f);

	// set discount curve
	LAPriceDataInterpolation d_inter = dynamic_cast<LAPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	const DoubleArray &terms_d = dynamic_cast<const LADataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix_d, ISNOTNULL)).get()).get();
	const DoubleArray &dfs_d   = dynamic_cast<const LADataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix_d, ISNOTNULL)).get()).get();
	d_inter.set(terms_d, dfs_d);

	const DoubleArray &terms_std = dynamic_cast<const LADataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms_std.back();
	// get base yield curve and asofdate
	const LAMathYieldCurve &baseYieldCurve = dynamic_cast<const LAMathYieldCurve &>(getBaseYieldCurve().get().get());
	const LADate &asof = dynamic_cast<const LADataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());	

	DoubleArray dfs; dfs.push_back(1.);
	DoubleArray terms; terms.push_back(0.);

	const double eps_term = 1E-10;
	
	dh = &getData(IR_CALIBRATION_DATA_BASISDATA + LAString("_") + tmpCurveName.toLower(),NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		LAString basisMkt = dynamic_cast<LADataString &>(dh->get()).get();
		LAString suffix = "_" + basisMkt;
		// get market data
		const LADataMultiReference& mr = dynamic_cast<const LADataMultiReference&>
			(getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
		vector<LAObject*> data;
		for(unsigned int i = 0; i < mr.getSize(); i++)
		{
			// check use grid
			const LADataHolder *dh = &mr.get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const LADataBool &>(dh->get()).get()) continue;
			
		   const LAString &dataType = dynamic_cast<const LADataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			LAString tmpType = dataType;
			tmpType.toUpper();
			if (tmpType.findString(BASIS) == 0) 
			{
				data.push_back(&mr.get(i).get());
			}
		}
		// data exist check
		if (data.empty())
		{
			LAString msg = "CurveName = "+ basisMkt + ", basis data is not set.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// sort
		sort(data.begin(), data.end(), Comp_term());
		// market convencion info
		const bool isTimeInter  = dynamic_cast<const LADataBool &>((data[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL)).get()).get();
		//eom roll
		bool eom = false;
		dh = &data[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) 
			eom = dynamic_cast<const LADataBool &>(dh->get()).get();
		// cashlet
		const LAPriceDataDayCount &c_dc = dynamic_cast<const LAPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		const LAPriceDataCalendar &c_cal = dynamic_cast<const LAPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		const LAPriceDataSlidingRule &c_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		const LADate &c_spotdate = dynamic_cast<const LADataDate &> (data[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
		LAString c_freq = dynamic_cast<const LADataString &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
		const int c_lag  = dynamic_cast<const LADataInt &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());
		// index
		const LAPriceDataDayCount &i_dc = dynamic_cast<const LAPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
		const LAPriceDataCalendar &i_fixcal = dynamic_cast<const LAPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
		const LAPriceDataCalendar &i_paycal = dynamic_cast<const LAPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
		const LAPriceDataSlidingRule &i_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
		const int i_lag  = dynamic_cast<const LADataInt &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
		LAString i_freq = dynamic_cast<const LADataString &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
		LAString i_accessary = dynamic_cast<const LADataString &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();
		// isfwdinterpolation
		bool isFWDInter = false;
		LAPriceDataInterpolation *fwdInter = 0;
		dh = &(data[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
			fwdInter = &dynamic_cast<LAPriceDataInterpolation &>(data[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		}
		if (isFWDInter)
		{
			LAPriceDataDayCount dc;
			LAPriceDataCalendar cal;
			LAPriceDataSlidingRule sld;
			LAString accessory;
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
		const LAString &termMax = dynamic_cast<const LADataString &>((data.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
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
			LAString msg = "frequency is wrong";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// set roll convention
		LAString roll_conv("");
		if (c_freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
		else if (eom) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;
	
		// get interpolation method
		LAPriceDataInterpolation &b_inter = getInterpolation_bs();
		DoubleArray b_t_grid(1, 0.0);
		DoubleArray b_termstruct_grid(1, 0.0);
		LAStringVector basisTerms;

		for (unsigned int i = 0; i < data.size(); i++)
		{
			const LAString &strTerm  = dynamic_cast<const LADataString &>((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			basisTerms.push_back(strTerm);

			double rate = dynamic_cast<const LADataDouble &>((data[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			double term = 0.0;
			LADate tmpDate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
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
			LAString strTerm = LAString(static_cast<int>(mUnit * (i - 1))) + LAString("M");
			LAString dfstrTerm = LAString(static_cast<int>(mUnit * i)) + LAString("M");
			LADate fdate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			LADate ldate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true, &roll_conv);
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

			const LAString &strTerm  = dynamic_cast<const LADataString &>((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			LADate matudate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			ret.push_back(c_spotdate);

			//find dvzeroDates
			unsigned int pos=0;
			if (!LAAlgorithm::find<DateVector,LADate>(dvzeroDates,matudate,0,dvzeroDates.size()-1 ,pos))
			{
				//this means basis grid e.x. 100y is longer than DF1 max grid, which ofcourse can not be calculated.
				if (matudate > dvzeroDates.back())
					continue;
				else
					throw LACoreInvalidData("BasisGrid Search Error",__FILE__,__LINE__);
			}

			ret.insert(ret.end(),dvzeroDates.begin(),dvzeroDates.begin()+pos+1);
			if(!data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO,NOCHECK).isDefined())
			{
				data[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new LADataDates(ret));
			}
			else
			{
				dynamic_cast<LADataDates&>(data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(ret);
			}
		}

		// cashlet
		objHolder.remove(IR_CALIBRATION_DATA_CASHLETCALENDAR + LAString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_CASHLETCALENDAR + LAString("_") + curveName, new LAPriceDataCalendar(c_cal));
		objHolder.remove(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE + LAString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE + LAString("_") + curveName, new LAPriceDataSlidingRule(c_sld));
		objHolder.remove(IR_CALIBRATION_DATA_CASHLETSPOTDATE + LAString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_CASHLETSPOTDATE + LAString("_") + curveName, new LADataDate(c_spotdate));
		objHolder.remove(IR_CALIBRATION_DATA_XCCYBASISTERM + LAString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_XCCYBASISTERM + LAString("_") + curveName, new LADataStrings(basisTerms));
		objHolder.remove(IR_CALIBRATION_DATA_ISEOMROLL + LAString("_") + curveName);
		objHolder.add(IR_CALIBRATION_DATA_ISEOMROLL + LAString("_") + curveName, new LADataBool(eom));

		const LADataHolder *dh = &data[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& adjustValue_term = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			objHolder.remove(IR_CALIBRATION_DATA_ADJUSTVALUETERM + LAString("_") + curveName);
			objHolder.add(IR_CALIBRATION_DATA_ADJUSTVALUETERM + LAString("_") + curveName, new LADataDoubles(adjustValue_term));

			const DoubleArray& adjustValue 
				= dynamic_cast<const LADataDoubles&>(data[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL).get()).get();
			objHolder.remove(IR_CALIBRATION_DATA_ADJUSTVALUE + LAString("_") + curveName);
			objHolder.add(IR_CALIBRATION_DATA_ADJUSTVALUE + LAString("_") + curveName, new LADataDoubles(adjustValue));

			const LAPriceDataInterpolation& interpAtt
				= dynamic_cast<const LAPriceDataInterpolation&>(data[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get());
			objHolder.remove(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION + LAString("_") + curveName);
			objHolder.add(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION + LAString("_") + curveName, new LAPriceDataInterpolation(interpAtt));
		}
	}
	else
	{
		// get market data
		const LADataMultiReference& mr = getMarketData();
		vector<LAObject*> data;
		for(unsigned int i = 0; i < mr.getSize(); i++)
		{
			// check use grid
			const LADataHolder *dh = &mr.get(i).get().getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const LADataBool &>(dh->get()).get()) continue;

			LAString datatype_str = dynamic_cast<const LADataString&> ((mr.get(i).get().getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			datatype_str.toUpper();
			if (datatype_str == PAR) data.push_back(&mr.get(i).get());
		}
		// data exist check
		if (data.empty())
		{
			LAString msg = "swap market is not set.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// sort
		sort(data.begin(), data.end(), Comp_term());
		//eom roll
		bool eom = false;
		dh = &data[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
		if (dh->isDefined() && !dh->isNull()) 
			eom = dynamic_cast<const LADataBool &>(dh->get()).get();
		// cashlet
		const LAPriceDataDayCount &c_dc = dynamic_cast<const LAPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL).get());
		const LAPriceDataCalendar &c_cal = dynamic_cast<const LAPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const LAPriceDataSlidingRule &c_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const LADate &c_spotdate = dynamic_cast<const LADataDate &> (data[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
		LAString c_freq = dynamic_cast<const LADataString &>(data[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL).get()).get();
		const int c_lag  = dynamic_cast<const LADataInt &>(data[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get());
		// index
		const LAPriceDataDayCount &i_dc = dynamic_cast<const LAPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL).get());
		const LAPriceDataCalendar &i_fixcal = dynamic_cast<const LAPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const LAPriceDataCalendar &i_paycal = dynamic_cast<const LAPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const LAPriceDataSlidingRule &i_sld  = dynamic_cast<const LAPriceDataSlidingRule &>(data[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const int i_lag  = dynamic_cast<const LADataInt &>(data[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get());
		LAString i_freq = dynamic_cast<const LADataString &>(data[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL).get()).get();
		// isfwdinterpolation
		bool isFWDInter = false;
		LAPriceDataInterpolation *fwdInter = 0;
		dh = &(data[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
			fwdInter = &dynamic_cast<LAPriceDataInterpolation &>(data[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		}
		if (isFWDInter)
		{
			LAPriceDataDayCount dc;
			LAPriceDataCalendar cal;
			LAPriceDataSlidingRule sld;
			LAString accessory;
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
		const LAString &termMax = dynamic_cast<const LADataString &>((data.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
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
			LAString msg = "frequency is wrong";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		LAString i_accessary = LAString(static_cast<int > (mUnit)) + LAString("M");
		// set roll convention
		LAString roll_conv("");
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
			LAString strTerm = LAString(static_cast<int>(mUnit * (i - 1))) + LAString("M");
			LAString dfstrTerm = LAString(static_cast<int>(mUnit * i)) + LAString("M");
			LADate fdate = LAMathDateCalculations::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			LADate ldate = LAMathDateCalculations::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true, &roll_conv);
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

	objHolder.remove(CALIBRATION_DATA_TERMS + LAString("_") + curveName);
	objHolder.remove(IR_CALIBRATION_DATA_DFS + LAString("_") + curveName);
	objHolder.add(CALIBRATION_DATA_TERMS + LAString("_") + curveName, new LADataDoubles(terms));
	objHolder.add(IR_CALIBRATION_DATA_DFS + LAString("_") + curveName, new LADataDoubles(dfs));
}

/*!
    @brief check whether dataValues of curves exist or not

	@param[in] market name
*/
bool 
LAMathYieldCurvePro::checkCurveAttr(const LAString& mktName) const
{
	bool ret = true;
	const LADataHolder *dh;
	const LADataReference& ref = getYieldData();
	LAObjectHolder objHolder = ref.get();
	for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (it->second == mktName)
		{
			const LAString& curveName = it->first;
			LAString suffix = "";
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
LAMathYieldCurvePro::removeAllCuveData(LAObject &yieldData) const
{
	for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (mNonRemovableMarket.end() != mNonRemovableMarket.find(it->second))
		{
			continue;
		}
		const LAString& curveName = it->first;
		LAString suffix = "";
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
LAMathYieldCurvePro::removeCuveData(LAObject &yieldData, const LAString& mktName) const
{
	if (mNonRemovableMarket.end() != mNonRemovableMarket.find(mktName))
	{
		LAString msg = "This market cannot be removed. market = " + mktName;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (it->second == mktName)
		{
			const LAString& curveName = it->first;
			LAString suffix = "";
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
LAMathYieldCurvePro::removeBasisCuveData(LAObject &yieldData) const
{
	const LADataHolder *dh = &getData(IR_CALIBRATION_DATA_BASISDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector &basisDFs = dynamic_cast<const LADataStrings &>(dh->get()).get();		
		for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
		{
			if (mNonRemovableMarket.end() != mNonRemovableMarket.find(it->second))
			{
				continue;
			}
			if (basisDFs.end() != std::find(basisDFs.begin(), basisDFs.end(), it->first))
			{
				const LAString& curveName = it->first;
				LAString suffix = "";
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
LAMathYieldCurvePro::getForwardConvention(const LAString &curveName, LAPriceDataDayCount &dc, LAPriceDataSlidingRule &sld, LAPriceDataCalendar &cal, LAString &accessary) const
{
	const LAString def_accessary = "1Y";
	const LADataHolder *dh = 0;
	if (mBCurveGenMap.find(curveName) != mBCurveGenMap.end())
	{
		LAString suffix = "_" + getMarketForCurve(curveName);
		dh = &getData(CALIBRATION_DATA_MARKETDATA + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{

			const LADataMultiReference& mr = dynamic_cast<const LADataMultiReference&>(dh->get());
			vector<LAObject*> data_basis;
			for (unsigned int i = 0; i < mr.getSize(); ++i)
			{
				LAString datatype_str = dynamic_cast<const LADataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
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
				LAString msg = "Basis Data is empty. curveName = " + curveName;
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			dc = dynamic_cast<const LAPriceDataDayCount &>(data_basis[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
			sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_basis[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
			// calendar get from cashflow info, because we use frn method.
			cal = dynamic_cast<const LAPriceDataCalendar &>(data_basis[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
			accessary = dynamic_cast<const LADataString &>(data_basis[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();
		}
		else
		{
			accessary = def_accessary;
		}
	}
	else
	{
		LAString suffix = "";
		const LAString market = getMarketForCurve(curveName);
		if (!getIsArbFree() && market != SWAP)
		{
			suffix = "_" + market;
		}

		dh = &getData(CALIBRATION_DATA_MARKETDATA + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			const LADataMultiReference& mr = dynamic_cast<const LADataMultiReference&>(dh->get());

			vector<LAObject*> data_libor;
			vector<LAObject*> data_swap;
			for (unsigned int i = 0; i < mr.getSize(); ++i)
			{
				LAString datatype_str = dynamic_cast<const LADataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
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
				//LAString msg = "Libor or Swap Data is empty. curveName = " + curveName;
				//throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			dc = dynamic_cast<const LAPriceDataDayCount &>(data_libor[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
			sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_libor[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
			cal = dynamic_cast<const LAPriceDataCalendar &>(data_libor[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());

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
				LAString freq;
				dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					freq = dynamic_cast<const LADataString &>(dh->get()).get();
				}
				else
				{
					dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
					if (dh->isDefined() && !dh->isNull())
					{
						freq = dynamic_cast<const LADataString &>(dh->get()).get();
					}
					else
					{
						freq = dynamic_cast<const LADataString &>(data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, NOCHECK).get()).get();
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
					LAString msg = "frequency is wrong";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
LAMathYieldCurvePro::insertNonRemovableMarket(const LAString& mktName)
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
LAMathYieldCurvePro::eraseNonRemovableMarket(const LAString& mktName)
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
LAMathYieldCurvePro::setCurveConvention(LAObjectHolder& objHolder,
									  std::vector<LAObject*>& mktData,
									  const LAString& curveName)
{
	LAString suffix = "";
	if (curveName != STD) suffix = "_" + curveName;
	
	if (mktData.size() == 0)
	{
		throw LACoreInvalidData("Swap size must be more than one", __FILE__, __LINE__);
	}

	LAString freq = dynamic_cast<const LADataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	const LAPriceDataCalendar* cal = &dynamic_cast<const LAPriceDataCalendar&> ((mktData[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL)).get());
	const LAPriceDataSlidingRule* sld  = &dynamic_cast<const LAPriceDataSlidingRule &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
	const LAPriceDataDayCount* dc = &dynamic_cast<const LAPriceDataDayCount &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
	LAString accessary = dynamic_cast<const LADataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

	objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
	objHolder.remove(CALIBRATION_DATA_CALENDAR + suffix);
	objHolder.remove(CALIBRATION_DATA_SLIDINGRULE + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
	
	objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new LADataString(freq));
	objHolder.add(CALIBRATION_DATA_CALENDAR + suffix, new LAPriceDataCalendar(*cal));
	objHolder.add(CALIBRATION_DATA_SLIDINGRULE + suffix, new LAPriceDataSlidingRule(*sld));
	objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT + suffix, new LAPriceDataDayCount(*dc));
	objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new LADataString(accessary));
}	

/*
	@Desctiption calculate an annuity

	@param spotTerm [in] spot term
	@param terms_grid [in] terms of payment timings of the annuity
	@param terms_interval [in] terms of calculation periods of the annuity
	@param d_inter [in] discount factor
	@return annuity
*/
double LAMathYieldCurvePro::calcAnnuity(double spotTerm, const DoubleArray& terms_grid, const DoubleArray& terms_interval, const LAPriceDataInterpolation& d_inter)
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
void LAMathYieldCurvePro::deducePaymentDatesAndTerms(const LADate& spotdate, const LAString& strTerm, const LAPriceDataSlidingRule& sld, const LAPriceDataCalendar& cal,
	const LAString& roll_conv, const LAString& freq, const LAString& freq_payment, const LAPriceDataDayCount& dc, bool eom, bool isBackward,
	DateVector& dates, DoubleArray& terms_grid, DoubleArray& terms_interval, IntArray& num_reset)
{
	LADate end;
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
		LADate end_unadjust = LAMathDateCalculations::getDate(spotdate, strTerm, true);
		DateVector reset_dates;
		LAString roll_conv_payment(LAPriceYieldGenerator::deduceRollConvention(freq_payment, eom));
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
			throw LACoreInvalidData("There are no payment date in the target basis swap.", __FILE__, __LINE__);
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
		throw LACoreInvalidData("Total number of fixing and total number of coupon reset time must be match.", __FILE__, __LINE__);
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
void LAMathYieldCurvePro::deducePaymentDatesAndTermsThisSide(const LAString& strTerm, const LAString& roll_conv, bool eom, bool isSameGridIndex,
	const LADate& c_spotdate, const LAPriceDataSlidingRule& c_sld, const LAPriceDataCalendar& c_cal, const LAString& c_freq, const LAString& c_freq_payment, const LAPriceDataDayCount& c_dc, bool c_isBackward, const LADate& a_c_spotdate,
	DateVector& dates, DoubleArray& terms_grid, DoubleArray& terms_interval, IntArray& num_reset)
{
	if (c_spotdate < a_c_spotdate && !isSameGridIndex)
	{
		// roll from a_c_spotdate
		deducePaymentDatesAndTerms(a_c_spotdate, strTerm, c_sld, c_cal, roll_conv, c_freq, c_freq_payment, c_dc, eom, c_isBackward, dates, terms_grid, terms_interval, num_reset);
		// calc odd date
		LAPriceDataDayCount dc_act(ACT_365_ISDA);
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
