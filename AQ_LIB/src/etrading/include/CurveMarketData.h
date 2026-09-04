
#pragma once

/*
 * @brief: This class represents the market data object used to build a curve
 */


#include "IsAQObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"


namespace etrading
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

    class CurveMarketData : public IsAQObject
    {
	public:

		/* @brief Main Constructor
		 * @param[in] objectName      The name of this CurveMarketData instance
		 * @param[in] marketDataKeys  A vector containing the names of each marketData block
		 * @param[in] infoBlocks      A vector containing the marketData blocks
		 */
		CurveMarketData( const std::string& objectName,
                         const std::vector<std::string>& marketDataKeys,
                         const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this CurveMarketData instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		CurveMarketData( const std::string& objectName,
                         const FreeObject& freeObject );

		/* @brief Copy Constructor
		 */
   		CurveMarketData(const CurveMarketData& rhs);
		virtual ~CurveMarketData() {}

		std::shared_ptr<CurveMarketData> clone() const;

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

        //This is a helper method to populate SchemaObject:
        virtual void const toSchemaObject(SchemaObject& schemaObject) const;

        /* @brief Static Helper Method to bump the marketData for the specified MarketDataStringMatrix, bumpSize and marketDataKey 
        * @param [in/out]   marketDataStringMatrix          The market data to be bumped - input the original market data to get bumped market data as output
        * @param [in]       bumpSize                        Enter the bump or shift size to apply
        * @param [in]       marketDataType                  The marketData key or type to be updated
	    */
        static void bumpMarketDataFromAQLStringMatrix( AQLStringMatrix & marketDataStringMatrix, const double bumpSize, const CurveMarketDataEnum & marketDataType );

		/* @brief Static Helper Method to perturb the nth marketData instrument for the specified MarketDataStringMatrix, bumpSize and marketDataKey 
        * @param [in/out]   marketDataStringMatrix          The market data to be bumped - input the original market data to get bumped market data as output
        * @param [in]       nthInstrumentToBump             Enter the nth instrument to bump using base zero
		* @param [in]       bumpSize                        Enter the bump or shift size to apply
        * @param [in]       marketDataType                  The marketData key or type to be updated
	    */
        static void perturbMarketDataFromAQLStringMatrix( AQLStringMatrix & marketDataStringMatrix, size_t nthInstrumentToBump, const double bumpSize, const CurveMarketDataEnum & marketDataType );

		/* @brief Static Helper Method to get the Tenor for the marketData instrument for the specified MarketDataStringMatrix
        * @param [in/out]   marketDataStringMatrix          The market data to be bumped - input the original market data to get bumped market data as output
        * @param [in]       marketDataType                  The marketData key or type to be updated
	    */
        static StandardStringVector getMarketDataTenorsFromAQLStringMatrix( const AQLStringMatrix & marketDataStringMatrix, const CurveMarketDataEnum & marketDataType );

		// *** Helper Method to provide the Market Data Rate Column to Bump ***
		 // Note: We return column number as -1 to indicate there is no rate column
		static int findMarketDataRateColumnNumber_ForAllInstrumentTypes(const CurveMarketDataEnum & marketDataType);

		/* @brief Method to recalculate the futures price for given bump in the underlying forward rate
		* @param [in]   futures price                   original futures price
		* @param [in]   bumpSize                        Enter the bump or shift size to apply
		*/
		double bumpFuturesPrice( const double & futuresPrice, const double & bumpSize) const;

        /* @brief Method to bump the marketData for the specified bumpSize and marketDataKey 
	    * @param [in]   bumpSize                        Enter the bump or shift size to apply
        * @param [in]   marketDataKey                   The marketData type to be updated
        * @param [in]   onlyBumpOutrightInstruments     Only bump outright instruments, defaults to true
        * @param [in]   noThrow                         No throw if market data not bumped, defaults to false i.e. will throw by default
	    */
        void bumpMarketData( const double bumpSize, const StandardString & marketDataType, const bool onlyBumpOutrightInstruments = true, const bool noThrow = false ) const;

        /* @brief Method to bump all marketData for the specified bumpSize
	    * @param [in]   bumpSize                        Enter the bump or shift size to apply
        * @param [in]   onlyBumpOutrightInstruments     Only bump outright instruments, defaults to true
        */
        void bumpAllMarketData( const double bumpSize, const bool onlyBumpOutrightInstruments = true ) const;
        
        /* @brief Updates the marketData for the specified marketDataKey
		 * @param [in]   marketData      The marketData
         * @param [in]   marketDataKey   The marketData type to be updated
         */
		void updateMarketDataLocalCache( const VariantMatrix & marketData, const StandardString & marketDataType ) const;

        /* @brief Clear the local market data cache(s) */
		void clearMarketDataLocalCache() const;

		/* @brief Returns the marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The marketData type to be displayed
         * @param [in]   columnNumber    Enter the column number using Base 1 to return a particular column or use -1 to show all columns
		 * @param [out]  A VariantMatrix containing a LabelValue block of properties
		 */
		const VariantMatrix viewInputParameters( const CurveMarketDataEnum marketDataType, const int columnNumber = -1 ) const;

        /* @brief Returns a column of marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The marketData type to be displayed
         * @param [in]   columnNumber    Enter the column number using Base 1 to return a particular column
		 * @param [out]  A VariantVector containing a LabelValue block of properties
		 */
		const VariantVector viewCurveMarketDataColumn( const CurveMarketDataEnum marketDataType, const int & columnNumber ) const;

		/* @brief Returns a VariantMatrix containing the marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The property to be displayed
	     * @param [out]  A VariantMatrix containing the LabelValue block
		 */
		VariantMatrix toVariantMatrix( const std::string& marketDataKey ) const;

		/* @brief Returns a AQLStringMatrix containing the marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The property to be displayed
		 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A AQLStringMatrix containing the LabelValue block
		 */
		AQLStringMatrix toAQLStringMatrix( const std::string& marketDataKey, const bool trimBlankRows = true ) const;
		
        /* @brief Returns a StandardStringMatrix containing the marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The property to be displayed
		 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A StandardStringMatrix containing the LabelValue block
		 */
		StandardStringMatrix toStandardStringMatrix( const std::string& marketDataKey, const bool trimBlankRows = true ) const;

        /* @brief Returns a LabelValueBlock containing the marketData information for the specified marketDataKey
		 * @param [in]   marketDataKey   The property to be displayed
		 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the AQLStringMatrix
	     * @param [out]  A AQLStringMatrix containing the LabelValue block
		 */
		LabelValueBlock toLabelValueBlock( const std::string& marketDataKey, const bool trimBlankRows = true ) const;

        /* @brief Returns a boolean to confirm if an optional marketData key has been specified
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A bool to confirm if the optional property key is present
		 */
		const bool doesKeyExist( const std::string& marketDataKey ) const;

		/* @brief Returns A vector of keys used in the block of data identified by the marketDataKey
		 * @param [in]   marketDataKey   Key that identifies the block of data interested
	     * @param [out]  A vector of keys
		 */
		std::vector<std::string> getKeysInTable( const std::string& marketDataKey );

		/* @brief Sets the local Market Data Cache, which is a temporary cache for market data that is in memory and not persisted as part of the Market Data Object
		 * @param [in]   marketDataKey      Key that identifies the block of data being set
	     * @param [in]   marketData	        A string matrix type of market data being set to the object
		 */
		void setMarketDataLocalCache(const std::string& marketDataType, const AQLStringMatrix& marketData);

		// Method to remove unused market data where the "USE" market data column flag is set to "FALSE"
		// Note the marketDataValues are transpose and the Use Column is optional and not always present
		static VariantMatrix checkDimensionsAndRemoveUnusedData( const std::string & marketDataKey, const VariantMatrix & marketDataValues );

	private:

		FreeObject freeObject_;

        // A Local and Temporary Market Data Cache to facilitate improved speed of performance
        // *** Important Note *** Data stored in this local cache is local only and will not be persisted ...
        // ... This means the cache data potentially acts as an override to the underlying persisted data.
		mutable std::map<std::string, VariantMatrix> variantMarketDataLocalCache_;
        mutable std::map<std::string, AQLStringMatrix> stringMarketDataLocalCache_;
	};

	typedef std::shared_ptr< CurveMarketData > CurveMarketDataPtr;
}
