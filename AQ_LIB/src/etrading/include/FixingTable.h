#pragma once

#include "TableDateDouble.h"
#include "HasInstance.h"
#include "CoreEnumerations.h"
#include "HasVariantMatrixAccess.h"
#include "SerializeInstance.h"
#include "SchemaObject.h"
#include "LabelValueBlock.h"


namespace etrading
{

	// A structure which indicates if a fixing is available for a specified date
	struct FixingResult
	{
		bool fixingFound_;		// True if a fixing is available for the specified date; false otherwise
		double fixingValue_;	// The fixing value from the fixing table
	};

    /*
    	This class represents a fixing table, with the inputs:
        const std::string& tableName = the name of the fixing table (in case it goes in Cache : not necessary
    	const std::vector<boost::gregorian::date>& dates = dates of the fixings (will get checked on increasing dates)
    	const std::vector<double>& values = values of the fixings (should be of same length as dates)
    	const etrading::CurveTenorEnum curveTenor = an enum denoting the curve tenor (cfr. CoreEnumerations.h)
    */
    class FixingTable
        : public TableDateDouble, // isLWOObject is inherited via TableDateDouble
          public HasVariantMatrixAccess
    {
    public:
        
        FixingTable( const std::string& tableName = "" );

		/* @brief	Consructor specific to interest rate fixings
		*  @param [in]		tableName		    Fixing Table name
		*  @param [in]		dates			    The fixing dates to store
		*  @param [in]		values				The fixing values to store
		*  @param [in]		curveTenor			The curve tenor corresponding to the interest rate fixings
		*  @param [in]		currency			The currency corresponding to the interest rate fixings
		*/
        FixingTable( const std::string& tableName,
                     const std::vector<boost::gregorian::date>& dates,
                     const std::vector<double>& values,
                     const etrading::CurveTenorEnum curveTenor,
                     const etrading::CCY currency );

		/* @brief	Consructor which accepts a parameterLVB, and can be used for many fixing types
		*			e.g. INTERESTRATE, INFLATION, FX
		*
		*  @param [in]		tableName		    Fixing Table name
		*  @param [in]		dates			    The fixing dates to store
		*  @param [in]		values				The fixing values to store
		*  @param [in]		parameterLVB		A set of parameters specifying the fixing table type,
		*										and other parameters used for validation
		*/
		FixingTable( const std::string& tableName,
					 const std::vector<boost::gregorian::date>& dates,
					 const std::vector<double>& values,
					 const LabelValueBlock& parameterLVB );


        FixingTable& operator=( const FixingTable& fixingTable );

		std::shared_ptr<FixingTable> clone();
        
		// These accessors kept for backwards compatibility and are specific to interest rate fixings
        const etrading::CurveTenorEnum getCurveTenor() const;
        const etrading::CCY getCurrency() const;

		// Return the parameterLVB which describes this fixing table
		LabelValueBlock getParameterLVB() const;
        
        // Functions to get fixing value(s) from fixing date(s)
        double getFixingValue( const boost::gregorian::date& fixingDate ) const;

		/* @brief	Returns a fixing for the specified date, if available.
		*			If there is no fixing available, this function does NOT throw;
		*			Instead, the success or failure is indicated by the FixingResult structure.
		*  @param	fixingDate	Return the fixing for this date
		*
		*  '@returns	A FixingResult structure, which contains a flag indicating whether the
		*				fixing was found, and the actual fixing value.
		*/
		FixingResult getOptionalFixingValue( const boost::gregorian::date& fixingDate ) const;

		/* @brief	Function to get the fixing values corresponding to the specified fixing dates
		*			This function is specific to interest rate fixings.
		*			The method performs validation to ensure the user is requesting fixings
		*			from the correct fixing table
		*/
        std::vector< double > getFixingValues( const std::string& currency, const std::string& curveTenor, const std::vector< boost::gregorian::date>& fixingDates ) const;

		/* @brief	General function to get the fixing values corresponding to the specified fixing dates
		*			This function works with all fixing types
		*			e.g. INTERESTRATE, INFLATION, FX fixings
		*			The method performs validation to ensure the user is requesting fixings
		*			from the correct fixing table
		*/
		std::vector< double > getFixingValues( const LabelValueBlock& parameterLVB, const std::vector< boost::gregorian::date>& fixingDates ) const;

        // Get the Fixing Table Object as a Variant Matrix. Contains Currency, CurveTenor, FixingDates and FixingValues
        virtual VariantMatrix getVariantMatrix() const;

		// Helper function to populate the fixingDates & fixingValues
		void populateFixingTableDatesValues(std::vector<Variant>& fixingDates, std::vector<Variant>& fixingRates) const;

		// Helper function to get a matrix of fixingDates & fixingValues
		AQLStringMatrix getFixingTableDateValueMatrix() const;

        // Populate the FixingTable Schema Object
        virtual const SchemaObject toSchemaObject() const;

    private:
		/* @brief	Schema Helper, used for serialization: Appends a schema and data for the parameterLVB
		*  @param	schemaObject	The schema object created by the main public toSchemaObject.
		*							This schemaObject is modified to include the parameterLVB data
		*/
		void toSchemaObject( SchemaObject& schemaObject ) const;

		/* @brief	Schema Helper, used for serialization: Creates a data schema suitable for storing the parameterLVB
		*  @param	schemaName	The name of the schema ( e.g. FIXING_TABLE_PARAMETER_LVB )
		*  @returns	A data schema object
		*/
		const DataSchema generateDataSchemaForParameterLVB( const std::string& schemaName ) const;

		/* @brief	Schema Helper, used for serialization:  Writes the internal parameterLVB_ into a map,
		*			suitable for attaching to a schema.
		*  @returns	A map from string to Variant, containting the keys and values of the parameter LabelValueBlock
		*/
		std::map<std::string, Variant> getParameterLVBDataMap() const;
		
		LabelValueBlock parameterLVB_;

        static const std::vector< Variant > FixingParameterNames;

		// These parameters kept for backwards compatibility
        etrading::CurveTenorEnum curveTenor_;
        etrading::CCY currency_;

    };

    typedef std::map<std::string, std::shared_ptr<FixingTable>> FixingTableMap;
	typedef std::shared_ptr< FixingTable > FixingTablePtr;

};



