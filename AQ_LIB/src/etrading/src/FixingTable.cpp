#include "FixingTable.h"
#include "CommonConstants.h"

#include <boost/assign.hpp>

namespace etrading
{
	// Store the Fixing Table Parameter Names i.e. Currency and CurveTenor
	const std::vector< Variant > FixingTable::FixingParameterNames = boost::assign::list_of(Variant("CURRENCY"))(Variant("CURVE_TENOR"));

	FixingTable::FixingTable(const std::string& tableName)
		: curveTenor_(etrading::CURVE_TENOR_1D), currency_(etrading::EUR),
		TableDateDouble(tableName)
	{
	};

	/* @brief	Consructor specific to interest rate fixings
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		dates			    The fixing dates to store
	*  @param [in]		values				The fixing values to store
	*  @param [in]		curveTenor			The curve tenor corresponding to the interest rate fixings
	*  @param [in]		currency			The currency corresponding to the interest rate fixings
	*/
	FixingTable::FixingTable(const std::string& tableName,
		const std::vector<boost::gregorian::date>& dates,
		const std::vector<double>& values,
		const etrading::CurveTenorEnum curveTenor,
		const etrading::CCY currency)
		: curveTenor_(curveTenor), currency_(currency),
		TableDateDouble(tableName, dates, values)
	{	};

	/* @brief	Consructor which accepts a parameterLVB, and can be used for many fixing types
	*			e.g. INTERESTRATE, INFLATION, FX
	*
	*  @param [in]		tableName		    Fixing Table name
	*  @param [in]		dates			    The fixing dates to store
	*  @param [in]		values				The fixing values to store
	*  @param [in]		parameterLVB		A set of parameters specifying the fixing table type,
	*										and other parameters used for validation
	*/
	FixingTable::FixingTable( const std::string& tableName,
							  const std::vector<boost::gregorian::date>& dates,
							  const std::vector<double>& values,
							  const LabelValueBlock& parameterLVB )
							  : parameterLVB_( parameterLVB ),
								curveTenor_(), currency_(),
							  TableDateDouble( tableName, dates, values )
	{	
		const std::string lvbName = "parameterLVB";
		const FixingTableEnum fixingTableType = toFixingTableEnum( parameterLVB.getCompulsoryValueAsString(FIXING_TABLE_KEYS::TABLE_TYPE, lvbName ) );
		switch (fixingTableType)
		{
		case INTEREST_RATE_FIXING_TABLE:
		{
			currency_   = toCCYEnum(parameterLVB.getCompulsoryValueAsString(FIXING_TABLE_KEYS::CURRENCY, lvbName ));
			curveTenor_ = toCurveTenorEnum(parameterLVB.getCompulsoryValueAsString(FIXING_TABLE_KEYS::TENOR, lvbName ));
			break;
		}
		case INFLATION_FIXING_TABLE:
		{
			std::string inflationIndex = parameterLVB.getCompulsoryValueAsString( FIXING_TABLE_KEYS::INFLATION_INDEX, lvbName);
			currency_ = toCCYEnum( parameterLVB.getCompulsoryValueAsString(FIXING_TABLE_KEYS::CURRENCY, lvbName));
			break;
		}
		case FX_FIXING_TABLE:
		{
			std::string currencyPair = parameterLVB.getCompulsoryValueAsString(FIXING_TABLE_KEYS::CURRENCY_PAIR, lvbName );
			break;
		}
		default:
			AQ_THROW( "Unsupported FixingTableType" + toString( fixingTableType ) );
		}

	};


	std::shared_ptr<FixingTable> FixingTable::clone()
	{
		FixingTable temp(*this);
		return std::make_shared<FixingTable>(temp);
	}

	// These accessors kept for backwards compatibility and are specific to interest rate fixings
	const etrading::CCY FixingTable::getCurrency() const
	{
		return currency_;
	};
	const etrading::CurveTenorEnum FixingTable::getCurveTenor() const
	{
		return curveTenor_;
	};
	
	// Return the parameterLVB which describes this fixing table
	LabelValueBlock FixingTable::getParameterLVB() const
	{
		return parameterLVB_;
	}

	FixingTable& FixingTable::operator=(const FixingTable& fixingTable)
	{
		if (&fixingTable != this)
		{
			// Parameters used for fixing table validation
			currency_ = fixingTable.currency_;
			curveTenor_ = fixingTable.curveTenor_;
			parameterLVB_ = fixingTable.parameterLVB_;

			setDates(Variant::createValueVector<boost::gregorian::date>(fixingTable.viewDates()));
			setValues(Variant::createValueVector<double>(fixingTable.viewValues()));
		}
		return *this;
	};

