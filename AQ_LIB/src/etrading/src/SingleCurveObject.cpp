#include "SingleCurveObject.h"
#include "ParameterValidation.h"    // etrading::getDataInstance()
#include "CommonConstants.h"
#include "ObjectUtilities.h"
#include "LAUpdateStaticDataManager.h"
#include "LWOUtilities.h"
#include "CreateDataFile.h"
#include "ExceptionMacros.h"
#include "Variant.h"
#include "CurveEngine.h"

#include <vector>
#include <string>
#include <boost/format.hpp>


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
		attributeValues.push_back( lwoCurveGeneratorName_ );
		attributeValues.push_back( lwoCurveMarketDataName_ );
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
	*  @param[out]	objectName				The LWO handle name for the curve object
	*  @param[out]	lwoCurveGeneratorName	The curve generator containing the conventions used to build this curve
	*  @param[out]	lwoCurveMarketDataName	The LWO handle of the object containing the market data for this curve
	*  @param[out]	domesticCurveCollection	The collection which this curve will be placed in, once built
	*  @param[out]	foreignCurveCollection	The collection containing foreign curve dependencies
	*/
	void SingleCurveObject::getBuildParametersFromFreeObject( std::string& objectName,
														      std::string& lwoCurveGeneratorName,
														      std::string& lwoCurveMarketDataName,
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
			throw LACoreInvalidData( ( boost::format( "#Error: Invalid number of columns in deserialised data. Expecting 2 columns, found '%d'. " ) % variantMatrix.size() ).str().c_str() , __FILE__, __LINE__ );
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
				lwoCurveGeneratorName = attributeValues[i].getValueAsString();
			}
			else if ( attributeNames[i] == CURVE_MARKET_CALIBRATION_DATAAME )
			{
				lwoCurveMarketDataName = attributeValues[i].getValueAsString();
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
				throw LACoreInvalidData( ( boost::format( "#Error: Unsupported Data Name: %s ." )  
						% attributeNames[i] ).str().c_str(), __FILE__, __LINE__ );
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
		        const std::shared_ptr<OISCurveObjectData> oisCurveData = std::make_shared<OISCurveObjectData>( lwoCurveGenerator_, lwoCurveMarketData_, marketDataAsOfDate );
                CurveEngine curveEngine( curveDescription, oisCurveData );
                break;
			}
			case etrading::ARR_CURVETYPE:
			{
				// Create ARR curve data object that supplies curve data for ARR curve build
				const std::shared_ptr<ARRCurveObjectData> arrCurveData = std::make_shared<ARRCurveObjectData>(lwoCurveGenerator_, lwoCurveMarketData_, marketDataAsOfDate);
				CurveEngine curveEngine( curveDescription, arrCurveData );
				break;
			}
			case etrading::SWAP_CURVETYPE:
			{
				// Create Swap curve data object that supplies curve data for swap curve build
				const std::shared_ptr<SwapCurveObjectData> swapCurveData = std::make_shared<SwapCurveObjectData>(lwoCurveGenerator_, lwoCurveMarketData_, configFrequency, marketDataAsOfDate);
                CurveEngine curveEngine( curveDescription, swapCurveData );
                break;
			}
			case etrading::TENORBASIS_CURVETYPE:
			{
				// Create tenor basis curve data object that supplies curve data for tenor basis curve build
				const std::shared_ptr<TenorBasisCurveObjectData> tenorBasisCurveData = std::make_shared<TenorBasisCurveObjectData>(lwoCurveGenerator_, lwoCurveMarketData_, configFrequency, marketDataAsOfDate, domesticCurveCollection_, foreignCurveCollection_);
                CurveEngine curveEngine( curveDescription, tenorBasisCurveData );
                break;
			}
			case etrading::XCCYBASIS_CURVETYPE:
			{		
				// Create xccy basis curve data object that supplies curve data for xccy basis curve build
				const std::shared_ptr<XccyBasisCurveObjectData> xccyBasisCurveData = std::make_shared<XccyBasisCurveObjectData>(lwoCurveGenerator_, lwoCurveMarketData_, marketDataAsOfDate, domesticCurveCollection_, foreignCurveCollection_);
                CurveEngine curveEngine( curveDescription, xccyBasisCurveData );
                break;
			}
			case etrading::FWDFXCONST_CURVETYPE:
			{		
				// Create forward constant curve data object that supplies curve data for forward constant curve build
				const std::shared_ptr<FwdConstantCurveObjectData> fxFwdConstantCurveData = std::make_shared<FwdConstantCurveObjectData>(lwoCurveGenerator_, marketDataAsOfDate, domesticCurveCollection_, foreignCurveCollection_);
                CurveEngine curveEngine( curveDescription, fxFwdConstantCurveData );
                break;
			}
			default:
				throw LACoreInvalidData( ( boost::format( "#Error: Unsupported CurveType: %s ." ) % configCurveType ).str().c_str(), __FILE__, __LINE__ );
		}

		// Throw exception if the curve has not been built.
		etrading::getCurveStaticDataTableName( curveDescription->curveCollection().c_str(), curveDescription->curveIndexList().c_str() );

		if ( curveTypeEnum != etrading::FWDFXCONST_CURVETYPE )
		{
			// Check that the curve is also available via the StaticDataTable lookup
			etrading::getCurveStaticDataTableName( curveDescription->curveCollection().c_str(), curveDescription->objectPoolLookupTable().c_str() );
		}

	}


	/* @brief Main Constructor of the LWO Curve object
	*  @param[in]	objectName				The LWO handle name for the curve object
	*  @param[in]	lwoCurveGeneratorName	The curve generator containing the conventions used to build this curve
	*  @param[in]	lwoCurveMarketDataName	The LWO handle of the object containing the market data for this curve
	*  @param[in]	domesticCurveCollection	The collection which this curve will be placed in, once built   ( The Target CurveCollection )
	*  @param[in]	foreignCurveCollection	The collection containing foreign curve dependencies   ( The Against CurveCollection )
	*/
	SingleCurveObject::SingleCurveObject( const std::string& objectName,
							const std::string& lwoCurveGeneratorName,
							const std::string& lwoCurveMarketDataName,
							const std::string& domesticCurveCollection,
							const std::string& foreignCurveCollection ) 
								: CurveObject(objectName), 
								  freeObject_(objectName),
								  objectName_(objectName),
								  lwoCurveGeneratorName_(lwoCurveGeneratorName),
								  lwoCurveMarketDataName_(lwoCurveMarketDataName),
								  domesticCurveCollection_(domesticCurveCollection),
								  foreignCurveCollection_(foreignCurveCollection)
	{
		lwoCurveGenerator_  = getCurveGenerator(  lwoCurveGeneratorName_ );
		lwoCurveMarketData_ = getCurveMarketData( lwoCurveMarketDataName_ );

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
		getBuildParametersFromFreeObject( tmpObjectName, lwoCurveGeneratorName_, lwoCurveMarketDataName_, domesticCurveCollection_, foreignCurveCollection_ );

		if ( tmpObjectName != objectName )
		{
			throw LACoreInvalidData( ( boost::format( "#Error: Inconsistent data when deserializing curve: Object handle name is '%s' while CurveData contains '%s' ." )  
						% objectName % tmpObjectName ).str().c_str(), __FILE__, __LINE__ );	
		}
		
		lwoCurveGenerator_  = getCurveGenerator(  lwoCurveGeneratorName_ );
		lwoCurveMarketData_ = getCurveMarketData( lwoCurveMarketDataName_ );
    }

	/* @brief Copy Constructor
	 */
	SingleCurveObject::SingleCurveObject(const SingleCurveObject& rhs) 
		: CurveObject(rhs),
		  freeObject_(rhs.freeObject_), 
		  cachedCurveIndexName_(rhs.cachedCurveIndexName_),
		  objectName_(rhs.objectName_),
		  lwoCurveGeneratorName_(rhs.lwoCurveGeneratorName_),
		  lwoCurveMarketDataName_(rhs.lwoCurveMarketDataName_),
		  domesticCurveCollection_(rhs.domesticCurveCollection_),
		  foreignCurveCollection_(rhs.foreignCurveCollection_),
          lwoCurveGenerator_(rhs.lwoCurveGenerator_),
		  lwoCurveMarketData_(rhs.lwoCurveMarketData_)
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
		schemaObject.addNestedSchemaObject( lwoCurveGenerator_->toSchemaObject() );
		schemaObject.addNestedSchemaObject( lwoCurveMarketData_->toSchemaObject() );

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
		return lwoCurveMarketData_;
	}

	/* @brief	Method that returns the curve generator object
	*  @returns	The curve generator object
	*/
	const CurveGeneratorPtr& SingleCurveObject::getCurveGeneratorObj() const
	{
		return lwoCurveGenerator_;
	}

	/* @brief	Method that sets the market data object back to the curve
	* @param[in] objectName    The name of this CurveMarketData instance
	*/
	void SingleCurveObject::setCurveMarketDataObj(const CurveMarketDataPtr& marketObj)
	{
		lwoCurveMarketData_ = marketObj;
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
		const LabelValueBlock curvePropertiesLVB        = lwoCurveGenerator_->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_CURVEPROPERTIES );
		const std::string configCurrency                = curvePropertiesLVB.getCompulsoryValue( "Currency" );
		configCurveType                                 = curvePropertiesLVB.getCompulsoryValue( "CurveType" );
		configFrequency                                 = curvePropertiesLVB.getCompulsoryValue( "CurveIndexFrequency" );
		
		const LabelValueBlock marketDataPropertiesLVB   = lwoCurveMarketData_->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES );
		const std::string marketDataCurrency            = marketDataPropertiesLVB.getCompulsoryValue( "Currency" );
		const std::string marketDataCurveType           = marketDataPropertiesLVB.getCompulsoryValue( "CurveType" );
		const std::string marketDataFrequency           = marketDataPropertiesLVB.getCompulsoryValue( "CurveIndexFrequency" );
		marketDataAsOfDate                              = marketDataPropertiesLVB.getCompulsoryValue( "AsOfDate" );

		if ( configCurrency != marketDataCurrency )
		{
			throw LACoreInvalidData( ( boost::format("#Error: CurveGenerator currency \"%s\" does not match MarketData Currency \"%s\"" )
                                   % configCurrency % marketDataCurrency ).str().c_str(), __FILE__, __LINE__ );
		}

		if ( configCurveType != marketDataCurveType )
		{
			throw LACoreInvalidData( ( boost::format("#Error: CurveGenerator CurveType \"%s\" does not match MarketData CurveType \"%s\"" )
                                   % configCurveType % marketDataCurveType ).str().c_str(), __FILE__, __LINE__ );
		}

		if ( configFrequency != marketDataFrequency )
		{
			throw LACoreInvalidData( ( boost::format("#Error: CurveGenerator Frequency \"%s\" does not match MarketData Frequency \"%s\"" )
                                   % configFrequency % marketDataFrequency ).str().c_str(), __FILE__, __LINE__ );
		}

		curveName = curvePropertiesLVB.getCompulsoryValue( "StaticDataTable" );
		curveCollection = domesticCurveCollection_;
		std::string curveIndexStdStr = curvePropertiesLVB.getCompulsoryValue( "IndexName" );

		// Prefix the staticDataTable onto the curveIndex Name, using the ':' delimiter by default
        cachedCurveIndexName_ = etrading::addPrefixStringAndCheckForDuplicates( curveIndexStdStr, curveName );
		curveIndexes = cachedCurveIndexName_.c_str();
	}	
}
