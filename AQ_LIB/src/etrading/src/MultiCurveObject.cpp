#include "MultiCurveObject.h"
#include "ParameterValidation.h"
#include "ObjectUtilities.h"
#include "AQLUpdateStaticDataManager.h"
#include "AQObjUtilities.h"
#include "CreateDataFile.h"
#include "ExceptionMacros.h"
#include "Variant.h"

#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

namespace etrading
{
	const std::string FAST_REBUILD		= "FASTREBUILD";
	const std::string EPSILON			= "EPSILON";
	const std::string GRADIENT_EPSILON	= "GRADIENTEPSILON";
	const std::string DELTA_EPSILON		= "DELTA";

	/* @brief Main Constructor of the AQObj Curve object
	*  @param[in]	objectName				The AQObj object handle name for the curve object
	*/
	MultiCurveObject::MultiCurveObject(const std::string& objectName): CurveObject(objectName), objectName_(objectName), freeObject_(objectName)
	{
		engineParams_.clear();
		singleCurves_.clear();
		componentCurveIndexNames_.clear();
	}

	/* @brief Main Constructor 
	*  @param[in]	objectName				The AQObj object handle name for the curve object
	*  @param[in]	curveGeneratorNames		The curve generators containing the conventions used to build this curve
	*  @param[in]	curveMarketDataNames	The AQObj object handles of the objects containing the market data for this curve
	*  @param[in]	engineParams			Engine parameters such as tolerance etc
	*  @param[in]	domesticCurveCollection	The collection which this curve will be placed in, once built   ( The Target CurveCollection )
	*  @param[in]	foreignCurveCollection	The collection containing foreign curve dependencies   ( The Against CurveCollection )
	*/
	MultiCurveObject::MultiCurveObject(const std::string& objectName,
						               const std::vector<std::string>& curveGeneratorNames,
						               const std::vector<std::string>& curveMarketDataNames,
						               const AQLStringMatrix& engineParams,
						               const std::string& domesticCurveCollection,
						               const std::string& foreignCurveCollection )
        : CurveObject(objectName),
		  domesticCurveCollection_(domesticCurveCollection), 
		  foreignCurveCollection_(foreignCurveCollection), 
		  objectName_(objectName),
		  freeObject_(objectName),
		  engineParams_(engineParams)

	{
		singleCurves_.clear();
		componentCurveIndexNames_.clear();

		// Instantiate data objects for each curve
		for (unsigned int i = 0; i < curveGeneratorNames.size(); ++i)
		{
			SingleCurveObjectPtr curve(new SingleCurveObject("" /*object name is not relevant*/, curveGeneratorNames[i], curveMarketDataNames[i], domesticCurveCollection, ""));
			singleCurves_.push_back(curve);
		}

		// Populate 'free object' so that object can be streamed
		populateFreeObjectFromBuildParameters();
	}


	/* @brief Constructor used by deserialization
	* @param[in] objectName    The name of this MultiCurveObject instance
	* @param[in] singleCurves  A collection of single curve objects
	*/
	MultiCurveObject::MultiCurveObject( const std::string& objectName, 
									    const std::vector<SingleCurveObjectPtr>& singleCurves)
		: CurveObject(objectName), 
		  singleCurves_(singleCurves),
		  objectName_(objectName),
		  freeObject_(objectName)
	{}

	/* @brief Copy Constructor
	 */
	MultiCurveObject::MultiCurveObject(const MultiCurveObject& rhs) 
								: CurveObject(rhs),
								  singleCurves_(rhs.singleCurves_),
									componentCurveIndexNames_(rhs.componentCurveIndexNames_),
									domesticCurveCollection_(rhs.domesticCurveCollection_),
									foreignCurveCollection_(rhs.foreignCurveCollection_),
									freeObject_(rhs.freeObject_)
	{}

