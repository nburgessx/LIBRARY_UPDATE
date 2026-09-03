/*
 * @brief			LWO curve class that specialises in building dual bootstrapping curves
 * @Created:		15 Sep 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "DualBootstrappedCurveObject.h"
#include "ParameterValidation.h"
#include "ObjectUtilities.h"
#include "LAUpdateStaticDataManager.h"
#include "LWOUtilities.h"
#include "CreateDataFile.h"
#include "Variant.h"

#include <vector>
#include <string>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>

namespace etrading
{
	/* @brief Main Constructor of the LWO Dual Bootstrapped Curve object
	*  @param [in]	objectName				The LWO handle name for the curve object
	*  @param [in]  curveCollection			Curve collection to which calibrated curves belong
	*  @param [in]  swapCurveGeneratorName  The name of the Swap curve generator that defines Swap curve's conventions
	*  @param [in]  oisCurveGeneratorName   The name of the OIS curve generator that defines OIS curve's conventions
	*  @param [in]  lwoSwapMarketObj		Object that encapsulates all of Swap curve's market data
	*  @param [in]  lwoOISMarketObj			Object that encapsulates all of OIS curve's market data
	*/
	DualBootstrappedCurveObject::DualBootstrappedCurveObject(const std::string& objectName,
															const std::string& curveCollection,
															const std::string& swapCurveGeneratorName,
															const std::string& oisCurveGeneratorName,
															const std::string& lwoSwapMarketObj,
															const std::string& lwoOISMarketObj)
		: curveCollection_(curveCollection),
		  oisCurveName_(""),
		  swapCurveName_(""),
		  oisCurveIndexNames_(""),
		  swapCurveIndexNames_(""),		
		  MultiCurveObject(objectName)
	{				
		engineParams_.clear();

		// Build the OIS and Swap curve objects
		swapCurveObj_ = SingleCurveObjectPtr(new SingleCurveObject("", swapCurveGeneratorName, lwoSwapMarketObj, curveCollection, ""));
		oisCurveObj_  = SingleCurveObjectPtr(new SingleCurveObject("", oisCurveGeneratorName, lwoOISMarketObj, curveCollection, ""));
	}


	/* @brief Constructor used by deserialization
	* @param[in] objectName     The name of this MultiCurveObject instance
	* @param[in] swapCurveObj	The swap curve object
	* @param[in] oisCurveObj	The OIS curve object
	*/
	DualBootstrappedCurveObject::DualBootstrappedCurveObject( const std::string& objectName, 
															  const SingleCurveObjectPtr& swapCurveObj,
															  const SingleCurveObjectPtr& oisCurveObj ) 
															  : swapCurveObj_(swapCurveObj),
															    oisCurveObj_(oisCurveObj),
																MultiCurveObject(objectName)
	{
		engineParams_.clear();
	}

	/* @brief Copy Constructor
	 */
	DualBootstrappedCurveObject::DualBootstrappedCurveObject(const DualBootstrappedCurveObject& rhs) 
														: MultiCurveObject(rhs.objectName_),
														  curveCollection_(rhs.curveCollection_),
														  swapCurveObj_(rhs.swapCurveObj_),
														  oisCurveObj_(rhs.oisCurveObj_),
														  oisCurveName_(rhs.oisCurveName_),
														  swapCurveName_(rhs.swapCurveName_),
														  oisCurveIndexNames_(rhs.oisCurveIndexNames_),
														  swapCurveIndexNames_(rhs.swapCurveIndexNames_)
	{}

	/* @brief	Given a set of curve-build parameters, populates the internal FreeObject member data of this class
	*/
	void DualBootstrappedCurveObject::populateFreeObjectFromBuildParameters()
	{}

	/* @brief	Converts the internal FreeObject into a group of string parameters.
	*			Used when de-serialising the curves.
	*  @param[out]	objectName				The LWO handle name for the curve object
	*  @param[out]	curveGeneratorNames		The curve generators containing the conventions used to build this curve
	*  @param[out]	curveMarketDataNames	The LWO handles of the objects containing the market data for this curve
	*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
	*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
	*/
	void DualBootstrappedCurveObject::getBuildParametersFromFreeObject( std::string& objectName,
											   std::vector<std::string>& curveGeneratorNames,
											   std::vector<std::string>& curveMarketDataNames,
											   std::string& domesticCurveCollection,
											   std::string& foreignCurveCollection ) const
	{}

	/* @brief	Function to identify the curve type i.e. SINGLE_CURVE, MULTI_CURVE, DUAL_CURVE
	*/
	CurveCalibrationTypeEnum DualBootstrappedCurveObject::curveCalibrationType() const
	{
		return etrading::DUAL_CURVE_CALIBRATION;
	}

	/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
	*/
	void DualBootstrappedCurveObject::calibrateCurve()
	{
        // Retrieve a list of key curve info that identifies the OIS curve
		std::string curveCollection_ois;
		std::string curveName_ois; 		
		std::string curveIndex_ois;
		std::string temp1;
		std::string temp2;
		std::string marketDataAsOfDate_ois;

		oisCurveObj_->getCurveIdentifierInfos(curveCollection_ois, 
								              curveName_ois, 
								              curveIndex_ois,
								              temp1,
								              temp2,
								              marketDataAsOfDate_ois);

		oisCurveName_ = curveName_ois;
		oisCurveIndexNames_ = curveIndex_ois;

		// Retrieve a list of key curve info that identifies the swap curve
		std::string curveCollection_swap;
		std::string curveName_swap; 		
		std::string curveIndex_swap;
		std::string frequency_swap;
		std::string marketDataAsOfDate_swap;

		swapCurveObj_->getCurveIdentifierInfos(curveCollection_swap, 
								               curveName_swap, 
								               curveIndex_swap,
								               temp1,
								               frequency_swap,
								               marketDataAsOfDate_swap);
		
		swapCurveName_ = curveName_swap;
		swapCurveIndexNames_ = curveIndex_swap;

		if (! boost::iequals( marketDataAsOfDate_ois.c_str(), marketDataAsOfDate_swap.c_str()) )
		{
			throw LACoreInvalidData("#Error: OIS curve and swap curve are given different asof dates", __FILE__, __LINE__);
		}

		if (! boost::iequals(curveCollection_ois, curveCollection_swap) )
		{
			throw LACoreInvalidData("#Error: OIS curve and swap curve are from different curve collections", __FILE__, __LINE__);
		}

		// Create OIS curve data object that supplies curve data for OIS curve build
		CurveGeneratorPtr oisCurveGeneratorObj	= oisCurveObj_->getCurveGeneratorObj();
		CurveMarketDataPtr oisCurveMarketObj	= oisCurveObj_->getCurveMarketDataObj();
		OISCurveObjectData curveData_ois(oisCurveGeneratorObj, oisCurveMarketObj, marketDataAsOfDate_ois, false);

		// Create Swap curve data object that supplies curve data for swap curve build
		CurveGeneratorPtr swapCurveGeneratorObj = swapCurveObj_->getCurveGeneratorObj();
		CurveMarketDataPtr swapCurveMarketObj	= swapCurveObj_->getCurveMarketDataObj();
		SwapCurveObjectData curveData_swap(swapCurveGeneratorObj, swapCurveMarketObj, frequency_swap, marketDataAsOfDate_swap);

		etrading::LAUpdateStaticDataManager::setUpDualBootstrapCurve( etrading::getDataInstance(),
												                      curveCollection_ois.c_str(),
												                      objectName_.c_str(),
												                      curveName_ois.c_str(),	
												                      curveName_swap.c_str(), 
												                      curveIndex_ois.c_str(),
												                      curveIndex_swap.c_str(),
												                      engineParams_,
												                      curveData_ois.curveConvLVB_.toLAStringMatrix(),       // Conventions as LVB 
												                      curveData_ois.oisRates_, 
												                      curveData_ois.oisConvLVB_.toLAStringMatrix(),         // Conventions as LVB
												                      curveData_ois.histRates_,
												                      curveData_ois.loBasisRates_, 
												                      curveData_ois.loBasisConvLVB_.toLAStringMatrix(),       // Conventions as LVB
												                      curveData_ois.swapConvLVB_.toLAStringMatrix(),          // Conventions as LVB
												                      curveData_swap.curveConvLVB_.toLAStringMatrix(),        // Conventions as LVB 
												                      curveData_swap.moneyMarketConvLVB_.toLAStringMatrix(),  // Conventions as LVB
												                      curveData_swap.liborRates_, 
												                      curveData_swap.liborConvLVB_.toLAStringMatrix(),        // Conventions as LVB
												                      curveData_swap.swapRates_, 
												                      curveData_swap.swapConvLVB_.toLAStringMatrix(),         // Conventions as LVB
												                      curveData_swap.fra3mRates_,
												                      curveData_swap.fra6mRates_,
												                      curveData_swap.fraConvLVB_.toLAStringMatrix(),          // Conventions as LVB
												                      curveData_swap.futureRates_, 
												                      curveData_swap.futureConvLVB_.toLAStringMatrix(),       // Conventions as LVB
												                      curveData_swap.basisAdjConvLVB_.toLAStringMatrix(),     // Conventions as LVB
												                      curveData_swap.basisAdjRates_	);
	}

	/* @brief Used to serialize an instance of this class
	* @param[out] the populated SchemaObject
	*/
	const SchemaObject DualBootstrappedCurveObject::toSchemaObject() const
	{
		SchemaObject schemaObject(CURVE, getRefToName());
		return schemaObject;
	}

}
