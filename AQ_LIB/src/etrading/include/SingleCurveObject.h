

#pragma once

/*
 * @File: SingleCurveObject.h
 * @brief: This class represents the curve object used to represent curve inputs which can be stored in the LWO Cache
 * @Created: 17 July 2017
 * @Author: Ian Castleton
 * @Department: ISO Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "CurveObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CurveGenerator.h"
#include "CurveMarketData.h"

namespace etrading
{
    class SingleCurveObject: public CurveObject
    {
	public:
		/* @brief Main Constructor of the LWO Curve object
		*  @param[in]	objectName				The LWO handle name for the curve object
		*  @param[in]	lwoCurveGeneratorName	The curve generator containing the conventions used to build this curve
		*  @param[in]	lwoCurveMarketDataName	The LWO handle of the object containing the market data for this curve
		*  @param[in]	domesticCurveCollection	The collection which this curve will be placed in, once built   ( The Target CurveCollection )
		*  @param[in]	foreignCurveCollection	The collection containing foreign curve dependencies   ( The Against CurveCollection )
		*/
		SingleCurveObject( const std::string& objectName,
					  	   const std::string& curveGeneratorName,
						   const std::string& curveMarketDataName,
						   const std::string& domesticCurveCollection,
						   const std::string& foreignCurveCollection );
		
		/* @brief Constructor used by deserialization
		* @param[in] objectName    The name of this CurveMarketData instance
		* @param[in] freeObject    A freeObject constructed from the serialized data
		*/
		SingleCurveObject( const std::string& objectName,
                           const FreeObject& freeObject );

		/* @brief Copy Constructor
		*/
   		SingleCurveObject(const SingleCurveObject& rhs);
		virtual ~SingleCurveObject() {}

		/* @brief Used to serialize an instance of this class, including the CurveGenerator and CurveMarketData objects this curve depends on
		* @param[out] the populated SchemaObject
		*/
		virtual const SchemaObject toSchemaObject() const;

		/* @brief	Helper function which returns the cached CurveIndexName of this curve
		*           The CurveIndex name is populated from the curve generator conventions
		* @returns	The curveIndexName
		*/
		const std::string& getCurveIndexName() const;

		/* @brief	Method that gets the market data object of the curve
		*  @returns	The underlying market data object
		*/
		const CurveMarketDataPtr& getCurveMarketDataObj() const;

		/* @brief	Method that sets the market data object back to the curve
		* @param[in] objectName    The name of this CurveMarketData instance
		*/
		void setCurveMarketDataObj(const CurveMarketDataPtr& marketObj);

		/* @brief	Method that returns the curve generator object
		*  @returns	The curve generator object
		*/
		const CurveGeneratorPtr& getCurveGeneratorObj() const;

		/* @brief	Function to identify the curve type i.e. SINGLE_CURVE, MULTI_CURVE, DUAL_CURVE
		*/
		virtual CurveCalibrationTypeEnum curveCalibrationType() const;

		/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
		*/
		virtual void calibrateCurve();
                
        /* @brief	Get the curve collection
		*/
        std::string getCurveCollection() const          { return domesticCurveCollection_; };

        /* @brief	Get the domestic curve collection
		*/
        std::string getDomesticCurveCollection() const  { return domesticCurveCollection_; };

        /* @brief	Get the foreign curve collection
		*/
        std::string getForeignCurveCollection() const   { return foreignCurveCollection_; };

		/* @brief	Return a series of information that uniquely identify a curve
		*/
		void getCurveIdentifierInfos( std::string& curveCollection, 
									  std::string& curveName, 
									  std::string& curveIndexes,
									  std::string& configCurveType,
									  std::string& configFrequency,
									  std::string& marketDataAsOfDate );
		
	protected:

		// Curve Build Parameters to be serialized are held in a FreeObject
		FreeObject freeObject_;
		
		// The curveIndex name this curve object constructs
		std::string cachedCurveIndexName_;

	private:

		// Name of the curve object
		std::string objectName_;

		// Name of the curve generator
		std::string lwoCurveGeneratorName_;

		// Curve generator object
		CurveGeneratorPtr lwoCurveGenerator_;

		// Name of the curve market data object
		std::string lwoCurveMarketDataName_;

		// Curve market data object
		CurveMarketDataPtr lwoCurveMarketData_;

		// Domestic curve collection
		std::string domesticCurveCollection_;

		// Foreign curve collection
		std::string foreignCurveCollection_;
		

		/* @brief	Given a set of curve-build parameters, populates the internal FreeObject member data of this class
		*/
		virtual void populateFreeObjectFromBuildParameters();

		/* @brief	Converts the internal FreeObject into a group of string parameters.
		*			Used when de-serialising the curve.
		*  @param[out]	objectName				The LWO handle name for the curve object
		*  @param[out]	lwoCurveGeneratorName	The curve generator containing the conventions used to build this curve
		*  @param[out]	lwoCurveMarketDataName	The LWO handle of the object containing the market data for this curve
		*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
		*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
		*/
		virtual void getBuildParametersFromFreeObject( std::string& objectName,
											   std::string& lwoCurveGeneratorName,
											   std::string& lwoCurveMarketDataName,
											   std::string& domesticCurveCollection,
											   std::string& foreignCurveCollection ) const;
						
	};

	// Shared pointer type declaration
	typedef std::shared_ptr<SingleCurveObject> SingleCurveObjectPtr;
}

