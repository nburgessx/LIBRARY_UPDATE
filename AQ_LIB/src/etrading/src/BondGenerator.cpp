#include "BondGenerator.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"

namespace etrading
{
	/* @brief Main Constructor
	 * @param[in] objectName    The name of this BondGenerator instance
	 * @param[in] propertyKeys  A vector containing the names of each configuration block
	 * @param[in] infoBlocks    A vector containing the configuation blocks
	 */
	BondGenerator::BondGenerator( const std::string& objectName,
                                    const std::vector<std::string>& propertyKeys,
                                    const std::vector<TableInfo>& infoBlocks ) 
											: IsLWOObject( objectName, BOND_GENERATOR ), 
											  freeObject_( objectName )
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

		// Verify that all BondGenerator properties are valid and expected
		validateKeys();
	}


	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this BondGenerator instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	BondGenerator::BondGenerator( const std::string& objectName, const FreeObject& freeObject ) 
											: IsLWOObject( objectName, BOND_GENERATOR ), 
											  freeObject_( freeObject )
	{
    }

	/* @brief Copy Constructor
	 */
	BondGenerator::BondGenerator( const BondGenerator& rhs )
									: IsLWOObject( rhs.getRefToName(), BOND_GENERATOR ), 
									  freeObject_( rhs.freeObject_ )
	{
	}

	/*
	*  @brief  validates the property keys of this bondenerator, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void BondGenerator::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for ( size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			LabelValueBlock inputLVB = toLabelValueBlock( propertyName );

			etrading::BondGeneratorEnum bondGeneratorEnum = etrading::toBondGeneratorEnum( propertyName );
			switch ( bondGeneratorEnum )
			{
				case BOND_STATICDATA:
					validateKeysForLVB( bond_staticdata_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				case BOND_SCHEDULE:
					validateKeysForLVB( bond_schedule_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				default:
					break;
			}
		}
	}

	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject BondGenerator::toSchemaObject() const
	{
		SchemaObject schemaObject( BOND_GENERATOR, getRefToName() );
        toSchemaObject( schemaObject );

        return schemaObject;
	}

    //This is a helper method to populate SchemaObject, which is not in the abstract class:
    void const BondGenerator::toSchemaObject( SchemaObject& schemaObject ) const
   	{
        
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for (size_t i=0; i < freeObject_.numberOfSchemas(); i++)
		{
			schemaObject.addDataSchema( freeObject_.viewSchema(i) );

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema( propertyName, variantMatrix );
		}
	}

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	const VariantMatrix BondGenerator::viewInputParameters( const std::string& propertyKey ) const
	{
		// If no propertyKey is specified, Return a matrix containing all properties
		if ( propertyKey == "" || propertyKey == "NONE" )
		{
			VariantMatrix allParameters;

			bool addSeparatorRow = false;
			const std::vector<std::string> keyNames = freeObject_.keyNames();
			for ( auto it = keyNames.begin(); it != keyNames.end(); ++it )
			{
				if ( addSeparatorRow )
				{
					std::vector<Variant> row( 2, "" );
					allParameters.push_back( row );
				}
				addSeparatorRow = true; // Add a separator before all but the first row

				std::string propertyKey = *it;
				std::vector<Variant> row( 2, "" );
				row[0] = propertyKey;
				allParameters.push_back( row );

				VariantMatrix variantMatrix = transpose( getVariantMatrixFromFreeObject ( freeObject_, propertyKey ) );
				for (unsigned int i = 0; i < variantMatrix.size(); i++)
				{
					allParameters.push_back( variantMatrix[i] );
				}
			}
			return transpose( allParameters );
		}

		// extract just the data for the specified property
		VariantMatrix variantMatrix = getVariantMatrixFromFreeObject ( freeObject_, propertyKey );
		return variantMatrix;
	}

   /* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A LabelValue block containing the properties
	*/
	LabelValueBlock BondGenerator::toLabelValueBlock( const std::string& propertyKey ) const
	{
		AQLStringMatrix stringMatrix = getLAStringMatrixFromFreeObject( freeObject_, propertyKey );
		LabelValueBlock lvb( stringMatrix );

		return lvb;
	}

}


