//
// CurveCalibrationData.h
// This file was previously called YieldCurvePro.h and before that AQLMathYieldCurvePro.h
//
#include "CurveCalibrationData.h"

#include <functional>
#include <algorithm>
#include <cmath>
#include <boost/math/special_functions/round.hpp>

#include "ExceptionMacros.h"
#include "AQLDefinitions.h"
#include "AQLMathDefine.h"
#include "AQLBasic.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataManager.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLDataMultiReference.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataFunction.h"
#include "AQLDataInstance.h"
#include "AQLOptimumBrent.h"
#include "AQLAlgorithm.h"
#include "AQLMatrix.h"
#include "AQLMathFXEntity.h"

#include "InitializeAQETrading.h"
#include "LADateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "CurveInstruments.h"
#include "AQLLinearInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLObjectHolder.h"
#include "DataUtilities.h"              // For AQ_TO_STRING macros

using etrading::DiscountFactors;
using etrading::bootstrapLibors;
using etrading::bootstrapFRAs;

namespace
{
	/* @brief			Convert the Frequency to Tenor
	* @param [in]		freqquency	Frequencey
	* @output			Term: 1Y, 6M, 3M, 1M, 1W
	*/
	AQLString fromFrequencyToTerm(const AQLString& frequency)
	{
		AQLString term;
		AQLString freq = AQLString(frequency).toUpper();

		if (freq == "ANNUAL")
		{
			term = "1Y";
		}
		else if (freq == "SEMI-ANNUAL")
		{
			term = "6M";
		}
		else if (freq == "QUARTERLY")
		{
			term = "3M";
		}
		else if (freq == "MONTHLY")
		{
			term = "1M";
		}
		else if (freq == "WEEKLY")
		{
			term = "1W";
		}
		else if (freq == "BUSINESS_DAYS" || freq == "DAILY")
		{
			term = "1D";
		}
		else
		{
			throw AQLCoreInvalidData("#Error: Frequency, must be Annual, Semi-Annual, Quarterly, Monthly, Weekly, or Business_Days.", __FILE__, __LINE__);
		}

		return term;
	}

	/*!
		@brief set cut-off date to linear spline interpolator
		@param[out] inter				Interpolator object
		@param[out] yieldData			YieldData object
		@param[in]  indexName			Name of curve where interpolation is being set
		@param[in]  noCurveTypeCheck	Check curve type or not
	*/
	void setLinearSplineCutoffDate(AQLPriceDataInterpolation& inter, AQLObject& yieldData, const AQLString& indexName, const CurveCalibrationData* curveCalibrationData, bool noCurveTypeCheck = false)
	{
		// Only set linear spline cut off date on swap curve
		if (inter.isHybrid())
		{
			// Get curve type
			AQLString curve = curveCalibrationData->getMarketForCurve(indexName);
			AQLString suffix = (curve == SWAP) ? "" : AQLString("_") + curve;

			AQLString curveType;
			AQLDataHolder* dh = &yieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
			if (dh->isDefined() && !dh->isNull())
			{
				curveType = dynamic_cast<AQLDataString&>(dh->get()).get();
			}

			if (curveType == SWAP || curveType == "BASIS" || curveType == "TENORBASIS" || curveType == "OIS" || curveType == "ARR" || noCurveTypeCheck)
			{
				const AQLDataHolder* cutOffAttr = &(yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK));
				if (cutOffAttr->isDefined() && !cutOffAttr->isNull())
				{
					double cuttOffDate = dynamic_cast<const AQLDataDouble&> (cutOffAttr->get()).get();
					inter.setJoinDateAsDouble(cuttOffDate);
				}
				else
				{
					const AQLDataHolder* cutOffAttr = &(yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, NOCHECK));
					if (cutOffAttr->isDefined() && !cutOffAttr->isNull())
					{
						double cuttOffDate = dynamic_cast<const AQLDataDouble&> (cutOffAttr->get()).get();
						inter.setJoinDateAsDouble(cuttOffDate);
					}
					else
					{
						AQLString err = "#Error: Cut off date is not defined when using Linear Spline interpolation on curve '";
						err += indexName;
						err += "'. Is the correct interpolation method being used?";
						throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
					}
				}
			}
			else
			{
				AQLString msg = "#Error: Curve '" + indexName + "' is NOT a swap or basis curve and won't work with linear spline interpolation.";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}
}

// Constructor
CurveCalibrationData::CurveCalibrationData()
	: LACurvePricingObject()
{
}

