/*
 * @brief			LWO curve class that builds multiple yield curves 
 * @Created:		18 Sep 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "CurveObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "SingleCurveObject.h"


namespace etrading
{
    class MultiCurveObject: public CurveObject
    {
	public:

		/* @brief Main Constructor of the LWO Curve object
		*  @param[in]	objectName				The LWO handle name for the curve object
		*/
		MultiCurveObject(const std::string& objectName);

		/* @brief Main Constructor 
		*  @param[in]	objectName				The LWO handle name for the curve object
		*  @param[in]	curveGeneratorNames		The curve generators containing the conventions used to build this curve
		*  @param[in]	curveMarketDataNames	The LWO handles of the objects containing the market data for this curve
		*  @param[in]	engineParams			Engine parameters such as tolerance etc
		*  @param[in]	domesticCurveCollection	The collection which this curve will be placed in, once built   ( The Target CurveCollection )
		*  @param[in]	foreignCurveCollection	The collection containing foreign curve dependencies   ( The Against CurveCollection )
		*/
		MultiCurveObject(const std::string& objectName,
						 const std::vector<std::string>& curveGeneratorNames,
						 const std::vector<std::string>& curveMarketDataNames,
						 const LAStringMatrix& engineParams,
						 const std::string& domesticCurveCollection,
						 const std::string& foreignCurveCollection = "");
		
		/* @brief Constructor used by deserialization
		* @param[in] objectName    The name of this CurveMarketData instance
		* @param[in] singleCurves  A collection of single curve objects
		*/
		MultiCurveObject(const std::string& objectName,
						 const std::vector<SingleCurveObjectPtr>& singleCurves);

		/* @brief Constructor used by deserialization
		* @param[in] objectName    The name of this MultiCurveObject instance
		* @param[in] freeObject    A freeObject constructed from the serialized data
		*/
		MultiCurveObject(const std::string& objectName, const FreeObject& freeObject);

		/* @brief Copy Constructor
		*/
   		MultiCurveObject(const MultiCurveObject& rhs);

		virtual ~MultiCurveObject() {}

		/* @brief Used to serialize an instance of this class, including the CurveGenerator and CurveMarketData objects this curve depends on
		* @param[out] the populated SchemaObject
		*/
		virtual const SchemaObject toSchemaObject() const;

		/* @brief	Helper function which returns the cached CurveIndexNames of all the component curves
		*           The CurveIndex names are populated from the curve generator conventions
		* @returns	The curveIndexNames
		*/
		const LAStringVector& getCurveIndexNames() const;

		/* @brief	Helper function which returns all the component single curve ojects
		* @returns	All component curve objects
		*/
		const std::vector<SingleCurveObjectPtr>& getCurveObjects() const { return singleCurves_; }

		/* @brief	Function to identify the curve type i.e. SINGLE_CURVE, MULTI_CURVE, DUAL_CURVE
		*/
		virtual CurveCalibrationTypeEnum curveCalibrationType() const;

		/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
		*/
		virtual void calibrateCurve();

		/* @brief	Set engine parameters that are common across all curves
		*/
		void setEngineParams(const LAStringMatrix& engineParams) { engineParams_ = engineParams; }

		// Get Curve Collection Names
		virtual std::string getCurveCollection() const				{ return domesticCurveCollection_; }
		virtual std::string getDomesticCurveCollection() const		{ return domesticCurveCollection_; }
		virtual std::string getForeignCurveCollection() const		{ return foreignCurveCollection_; }

	protected:

		// Curve Build Parameters to be serialized are held in a FreeObject
		std::vector<SingleCurveObjectPtr> singleCurves_;
		
		// Common parameters across the curves
		LAStringMatrix engineParams_;

		// Name of the dual-bootstrapped curves object
		std::string objectName_;

		// Curve Build Parameters to be serialized are held in a FreeObject
		FreeObject freeObject_;

	private:

		// Disable assignment operator
		MultiCurveObject& operator=(const MultiCurveObject& MultiCurveObject) = delete;

		//----------------------------------------------------------------------
		// private member funnctions

		/* @brief	Given a set of curve-build parameters, populates the internal FreeObject member data of this class
		*/
		virtual void populateFreeObjectFromBuildParameters();

		/* @brief	Converts the internal FreeObject into a group of string parameters.
		*			Used when de-serialising the curves.
		*  @param[out]	objectName				The LWO handle name for the curve object
		*  @param[out]	curveGeneratorNames		The curve generators containing the conventions used to build this curve
		*  @param[out]	curveMarketDataNames	The LWO handles of the objects containing the market data for this curve
		*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
		*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
		*  @param[out]	engineParams			Engine parameters
		*/
		virtual void getBuildParametersFromFreeObject( std::string& objectName,
													   std::vector<std::string>& curveGeneratorNames,
													   std::vector<std::string>& curveMarketDataNames,
													   std::string& domesticCurveCollection,
													   std::string& foreignCurveCollection,
												       LAStringMatrix& engineParams ) const;
		

		//----------------------------------------------------------------------
		// private member variables

		// Curve indexes
		LAStringVector componentCurveIndexNames_;

		// Domestic curve collection
		std::string domesticCurveCollection_;

		// Foreign curve collection
		std::string foreignCurveCollection_;
	};

	// Shared pointer type declaration
	typedef std::shared_ptr<MultiCurveObject> MultiCurveObjectPtr;
}