	/* @brief Constructor used by deserialization
	* @param[in] objectName    The name of this MultiCurveObject instance
	* @param[in] freeObject    A freeObject constructed from the serialized data
	*/
	MultiCurveObject::MultiCurveObject(const std::string& objectName, const FreeObject& freeObject)
								: CurveObject(objectName),
								freeObject_(freeObject),
								objectName_(objectName)
	{
		std::string tmpObjectName;
		std::vector<std::string> curveGeneratorNames;
		std::vector<std::string> curveMarketDataNames;
		engineParams_.clear();
		getBuildParametersFromFreeObject(tmpObjectName, curveGeneratorNames, curveMarketDataNames, domesticCurveCollection_, foreignCurveCollection_, engineParams_);

		if (tmpObjectName != objectName)
		{
			{ std::ostringstream aqCoreMsg20;
aqCoreMsg20 << "Inconsistent data when deserializing curve: Object handle name is '" << objectName << "' while CurveData contains '" << tmpObjectName << "' ."; AQ_THROW( aqCoreMsg20.str() ); }
		}
		
		singleCurves_.clear();
		componentCurveIndexNames_.clear();

		// Instantiate data objects for each curve
		for (unsigned int i = 0; i < curveGeneratorNames.size(); ++i)
		{
			SingleCurveObjectPtr curve(new SingleCurveObject("" /*object name is not relevant*/, curveGeneratorNames[i], curveMarketDataNames[i], domesticCurveCollection_, ""));
			singleCurves_.push_back(curve);
		}
	}

	// --------------------------------------------------------------------------------------------

	/* @brief	Function to identify the curve type i.e. SINGLE_CURVE, MULTI_CURVE, DUAL_CURVE
	*/
	CurveCalibrationTypeEnum MultiCurveObject::curveCalibrationType() const
	{
		return etrading::MULTI_CURVE_CALIBRATION;
	}

	/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
	*/
	void MultiCurveObject::calibrateCurve( )
	{
        std::vector<CurveObjectDataPtr> curveDataCollection;

		std::string curveName("");
		std::string curveName_temp("");
		std::string curveIndex("");
		std::string temp("");
		std::string frequency("");

		std::string curveCollection("");
		std::string curveCollection_temp("");
		std::string marketDataAsOfDate("");
		std::string marketDataAsOfDate_temp("");

		for (unsigned int i = 0; i < singleCurves_.size(); ++i)
		{			
			SingleCurveObjectPtr curveObj = singleCurves_[i];
			CurveGeneratorPtr curveGenerator = curveObj->getCurveGeneratorObj();
			CurveMarketDataPtr marketDataObj = curveObj->getCurveMarketDataObj();
			
			// Retrieve a list of key curve info that identifies the current curve				
			std::string curveType;
			curveObj->getCurveIdentifierInfos(curveCollection,
											  curveName,
											  curveIndex,
											  curveType,
											  frequency,
											  marketDataAsOfDate);

			// Perform some basic consistency checks between the current curve and the previous curve
			if (i != 0)
			{
				if (! boost::iequals(curveCollection, curveCollection_temp) )
				{ 
					AQ_THROW("Curve '" + curveName_temp + "' and curve '" + curveName + "' are from different curve collections.");
				}

				if (! boost::iequals(marketDataAsOfDate.c_str(), marketDataAsOfDate_temp.c_str()) )
				{
					AQ_THROW("Curve '" + curveName_temp + "' and curve '" + curveName + "' have different asof dates.");
				}
			}

			curveCollection_temp	    = curveCollection;
			marketDataAsOfDate_temp	    = marketDataAsOfDate;
			curveName_temp			    = curveName;

			// Create one curve data object for each curve
			if (curveType == "OIS")
			{
				// Create OIS curve data object that supplies curve data for OIS curve build
				CurveObjectDataPtr curveData( new OISCurveObjectData (curveGenerator, marketDataObj, marketDataAsOfDate, false, curveName, curveIndex));
				curveDataCollection.push_back(curveData);
				componentCurveIndexNames_.push_back(curveData->curveIndex_);
			}
			else if (curveType == "SWAP")
			{
				// Create Swap curve data object that supplies curve data for swap curve build
				CurveObjectDataPtr curveData(new SwapCurveObjectData(curveGenerator, marketDataObj, frequency, marketDataAsOfDate, curveName, curveIndex));
				curveDataCollection.push_back(curveData);
				componentCurveIndexNames_.push_back(curveData->curveIndex_);
			}
			else if (curveType == "TENORBASIS")
			{
				// Create Basis curve data object that supplies curve data for swap curve build
				CurveObjectDataPtr curveData(new TenorBasisCurveObjectData(curveGenerator, marketDataObj, frequency, marketDataAsOfDate, curveCollection, "", curveName, curveIndex));
				curveDataCollection.push_back(curveData);
				componentCurveIndexNames_.push_back(curveData->curveIndex_);
			}
			else
			{
				AQ_THROW("'" + curveType + "' is not a valid curve type.Use either OIS, SWAP, or BASIS.");
			}
		}

		AQLUpdateStaticDataManager::setUpGlobalEngineCurves(etrading::getDataInstance(),
													   objectName_.c_str(),
													   domesticCurveCollection_.c_str(),
													   engineParams_,
													   curveDataCollection);

	}
	