//====================================================================
/*!
	@brief constructor

	@param[in] dataInstance pointer of AQLDataInstance

*/
CurveCalibrationData::CurveCalibrationData(AQLDataInstance* dataInstance)
	: LACurvePricingObject(dataInstance)//,mCurveVersion(0)
{
	setDataInstance(dataInstance);
	AQLPriceDataManager& dm = dataInstance->getDataMaster();

	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(CALIBRATION_DATA_CURVEGENERATOR, DATA_PROCEDURE);
	dm.setData(CALIBRATION_DATA_MARKETDATA, DATA_MULTIREFERENCE);
	dm.setData(IR_CALIBRATION_DATA_BASISRATES, DATA_DOUBLE_MATRIX);
	dm.setData(IR_CALIBRATION_DATA_BASISDATA, DATA_MULTIREFERENCE);
	dm.setData(IR_CALIBRATION_DATA_ISFUTUREUSE, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_BASEYIELDCURVE, DATA_REFERENCE);
	dm.setData(IR_CALIBRATION_DATA_CURRENCY, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_INTERPOLATIONYG, DATA_INTERPOLATION);
	dm.setData(IR_CALIBRATION_DATA_INTERPOLATIONFW, DATA_INTERPOLATION);
	dm.setData(IR_CALIBRATION_DATA_INTERPOLATIONBS, DATA_INTERPOLATION);
	dm.setData(IR_CALIBRATION_DATA_BASISFUNCTION, DATA_FUNCTION);
	dm.setData(PRICING_DATA_RATEPRIORITY, DATA_STRINGS);
	dm.setData(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_ISDISCOUNTCURVE, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_GENERATEDFS, DATA_STRINGS);
	dm.setData(IR_CALIBRATION_DATA_ISARBFREE, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_ISFRAUSE, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_FOREIGNYIELDDATA, DATA_REFERENCE);
	dm.setData(IR_CALIBRATION_DATA_FXENTITY, DATA_REFERENCE);
	dm.setData(IR_CALIBRATION_DATA_COLYIELDDATA, DATA_REFERENCE);
	dm.setData(IR_CALIBRATION_DATA_ISFWDFX, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_ISFWDBASIS, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_ISDATE, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_STARTTERM, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_TENOR, DATA_STRING);

	mpAsOfDate = &add(CALIBRATION_DATA_ASOFDATE);
	mpProcedure = &add(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData = &add(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates = &add(IR_CALIBRATION_DATA_BASISRATES);
	mpBasisData = &add(IR_CALIBRATION_DATA_BASISDATA);
	mpIsFutureUse = &add(IR_CALIBRATION_DATA_ISFUTUREUSE);
	mpBaseYieldCurve = &add(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency = &add(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG = &add(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW = &add(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS = &add(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction = &add(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority = &add(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree = &add(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse = &add(IR_CALIBRATION_DATA_ISFRAUSE);
	mpIsSwapTenorAdjust = &add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);
	mpForeignYieldData = &add(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);
	mpFXEntity = &add(IR_CALIBRATION_DATA_FXENTITY);
	mpColYieldData = &add(IR_CALIBRATION_DATA_COLYIELDDATA);
}
/*!
	@brief copy constructor

	@param[in] curve original object
*/
CurveCalibrationData::CurveCalibrationData(
	const CurveCalibrationData& curve) :
	LACurvePricingObject(curve)//,mCurveVersion(0)
{
	mpAsOfDate = &getData(CALIBRATION_DATA_ASOFDATE);
	mpProcedure = &getData(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData = &getData(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates = &getData(IR_CALIBRATION_DATA_BASISRATES);
	mpBasisData = &getData(IR_CALIBRATION_DATA_BASISDATA);
	mpIsFutureUse = &getData(IR_CALIBRATION_DATA_ISFUTUREUSE);
	mpBaseYieldCurve = &getData(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency = &getData(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG = &getData(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW = &getData(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS = &getData(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction = &getData(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority = &getData(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree = &getData(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse = &getData(IR_CALIBRATION_DATA_ISFRAUSE);
	mpIsSwapTenorAdjust = &getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);
	mpForeignYieldData = &getData(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);
	mpFXEntity = &getData(IR_CALIBRATION_DATA_FXENTITY);
	mpColYieldData = &getData(IR_CALIBRATION_DATA_COLYIELDDATA);
}
/*!
	@brief destructor
*/
CurveCalibrationData::~CurveCalibrationData()
{
}

// QUERY
/*!
	@brief get EntityType

	@return EntityType
*/
object_t
CurveCalibrationData::getType(void) const
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
CurveCalibrationData::isTypeOf(object_t id) const
{
	return (id == ENTITY_IRYCPRO ? true : LACurvePricingObject::isTypeOf(id));
}

/*!
	@brief get AsOfDate
	@return AsOfDate
*/
const AQLDataDate&
CurveCalibrationData::getAsOfDate(void) const
{
	return dynamic_cast<const AQLDataDate&>(mpAsOfDate->get());
}
/*!
	@brief get AsOfDate
	@return AsOfDate
*/
AQLDataDate&
CurveCalibrationData::getAsOfDate(void)
{
	return dynamic_cast<AQLDataDate&>(mpAsOfDate->get());
}

/*!
	@brief get IsArbFree
	@return IsArbFree
*/
const AQLDataBool&
CurveCalibrationData::getIsArbFree(void) const
{
	return dynamic_cast<const AQLDataBool&>(mpIsArbFree->get());
}
/*!
	@brief get IsArbFree
	@return IsArbFree
*/
AQLDataBool&
CurveCalibrationData::getIsArbFree(void)
{
	return dynamic_cast<AQLDataBool&>(mpIsArbFree->get());
}

/*!
	@brief get MarketData

	@return MarketData
*/
const AQLDataMultiReference&
CurveCalibrationData::getMarketData() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpMarketData->get());
}
/*!
	@brief get MarketData

	@return MarketData
*/
AQLDataMultiReference&
CurveCalibrationData::getMarketData()
{
	return dynamic_cast<AQLDataMultiReference&>(mpMarketData->get());
}
/*!
	@brief get BasisRates

	@return BasisRates
*/

const AQLDataDoubleMatrix&
CurveCalibrationData::getBasisRates() const
{
	return dynamic_cast<const AQLDataDoubleMatrix&>(mpBasisRates->get());
}

/*!
	@brief get BasisRates and set BasisRates

	@return BasisRates
*/
AQLDataDoubleMatrix&
CurveCalibrationData::getBasisRates()
{
	return dynamic_cast<AQLDataDoubleMatrix&>(mpBasisRates->get());
}

/*!
	@brief get BasisData

	@return BasisData
*/

const AQLDataMultiReference&
CurveCalibrationData::getBasisData() const
{
	return dynamic_cast<const AQLDataMultiReference&>(mpBasisData->get());
}

/*!
	@brief get BasisData and set BasisData

	@return BasisData
*/
AQLDataMultiReference&
CurveCalibrationData::getBasisData()
{
	return dynamic_cast<AQLDataMultiReference&>(mpBasisData->get());
}


/*!
	@brief get Use FutureRate or not

	@return true : use FutureRate
			false: don't use FutureRate
*/

const AQLDataBool&
CurveCalibrationData::getIsFutureUse() const
{
	return dynamic_cast<const AQLDataBool&>(mpIsFutureUse->get());
}

/*!
	@brief get Use FutureRate or not and set Use FutureRate or not

	@return true : use FutureRate
			false: don't use FutureRate
*/
AQLDataBool&
CurveCalibrationData::getIsFutureUse()
{
	return dynamic_cast<AQLDataBool&>(mpIsFutureUse->get());
}

/*!
	@brief get Use FutureRate or not

	@return true : use FutureRate
			false: don't use FutureRate
*/

const AQLDataBool&
CurveCalibrationData::getIsFRAUse() const
{
	return dynamic_cast<const AQLDataBool&>(mpIsFRAUse->get());
}

/*!
	@brief get Use FutureRate or not and set Use FutureRate or not

	@return true : use FutureRate
			false: don't use FutureRate
*/
AQLDataBool&
CurveCalibrationData::getIsFRAUse()
{
	return dynamic_cast<AQLDataBool&>(mpIsFRAUse->get());
}

/*!
	@brief get BaseYieldCurve

	@return BaseYieldCurve
*/

const AQLDataReference&
CurveCalibrationData::getBaseYieldCurve() const
{
	return dynamic_cast<const AQLDataReference&>(mpBaseYieldCurve->get());
}

/*!
	@brief get BaseYieldCurve and set BaseYieldCurve

	@return BaseYieldCurve
*/
AQLDataReference&
CurveCalibrationData::getBaseYieldCurve()
{
	return dynamic_cast<AQLDataReference&>(mpBaseYieldCurve->get());
}

/*!
	@brief get Interpolation to generate yield curve

	@return Interpolation to generate yield curve
*/
const AQLPriceDataInterpolation&
CurveCalibrationData::getInterpolation_yg() const
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInterYG->get());
}

/*!
	@brief get Interpolation to generate yield curve

	@return Interpolation to generate yield curve
*/
AQLPriceDataInterpolation&
CurveCalibrationData::getInterpolation_yg()
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInterYG->get());
}

/*!
	@brief get Interpolation to use future

	@return Interpolation to use future
*/
const AQLPriceDataInterpolation&
CurveCalibrationData::getInterpolation_fw() const
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInterFW->get());
}

/*!
	@brief get Interpolation to use future

	@return Interpolation to use future
*/
AQLPriceDataInterpolation&
CurveCalibrationData::getInterpolation_fw()
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInterFW->get());
}

/*!
	@brief get Interpolation to set basis rates

	@return Interpolation to set basis rates
*/
const AQLPriceDataInterpolation&
CurveCalibrationData::getInterpolation_bs() const
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInterBS->get());
}

/*!
	@brief get Interpolation to set basis rates

	@return Interpolation to set basis rates
*/
AQLPriceDataInterpolation&
CurveCalibrationData::getInterpolation_bs()
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInterBS->get());
}

/*!
	@brief get Basis Function

	@return Basis Function
*/
const AQLPriceDataFunction&
CurveCalibrationData::getBasisFunction() const
{
	return dynamic_cast<const AQLPriceDataFunction&>(mpBasisFunction->get());
}

/*!
	@brief get Basis Function

	@return Basis Function
*/
AQLPriceDataFunction&
CurveCalibrationData::getBasisFunction()
{
	return dynamic_cast<AQLPriceDataFunction&>(mpBasisFunction->get());
}

AQLDataStrings&
CurveCalibrationData::getRatePriority()
{
	return dynamic_cast<AQLDataStrings&>(mpRatePriority->get());
}

const AQLDataStrings&
CurveCalibrationData::getRatePriority() const
{
	return dynamic_cast<AQLDataStrings&>(mpRatePriority->get());
}

/*!
	@brief get FXEntity

	@return FXEntity
*/
const AQLDataReference&
CurveCalibrationData::getFXEntity() const
{
	return dynamic_cast<const AQLDataReference&>(mpFXEntity->get());
}

/*!
	@brief get FXEntity and set FXEntity

	@return FXEntity
*/
AQLDataReference&
CurveCalibrationData::getFXEntity()
{
	return dynamic_cast<AQLDataReference&>(mpFXEntity->get());
}

// OPERATION
/*!
	@brief set Interpolation

	@param[in] a	pointer of Interpolation function
	@param[in] name name of Interpolation
*/
void
CurveCalibrationData::setInterpolation(
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
CurveCalibrationData::setInterpolation(const AQLString& name)
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
CurveCalibrationData::setDFGenerator(const AQLCoreProcedure* a,
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
CurveCalibrationData::setDFGenerator(const AQLString& name)
{
	dynamic_cast<AQLDataProcedure*>(&(
		getData(CALIBRATION_DATA_CURVEGENERATOR).get()))->setMethod(name);
}

/*!
	@brief clone this class

	@return	pointer of this class
*/
AQLObject*
CurveCalibrationData::clone() const
{
	try {
		CurveCalibrationData*	pCurve = new CurveCalibrationData(*this);
		return pCurve;
	}
	catch (std::bad_alloc e) {
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}
void
CurveCalibrationData::remove(
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
	LACurvePricingObject::remove(dataName);
}

void
CurveCalibrationData::reset(void)
{
	LACurvePricingObject::reset();
	mpAsOfDate = &add(CALIBRATION_DATA_ASOFDATE);
	mpProcedure = &add(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData = &add(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates = &add(IR_CALIBRATION_DATA_BASISRATES);
	mpBasisData = &add(IR_CALIBRATION_DATA_BASISDATA);
	mpIsFutureUse = &add(IR_CALIBRATION_DATA_ISFUTUREUSE);
	mpBaseYieldCurve = &add(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency = &add(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG = &add(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW = &add(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS = &add(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction = &add(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority = &add(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree = &add(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse = &add(IR_CALIBRATION_DATA_ISFRAUSE);
	mpIsSwapTenorAdjust = &add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);
	mpForeignYieldData = &add(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);
	mpFXEntity = &add(IR_CALIBRATION_DATA_FXENTITY);
	mpColYieldData = &add(IR_CALIBRATION_DATA_COLYIELDDATA);
}

/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy object

	@param[in] e copy object
	@return Reference of this object
*/
AQLObject&
CurveCalibrationData::copy(
	const AQLObject& e)
{
	if (this == &e)
	{
		return *this;
	}

	LACurvePricingObject::copy(e);
	if (!e.isTypeOf(ENTITY_IRYCPRO))
	{
		AQLString err = "Assignement error for CurveCalibrationData : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mpAsOfDate = &getData(CALIBRATION_DATA_ASOFDATE);
	mpProcedure = &getData(CALIBRATION_DATA_CURVEGENERATOR);
	mpMarketData = &getData(CALIBRATION_DATA_MARKETDATA);
	mpBasisRates = &getData(IR_CALIBRATION_DATA_BASISRATES);
	mpBasisData = &getData(IR_CALIBRATION_DATA_BASISDATA);
	mpIsFutureUse = &getData(IR_CALIBRATION_DATA_ISFUTUREUSE);
	mpBaseYieldCurve = &getData(IR_CALIBRATION_DATA_BASEYIELDCURVE);
	mpCurrency = &getData(IR_CALIBRATION_DATA_CURRENCY);
	mpInterYG = &getData(IR_CALIBRATION_DATA_INTERPOLATIONYG);
	mpInterFW = &getData(IR_CALIBRATION_DATA_INTERPOLATIONFW);
	mpInterBS = &getData(IR_CALIBRATION_DATA_INTERPOLATIONBS);
	mpBasisFunction = &getData(IR_CALIBRATION_DATA_BASISFUNCTION);
	mpRatePriority = &getData(PRICING_DATA_RATEPRIORITY);
	mpIsArbFree = &getData(IR_CALIBRATION_DATA_ISARBFREE);
	mpIsFRAUse = &getData(IR_CALIBRATION_DATA_ISFRAUSE);
	mpIsSwapTenorAdjust = &getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST);
	mpForeignYieldData = &getData(IR_CALIBRATION_DATA_FOREIGNYIELDDATA);
	mpColYieldData = &getData(IR_CALIBRATION_DATA_COLYIELDDATA);

	return *this;
}

/*!
	@brief Set data by name

	@param[in] name name of Data

	@return Reference of AQLDataHolder include this Data

*/
AQLDataHolder&
CurveCalibrationData::add(const AQLString& name)
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
CurveCalibrationData::reset(const AQLString& name)
{
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::reset(name, dh);
}

/*!
	@brief calc DF from marketData

	@param[in] asof As of Date
*/
void
CurveCalibrationData::calcDiscountFactor(const AQLDate& asof)
{
	try
	{
		if (getAsOfDate().get() != asof)
		{
			getAsOfDate() = asof;
		}
		dynamic_cast<AQLDataProcedure&>(mpProcedure->get()).calibrateModel(asof);
	}
	catch (AQLCoreError& e)
	{
		AQLString msg("#Error: DiscountFactor Calculation Error at ");
		msg += getName();
		AQLCoreInvalidData err(msg.getCString(), __FILE__, __LINE__);
		err += e;
		throw err;
	}
	catch (...)
	{
		AQLString msg("#Error: DiscountFactor Calculation at ");
		msg += getName().get();
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}
/*!
	@brief set DF By ShiftZero

	@param[in] width width of Shift Zero

*/
void
CurveCalibrationData::setDFByShiftZero(double width, unsigned int pos, const UintArray& grids)
{
	unsigned int i, j, i_l, i_r;

	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject& YieldData = objHolder.get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get());

	AQLDataDoubles rates;
	rates.resize(terms.getSize());
	for (i = 0; i < terms.getSize(); i++)
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
		{
			rates.set(rates[j] + width, j);
		}

		if (grids.size() == 1)
		{
			for (j = i + 1; j < rates.getSize(); j++)
			{
				rates.set(rates[j] + width, j);
			}
		}
		else
		{
			i_r = grids[pos + 1];
			for (j = i + 1; j < i_r; j++)
			{
				rates.set(rates[j] + width * (terms[i_r] - terms[j]) / (terms[i_r] - terms[i]), j);
			}
		}
	}
	else if (pos == grids.size() - 1)
	{
		for (j = i + 1; j < rates.getSize(); j++)
		{
			rates.set(rates[j] + width, j);
		}

		i_l = grids[pos - 1];
		for (j = i_l + 1; j < i; j++)
		{
			rates.set(rates[j] + width * (terms[j] - terms[i_l]) / (terms[i] - terms[i_l]), j);
		}
	}
	else
	{
		i_l = grids[pos - 1];
		i_r = grids[pos + 1];
		for (j = i_l + 1; j < i; j++)
		{
			rates.set(rates[j] + width * (terms[j] - terms[i_l]) / (terms[i] - terms[i_l]), j);
		}

		for (j = i + 1; j < i_r; j++)
		{
			rates.set(rates[j] + width * (terms[i_r] - terms[j]) / (terms[i_r] - terms[i]), j);
		}
	}

	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);
	for (i = 0; i < terms.getSize(); i++)
		rates.set(1 / AQLPriceDataConvention::rateToRet(rates[i], terms[i], conv), i);

	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new AQLDataDoubles(rates));
}

/*!
	@brief set DF By parallel ShiftZero

	@param[in] width width of Shift Zero

*/
void
CurveCalibrationData::setDFByShiftZero(double width, FloorType type, double floor)
{
	unsigned int i;

	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject& YieldData = objHolder.get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL)).get());

	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);

	AQLDataDoubles rates;
	rates.resize(terms.getSize());

	for (i = 0; i < terms.getSize(); i++)
	{
		rates.set(getZeroRate(terms[i]), i);
	}
	for (i = 0; i < rates.getSize(); i++)
	{
		double rnew;
		switch (type)
		{
		case NOFLOOR:
			rnew = rates[i] + width;
			break;
		case NORMAL:
			rnew = AQLMath::max(rates[i] + width, floor);
			if (width < 0 && rnew > rates[i])
			{
				rnew = rates[i];
			}
			break;
		case SYMMETRIC:
			if (width < 0)
			{
				rnew = AQLMath::min(rates[i], AQLMath::max(rates[i] + width, floor));
			}
			else
			{
				rnew = AQLMath::min(rates[i], AQLMath::max(rates[i] - width, floor));
				rnew = 2 * rates[i] - rnew;
			}
			break;
		default:
			throw AQLCoreInvalidData("#Error: Not Supported Floor Type", __FILE__, __LINE__);
		}
		rates.set(1 / AQLPriceDataConvention::rateToRet(rnew, terms[i], conv), i);
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new AQLDataDoubles(rates));
}

/*!
	@brief set BasisDF By ShiftZero

	@param[in] width width of Shift Zero

*/
void
CurveCalibrationData::setBasisDFByShiftZero(double width, unsigned int pos, const UintArray& grids)
{
	AQLObjectHolder &eh_y = getYieldData().get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> (eh_y.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL).get());

	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);

	const unsigned int termSize = terms.getSize();
	AQLDataDoubles rates;
	rates.resize(termSize);
	for (unsigned int i = 0; i < termSize; ++i)
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

		if (grids.size() == 1)
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

		for (unsigned int j = center + 1; j < termSize; ++j)
		{
			rates.set(rates[j] + width, j);
		}

		const unsigned int center_l = grids[pos - 1];
		for (unsigned int j = center_l + 1; j < center; ++j)
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

	for (unsigned int i = 0; i < termSize; i++)
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
CurveCalibrationData::setBasisDFByShiftZero(double width, FloorType type, double floor)
{
	AQLObjectHolder &eh_y = getYieldData().get();
	const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles&> (eh_y.getData(CALIBRATION_DATA_TERMS + mCurveSuffix, ISNOTNULL).get());

	RateConvention rc = setRC(getFrequency().get());
	AQLPriceDataConvention conv(getDayCount().getDayCount(), rc);

	const unsigned int termSize = terms.getSize();
	AQLDataDoubles rates;
	rates.resize(terms.getSize());

	for (unsigned int i = 0; i < termSize; ++i)
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
	for (unsigned int i = 0; i < termSize; ++i)
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
			throw AQLCoreInvalidData("#Error: Not Supported Floor Type", __FILE__, __LINE__);
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
CurveCalibrationData::setBasisRates(const DoubleMatrix& values)
{
	AQLDataDoubleMatrix& attrvalue = dynamic_cast<AQLDataDoubleMatrix&>(mpBasisRates->get());
	attrvalue.set(values);
}

/*!
	@brief calc basis rates
	@param[in] curveType

*/
void
CurveCalibrationData::setBasisRates(const AQLString &curveType)
{
	AQLDataHolder *dh = 0;
	dh = &getData(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLStringVector& nonvec = dynamic_cast<AQLDataStrings &>(dh->get()).get();
		if (nonvec.end() != std::find(nonvec.begin(), nonvec.end(), curveType))
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
	std::vector<AQLObject*> data_;
	std::vector<AQLObject*> data_fwd;
	std::vector<AQLObject*> data_fra;
	std::vector<AQLObject*> data_libor;
	std::vector<AQLObject*> data_ndf;
	for (unsigned int i = 0; i < mr_->getSize(); i++)
	{
		// check use grid
		const AQLDataHolder *dh = &mr_->get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get())
		{
			continue;
		}

		const AQLString &dataType = dynamic_cast<const AQLDataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		AQLString tmpType = dataType;
		tmpType.toUpper();
		if (tmpType.findString(BASIS) == 0)						// Basis Swaps
		{
			data_.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(FWDFX) == 0)				// FX Forwards
		{
			data_fwd.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(FRA) == 0)					// FRA
		{
			data_fra.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(ZERO) == 0)					// Libor
		{
			data_libor.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(NDF) == 0)					// NDF
		{
			data_ndf.push_back(&mr_->get(i).get());
		}
	}

	size_t fra_size = data_fra.size();
	size_t fwd_size = data_fwd.size();
	size_t ndf_size = data_ndf.size();
	bool fwd_isonly = false;

	// data exist check
	if (data_.empty() && data_fwd.empty())
	{
		AQLString msg = "#Error: CurveName = " + curveType + ", basis/fwdfx data is not set.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	else if (data_.empty() && !data_fwd.empty()) // only fwdfx case
	{
		data_ = data_fwd;
		fwd_isonly = true;
	}
	else if (!data_fra.empty() && !data_fwd.empty())
	{
		AQLString msg = "#Error: FX forwards and FRAs must not be used at the same time to calibrate basis curve";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	bool is_fra_use = false;
	if (!data_fra.empty())
	{
		is_fra_use = true;
	}

	// sort
	InstrumentComp comp;
	sort(data_.begin(), data_.end(), comp);

	if (!data_fwd.empty())
	{
		sort(data_fwd.begin(), data_fwd.end(), comp);
		if (!fwd_isonly && comp(data_.front(), data_fwd.back()))
		{
			throw AQLCoreInvalidData("#Error: ForwardFX Term must be smaller than CCS Term", __FILE__, __LINE__);
		}
	}

	if (!data_fra.empty())
	{
		sort(data_fra.begin(), data_fra.end(), comp);
	}

	// foreign currency flag
	bool isForeignCcyLeg = false;
	dh = &getData(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isForeignCcyLeg = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// get first element val
	// curve info
	// setBasisRates by recursive

	bool isBasisEnabled = false;
	dh = &getData(IR_CALIBRATION_DATA_ENABLECALCULATION, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isBasisEnabled = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	const AQLString &fCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_FORECAST, ISNOTNULL).get()).get();
	if ((fCurve != STD) && (fCurve != DUMMY) && (fCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), fCurve)) && !isBasisEnabled)
	{
		setBasisRates(fCurve);
	}

	const AQLString &dCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_DISCOUNT, ISNOTNULL).get()).get();
	if ((dCurve != STD) && (dCurve != DUMMY) && (dCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), dCurve)) && !isBasisEnabled)
	{
		setBasisRates(dCurve);
	}

	const AQLString &a_fCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
	if ((a_fCurve != STD) && (a_fCurve != DUMMY) && (a_fCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), a_fCurve)) && !isBasisEnabled)
	{
		if (!isForeignCcyLeg) setBasisRates(a_fCurve);
	}

	const AQLString &a_dCurve = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL).get()).get();
	if ((a_dCurve != STD) && (a_dCurve != DUMMY) && (a_dCurve != FIXEDRATE) && (gCurveNames.end() == std::find(gCurveNames.begin(), gCurveNames.end(), a_dCurve)) && !isBasisEnabled)
	{
		if (!isForeignCcyLeg) setBasisRates(a_dCurve);
	}

	// check Dummy
	if (fCurve == DUMMY || dCurve == DUMMY)
	{
		throw AQLCoreInvalidData("#Error: Dummy curve must be used for against curve only.", __FILE__, __LINE__);
	}
	if ((a_fCurve == DUMMY && a_dCurve != DUMMY) || (a_fCurve != DUMMY && a_dCurve == DUMMY))
	{
		throw AQLCoreInvalidData("#Error: If DUMMY curve is used, both forecast and discount must be DUMMY curve.", __FILE__, __LINE__);
	}
	// check FixedRate
	if (a_fCurve == FIXEDRATE || a_dCurve == FIXEDRATE)
	{
		throw AQLCoreInvalidData("#Error: FIXEDRATE must be used for target curve only.", __FILE__, __LINE__);
	}
	if ((fCurve == FIXEDRATE && dCurve != FIXEDRATE) || (fCurve != FIXEDRATE && dCurve == FIXEDRATE))
	{
		throw AQLCoreInvalidData("#Error: If FIXEDRATE is used, both forecast and discount must be FIXEDRATE.", __FILE__, __LINE__);
	}

	const bool isTargetDiscountCurve = dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
	const bool isTimeInter = dynamic_cast<const AQLDataBool &>((data_[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL)).get()).get();
	const bool isSpreadOnAgainstLeg = dynamic_cast<const AQLDataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();

	// market convention info
	// cashlet
	const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &c_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
	const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
	AQLString c_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
	const int c_lag = dynamic_cast<const AQLDataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());

	// index
	const AQLPriceDataDayCount &i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
	const AQLPriceDataCalendar &i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &i_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
	const int i_lag = dynamic_cast<const AQLDataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
	AQLString i_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	AQLString i_accessary = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

	if (!isTargetDiscountCurve)
	{
		// if forecast only
		i_freq.toUpper();
		if (i_freq != SIMPLE)
		{
			throw AQLCoreInvalidData("#Error: If forecast mode, only simple is possible in frequency.", __FILE__, __LINE__);
		}

		// if FixedRate
		if (fCurve == FIXEDRATE)
		{
			throw AQLCoreInvalidData("#Error: If FIXEDRATE is used, target must be discount curve.", __FILE__, __LINE__);
		}
	}

	// against cashlet
	const AQLPriceDataDayCount &a_c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &a_c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &a_c_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, ISNOTNULL).get());
	const AQLDate &a_c_spotdate = dynamic_cast<const AQLDataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, ISNOTNULL).get());
	AQLString a_c_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL).get()).get();

	// against index
	const AQLPriceDataDayCount &a_i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL).get());
	const AQLPriceDataCalendar &a_i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, ISNOTNULL).get());
	const AQLPriceDataCalendar &a_i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule &a_i_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, ISNOTNULL).get());
	const int a_i_lag = dynamic_cast<const AQLDataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, ISNOTNULL).get());
	AQLString a_i_freq = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, ISNOTNULL).get()).get();
	AQLString a_i_accessary = dynamic_cast<const AQLDataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, ISNOTNULL).get()).get();

	// simultaneous equation
	bool isSimuEq = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSimuEq = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// optimize performance
	bool optimizePerformance = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		optimizePerformance = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	bool fastRebuild = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_FASTREBUILD, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		fastRebuild = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// issamegridindex
	bool isSameGridIndex = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSameGridIndex = dynamic_cast<const AQLDataBool &>(dh->get()).get();
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
		throw AQLCoreInvalidData("#Error: yield spread calc flag must be FALSE when building xccy curves.", __FILE__, __LINE__);
	}

	if (is_fra_use && isYieldSpreadCalc)
	{
		throw AQLCoreInvalidData("#Error: yield spread calc flag must be FALSE when FRAs are used.", __FILE__, __LINE__);
	}

	//iseomroll
	bool isEomRoll = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEomRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// isfwdinterpolation
	bool useForwardInterpolation = false;
	AQLPriceDataInterpolation *forwardInterpolationAgainstLeg = 0;
	std::unique_ptr<AQLPriceDataInterpolation> forwardInterpolation;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		useForwardInterpolation = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		forwardInterpolationAgainstLeg = &dynamic_cast<AQLPriceDataInterpolation &>(data_[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		if (isTargetDiscountCurve)
			forwardInterpolation.reset(dynamic_cast<AQLPriceDataInterpolation *>(forwardInterpolationAgainstLeg->clone()));
	}

	// compounding	
	int numberOfTimesToCompoundTargetLeg = 1;
	int numberOfTimesToCompoundAgainstLeg = 1;
	AQLString c_freq_cpd, a_c_freq_cpd;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		c_freq_cpd = dynamic_cast<const AQLDataString &>(dh->get()).get();
		numberOfTimesToCompoundTargetLeg = etrading::LADateHelpers::calcCompoundingTimes(c_freq, c_freq_cpd);
	}
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		a_c_freq_cpd = dynamic_cast<const AQLDataString &>(dh->get()).get();
		numberOfTimesToCompoundAgainstLeg = etrading::LADateHelpers::calcCompoundingTimes(a_c_freq, a_c_freq_cpd);
	}

	// get max term
	const AQLString &termMax = dynamic_cast<const AQLDataString &>((data_.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

	// calc term (apply to month)
	int y, m, d, w;
	etrading::LADateHelpers::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	c_freq.toUpper();
	unsigned int mUnit = 0;
	AQLString refRateTerm;
	if (c_freq == ANNUAL)
	{
		mUnit = 12;
		refRateTerm = "12M";
	}
	else if (c_freq == SEMI_ANNUAL)
	{
		mUnit = 6;
		refRateTerm = "6M";
	}
	else if (c_freq == QUARTERLY)
	{
		mUnit = 3;
		refRateTerm = "3M";
	}
	else if (c_freq == MONTHLY || c_freq == LUNAR)
	{
		mUnit = 1;
		refRateTerm = "1M";
	}
	else
	{
		AQLString msg = "#Error: frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		AQLString msg = "#Error: frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	a_c_freq.toUpper();
	unsigned int a_mUnit = 0;
	AQLString a_refRateTerm("");
	if (a_c_freq == ANNUAL)
	{
		a_mUnit = 12;
		a_refRateTerm = "12M";
	}
	else if (a_c_freq == SEMI_ANNUAL)
	{
		a_mUnit = 6;
		a_refRateTerm = "6M";
	}
	else if (a_c_freq == QUARTERLY)
	{
		a_mUnit = 3;
		a_refRateTerm = "3M";
	}
	else if (a_c_freq == MONTHLY || a_c_freq == LUNAR)
	{
		a_mUnit = 1;
		a_refRateTerm = "1M";
	}
	else
	{
		AQLString msg = "#Error: frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if ((c_freq == LUNAR && a_c_freq != LUNAR) || (c_freq != LUNAR && a_c_freq == LUNAR))
	{
		AQLString msg = "#Error: if the one frequency is LUNAR, the other must be LUNAR";
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
	AQLString roll_conv("");
	if (c_freq == LUNAR)
	{
		roll_conv = ROLLCONV_LUNAR;
	}
	else if (isEomRoll)
	{
		roll_conv = ROLLCONV_EOM;
	}
	else
	{
		roll_conv = ROLLCONV_NORMAL;
	}

	AQLObjectHolder &objHolder = getYieldData().get();
	AQLObject yieldData = objHolder.get();

	// Allow the target leg to use curves from a different external collection. When this happens, source
	// yield data from the external collection
	dh = &getData(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString externalCurveCollection = dynamic_cast<const AQLDataString &>(dh->get()).get();

		AQLDataInstance* dataInstance = getDataInstance();
		AQLObjectPool &objPool = dataInstance->getObjectPool();
		AQLString err = "#Error: Could not locate the foreign Yield Curve data using '" + externalCurveCollection + "'";

		CurveCalibrationData* externalYieldCurvePro = getYieldCurvePro(objPool, externalCurveCollection, err);

		yieldData = externalYieldCurvePro->getYieldData().get().get();
	}

	//By Default, the stateVariable of the interpolator is discount factor
	StateVariableEnum stateVariable = STATE_VARIABLE_DF;
	const AQLDataHolder* handle = &(objHolder.getData(IR_CALIBRATION_DATA_STATEVARIABLE + suffix, NOCHECK));
	if (handle->isDefined() && !handle->isNull())
	{
		stateVariable = etrading::toStateVariableEnum(dynamic_cast<const AQLDataString&>(handle->get()).get().getCString());

		if (stateVariable != STATE_VARIABLE_DF)
		{
			throw AQLCoreInvalidData("#Error: For Basis Curve, interpolator's StateVariable only supports DF", __FILE__, __LINE__);
		}
	}

	const StateVariableEnum stateVariableFutureFra = STATE_VARIABLE_LOG_DF;

	// get base yield curve and asofdate
	const AQLMathYieldCurve &baseYieldCurve = dynamic_cast<const AQLMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());

	// is renotional adjust or not
	bool isMarkedToMarketXccySwap = dynamic_cast<const AQLDataBool&> ((getData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffix, ISNOTNULL)).get());
	bool isCurveUSD = false;
	AQLString currency = dynamic_cast<AQLDataString&> ((yieldData.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
	if (currency.toUpper() == "USD")
	{
		isCurveUSD = true;
	}

	// target side curves

	AQLPriceDataDayCount dc_act(ACT_365);
	DoubleArray b_t(1, 0.0);
	DoubleArray df_mod(1, 1.0);
	DoubleArray _terms;
	DoubleArray std_terms;
	DoubleArray std_dfs;

	// target side forecast curve
	if (fCurve == STD || dCurve == STD
		|| !isForeignCcyLeg && (a_fCurve == STD || a_fCurve == DUMMY || a_fCurve == STD || a_fCurve == DUMMY))
	{
		std_terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
		std_dfs = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL)).get()).get();
	}

	AQLPriceDataInterpolation forwardRatesTargetLeg;
	std::shared_ptr<AQLPriceDataInterpolation> f_inter_cloned;
	dh = &yieldData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + ((fCurve == STD) ? "" : AQLString("_") + fCurve), NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		forwardRatesTargetLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION + ((fCurve == STD) ? "" : AQLString("_") + fCurve), NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			forwardRatesTargetLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		}
		else
		{
			f_inter_cloned.reset(dynamic_cast<AQLPriceDataInterpolation *>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get().clone()));
			forwardRatesTargetLeg = *f_inter_cloned.get();
		}
	}

	setLinearSplineCutoffDate(forwardRatesTargetLeg, yieldData, fCurve, this);
	if (fCurve == STD)
	{
		forwardRatesTargetLeg.set(std_terms, std_dfs);
		if (!isTargetDiscountCurve)
		{
			_terms = std_terms;
		}
	}
	else if (fCurve == FIXEDRATE)
	{
		; // see below
	}
	else
	{
		const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + fCurve, ISNOTNULL)).get()).get();
		const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + fCurve, ISNOTNULL)).get()).get();
		forwardRatesTargetLeg.set(terms, dfs);
		if (!isTargetDiscountCurve)
		{
			_terms = terms;
		}
	}

	// target side discount curve
	AQLPriceDataInterpolation discountFactorsTargetLeg;
	std::shared_ptr<AQLPriceDataInterpolation> d_inter_cloned;
	dh = &yieldData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + dCurve, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		discountFactorsTargetLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + dCurve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			discountFactorsTargetLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		}
		else
		{
			d_inter_cloned.reset(dynamic_cast<AQLPriceDataInterpolation *>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get().clone()));
			discountFactorsTargetLeg = *d_inter_cloned.get();
		}
	}

	setLinearSplineCutoffDate(discountFactorsTargetLeg, yieldData, dCurve, this);
	if (dCurve == STD)
	{
		discountFactorsTargetLeg.set(std_terms, std_dfs);
		if (isTargetDiscountCurve)
		{
			_terms = std_terms;
		}
	}
	else if (dCurve == FIXEDRATE)
	{
		; // see below
	}
	else
	{
		const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + dCurve, ISNOTNULL)).get()).get();
		const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + dCurve, ISNOTNULL)).get()).get();
		discountFactorsTargetLeg.set(terms, dfs);
		if (isTargetDiscountCurve)
		{
			_terms = terms;
		}
	}

	// against side curves

	// against side forecast curve
	AQLPriceDataInterpolation forwardRatesAgainstLeg;
	std::shared_ptr<AQLPriceDataInterpolation> a_f_inter_cloned;
	dh = &yieldData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + ((a_fCurve == STD) ? "" : AQLString("_") + a_fCurve), NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		forwardRatesAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION + ((a_fCurve == STD) ? "" : AQLString("_") + a_fCurve), NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			forwardRatesAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		}
		else
		{
			a_f_inter_cloned.reset(dynamic_cast<AQLPriceDataInterpolation *>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get().clone()));
			forwardRatesAgainstLeg = *a_f_inter_cloned.get();
		}
	}

	CurveCalibrationData* foreignYieldCurvePro = NULL;
	if (isForeignCcyLeg)
	{
		AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if (!eh_fy.isDefined())
		{
			throw AQLCoreInvalidData("#Error: Foreign Yield Data does not exist!", __FILE__, __LINE__);
		}
		else
		{
			/*
			 * As this is a foreign currency leg, get the CurveCalibrationData corresponding to the foreign yield data.
			 * This is required when invoking setLinearSplineCutOfDate() below.
			 */

			AQLDataInstance* dataInstance = getDataInstance();
			AQLObjectPool &objPool = dataInstance->getObjectPool();
			AQLString foreignCurveCollectionID = eh_fy.getName();
			foreignYieldCurvePro = getYieldCurvePro(objPool, foreignCurveCollectionID, "#Error: Could not locate the foreign Yield Curve data.");

			AQLObject &fYieldData = eh_fy.get();
			AQLString suffix;
			if (a_fCurve != STD)
			{
				suffix = AQLString("_") + a_fCurve;
			}
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();

			// Redefine the interpolation object using fYieldData
			dh = &fYieldData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + ((a_fCurve == STD) ? "" : AQLString("_") + a_fCurve), NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				forwardRatesAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
			}
			else
			{
				dh = &fYieldData.getData(CALIBRATION_DATA_INTERPOLATION + ((a_fCurve == STD) ? "" : AQLString("_") + a_fCurve), NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					forwardRatesAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
				}
				else
				{
					a_f_inter_cloned.reset(dynamic_cast<AQLPriceDataInterpolation *>(fYieldData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get().clone()));
					forwardRatesAgainstLeg = *a_f_inter_cloned.get();
				}
			}

			setLinearSplineCutoffDate(forwardRatesAgainstLeg, fYieldData, a_fCurve, foreignYieldCurvePro);

			forwardRatesAgainstLeg.set(terms, dfs);

			if (fCurve == FIXEDRATE) // set const DF(1.0)
			{
				const DoubleArray fixed_terms = terms;
				const DoubleArray fixed_dfs(dfs.size(), 1.0);
				forwardRatesTargetLeg.set(fixed_terms, fixed_dfs);
			}
		}
	}
	else
	{
		setLinearSplineCutoffDate(forwardRatesAgainstLeg, yieldData, a_fCurve, this);

		if (a_fCurve == STD || a_fCurve == DUMMY)
		{
			forwardRatesAgainstLeg.set(std_terms, std_dfs);
		}
		else
		{
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + a_fCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + a_fCurve, ISNOTNULL)).get()).get();
			forwardRatesAgainstLeg.set(terms, dfs);
		}
	}

	// against side discount curve
	AQLPriceDataInterpolation discountFactorsAgainstLeg;
	std::shared_ptr<AQLPriceDataInterpolation> a_d_inter_cloned;
	dh = &yieldData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + a_dCurve, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		discountFactorsAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		dh = &yieldData.getData(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + a_dCurve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			discountFactorsAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		}
		else
		{
			a_d_inter_cloned.reset(dynamic_cast<AQLPriceDataInterpolation *>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get().clone()));
			discountFactorsAgainstLeg = *a_d_inter_cloned.get();
		}
	}

	if (isForeignCcyLeg)
	{
		AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if (!eh_fy.isDefined())
		{
			throw AQLCoreInvalidData("#Error: Foreign Yield Data does not exist!", __FILE__, __LINE__);
		}
		else
		{
			AQLObject &fYieldData = eh_fy.get();
			AQLString suffix;
			if (a_dCurve != STD)
			{
				suffix = AQLString("_") + a_dCurve;
			}
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((fYieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();

			// Redefine the interpolation object using fYieldData
			dh = &fYieldData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + a_dCurve, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				discountFactorsAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
			}
			else
			{
				dh = &fYieldData.getData(CALIBRATION_DATA_INTERPOLATION + AQLString("_") + a_dCurve, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					discountFactorsAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
				}
				else
				{
					a_d_inter_cloned.reset(dynamic_cast<AQLPriceDataInterpolation *>(fYieldData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get().clone()));
					discountFactorsAgainstLeg = *a_d_inter_cloned.get();
				}
			}

			setLinearSplineCutoffDate(discountFactorsAgainstLeg, fYieldData, a_dCurve, foreignYieldCurvePro);

			discountFactorsAgainstLeg.set(terms, dfs);

			if (dCurve == FIXEDRATE) // set same DF as a_dCurve
			{
				discountFactorsTargetLeg.set(terms, dfs);
			}
		}
	}
	else
	{
		setLinearSplineCutoffDate(discountFactorsAgainstLeg, yieldData, a_dCurve, this);
		if (a_dCurve == STD || a_dCurve == DUMMY)
		{
			discountFactorsAgainstLeg.set(std_terms, std_dfs);
		}
		else
		{
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + AQLString("_") + a_dCurve, ISNOTNULL)).get()).get();
			const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + a_dCurve, ISNOTNULL)).get()).get();
			discountFactorsAgainstLeg.set(terms, dfs);
		}
	}

	const unsigned int max = m / mUnit + 1;
	AQLDate fdate = c_spotdate;
	AQLDate a_fdate = a_c_spotdate;
	AQLString strUnit = AQLString(static_cast<int>(mUnit)) + AQLString("M");

	//const unsigned int MAX_LOOP = 1000;

	const double spotTermTargetLeg = dc_act.getTerm(asof, c_spotdate);
	const double d_spotdf = discountFactorsTargetLeg.value(spotTermTargetLeg);
	const double f_spotdf = forwardRatesTargetLeg.value(spotTermTargetLeg);
	const double spotTermAgainstLeg = dc_act.getTerm(asof, a_c_spotdate);
	const double a_d_spotdf = discountFactorsAgainstLeg.value(spotTermAgainstLeg);

	// if spotTermTargetLeg is different between legs, adjust against leg df.
	double spotTermAdjustmentAgainstLeg = 1.0;
	if (isTargetDiscountCurve && (spotTermTargetLeg != spotTermAgainstLeg))
	{
		spotTermAdjustmentAgainstLeg = a_d_spotdf / discountFactorsAgainstLeg.value(spotTermTargetLeg);
	}

	// calc extrapolation terms
	AQLString maxTerm, maxFreq;
	DoubleArray extra_terms;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		maxTerm = dynamic_cast<AQLDataString&>(dh->get()).get();
		maxTerm += "Y";
		maxFreq = dynamic_cast<AQLDataString&>(objHolder.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
		const AQLDate& maxDate = etrading::LADateHelpers::getDate(c_spotdate, maxTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
		DateVector tmp_dates; DoubleArray tmp_taus;
		etrading::updateAccrualPeriodsAndPaymentDates(c_spotdate, maxDate, maxFreq, c_cal, c_sld, dc_act, tmp_dates, extra_terms, tmp_taus, isEomRoll);
	}

	size_t preSwapInstrumentSize = 0;

	// calc DF from FwdFX
	double fwd_spotTerm = 0.0;
	double fwd_spotdf = 1.0;
	int fwd_size_mm = 0;
	DoubleArray fwd_terms;
	DoubleArray fwd_dfs;
	
	if (fwd_size > 0)
	{
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points)
		const bool isFXForwardQuotedAsOutright = dynamic_cast<const AQLDataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISRATIO, IR_CALIBRATION_DATA_ISFXOUTRIGHT, ISNOTNULL).get()).get(); // ISRATIO has alias ISFXOUTRIGHT
		const AQLPriceDataCalendar &fwd_cal = dynamic_cast<const AQLPriceDataCalendar &>(data_fwd[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &fwd_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_fwd[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const AQLDate &fwd_spotdate = dynamic_cast<const AQLDataDate &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
		const int fwd_spotlag = dynamic_cast<const AQLDataInt &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get()).get();
		fwd_spotTerm = dc_act.getTerm(asof, fwd_spotdate);
		fwd_spotdf = discountFactorsTargetLeg.value(fwd_spotTerm);
		AQLString fwd_freq = dynamic_cast<const AQLDataString &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
		const bool fwd_eom = dynamic_cast<const AQLDataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL).get()).get();

		// set roll convention
		AQLString fwd_roll_conv("");
		if (fwd_freq == LUNAR)
		{
			fwd_roll_conv = ROLLCONV_LUNAR;
		}
		else if (fwd_eom)
		{
			fwd_roll_conv = ROLLCONV_EOM;
		}
		else
		{
			fwd_roll_conv = ROLLCONV_NORMAL;
		}

		// calc extrapolation terms for fwdfx
		if (fwd_isonly && extra_terms.size() > 0)
		{
			extra_terms.clear();
			const AQLDate& maxDate = etrading::LADateHelpers::getDate(fwd_spotdate, maxTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
			DateVector tmp_dates; DoubleArray tmp_taus;
			etrading::updateAccrualPeriodsAndPaymentDates(fwd_spotdate, maxDate, maxFreq, fwd_cal, fwd_sld, dc_act, tmp_dates, extra_terms, tmp_taus, fwd_eom);
		}

		double term = 0.0;
		double df = 1.0;
		AQLDate end;
		// isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
		if (isFXForwardQuotedAsOutright)
		{
			double fwd_ratio_pow = 1.0;
			for (size_t i = 0; i < fwd_size; i++)
			{
				const double fwd_ratio = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const AQLString &fwd_termStr = dynamic_cast<const AQLDataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				if (fwd_termStr == "ON")
				{
					if (fwd_spotlag < 1)
					{
						continue;
					}
					else
					{
						fwd_ratio_pow *= fwd_ratio;
						end = etrading::LADateHelpers::getDate(asof, fwd_termStr, SLIDING_RULE_FOLLOWING, &fwd_cal, true);
						term = dc_act.getTerm(asof, end);
						df = discountFactorsTargetLeg.value(term) / fwd_ratio_pow;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else if (fwd_termStr == "TN")
				{
					if (fwd_spotlag < 2)
					{
						continue;
					}
					else
					{
						fwd_ratio_pow *= fwd_ratio;
						term = dc_act.getTerm(asof, fwd_spotdate); //term is asof to spot if spotlag != 2
						df = discountFactorsTargetLeg.value(term) / fwd_ratio_pow;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else
				{
					end = etrading::LADateHelpers::getDate(fwd_spotdate, fwd_termStr, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
					term = dc_act.getTerm(asof, end);
					df = discountFactorsTargetLeg.value(term) / (fwd_ratio * fwd_ratio_pow);
				}
				fwd_terms.push_back(term);
				fwd_dfs.push_back(df);
			}
		}
		else
		{
			const bool fwd_ispriceccy = dynamic_cast<const AQLDataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISPRICECCY, ISNOTNULL).get()).get();
			//get spot fx
			const AQLMathFXEntity& fx = dynamic_cast<const AQLMathFXEntity&>(getFXEntity().get().get());
			const AQLStringVector spotccys = fx.getCurrencys().get();
			const DoubleArray spotfxs = fx.getSpotRates().get();

			double spotfx = 0.0;
			if (spotfxs.size() == 1)
			{
				spotfx = spotfxs[0];
			}
			else if (spotfxs.size() == 2)
			{
				double spotfx_unitccy = 0.0;
				double spotfx_usd_unitccy = 0.0;
				for (unsigned int i = 0; i < spotccys.size(); i++)
				{
					if (spotccys[i] == currency)
					{
						spotfx_unitccy = spotfxs[i];
					}
					else if (spotccys[i] == "USD")
					{
						spotfx_usd_unitccy = spotfxs[i];
					}
				}

				if (spotfx_unitccy == 0.0 || spotfx_usd_unitccy == 0.0)
				{
					throw AQLCoreInvalidData("#Error: SpotFX rates do not exist!", __FILE__, __LINE__);
				}
				spotfx = fwd_ispriceccy ? spotfx_usd_unitccy / spotfx_unitccy : spotfx_unitccy / spotfx_usd_unitccy;
			}
			else
			{
				throw AQLCoreInvalidData("#Error: Please provide one or two FX Spot rates", __FILE__, __LINE__);
			}

			//search ON&TN fwd spread
			double fwd_spread_on = 0.0;
			double fwd_spread_tn = 0.0;
			for (size_t i = 0; i < fwd_size; i++)
			{
				const AQLString &termStr = dynamic_cast<const AQLDataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				if (termStr == "ON")
				{
					fwd_spread_on = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				}
				else if (termStr == "TN")
				{
					fwd_spread_tn = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				}
				else if (fwd_spread_on != 0.0 && fwd_spread_tn != 0.0)
				{
					break;
				}
			}

			const double fwd_fx_on = fwd_spotlag != 1 ? spotfx - fwd_spread_tn - fwd_spread_on : spotfx - fwd_spread_on;
			const double fwd_fx_tn = spotfx - fwd_spread_tn;
			for (size_t i = 0; i < fwd_size; i++)
			{
				const AQLString &fwd_termStr = dynamic_cast<const AQLDataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				double fwd_ratio = 1.0;
				if (fwd_termStr == "ON")
				{
					if (fwd_spotlag < 1)
					{
						continue;
					}
					else
					{
						if (fwd_spotlag == 1)
						{
							fwd_ratio = fwd_ispriceccy ? fwd_fx_on / spotfx : spotfx / fwd_fx_on;
						}
						else
						{
							fwd_ratio = fwd_ispriceccy ? fwd_fx_on / fwd_fx_tn : fwd_fx_tn / fwd_fx_on;
						}

						end = etrading::LADateHelpers::getDate(asof, fwd_termStr, SLIDING_RULE_FOLLOWING, &fwd_cal, true);
						term = dc_act.getTerm(asof, end);
						df = discountFactorsAgainstLeg.value(term) * fwd_ratio;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else if (fwd_termStr == "TN")
				{
					if (fwd_spotlag < 2)
					{
						continue;
					}
					else
					{
						fwd_ratio = fwd_ispriceccy ? fwd_fx_on / spotfx : spotfx / fwd_fx_on;
						term = dc_act.getTerm(asof, fwd_spotdate); //term is asof to spot if spotlag != 2
						df = discountFactorsAgainstLeg.value(term) * fwd_ratio;
						fwd_spotdf = df;
						++fwd_size_mm;
					}
				}
				else
				{
					const double fwd_spread = dynamic_cast<const AQLDataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
					const double fwd_fx = spotfx + fwd_spread;
					if (fwd_spotlag == 0)
					{
						fwd_ratio = fwd_ispriceccy ? spotfx / fwd_fx : fwd_fx / spotfx;
					}
					else
					{
						fwd_ratio = fwd_ispriceccy ? fwd_fx_on / fwd_fx : fwd_fx / fwd_fx_on;
					}
					end = etrading::LADateHelpers::getDate(fwd_spotdate, fwd_termStr, fwd_sld, &fwd_cal, true, &fwd_roll_conv);

					term = dc_act.getTerm(fwd_spotdate, end) + fwd_spotTerm;

					df = discountFactorsAgainstLeg.value(term) * fwd_ratio;
				}

				//term and df both to asOfDate
				fwd_terms.push_back(term);
				fwd_dfs.push_back(df);
			}
		}
		fwd_size = fwd_terms.size();

		preSwapInstrumentSize = fwd_size - fwd_size_mm;
	}

	// Finish curve calibration here using FXFwd only
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
			for (unsigned int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + fwd_spotTerm;
				if (term <= terms.back())
				{
					continue;
				}

				terms.push_back(term);
				dfs.push_back(dfs.back());
				if (!termsmtx_fwd[0].empty())
				{
					termsmtx_fwd[0].push_back(termsmtx_fwd[1].back());
					termsmtx_fwd[1].push_back(term);
				}
			}
		}

		saveBasisCurve(curveType, terms, termsmtx_fwd, dfs, isTargetDiscountCurve ? discountFactorsTargetLeg.convertToString() : forwardRatesTargetLeg.convertToString(), data_);

		return;
	}

	// get fwd basis
	bool isFwdBasis = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISFWDBASIS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isFwdBasis = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	//Spot date of Libor
	AQLDate spotdate_l;
	for (unsigned int i = 0; i < data_libor.size(); i++)
	{
		const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0)
		{
			spotdate_l = spotdate;
		}
		else if (spotdate_l != spotdate)
		{
			AQLString msg = "#Error: Same rate type must have same spotdate";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	// Curve Results
	DiscountFactors dfResults;
	DoubleArray		fwds;
	DoubleMatrix	fwd_termsmtx;		// Dummy variables for basis curve as basis curve doesn't calculate forward rates as outputs yet
	MoneyMarketData data_moneymarket;	// Dummy variable for basis curve

	// calc DF from FRAs
	if (!isTargetDiscountCurve && is_fra_use)
	{
		if (data_libor.size() == 0)
		{
			AQLString msg = "#Error: " + refRateTerm + " when FRAs are used in the Basis Curve then Libor fixings must be populated in the corresponding STD swap curve.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		bool is_fwdswap = false;
		dh = &(data_fra[0]->getData(PRICING_DATA_ISFWDSWAP, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			is_fwdswap = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// Discard swaps prior to FRAs
		AQLDate startDate, endDate;
		const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((data_fra.back()->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		etrading::calculateFraDates(startDate, endDate, c_spotdate, data_fra.back(), refRateTerm, c_freq, dc);

		size_t numberOfBasisSwaps = data_.size();
		std::vector<AQLObject*> temp_Data_Basis(data_.begin(), data_.end());
		data_.clear();
		for (size_t i = 0; i < numberOfBasisSwaps; ++i)
		{
			const AQLString &strTerm = dynamic_cast<const AQLDataString &>((temp_Data_Basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			const AQLDate swapEnd = etrading::LADateHelpers::getDate(c_spotdate, strTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr

			if (endDate < swapEnd)
			{
				data_.push_back(temp_Data_Basis[i]);
			}
		}

		// FRA interpolation
		// Try getting it from the suffixed INTERPOLATIONFW first, failing that falls back to yieldgen.interpolation
		AQLPriceDataInterpolation fraInterp;
		dh = &getData(IR_CALIBRATION_DATA_INTERPOLATIONFW + AQLString("_") + AQLString(curveType).toUpper(), NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			fraInterp = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		}
		else
		{
			dh = &getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + AQLString(curveType).toUpper(), NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				fraInterp = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
			}
			else
			{
				AQLString err = "#Error: Can not locate FRA/Futures interpolation method for curve '" + curveType + "'";
				throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
			}
		}
		setLinearSplineCutoffDate(fraInterp, yieldData, curveType, this, true);

		std::unique_ptr<AQLInterpolationBase>  pInter_fw(dynamic_cast<AQLInterpolationBase*>(fraInterp.getMethod().clone()));

		// Libor instrument spot date
		AQLDate spotdate_l = c_spotdate;
		const AQLDataHolder *dh = &data_libor[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			spotdate_l = dynamic_cast<const AQLDataDate&> (dh->get());
		}

		// First swap date as cutoff date
		AQLDate firstSwapDate;
		const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		if (isFwdBasis)
		{
			const bool is_date = dynamic_cast<const AQLDataBool&> ((data_[0]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
			AQLDate start, end;
			if (is_date)
			{
				firstSwapDate = dynamic_cast<const AQLDataDate&> ((data_[0]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			}
			else
			{
				const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_[0]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
				start = etrading::LADateHelpers::getDate(c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
				const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_[0]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
				firstSwapDate = etrading::LADateHelpers::getDate(start, tenor_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
			}
		}
		else
		{
			firstSwapDate = etrading::LADateHelpers::getDate(c_spotdate, strTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, &roll_conv);
		}

		// Bootstrap Libor Cash Deposits
		dfResults = bootstrapLibors( fwds,
									 fwd_termsmtx,
									 data_libor,
									 data_moneymarket,
									 true,				// is_fra_use
									 false,				// is_f_use
									 c_spotdate,		// spotdate_s
									 spotdate_l,
									 refRateTerm,
									 false );			// implyForwards
		
		// Bootstrap FRA Instruments
		bootstrapFRAs( dfResults,
					   fwds,
					   fwd_termsmtx,
					   data_fra,
					   data_libor,
					   pInter_fw.get(),
					   stateVariableFutureFra,
					   is_fwdswap,
					   c_freq,
					   refRateTerm,
					   asof,
					   c_spotdate,
					   spotdate_l,
					   firstSwapDate,
					   false );

		preSwapInstrumentSize = dfResults.paymentDates_.size() - 1;
	}

	// calc DF from NDFs
	DateVector NDF_df_dates;
	DoubleMatrix NDF_DFs(2);
	NDF_DFs[0].push_back(0.0);		//first term=0.0;
	NDF_DFs[1].push_back(1.0);		//first df = 1.0;
	NDF_df_dates.push_back(c_spotdate);

	if (isTargetDiscountCurve && ndf_size != 0)
	{
		for (unsigned int i = 0; i < ndf_size; ++i)
		{
			// NDF conventions
			const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount &>(data_ndf[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
			const AQLPriceDataCalendar &cal = dynamic_cast<const AQLPriceDataCalendar &>(data_ndf[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
			const AQLPriceDataSlidingRule &sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data_ndf[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
			double rate = dynamic_cast<const AQLDataDouble&> ((data_ndf[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const AQLString& term = dynamic_cast<const AQLDataString&> ((data_ndf[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLDate endDate = etrading::LADateHelpers::getDate(c_spotdate, term, sld, &cal, true, &roll_conv);

			// Interest Rate Convention - Stores instrument daycount and compounding conventions e.g. Simple Interest Act/Act.
			RateConvention rc = AQLMathYieldCurve::setRC(CONTINUOUS);
			AQLPriceDataDayCount dc_act365(ACT_365);
			AQLPriceDataConvention conv(dc_act365.getDayCount(), rc);

			// The market price of a NDF is the implied zero rate that together with foreign discount factor will calculate back to the NDF FX forward
			double tau = dc_act365.getTerm(c_spotdate, endDate);
			double df = conv.getDF(rate, c_spotdate, endDate);
			NDF_DFs[0].push_back(tau);
			NDF_DFs[1].push_back(df);
			NDF_df_dates.push_back(endDate);
		}

		preSwapInstrumentSize = NDF_df_dates.size() - 1;
	}

	double eps = 1.0e-9;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_EPSILON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eps = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	double grad_eps = 1.0e-10;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_GRADIENTEPSILON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		grad_eps = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	double delta = 1.0e-10;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_DELTA, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		delta = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	int max_loop = 1000;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_MAXLOOP, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		max_loop = dynamic_cast<const AQLDataInt &>(dh->get()).get();
	}

	// Calibrate DFs using basis swap instruments
	DateVector dvzeroDates;
	DoubleArray yield_mod;
	DoubleMatrix termsmtx_fwd(2);
	AQLPriceDataInterpolation spread_time;
	if (isSimuEq)
	{
		unsigned int numberOfBasisSwaps = data_.size();
		if (!numberOfBasisSwaps)
		{
			throw AQLCoreInvalidData("#Error: Basis data is empty.", __FILE__, __LINE__);
		}

		DoubleArray againstLegPVs(numberOfBasisSwaps, 0.0);
		DoubleArray swapSpreads(numberOfBasisSwaps, 0.0);
		std::vector<DateVector> datesVec(numberOfBasisSwaps);
		std::vector<DoubleArray> paymentDatesTargetLeg(numberOfBasisSwaps);
		DoubleArray gridVec_s(numberOfBasisSwaps);
		DoubleArray spotLag(numberOfBasisSwaps);
		std::vector<DoubleArray> accrualPeriodsTargetLeg(numberOfBasisSwaps);
		std::vector<DoubleMatrix> fixingDatesTargetLeg(numberOfBasisSwaps);
		std::vector<DoubleMatrix> indexTermsTargetLeg(numberOfBasisSwaps);
		std::vector<DoubleArray> paymentDatesAgainstLeg(numberOfBasisSwaps);
		std::vector<DoubleArray> accrualPeriodsAgainstLeg(numberOfBasisSwaps);
		std::vector<DoubleMatrix> fixingDatesAgainstLeg(numberOfBasisSwaps);
		std::vector<DoubleMatrix> indexTermsAgainstLeg(numberOfBasisSwaps);
		std::vector<DoubleMatrix> basisCurveRateTimesTimeVector(numberOfBasisSwaps);

		// Retrieve basis interpolation of the same curve name first before resorting to yield gen interp of the same
		// curve, otherwise use the yield gen interp of the STD curve.
		dh = &getData(IR_CALIBRATION_DATA_INTERPOLATIONBS + AQLString("_") + AQLString(curveType).toUpper(), NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			spread_time = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		}
		else
		{
			dh = &getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + AQLString("_") + AQLString(curveType).toUpper(), NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				spread_time = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
			}
			else
			{
				spread_time = getInterpolation_yg();
			}
		}
		setLinearSplineCutoffDate(spread_time, yieldData, curveType, this, true);

		DoubleArray grid_spread_time(preSwapInstrumentSize + numberOfBasisSwaps, 0.0);
		DoubleArray spread_timeVec(preSwapInstrumentSize + numberOfBasisSwaps, 0.0);

		if (fwd_size > 0)					// Prepopulate DFs if FX forwards are used
		{
			if (fwd_terms.size() > 0 && fwd_spotTerm != spotTermTargetLeg)
			{
				AQLPriceDataInterpolation tmp_inter = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
				tmp_inter.set(fwd_terms, fwd_dfs);
				fwd_spotdf = tmp_inter.value(spotTermTargetLeg);//to adjust for xccy basis spot
			}

			for (size_t i = fwd_size_mm; i < fwd_size; i++)
			{
				//*** Here it scales the fwd_terms/fwd_dfs to spotDate started before calibration, and after calibration it will scales all the instruments back to asOfDate.
				grid_spread_time[i - fwd_size_mm] = fwd_terms[i] - spotTermTargetLeg;
				spread_timeVec[i - fwd_size_mm] = -AQLMath::log(fwd_dfs[i] / fwd_spotdf);
			}
		}
		else if (!isTargetDiscountCurve && is_fra_use)	// Prepopulate DFs if FRAs are used
		{
			for (size_t i = 1; i < preSwapInstrumentSize + 1; ++i)	// i == 0 is on the spot date
			{
				grid_spread_time[i - 1] = dfResults.paymentDatesAsTerms_[i];
				spread_timeVec[i - 1] = -AQLMath::log(dfResults.discountFactors_[i]);
			}
		}
		else if (isTargetDiscountCurve && ndf_size != 0)	// Prepopulate DFs if NDFs are used
		{
			for (size_t i = 1; i < preSwapInstrumentSize + 1; ++i)	// i == 0 is on the spot date
			{
				grid_spread_time[i - 1] = NDF_DFs[0][i];
				spread_timeVec[i - 1] = -AQLMath::log(NDF_DFs[1][i]);
			}
		}

		AQLString a_fMarket = getMarketForCurve(a_fCurve);
		AQLString a_dMarket = getMarketForCurve(a_dCurve);
		bool isSameMarket = false;
		if (a_fMarket == a_dMarket)
		{
			if (a_fMarket == IR_NO_DATA)
			{
				isSameMarket = a_fCurve == a_dCurve;
			}
			else
			{
				isSameMarket = true;
			}
		}

		if (useForwardInterpolation)
		{
			// Check if the Dependency Curve is a Basis Curve
			// ----------------------------------------------
			// Important Note: Basis curves do not store and therfore allow interpolation on forwards. 
			// Therefore we cannot set useForwardInterpolation = TRUE, when the target / dependency curve is a basis curve.

			// Check if the curve type of 'a_fCurve' on the against leg is suitable for useForwardInterpolation
			AQLString curve("");
			if (isForeignCcyLeg)
			{
				AQLDataInstance* dataInstance = getDataInstance();
				AQLObjectPool &objPool = dataInstance->getObjectPool();
				AQLString foreignCurveCollectionID = getForeignYieldData().get().getName();
				CurveCalibrationData* foreignYieldCurvePro = getYieldCurvePro(objPool, foreignCurveCollectionID, "#Error: Could not locate the foreign Yield Curve data.");
				curve = foreignYieldCurvePro->getMarketForCurve(a_fCurve);
			}
			else
			{
				curve = getMarketForCurve(a_fCurve);
			}

			AQLString suffix = (curve == SWAP) ? "" : AQLString("_") + curve;

			AQLString curveType;
			AQLDataHolder* dh;
			if (isForeignCcyLeg)
			{
				AQLObject &fYieldData = getForeignYieldData().get().get();
				dh = &fYieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
			}
			else
			{
				dh = &yieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
			}

			if (dh->isDefined() && !dh->isNull())
			{
				curveType = dynamic_cast<AQLDataString&>(dh->get()).get();
			}

			if (curveType != SWAP)
			{
				throw AQLCoreInvalidData("#Error: Basis curve Error; isFwdInter can only be set to TRUE when the dependency curve is a STD Swap Curve. Basis curves do not support interpolation on forwards.", __FILE__, __LINE__);
			}

			// Check if the curve type of 'fCurve' on the target leg is suitable for useForwardInterpolation
			curve = getMarketForCurve(fCurve);
			suffix = (curve == SWAP) ? "" : AQLString("_") + curve;
			dh = &yieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
			if (dh->isDefined() && !dh->isNull())
			{
				curveType = dynamic_cast<AQLDataString&>(dh->get()).get();
			}

			if (curveType != SWAP)
			{
				throw AQLCoreInvalidData("#Error: Basis curve Error; isFwdInter can only be set to TRUE when the dependency curve is a STD Swap Curve. Basis curves do not support interpolation on forwards.", __FILE__, __LINE__);
			}

			// ----------------------------------------------

			AQLPriceDataDayCount dc;
			AQLPriceDataCalendar cal;
			AQLPriceDataSlidingRule sld;
			AQLString accessory;
			AQLString freq;
			DoubleArray terms;
			DoubleArray fwds;
			DoubleArray taus;
			DoubleMatrix termsMat;
			// target curve
			if (isTargetDiscountCurve && fCurve != DUMMY)
			{
				getForwardConvention(fCurve, dc, sld, cal, accessory);
				getDayCount(fCurve) = dc;
				getBaseForwardRate(fCurve, terms, termsMat, taus, fwds, 0, 0, useForwardInterpolation);
				setLinearSplineCutoffDate(*forwardInterpolation, yieldData, fCurve, this);
				forwardInterpolation->set(terms, fwds);
			}

			// against curve;
			if (a_fCurve != DUMMY)
			{
				if (isForeignCcyLeg)
				{
					AQLString fYieldDataName = dynamic_cast<const AQLDataString &>(getForeignYieldData().get().get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					AQLDataInstance* dataInstance = getDataInstance();
					AQLMathYieldCurve fYC(dataInstance);
					fYC.getYieldData().convertFromString(fYieldDataName);
					fYC.getCurveConvention(freq, cal, sld, dc, accessory, a_fCurve);
					fYC.getDayCount(a_fCurve) = dc;
					fYC.setInterpolation(forwardInterpolationAgainstLeg->convertToString());
					fYC.getBaseForwardRate(a_fCurve, terms, termsMat, taus, fwds, 0, 0, useForwardInterpolation);

					AQLObject &fYieldData = getForeignYieldData().get().get();
					setLinearSplineCutoffDate(*forwardInterpolationAgainstLeg, fYieldData, a_fCurve, foreignYieldCurvePro);
				}
				else
				{
					getForwardConvention(a_fCurve, dc, sld, cal, accessory);
					getDayCount(a_fCurve) = dc;
					getBaseForwardRate(a_fCurve, terms, termsMat, taus, fwds, 0, 0, useForwardInterpolation);

					setLinearSplineCutoffDate(*forwardInterpolationAgainstLeg, yieldData, a_fCurve, this);
				}
				forwardInterpolationAgainstLeg->set(terms, fwds);
			}
		}

		// Loop through all swaps and prepare for all date information
		for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
		{
			const double spread = dynamic_cast<const AQLDataDouble &>((data_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			swapSpreads[i] = spread;
			const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

			//-------------------------------------------------------------------------------
			// 1. Create date schedule for the 'against' leg, when certain conditons are met

			if (isSpreadOnAgainstLeg ||
				(a_fCurve != DUMMY && (!isSameGridIndex || !isSameMarket || useForwardInterpolation)))
			{
				DateVector a_dates;
				DoubleArray a_terms_grid;
				DoubleArray a_terms_interval;
				AQLDate fDate = a_c_spotdate;
				double a_effectiveStartTerm(0.0);

				// Build cash flows of the reference leg, i.e. the 'against' leg
				if (isFwdBasis)
				{
					const bool is_date = dynamic_cast<const AQLDataBool&> ((data_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
					AQLDate start, end;
					if (is_date)
					{
						start = dynamic_cast<const AQLDataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						end = dynamic_cast<const AQLDataDate&> ((data_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
					}
					else
					{
						const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						start = etrading::LADateHelpers::getDate(a_c_spotdate, sterm_str, a_c_sld, &a_c_cal, true, &roll_conv);
						const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
						end = etrading::LADateHelpers::getDate(start, tenor_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
					}
					etrading::updateAccrualPeriodsAndPaymentDates(start, end, a_c_freq, a_c_cal, a_c_sld, a_c_dc, a_dates, a_terms_grid, a_terms_interval, isEomRoll, &a_c_spotdate);
					fDate = start;
					a_effectiveStartTerm = dc_act.getTerm(a_c_spotdate, fDate);
				}
				else
				{
					const AQLDate end = etrading::LADateHelpers::getDate(a_c_spotdate, strTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
					etrading::updateAccrualPeriodsAndPaymentDates(a_c_spotdate, end, a_c_freq, a_c_cal, a_c_sld, a_c_dc, a_dates, a_terms_grid, a_terms_interval, isEomRoll);
				}

				DoubleMatrix a_i_gridMat(a_dates.size());
				DoubleMatrix a_i_termMat(a_dates.size());
				double targetPV = 0.0;
				double df = 1.0;

				for (unsigned int j = 0; j < a_dates.size(); ++j)
				{
					DoubleArray a_i_gridVec;
					DoubleArray a_i_termVec;
					if (isSameGridIndex)
					{
						// Fixing date terms
						a_i_gridVec.push_back(dc_act.getTerm(asof, fDate));
						AQLDate fixingEndDate = etrading::LADateHelpers::getDate(fDate, a_refRateTerm, a_c_sld, &a_c_cal, true, nullptr);
						a_i_gridVec.push_back(dc_act.getTerm(asof, fixingEndDate));

						// Index tau
						a_i_termVec.push_back(a_i_dc.getTerm(fDate, fixingEndDate));

						fDate = a_dates[j];
					}
					else
					{
						if (j == 0)
						{
							calcIndexGrid(asof, fDate, a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
						}
						else
						{
							calcIndexGrid(asof, a_dates[j - 1], a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
						}
					}
					a_i_gridMat[j] = a_i_gridVec;
					a_i_termMat[j] = a_i_termVec;
				}


				paymentDatesAgainstLeg[i] = a_terms_grid;
				accrualPeriodsAgainstLeg[i] = a_terms_interval;

				fixingDatesAgainstLeg[i] = a_i_gridMat;
				indexTermsAgainstLeg[i] = a_i_termMat;

				if (isTargetDiscountCurve && isMarkedToMarketXccySwap && !isCurveUSD) // fwd renotional
				{
					//calc after (at calcBasisCF(...))
				}
				else // spot renotional
				{
					// Calculate the PV of the 'against' leg. This PV is as of the spot date.
					againstLegPVs[i] = calcAgainstPV(useForwardInterpolation, forwardRatesAgainstLeg, forwardInterpolationAgainstLeg, discountFactorsAgainstLeg, isSpreadOnAgainstLeg ? spread : 0.0, numberOfTimesToCompoundAgainstLeg,
						spotTermAgainstLeg, a_terms_grid, a_terms_interval, a_i_gridMat, a_i_termMat, a_effectiveStartTerm) * spotTermAdjustmentAgainstLeg;
				}
			}

			//------------------------------------------------------------------------
			// 2. Create date schedule for the usual leg

			AQLPriceDataDayCount dc_act365(ACT_365);
			AQLDate fDate = c_spotdate;
			DateVector dates;
			DoubleArray cashflowDates;
			DoubleArray cashflowAccuralPeriods;

			if (c_spotdate < a_c_spotdate && !isSameGridIndex)
			{
				// roll from a_c_spotdate

				// Build cash flows of the target leg
				if (isFwdBasis)
				{
					const bool is_date = dynamic_cast<const AQLDataBool&> ((data_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
					AQLDate start, end;
					if (is_date)
					{
						start = dynamic_cast<const AQLDataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						end = dynamic_cast<const AQLDataDate&> ((data_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
					}
					else
					{
						const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						start = etrading::LADateHelpers::getDate(a_c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
						const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
						end = etrading::LADateHelpers::getDate(start, tenor_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
					}

					etrading::updateAccrualPeriodsAndPaymentDates(start, end, c_freq, c_cal, c_sld, c_dc, dates, cashflowDates, cashflowAccuralPeriods, isEomRoll, &a_c_spotdate);
					fDate = start;

					// The date fraction between the **spot date** of the curve and the start date of each **forward starting swap**
					gridVec_s[i] = dc_act365.getTerm(a_c_spotdate, start);
					spotLag[i] = dc_act365.getTerm(a_c_spotdate, start);
				}
				else
				{
					const AQLDate end = etrading::LADateHelpers::getDate(a_c_spotdate, strTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, &roll_conv);
					etrading::updateAccrualPeriodsAndPaymentDates(a_c_spotdate, end, c_freq, c_cal, c_sld, c_dc, dates, cashflowDates, cashflowAccuralPeriods, isEomRoll);
					spotLag[i] = 0.0;
				}
				// calc odd date
				const double odd_term = dc_act.getTerm(c_spotdate, a_c_spotdate);
				const double odd_interval = c_dc.getTerm(c_spotdate, a_c_spotdate, false);
				// term from c_spotdate
				DoubleArray plus_term(cashflowDates.size(), odd_term);
				transform(cashflowDates.begin(), cashflowDates.end(), plus_term.begin(), cashflowDates.begin(), std::plus<double>());
				// insert odd term
				dates.insert(dates.begin(), c_spotdate);
				cashflowDates.insert(cashflowDates.begin(), odd_term);
				cashflowAccuralPeriods.insert(cashflowAccuralPeriods.begin(), odd_interval);
			}
			else
			{
				if (isFwdBasis)
				{
					const bool is_date = dynamic_cast<const AQLDataBool&> ((data_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
					AQLDate start, end;
					if (is_date)
					{
						start = dynamic_cast<const AQLDataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						end = dynamic_cast<const AQLDataDate&> ((data_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
					}
					else
					{
						const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						start = etrading::LADateHelpers::getDate(c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
						const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
						end = etrading::LADateHelpers::getDate(start, tenor_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
					}
					etrading::updateAccrualPeriodsAndPaymentDates(start, end, c_freq, c_cal, c_sld, c_dc, dates, cashflowDates, cashflowAccuralPeriods, isEomRoll, &c_spotdate);
					fDate = start;

					// The date fraction between the **spot date** of the curve and the start date of each **forward starting swap**
					gridVec_s[i] = dc_act365.getTerm(c_spotdate, start);
					spotLag[i] = dc_act365.getTerm(c_spotdate, start);
				}
				else
				{
					const AQLDate end = etrading::LADateHelpers::getDate(c_spotdate, strTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
					etrading::updateAccrualPeriodsAndPaymentDates(c_spotdate, end, c_freq, c_cal, c_sld, c_dc, dates, cashflowDates, cashflowAccuralPeriods, isEomRoll);
					spotLag[i] = 0.0;
				}
			}

			// calc basis r * t
			DoubleArray dfs(dates.size(), 0.0);
			DoubleMatrix indexStartAndEndDates(dates.size());
			DoubleMatrix indexAccrualPeriods(dates.size());
			DoubleMatrix basisCurveRateTimesTime(dates.size());

			for (unsigned int j = 0; j < dates.size(); ++j)
			{

				DoubleArray i_gridVec;
				DoubleArray i_termVec;
				if (isSameGridIndex)
				{
					// Fixing date terms
					i_gridVec.push_back(dc_act.getTerm(asof, fDate));

					AQLDate fixingEndDate = etrading::LADateHelpers::getDate(fDate, refRateTerm, c_sld, &c_cal, true, nullptr);
					i_gridVec.push_back(dc_act.getTerm(asof, fixingEndDate));

					// Index tau
					i_termVec.push_back(i_dc.getTerm(fDate, fixingEndDate));

					fDate = dates[j];
				}
				else
				{
					if (j == 0)
					{
						calcIndexGrid(asof, fDate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
					}
					else
					{
						calcIndexGrid(asof, dates[j - 1], i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
					}
				}
				indexStartAndEndDates[j] = i_gridVec;
				indexAccrualPeriods[j] = i_termVec;

				if (isTargetDiscountCurve)
				{
					if (isYieldSpreadCalc)
					{
						const double b_yield_time = -AQLMath::log(discountFactorsTargetLeg.value(cashflowDates[j] + spotTermTargetLeg) / d_spotdf);
						basisCurveRateTimesTime[j] = DoubleArray(1, b_yield_time);
					}
					else
					{
						basisCurveRateTimesTime[j] = DoubleArray(1, 0.0);
					}
				}
				else
				{
					DoubleArray b_yieldTimeVec(i_gridVec.size(), 0.0);
					if (isYieldSpreadCalc)
					{
						for (unsigned int k = 1; k < i_gridVec.size(); ++k)
						{
							const double df = forwardRatesTargetLeg.value(i_gridVec[k]);
							b_yieldTimeVec[k] = -AQLMath::log(df / f_spotdf);
						}
						const double df1 = forwardRatesTargetLeg.value(i_gridVec[0]);
						b_yieldTimeVec[0] = -AQLMath::log(df1 / f_spotdf);
					}
					basisCurveRateTimesTime[j] = b_yieldTimeVec;
				}
			}

			datesVec[i] = dates;

			fixingDatesTargetLeg[i] = indexStartAndEndDates;
			indexTermsTargetLeg[i] = indexAccrualPeriods;
			paymentDatesTargetLeg[i] = cashflowDates;
			accrualPeriodsTargetLeg[i] = cashflowAccuralPeriods;

			// basisCurveRateTimesTime OR
			// b_yieldTimeVec OR
			// basisCurveRateTimesTimeVector
			// The r*t term of discount factors between spot date and each index date of a swap on the **target** curve used for the **target** leg

			basisCurveRateTimesTimeVector[i] = basisCurveRateTimesTime;

			grid_spread_time[preSwapInstrumentSize + i] = cashflowDates.back();

			const double df = discountFactorsTargetLeg.value(cashflowDates.back() + spotTermTargetLeg) / d_spotdf;

			spread_timeVec[preSwapInstrumentSize + i] = -AQLMath::log(df);
		}

		// Only uses this block if the spot Libor rate of the target tenor
		// has been provided in the base STD curve
		if (!isTargetDiscountCurve && !is_fra_use)
		{
			dh = &(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATETERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				// Find maturity date of the first bais swap
				const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				AQLDate date;
				if (isFwdBasis)
				{
					const bool is_date = dynamic_cast<const AQLDataBool&> ((data_[0]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
					if (is_date)
					{
						date = dynamic_cast<const AQLDataDate&> ((data_[0]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
					}
					else
					{
						const AQLString sterm_str = dynamic_cast<const AQLDataString&> ((data_[0]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						AQLDate start = etrading::LADateHelpers::getDate(c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
						const AQLString tenor_str = dynamic_cast<const AQLDataString&> ((data_[0]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
						date = etrading::LADateHelpers::getDate(start, tenor_str, c_sld, &c_cal, true, &roll_conv);
					}
				}
				else
				{
					date = etrading::LADateHelpers::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
				}

				// Spot libor forecasts from spot date
				const AQLString &strTerm_s = dynamic_cast<const AQLDataString &>(dh->get()).get();
				AQLDate date_l = etrading::LADateHelpers::getDate(c_spotdate, strTerm_s, c_sld, &c_cal, true, &roll_conv);

				if (date_l < date)
				{
					// create data for spotrate
					DoubleArray i_gridVec;
					DoubleArray i_termVec;

					i_gridVec.push_back(dc_act.getTerm(asof, c_spotdate));
					i_gridVec.push_back(dc_act.getTerm(asof, date_l));
					i_termVec.push_back(i_dc.getTerm(c_spotdate, date_l));

					DoubleMatrix indexStartAndEndDates;
					DoubleMatrix indexAccrualPeriods;
					indexStartAndEndDates.push_back(i_gridVec);
					indexAccrualPeriods.push_back(i_termVec);

					DoubleArray b_yieldTimeVec(i_gridVec.size());
					if (isYieldSpreadCalc)
					{
						for (unsigned int k = 1; k < i_gridVec.size(); ++k)
						{
							const double df = forwardRatesTargetLeg.value(i_gridVec[k]);
							b_yieldTimeVec[k] = -AQLMath::log(df / f_spotdf);
						}
						const double df1 = forwardRatesTargetLeg.value(i_gridVec[0]);
						b_yieldTimeVec[0] = -AQLMath::log(df1 / f_spotdf);
					}

					DoubleMatrix basisCurveRateTimesTime;
					basisCurveRateTimesTime.push_back(b_yieldTimeVec);

					const double term = dc_act.getTerm(c_spotdate, date_l);
					const double tau = i_dc.getTerm(c_spotdate, date_l);
					const double df = discountFactorsTargetLeg.value(term + spotTermTargetLeg) / d_spotdf;	// Discount factor off the discount curve on the target leg

					const double spotRate = dynamic_cast<const AQLDataDouble &>(data_libor[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
					const double targetPV = (1.0 + spotRate * tau) * df - 1.0;		// PV of one cash flow

					fixingDatesTargetLeg.insert(fixingDatesTargetLeg.begin(), indexStartAndEndDates);
					indexTermsTargetLeg.insert(indexTermsTargetLeg.begin(), indexAccrualPeriods);
					basisCurveRateTimesTimeVector.insert(basisCurveRateTimesTimeVector.begin(), basisCurveRateTimesTime);

					paymentDatesTargetLeg.insert(paymentDatesTargetLeg.begin(), DoubleArray(1, term));
					accrualPeriodsTargetLeg.insert(accrualPeriodsTargetLeg.begin(), DoubleArray(1, tau));
					datesVec.insert(datesVec.begin(), DateVector(1, datesVec.back().front()));
					grid_spread_time.insert(grid_spread_time.begin(), term);
					spread_timeVec.insert(spread_timeVec.begin(), -AQLMath::log(df));
					spotLag.insert(spotLag.begin(), 0.0);

					swapSpreads.insert(swapSpreads.begin(), 0.0);
					againstLegPVs.insert(againstLegPVs.begin(), targetPV);

					++numberOfBasisSwaps;
				}
				else if (date_l == date)
				{
					if (paymentDatesTargetLeg[0].size() != 1)
					{
						throw AQLCoreInvalidData("#Error: basis first term, cashlet size must be one", __FILE__, __LINE__);
					}
					const double term = paymentDatesTargetLeg[0][0];
					const double tau = accrualPeriodsTargetLeg[0][0];
					const double df = discountFactorsTargetLeg.value(term + spotTermTargetLeg) / d_spotdf;	// Discount factor off the discount curve on the target leg

					// spotRate:	Retrieved from the base STD curve and is the actual spot rate of the target tenor
					const double spotRate = dynamic_cast<const AQLDataDouble &>(data_libor[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();
					const double targetPV = (1.0 + spotRate * tau) * df - 1.0;

					// againstLegPVs[0]:	PV of the 'against' leg. If spreads are in the 'against' leg, the spreads will have been used in calculating PV
					// targetPV:			PV of the target leg
					// diff_targetPV:		PV difference of the two spot-starting legs. 
					const double diff_targetPV = againstLegPVs[0] - targetPV;

					if (isSpreadOnAgainstLeg)
					{
						DateVector a_dates;
						DoubleArray a_terms_grid;
						DoubleArray a_terms_interval;
						AQLDate end = etrading::LADateHelpers::getDate(a_c_spotdate, strTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr

						// Find the schedules and then PV of the 'against' leg
						etrading::updateAccrualPeriodsAndPaymentDates(a_c_spotdate, end, a_c_freq, a_c_cal, a_c_sld, a_c_dc, a_dates, a_terms_grid, a_terms_interval, isEomRoll);
						double a_tau_df = 0.0;
						for (unsigned int i = 0; i < a_terms_grid.size(); ++i)
						{
							const double df = discountFactorsAgainstLeg.value(a_terms_grid[i] + spotTermAgainstLeg) / a_d_spotdf;
							// tau * df of the 'against' leg
							a_tau_df += a_terms_interval[i] * df;
						}

						// s_spread: Current spread amount. Expected to be zero for a par basis swap 
						const double s_spread = -diff_targetPV / a_tau_df;
						swapSpreads[0] += s_spread;
						againstLegPVs[0] = targetPV;
					}
					else
					{
						const double spread = diff_targetPV / (tau * df);
						swapSpreads[0] = spread;
					}

					dynamic_cast<AQLDataDouble &>(data_[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).set(swapSpreads[0]);
					data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATE);
					data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATETERM);
				}
				else
				{
					AQLString msg = "#Error: spotrateterm is after basis first term. spotrateterm = " + strTerm_s + ", basis first term = " + strTerm;
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}
		}

		// for analytic
		dvzeroDates = datesVec.back();
		if (!dvzeroDates.empty() && dvzeroDates[0] == c_spotdate)
		{
			dvzeroDates.erase(dvzeroDates.begin());
		}

		// spread_time: Interpolator on the r*tau terms of the Discount Curve
		grid_spread_time.insert(grid_spread_time.begin(), 0.0);
		spread_timeVec.insert(spread_timeVec.begin(), 0.0);
		spread_time.set(grid_spread_time, spread_timeVec);

		//get convexity adjust interpolation
		AQLPriceDataInterpolation* convexityAdjustment = NULL;
		if (isTargetDiscountCurve && isMarkedToMarketXccySwap)
		{
			dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DoubleArray& adjustValue_term = dynamic_cast<const AQLDataDoubles&> (dh->get()).get();
				const DoubleArray& adjustValue
					= dynamic_cast<const AQLDataDoubles&> ((data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL)).get()).get();
				convexityAdjustment = &(dynamic_cast<AQLPriceDataInterpolation&> (data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get()));
				convexityAdjustment->set(adjustValue_term, adjustValue);
			}
		}

		// Important Check to Prevent Build Failure
		// ----------------------------------------
		// Check the Maximum Against Leg Interpolator Maturity is Greater than or Equal to the Target Leg Maturity
		// Extrapolation of the dependent curve (against leg) can lead to extreme unusable data, negative discount factors et al


		// Find the difference between target and reference legs. basisSwapPVs carries the differences on all the basis swaps.
		DoubleArray basisSwapPVs(numberOfBasisSwaps, 0.0);
		calcBasisCF(spread_time,						// [1] Spread interpolator
					isTargetDiscountCurve,				// [2] Is target a discount curve
					isMarkedToMarketXccySwap,			// [3] Is notional adjustment
					isCurveUSD,							// [4] Is curve in USD currency
					useForwardInterpolation,			// [5] Is interpolating forwards directly
					forwardRatesTargetLeg,				// [6] Forecast curve - DF interpolator - target leg
					forwardInterpolation.get(),			// [7] Forecast curve - forward rate interpolator - target leg
					discountFactorsTargetLeg,			// [8] Discount curve - DF interpolator - target leg
					forwardRatesAgainstLeg,				// [9] Forecast curve - DF interpolator - against leg
					forwardInterpolationAgainstLeg,		// [10]	Forecast curve - forward rate interpolator - against leg
					discountFactorsAgainstLeg,			// [11] Discount curve - DF interpolator - against leg
					spotTermAdjustmentAgainstLeg,		// [12] Spot term adjust (if spot term is different between the two legs)
					convexityAdjustment,				// [13] Convexity adjustment interpolator
					isSpreadOnAgainstLeg,				// [14] Is spread on the against leg?
					swapSpreads,						// [15] Swap basis inputs
					numberOfTimesToCompoundTargetLeg,	// [16] The number of times compoundings happen with one accural period - target leg
					numberOfTimesToCompoundAgainstLeg,	// [17] The number of times compoundings happen with one accural period - against leg
					spotTermTargetLeg,					// [18] Spot term - target leg
					spotTermAgainstLeg,					// [19] Spot term - against leg
					paymentDatesTargetLeg,				// [20] Date fraction between spot date and payment dates of every swap - target leg
					accrualPeriodsTargetLeg,			// [21] Date fraction of each payment period of every swap - target leg
					fixingDatesTargetLeg,				// [22] Date fraction between asof date and fixing dates of every swap - target leg
					indexTermsTargetLeg,				// [23] Date fraction of each indexing period of every swap - target leg
					paymentDatesAgainstLeg,				// [24] Date fraction between spot date and payment dates of every swap - against leg
					accrualPeriodsAgainstLeg,			// [25] Date fraction of each payment period of every swap - against leg
					fixingDatesAgainstLeg,				// [26] Date fraction between asof date and fixing dates of every swap - against leg
					indexTermsAgainstLeg,				// [27] Date fraction of each indexing period of every swap - against leg
					basisCurveRateTimesTimeVector,		// [28] A vector of the r*t term of discount factors of the target curve from spot date to each index date - target leg
					againstLegPVs,						// [29] A vector of PVs of the against leg of all the swaps - against leg
					basisSwapPVs,						// [30] Output of result PVs
					true,								// [31] Sign of the output PVs - isNegative?
					spotLag);							// [32] Date fraction between spot date and effective start date - target leg

		// Perturb the DFs of the SPREADS by a scale of 1.0001 in multiple. These spreads are added to the r*t term of the target curve in pricing.
		DoubleArray multiple_spread_timeVec(spread_timeVec.size() - 1, 1.0001);
		transform(spread_timeVec.begin() + preSwapInstrumentSize + 1,
			spread_timeVec.end(),
			multiple_spread_timeVec.begin(),
			spread_timeVec.begin() + preSwapInstrumentSize + 1,
			std::multiplies<double>());
		spread_time.set(grid_spread_time, spread_timeVec);

		// Calculate the PV differene between the two legs again, having perturbed the spreads
		DoubleArray targetValVec1(numberOfBasisSwaps, 0.0);
		calcBasisCF(spread_time, isTargetDiscountCurve, isMarkedToMarketXccySwap, isCurveUSD, useForwardInterpolation, forwardRatesTargetLeg, forwardInterpolation.get(), discountFactorsTargetLeg, forwardRatesAgainstLeg, forwardInterpolationAgainstLeg, discountFactorsAgainstLeg, spotTermAdjustmentAgainstLeg, convexityAdjustment, isSpreadOnAgainstLeg, swapSpreads,
			numberOfTimesToCompoundTargetLeg, numberOfTimesToCompoundAgainstLeg, spotTermTargetLeg, spotTermAgainstLeg, paymentDatesTargetLeg, accrualPeriodsTargetLeg, fixingDatesTargetLeg, indexTermsTargetLeg, paymentDatesAgainstLeg, accrualPeriodsAgainstLeg, fixingDatesAgainstLeg, indexTermsAgainstLeg,
			basisCurveRateTimesTimeVector, againstLegPVs, targetValVec1, true, spotLag);

		// N-dimensional Newton Raphson solving for spreads that price all basis swaps to PV zero
		bool solutionFound = false;
		int loopNum = max_loop;
		while (loopNum--)
		{
			bool isEnd = true;
			for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
			{
				if (AQLMath::abs(basisSwapPVs[i] - targetValVec1[i]) >= grad_eps)
				{
					isEnd = false;
					break;
				}
			}
			if (isEnd)
			{
				break;
			}

			AQLMatrix divMat(numberOfBasisSwaps, numberOfBasisSwaps);
			// create divMat
			for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
			{
				DoubleArray spread_timeVec_t = spread_timeVec;
				spread_timeVec_t[preSwapInstrumentSize + i + 1] += delta;
				spread_time.set(grid_spread_time, spread_timeVec_t);
				DoubleArray vals;
				calcBasisCF(spread_time, isTargetDiscountCurve, isMarkedToMarketXccySwap, isCurveUSD, useForwardInterpolation, forwardRatesTargetLeg, forwardInterpolation.get(), discountFactorsTargetLeg, forwardRatesAgainstLeg, forwardInterpolationAgainstLeg, discountFactorsAgainstLeg, spotTermAdjustmentAgainstLeg, convexityAdjustment, isSpreadOnAgainstLeg, swapSpreads,
					numberOfTimesToCompoundTargetLeg, numberOfTimesToCompoundAgainstLeg, spotTermTargetLeg, spotTermAgainstLeg, paymentDatesTargetLeg, accrualPeriodsTargetLeg, fixingDatesTargetLeg, indexTermsTargetLeg, paymentDatesAgainstLeg, accrualPeriodsAgainstLeg, fixingDatesAgainstLeg, indexTermsAgainstLeg,
					basisCurveRateTimesTimeVector, againstLegPVs, vals, false, spotLag);
				for (unsigned int j = 0; j < numberOfBasisSwaps; ++j)
				{
					if (vals.size() != numberOfBasisSwaps)
					{
						throw AQLCoreInvalidData("size error!", __FILE__, __LINE__);
					}
					const double divVal = (vals[j] + targetValVec1[j]) / delta;
					divMat.setValue(j, i, divVal);
				}
			}

			AQLMatrix invMat = divMat.inverseMatrix();
			AQLMatrix valMat(targetValVec1);
			AQLMatrix deltaMat = invMat * valMat;

			// plus delta
			for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
			{
				spread_timeVec[preSwapInstrumentSize + i + 1] += deltaMat.getValue(i, 0);
			}

			spread_time.set(grid_spread_time, spread_timeVec);
			basisSwapPVs = targetValVec1;
			calcBasisCF(spread_time,						// [1] Spread interpolator 
				isTargetDiscountCurve,				// [2] Is target a discount curve
				isMarkedToMarketXccySwap,			// [3] Is notional adjustment
				isCurveUSD,							// [4] Is curve in USD currency
				useForwardInterpolation,			// [5] Is interpolating forwards directly
				forwardRatesTargetLeg,				// [6] Forecast curve - DF interpolator - target leg
				forwardInterpolation.get(),			// [7] Forecast curve - forward rate interpolator - target leg
				discountFactorsTargetLeg,			// [8] Discount curve - DF interpolator - target leg
				forwardRatesAgainstLeg,				// [9] Discount curve - DF interpolator - against leg
				forwardInterpolationAgainstLeg,		// [10] Forecast curve - forward rate interpolator - against leg	
				discountFactorsAgainstLeg,			// [11] Discount curve - DF interpolator - against leg
				spotTermAdjustmentAgainstLeg,		// [12] Discount curve - DF interpolator - against leg
				convexityAdjustment,				// [13] Convexity adjustment interpolator
				isSpreadOnAgainstLeg,				// [14] Is spread on the against leg?
				swapSpreads,						// [15] Swap basis inputs
				numberOfTimesToCompoundTargetLeg,	// [16] The number of times compoundings happen with one accural period - target leg	
				numberOfTimesToCompoundAgainstLeg,	// [17] The number of times compoundings happen with one accural period - against leg
				spotTermTargetLeg,					// [18] Spot term - target leg
				spotTermAgainstLeg,					// [19] Spot term - against leg
				paymentDatesTargetLeg,				// [20] Date fraction between spot date and payment dates of every swap - target leg
				accrualPeriodsTargetLeg,			// [21] Date fraction of each payment period of every swap - target leg
				fixingDatesTargetLeg,				// [22] Date fraction between asof date and fixing dates of every swap - target leg
				indexTermsTargetLeg,				// [23] Date fraction of each indexing period of every swap - target leg
				paymentDatesAgainstLeg,				// [24] Date fraction between spot date and payment dates of every swap - against leg
				accrualPeriodsAgainstLeg,			// [25] Date fraction of each payment period of every swap - against leg
				fixingDatesAgainstLeg,				// [26] Date fraction between asof date and fixing dates of every swap - against leg
				indexTermsAgainstLeg,				// [27] Date fraction of each indexing period of every swap - against leg
				basisCurveRateTimesTimeVector,		// [28] A vector of the r*t term of discount factors of the target curve from spot date to each index date - target leg
				againstLegPVs,						// [29] A vector of PVs of the against leg of all the swaps - against leg
				targetValVec1,						// [30] Output of result PVs
				true,								// [31] Sign of the output PVs
				spotLag);							// [32] Date fraction between spot date and effective start date - target leg

	// Check if converged and solution found
			if (!optimizePerformance)
			{
				solutionFound = true;
				for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
				{
					// check error is within epsilon
					if (AQLMath::abs(targetValVec1[i]) >= eps)
					{
						solutionFound = false;
						break;
					}
				}
			}
			else
			{
				for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
				{
					// check error is within epsilon
					if (AQLMath::abs(targetValVec1[i]) < eps)
					{
						solutionFound = true;
						break;
					}
				}
			}

			// Exit while loop if solution has been found
			if (solutionFound)
			{
				break;
			}
		}

		// Throw error if solution has not been found
		AQ_REQUIRE(solutionFound, "Unable to calibrate the Basis Curve. The curve Newton Raphson Discount Factor solver did not converge to a solution")

			AQLPriceDataInterpolation *p_base_inter = 0;
		double b_spotdf = 1.0;
		if (isTargetDiscountCurve)
		{
			p_base_inter = &discountFactorsTargetLeg;
			b_spotdf = d_spotdf;
		}
		else
		{
			p_base_inter = &forwardRatesTargetLeg;
			b_spotdf = f_spotdf;
		}

		// Streamline all swap start and payment date fractions into a single vector 
		DoubleArray terms_mod = paymentDatesTargetLeg.back();
		if (isFwdBasis)
		{
			double grid = 0.0;
			unsigned int pos = 0;

			// Add start date of forward starting swaps to 'terms_mod'. 
			// Start date of forward starting swaps were not part of 'paymentDatesTargetLeg'
			for (unsigned int i = 0; i < spotLag.size(); ++i)
			{
				// spotLag: The date fraction between the spot date of the curve and the start date of each **forward starting swap**. 

				grid = spotLag[i];
				if (grid == 0.0)
				{
					continue;
				}

				AQLAlgorithm::locate(terms_mod, grid, terms_mod.size(), pos);
				if (pos == terms_mod.size() || terms_mod[pos] != grid)
				{
					terms_mod.insert(terms_mod.begin() + pos, grid);
				}
			}

			// Streamline all swap payment dates from 'paymentDatesTargetLeg' into 'terms_mod'
			for (unsigned int i = 0; i < paymentDatesTargetLeg.size() - 1; ++i)
			{
				for (unsigned int j = 0; j < paymentDatesTargetLeg[i].size(); ++j)
				{
					grid = paymentDatesTargetLeg[i][j];
					if (grid == 0.0)
					{
						continue;
					}
					AQLAlgorithm::locate(terms_mod, grid, terms_mod.size(), pos);
					if (pos == terms_mod.size() || terms_mod[pos] != grid)
					{
						terms_mod.insert(terms_mod.begin() + pos, grid);
					}
				}
			}
		}

		// Streamline all fixing start and end date fractions into terms_mod
		if (!isTargetDiscountCurve)
		{
			for (size_t i = 0; i < fixingDatesTargetLeg.size(); ++i)
			{
				for (size_t j = 0; j < fixingDatesTargetLeg[i].size(); ++j)
				{
					for (size_t k = 0; k < fixingDatesTargetLeg[i][j].size(); ++k)	// i: trade index; j: cash flow index of trade; k: fixing start/end dates of a particular cash flow
					{
						double fixingDateGrid = fixingDatesTargetLeg[i][j][k] - spotTermTargetLeg;

						if (fixingDateGrid < spotTermTargetLeg * -1)
						{
							continue;
						}

						unsigned int pos(0);
						AQLAlgorithm::locate(terms_mod, fixingDateGrid, terms_mod.size(), pos);
						if (pos == terms_mod.size() || terms_mod[pos] != fixingDateGrid)
						{
							terms_mod.insert(terms_mod.begin() + pos, fixingDateGrid);
						}
					}
				}
			}

			// Round the date count fractions to the e-9 level and remove duplicated entries in 'terms_mod'
			double precision = 1e-9;
			transform(terms_mod.begin() + 1, terms_mod.end(), terms_mod.begin() + 1, [precision](double x)->double
			{
				return boost::math::round(x / precision) * precision;
			});

			sort(terms_mod.begin(), terms_mod.end());
			removeDuplicatedTerms(terms_mod);

			if (terms_mod.front() == 0.0)
			{
				// remove the front element
				terms_mod.erase(terms_mod.begin());
			}
		}

		if (terms_mod.empty())
		{
			throw AQLCoreInvalidData("#Error: Dependency curves have not been built. The dependency curve(s) term and/or discount factor lookup table is empty", __FILE__, __LINE__);
		}

		// Insert FRA dates to terms_mod (node points)
		if (!isTargetDiscountCurve && is_fra_use)
		{
			terms_mod.insert(terms_mod.end(), dfResults.paymentDatesAsTerms_.begin() + 1, dfResults.paymentDatesAsTerms_.end());
			std::sort(terms_mod.begin(), terms_mod.end());
			removeDuplicatedTerms(terms_mod);
		}

		// Insert NDF dates to terms_mod (node points)
		if (isTargetDiscountCurve && ndf_size != 0)
		{
			terms_mod.insert(terms_mod.end(), NDF_DFs[0].begin() + 1, NDF_DFs[0].end());
			std::sort(terms_mod.begin(), terms_mod.end());
			removeDuplicatedTerms(terms_mod);
		}

		// Insert FXFwd dates to terms_mod (node points), starting the node after spot term
		if (isTargetDiscountCurve && fwd_size != 0)
		{
			// Before calibration, when fwd_terms is put to spread_timeVec, we artificially make it spotDate started, so here we need to copy the "adjusted" fwd_terms, 
            // so that when using spread_time.value(), it will return the correct interpolated value.
			DoubleVector spotAdjustedFwdTerms(fwd_terms.size());
			for (size_t i = 0; i < fwd_terms.size(); ++i)
			{
				spotAdjustedFwdTerms[i] = fwd_terms[i] - spotTermTargetLeg;
			}

			//fwd_size_mm is the number of instruments earlier/equal to spot term
			terms_mod.insert(terms_mod.end(), spotAdjustedFwdTerms.begin() + fwd_size_mm, spotAdjustedFwdTerms.end());

			std::sort(terms_mod.begin(), terms_mod.end());
			removeDuplicatedTerms(terms_mod);

		}

		// Determine the DFs on all the swap start, fxing and payment dates
		DoubleArray dfs(terms_mod.size());
		if (isYieldSpreadCalc)
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				const double y_spread_time = spread_time.value(terms_mod[i]);
				const double b_spread_time = -AQLMath::log(p_base_inter->value(terms_mod[i] + spotTermTargetLeg) / b_spotdf);
				dfs[i] = AQLMath::exp(-(b_spread_time + y_spread_time));
			}
		}
		else
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				dfs[i] = AQLMath::exp(-spread_time.value(terms_mod[i]));
			}
		}

		yield_mod.resize(terms_mod.size(), 0.0);

		// set spot -> asof
		if (spotTermTargetLeg > 0.0)
		{
			double spotdf = 1.0;
			double yield_spot = 0.0;

			if (isTargetDiscountCurve)
			{
				if (fwd_size > 0)
				{
					spotdf = fwd_spotdf;
				}
				else if (isYieldSpreadCalc)
				{
					spotdf = AQLMath::exp((-spread_timeVec[1] / grid_spread_time[1]) * spotTermTargetLeg) * b_spotdf;
				}
				else
				{
					spotdf = AQLMath::exp((-spread_timeVec[1] / grid_spread_time[1]) * spotTermTargetLeg);
				}
				yield_spot = -AQLMath::log(spotdf) / spotTermTargetLeg;
			}
			else
			{
				// Retrieve zero rates from the r*tau 
				AQLPriceDataInterpolation tmp_inter = *p_base_inter;
				DoubleArray tmp_yields(dfs.size());
				for (unsigned int i = 0; i < terms_mod.size(); ++i)
				{
					if (terms_mod[i] == 0)
					{
						tmp_yields[i] = 0.0;
					}
					else
					{
						tmp_yields[i] = -AQLMath::log(dfs[i]) / terms_mod[i];
					}
				}

				// Retrieve spot rate through zero rate interpolator. Calculate spot DF.
				tmp_inter.set(terms_mod, tmp_yields);
				yield_spot = tmp_inter.value(spotTermTargetLeg);
				spotdf = AQLMath::exp(-yield_spot * spotTermTargetLeg);
			}
			
			// Re-scale all discount factors and zero rates so that they all start from the asof date of the curve
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				terms_mod[i] += spotTermTargetLeg;
				dfs[i] *= spotdf;
				if (terms_mod[i] == 0)
				{
					yield_mod[i] = 0.0;
				}
				else
				{
					yield_mod[i] =  -AQLMath::log(dfs[i]) / terms_mod[i];
				}
			}

			//Add spot term, which may not always sit at the front duing to presence os fixing lags
			if (fwd_size > 0)
			{
				for (int i = fwd_size - 1; i >= 0; --i)
				{
					if (fwd_terms[i] < terms_mod.front())
					{
						terms_mod.insert(terms_mod.begin(), fwd_terms[i]);
						dfs.insert(dfs.begin(), fwd_dfs[i]);
						yield_mod.insert(yield_mod.begin(), -AQLMath::log(fwd_dfs[i]) / fwd_terms[i]);
					}
				}
			}
			else
			{
				unsigned int pos(0);
				AQLAlgorithm::locate(terms_mod, spotTermTargetLeg, terms_mod.size(), pos);
				if (pos == terms_mod.size() || terms_mod[pos] != spotTermTargetLeg)
				{
					terms_mod.insert(terms_mod.begin() + pos, spotTermTargetLeg);
					dfs.insert(dfs.begin() + pos, spotdf);
					yield_mod.insert(yield_mod.begin() + pos, yield_spot);
				}
			}
		}
		else
		{
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				yield_mod[i] = -AQLMath::log(dfs[i]) / terms_mod[i];
			}
		}

		termsmtx_fwd[0] = terms_mod;
		termsmtx_fwd[0].erase(termsmtx_fwd[0].end() - 1);
		termsmtx_fwd[1] = terms_mod;
		termsmtx_fwd[1].erase(termsmtx_fwd[1].begin());

		if (spotTermTargetLeg == 0.0 && !termsmtx_fwd[0].empty() && termsmtx_fwd[0].front() != 0.0)
		{
			termsmtx_fwd[0].insert(termsmtx_fwd[0].begin(), 0.0);
			termsmtx_fwd[1].insert(termsmtx_fwd[1].begin(), terms_mod.front());
		}

		// Fix the curve front DF to 1.0
		unsigned int pos(0);
		AQLAlgorithm::locate(terms_mod, 0.0, terms_mod.size(), pos);
		if (pos == terms_mod.size() || (pos == 0 && terms_mod[pos] != 0.0))
		{
			terms_mod.insert(terms_mod.begin(), 0.0);
			dfs.insert(dfs.begin(), 1.0);
			yield_mod.insert(yield_mod.begin(), 0.0);
		}
		else if (dfs[pos] != 1.0 || yield_mod[pos] != 0.0)
		{
			terms_mod[pos] = 0.0;
			dfs[pos] = 1.0;
			yield_mod[pos] = 0.0;
		}

		// insert extrapolation terms
		if (extra_terms.size() > 0 && extra_terms.back() + spotTermTargetLeg > terms_mod.back())
		{
			for (unsigned int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + spotTermTargetLeg;
				if (term <= terms_mod.back())
				{
					continue;
				}
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
		throw AQLCoreInvalidData("#Error: Invalid Curve Calibration Method. We support Simultaneous-Equation method only.", __FILE__, __LINE__);
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

	// Determine the interpolation scheme adopted by the target curve
	AQLString basisInter;
	double joinDateAsDouble = 0.0;
	if (isYieldSpreadCalc)
	{
		// When solving is done on the spreads, assume the reference curve's interpolation scheme is the basis interpolation
		if (isTargetDiscountCurve)
		{
			basisInter = discountFactorsTargetLeg.convertToString();
			joinDateAsDouble = (discountFactorsTargetLeg.isHybrid()) ? discountFactorsTargetLeg.getJoinDateAsDouble() : 0.0;
		}
		else
		{
			basisInter = forwardRatesTargetLeg.convertToString();
			joinDateAsDouble = (forwardRatesTargetLeg.isHybrid()) ? forwardRatesTargetLeg.getJoinDateAsDouble() : 0.0;
		}
	}
	else
	{
		basisInter = spread_time.convertToString();	// spread_time is defined by the interpolation.basis
		joinDateAsDouble = (spread_time.isHybrid()) ? spread_time.getJoinDateAsDouble() : 0.0;
	}

	saveBasisCurve(curveType, _terms, termsmtx_fwd, dfs_mod, basisInter, data_, spread_time.isHybrid(), joinDateAsDouble);
}

/*!
	@brief calc fwdfx constant curve

*/
void
CurveCalibrationData::calcFwdFXConstantCurve(void)
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
		calcFwdFXConstantCurve(curveType);
	}
}

/*!
	@brief calc fwdfx constant curve

*/
void
CurveCalibrationData::calcFwdFXConstantCurveUsingMarketName(const AQLString &mktName)
{
	AQLString curveType;
	AQLString temp = mktName;
	temp.toUpper();

	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); ++it)
	{
		AQLString market = it->second;
		if (market.toUpper() == temp)
		{
			curveType = it->first;
			break;
		}
	}
	if (curveType.size() > 0)
	{
		calcFwdFXConstantCurve(curveType);
	}
}

/*!
	@brief calc fwdfx constant curve

*/
void
CurveCalibrationData::calcFwdFXConstantCurve(const AQLString &curveType)
{
	// get yield data
	AQLObject &yData = getYieldData().get().get();
	AQLObject &colYData = getColYieldData().get().get();
	// get market data
	const AQLString& market = mAssignedCurveMktMap[curveType];
	AQLObjectHolder& mktData = getMarketDataRef(market).get(0);

	// get forecast curve
	const AQLString &fCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_FORECAST).get()).get();
	AQLPriceDataInterpolation forwardRatesTargetLeg;
	AQLDataHolder* dh = &yData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + (fCurve == STD ? AQLString("") : AQLString("_") + fCurve), NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		forwardRatesTargetLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		forwardRatesTargetLeg = dynamic_cast<AQLPriceDataInterpolation &>(yData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	}

	setCurveInterpolation(fCurve, yData, forwardRatesTargetLeg);

	// get against forecast curve
	const AQLString &a_fCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_AGTFORECAST).get()).get();
	AQLPriceDataInterpolation forwardRatesAgainstLeg;
	dh = &colYData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + (a_fCurve == STD ? AQLString("") : AQLString("_") + a_fCurve), NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		forwardRatesAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		forwardRatesAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(colYData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	}

	setCurveInterpolation(a_fCurve, colYData, forwardRatesAgainstLeg);

	// get against discount curve
	const AQLString &a_dCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_AGTDISCOUNT).get()).get();
	AQLPriceDataInterpolation discountFactorsAgainstLeg;
	dh = &colYData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + (a_dCurve == STD ? AQLString("") : AQLString("_") + a_dCurve), NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		discountFactorsAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		discountFactorsAgainstLeg = dynamic_cast<AQLPriceDataInterpolation &>(colYData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	}

	setCurveInterpolation(a_dCurve, colYData, discountFactorsAgainstLeg);

	// calc discount curve
	const AQLString &dCurve = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_DISCOUNT).get()).get();
	const AQLString &suffix = dCurve == STD ? AQLString("") : AQLString("_") + dCurve;
	const DoubleArray &_terms = dynamic_cast<const AQLDataDoubles&> ((yData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
	DoubleVector _dfs(_terms.size());
	for (unsigned int i = 0; i < _terms.size(); ++i)
	{
		_dfs[i] = discountFactorsAgainstLeg.value(_terms[i]) * forwardRatesTargetLeg.value(_terms[i]) / forwardRatesAgainstLeg.value(_terms[i]);
	}

	// get the yieldgen interpolation of this FX Fwd Const curve
	AQLString epSuffix = curveType == STD ? AQLString("") : AQLString("_") + curveType;
	epSuffix.toUpper();
	dh = &yData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix, NOCHECK);
	AQLPriceDataInterpolation inter;
	if (dh->isDefined() && !dh->isNull())
	{
		inter = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		inter = dynamic_cast<AQLPriceDataInterpolation &>(yData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	}

	// save curves
	AQLObjectHolder &objHolder = getYieldData().get();
	for (std::map<AQLString, AQLString>::const_iterator it = getAssignedCurveMktMap().begin(); it != getAssignedCurveMktMap().end(); ++it)
	{
		if (it->second == market)
		{
			const AQLString& curveName = it->first;
			const AQLString suffix = "_" + curveName;
			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix);

			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(_terms));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new AQLDataDoubles(_dfs));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION + suffix, new AQLPriceDataInterpolation()).convertFromString(inter.convertToString());
			objHolder.add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, new AQLPriceDataInterpolation()).convertFromString(inter.convertToString());

			//mBCurveGenMap[curveName] = true;
			mGCurveGenMap[curveName] = true;
		}
	}

	// set df2
	dh = &getData(IR_CALIBRATION_DATA_MAINBASISDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		setDF2();
	}
}

/*!
	@brief calc cheapest-to-deliver curve

	@param[in] curveName	Name of the CTD curve
	@param[in] csaCurves	All CSA curves contributing to CTD selection
*/
void CurveCalibrationData::calcCheapestToDeliverCurve(const AQLString& curveName, const AQLStringVector& csaCurves)
{
	// get yield data
	AQLObject &yData = getYieldData().get().get();

	// get market data
	AQLObjectHolder& mktData = getMarketDataRef(curveName).get(0);

	AQLString epSuffix = curveName == STD ? AQLString("") : AQLString("_") + curveName;
	epSuffix.toUpper();

	// Get conventions
	const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar &>(mktData.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
	const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule &>(mktData.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
	const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount &>(mktData.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
	const AQLString& frequency = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();

	// get AsOf date
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (yData.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());

	// Initialise DF interpolators
	std::vector<AQLPriceDataInterpolation> inters;
	std::set<double> largestDates;
	for (unsigned int i = 0; i < csaCurves.size(); ++i)
	{
		AQLString curve = csaCurves[i];
		AQLPriceDataInterpolation inter;
		try
		{
			// Get the interpolator			
			curve.toUpper();
			const AQLString &suffix = curve == STD ? AQLString("") : AQLString("_") + curve;

			AQLDataHolder* dh = &yData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				inter = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
			}
			else
			{
				inter = dynamic_cast<AQLPriceDataInterpolation &>(yData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
			}

			// Get the terms
			const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			largestDates.insert(terms.back());
		}
		catch (std::exception&)
		{
			AQLString err = "#Error: Curve '" + curve + "' does not exist in the object pool. Has it been built?";
			throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
		}

		inters.push_back(inter);
	}

	// Determine the date vectors used in calculating overnight forward rates
	AQLPriceDataDayCount dc_act365(ACT_365);
	double smallestCommonEndTerm = *largestDates.begin();
	AQLDate asOfDate(asof);
	AQLString daycount(dc_act365.convertToString());
	AQLDate smallestCommonEndDate = etrading::LADateScheduleHelpers::getDateFromTerm(asOfDate, smallestCommonEndTerm, daycount);

	DateVector startDates, endDates;
	AQLDate startDate = asof;
	AQLString tenor = etrading::fromFrequencyToTerm(frequency);
	while (startDate < smallestCommonEndDate)
	{
		startDates.push_back(startDate);
		AQLDate endDate = etrading::LADateHelpers::getDate(startDate, tenor, sld, &cal, true);	// Always calculate overnight forward rates
		endDates.push_back(endDate);

		startDate = endDate;
	}

	// Calculate and compare overnight forward rates of all CSA curves; imply DF from the highest rate	
	double df = 1.0;
	DoubleArray dfs;
	dfs.push_back(df);

	DoubleArray terms;
	terms.push_back(0.0);

	// Get AQLMathYieldCurve objecct and initialise it
	AQLString yieldDataName = dynamic_cast<const AQLDataString &>(getYieldData().get().get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	AQLDataInstance* dataInstance = getDataInstance();
	etrading::LACurvePricingObject yc(dataInstance);

	yc.getYieldData().convertFromString(yieldDataName);
	yc.getDayCount().setDayCount(dc.convertToString());
	yc.getSlidingRule().convertFromString(sld.convertToString());
	yc.getFrequency().convertFromString(SIMPLE);
	etrading::LACurveForwardRateHelpers::setCalendarForCurveID(yc, cal.convertToString());

	// Loop through all the CSA curves and determine the highest forward rate between two collateral posting dates
	DoubleArray highestRates;
	for (unsigned int i = 0; i < csaCurves.size(); ++i)
	{
		AQLString curve = csaCurves[i];
		yc.setInterpolation(inters[i].convertToString());
		yc.setCurveType(curve);

		// Loop through all collateral posting dates
		for (unsigned int k = 0; k < startDates.size(); ++k)
		{
			AQLDate startDate = startDates[k];
			AQLDate endDate = endDates[k];

			double currentRate = yc.getZeroRate(startDate, endDate, false, false);		// useForwardInterpolation = FALSE; useFwdData = FALSE

			if (i == 0)
			{
				// If this is the first csa curve we are examining, simply store the currentRate as the highestRate so far
				highestRates.push_back(currentRate);
			}
			else
			{
				// Keep the highest rate for this particular collateral posting date
				if (currentRate >= highestRates[k])
				{
					highestRates[k] = currentRate;
				}
			}
		}
	}

	// Finally Loop through all collateral posting dates and calculate the DF corresponding to the highestRate at each point
	for (unsigned int k = 0; k < startDates.size(); ++k)
	{
		AQLDate startDate = startDates[k];
		AQLDate endDate = endDates[k];

		// Calculate discount factor using the highest overnight forward rate
		RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
		AQLPriceDataConvention conv(dc.getDayCount(), rc);
		df *= conv.getDF(highestRates[k], startDate, endDate);
		dfs.push_back(df);

		double term = dc_act365.getTerm(asof, endDate);
		terms.push_back(term);
	}

	// get the yieldgen interpolation of this FX Fwd Const curve
	AQLDataHolder* dh = &yData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + epSuffix, NOCHECK);
	AQLPriceDataInterpolation inter;
	if (dh->isDefined() && !dh->isNull())
	{
		inter = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
	}
	else
	{
		inter = dynamic_cast<AQLPriceDataInterpolation &>(yData.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	}

	// save curves
	AQLObjectHolder &objHolder = getYieldData().get();
	for (std::map<AQLString, AQLString>::const_iterator it = getAssignedCurveMktMap().begin(); it != getAssignedCurveMktMap().end(); ++it)
	{
		if (it->second == curveName)
		{
			const AQLString& curveName = it->first;
			const AQLString suffix = "_" + curveName;
			// set result
			objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix);

			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(terms));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new AQLDataDoubles(dfs));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION + suffix, new AQLPriceDataInterpolation()).convertFromString(inter.convertToString());
			objHolder.add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, new AQLPriceDataInterpolation()).convertFromString(inter.convertToString());

			//mBCurveGenMap[curveName] = true;
			mGCurveGenMap[curveName] = true;
		}
	}

}


/*!
	@brief get market data reference

	@param[in] market name
*/
const AQLDataMultiReference&
CurveCalibrationData::getMarketDataRef(const AQLString& mktName) const
{
	AQLString suffix;
	AQLStringVector tmpMktNames = mktName.toToken('_');
	if (tmpMktNames.size() == 2)
	{
		const AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if (!eh_fy.isDefined())
		{
			throw AQLCoreInvalidData("#Error: Foreign Yield Data does not exist!", __FILE__, __LINE__);
		}
		else
		{
			suffix = "_" + tmpMktNames[1];
			return dynamic_cast<const AQLDataMultiReference&>(eh_fy.get().getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
		}
	}
	else
	{
		suffix = "_" + tmpMktNames[0];
		return dynamic_cast<const AQLDataMultiReference&>(getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
	}
}

/*!
	@brief save basis curve

	@param[in] curveType				Curve name
	@param[in] terms					Calibrated discount factor terms of target basis curve
	@param[in] termsmtx_fwd				Calibrated forward rate terms of target basis curve
	@param[in] dfs						Calibrated discount factors of target basis curve
	@param[in] interpolation			Interpolation scheme used by target basis curve
	@param[in] interpolationJoinDate	Join date used in hybrid interpolation
	@param[in] isBuiltFromEngine		Curve built by the globle yield curve engine?
	@param[out] data					Convention data
*/
void
CurveCalibrationData::saveBasisCurve(const AQLString& curveType, const DoubleArray& terms, const DoubleMatrix& termsmtx_fwd, const DoubleArray& dfs, const AQLString& interpolation, std::vector<AQLObject*>& data, const bool isHybrid, double interpolationJoinDate, bool isBuiltFromEngine)
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
			objHolder.remove(CALIBRATION_DATA_INTERPOLATION + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix);
			objHolder.remove(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix);

			objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(terms));
			objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, new AQLDataDoubleMatrix(termsmtx_fwd));
			objHolder.add(IR_CALIBRATION_DATA_DFS + suffix, new AQLDataDoubles(dfs));
			objHolder.add(CALIBRATION_DATA_INTERPOLATION + suffix, new AQLPriceDataInterpolation()).convertFromString(interpolation);
			objHolder.add(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix, new AQLPriceDataInterpolation()).convertFromString(interpolation);
			objHolder.add(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix, new AQLDataBool(isBuiltFromEngine));

			if (isHybrid)
			{
				objHolder.remove(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix);
				objHolder.add(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, new AQLDataDouble(interpolationJoinDate));
			}
			// set generate flag true
			mBCurveGenMap[curveName] = true;
			setCurveConvention(objHolder, data, curveName);
			//set STD curve
			const AQLDataHolder& dh = objHolder.getData(CALIBRATION_DATA_TERMS);
			if (!dh.isDefined() || dh.isNull())
			{
				objHolder.add(IR_CALIBRATION_DATA_ISFWDFX, new AQLDataBool(true));
				objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(terms));
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(termsmtx_fwd));
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
CurveCalibrationData::setCurveInterpolation(const AQLString& curveName, const AQLObject &yieldData, AQLPriceDataInterpolation& inter) const
{
	const AQLString &suffix = curveName == STD ? AQLString("") : AQLString("_") + curveName;
	if (inter.isHybrid())
	{
		const AQLDataHolder* dh = &(yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			double cuttOffDate = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
			inter.setJoinDateAsDouble(cuttOffDate);
		}
		else
		{
			throw AQLCoreInvalidData("#Error: Join date is required when mixed hybrid interpolation types are used.", __FILE__, __LINE__);
		}
	}

	const DoubleArray &terms = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
	const DoubleArray &dfs = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
	
	// Check for Duplicate Terms
	if ( terms.size() > 1 )
	{
		double previousIndex = terms[0];
		double thisIndex = terms[1];
		for ( size_t i = 1; i < terms.size(); ++i )
		{
			thisIndex		= terms[i];
			previousIndex	= terms[i-1];
			AQ_THROW_IF(thisIndex == previousIndex, "Invalid Curve Data: Curve '" + curveName + "' contains duplicate data with time value: " + AQ_TO_STRING_FROM_DOUBLE(thisIndex) + " years" )
		}
	}

	// Check for Error Discount Factors
	for ( size_t i = 1; i < dfs.size(); ++i )
	{
		AQ_THROW_IF( std::isnan(dfs[i]), "Invalid Curve Data: Curve '" + curveName + "' has not been built or contains invalid discount factors" )
	}

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
	@param[out] paymentDates index grid (from asofdate, daycount = ACT/365)
	@param[out] termVec index span (daycount = dc)

*/
void
CurveCalibrationData::calcIndexGrid(const AQLDate &asofdate, const AQLDate &date, int resetLag, const AQLPriceDataDayCount &dc,
	const AQLPriceDataCalendar &fixcal, const AQLPriceDataCalendar &paycal, const AQLPriceDataSlidingRule &sld,
	const AQLString &freq, const AQLString &accessary, DoubleVector &paymentDates, DoubleVector &termVec)
{
	paymentDates.clear();
	termVec.clear();

	AQLString tmpFreq = freq;
	tmpFreq.toUpper();
	int y, m, d, w;
	etrading::LADateHelpers::termStrtoYMDW(accessary, y, m, d, w);

	unsigned int size = 0;
	unsigned int addmonth = 0;
	if (d != 0)
	{
		throw AQLCoreInvalidData("d != 0 is not support", __FILE__, __LINE__);
	}
	if (0 == y && 0 == m)
	{
		throw AQLCoreInvalidData("#Error: Curve Frequency Accessory input supports only y,m,d", __FILE__, __LINE__);
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
			throw AQLCoreInvalidData("#Error: Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		size = y * 4 + m / 3 + 1;
		addmonth = 3;

	}
	else if (freq == SEMI_ANNUAL)
	{
		if (m % 6 != 0)
		{
			throw AQLCoreInvalidData("#Error: Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		size = y * 2 + m / 6 + 1;
		addmonth = 6;

	}
	else if (freq == ANNUAL)
	{
		if (m % 12 != 0)
		{
			throw AQLCoreInvalidData("#Error: Frequency and Accessory are not consistent", __FILE__, __LINE__);
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

	AQLPriceDataDayCount dc_act365(ACT_365);
	paymentDates.resize(size);
	termVec.resize(size - 1);
	AQLDate fixingdate = fixcal.getBusinessDay(date, -1 * resetLag);
	AQLDate paymentdate = paycal.getBusinessDay(fixingdate, resetLag);
	AQLDate noadjdate = paymentdate;
	paymentDates[0] = dc_act365.getTerm(asofdate, paymentdate);

	for (unsigned int i = 1; i < size; i++)
	{
		AQLDate olddate = paymentdate;
		noadjdate.addMonths(addmonth);
		paymentdate = sld.getDate(noadjdate, paycal);
		paymentDates[i] = dc_act365.getTerm(asofdate, paymentdate);
		termVec[i - 1] = dc.getTerm(olddate, paymentdate, false);
	}
}
/*!
	@brief calc basis cashflow

	@param[in] inter : inter of target spread*t
	@param[in] isTargetDiscountCurve : for generate Discount Curve or not
	@param[in] isMarkedToMarketXccySwap : is forwardfx renotional or not (spotfx renotional)
	@param[in] forwardRates : inter of target forecast DF
	@param[in] discountFactor : inter of target discount DF
	@param[in] discountFactorsAgainstLeg : inter of against discount DF
	@param[in] spotTermAdjustmentAgainstLeg : base date adjustment for against discount DF
	@param[in] convexityAdjustment : inter of convexity adjust
	@param[in] isSpreadOnAgainstLeg : spread is on against leg or not
	@param[in] swapSpreads : basis spread
	@param[in] numberOfTimesToCompound : compounding times
	@param[in] spotTerm : cashlet spot term
	@param[in] paymentDates : cashlet grid
	@param[in] accrualPeriodsTargetLeg : cashlet tau
	@param[in] fixingDatesTargetLeg : index grid
	@param[in] indexTermsTargetLeg : index tau
	@param[in] basisCurveRateTimesTimeVector : forecast r*t
	@param[in] againstLegPVs : against target leg PV
	@param[out] basisSwapPVs : output of result PV
	@param[in] isNegative : is negative or not
	@param[in] spotLag : Date fraction between spot date and effective start date
*/
void
CurveCalibrationData::calcBasisCF(const AQLPriceDataInterpolation &spreadTimesTime,
	const bool isTargetDiscountCurve,
	const bool isMarkedToMarketXccySwap,
	const bool isCurveUSD,
	const bool useForwardInterpolation,
	const AQLPriceDataInterpolation &forwardRatesTargetLeg,
	const AQLPriceDataInterpolation *forwardInterpolationTargetLeg,
	const AQLPriceDataInterpolation &discountFactorsTargetLeg,
	const AQLPriceDataInterpolation &forwardRatesAgainstLeg,
	const AQLPriceDataInterpolation *forwardInterpolationAgainstLeg,
	const AQLPriceDataInterpolation &discountFactorsAgainstLeg,
	const double spotTermAdjustmentAgainstLeg,
	const AQLPriceDataInterpolation *convexityAdjustment,
	const bool isSpreadOnAgainstLeg,
	const DoubleArray &swapSpreads,
	const int numberOfTimesToCompoundTargetLeg,
	const int numberOfTimesToCompoundAgainstLeg,
	const double spotTermTargetLeg,
	const double spotTermAgainstLeg,
	const std::vector<DoubleArray> &paymentDatesTargetLeg,
	const std::vector<DoubleArray> &accrualPeriodsTargetLeg,
	const std::vector<DoubleMatrix> &fixingDatesTargetLeg,
	const std::vector<DoubleMatrix> &indexTermsTargetLeg,
	const std::vector<DoubleArray> &paymentDatesAgainstLeg,
	const std::vector<DoubleArray> &accrualPeriodsAgainstLeg,
	const std::vector<DoubleMatrix> &fixingDatesAgainstLeg,
	const std::vector<DoubleMatrix> &indexTermsAgainstLeg,
	const std::vector<DoubleMatrix> &basisCurveRateTimesTimeVector,
	DoubleArray &againstLegPVs,
	DoubleArray &basisSwapPVs,
	const bool isNegative,
	const std::vector<double>& spotLag)
{
	unsigned int numberOfBasisSwaps = swapSpreads.size();
	if (paymentDatesTargetLeg.size() != numberOfBasisSwaps ||
		accrualPeriodsTargetLeg.size() != numberOfBasisSwaps ||
		fixingDatesTargetLeg.size() != numberOfBasisSwaps ||
		indexTermsTargetLeg.size() != numberOfBasisSwaps ||
		basisCurveRateTimesTimeVector.size() != numberOfBasisSwaps ||
		againstLegPVs.size() != numberOfBasisSwaps)
	{
		throw AQLCoreInvalidData("Invalid Basis Swap Data with inconsistent dimensions", __FILE__, __LINE__);
	}

	for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
	{
		const unsigned int gridSize = paymentDatesTargetLeg[i].size();
		if (fixingDatesTargetLeg[i].size() != gridSize ||
			accrualPeriodsTargetLeg[i].size() != gridSize ||
			basisCurveRateTimesTimeVector[i].size() != gridSize)
		{
			throw AQLCoreInvalidData("Invalid Basis Swap Data with inconsistent dimensions", __FILE__, __LINE__);
		}
	}

	DoubleArray basisSpreadTargetLeg(numberOfBasisSwaps, 0.0);
	if (!isSpreadOnAgainstLeg)
	{
		basisSpreadTargetLeg = swapSpreads;
	}

	DoubleArray targetPVs(numberOfBasisSwaps, 0.0);
	if (isTargetDiscountCurve) // for discount curve
	{
		if (isMarkedToMarketXccySwap) // fwd renotional
		{
			if (isCurveUSD) // target renotional
			{
				for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
				{
					targetPVs[i] = calcTargetPV(spreadTimesTime, useForwardInterpolation, forwardRatesTargetLeg, forwardInterpolationTargetLeg, discountFactorsAgainstLeg, convexityAdjustment, basisSpreadTargetLeg[i], numberOfTimesToCompoundTargetLeg, spotTermTargetLeg, paymentDatesTargetLeg[i],
						accrualPeriodsTargetLeg[i], fixingDatesTargetLeg[i], indexTermsTargetLeg[i], basisCurveRateTimesTimeVector[i]);
				}
			}
			else // against renotional
			{
				for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
				{
					targetPVs[i] = calcTargetPV(spreadTimesTime, useForwardInterpolation, forwardRatesTargetLeg, forwardInterpolationTargetLeg, basisSpreadTargetLeg[i], numberOfTimesToCompoundTargetLeg, paymentDatesTargetLeg[i], accrualPeriodsTargetLeg[i],
						fixingDatesTargetLeg[i], indexTermsTargetLeg[i], basisCurveRateTimesTimeVector[i], spotLag[i]);
				}

				againstLegPVs.clear();
				againstLegPVs.resize(numberOfBasisSwaps);
				DoubleArray basisSpreadsAgainstLeg(numberOfBasisSwaps, 0.0);
				if (isSpreadOnAgainstLeg)
				{
					basisSpreadsAgainstLeg = swapSpreads;
				}

				for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
				{
					againstLegPVs[i] = calcAgainstPV(useForwardInterpolation, forwardRatesAgainstLeg, forwardInterpolationTargetLeg, discountFactorsAgainstLeg, spreadTimesTime, convexityAdjustment, basisSpreadsAgainstLeg[i], numberOfTimesToCompoundAgainstLeg,
						spotTermAgainstLeg, spotTermTargetLeg, paymentDatesAgainstLeg[i], accrualPeriodsAgainstLeg[i], fixingDatesAgainstLeg[i], indexTermsAgainstLeg[i], spotLag[i]) * spotTermAdjustmentAgainstLeg;
				}
			}
		}
		else // spot renotional
		{
			for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
			{
				targetPVs[i] = calcTargetPV(spreadTimesTime, useForwardInterpolation, forwardRatesTargetLeg, forwardInterpolationTargetLeg, basisSpreadTargetLeg[i], numberOfTimesToCompoundTargetLeg, paymentDatesTargetLeg[i], accrualPeriodsTargetLeg[i],
					fixingDatesTargetLeg[i], indexTermsTargetLeg[i], basisCurveRateTimesTimeVector[i], spotLag[i]);
			}
		}
	}
	else // for forecast curve
	{
		for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
		{
			targetPVs[i] = calcTargetPV(spreadTimesTime, discountFactorsTargetLeg, basisSpreadTargetLeg[i], numberOfTimesToCompoundTargetLeg, spotTermTargetLeg, paymentDatesTargetLeg[i],
				accrualPeriodsTargetLeg[i], fixingDatesTargetLeg[i], indexTermsTargetLeg[i], basisCurveRateTimesTimeVector[i], spotLag[i]);
		}
	}

	basisSwapPVs.clear();
	basisSwapPVs.resize(numberOfBasisSwaps);
	const double sign = isNegative ? -1.0 : 1.0;
	for (unsigned int i = 0; i < numberOfBasisSwaps; ++i)
	{
		basisSwapPVs[i] = sign * (againstLegPVs[i] - targetPVs[i]);
	}
}

/*!
	@brief calc against PV (spot renotional)

	@param[in] useForwardInterpolation : use inter of forward rate or not
	@param[in] forwardRates : inter of forecast DF
	@param[in] forwardInterpolation : inter of forcast forward rate
	@param[in] discountFactor : inter of discount DF
	@param[in] spread : basis spread
	@param[in] numberOfTimesToCompound : compounding times
	@param[in] spotDate : term of spot date
	@param[in] cashflowDates : cashlet grid
	@param[in] cashflowAccuralPeriods : cashlet tau
	@param[in] indexStartAndEndDates : index grid
	@param[in] indexAccrualPeriods : index tau
	@param[in] effectiveStartingTerm : Date fraction between spot date and effective start date

	@return against PV
*/
double
CurveCalibrationData::calcAgainstPV(const bool useForwardInterpolation,
	const AQLPriceDataInterpolation &forwardRates,
	const AQLPriceDataInterpolation *forwardInterpolation,
	const AQLPriceDataInterpolation &discountFactor,
	const double spread,
	const int numberOfTimesToCompound,
	const double spotDate,
	const DoubleArray &cashflowDates,
	const DoubleArray &cashflowAccuralPeriods,
	const DoubleMatrix &indexStartAndEndDates,
	const DoubleMatrix &indexAccrualPeriods,
	const double effectiveStartingTerm)
{
	double resultPV = 0.0;
	if (cashflowDates.empty())
	{
		return resultPV;
	}

	// Validation of Interpolators - Check the Against Curve has Sufficient Calibration Instruments to Support the Basis Curve
	// ****************************
	// Interpolation Extrapolation Can Lead to Spurious Results and Curve Build Failure
	// This is beacuse the interpolators do not respect data boundaries when extrapolating,
	// which can lead to unusable data such as negative discount factors et al.
	// -----------------------------------------------------------------
	double maxCashflowDateAsDouble = cashflowDates.back();
	const double extrapolationTolerance = 2.0; // i.e. a very generous 2 year tolerance

	if ( !forwardRates.isNullOrUndefined() && forwardRates.getMethod().getXValues().size() != 0 )
	{
		double maxForwardRates = forwardRates.getMethod().getXValues().back();
		AQ_REQUIRE( maxCashflowDateAsDouble < maxForwardRates + extrapolationTolerance, "Invalid Forward Dependency Curve: The Basis Curve is Longer than the Dependent FORWARD Curve. The dependent FORWARD curve does not have enough instruments to support the basis curve. ( Basis Max Tenor in Years: " + AQ_TO_STRING_FROM_DOUBLE( maxCashflowDateAsDouble ) + " vs Dependent Forward Curve : " + AQ_TO_STRING_FROM_DOUBLE( maxForwardRates ) + " )" )
	}

	if ( !forwardInterpolation->isNullOrUndefined() && forwardInterpolation->getMethod().getXValues().size() != 0 )
	{
		double maxForwardInterpolation = forwardInterpolation->getMethod().getXValues().back();
		AQ_REQUIRE( maxCashflowDateAsDouble < maxForwardInterpolation + extrapolationTolerance, "Invalid Forward Dependency Curve: The Basis Curve is Longer than the Dependent FORWARD Curve. The dependent FORWARD curve does not have enough instruments to support the basis curve. ( Basis Max Tenor in Years: " + AQ_TO_STRING_FROM_DOUBLE( maxCashflowDateAsDouble ) + " vs Dependent Forward Curve : " + AQ_TO_STRING_FROM_DOUBLE( maxForwardInterpolation ) + " )" )
	}

	if ( !discountFactor.isNullOrUndefined() && discountFactor.getMethod().getXValues().size() != 0 )
	{
		double maxDiscountFactor = discountFactor.getMethod().getXValues().back();
		AQ_REQUIRE( maxCashflowDateAsDouble < maxDiscountFactor + extrapolationTolerance, "Invalid Discount Factor Dependency Curve: The Basis Curve is Longer than the Dependent DISCOUNT FACTOR Curve. The dependent DISCOUNT FACTOR curve does not have enough instruments to support the basis curve. ( Basis Max Tenor in Years: " + AQ_TO_STRING_FROM_DOUBLE( maxCashflowDateAsDouble ) + " vs Dependent Discount Factor Curve : " + AQ_TO_STRING_FROM_DOUBLE( maxDiscountFactor ) + " )" )
	}
	// -----------------------------------------------------------------

	const double spotDiscountFactor = discountFactor.value(spotDate);

	double df = 1.0;
	unsigned int compoundStartIndex = 1;
	for (compoundStartIndex = 1; compoundStartIndex * numberOfTimesToCompound - 1 < cashflowDates.size(); compoundStartIndex++) // compound grid
	{
		// calc compounding
		double coupon = 0.0;
		for (unsigned int j = (compoundStartIndex - 1) * numberOfTimesToCompound; j < compoundStartIndex * numberOfTimesToCompound; j++)
		{
			// calc index
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[j].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[j].size(); thisCouponIndex++)
				{
					double temp = forwardRates.value(indexStartAndEndDates[j][thisCouponIndex]);
					sumRateTimesTime += indexAccrualPeriods[j][thisCouponIndex - 1] * temp;
				}

				double frontDF = forwardRates.value(indexStartAndEndDates[j].front());
				double backDF = forwardRates.value(indexStartAndEndDates[j].back());
				rate = (frontDF - backDF) / sumRateTimesTime;
			}
			coupon += ((rate + spread) + coupon * rate) * cashflowAccuralPeriods[j];
		}

		double endTerm = cashflowDates[compoundStartIndex * numberOfTimesToCompound - 1] + spotDate;
		df = discountFactor.value(endTerm) / spotDiscountFactor;

		resultPV += coupon * df;
	}

	if (cashflowDates.size() % numberOfTimesToCompound != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (compoundStartIndex - 1) * numberOfTimesToCompound; j < cashflowDates.size(); j++)
		{
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[j].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[j].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[j][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[j][thisCouponIndex]);
				}
				rate = (forwardRates.value(indexStartAndEndDates[j].front()) - forwardRates.value(indexStartAndEndDates[j].back())) / sumRateTimesTime;
			}

			double endTerm = cashflowDates[j] + spotDate;
			df = discountFactor.value(endTerm) / spotDiscountFactor;

			resultPV += (rate + spread) * cashflowAccuralPeriods[j] * df;
		}
	}

	// Notional exchange
	// Both 'df' and 'spotDiscountFactor' are discount factors back to the spot date. 
	// 'resultPV' is the swap PV as of the spot date.
	const double df_start = discountFactor.value(spotDate + effectiveStartingTerm) / spotDiscountFactor;
	resultPV += 1.0 * df - 1.0 * df_start;

	return resultPV;
}

/*!
	@brief calc against PV (forward renotional)

	@param[in] useForwardInterpolation : use inter of forward rate or not
	@param[in] forwardRates : inter of forecast DF
	@param[in] forwardInterpolation : inter of forcast forward rate
	@param[in] discountFactor : inter of discount DF
	@param[in] spreadInterpolation : inter of against spread*t
	@param[in] convexityAdjustment : inter of convexity adjust
	@param[in] spread : basis spread
	@param[in] numberOfTimesToCompound : compounding times
	@param[in] spotDate : term of spot date
	@param[in] spotDateAgainstLeg : term of against spot date
	@param[in] cashflowDates : cashlet grid
	@param[in] cashflowAccuralPeriods : cashlet tau
	@param[in] indexStartAndEndDates : index grid
	@param[in] indexAccrualPeriods : index tau
	@param[in] effectiveStartingTerm : Date fraction between spot date and effective start date

	@return against PV
*/
double CurveCalibrationData::calcAgainstPV( const bool useForwardInterpolation,
										    const AQLPriceDataInterpolation &forwardRates,
										    const AQLPriceDataInterpolation *forwardInterpolation,
										    const AQLPriceDataInterpolation &discountFactor,
										    const AQLPriceDataInterpolation &spreadInterpolation,
										    const AQLPriceDataInterpolation *convexityAdjustment,
										    const double spread,
										    const int numberOfTimesToCompound,
										    const double spotDate,
										    const double spotDateAgainstLeg,
										    const DoubleArray &cashflowDates,
										    const DoubleArray &cashflowAccuralPeriods,
										    const DoubleMatrix &indexStartAndEndDates,
										    const DoubleMatrix &indexAccrualPeriods,
										    const double effectiveStartingTerm )
{
	double resultPV = 0.0;
	if (cashflowDates.empty())
	{
		return resultPV;
	}

	// Validation of Interpolators - Check the Against Curve has Sufficient Calibration Instruments to Support the Basis Curve
	// ****************************
	// Interpolation Extrapolation Can Lead to Spurious Results and Curve Build Failure
	// This is beacuse the interpolators do not respect data boundaries when extrapolating,
	// which can lead to unusable data such as negative discount factors et al.
	// -----------------------------------------------------------------
	double maxCashflowDateAsDouble = cashflowDates.back();
	const double extrapolationTolerance = 2.0; // i.e. a very generous 2 year tolerance

	if ( !forwardRates.isNullOrUndefined() && forwardRates.getMethod().getXValues().size() != 0 )
	{
		double maxForwardRates = forwardRates.getMethod().getXValues().back();
		AQ_REQUIRE( maxCashflowDateAsDouble < maxForwardRates + extrapolationTolerance, "Invalid Forward Dependency Curve: The Basis Curve is Longer than the Dependent FORWARD Curve. The dependent FORWARD curve does not have enough instruments to support the basis curve. ( Basis Max Tenor in Years: " + AQ_TO_STRING_FROM_DOUBLE( maxCashflowDateAsDouble ) + " vs Dependent Forward Curve : " + AQ_TO_STRING_FROM_DOUBLE( maxForwardRates ) + " )" )
	}

	if ( !forwardInterpolation->isNullOrUndefined() && forwardInterpolation->getMethod().getXValues().size() != 0 )
	{
		double maxForwardInterpolation = forwardInterpolation->getMethod().getXValues().back();
		AQ_REQUIRE( maxCashflowDateAsDouble < maxForwardInterpolation + extrapolationTolerance, "Invalid Forward Dependency Curve: The Basis Curve is Longer than the Dependent FORWARD Curve. The dependent FORWARD curve does not have enough instruments to support the basis curve. ( Basis Max Tenor in Years: " + AQ_TO_STRING_FROM_DOUBLE( maxCashflowDateAsDouble ) + " vs Dependent Forward Curve : " + AQ_TO_STRING_FROM_DOUBLE( maxForwardInterpolation ) + " )" )
	}

	if ( !discountFactor.isNullOrUndefined() && discountFactor.getMethod().getXValues().size() != 0 )
	{
		double maxDiscountFactor = discountFactor.getMethod().getXValues().back();
		AQ_REQUIRE( maxCashflowDateAsDouble < maxDiscountFactor + extrapolationTolerance, "Invalid Discount Factor Dependency Curve: The Basis Curve is Longer than the Dependent DISCOUNT FACTOR Curve. The dependent DISCOUNT FACTOR curve does not have enough instruments to support the basis curve. ( Basis Max Tenor in Years: " + AQ_TO_STRING_FROM_DOUBLE( maxCashflowDateAsDouble ) + " vs Dependent Discount Factor Curve : " + AQ_TO_STRING_FROM_DOUBLE( maxDiscountFactor ) + " )" )
	}
	// -----------------------------------------------------------------

	const double spotDiscountFactor = discountFactor.value(spotDate);

	double df = 1.0;
	double df_0 = 1.0;
	double df_0_agt = 1.0;
	double fwdfx = 1.0;
	unsigned int compoundStartIndex = 1;
	for (compoundStartIndex = 1; compoundStartIndex * numberOfTimesToCompound - 1 < cashflowDates.size(); compoundStartIndex++) // compound grid
	{
		// calc compounding
		double coupon = 0.0;
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < compoundStartIndex * numberOfTimesToCompound; compoundEndIndex++)
		{
			// calc index
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[compoundEndIndex].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[compoundEndIndex][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[compoundEndIndex][thisCouponIndex]);
				}

				// r = (DF0 - DF1)/tau*DF1
				rate = (forwardRates.value(indexStartAndEndDates[compoundEndIndex].front()) - forwardRates.value(indexStartAndEndDates[compoundEndIndex].back())) / sumRateTimesTime;
			}
			coupon += ((rate + spread) + coupon * rate) * cashflowAccuralPeriods[compoundEndIndex];
		}

		double endTerm = cashflowDates[compoundStartIndex * numberOfTimesToCompound - 1] + spotDate;
		df = discountFactor.value(endTerm) / spotDiscountFactor;

		df_0 = discountFactor.value(indexStartAndEndDates[compoundStartIndex * numberOfTimesToCompound - 1].front()) / spotDiscountFactor;
		df_0_agt = AQLMath::exp(-spreadInterpolation.value(indexStartAndEndDates[compoundStartIndex * numberOfTimesToCompound - 1].front() - spotDateAgainstLeg));
		fwdfx = df_0_agt / df_0; // ForwardFX(domestic/foreign) = DF_foregin / DF_domestic

		// get convexity adjust value
		double adjust_value = 1.0;
		if (convexityAdjustment)
		{
			adjust_value = convexityAdjustment->value(indexStartAndEndDates[compoundStartIndex * numberOfTimesToCompound - 1].front());
		}

		resultPV += ((1.0 + coupon) * df - df_0) * fwdfx * adjust_value;
	}
	if (cashflowDates.size() % numberOfTimesToCompound != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
		{
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[compoundEndIndex].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[compoundEndIndex][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[compoundEndIndex][thisCouponIndex]);
				}
				rate = (forwardRates.value(indexStartAndEndDates[compoundEndIndex].front()) - forwardRates.value(indexStartAndEndDates[compoundEndIndex].back())) / sumRateTimesTime;
			}

			double endTerm = cashflowDates[compoundEndIndex] + spotDate;
			df = discountFactor.value(endTerm) / spotDiscountFactor;

			df_0 = discountFactor.value(indexStartAndEndDates[compoundEndIndex].front()) / spotDiscountFactor;
			df_0_agt = AQLMath::exp(-spreadInterpolation.value(indexStartAndEndDates[compoundEndIndex].front() - spotDateAgainstLeg));
			fwdfx = df_0_agt / df_0;

			double adjust_value = 1.0;
			if (convexityAdjustment)
			{
				adjust_value = convexityAdjustment->value(indexStartAndEndDates[compoundEndIndex].front());
			}

			resultPV += ((1.0 + (rate + spread) * cashflowAccuralPeriods[compoundEndIndex]) * df - df_0) * fwdfx * adjust_value;
		}
	}
	return resultPV;
}

/*
	@brief calc target PV for Newton-Raphson method to generate Discount Curve (spot renotional)

	@param[in] spreadTimesTime : inter of target spread*t
	@param[in] forwardRates : inter of forecast DF
	@param[in] spread : basis spread
	@param[in] numberOfTimesToCompound : compounding times
	@param[in] cashflowDates : cashlet grid
	@param[in] cashflowAccuralPeriods : cashlet tau
	@param[in] indexStartAndEndDates : index grid
	@param[in] indexAccrualPeriods : index tau
	@param[in] basisCurveRateTimesTime : forecast r*t
	@param[in] effectiveStartTerm : Date fractions between spot date and effective start date

	@return target PV
*/
double
CurveCalibrationData::calcTargetPV(const AQLPriceDataInterpolation &spreadTimesTime,
	const bool useForwardInterpolation,
	const AQLPriceDataInterpolation &forwardRates,
	const AQLPriceDataInterpolation *forwardInterpolation,
	const double spread,
	const int numberOfTimesToCompound,
	const DoubleArray &cashflowDates,
	const DoubleArray &cashflowAccuralPeriods,
	const DoubleMatrix &indexStartAndEndDates,
	const DoubleMatrix &indexAccrualPeriods,
	const DoubleMatrix &basisCurveRateTimesTime,
	const double effectiveStartTerm)
{
	double resultPV = 0.0;
	if (cashflowDates.empty())
	{
		return resultPV;
	}

	for (unsigned int compoundEndIndex = 0; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
	{
		if (basisCurveRateTimesTime[compoundEndIndex].empty())
		{
			throw AQLCoreInvalidData("#Error: size is not consistent.", __FILE__, __LINE__);
		}
	}

	double df = 1.0;
	unsigned int compoundStartIndex = 1;
	for (compoundStartIndex = 1; compoundStartIndex * numberOfTimesToCompound - 1 < cashflowDates.size(); compoundStartIndex++) // compound grid
	{
		// calc compounding
		double coupon = 0.0;
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < compoundStartIndex * numberOfTimesToCompound; compoundEndIndex++)
		{
			// calc index
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[compoundEndIndex].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[compoundEndIndex][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[compoundEndIndex][thisCouponIndex]);
				}
				rate = (forwardRates.value(indexStartAndEndDates[compoundEndIndex].front()) - forwardRates.value(indexStartAndEndDates[compoundEndIndex].back())) / sumRateTimesTime;
			}
			coupon += ((rate + spread) + coupon * rate) * cashflowAccuralPeriods[compoundEndIndex];
		}
		const double y_spread_time = spreadTimesTime.value(cashflowDates[compoundStartIndex * numberOfTimesToCompound - 1]);
		const double yield_time = y_spread_time + basisCurveRateTimesTime[compoundStartIndex * numberOfTimesToCompound - 1][0];
		df = AQLMath::exp(-yield_time);
		resultPV += coupon * df;
	}
	if (compoundStartIndex * numberOfTimesToCompound % cashflowDates.size() != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
		{
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[compoundEndIndex].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[compoundEndIndex][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[compoundEndIndex][thisCouponIndex]);
				}
				rate = (forwardRates.value(indexStartAndEndDates[compoundEndIndex].front()) - forwardRates.value(indexStartAndEndDates[compoundEndIndex].back())) / sumRateTimesTime;
			}
			const double y_spread_time = spreadTimesTime.value(cashflowDates[compoundEndIndex]);
			const double yield_time = y_spread_time + basisCurveRateTimesTime[compoundEndIndex][0];
			df = AQLMath::exp(-yield_time);
			resultPV += (rate + spread) * cashflowAccuralPeriods[compoundEndIndex] * df;
		}
	}

	const double startDF_spread_time = spreadTimesTime.value(effectiveStartTerm);
	const double startDF = AQLMath::exp(-startDF_spread_time);
	return resultPV + 1.0 * df - 1.0 * startDF;
}

/*
	@brief calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)

	@param[in] spreadTimesTime : inter of target spread*t
	@param[in] forwardRates : inter of forecast DF
	@param[in] discountFactorsAgainstLeg : inter of against discount DF
	@param[in] convexityAdjustment : inter of convexity adjust
	@param[in] spread : basis spread
	@param[in] numberOfTimesToCompound : compounding times
	@param[in] spotDate : cashlet spot term
	@param[in] cashflowDates : cashlet grid
	@param[in] cashflowAccuralPeriods : cashlet tau
	@param[in] spotDate : cashlet spot term
	@param[in] indexStartAndEndDates : index grid
	@param[in] indexAccrualPeriods : index tau
	@param[in] basisCurveRateTimesTime : base curve r*t

	@return target PV
*/
double
CurveCalibrationData::calcTargetPV(const AQLPriceDataInterpolation &spreadTimesTime,
	const bool useForwardInterpolation,
	const AQLPriceDataInterpolation &forwardRates,
	const AQLPriceDataInterpolation *forwardInterpolation,
	const AQLPriceDataInterpolation &discountFactorsAgainstLeg,
	const AQLPriceDataInterpolation *convexityAdjustment,
	const double spread,
	const int numberOfTimesToCompound,
	const double spotDate,
	const DoubleArray &cashflowDates,
	const DoubleArray &cashflowAccuralPeriods,
	const DoubleMatrix &indexStartAndEndDates,
	const DoubleMatrix &indexAccrualPeriods,
	const DoubleMatrix &basisCurveRateTimesTime)
{
	double resultPV = 0.0;
	if (cashflowDates.empty())
	{
		return resultPV;
	}

	for (unsigned int compoundEndIndex = 0; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
	{
		if (basisCurveRateTimesTime[compoundEndIndex].empty())
		{
			throw AQLCoreInvalidData("#Error: size is not consistent.", __FILE__, __LINE__);
		}
	}

	const double a_df_spot = discountFactorsAgainstLeg.value(spotDate);

	double df = 1.0;
	double df_0 = 1.0;
	double df_0_agt = 1.0;
	double fwdfx = 1.0;
	unsigned int compoundStartIndex = 1;
	for (compoundStartIndex = 1; compoundStartIndex * numberOfTimesToCompound - 1 < cashflowDates.size(); compoundStartIndex++) // compound grid
	{
		// calc compounding
		double coupon = 0.0;
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < compoundStartIndex * numberOfTimesToCompound; compoundEndIndex++)
		{
			// calc index
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[compoundEndIndex].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[compoundEndIndex][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[compoundEndIndex][thisCouponIndex]);
				}
				rate = (forwardRates.value(indexStartAndEndDates[compoundEndIndex].front()) - forwardRates.value(indexStartAndEndDates[compoundEndIndex].back())) / sumRateTimesTime;
			}
			coupon += ((rate + spread) + coupon * rate) * cashflowAccuralPeriods[compoundEndIndex];
		}

		const double y_spread_time = spreadTimesTime.value(cashflowDates[compoundStartIndex * numberOfTimesToCompound - 1]);
		const double yield_time = y_spread_time + basisCurveRateTimesTime[compoundStartIndex * numberOfTimesToCompound - 1][0];
		df = AQLMath::exp(-yield_time);

		const double y_spread_time_0 = spreadTimesTime.value(indexStartAndEndDates[compoundStartIndex * numberOfTimesToCompound - 1].front() - spotDate);
		const double yield_time_0 = y_spread_time_0 + basisCurveRateTimesTime[compoundStartIndex * numberOfTimesToCompound - 1].front();
		df_0 = AQLMath::exp(-yield_time_0);
		df_0_agt = discountFactorsAgainstLeg.value(indexStartAndEndDates[compoundStartIndex * numberOfTimesToCompound - 1].front()) / a_df_spot;
		fwdfx = df_0_agt / df_0; // ForwardFX(domestic/foreign) = DF_foregin / DF_domestic

		// get convexity adjust value
		double adjust_value = 1.0;
		if (convexityAdjustment)
		{
			adjust_value = convexityAdjustment->value(indexStartAndEndDates[compoundStartIndex * numberOfTimesToCompound - 1].front());
		}

		resultPV += ((1.0 + coupon) * df - df_0) * fwdfx * adjust_value;
	}

	if (compoundStartIndex * numberOfTimesToCompound % cashflowDates.size() != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
		{
			double rate = 0.0;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(indexStartAndEndDates[compoundEndIndex].front());
			}
			else
			{
				double sumRateTimesTime = 0.0;
				for (unsigned int thisCouponIndex = 1; thisCouponIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponIndex++)
				{
					sumRateTimesTime += indexAccrualPeriods[compoundEndIndex][thisCouponIndex - 1] * forwardRates.value(indexStartAndEndDates[compoundEndIndex][thisCouponIndex]);
				}
				rate = (forwardRates.value(indexStartAndEndDates[compoundEndIndex].front()) - forwardRates.value(indexStartAndEndDates[compoundEndIndex].back())) / sumRateTimesTime;
			}

			const double y_spread_time = spreadTimesTime.value(cashflowDates[compoundEndIndex]);
			const double yield_time = y_spread_time + basisCurveRateTimesTime[compoundEndIndex][0];
			df = AQLMath::exp(-yield_time);

			const double y_spread_time_0 = spreadTimesTime.value(indexStartAndEndDates[compoundEndIndex].front() - spotDate);
			const double yield_time_0 = y_spread_time_0 + basisCurveRateTimesTime[compoundEndIndex].front();
			df_0 = AQLMath::exp(-yield_time_0);
			df_0_agt = discountFactorsAgainstLeg.value(indexStartAndEndDates[compoundEndIndex].front()) / a_df_spot;
			fwdfx = df_0_agt / df_0;

			double adjust_value = 1.0;
			if (convexityAdjustment)
			{
				adjust_value = convexityAdjustment->value(indexStartAndEndDates[compoundEndIndex].front());
			}

			resultPV += ((1.0 + (rate + spread) * cashflowAccuralPeriods[compoundEndIndex]) * df - df_0) * fwdfx * adjust_value;
		}
	}
	return resultPV;
}

/*!
	@brief calc target PV for Newton-Raphson method to generate Forecast Curve

	@param[in] spreadTimesTime : inter of target spread*t
	@param[in] discountFactor : inter of target discount DF
	@param[in] spread : basis spread
	@param[in] numberOfTimesToCompound : compounding times
	@param[in] spotDate : term of spot date
	@param[in] cashflowDates : cashlet grid
	@param[in] cashflowAccuralPeriods : cashlet tau
	@param[in] indexStartAndEndDates : index grid
	@param[in] indexAccrualPeriods : index tau
	@param[in] basisCurveRateTimesTime : forecast r*t
	@param[in] spotLag : Date fractions between spot date and effective start date

	@return target PV
*/
double
CurveCalibrationData::calcTargetPV(const AQLPriceDataInterpolation &spreadTimesTime,
	const AQLPriceDataInterpolation &discountFactor,
	const double spread,
	const int numberOfTimesToCompound,
	const double spotDate,
	const DoubleArray &cashflowDates,
	const DoubleArray &cashflowAccuralPeriods,
	const DoubleMatrix &indexStartAndEndDates,
	const DoubleMatrix &indexAccrualPeriods,
	const DoubleMatrix &basisCurveRateTimesTime,
	const double spotLag)
{
	double targetLegPV = 0.0;
	if (cashflowDates.empty())
	{
		return targetLegPV;
	}

	for (unsigned int compoundEndIndex = 0; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
	{
		AQ_REQUIRE(!basisCurveRateTimesTime[compoundEndIndex].empty(), "#Basis Curve Calibration Error: The basis curve state variable is empty")
	}

	const double spotDiscountFactor = discountFactor.value(spotDate);
	double compounded_df_end = spotDiscountFactor;

	unsigned int compoundStartIndex = 1;

	// Compound Dates
	for (compoundStartIndex = 1; compoundStartIndex * numberOfTimesToCompound - 1 < cashflowDates.size(); compoundStartIndex++)
	{
		// Compouding Calculation
		double coumpoundedCoupon = 0.0;
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < compoundStartIndex * numberOfTimesToCompound; compoundEndIndex++)
		{
			// Index Calculation
			double compoundedEndDF = 1.0;
			double annuity = 0.0;

			// cashflowDates:
			// The date fractions between the **spot date** of the curve and the individual cash flow dates of a swap
			//
			// indexStartAndEndDates:
			// The date fraction between the **asof date** of the curve and both the start date and the end date of each cash flow of a spot or forward starting swaps
			//
			// spreadTimesTime:
			// Spread interpolator to be added to the r*t term of the target curve

			// Compounding Calculations: Calculate the Annuity using the Compounded Discount Factor
			for (unsigned int thisCouponEndIndex = 1; thisCouponEndIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponEndIndex++)
			{
				const double indexEndDate = indexStartAndEndDates[compoundEndIndex][thisCouponEndIndex] - spotDate;
				const double indexEndSpreadTimesTime = spreadTimesTime.value(indexEndDate);
				const double indexEndYield = indexEndSpreadTimesTime + basisCurveRateTimesTime[compoundEndIndex][thisCouponEndIndex];

				compoundedEndDF = AQLMath::exp(-indexEndYield);
				annuity += indexAccrualPeriods[compoundEndIndex][thisCouponEndIndex - 1] * compoundedEndDF;
			}

			const double indexSpotDateAsDouble = indexStartAndEndDates[compoundEndIndex][0] - spotDate;
			const double indexSpotSpreadTimesTime = spreadTimesTime.value(indexSpotDateAsDouble);
			const double indexSpotYieldTimesTime = indexSpotSpreadTimesTime + basisCurveRateTimesTime[compoundEndIndex][0];
			const double indexSpotDF = AQLMath::exp(-indexSpotYieldTimesTime);

			// calculate the par rate
			AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(annuity), "Basis Curve Calibration Error: Unable to solve for the par rate. The swap annuity term is zero.")
				const double pvFloat = (indexSpotDF - compoundedEndDF);
			const double rate = pvFloat / annuity;

			coumpoundedCoupon += ((rate + spread) + coumpoundedCoupon * rate) * cashflowAccuralPeriods[compoundEndIndex];
		}

		double endTerm = cashflowDates[compoundStartIndex * numberOfTimesToCompound - 1] + spotDate;
		compounded_df_end = discountFactor.value(endTerm) / spotDiscountFactor;
		targetLegPV += coumpoundedCoupon * compounded_df_end;
	}

	// Compound any partial coupons; for broken dated swaps and swaps with short- and long-stubs
	if (cashflowDates.size() % numberOfTimesToCompound != 0)
	{
		for (unsigned int compoundEndIndex = (compoundStartIndex - 1) * numberOfTimesToCompound; compoundEndIndex < cashflowDates.size(); compoundEndIndex++)
		{
			double indexEndDF = 1.0;
			double annuity = 0.0;
			for (unsigned int thisCouponEndIndex = 1; thisCouponEndIndex < indexStartAndEndDates[compoundEndIndex].size(); thisCouponEndIndex++)
			{
				const double indexEndDate = indexStartAndEndDates[compoundEndIndex][thisCouponEndIndex] - spotDate;
				const double indexEndSpreadTimesTime = spreadTimesTime.value(indexEndDate);
				const double indexEndYieldTimesTime = indexEndSpreadTimesTime + basisCurveRateTimesTime[compoundEndIndex][thisCouponEndIndex];

				indexEndDF = AQLMath::exp(-indexEndYieldTimesTime);
				annuity += indexAccrualPeriods[compoundEndIndex][thisCouponEndIndex - 1] * indexEndDF;
			}

			const double indexSpotDate = indexStartAndEndDates[compoundEndIndex][0] - spotDate;
			const double indexSpotSpreadTimesTime = spreadTimesTime.value(indexSpotDate);
			const double indexSpotYieldTimesTime = indexSpotSpreadTimesTime + basisCurveRateTimesTime[compoundEndIndex][0];
			const double indexSpotDF = AQLMath::exp(-indexSpotYieldTimesTime);

			AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(annuity), "Basis Curve Calibration Error: Unable to solve for the par rate. The swap annuity term is zero.")
				const double rate = (indexSpotDF - indexEndDF) / annuity;
			const double endTerm = cashflowDates[compoundEndIndex] + spotDate;

			compounded_df_end = discountFactor.value(endTerm) / spotDiscountFactor;
			targetLegPV += (rate + spread) * cashflowAccuralPeriods[compoundEndIndex] * compounded_df_end;
		}
	}

	const double compounded_df_start = discountFactor.value(spotDate + spotLag) / spotDiscountFactor;
	targetLegPV += 1.0 * compounded_df_end - 1.0 * compounded_df_start;

	return targetLegPV;
}

// Calibrate Basis Curve and Set Basis Rates
void CurveCalibrationData::setBasisRates(void)
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
			if (dh->isDefined() && !dh->isNull())
			{
				isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}

			bool isCurveAttrExist = checkCurveAttr(getMarketForCurve(*it));
			if (!isCheckCurves || !isCurveAttrExist)
			{
				dh = &getData(IR_CALIBRATION_DATA_BASISTARGETDF, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const AQLString &targetDF = dynamic_cast<const AQLDataString &>(dh->get()).get();
					if (AQLString(targetDF).toUpper() == AQLString(*it).toUpper())
					{
                        // *** Main Basis Curve Calibration Routine ***
						setBasisRates(*it);
					}
					else
					{
						mBCurveGenMap[*it] = true;
					}
				}
				else
				{
                    // *** Main Basis Curve Calibration Routine ***
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
        // *** Legacy Basis Curve Calibration Routine ***
		setBasisRates_old();
	}
}

/*!
	@brief a function for setting DF2

*/
void
CurveCalibrationData::setDF2(void)
{
	/*
		const AQLString &mainCName = dynamic_cast<const AQLDataString &>(getData(IR_CALIBRATION_DATA_MAINBASISDF, ISNOTNULL).get()).get();
		if (mGCurveGenMap.find(mainCName) == mGCurveGenMap.end() &&
			mBCurveGenMap.find(mainCName) == mBCurveGenMap.end() &&
			mArbFreeCurveGenMap.find(mainCName) == mArbFreeCurveGenMap.end())
		{
			return;
		}

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
		AQLDataHolder& dh = objHolder.getData(CALIBRATION_DATA_TERMS, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			terms = dynamic_cast<AQLDataDoubles &>(objHolder.getData(CALIBRATION_DATA_TERMS, NOCHECK).get()).get();
		}
		// adjust terms
		AQLPriceDataInterpolation inter = getInterpolation();
		inter.set(terms_mod, dfs_mod_);
		dfs_mod.resize(terms.size());
		for (unsigned int i = 0; i < terms.size(); ++i)
		{
			dfs_mod[i] = inter.value(terms[i]);
		}
		// set DF2, grid is same with attr CALIBRATION_DATA_TERMS
		objHolder.remove(IR_CALIBRATION_DATA_DFS2);
		objHolder.add(IR_CALIBRATION_DATA_DFS2, new AQLDataDoubles(dfs_mod));
	*/
}

/*!
	@brief calc basis rates and add DF2 data as modified DF

*/
void
CurveCalibrationData::setBasisRates_old(void)
{
	std::vector<const AQLObject *> data_basis;
	// get market data
	const AQLDataMultiReference &mr = getMarketData();
	for (unsigned int i = 0; i < mr.getSize(); i++)
	{
		AQLString term = dynamic_cast<const AQLDataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		term.toUpper();
		if (term == BASIS)
		{
			data_basis.push_back(&mr.get(i).get());
		}
	}
	// data exist check
	if (data_basis.empty())
	{
		return;
		//throw AQLCoreInvalidData("basis data is not set.", __FILE__, __LINE__);
	}
	// sort
	sort(data_basis.begin(), data_basis.end(), InstrumentComp());

	// get spotdate
	const AQLDate &spotdate = dynamic_cast<const AQLDataDate &> ((data_basis[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	// get base yield curve and asofdate
	const AQLMathYieldCurve &baseYieldCurve = dynamic_cast<const AQLMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
	// asof and spot check
	if (asof > spotdate)
	{
		throw AQLCoreInvalidData("#Error: asofdate > spotdate, cannnot calc basis DF", __FILE__, __LINE__);
	}

	// get first element val
	const AQLPriceDataDayCount &dcbs = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNTBASE, ISNOTNULL)).get());
	const AQLPriceDataDayCount &dc = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const AQLPriceDataCalendar &calbs = dynamic_cast<const AQLPriceDataCalendar &>((data_basis[0]->getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule &sldbs = dynamic_cast<const AQLPriceDataSlidingRule &>((data_basis[0]->getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
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
	if (freqbs == LUNAR)
	{
		roll_conv_bs = ROLLCONV_LUNAR;
	}
	else if (eombs)
	{
		roll_conv_bs = ROLLCONV_EOM;
	}
	else
	{
		roll_conv_bs = ROLLCONV_NORMAL;
	}

	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	for (unsigned int i = 0; i < data_basis.size(); i++)
	{
		const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data_basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		double rate = dynamic_cast<const AQLDataDouble &>((data_basis[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLDate tmpDate = etrading::LADateHelpers::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
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
	etrading::LADateHelpers::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	unsigned int mUnit = 0;
	if (freqbs == ANNUAL)
	{
		mUnit = 12;
	}
	else if (freqbs == SEMI_ANNUAL)
	{
		mUnit = 6;
	}
	else if (freqbs == QUARTERLY)
	{
		mUnit = 3;
	}
	else if (freqbs == MONTHLY || freqbs == LUNAR)
	{
		mUnit = 1;
	}
	else
	{
		AQLString msg = "#Error: frequency is wrong";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		AQLString msg = "#Error: frequency is wrong";
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

		AQLDate ldate = etrading::LADateHelpers::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
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
		ldate = etrading::LADateHelpers::getDate(spotdate, strTerm_, sldbs, &calbs, true, &roll_conv_bs);
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
			throw AQLCoreInvalidData("#Error: Df is below zero.", __FILE__, __LINE__);
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
		throw AQLCoreInvalidData("#Error: Df from asofdate to spotdate is below zero.", __FILE__, __LINE__);
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
			throw AQLCoreInvalidData("#Error: Df is below zero.", __FILE__, __LINE__);
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
CurveCalibrationData::setBasisRates2(const AQLString& basisCurveID)
{
	std::vector<const AQLObject *> data_basis;
	// get market data
	const AQLDataMultiReference &mr = getMarketData();
	for (unsigned int i = 0; i < mr.getSize(); i++)
	{
		AQLString term = dynamic_cast<const AQLDataString &>((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		term.toUpper();
		if (term == BASIS)
		{
			data_basis.push_back(&mr.get(i).get());
		}
	}
	// data exist check
	if (data_basis.empty())
	{
		throw AQLCoreInvalidData("#Error: basis data is not set.", __FILE__, __LINE__);
	}
	// sort
	sort(data_basis.begin(), data_basis.end(), InstrumentComp());

	// get spotdate
	const AQLDate &spotdate = dynamic_cast<const AQLDataDate &> ((data_basis[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	// get base yield curve and asofdate
	const AQLMathYieldCurve &baseYieldCurve = dynamic_cast<const AQLMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
	// asof and spot check
	if (asof > spotdate)
	{
		throw AQLCoreInvalidData("#Error: asofdate > spotdate, cannnot calc basis DF", __FILE__, __LINE__);
	}

	// get first element val
	const AQLPriceDataDayCount &dcbs = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNTBASE, ISNOTNULL)).get());
	const AQLPriceDataDayCount &dc = dynamic_cast<const AQLPriceDataDayCount &>((data_basis[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const AQLPriceDataCalendar &calbs = dynamic_cast<const AQLPriceDataCalendar &>((data_basis[0]->getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule &sldbs = dynamic_cast<const AQLPriceDataSlidingRule &>((data_basis[0]->getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
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
	if (freqbs == LUNAR)
	{
		roll_conv_bs = ROLLCONV_LUNAR;
	}
	else if (eombs)
	{
		roll_conv_bs = ROLLCONV_EOM;
	}
	else
	{
		roll_conv_bs = ROLLCONV_NORMAL;
	}

	DoubleArray b_t_grid(1, 0.0);
	DoubleArray b_termstruct_grid(1, 0.0);
	for (unsigned int i = 0; i < data_basis.size(); i++)
	{
		const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data_basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		double rate = dynamic_cast<const AQLDataDouble &>((data_basis[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLDate tmpDate = etrading::LADateHelpers::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
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
	etrading::LADateHelpers::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	unsigned int mUnit = 0;
	if (freqbs == ANNUAL)
	{
		mUnit = 12;
	}
	else if (freqbs == SEMI_ANNUAL)
	{
		mUnit = 6;
	}
	else if (freqbs == QUARTERLY)
	{
		mUnit = 3;
	}
	else if (freqbs == MONTHLY || freqbs == LUNAR)
	{
		mUnit = 1;
	}
	else
	{
		AQLString msg = "#Error: frequency is wrong";
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

		AQLDate ldate = etrading::LADateHelpers::getDate(spotdate, strTerm, sldbs, &calbs, true, &roll_conv_bs);
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
			ldate = etrading::LADateHelpers::getDate(spotdate, strTerm_, sldbs, &calbs, true, &roll_conv_bs);

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
	objHolder = objPool.getObject(basisCurveID, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add("Name", new AQLDataString()).convertFromString(basisCurveID);
		e->add("AsOfDate", new AQLDataDate(asof));
		e->add("Terms", new AQLDataDoubles(terms));
		e->add("DiscountFactors", new AQLDataDoubles(dfs_mod));
		objPool.set(basisCurveID, e);
	}
	else if (objHolder.isDefined())
	{
		AQLDataHolder* dh;
		dh = &objHolder.getData("AsOfDate", ISDEFINED);
		AQLDataDate& date = dynamic_cast<AQLDataDate &>(dh->get());
		date.set(asof);

		dh = &objHolder.getData("Terms", ISDEFINED);
		AQLDataDoubles& terms_att = dynamic_cast<AQLDataDoubles&>(dh->get());
		terms_att.set(terms);
		dh = &objHolder.getData("DiscountFactors", ISDEFINED);
		AQLDataDoubles& dfs = dynamic_cast<AQLDataDoubles&>(dh->get());
		dfs.set(dfs_mod);
	}
}

/*!
	@brief add basis rates

*/
void
CurveCalibrationData::addBasisRates(void)
{
	AQLObjectHolder objHolder = getYieldData().get();
	const AQLObject& YieldData = objHolder.get();
	const DoubleArray& dfs = dynamic_cast<const AQLDataDoubles&> ((YieldData.getData(IR_CALIBRATION_DATA_DFS + mCurveSuffix, ISNOTNULL)).get()).get();
	DoubleArray dfs_mod(dfs.size());

	const DoubleMatrix& d_matrix = getBasisRates().get();

	for (unsigned int i = 0; i < d_matrix.size(); i++)
	{
		for (unsigned int j = 0; j < d_matrix[i].size(); j++)
			dfs_mod[j] = AQLMath::exp(-d_matrix[i][j]/* * terms[j]*/) * dfs[j];
	}
	objHolder.remove(IR_CALIBRATION_DATA_DFS + mCurveSuffix);
	objHolder.add(IR_CALIBRATION_DATA_DFS + mCurveSuffix, new AQLDataDoubles(dfs_mod));
}

/*!
	@brief set arb free flag and curve name
*/
void
CurveCalibrationData::setArbFreeCurveName(const AQLStringVector& curveNames_6ML,
	const AQLStringVector& curveNames_DF,
	const AQLStringVector& curveNames_3ML)
{
	for (size_t i = 0; i < curveNames_6ML.size(); i++)
	{
		if (curveNames_6ML[i] != STD)
		{
			mArbFreeCurveGenMap[curveNames_6ML[i]] = true;
			mAssignedCurveMktMap[curveNames_6ML[i]] = AF6ML;
		}
	}
	for (size_t i = 0; i < curveNames_DF.size(); i++)
	{
		if (curveNames_DF[i] != STD)
		{
			mArbFreeCurveGenMap[curveNames_DF[i]] = true;
			mAssignedCurveMktMap[curveNames_DF[i]] = AFDF;
		}
	}
	for (size_t i = 0; i < curveNames_3ML.size(); i++)
	{
		if (curveNames_3ML[i] != STD)
		{
			mArbFreeCurveGenMap[curveNames_3ML[i]] = true;
			mAssignedCurveMktMap[curveNames_3ML[i]] = AF3ML;
		}
	}
}

void
CurveCalibrationData::setdNPVdm(const AQLString &curveType)
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	//check whethor Matrix has been made or not
	AQLDataHolder *dh = 0;

	dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix));
	if (dh->isDefined() && !dh->isNull())
	{
		return;
	}

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
	{
		suffix_mkt = "_" + mktName;
	}
	const AQLDataMultiReference* mr_;
	AQLStringVector tmpMktNames = mktName.toToken('_');
	if (tmpMktNames.size() == 2)
	{
		AQLObjectHolder &eh_fy = getForeignYieldData().get();
		if (!eh_fy.isDefined())
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
	std::vector<AQLObject*> dataall;
	IntArray omitvec;
	for (unsigned int i = 0; i < mr_->getSize(); i++)
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
		else if (tmpType == PAR)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == ZERO)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == O_N || tmpType == T_N)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == FUTURE)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == BOJ || tmpType == FEDFUNDRATE)
		{
			dataall.push_back(&mr_->get(i).get());
		}
		else if (tmpType == FRA6M || tmpType == FRA3M)
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
	AQLPriceDataDayCount dc_act365(ACT_365);

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
	bool isdiscount, isagtspd;
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
			gridstrvec[i] = dynamic_cast<const AQLDataString &>(dh->get());
		}
		else
			gridstrvec[i] = dynamic_cast<const AQLDataString &>(dh->get());


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
			{
				throw AQLCoreInvalidData("dNPVdzero Error", __FILE__, __LINE__);
			}

			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			double mrate = dynamic_cast<const AQLDataDouble &>(dh->get());

			double calcterm = dcCalc.getTerm(dateMat[i][0], dateMat[i][1]);
			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dfterm2 = dc_act365.getTerm(asOfDate, dateMat[i][1]);

			double df1 = pInter.value(dfterm1);
			double df2 = pInter.value(dfterm2);

			dNPVdm[i] = calcterm * df2;
			dNPVdzero[i][0] = -dfterm1 * df1;
			dNPVdzero[i][1] = dfterm2 * (1.0 + calcterm * mrate) * df2;
			mratevec[i] = mrate;
		}//swap case 
		else if (typevec[i] == PAR)
		{
			if (dateMat[i].size() < 2)
			{
				throw AQLCoreInvalidData("dNPVdzero Error", __FILE__, __LINE__);
			}

			double dfterm1 = dc_act365.getTerm(asOfDate, dateMat[i][0]);
			double dftermLast = dc_act365.getTerm(asOfDate, dateMat[i].back());

			double df1 = pInter.value(dfterm1);
			double dfLast = pInter.value(dftermLast);

			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			double mrate = dynamic_cast<const AQLDataDouble &>(dh->get());
			if (mrate == 0.0)
			{
				//throw AQLCoreInvalidData("Market rate 0", __FILE__,__LINE__);
				mrate = EPS;
				dNPVdm[i] = EPS;
			}
			else
			{
				dNPVdm[i] = -(dfLast - df1) / mrate;
			}

			dNPVdzero[i][0] = -dfterm1 * df1;

			double dfterm = 0.0;
			double calcterm = 0.0;
			double dfj = 0.0;
			for (unsigned int j = 1; j < dNPVdzero[i].size(); j++)
			{
				dfterm = dc_act365.getTerm(asOfDate, dateMat[i][j]);
				calcterm = dcCalc.getTerm(dateMat[i][j - 1], dateMat[i][j]);
				dfj = pInter.value(dfterm);
				dNPVdzero[i][j] = dfterm * calcterm * mrate * dfj;
			}
			dNPVdzero[i].back() += dfterm * dfj;
			mratevec[i] = mrate;

		}//basis case
		else if (typevec[i] == BASIS)
		{
			// in case of basis there are 3cases, discountcase, forecastAndagainstspreadcase,
			//and forecastAndnotspreadcase
			if (dateMat[i].size() < 2)
			{
				throw AQLCoreInvalidData("dNPVdzero Error", __FILE__, __LINE__);
			}

			if (0 == i)
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
				double dfj = 0.0;
				double dfj_1 = pInterSTD.value(dfterm1);
				double baisdfj = 0.0;
				double spreadj = 0.0;

				dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
				double brate = dynamic_cast<const AQLDataDouble &>(dh->get());
				dNPVdzero[i][0] = -dfterm1 * basisdf1;
				for (unsigned int j = 1; j < dNPVdzero[i].size(); j++)
				{
					dfterm = dc_act365.getTerm(asOfDate, dateMat[i][j]);
					calcterm = dcCalc.getTerm(dateMat[i][j - 1], dateMat[i][j]);
					dfj = pInterSTD.value(dfterm);
					baisdfj = pInter.value(dfterm);
					spreadj = baisdfj / dfj;

					dNPVdzero[i][j] = dfterm * spreadj * (dfj_1 + (brate * calcterm - 1.0) * dfj);
					dNPVdm[i] += calcterm * baisdfj;

					dfj_1 = dfj;
				}
				dNPVdzero[i].back() += dfterm * pInter.value(dfterm);
				mratevec[i] = brate;
			}
			else //3M-6MBasis
			{
				double sgn = (isagtspd) ? 1.0 : -1.0;

				double dfterm = dc_act365.getTerm(asOfDate, dateMat[i][0]);
				double calcterm = dcCalc.getTerm(dateMat[i][0], dateMat[i][1]);
				double baisdfj = pInter.value(dfterm);
				double divspreadj = pInterSTD.value(dfterm) / baisdfj;

				double bfbasisdfj = 0.0;
				double bfdivspreadj = 0.0;

				double afdfterm = dc_act365.getTerm(asOfDate, dateMat[i][1]);
				double afdivspreadj = pInterSTD.value(afdfterm) / pInter.value(afdfterm);
				dNPVdzero[i][0] = dfterm * baisdfj * divspreadj;
				for (unsigned int j = 1; j < dNPVdzero[i].size() - 1; j++)
				{
					double bfbasisdfj = baisdfj;
					double bfdivspreadj = divspreadj;

					dfterm = dc_act365.getTerm(asOfDate, dateMat[i][j]);
					calcterm = dcCalc.getTerm(dateMat[i][j - 1], dateMat[i][j]);
					baisdfj = pInter.value(dfterm);
					divspreadj = pInterSTD.value(dfterm) / baisdfj;

					afdfterm = dc_act365.getTerm(asOfDate, dateMat[i][j + 1]);
					afdivspreadj = pInterSTD.value(afdfterm) / pInter.value(afdfterm);
					dNPVdzero[i][j] = dfterm * bfbasisdfj * divspreadj - dfterm * baisdfj * afdivspreadj;
					dNPVdzero[i][j] *= sgn;

					dNPVdm[i] += calcterm * baisdfj;

				}
				dNPVdzero[i].back() += sgn * afdfterm * baisdfj * afdivspreadj;
				dNPVdm[i] += calcterm * baisdfj;
				dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
				double brate = dynamic_cast<const AQLDataDouble &>(dh->get());
				mratevec[i] = brate;
			}
		}//future case
		else if (typevec[i] == FUTURE)
		{
			if (dateMat[i].size() != 2)
			{
				throw AQLCoreInvalidData("dNPVdzero Error", __FILE__, __LINE__);
			}

			double mrate = 0.0;
			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				mrate = dynamic_cast<const AQLDataDouble &>(dh->get());
			}
			else
			{
				dh = &(dataall[i]->getData(PRICING_DATA_PRICE, ISNOTNULL));
				mrate = 1.0 - 0.01 *  dynamic_cast<const AQLDataDouble &>(dh->get());
			}

			double calcterm = dcCalc.getTerm(dateMat[i][0], dateMat[i][1]);
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
			throw AQLCoreInvalidData("Not Support Now", __FILE__, __LINE__);
		}
		else if (typevec[i] == FRA3M || typevec[i] == FRA6M)
		{
			if (dateMat[i].size() != 2)
			{
				throw AQLCoreInvalidData("dNPVdzero Error", __FILE__, __LINE__);
			}

			double mrate = 0.0;
			dh = &(dataall[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL));
			mrate = dynamic_cast<const AQLDataDouble &>(dh->get());

			double calcterm = dcCalc.getTerm(dateMat[i][0], dateMat[i][1]);
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
	DoubleMatrix decompMat(sizeAll, DoubleVector(sizeAll, 0.0));
	for (unsigned int i = 0; i < sizeAll; i++)
	{
		//decompose dNPVdzero
		for (unsigned int j = 0; j < dateMat[i].size(); j++)
		{
			AQLDate targetdate = dateMat[i][j];
			AQLAlgorithm::locate<DateVector, AQLDate>(gridvec, targetdate, gridvec.size(), gridpos);

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
				double diffD = static_cast<double>(gridvec[gridpos - 1].intervalDays(gridvec[gridpos]));
				if (diffD == 0.0)
				{
					throw AQLCoreInvalidData("DateMatrix Error", __FILE__, __LINE__);
				}

				double ratio1 = targetdate.intervalDays(gridvec[gridpos]) / diffD;

				decompMat[i][gridpos - 1] += dNPVdzero[i][j] * ratio1;
				decompMat[i][gridpos] += dNPVdzero[i][j] * (1.0 - ratio1);
			}
		}
	}

	//dMdZ
	for (unsigned int i = 0; i < sizeAll; i++)
	{
		if (dNPVdm[i] == 0.0)
		{
			throw AQLCoreInvalidData("dNPVdm 0 Error", __FILE__, __LINE__);
		}
		DoubleVector div_vec(sizeAll, dNPVdm[i]);
		transform(decompMat[i].begin(), decompMat[i].end(), div_vec.begin(), decompMat[i].begin(), std::divides<double>());
	}

	//inverse
	AQLMatrix matobj(decompMat);
	const AQLMatrix& invmat = matobj.inverseMatrix();
	const AQLMatrix& transmat = invmat.transpose();

	DoubleMatrix ret(sizeAll, DoubleVector(sizeAll, 0.0));
	for (unsigned int i = 0; i < sizeAll; i++)
		for (unsigned int j = 0; j < sizeAll; j++)
			ret[i][j] = transmat.getValue(i, j);

	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix, new AQLDataDoubleMatrix(ret));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix, new AQLDataDoubles(gridtermvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix, new AQLDataInts(omitvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES + suffix, new AQLDataStrings(gridstrvec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS + suffix, new AQLDataDoubles(mratevec));
	getYieldData().get().add(IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES + suffix, new AQLDataStrings(typevec));

	return;
}


const AQLDataDoubleMatrix&
CurveCalibrationData::getConversionMatrix(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIX + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataDoubleMatrix &>(dh->get());
}

const AQLDataDoubles&
CurveCalibrationData::getConversionMatrixTerm(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataDoubles &>(dh->get());
}

const AQLDataDoubles&
CurveCalibrationData::getConversionMarketRates(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataDoubles &>(dh->get());
}


const AQLDataInts&
CurveCalibrationData::getConversionOmitGrids(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataInts &>(dh->get());
}

const AQLDataStrings&
CurveCalibrationData::getConversionMatrixTermTypes(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataStrings &>(dh->get());
}

const AQLDataStrings&
CurveCalibrationData::getConversionMarketTypes(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES + suffix, ISNOTNULL));
	return dynamic_cast<const AQLDataStrings &>(dh->get());
}


bool
CurveCalibrationData::isOmitGridsExist(const AQLString &curveType) const
{
	AQLString suffix = "";
	if (curveType != STD)
	{
		suffix = "_" + curveType;
	}

	const AQLDataHolder* dh = &(getYieldData().get().getData(IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS + suffix));
	return (dh->isDefined() && !dh->isNull());

}
void
CurveCalibrationData::setCurveDependencyMap(void)
{
	//bcurve = STD + alpha then map[bcurve][Base][+alpha]
	std::map<AQLString, bool>::iterator itbgen = mBCurveGenMap.begin();
	for (itbgen = mBCurveGenMap.begin(); itbgen != mBCurveGenMap.end(); ++itbgen)
	{
		AQLString curveType = itbgen->first;

		AQLString suffix = "";
		if (curveType != STD)
		{
			suffix = "_" + curveType;
		}

		//	// get market data
		AQLString suffix_mkt = "";
		if (curveType != STD)
		{
			suffix_mkt = "_" + mAssignedCurveMktMap[curveType];
		}

		const AQLDataMultiReference* mr_;
		AQLStringVector tmpMktNames = mAssignedCurveMktMap[curveType].toToken('_');
		if (tmpMktNames.size() == 2)
		{
			AQLObjectHolder &eh_fy = getForeignYieldData().get();
			if (!eh_fy.isDefined())
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
		unsigned int bpos = mr_->getSize();
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
		{
			throw AQLCoreInvalidData("Not found DataType = BASIS", __FILE__, __LINE__);
		}

		AQLObject& ebasis = mr_->get(bpos).get();
		bool isdiscount = dynamic_cast<const AQLDataBool &>(ebasis.getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
		bool isagtspd = dynamic_cast<const AQLDataBool &>(ebasis.getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();
		double spdval = 0.0;
		AQLString targetCurve;
		//forecasttype
		if (!isdiscount)
		{
			//now under construnction
			spdval = 1.0;
			targetCurve = dynamic_cast<const AQLDataString &>(ebasis.getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
		}
		else //discount
		{
			//now under construnction
			spdval = 1.0;
			if (isagtspd)
			{
				targetCurve = dynamic_cast<const AQLDataString &>(ebasis.getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
			}
			else
			{
				targetCurve = dynamic_cast<const AQLDataString &>(ebasis.getData(IR_CALIBRATION_DATA_FORECAST, ISNOTNULL).get()).get();
			}
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
CurveCalibrationData::getCurveDependeny(const AQLString &curveType) const
{
	AQLString tmp(curveType);
	if (mBfCurveType == tmp)
	{
		return mBfDpnMap;
	}

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
		mBfDpnMap.insert(std::make_pair(curveType, 1.0));
		return mBfDpnMap;
	}
	//std, 6mlibor each shift case

	if (mBCurveGenMap.find(curveType) == mBCurveGenMap.end() ||
		mDpnCurveMap.find(curveType) == mDpnCurveMap.end())
	{
		throw AQLCoreInvalidData("Curve Generate Map Error", __FILE__, __LINE__);
	}

	AQLString nameTmp = curveType;
	bool iscomplete = false;
	while (!iscomplete)
	{

		itdpn = mDpnCurveMap.find(nameTmp);
		if (itdpn->second.size() != 1)
		{
			throw AQLCoreInvalidData("Not Support Now", __FILE__, __LINE__);
		}

		std::map<AQLString, double>::const_iterator itTmp = itdpn->second.begin();
		if (itTmp->first == nameTmp)
		{
			throw AQLCoreInvalidData("Loop Error", __FILE__, __LINE__);
		}

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
			mBfDpnMap.insert(std::make_pair(nameTmp, itTmp->second));
			//we register gen curve here
			mBfDpnMap.insert(std::make_pair(itTmp->first, itTmp->second));
			iscomplete = true;
		}
		//std, 6mlibor each shift case
		else
		{
			mBfDpnMap.insert(std::make_pair(nameTmp, itTmp->second));
			nameTmp = itTmp->first;
		}
	}

	return mBfDpnMap;
}

void
CurveCalibrationData::changeZeroRiskIntoMarketRisk(AQLString curveType, const DoubleVector& termZeroVals, const DoubleVector& riskZeroVals,
	AQLStringVector& termMarketGrids, DoubleVector& riskMarketVals) const
{

	termMarketGrids = getConversionMatrixTermTypes(curveType).get();
	const DoubleMatrix& mat = getConversionMatrix(curveType).get();
	const DoubleVector& termvec = getConversionMatrixTerm(curveType).get();
	DoubleVector dNPVdzero(termvec.size(), 0.0);
	DoubleVector dNPVdm(termvec.size(), 0.0);

	riskMarketVals.resize(termvec.size());

	unsigned int gridpos = 0;
	//calc dNPVdm and dNPVdm if exist
	for (unsigned int i = 0; i < termZeroVals.size(); i++)
	{
		double term = termZeroVals[i];
		double riskval = riskZeroVals[i];
		AQLAlgorithm::locate<DoubleVector, double>(termvec, term, termvec.size(), gridpos);
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
			double diffD = termvec[gridpos] - termvec[gridpos - 1];
			if (diffD == 0.0)
			{
				throw AQLCoreInvalidData("TermVec Error", __FILE__, __LINE__);
			}

			double ratio1 = (termvec[gridpos] - term) / diffD;
			dNPVdzero[gridpos - 1] += riskval * ratio1;
			dNPVdzero[gridpos] += riskval * (1.0 - ratio1);
		}
	}

	AQLMatrix convertMat(mat);
	AQLMatrix dNPVdzeroMat(dNPVdzero);
	const AQLMatrix& multimat = convertMat * dNPVdzeroMat;
	for (unsigned int l = 0; l < riskMarketVals.size(); l++)
		riskMarketVals[l] = multimat.getValue(l, 0);

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
CurveCalibrationData::getIsSwapTenorAdjust(void) const
{
	return dynamic_cast<const AQLDataBool&>(mpIsSwapTenorAdjust->get());
}
/*!
	@brief get IsSwapTenorCahnge
	@return IsSwapTenorCahnge
*/
AQLDataBool&
CurveCalibrationData::getIsSwapTenorAdjust(void)
{
	return dynamic_cast<AQLDataBool&>(mpIsSwapTenorAdjust->get());
}

/*!
	@brief

	@return
*/
const AQLDataReference&
CurveCalibrationData::getForeignYieldData() const
{
	return dynamic_cast<const AQLDataReference&>(mpForeignYieldData->get());
}
/*!
	@brief

	@return
*/
AQLDataReference&
CurveCalibrationData::getForeignYieldData()
{
	return dynamic_cast<AQLDataReference&>(mpForeignYieldData->get());
}

/*!
	@brief

	@return
*/
const AQLDataReference&
CurveCalibrationData::getColYieldData() const
{
	return dynamic_cast<const AQLDataReference&>(mpColYieldData->get());
}

/*!
	@brief

	@return
*/
AQLDataReference&
CurveCalibrationData::getColYieldData()
{
	return dynamic_cast<AQLDataReference&>(mpColYieldData->get());
}

/*!
	@brief calc floater PV
	@param[in] curveName

*/
void
CurveCalibrationData::setFloater(const AQLString& curveName)
{
	AQLString tmpCurveName = curveName;
	AQLObjectHolder &objHolder = getYieldData().get();
	AQLDataHolder *dh;

	// data check
	bool isCheckCurves = false;
	dh = &getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	bool isCurveAttrExist = checkCurveAttr(curveName);
	if (isCheckCurves && isCurveAttrExist)
	{
		return;
	}

	AQLPriceDataDayCount dc_act365(ACT_365);
	AQLObject &yieldData = objHolder.get();

	AQLString discountName =
		dynamic_cast<AQLDataString&> (getData(IR_CALIBRATION_DATA_DISCOUNT + AQLString("_") + tmpCurveName.toLower(), ISNOTNULL).get());
	AQLString forecastName =
		dynamic_cast<AQLDataString&> (getData(IR_CALIBRATION_DATA_FORECAST + AQLString("_") + tmpCurveName.toLower(), ISNOTNULL).get());

	AQLString suffix_d = "";
	if (discountName != STD)
	{
		suffix_d = AQLString("_") + discountName;
	}

	AQLString suffix_f = "";
	if (forecastName != STD)
	{
		suffix_f = AQLString("_") + forecastName;
	}

	// set forecast curve
	AQLPriceDataInterpolation forwardRates = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	const DoubleArray &terms_f = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix_f, ISNOTNULL)).get()).get();
	const DoubleArray &dfs_f = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix_f, ISNOTNULL)).get()).get();
	forwardRates.set(terms_f, dfs_f);

	// set discount curve
	AQLPriceDataInterpolation discountFactor = dynamic_cast<AQLPriceDataInterpolation &>(getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
	const DoubleArray &terms_d = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(CALIBRATION_DATA_TERMS + suffix_d, ISNOTNULL)).get()).get();
	const DoubleArray &dfs_d = dynamic_cast<const AQLDataDoubles&> ((yieldData.getData(IR_CALIBRATION_DATA_DFS + suffix_d, ISNOTNULL)).get()).get();
	discountFactor.set(terms_d, dfs_d);

	const DoubleArray &terms_std = dynamic_cast<const AQLDataDoubles &>((yieldData.getData(CALIBRATION_DATA_TERMS, ISNOTNULL)).get()).get();
	// maxterm for calc
	const double tmax = terms_std.back();
	// get base yield curve and asofdate
	const AQLMathYieldCurve &baseYieldCurve = dynamic_cast<const AQLMathYieldCurve &>(getBaseYieldCurve().get().get());
	const AQLDate &asof = dynamic_cast<const AQLDataDate &> (baseYieldCurve.getYieldData().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());

	DoubleArray dfs; dfs.push_back(1.);
	DoubleArray terms; terms.push_back(0.);

	const double eps_term = 1E-10;

	dh = &getData(IR_CALIBRATION_DATA_BASISDATA + AQLString("_") + tmpCurveName.toLower(), NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString basisMkt = dynamic_cast<AQLDataString &>(dh->get()).get();
		AQLString suffix = "_" + basisMkt;
		// get market data
		const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&>
			(getData(CALIBRATION_DATA_MARKETDATA + suffix, ISNOTNULL).get());
		std::vector<AQLObject*> data;
		for (unsigned int i = 0; i < mr.getSize(); i++)
		{
			// check use grid
			const AQLDataHolder *dh = &mr.get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get())
			{
				continue;
			}

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
			AQLString msg = "CurveName = " + basisMkt + ", basis data is not set.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// sort
		sort(data.begin(), data.end(), InstrumentComp());

		// market convencion info
		const bool isTimeInter = dynamic_cast<const AQLDataBool &>((data[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS, ISNOTNULL)).get()).get();

		//eom roll
		bool isEomRoll = false;
		dh = &data[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			isEomRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// cashlet
		const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &c_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
		AQLString c_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
		const int c_lag = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());

		// index
		const AQLPriceDataDayCount &i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &i_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
		const int i_lag = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
		AQLString i_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
		AQLString i_accessary = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

		// isfwdinterpolation
		bool useForwardInterpolation = false;
		AQLPriceDataInterpolation *forwardInterpolation = 0;
		dh = &(data[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			useForwardInterpolation = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			forwardInterpolation = &dynamic_cast<AQLPriceDataInterpolation &>(data[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		}

		if (useForwardInterpolation)
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
			forwardInterpolation->set(terms, fwds);
		}

		// get max term
		const AQLString &termMax = dynamic_cast<const AQLDataString &>((data.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

		// calc term (apply to month)
		int y, m, d, w;
		etrading::LADateHelpers::termStrtoYMDW(termMax, y, m, d, w);
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
		else if (isEomRoll) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;

		// get interpolation method
		AQLPriceDataInterpolation &b_inter = getInterpolation_bs();
		DoubleArray b_t_grid(1, 0.0);
		DoubleArray b_termstruct_grid(1, 0.0);
		AQLStringVector basisTerms;

		for (unsigned int i = 0; i < data.size(); i++)
		{
			const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			basisTerms.push_back(strTerm);

			double rate = dynamic_cast<const AQLDataDouble &>((data[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			double term = 0.0;
			AQLDate tmpDate = etrading::LADateHelpers::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
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

		const double spotTerm = dc_act365.getTerm(asof, c_spotdate);
		const double d_spotdf = discountFactor.value(spotTerm);
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
			AQLDate fdate = etrading::LADateHelpers::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			AQLDate ldate = etrading::LADateHelpers::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true, &roll_conv);
			double dfTerm = dc_act365.getTerm(asof, ldate);
			double dfTerm_last = dc_act365.getTerm(asof, fdate);
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
				ret += i_termVec[k - 1] * forwardRates.value(i_gridVec[k]);
			}*/
			const double delta = c_dc.getTerm(fdate, ldate);
			double rate = 0.;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(dfTerm_last);
			}
			else
			{
				double ret = delta * forwardRates.value(dfTerm);
				//const double rate = (forwardRates.value(i_gridVec[0]) -  forwardRates.value(i_gridVec.back())) / ret;
				rate = (forwardRates.value(dfTerm_last) - forwardRates.value(dfTerm)) / ret;
			}
			const double df = discountFactor.value(dfTerm);

			annuity += delta * df;
			rateTauDF += rate * delta * df;
			double floaterPV = rateTauDF + annuity * spread + 1.0 * df;
			dfs.push_back(floaterPV);
			terms.push_back(dfTerm);
		}

		for (unsigned int i = 0; i < data.size(); i++)
		{
			DateVector ret;

			const AQLString &strTerm = dynamic_cast<const AQLDataString &>((data[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLDate matudate = etrading::LADateHelpers::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			ret.push_back(c_spotdate);

			//find dvzeroDates
			unsigned int pos = 0;
			if (!AQLAlgorithm::find<DateVector, AQLDate>(dvzeroDates, matudate, 0, dvzeroDates.size() - 1, pos))
			{
				//this means basis grid e.x. 100y is longer than DF1 max grid, which ofcourse can not be calculated.
				if (matudate > dvzeroDates.back())
				{
					continue;
				}
				else
				{
					throw AQLCoreInvalidData("BasisGrid Search Error", __FILE__, __LINE__);
				}
			}

			ret.insert(ret.end(), dvzeroDates.begin(), dvzeroDates.begin() + pos + 1);
			if (!data[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
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
		objHolder.add(IR_CALIBRATION_DATA_ISEOMROLL + AQLString("_") + curveName, new AQLDataBool(isEomRoll));

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
		std::vector<AQLObject*> data;
		for (unsigned int i = 0; i < mr.getSize(); i++)
		{
			// check use grid
			const AQLDataHolder *dh = &mr.get(i).get().getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
			if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get())
			{
				continue;
			}

			AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(i).get().getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			datatype_str.toUpper();
			if (datatype_str == PAR)
			{
				data.push_back(&mr.get(i).get());
			}
		}

		// data exist check
		if (data.empty())
		{
			AQLString msg = "swap market is not set.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// sort
		sort(data.begin(), data.end(), InstrumentComp());

		//eom roll
		bool isEomRoll = false;
		dh = &data[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			isEomRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// cashlet
		const AQLPriceDataDayCount &c_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL).get());
		const AQLPriceDataCalendar &c_cal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &c_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const AQLDate &c_spotdate = dynamic_cast<const AQLDataDate &> (data[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
		AQLString c_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL).get()).get();
		const int c_lag = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get());

		// index
		const AQLPriceDataDayCount &i_dc = dynamic_cast<const AQLPriceDataDayCount &>(data[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_fixcal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataCalendar &i_paycal = dynamic_cast<const AQLPriceDataCalendar &>(data[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
		const AQLPriceDataSlidingRule &i_sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		const int i_lag = dynamic_cast<const AQLDataInt &>(data[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get());
		AQLString i_freq = dynamic_cast<const AQLDataString &>(data[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL).get()).get();

		// isfwdinterpolation
		bool useForwardInterpolation = false;
		AQLPriceDataInterpolation *forwardInterpolation = 0;
		dh = &(data[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			useForwardInterpolation = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			forwardInterpolation = &dynamic_cast<AQLPriceDataInterpolation &>(data[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
		}
		if (useForwardInterpolation)
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
			forwardInterpolation->set(terms, fwds);
		}
		// get max term
		const AQLString &termMax = dynamic_cast<const AQLDataString &>((data.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		// calc term (apply to month)
		int y, m, d, w;
		etrading::LADateHelpers::termStrtoYMDW(termMax, y, m, d, w);
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
		AQLString i_accessary = AQLString(static_cast<int> (mUnit)) + AQLString("M");
		// set roll convention
		AQLString roll_conv("");
		if (c_freq == LUNAR) roll_conv = ROLLCONV_LUNAR;
		else if (isEomRoll) roll_conv = ROLLCONV_EOM;
		else roll_conv = ROLLCONV_NORMAL;

		const unsigned int max = m / mUnit + 1;

		const double spotTerm = dc_act365.getTerm(asof, c_spotdate);
		const double d_spotdf = discountFactor.value(spotTerm);
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
			AQLDate fdate = etrading::LADateHelpers::getDate(c_spotdate, strTerm, c_sld, &c_cal, true, &roll_conv);
			AQLDate ldate = etrading::LADateHelpers::getDate(c_spotdate, dfstrTerm, c_sld, &c_cal, true, &roll_conv);
			double dfTerm = dc_act365.getTerm(asof, ldate);
			double dfTerm_last = dc_act365.getTerm(asof, fdate);
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
				ret += i_termVec[k - 1] * forwardRates.value(i_gridVec[k]);
			}*/
			const double delta = c_dc.getTerm(fdate, ldate);
			double rate = 0.;
			if (useForwardInterpolation)
			{
				rate = forwardInterpolation->value(dfTerm_last);
			}
			else
			{
				double ret = delta * forwardRates.value(dfTerm);
				//const double rate = (forwardRates.value(i_gridVec[0]) -  forwardRates.value(i_gridVec.back())) / ret;
				rate = (forwardRates.value(dfTerm_last) - forwardRates.value(dfTerm)) / ret;
			}
			const double df = discountFactor.value(dfTerm);
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
CurveCalibrationData::checkCurveAttr(const AQLString& mktName) const
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
			if (curveName != STD)
			{
				suffix = "_" + curveName;
			}

			dh = &(objHolder.getData(CALIBRATION_DATA_TERMS + suffix, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				ret = false;
			}

			dh = &(objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				ret = false;
			}

			dh = &(objHolder.getData(IR_CALIBRATION_DATA_FORWARDRATES + suffix, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				ret = false;
			}
		}
	}

	return ret;
}

/*!
	@brief remove all curve data
	@param[out] yieldData
*/
void
CurveCalibrationData::removeAllCuveData(AQLObject &yieldData) const
{
	for (std::map<AQLString, AQLString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	{
		if (mNonRemovableMarket.end() != mNonRemovableMarket.find(it->second))
		{
			continue;
		}
		const AQLString& curveName = it->first;
		AQLString suffix = "";
		if (curveName != STD)
		{
			suffix = "_" + curveName;
		}
		yieldData.remove(CALIBRATION_DATA_TERMS + suffix);
		yieldData.remove(IR_CALIBRATION_DATA_DFS + suffix);
		yieldData.remove(IR_CALIBRATION_DATA_FORWARDRATES + suffix);
	}
}

/*!
	@brief remove curve data

	@param[out] yieldData
	@param[in] market name
*/
void
CurveCalibrationData::removeCuveData(AQLObject &yieldData, const AQLString& mktName) const
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
			if (curveName != STD)
			{
				suffix = "_" + curveName;
			}
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
CurveCalibrationData::removeBasisCuveData(AQLObject &yieldData) const
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
				if (curveName != STD)
				{
					suffix = "_" + curveName;
				}
				yieldData.remove(CALIBRATION_DATA_TERMS + suffix);
				yieldData.remove(IR_CALIBRATION_DATA_DFS + suffix);
				yieldData.remove(IR_CALIBRATION_DATA_FORWARDRATES + suffix);
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
CurveCalibrationData::getForwardConvention(const AQLString &curveName, AQLPriceDataDayCount &dc, AQLPriceDataSlidingRule &sld, AQLPriceDataCalendar &cal, AQLString &accessary) const
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
			std::vector<AQLObject*> data_basis;

			bool isSetDataBasis = false;

			for (unsigned int i = 0; i < mr.getSize(); ++i)
			{
				if (isSetDataBasis)
				{
					break;
				}

				AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				datatype_str.toUpper();
				if (datatype_str == BASIS)
				{
					data_basis.push_back(&mr.get(i).get());
					isSetDataBasis = true;
				}
				else if (datatype_str == FWDFX)
				{
					data_basis.push_back(&mr.get(i).get());
					isSetDataBasis = true;
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

			std::vector<AQLObject*> data_libor;
			std::vector<AQLObject*> data_swap;

			bool isSetDataLibor = false;
			bool isSetDataSwap = false;

			for (unsigned int i = 0; i < mr.getSize(); ++i)
			{
				if (isSetDataLibor && isSetDataSwap)
				{
					break;
				}

				AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				datatype_str.toUpper();

				if (datatype_str == ZERO)
				{
					data_libor.push_back(&mr.get(i).get());
					isSetDataLibor = true;
				}
				else if (datatype_str == PAR)
				{
					data_swap.push_back(&mr.get(i).get());
					isSetDataSwap = true;
				}
				else if (datatype_str == YIELD_TYPE_CTD)
				{
					AQLObject* data = &mr.get(i).get();
					dc = dynamic_cast<const AQLPriceDataDayCount &>(data->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
					sld = dynamic_cast<const AQLPriceDataSlidingRule &>(data->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
					cal = dynamic_cast<const AQLPriceDataCalendar &>(data->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
					accessary = etrading::fromFrequencyToTerm(dynamic_cast<const AQLDataString &>(data->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get());
					return;
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
					AQ_THROW("Invalid Frequency: Must be ANNUAL, SEMI-ANNUAL, QUARTERLY or MONTHLY")
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
	@brief get basis curve frequency - TODO: This function should be deprecated!!! It's completely mad to load and search all basis instruments, just to get the curve frequency. Mad Mad Mad!!!

	@param[in] curvename
	@param[out] cal
*/
void
CurveCalibrationData::getBasisCurveFrequency(const AQLString &curveName, AQLString &frequency) const
{
	const AQLString default_frequency = "1Y";
	const AQLDataHolder *dh = 0;

	if (mBCurveGenMap.find(curveName) != mBCurveGenMap.end())
	{
		AQLString suffix = "_" + getMarketForCurve(curveName);
		dh = &getData(CALIBRATION_DATA_MARKETDATA + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			// ------------------------------------------------------------------------
			// The long end of all basis curves is always made up of basis swaps, therefore we wearch for the first basis instrument starting
			// from the last instrument, which is typically a basis instrument, which is more efficient than searching from the front.
			// ------------------------------------------------------------------------
			const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&>(dh->get());
			AQ_REQUIRE(mr.getSize() > 0, "Invalid Basis Curve Calibration Instrument(s): The Basis Curve contains no calibration instruments for CurveName: " + curveName)

				bool foundBasisInstrument = false;
			size_t basisInstrumentPosition = mr.getSize();
			AQLString datatype_str;

			// Search backwards for the position of the Basis Calibration Instrument
			for (size_t i = 0; i < mr.getSize(); ++i)
			{
				basisInstrumentPosition--;
				datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(basisInstrumentPosition).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				datatype_str.toUpper();

				// Exit when we find a basis instrument and indicate as found
				if (datatype_str == BASIS || datatype_str == FWDFX)
				{
					foundBasisInstrument = true;
					break;
				}
			}

			AQ_REQUIRE(foundBasisInstrument == true, "Invalid Basis Curve Calibration Instrument(s): No basis swap reference calibration instruments found")

				AQLObject* basis_instrument = &mr.get(basisInstrumentPosition).get();
			AQ_REQUIRE(basis_instrument != nullptr, "Invalid Basis Curve Calibration Instrument(s): The reference basis calibration instrument is invalid")

				frequency = dynamic_cast<const AQLDataString &>(basis_instrument->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();
			return;
		}
		else
		{
			frequency = default_frequency;
			return;
		}
	}
	else
	{
		AQLString suffix = "";
		const AQLString market = getMarketForCurve(curveName);
		if (market != SWAP)
		{
			suffix = "_" + market;
		}

		dh = &getData(CALIBRATION_DATA_MARKETDATA + suffix, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			// ------------------------------------------------------------------------
			// The long end of all basis curves is always made up of basis swaps, therefore we wearch for the first basis instrument starting
			// from the last instrument, which is typically a basis instrument, which is more efficient than searching from the front.
			// ------------------------------------------------------------------------
			const AQLDataMultiReference& mr = dynamic_cast<const AQLDataMultiReference&>(dh->get());
			AQ_REQUIRE(mr.getSize() > 0, "Invalid Basis Curve Calibration Instrument(s): The Basis Curve contains no calibration instruments for CurveName: " + curveName)

				bool foundBasisInstrument = false;
			size_t basisInstrumentPosition = mr.getSize();
			AQLString datatype_str;

			// Search backwards for the position of the Basis Calibration Instrument
			for (size_t i = 0; i < mr.getSize(); ++i)
			{
				basisInstrumentPosition--;
				datatype_str = dynamic_cast<const AQLDataString&> ((mr.get(basisInstrumentPosition).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				datatype_str.toUpper();

				// Exit early when we find basis instrument flagged as YIELD_TYPE_CTD, for Cheapest-to-Deliver Curves
				if (datatype_str == YIELD_TYPE_CTD)
				{
					AQLObject* basisCurveData = &mr.get(i).get();
					frequency = etrading::fromFrequencyToTerm(dynamic_cast<const AQLDataString &>(basisCurveData->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get());
					return;
				}

				// Exit when we find a basis instrument and indicate as found
				if (datatype_str == PAR)
				{
					foundBasisInstrument = true;
					break;
				}
			}

			AQ_REQUIRE(foundBasisInstrument == true, "Invalid Basis Curve Calibration Instrument(s): No basis swap reference calibration instruments found")

				AQLObject* basis_instrument = &mr.get(basisInstrumentPosition).get();
			AQ_REQUIRE(basis_instrument != nullptr, "Invalid Basis Curve Calibration Instrument(s): The reference basis calibration instrument is invalid")

				AQLString freq;
			dh = &basis_instrument->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				freq = dynamic_cast<const AQLDataString &>(dh->get()).get();
			}
			else
			{
				dh = &basis_instrument->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					freq = dynamic_cast<const AQLDataString &>(dh->get()).get();
				}
				else
				{
					freq = dynamic_cast<const AQLDataString &>(basis_instrument->getData(IR_CALIBRATION_DATA_FREQUENCY, NOCHECK).get()).get();
				}
			}

			// We are searching for 1M, 3M, 6M or 12M ... This function is super slow, so we are trying to do everything we can to exit early....
			// Hence the strange order below, Quarterly is the most common frequency, so we search for that first ...
			if (freq == QUARTERLY)
			{
				frequency = "3M";
				return;
			}
			else if (freq == SEMI_ANNUAL)
			{
				frequency = "6M";
				return;
			}
			else if (freq == ANNUAL)
			{
				frequency = "12M";
				return;
			}
			else if (freq == MONTHLY || freq == LUNAR)
			{
				frequency = "1M";
				return;
			}
			else
			{
				AQ_THROW("Invalid Basis Curve Calibration Instrument(s): Invalid Basis Instrument Frequency")
			}
		}
		else
		{
			frequency = default_frequency;
			return;
		}
	}
}

/*!
	@brief insert non removable market

	@param[in] mktName

*/
void
CurveCalibrationData::insertNonRemovableMarket(const AQLString& mktName)
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
CurveCalibrationData::eraseNonRemovableMarket(const AQLString& mktName)
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
CurveCalibrationData::setCurveConvention(AQLObjectHolder& objHolder,
	std::vector<AQLObject*>& mktData,
	const AQLString& curveName)
{
	AQLString suffix = "";
	if (curveName != STD)
	{
		suffix = "_" + curveName;
	}

	if (mktData.size() == 0)
	{
		throw AQLCoreInvalidData("#Error: Missing Swap Market Data. Swap size must be more than one", __FILE__, __LINE__);
	}

	AQLString freq = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	const AQLPriceDataCalendar* cal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule* sld = &dynamic_cast<const AQLPriceDataSlidingRule &>(mktData[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
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
	@brief remove an element from the market map

	@param[in] key		Key of the element to be removed
*/
void CurveCalibrationData::removeAssignedCurveMktMap(const AQLString& key)
{
	std::map<AQLString, AQLString>::const_iterator iter = mAssignedCurveMktMap.find(key);
	if (iter != mAssignedCurveMktMap.end())
	{
		mAssignedCurveMktMap.erase(key);
	}
}

/* @brief		Retrieve the CurveCalibrationData object for the current curve set
*  @param [in]	objPool					Object pool object
*  @param [in]	curveCollectionID	ID for the curve collection
*  @param [in]	errMsg				Error message when CurveCalibrationData is not found
*  @return		CurveCalibrationData pointer
*/
CurveCalibrationData* CurveCalibrationData::getYieldCurvePro(AQLObjectPool& objPool,
	const AQLString& curveCollectionID,
	const AQLString& errMsg)
{
	AQLDataInstance* dataInstance = etrading::InitializeAQETrading::instance().dataInstance();
	AQLString yieldName(curveCollectionID);

	// get yield data pro
	AQLString CurveID = etrading::LACurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollectionID;
	const AQLObjectHolder ehCur = objPool.getObject(CurveID);
	CurveCalibrationData* curveCalibrationData = NULL;
	if (!ehCur.isDefined())
	{
		if (errMsg.size() == 0)
		{
			curveCalibrationData = new CurveCalibrationData(dataInstance);
			objPool.set(CurveID, curveCalibrationData);
		}
		else
		{
			throw AQLCoreInvalidData(errMsg.getCString(), __FILE__, __LINE__);
		}
	}
	else
	{
		curveCalibrationData = &dynamic_cast<CurveCalibrationData&>(objPool.getObject(CurveID).get());
	}

	// This object will be used to provide 1. input market data
	// and 2. perform the curve solving.
	return curveCalibrationData;
}