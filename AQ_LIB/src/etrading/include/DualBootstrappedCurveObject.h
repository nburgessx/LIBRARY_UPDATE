#pragma once

#include "MultiCurveObject.h"
#include "SingleCurveObject.h"
#include "IsAQObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"


namespace etrading
{
    class DualBootstrappedCurveObject: public MultiCurveObject
    {
	public:
		/* @brief Main Constructor of the AQO Dual Bootstrapped Curve object
		*  @param [in]	objectName				The AQO object handle name for the curve object
		*  @param [in]  curveCollection			Curve collection to which calibrated curves belong
		*  @param [in]  swapCurveGeneratorName  The name of the Swap curve generator that defines Swap curve's conventions
		*  @param [in]  oisCurveGeneratorName   The name of the OIS curve generator that defines OIS curve's conventions
		*  @param [in]  aqoSwapMarketObj		Object that encapsulates all of Swap curve's market data
		*  @param [in]  aqoOISMarketObj			Object that encapsulates all of OIS curve's market data
		*/
		DualBootstrappedCurveObject(const std::string& objectName,
									const std::string& curveCollection,
									const std::string& swapCurveGeneratorName,
									const std::string& oisCurveGeneratorName,
									const std::string& aqoSwapMarketObj,
									const std::string& aqoOISMarketObj);
		
		/* @brief Constructor used by deserialization
		* @param[in] objectName     The name of this CurveMarketData instance
		* @param[in] swapCurveObj	The swap curve object
		* @param[in] oisCurveObj	The OIS curve object
		*/
		DualBootstrappedCurveObject(const std::string& objectName,
									const SingleCurveObjectPtr& swapCurveObj,
									const SingleCurveObjectPtr& oisCurveObj);

		/* @brief Copy Constructor
		*/
   		DualBootstrappedCurveObject(const DualBootstrappedCurveObject& rhs);

		/* @brief Destructor
		*/
		virtual ~DualBootstrappedCurveObject() {}

		/* @brief Used to serialize an instance of this class, including the CurveGenerator and CurveMarketData objects this curve depends on
		* @param[out] the populated SchemaObject
		*/
		virtual const SchemaObject toSchemaObject() const;

		/* @brief	Function to identify the curve type i.e. SINGLE_CURVE, MULTI_CURVE, DUAL_CURVE
		*/
		virtual CurveCalibrationTypeEnum curveCalibrationType() const;

		/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
		*/
		virtual void calibrateCurve();

		/* @brief	Get swap curve index names
		*/
		std::string getSwapCurveIndexNames() { return swapCurveIndexNames_; }

		/* @brief	Get OIS curve index names
		*/
		std::string getOISCurveIndexNames() { return oisCurveIndexNames_; }
		
	private:

		//----------------------------------------------------------------------
		// private member variables

		// Calibrated curve objects for OIS and Swap curves
		SingleCurveObjectPtr oisCurveObj_;
		SingleCurveObjectPtr swapCurveObj_;
		
		// Curve names for OIS and Swap curves
		std::string swapCurveName_;
		std::string oisCurveName_;

		// Curve index names for OIS and Swap curves
		std::string swapCurveIndexNames_;
		std::string oisCurveIndexNames_;

		// Curve collection used by the curves
		std::string curveCollection_;

		//----------------------------------------------------------------------
		// private member funnctions

		/* @brief	Given a set of curve-build parameters, populates the internal FreeObject member data of this class
		*/
		virtual void populateFreeObjectFromBuildParameters();

		/* @brief	Converts the internal FreeObject into a group of string parameters.
		*			Used when de-serialising the curves.
		*  @param[out]	objectName				The AQO object handle name for the curve object
		*  @param[out]	curveGeneratorNames		The curve generators containing the conventions used to build this curve
		*  @param[out]	curveMarketDataNames	The AQO object handles of the objects containing the market data for this curve
		*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
		*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
		*/
		virtual void getBuildParametersFromFreeObject( std::string& objectName,
											   std::vector<std::string>& curveGeneratorNames,
											   std::vector<std::string>& curveMarketDataNames,
											   std::string& domesticCurveCollection,
											   std::string& foreignCurveCollection ) const;
		
	};

	// Shared pointer type declaration
	typedef std::shared_ptr<DualBootstrappedCurveObject> DualBootstrappedCurveObjectPtr;
}