	const SchemaObject FixingTable::toSchemaObject() const
	{
		SchemaObject schemaObject(FIXING_TABLE, getRefToName());
		schemaObject.setDataForSchema( FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE, TableDateDouble::viewAllData());

		// Set the Parameter Keys Column: Column 0
		schemaObject.setColumnData (FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE_PARAMETERS, 0, FixingParameterNames);

		// Set the Parameter Values Column: Column 1
		const std::vector< Variant > fixingParameterValues = boost::assign::list_of(Variant(toString(currency_)))(Variant(toString(curveTenor_)));
		schemaObject.setColumnData( FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE_PARAMETERS, 1, fixingParameterValues);

		// Add the parameterLVB as a dynamic schema
		toSchemaObject( schemaObject );

		return schemaObject;
	}

	/* @brief	Schema Helper, used for serialization: Appends a schema and data for the parameterLVB
	*  @param	schemaObject	The schema object created by the main public toSchemaObject.
	*							This schemaObject is modified to include the parameterLVB data
	*/
	void FixingTable::toSchemaObject( SchemaObject& schemaObject ) const
	{
		const std::string lvbSchemaName = FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE_PARAMETERLVB;

		//dynamic generate schema
		schemaObject.addDataSchema( generateDataSchemaForParameterLVB ( lvbSchemaName ) );

		//add data to schema
		schemaObject.setDataForSchemaWithMap( lvbSchemaName, getParameterLVBDataMap() );
	}

	/* @brief	Schema Helper, used for serialization: Creates a data schema suitable for storing the parameterLVB
	*  @param	schemaName	The name of the schema ( e.g. FIXING_TABLE_PARAMETER_LVB )
	*  @returns	A data schema object
	*/
	const DataSchema FixingTable::generateDataSchemaForParameterLVB( const std::string& schemaName ) const
	{
		const DataSchema  dynamicSchema( schemaName, 2, boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
														boost::assign::list_of(FIXING_TABLE_SERIALIZATION_KEYS::PARAMETER_NAME)
																			  (FIXING_TABLE_SERIALIZATION_KEYS::PARAMETER_VALUE) );

		return dynamicSchema;
	}

	/* @brief	Schema Helper, used for serialization:  Writes the internal parameterLVB_ into a map,
	*			suitable for attaching to a schema.
	*  @returns	A map from string to Variant, containting the keys and values of the parameter LabelValueBlock
	*/
	std::map<std::string, Variant> FixingTable::getParameterLVBDataMap() const
	{
		std::map<std::string, Variant> dataMap;

		auto keys   = parameterLVB_.getKeys();
		auto values = parameterLVB_.getValues();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			dataMap[ keys[ i ] ] = values[ i ];
		}

