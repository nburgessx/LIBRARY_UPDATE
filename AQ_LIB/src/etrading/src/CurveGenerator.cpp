#include "CurveGenerator.h"
#include "ParameterValidation.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"

#include <vector>
#include <string>
#include <boost/format.hpp>
#include <boost/foreach.hpp>


namespace etrading
{
	/* @brief Main Constructor
	 * @param[in] objectName    The name of this CurveGenerator instance
	 * @param[in] propertyKeys  A vector containing the names of each configuration block
	 * @param[in] infoBlocks    A vector containing the configuation blocks
	 */
	CurveGenerator::CurveGenerator( const std::string& objectName,
                                    const std::vector<std::string>& propertyKeys,
                                    const std::vector<TableInfo>& infoBlocks ) 
						: IsAQObject(objectName, CURVE_GENERATOR), 
						  freeObject_(objectName),
						  variantMatrixByKey_(),
						  stringMatrixByKey_()
	{

		// Create a FreeObject from each property label-value block, and concatenate to our FreeObject data member
		const bool allowJaggedData = false;
        for( unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++ )
        {
            const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const VariantMatrix& rangeData = std::get<2>( infoBlocks[ gridCounter ] );
            freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, propertyKeys[ gridCounter ], allowJaggedData );
        }

		// Verify that all CurveGenerator properties are valid and expected
		validateKeys();
	}

	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this CurveGenerator instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	CurveGenerator::CurveGenerator( const std::string& objectName, const FreeObject& freeObject ) 
					: IsAQObject(objectName, CURVE_GENERATOR), 
					  freeObject_(freeObject),
					  variantMatrixByKey_(),
				      stringMatrixByKey_()
	{
    }

	/* @brief Copy Constructor
	 */
	CurveGenerator::CurveGenerator(const CurveGenerator& rhs) 
		: IsAQObject(rhs.getRefToName(), CURVE_GENERATOR), 
		  freeObject_(rhs.freeObject_),
		  variantMatrixByKey_(rhs.variantMatrixByKey_),
	      stringMatrixByKey_(rhs.stringMatrixByKey_)
	{
	}

	std::shared_ptr<CurveGenerator> CurveGenerator::clone() const
    {
       auto data = std::make_shared<CurveGenerator>(*this);
       return data;
    }

	/* @brief Builds a new CurveGenerator from an existing base CurveGenerator, with additional modified properties
	*         This method may be viewed as a specialized type of copy constructor.
	*  @param[in] newObjectName			The AQObj object cache handle name to use for the new object
	*  @param[in] baseCurveGenerator	The CurveGenerator object on which to base the properties of this new object
	*  @param[in] modifiedValuesLVB		A LabelValueBlock containing key/value pairs to update. Each key is specified in the format: PROPERTYNAME:KEYNAME.
	*/
	CurveGenerator::CurveGenerator(const std::string& newObjectName, const CurveGenerator& baseCurveGenerator, const LabelValueBlock& modifiedValuesLVB) 
		: IsAQObject(newObjectName, CURVE_GENERATOR), 
		  freeObject_(newObjectName),
		  variantMatrixByKey_(),
	      stringMatrixByKey_()
	{
		/* A searchable index of keys/values for each property block in the baseCurveGenerator. 
		* The map is indexed by property name. This allows convenient lookup of property values
		*/
		std::unordered_map< std::string, std::unordered_map<std::string, Variant > > propertyIndex;

		/* Stores the union of baseCurveGenerator properties and additional keys/values.
		 * Indexed by propertyName. Used to construct the new CurveGenerator. 
		 */
		std::unordered_map< std::string, VariantMatrix> mergedProperties;

		mergePropertiesFromLVB( baseCurveGenerator, modifiedValuesLVB, propertyIndex, mergedProperties );
		
		/*
		* Construct a FreeObject from each property key-value block, and concatenate to our FreeObject data member
		*/
		const FreeObject& baseFreeObject = baseCurveGenerator.freeObject_;
		const std::vector<std::string>& propertyNames = baseFreeObject.keyNames();

		for (size_t i=0; i < baseFreeObject.numberOfSchemas(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			VariantMatrix& propertyValues = mergedProperties[ propertyName ];
			VariantVector& propertyKeys   = propertyValues[0];
			VariantVector& propertyData   = propertyValues[1];

			// Search the property index for any updated values
			auto keyIndex = propertyIndex[ propertyName ];
			for (size_t row = 0; row < propertyKeys.size(); row++)
			{
				auto keyName = trim_to_upper( propertyKeys[ row ].toString().c_str() );
				if ( keyIndex.find( keyName ) != keyIndex.end() )
				{
					// The propertyIndex contains a potentially updated key/value. Use this updated value
					propertyData[ row ] = keyIndex[ keyName ];
				}
			}

			const DataSchema& baseDataSchema = baseFreeObject.viewSchema( i );
			const std::vector<std::string>& columnNames = baseDataSchema.getColumnNames();
			const std::vector<etrading::ContainedTypeEnum>& colTypes = Variant::getContainedTypeInfo( propertyValues );
			freeObject_ += createFreeObjectFromGrid( newObjectName, columnNames, colTypes, propertyValues, propertyName, false /* allowJaggedData */ );
		}

		// Verify that all CurveGenerator properties are valid and expected
		validateKeys();
	}
	
	/*
	*  @brief  Utility method which reads in a BaseCurveGenerator and modifiedValuesLVB, and constructs a merged set of properties and searchable index of base property keys as follows:
	*          If a key from modifiedValuesLVB exists in the BaseCurveGenerator, the property index is updated with the new value
	*          If this is a new key, the key-value pair is added directly to the mergedProperties
	*
	*  @param[in]	baseCurveGenerator	The input curveGenerator from which properties are extracted
	*  @param[in]	modifiedValuesLVB	A LabelValueBlock containing keys/values
	*  @param[out]	propertyIndex		A searcheable index of base property keys initialized from the baseCurveGenerator, updated from the modifiedValuesLVB.
	*  @param[out]	mergedProperties	Properties obtained from the baseCurveGenerator, merged with any additional keys/values from the LabelValueBlock that were not found in the baseCurveGenerator
	*                                   These merged properties are in a format convenient for constructing a new CurveGenerator.
	*/
	void CurveGenerator::mergePropertiesFromLVB( const CurveGenerator& baseCurveGenerator,
												 const LabelValueBlock& modifiedValuesLVB,
												 std::unordered_map< std::string, std::unordered_map<std::string, Variant > >& propertyIndex,
												 std::unordered_map< std::string, VariantMatrix>& mergedProperties ) const
	{	
		// Initialize the searchable property index and mergedProperties from the baseCurveGenerator
		constructPropertyIndexFromCurveGenerator( baseCurveGenerator, propertyIndex, mergedProperties );

		// Update property values using the modifiedValuesLVB
		const AQLStringMatrix& modifiedValues = modifiedValuesLVB.toAQLStringMatrix();
		BOOST_FOREACH( auto rowData, modifiedValues )
		{
			const AQLString& key      = rowData[0];
			const AQLString& newValue = rowData[1];

			// Split the key into two components delimited by ':'   requiredPropertyName:requiredKeyName
			AQLStringVector tokens = key.toToken(':');
            
            // Uppercase
            for( size_t i = 0; i < tokens.size(); ++i )
            {
                tokens[i].toUpper();
            }
			
            // Check Format
            if (tokens.size() != 2)
			{
				throw AQLCoreInvalidData( ( boost::format( "#Error: Invalid format of property key: Expecting 'PROPERTYNAME:KEYNAME', but found '%s'. " ) % key ).str().c_str() , __FILE__, __LINE__ );
			}
			
            const std::string requiredPropertyName = tokens[0].getCString();
            const std::string requiredKeyName      = tokens[1].getCString();

			// Search for the specified propertyName block
			if ( propertyIndex.find( requiredPropertyName ) != propertyIndex.end() )
			{
				// Search for the requiredKey within the key/value index
				std::unordered_map<std::string, Variant>& keyIndex = propertyIndex[ requiredPropertyName ];
				if ( keyIndex.find( requiredKeyName ) != keyIndex.end() )
				{
					// The baseCurveGenerator already has this key. Simply update the value in the index
					keyIndex[ requiredKeyName ] = newValue.getCString();
				}
				else
				{
					// This is a new key which was not present in the baseCurveGenerator.
					// Add this new key / value pair directly to the mergedProperties
					VariantMatrix& propertyValues = mergedProperties[ requiredPropertyName ];
					propertyValues[0].push_back( requiredKeyName );
					propertyValues[1].push_back( newValue );
				}
			}
			else
			{
				throw AQLCoreInvalidData( ( boost::format( "#Error: Base CurveGenerator '%s' does not contain property name '%s'. " ) 
					% baseCurveGenerator.getRefToName()
					% requiredPropertyName ).str().c_str() , __FILE__, __LINE__ );
			}
		}
	}


	/*
	*  @brief  Utility method which constructs a searchable index containing property keys from the specified baseCurveGenerator.
	*          This index allows convenient property lookup.
	*          The method also stores the property blocks from the baseCurveGenerator in a baseProperties map.
	*  @param[in]  baseCurveGenerator	The input curveGenerator from which properties are extracted
	*  @param[out] propertyIndex		A searchable index of property values, initialized from the baseCurveGenerator.
	*  @param[out] baseProperties		A map of VariantMatrix property data, initialized from the baseCurveGenerator. Convenient for creating a new CurveGenerator.
	*/
	void CurveGenerator::constructPropertyIndexFromCurveGenerator( const CurveGenerator& baseCurveGenerator,
																   std::unordered_map< std::string, std::unordered_map<std::string, Variant > >& propertyIndex,
																   std::unordered_map< std::string, VariantMatrix>& baseProperties ) const
	{
		/*
		* Extract the property values from the baseCurveGenerator and construct the propertyIndex
		*/
		const FreeObject& baseFreeObject                      = baseCurveGenerator.freeObject_;
		const std::vector<std::string>& propertyNames         = baseFreeObject.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = baseFreeObject.viewAllData();

		for (size_t i=0; i < baseFreeObject.numberOfSchemas(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			const auto columnIndices = baseFreeObject.columnsOfSchema( propertyName );
			VariantMatrix rangeData;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&rangeData, &allDataView]( const int idx )
			{
				rangeData.push_back( allDataView[ idx ] );
			} );

			// Initialize the mergedProperties with values from the baseCurveGenerator
			baseProperties [ propertyName ] = rangeData;

			/*
			* Construct an index of property values for easy lookup
			*/
			VariantVector& propertyKey   = rangeData[0];
			VariantVector& propertyValue = rangeData[1];

			std::unordered_map< std::string, Variant> keyIndex;
			for (size_t row=0; row< propertyKey.size(); row++)
			{
				const std::string& keyName = trim_to_upper( ( propertyKey[row].toString() ).c_str() );
				keyIndex[ keyName ] = propertyValue[ row ];
			}
			propertyIndex[ propertyName ] = keyIndex;
		}
	}

	/*
	*  @brief  validates the property keys of this curveGenerator, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void CurveGenerator::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for ( size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			LabelValueBlock inputLVB = toLabelValueBlock( propertyName );

			etrading::CurveGeneratorEnum curveGeneratorEnum = etrading::toCurveGeneratorEnum( propertyName );
			switch (curveGeneratorEnum)
			{
				case CURVE_PROPERTIES:
					validateKeysForLVB( curve_properties_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case OIS_CONVENTIONS:
					validateKeysForLVB( ois_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case LIBOR_OIS_BASIS_CONVENTIONS:
					validateKeysForLVB( libor_ois_basis_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;
				
				case SWAP_CONVENTIONS:
					validateKeysForLVB( swaps_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case MONEY_MARKET_CONVENTIONS:
					validateKeysForLVB( money_market_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case LIBOR_FIXING_CONVENTIONS:
					validateKeysForLVB( libor_fixings_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case FRA_CONVENTIONS:
					validateKeysForLVB( fras_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case FUTURES_CONVENTIONS:
					validateKeysForLVB( futures_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case BASIS_SWAP_CONVENTIONS:
				case XCCY_BASIS_CONVENTIONS:
					validateKeysForLVB( basisswaps_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case FXFWD_CONVENTIONS:
					validateKeysForLVB( fxfwds_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case FWDFXCONST_CONVENTIONS:
					validateKeysForLVB( fwdfxconst_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				default:
					break;
			}
		}

	}


	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject CurveGenerator::toSchemaObject() const
	{
		SchemaObject schemaObject( CURVE_GENERATOR, getRefToName());
        toSchemaObject(schemaObject);

        return schemaObject;
	}

    //This is a helper method to populate SchemaObject, which is not in the abstract class:
    void const CurveGenerator::toSchemaObject(SchemaObject& schemaObject) const
   	{
        
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for (size_t i=0; i < freeObject_.numberOfSchemas(); i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	const VariantMatrix CurveGenerator::viewInputParameters(const std::string& propertyKey ) const
	{
		// If no propertyKey is specified, Return a matrix containing all properties
		if (propertyKey == "" || propertyKey == "NONE")
		{
			VariantMatrix allParameters;

			bool addSeparatorRow = false;
			const std::vector<std::string> keyNames = freeObject_.keyNames();
			for (auto it = keyNames.begin(); it != keyNames.end(); ++it)
			{
				if ( addSeparatorRow )
				{
					std::vector<Variant> row(2, "");
					allParameters.push_back( row );
				}
				addSeparatorRow = true; // Add a separator before all but the first row

				std::string propertyKey = *it;
				std::vector<Variant> row(2, "");
				row[0] = propertyKey;
				allParameters.push_back( row );

				VariantMatrix variantMatrix = transpose( toVariantMatrix ( propertyKey ) );
				for (unsigned int i = 0; i < variantMatrix.size(); i++)
				{
					allParameters.push_back( variantMatrix[i] );
				}
			}
			return transpose( allParameters );
		}

		// extract just the data for the specified property
		VariantMatrix variantMatrix = toVariantMatrix( propertyKey );
		return variantMatrix;
	}

	/* @brief Returns a VariantMatrix containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A VariantMatrix containing the LabelValue block
	*/
	VariantMatrix CurveGenerator::toVariantMatrix( const std::string& propertyKey ) const
	{
		etrading::VariantMatrix variantMatrix;
		auto iter = variantMatrixByKey_.find(propertyKey);

		if (iter == variantMatrixByKey_.end())
		{
			// extract just the data for the specified property
			variantMatrix = getVariantMatrixFromFreeObject( freeObject_, propertyKey );
			variantMatrixByKey_[propertyKey] = variantMatrix;
		}
		else
		{
			variantMatrix = iter->second;
		}

		return variantMatrix;
	}

	/* @brief Returns a AQLStringMatrix containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [in]   trimBlankRows Whether to remove blank rows from the end of the AQLStringMatrix
	* @param [out]  A AQLStringMatrix containing the LabelValue block
	*/
	AQLStringMatrix CurveGenerator::toAQLStringMatrix( const std::string& propertyKey, const bool trimBlankRows ) const
	{
		AQLStringMatrix stringMatrix;
		auto iter = stringMatrixByKey_.find(propertyKey);
		
		if (iter == stringMatrixByKey_.end())
		{
			stringMatrix = getAQLStringMatrixFromFreeObject( freeObject_, propertyKey, trimBlankRows );
			stringMatrixByKey_[propertyKey] = stringMatrix;
		}
		else
		{
			stringMatrix = iter->second;
		}

		return stringMatrix;
	}

    /* @brief Returns a StandardStringMatrix containing the configuration information for the specified propertyKey
	 * @param [in]   propertyKey   The property to be displayed
	 * @param [in]   trimBlankRows Whether to remove blank rows from the end of the AQLStringMatrix
	 * @param [out]  A StandardStringMatrix containing the LabelValue block
	 */
	StandardStringMatrix CurveGenerator::toStandardStringMatrix( const std::string& propertyKey, const bool trimBlankRows ) const
    {
        const AQLStringMatrix laStringMatrix =  toAQLStringMatrix( propertyKey, trimBlankRows );
        const StandardStringMatrix standardStringMatrix = convertToStandardStringMatrix( laStringMatrix );
        return standardStringMatrix;
    }

    /* @brief Returns a LabelValueBlock containing the configuration information for the specified propertyKey
	 * @param [in]   propertyKey   The property to be displayed
	 * @param [in]   trimBlankRows Whether to remove blank rows from the end of the AQLStringMatrix
	 * @param [out]  A LabelValueBlock containing the LabelValue block
	 */
	LabelValueBlock CurveGenerator::toLabelValueBlock( const std::string& propertyKey, const bool trimBlankRows ) const
    {
        const AQLStringMatrix laStringMatrix =  toAQLStringMatrix( propertyKey, trimBlankRows );
        const LabelValueBlock LVB( laStringMatrix );
        return LVB;
    }


    /* @brief Returns a boolean to confirm if an optional property key has been specified
	 * @param [in]   propertyKey   The property to be displayed
	 * @param [out]  A bool to confirm if the optional property key is present
	 */
	const bool CurveGenerator::doesKeyExist( const std::string& propertyKey ) const
    {
        bool keyFound = true;

        // 1. FirstCheck for the propertyKey in the AQLStringMatrix Cache
        auto iter = stringMatrixByKey_.find( propertyKey );
		if (iter == stringMatrixByKey_.end())  // Not Found
        {
            // 2. Second Check for the propertyKey in the VariantMatrix Cache
		    auto iter = variantMatrixByKey_.find( propertyKey );
            if (iter == variantMatrixByKey_.end())
            {
                // 3. Thirdly Check for the propertyKey in the freeobject itself
                const etrading::VariantMatrix& allDataView = freeObject_.viewAllData();
                keyFound = freeObject_.doesKeyExist( propertyKey );
                return keyFound;
            }
        }
        
        return keyFound;
    }
}
