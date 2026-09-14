#include "SingleCurveObject.h"
#include "ParameterValidation.h"    // etrading::getDataInstance()
#include "CommonConstants.h"
#include "ObjectUtilities.h"
#include "AQLUpdateStaticDataManager.h"
#include "AQObjUtilities.h"
#include "CreateDataFile.h"
#include "ExceptionMacros.h"
#include "Variant.h"
#include "CurveEngine.h"

#include <vector>
#include <string>


namespace etrading
{
	/* @brief	Given a set of curve-build parameters, populates the internal FreeObject member data of this class
	*/
	void SingleCurveObject::populateFreeObjectFromBuildParameters()
	{
		std::vector<std::string> columnNames;
		columnNames.push_back( DATA_NAMES );
		columnNames.push_back( DATA_VALUES );

		std::vector<ContainedTypeEnum> colTypes;
		colTypes.push_back( STRING_VALUE );
		colTypes.push_back( STRING_VALUE );

		VariantVector attributeNames;
		attributeNames.push_back( OBJECT_NAME );
		attributeNames.push_back( CURVE_GENERATOR_NAME );
		attributeNames.push_back( CURVE_MARKET_CALIBRATION_DATAAME );
		attributeNames.push_back( DOMESTIC_CURVE_COLLECTION );
		attributeNames.push_back( FOREIGN_CURVE_COLLECTION );

		VariantVector attributeValues;
		attributeValues.push_back( objectName_ );
		attributeValues.push_back( aqObjCurveGeneratorName_ );
		attributeValues.push_back( aqObjCurveMarketDataName_ );
		attributeValues.push_back( domesticCurveCollection_ );
		attributeValues.push_back( foreignCurveCollection_ );

		VariantMatrix rangeData;
		rangeData.push_back( attributeNames );
		rangeData.push_back( attributeValues );

		std::string schemaName = CURVE_BUILD_SCHEMA_NAME;
		const bool allowJaggedData = false;

		freeObject_.clearAll();
		freeObject_ += createFreeObjectFromGrid( objectName_, columnNames, colTypes, rangeData, schemaName, allowJaggedData );
	}

	/* @brief	Converts the internal FreeObject into a group of string parameters.
	*			Used when de-serialising the curve.
	*  @param[out]	objectName				The AQObj object handle name for the curve object
	*  @param[out]	aqObjCurveGeneratorName	The curve generator containing the conventions used to build this curve
	*  @param[out]	aqObjCurveMarketDataName	The AQObj object handle of the object containing the market data for this curve
	*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
	*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
	*/
	void SingleCurveObject::getBuildParametersFromFreeObject( std::string& objectName,
														      std::string& aqObjCurveGeneratorName,
														      std::string& aqObjCurveMarketDataName,
														      std::string& domesticCurveCollection,
														      std::string& foreignCurveCollection ) const
	{
		// Get the curve build parameters
        const etrading::VariantMatrix& allDataView = freeObject_.viewAllData();
        const auto columnIndices = freeObject_.columnsOfSchema( CURVE_BUILD_SCHEMA_NAME );
        etrading::VariantMatrix variantMatrix;
        std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
        {
            variantMatrix.push_back( allDataView[ idx ] );
        } );


		if ( variantMatrix.size() != 2)
		{
			{ std::ostringstream aqCoreMsg26;
aqCoreMsg26 << "Invalid number of columns in deserialised data. Expecting 2 columns, found '" << variantMatrix.size() << "'. "; AQ_THROW( aqCoreMsg26.str() ); }
		}

		const VariantVector& attributeNames  = variantMatrix[0];
		const VariantVector& attributeValues = variantMatrix[1];