	/* @brief	Given a set of curve-build parameters, populates the internal FreeObject member data of this class
	*/
	void MultiCurveObject::populateFreeObjectFromBuildParameters()
	{
		std::vector<std::string> columnNames;
		columnNames.push_back(DATA_NAMES);
		columnNames.push_back(DATA_VALUES);

		std::vector<ContainedTypeEnum> colTypes;
		colTypes.push_back(STRING_VALUE);
		colTypes.push_back(VARIANT_VALUE);

		//----------------------------------------------------------------
		// Data names

		VariantVector attributeNames;
		attributeNames.push_back(OBJECT_NAME);
		attributeNames.push_back(DOMESTIC_CURVE_COLLECTION);
		attributeNames.push_back(FOREIGN_CURVE_COLLECTION);
		
		// Add engine parameters if there's any
		for (size_t i = 0; i < engineParams_.size(); ++i)
		{
			AQLStringVector param = engineParams_[i];
			if (param.size() == 2)
			{
				AQLString attributeName = param[0];
				attributeName.toUpper();
				attributeNames.push_back(attributeName);
			}
		}

		// Add engine parameters if there's any
		for (size_t i = 0; i < singleCurves_.size(); ++i)
		{
			std::stringstream s;
			s << i;
			std::string index = s.str().c_str();
			attributeNames.push_back(CURVE_GENERATOR_NAME + index);
			attributeNames.push_back(CURVE_MARKET_CALIBRATION_DATAAME + index);
		}

		//----------------------------------------------------------------
		// Data values

		VariantVector attributeValues;
		attributeValues.push_back(objectName_);						// Engine object name
		attributeValues.push_back(domesticCurveCollection_);		// Domestic curve collection
		attributeValues.push_back(foreignCurveCollection_);			// Foreign curve collection
		
		// Add engine parameters if there's any
		for (size_t i = 0; i < engineParams_.size(); ++i)
		{
			AQLStringVector param = engineParams_[i];
			if (param.size() == 2)
			{
				attributeValues.push_back(param[1]);
			}
		}

		// Add curve generator name and market data name for each component curve
		for (size_t i = 0; i < singleCurves_.size(); ++i)
		{
			SingleCurveObjectPtr curve = singleCurves_[i];
			std::string curveGeneratorObjName	= curve->getCurveGeneratorObj()->getRefToName();
			std::string curveMarketObjName		= curve->getCurveMarketDataObj()->getRefToName();
			attributeValues.push_back(curveGeneratorObjName);
			attributeValues.push_back(curveMarketObjName);
		}

		VariantMatrix rangeData;
		rangeData.push_back(attributeNames);
		rangeData.push_back(attributeValues);

		std::string schemaName = CURVE_BUILD_SCHEMA_NAME;
		const bool allowJaggedData = false;

		freeObject_.clearAll();
		freeObject_ += createFreeObjectFromGrid(objectName_, columnNames, colTypes, rangeData, schemaName, allowJaggedData);
	}

	/* @brief	Converts the internal FreeObject into a group of string parameters.
	*			Used when de-serialising the curves.
	*  @param[out]	objectName				The AQObj object handle name for the curve object
	*  @param[out]	curveGeneratorNames		The curve generators containing the conventions used to build this curve
	*  @param[out]	curveMarketDataNames	The AQObj object handles of the objects containing the market data for this curve
	*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
	*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
	*  @param[out]	engineParams			Engine parameters
	*/
	void MultiCurveObject::getBuildParametersFromFreeObject( std::string& objectName,
											   std::vector<std::string>& curveGeneratorNames,
											   std::vector<std::string>& curveMarketDataNames,
											   std::string& domesticCurveCollection,
											   std::string& foreignCurveCollection,
												AQLStringMatrix& engineParams) const
	{
		// Get the curve build parameters
		const etrading::VariantMatrix& allDataView = freeObject_.viewAllData();
		const auto columnIndices = freeObject_.columnsOfSchema(CURVE_BUILD_SCHEMA_NAME);
		etrading::VariantMatrix variantMatrix;
		std::for_each(columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView](const int idx)
		{
			variantMatrix.push_back(allDataView[idx]);
		});