		return dataMap;
	}

	double FixingTable::getFixingValue(const boost::gregorian::date& fixingDate) const
	{
		// Note the index is -1 if not found
		int dateIndex = getIndexOfDate(fixingDate);

		if (dateIndex < 0)
		{
			throw ETradingException((boost::format("#Error: Fixing Date %s not found in fixing table %s") % toYYYYMMDDFromGregorianDate(fixingDate) % getRefToName()).str().c_str());
		}

		double fixingValue = getData(dateIndex).second;
		return fixingValue;
	}

	/* @brief	Returns a fixing for the specified date, if available.
	*			If there is no fixing available, this function does NOT throw;
	*			Instead, the success or failure is indicated by the FixingResult structure.
	*  @param	fixingDate	Return the fixing for this date
	*
	*  '@returns	A FixingResult structure, which contains a flag indicating whether the
	*				fixing was found, and the actual fixing value.
	*/
	FixingResult FixingTable::getOptionalFixingValue( const boost::gregorian::date& fixingDate ) const
	{
		FixingResult fixingResult;

		const int dateIndex = getIndexOfDate(fixingDate);
		if ( dateIndex < 0 )
		{
			fixingResult.fixingFound_ = false;
			fixingResult.fixingValue_ = 0.0;
		}
		else
		{
			fixingResult.fixingFound_ = true;
			fixingResult.fixingValue_ = getData( dateIndex ).second;
		}
		return fixingResult;
	}

	/* @brief	Function to get the fixing values corresponding to the specified fixing dates
	*			This function is specific to interest rate fixings.
	*			The method performs validation to ensure the user is requesting fixings
	*			from the correct fixing table
	*/
	std::vector< double > FixingTable::getFixingValues(const std::string& currency, const std::string& curveTenor, const std::vector< boost::gregorian::date>& fixingDates) const
	{
		// Check the Fixing Table being queried has the correct currency
		std::string fixingTableCurrency = toString(currency_);
		if (currency != fixingTableCurrency)
		{
			throw ETradingException((boost::format("#Error: Invalid Currency; Cannot request currency '%s' fixing from a %s fixing table") % currency % fixingTableCurrency).str().c_str());
		}

		// Check the Fixing Table being queried has the correct currency
		std::string fixingTableCurveTenor = toString(curveTenor_);
		if (curveTenor != fixingTableCurveTenor)
		{
			throw ETradingException((boost::format("#Error: Invalid Curve Tenor; Cannot request curve tenor '%s' fixing from a %s fixing table") % curveTenor % fixingTableCurveTenor).str().c_str());
		}

		std::vector<double> results;
		for (auto cit = fixingDates.cbegin(); cit != fixingDates.cend(); cit++)
		{
			auto result = getFixingValue(*cit);
			results.push_back(result);
		}
		return results;
	}

	/* @brief	General function to get the fixing values corresponding to the specified fixing dates
	*			This function works with all fixing types
	*			e.g. INTERESTRATE, INFLATION, FX fixings
	*			The method performs validation to ensure the user is requesting fixings
	*			from the correct fixing table
	*/
	std::vector< double > FixingTable::getFixingValues( const LabelValueBlock& parameterLVB, const std::vector< boost::gregorian::date>& fixingDates ) const
	{
		// Check the Fixing Table being queried has matching paameters
		auto keysInput = parameterLVB.getKeys();
		auto keys = parameterLVB_.getKeys();

		AQ_REQUIRE( keysInput.size() == keys.size(), "The specified parameterLVB does not match the parameterLVB in the Fixing Table: Different number of keys." );

		for (auto key : keys)
		{
			auto valueInternal	= parameterLVB_.getCompulsoryValueAsString( key );
			auto valueInput		= parameterLVB.getCompulsoryValueAsString( key );
			AQ_REQUIRE( valueInput == valueInternal, "Invalid parameterLVB. Parameter '" + key + "' does not match" );
		}

		std::vector<double> results;
		for ( auto fixingDate: fixingDates )
		{
			auto result = getFixingValue( fixingDate );
			results.push_back (result );
		}

		return results;
	}

	
	VariantMatrix FixingTable::getVariantMatrix() const
	{
		// Create First Column
		std::vector<Variant> columnOne;
		columnOne.push_back("CURRENCY");
		columnOne.push_back("CURVE_TENOR");
		columnOne.push_back("FIXING_DATES");

		// Get the Fixing Dates from the first column of the TableDateDouble object i.e. the fixing table
		auto& dates = TableDateDouble::viewData(0);
		for (auto dateCounter = 0u; dateCounter < dates.size(); dateCounter++)
		{
			columnOne.push_back(dates[dateCounter]);
		}

		// Create Second Column
		std::vector<Variant> columnTwo;
		columnTwo.push_back(toString(this->currency_));
		columnTwo.push_back(toString(this->curveTenor_));

		// Get the Fixing Values from the second column of the TableDateDouble object i.e. the fixing table
		auto& values = TableDateDouble::viewData(1);
		columnTwo.push_back("FIXING_VALUES");
		for (auto valuesCounter = 0u; valuesCounter < values.size(); valuesCounter++)
		{
			columnTwo.push_back(values[valuesCounter]);
		}

		// Update the Variant Matrix
		VariantMatrix variantMatrix;
		variantMatrix.push_back(columnOne);
		variantMatrix.push_back(columnTwo);

		return variantMatrix;
	};
	

	void FixingTable::populateFixingTableDatesValues(std::vector<Variant>& fixingDates, std::vector<Variant>& fixingValues) const
	{
		auto fixingTableMatrix = getVariantMatrix();
		if (fixingTableMatrix.size() < 1)
		{
			throw etrading::ETradingException((boost::format("#Error: Fixing Table %s results are empty or incomplete") % getName()).str().c_str());
		}

		if (fixingTableMatrix[0].size() < 3)
		{
			throw etrading::ETradingException((boost::format("#Error: Fixing Table %s results are empty or incomplete") % getName()).str().c_str());
		}

		// Get the fixing dates and values: ignoring the 3 text fields at the start of each vector i.e. ignoring currency, curveTenor and fixingHeader
		fixingDates = std::vector<Variant>(fixingTableMatrix[0].cbegin() + 3, fixingTableMatrix[0].cend());
		fixingValues = std::vector<Variant>(fixingTableMatrix[1].cbegin() + 3, fixingTableMatrix[1].cend());
	}

	LAStringMatrix FixingTable::getFixingTableDateValueMatrix() const
	{
		std::vector<Variant> fixingDates;
		std::vector<Variant> fixingValues;

		populateFixingTableDatesValues(fixingDates, fixingValues);

		LAStringMatrix fixingDatesValues;
		for (unsigned int i = 0; i < fixingDates.size(); ++i)
		{
			LAStringVector tempVec;
			tempVec.push_back(fixingDates[i].getValueAsString());
			tempVec.push_back(fixingValues[i].getValueAsString());
			fixingDatesValues.push_back(tempVec);
		}

		return fixingDatesValues;
	}


}