		const size_t numRows = variantMatrix[0].size();
		for (size_t i=0; i<numRows; i++)
		{
			if ( attributeNames[i] == OBJECT_NAME )
			{
				objectName = attributeValues[i].getValueAsString();
			}
			else if ( attributeNames[i] == CURVE_GENERATOR_NAME )
			{
				aqObjCurveGeneratorName = attributeValues[i].getValueAsString();
			}
			else if ( attributeNames[i] == CURVE_MARKET_CALIBRATION_DATAAME )
			{
				aqObjCurveMarketDataName = attributeValues[i].getValueAsString();
			}
			else if ( attributeNames[i] == DOMESTIC_CURVE_COLLECTION )
			{
				domesticCurveCollection = attributeValues[i].getValueAsString();
			}
			else if ( attributeNames[i] == FOREIGN_CURVE_COLLECTION )
			{
				foreignCurveCollection = attributeValues[i].getValueAsString();
			}
			else
			{
				{ std::ostringstream aqCoreMsg27;
aqCoreMsg27 << "Unsupported Data Name: " << attributeNames[i] << " ."; AQ_THROW( aqCoreMsg27.str() ); }
			}
		}
	}

	CurveCalibrationTypeEnum SingleCurveObject::curveCalibrationType() const
	{
		return etrading::SINGLE_CURVE_CALIBRATION;
	}

	/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
	*/
	void SingleCurveObject::calibrateCurve()
	{
		// Retrieve a list of key curve info that identifies this curve
		std::string curveCollectionName; 
		std::string staticDataTable;
		std::string curveIndex;
		std::string configCurveType;
		std::string configFrequency;
		std::string marketDataAsOfDate;

		getCurveIdentifierInfos( curveCollectionName, 
								 staticDataTable,
								 curveIndex,
								 configCurveType,
								 configFrequency,
								 marketDataAsOfDate);
		
        const CurveTypeEnum curveTypeEnum   = toCurveTypeEnum( configCurveType );
		const FrequencyEnum frequencyEnum   = toFrequencyEnum( configFrequency );
        const CurveTenorEnum curveTenorEnum = fromFrequencyEnumtoCurveTenorEnum( frequencyEnum );
        
        // Curve Description Object
        const std::shared_ptr<CurveDescription> curveDescription = std::make_shared<CurveDescription>( curveTypeEnum, curveTenorEnum, curveCollectionName, curveIndex, staticDataTable );

        switch ( curveTypeEnum )
		{
			case etrading::OIS_CURVETYPE:
			{
				// Create OIS curve data object that supplies curve data for OIS curve build
		        const std::shared_ptr<OISCurveObjectData> oisCurveData = std::make_shared<OISCurveObjectData>( aqObjCurveGenerator_, aqObjCurveMarketData_, marketDataAsOfDate );
                CurveEngine curveEngine( curveDescription, oisCurveData );
                break;
			}
			case etrading::ARR_CURVETYPE:
			{
				// Create ARR curve data object that supplies curve data for ARR curve build
				const std::shared_ptr<ARRCurveObjectData> arrCurveData = std::make_shared<ARRCurveObjectData>(aqObjCurveGenerator_, aqObjCurveMarketData_, marketDataAsOfDate);
				CurveEngine curveEngine( curveDescription, arrCurveData );
				break;
			}
			case etrading::SWAP_CURVETYPE:
			{
				// Create Swap curve data object that supplies curve data for swap curve build
				const std::shared_ptr<SwapCurveObjectData> swapCurveData = std::make_shared<SwapCurveObjectData>(aqObjCurveGenerator_, aqObjCurveMarketData_, configFrequency, marketDataAsOfDate);
                CurveEngine curveEngine( curveDescription, swapCurveData );
                break;
			}
			case etrading::TENORBASIS_CURVETYPE:
			{
				// Create tenor basis curve data object that supplies curve data for tenor basis curve build
				const std::shared_ptr<TenorBasisCurveObjectData> tenorBasisCurveData = std::make_shared<TenorBasisCurveObjectData>(aqObjCurveGenerator_, aqObjCurveMarketData_, configFrequency, marketDataAsOfDate, domesticCurveCollection_, foreignCurveCollection_);
                CurveEngine curveEngine( curveDescription, tenorBasisCurveData );
                break;
			}
			case etrading::XCCYBASIS_CURVETYPE:
			{		
				// Create xccy basis curve data object that supplies curve data for xccy basis curve build
				const std::shared_ptr<XccyBasisCurveObjectData> xccyBasisCurveData = std::make_shared<XccyBasisCurveObjectData>(aqObjCurveGenerator_, aqObjCurveMarketData_, marketDataAsOfDate, domesticCurveCollection_, foreignCurveCollection_);
                CurveEngine curveEngine( curveDescription, xccyBasisCurveData );
                break;
			}
			case etrading::FWDFXCONST_CURVETYPE:
			{		
				// Create forward constant curve data object that supplies curve data for forward constant curve build
				const std::shared_ptr<FwdConstantCurveObjectData> fxFwdConstantCurveData = std::make_shared<FwdConstantCurveObjectData>(aqObjCurveGenerator_, marketDataAsOfDate, domesticCurveCollection_, foreignCurveCollection_);
                CurveEngine curveEngine( curveDescription, fxFwdConstantCurveData );
                break;
			}
			default:
				{ std::ostringstream aqCoreMsg28;
aqCoreMsg28 << "Unsupported CurveType: " << configCurveType << " ."; AQ_THROW( aqCoreMsg28.str() ); }
		}

		// Throw exception if the curve has not been built.
		etrading::getCurveStaticDataTableName( curveDescription->curveCollection().c_str(), curveDescription->curveIndexList().c_str() );

		if ( curveTypeEnum != etrading::FWDFXCONST_CURVETYPE )
		{
			// Check that the curve is also available via the StaticDataTable lookup
			etrading::getCurveStaticDataTableName( curveDescription->curveCollection().c_str(), curveDescription->objectPoolLookupTable().c_str() );
		}

	}


	/* @brief Main Constructor of the AQObj Curve object
	*  @param[in]	objectName				The AQObj object handle name for the curve object
	*  @param[in]	aqObjCurveGeneratorName	The curve generator containing the conventions used to build this curve
	*  @param[in]	aqObjCurveMarketDataName	The AQObj object handle of the object containing the market data for this curve
	*  @param[in]	domesticCurveCollection	The collection which this curve will be placed in, once built   ( The Target CurveCollection )
	*  @param[in]	foreignCurveCollection	The collection containing foreign curve dependencies   ( The Against CurveCollection )
	*/
	SingleCurveObject::SingleCurveObject( const std::string& objectName,
							const std::string& aqObjCurveGeneratorName,
							const std::string& aqObjCurveMarketDataName,
							const std::string& domesticCurveCollection,
							const std::string& foreignCurveCollection ) 
								: CurveObject(objectName), 
								  freeObject_(objectName),
								  objectName_(objectName),
								  aqObjCurveGeneratorName_(aqObjCurveGeneratorName),
								  aqObjCurveMarketDataName_(aqObjCurveMarketDataName),
								  domesticCurveCollection_(domesticCurveCollection),
								  foreignCurveCollection_(foreignCurveCollection)
	{
		aqObjCurveGenerator_  = getCurveGenerator(  aqObjCurveGeneratorName_ );
		aqObjCurveMarketData_ = getCurveMarketData( aqObjCurveMarketDataName_ );

		populateFreeObjectFromBuildParameters();
	}


	/* @brief Constructor used by deserialization
	* @param[in] objectName    The name of this SingleCurveObject instance
	* @param[in] freeObject    A freeObject constructed from the serialized data
	*/
	SingleCurveObject::SingleCurveObject( const std::string& objectName, const FreeObject& freeObject ) 
										: CurveObject(objectName), 
										  freeObject_(freeObject),
										  objectName_(objectName)
	{
		std::string tmpObjectName;
		getBuildParametersFromFreeObject( tmpObjectName, aqObjCurveGeneratorName_, aqObjCurveMarketDataName_, domesticCurveCollection_, foreignCurveCollection_ );

		if ( tmpObjectName != objectName )
		{
			{ std::ostringstream aqCoreMsg29;
aqCoreMsg29 << "Inconsistent data when deserializing curve: Object handle name is '" << objectName << "' while CurveData contains '" << tmpObjectName << "' ."; AQ_THROW( aqCoreMsg29.str() ); }	
		}
		
		aqObjCurveGenerator_  = getCurveGenerator(  aqObjCurveGeneratorName_ );
		aqObjCurveMarketData_ = getCurveMarketData( aqObjCurveMarketDataName_ );
    }

	/* @brief Copy Constructor
	 */
	SingleCurveObject::SingleCurveObject(const SingleCurveObject& rhs) 
		: CurveObject(rhs),
		  freeObject_(rhs.freeObject_), 
		  cachedCurveIndexName_(rhs.cachedCurveIndexName_),
		  objectName_(rhs.objectName_),
		  aqObjCurveGeneratorName_(rhs.aqObjCurveGeneratorName_),
		  aqObjCurveMarketDataName_(rhs.aqObjCurveMarketDataName_),
		  domesticCurveCollection_(rhs.domesticCurveCollection_),
		  foreignCurveCollection_(rhs.foreignCurveCollection_),
          aqObjCurveGenerator_(rhs.aqObjCurveGenerator_),
		  aqObjCurveMarketData_(rhs.aqObjCurveMarketData_)
	{
		// The copy constructor does not need to invoke SingleCurveObject::calibrateCurve() :
		// The curve will have already been built by the rhs object.
	}

	/* @brief Used to serialize an instance of this class
	* @param[out] the populated SchemaObject
	*/
	const SchemaObject SingleCurveObject::toSchemaObject() const
	{
		SchemaObject schemaObject( CURVE, getRefToName());
        
		// First serialize out the parameters contained in the freeObject_
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();
		const int nSchemas = freeObject_.numberOfSchemas();
		for (int i=0; i<nSchemas; i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix infoBlock;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&infoBlock, &allDataView]( const int idx )
            {
				infoBlock.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, infoBlock);
		}
		
		// Add these dependencies as nested schema objects
		schemaObject.addNestedSchemaObject( aqObjCurveGenerator_->toSchemaObject() );
		schemaObject.addNestedSchemaObject( aqObjCurveMarketData_->toSchemaObject() );

		return schemaObject;
	}

	/* @brief		Helper function which returns the cached CurveIndexName of this curve
	 *				The CurveIndex name is populated from the curve generator conventions
	 * @returns		The curveIndexName
	 */
	const std::string& SingleCurveObject::getCurveIndexName() const
	{
		return cachedCurveIndexName_;
	}

	/* @brief	Method that gets the market data object of the curve
	*  @returns	The underlying market data object
	*/
	const CurveMarketDataPtr& SingleCurveObject::getCurveMarketDataObj() const
	{
		return aqObjCurveMarketData_;
	}

	/* @brief	Method that returns the curve generator object
	*  @returns	The curve generator object
	*/
	const CurveGeneratorPtr& SingleCurveObject::getCurveGeneratorObj() const
	{
		return aqObjCurveGenerator_;
	}

	/* @brief	Method that sets the market data object back to the curve
	* @param[in] objectName    The name of this CurveMarketData instance
	*/
	void SingleCurveObject::setCurveMarketDataObj(const CurveMarketDataPtr& marketObj)
	{
		aqObjCurveMarketData_ = marketObj;
	}

	/* @brief	Return a series of information that uniquely identify a curve
	*  @param[out]		curveCollection		Curve collection name
	*  @param[out]		curveName			Curve name (or static data table name)
	*  @param[out]		curveIndexes		Curve index names
	*  @param[out]		configCurveType		Curve type
	*  @param[out]		configFrequency		Curve frequency
	*  @param[out]		marketDataAsOfDate	As of date
	*/
	void SingleCurveObject::getCurveIdentifierInfos( std::string& curveCollection, 
													 std::string& curveName, 
													 std::string& curveIndexes,
													 std::string& configCurveType,
													 std::string& configFrequency,
													 std::string& marketDataAsOfDate )
	{
		// Check for matching IdentityParams from the CurveGenerator and CurveMarketData
		const LabelValueBlock curvePropertiesLVB        = aqObjCurveGenerator_->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_CURVEPROPERTIES );
		const std::string configCurrency                = curvePropertiesLVB.getCompulsoryValue( "Currency" );
		configCurveType                                 = curvePropertiesLVB.getCompulsoryValue( "CurveType" );
		configFrequency                                 = curvePropertiesLVB.getCompulsoryValue( "CurveIndexFrequency" );
		
		const LabelValueBlock marketDataPropertiesLVB   = aqObjCurveMarketData_->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES );
		const std::string marketDataCurrency            = marketDataPropertiesLVB.getCompulsoryValue( "Currency" );
		const std::string marketDataCurveType           = marketDataPropertiesLVB.getCompulsoryValue( "CurveType" );
		const std::string marketDataFrequency           = marketDataPropertiesLVB.getCompulsoryValue( "CurveIndexFrequency" );
		marketDataAsOfDate                              = marketDataPropertiesLVB.getCompulsoryValue( "AsOfDate" );

		if ( configCurrency != marketDataCurrency )
		{
			{ std::ostringstream aqCoreMsg30;
aqCoreMsg30 << "CurveGenerator currency \"" << configCurrency << "\" does not match MarketData Currency \"" << marketDataCurrency << "\""; AQ_THROW( aqCoreMsg30.str() ); }
		}

		if ( configCurveType != marketDataCurveType )
		{
			{ std::ostringstream aqCoreMsg31;
aqCoreMsg31 << "CurveGenerator CurveType \"" << configCurveType << "\" does not match MarketData CurveType \"" << marketDataCurveType << "\""; AQ_THROW( aqCoreMsg31.str() ); }
		}

		if ( configFrequency != marketDataFrequency )
		{
			{ std::ostringstream aqCoreMsg32;
aqCoreMsg32 << "CurveGenerator Frequency \"" << configFrequency << "\" does not match MarketData Frequency \"" << marketDataFrequency << "\""; AQ_THROW( aqCoreMsg32.str() ); }
		}

		curveName = curvePropertiesLVB.getCompulsoryValue( "StaticDataTable" );
		curveCollection = domesticCurveCollection_;
		std::string curveIndexStdStr = curvePropertiesLVB.getCompulsoryValue( "IndexName" );

		// Prefix the staticDataTable onto the curveIndex Name, using the ':' delimiter by default
        cachedCurveIndexName_ = etrading::addPrefixStringAndCheckForDuplicates( curveIndexStdStr, curveName );
		curveIndexes = cachedCurveIndexName_.c_str();
	}	
}