		if (variantMatrix.size() != 2)
		{
			{ std::ostringstream aqCoreMsg21;
aqCoreMsg21 << "Invalid number of columns in deserialised data. Expecting 2 columns, found '" << variantMatrix.size() << "'. "; AQ_THROW( aqCoreMsg21.str() ); }
		}

		const VariantVector& attributeNames = variantMatrix[0];
		const VariantVector& attributeValues = variantMatrix[1];
		
		const size_t numRows = variantMatrix[0].size();
		for (size_t i = 0; i<numRows; i++)
		{
			if (attributeNames[i] == OBJECT_NAME)
			{
				objectName = attributeValues[i].getValueAsString();
			}
			else if (attributeNames[i] == FAST_REBUILD)
			{
				AQLStringVector param;
				param.push_back(FAST_REBUILD.c_str());
				param.push_back(attributeValues[i].getValueAsString().c_str());
				engineParams.push_back(param);
			}
			else if (attributeNames[i] == EPSILON)
			{
				AQLStringVector param;
				param.push_back(EPSILON.c_str());
				param.push_back(attributeValues[i].getValueAsString().c_str());
				engineParams.push_back(param);
			}
			else if (attributeNames[i] == GRADIENT_EPSILON)
			{
				AQLStringVector param;
				param.push_back(GRADIENT_EPSILON.c_str());
				param.push_back(attributeValues[i].getValueAsString().c_str());
				engineParams.push_back(param);
			}
			else if (attributeNames[i] == DELTA_EPSILON)
			{
				AQLStringVector param;
				param.push_back(DELTA_EPSILON.c_str());
				param.push_back(attributeValues[i].getValueAsString().c_str());
				engineParams.push_back(param);
			}
			else if (attributeNames[i] == DOMESTIC_CURVE_COLLECTION)
			{
				domesticCurveCollection = attributeValues[i].getValueAsString();
			}
			else if (attributeNames[i] == FOREIGN_CURVE_COLLECTION)
			{
				foreignCurveCollection = attributeValues[i].getValueAsString();
			}
			else
			{
				std::string attributeName = attributeNames[i].toString();
				if (attributeName.find(CURVE_GENERATOR_NAME) != std::string::npos)
				{
					curveGeneratorNames.push_back(attributeValues[i].getValueAsString());
				}
				else if (attributeName.find(CURVE_MARKET_CALIBRATION_DATAAME) != std::string::npos)
				{ 
					curveMarketDataNames.push_back(attributeValues[i].getValueAsString());
				}
				else
				{
					{ std::ostringstream aqCoreMsg22;
aqCoreMsg22 << "Unsupported Data Name: " << attributeNames[i] << " ."; AQ_THROW( aqCoreMsg22.str() ); }
				}
			}
		}
	}

	/* @brief Used to serialize an instance of this class
	* @param[out] the populated SchemaObject
	*/
	const SchemaObject MultiCurveObject::toSchemaObject() const
	{
		SchemaObject schemaObject(MULTICURVE, getRefToName());

		// First serialize out the parameters contained in the freeObject_
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();
		const int nSchemas = freeObject_.numberOfSchemas();
		for (int i = 0; i<nSchemas; i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema(propertyName);
			VariantMatrix infoBlock;
			std::for_each(columnIndices.cbegin(), columnIndices.cend(), [&infoBlock, &allDataView](const int idx)
			{
				infoBlock.push_back(allDataView[idx]);
			});

			std::vector<etrading::ContainedTypeEnum> types = boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE);
			schemaObject.setDataForSchema(propertyName, infoBlock, types);
		}

		// Add these dependencies as nested schema objects
		for (size_t i = 0; i < singleCurves_.size(); ++i)
		{
			SingleCurveObjectPtr curve = singleCurves_[i];
			CurveGeneratorPtr curveGeneratorObj = curve->getCurveGeneratorObj();
			CurveMarketDataPtr curveMarketObj = curve->getCurveMarketDataObj();
			schemaObject.addNestedSchemaObject(curveGeneratorObj->toSchemaObject());
			schemaObject.addNestedSchemaObject(curveMarketObj->toSchemaObject());
		}

		return schemaObject;
	}

	/* @brief	Helper function which returns the cached CurveIndexNames of all the component curves
	*           The CurveIndex names are populated from the curve generator conventions
	* @returns	The curveIndexNames
	*/
	const AQLStringVector& MultiCurveObject::getCurveIndexNames() const
	{
		return componentCurveIndexNames_;
	}

}
