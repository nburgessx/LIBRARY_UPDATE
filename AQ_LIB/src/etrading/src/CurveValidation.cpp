#include "CurveValidation.h"
#include "CurveBuildDefaults.h"
#include "CurveResultsContainer.h"      // Accessors to Curve Results, Description and Discount Factors
#include "InitializeETrading.h"
#include "ParameterValidation.h"
#include "AQLDefinitions.h"
#include "AQLDateSchedule.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLCurveCalibrationHelpers.h"
#include "AQLPriceDataCalendar.h"
#include "CurveCalibrationData.h"
#include "AQLMarketData.h"
#include "AQLPriceDataInterpolation.h"
#include "CommonConstants.h"
#include "AQLStaticData.h"
#include "ExceptionMacros.h"
#include "Solvers.h"
#include "CurveUtilities.h"
#include "SimpsonsRuleIntegration.h"

#include <cctype>
#include <utility>
#include <boost/assign.hpp>
#include <set>

namespace etrading
{

    /* @brief			Validate if the curve exists
    *  @param [in]		dataInstance		Pointer to the object pool
    *  @param [in]		curveId		CurveID
    */
    void checkIfCurveExists( AQLDataInstance* dataInstance, const AQLString& curveId )
    {
        AQLObjectPool& objPool = dataInstance->getObjectPool();
        if ( !objPool.getObject( curveId ).isDefined() )
        {
            AQ_THROW("Invalid Curve: CurveCollection '" + curveId + "' does not exist");
        }
    }

    /* @brief			Convert the Frequency to Tenor
    * @param [in]		freqquency	Frequencey
    * @output			Term: 1Y, 6M, 3M, 1M, 1W
    */
    AQLString fromFrequencyToTerm( const AQLString& frequency )
    {
        AQLString term;
        AQLString freq  = AQLString( frequency ).toUpper();

        if ( freq == "ANNUAL" )
        {
            term = "1Y";
        }
        else if ( freq == "SEMI-ANNUAL" )
        {
            term = "6M";
        }
        else if ( freq == "QUARTERLY" )
        {
            term = "3M";
        }
        else if ( freq == "MONTHLY" )
        {
            term = "1M";
        }
        else if ( freq == "WEEKLY" )
        {
            term = "1W";
        }
        else if ( freq == "BUSINESS_DAYS" || freq == "DAILY" )
        {
            term = "1D";
        }
        else
        {
            AQ_THROW("Frequency must be Annual, Semi-Annual, Quarterly, Monthly, Weekly, or Daily");
        }

        return term;
    }

    /* @brief			Validate and populate default frequency
    * @param [in]		isForwardInterp	True to indicate the interpolation is applied on forwards.
    * @param [inout]	frequency	Frequency
    */
    void validateFrequency( bool isForwardInterp, AQLString& frequency )
    {
        if ( isForwardInterp )
        {
            frequency = getDefaultValueForEmptyString( frequency, AQLString( "SIMPLE" ) );
            if( AQLString( frequency ).toUpper() != "SIMPLE" )
            {
                AQ_THROW("Forward interpolation may not be used with a frequency other than 'SIMPLE'");
            }
        }
        else
        {
            frequency =	getDefaultValueForEmptyString( frequency, AQLString( "ANNUAL" ) );
        }
    }

    /* @brief			Get the curve currency
    * @output			currency name
    */
    AQLString getCurveCurrency( const AQLString& curveCollection )
    {
        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
        AQLObjectHolder* objHolder = &objPool.getObject( curveCollection, ENCHKTYPE_NOCHECK );
        AQLString currency = "";
        if ( objHolder->isDefined() )
        {
            AQLDataHolder* dh = &objHolder->get().getData( IR_CALIBRATION_DATA_CURRENCY );

            if ( dh->isDefined() && !dh->isNull() )
            {
                currency = ( dynamic_cast<const AQLDataString&> ( dh->get() ) ).get();
                currency.toLower();
            }
            else
            {
                AQ_THROW("Curve collection '" + curveCollection + "' does not exist");
            }
        }
        else
        {
            AQ_THROW("Curve collection '" + curveCollection + "' does not exist");
        }
        return currency;
    }

    /*  @brief is fwdfx constant curve or not, the logic is from AQLCalibrateModel.cpp
    * @param[in]   currency				Currency or fx ex.JPY/USD
    * @param[in]   staticDataTable  Static data table/market name
    * @param[out]   is fwdfx constant curve or not
    */
    bool isFwdFXConst( const AQLString& currency, const AQLString& staticDataTable )
    {
        AQLString tmpCcy = currency;
        AQLStaticData& irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();
        AQLString suffix = "." + staticDataTable;
        AQLString str = irStaticData.getStaticData( tmpCcy.toLower() + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST + suffix.toLower() );
        bool isFwdFx = ( str.toUpper() == "TRUE" );
        return isFwdFx;
    }


    /* @brief			Get the forward interpolation parameters
    * @param [in]		curveCollection		Curve collection
    * @param [in]		staticDataTable		Market name
    * @param [in]		fwdInterOverride			User specified fwdInter flag
    * @output			FwdInterInfo
    */
    FwdInterInfo getfwdInterInfo( const AQLString& curveCollection, const AQLString& staticDataTable, const BooleanEnum& fwdInterOverride )
    {

		AQLString curveType = (curveCollection.size() != 0 && staticDataTable.size() != 0) ? getCurveType(curveCollection, staticDataTable) : "";
		bool isCurveWithFwdTable = (curveType == SWAP);
		// bool isCurveWithFwdTable = (curveType == SWAP || curveType == CURVETYPE_ARR); // Needed if we want to use a foward table on ARR Curve

        bool isFwdInter = false;

		// If user specify the fwdInter, it takes priority
		if (fwdInterOverride != NONE_BOOL)
		{
			isFwdInter = toBoolean(fwdInterOverride);

			if (!isCurveWithFwdTable && isFwdInter)
			{
                AQ_THROW("Forward Interpolation can only be used on the Standard Outright Swap Curves. Curve Index '" + staticDataTable +"' is not a STD Swap Curve.");
			}

		}
		else
		{
			// By default, use the true for isCurveWithFwdTable, false for all other curves
			isFwdInter = isCurveWithFwdTable ? true : false;
		}


		FwdInterInfo info;
        info.isFwdInter = isFwdInter;
        info.useFwdData = isFwdInter;

		return info;

    }

	// Get the frequency string e.g 1D -> DAILY, 1W -> WEEKLY, 1M -> MONTHLY etc ...
	AQLString getFrequencyString(const AQLString& frequencyTenor)
	{
		AQLString frequencyString = "";
        AQLString frequencyTenorUpperCase = frequencyTenor;
        frequencyTenorUpperCase.toUpper();

        if( frequencyTenorUpperCase == "1Y" )
        {
            frequencyString = "ANNUAL";
        }
        else if( frequencyTenorUpperCase == "12M" )
        {
            frequencyString = "ANNUAL";
        }
        else if( frequencyTenorUpperCase == "6M" )
        {
            frequencyString = "SEMI-ANNUAL";
        }
        else if( frequencyTenorUpperCase == "3M" )
        {
            frequencyString = "QUARTERLY";
        }
        else if( frequencyTenorUpperCase == "1M" )
        {
            frequencyString = "MONTHLY";
        }
		else if( frequencyTenorUpperCase == "1W" )
        {
            frequencyString = "WEEKLY";
        }
        else if( frequencyTenorUpperCase == "1D" )
        {
            frequencyString = "DAILY";
        }
        else
        {
            AQ_THROW("Invalid Frequency or Tenor '" + frequencyTenorUpperCase + "': Only frequencies of be ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, WEEKLY or DAILY are supported");
        }

        return frequencyString;
	}

    /* @brief			return a number to indicate the freqOrTenor in months
    * @param [in]		freqOrTenor Frequency or Tenor
    * @output			number of months 
    */
    unsigned int getFrequencyOrTenorMonth( const AQLString& freqOrTenor )
    {
        if (same(freqOrTenor, "ANNUAL") || same(freqOrTenor, "1Y") || same(freqOrTenor, "12M")) return 12;
	    else if (same(freqOrTenor, "SEMI-ANNUAL")  || same(freqOrTenor, "6M") ) return 6;
	    else if (same(freqOrTenor, "QUARTERLY") || same(freqOrTenor, "3M") ) return 3;
	    else if (same(freqOrTenor, "MONTHLY") || same(freqOrTenor, "1M")  ) return 1;
		else if (same(freqOrTenor, "WEEKLY") || same(freqOrTenor, "1W")  ) return 0;
	    else if (same(freqOrTenor, "DAILY") || same(freqOrTenor, "BUSINESS_DAYS") || same(freqOrTenor, "1D") ) return 0;
	    else 
	    {
            AQ_THROW("Invalid Curve Frequency or Tenor '" + freqOrTenor + "': Only curve frequencies of ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, WEEKLY DAILY are supported");
        }
    }

    /* @brief			return a number to indicate the freq in months
    * @param [in]		freq Frequency
    * @output			number of months 
    */
    unsigned int getFrequencyNumber(const FrequencyEnum& freq)
    {
		switch ( freq )
		{
		case ANNUAL_FREQUENCY:
			return 12;
		case SEMI_ANNUAL_FREQUENCY:
			return 6;
		case QUARTERLY_FREQUENCY:
			return 3;
		case MONTHLY_FREQUENCY:
			return 1;
		case DAILY_FREQUENCY:
			return 0;
		default:
			AQ_THROW( "Unsupported frequency: " + toString( freq ) );
		}
    }

 	// Convert a frequency to a tenor
	AQLString getFrequencyTenor(const FrequencyEnum& frequencyString)
	{
		AQLString frequencyTenor = "";

        if( frequencyString == ANNUAL_FREQUENCY )
        {
            frequencyTenor = "1Y";
        }
        else if( frequencyString == SEMI_ANNUAL_FREQUENCY )
        {
            frequencyTenor = "6M";
        }
        else if( frequencyString == QUARTERLY_FREQUENCY )
        {
            frequencyTenor = "3M";
        }
        else if( frequencyString == MONTHLY_FREQUENCY )
        {
            frequencyTenor = "1M";
        }
		else if( frequencyString == WEEKLY_FREQUENCY )
        {
            frequencyTenor = "1W";
        }
        else if( frequencyString == DAILY_FREQUENCY )
        {
            frequencyTenor = "1D";
        }
        else
        {
            AQ_THROW("Invalid Curve Frequency '" + toString(frequencyString) + "': Only frequencies of ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, WEEKLY or DAILY are supported");
        }

        return frequencyTenor;
	}

    /* @brief			Function to return the curve frequency given the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @param [in]		uppercaseResult		The market name result is returned uppercase by default (true), some functions require the marketname to be un touched (false)
    * @output			curveFrequency
    */
    AQLString validateCurveAndGetCurveFrequency( const AQLString& curveCollection, const AQLString& curveIndex )
    {
        // Get the Market Name
        AQLString marketName = etrading::getCurveStaticDataTableName( curveCollection, curveIndex, false ); // uppercase result = false

        AQLString marketNameUppercase = marketName;
        marketNameUppercase.toUpper();

        // ------------------------------------------------------------------------------------------------
        // Create an instance of the yield curve procedure class that contains yield curve market data

        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
        if ( !objPool.getObject( curveCollection ).isDefined() )
        {
            AQ_THROW("Invalid Curve: CurveCollection '" + curveCollection + "' does not exist");
        }


        // ------------------------------------------------------------------------------------------------

        // Default Curve Frequency to Match USD Swaps and Xccy Basis Instruments
        AQLString curveFrequencyString;
        AQLString curveFrequencyTenor;
		getCurveFrequency(objPool, marketNameUppercase, curveCollection, curveIndex, curveFrequencyString, curveFrequencyTenor);

        return curveFrequencyTenor;
    }
	
    /* @brief			Function to return the curve frequency as a year fractiongiven the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			curveFrequency as a year fraction
    */
    double getCurveFrequencyAsYearFraction( const AQLString& curveCollection, const AQLString& curveIndex )
    {
        const AQLString curveFrequencyTenor = validateCurveAndGetCurveFrequency( curveCollection, curveIndex );
        
        if( curveFrequencyTenor == "1Y" || curveFrequencyTenor == "12M")
        {
            return 1.0;
        }

        if( curveFrequencyTenor == "6M" )
        {
            return 0.5;
        }

        if( curveFrequencyTenor == "3M" )
        {
            return 0.25;
        }

        if( curveFrequencyTenor == "1M" )
        {
            const double oneMonth = 1.0 / 12.0;
            return oneMonth;
        }
        
        if( curveFrequencyTenor == "1D" )
        {
            const double oneDay = 1.0 / 365.0; // ACT/365 Fixed
            return oneDay;
        }

        // We should never reach here
        AQ_THROW("Invalid Curve Frequency")
        return 0.0;
    }

	/* @brief	Function to return the curveFrequencyString and curveFrequencyTenor corresponding to specified curveIndex and CurveCollection
	 * @param [in]		objPool						The EntityPool
	 * @param [in]		marketNameUpperCase		The MarketName / StaticDataTable table name
	 * @param [in]		curveCollection			The curve collection Id
	 * @param [in]		curveIndex				The curveIndex used to look up the curveFrequencyString and curveFrequencyTenor
	 * @param [out]		curveFrequencyString	The curveFrequencyString corresponding to the specified curveIndex
	 * @param [out]		curveFrequencyTenor		The curveFrequencyTenor corresponding to the specified curveIndex
     * @param [in]		enableThrow		        Enable Throw - Throw on Error True or False - Defaults to True
	 */
	void getCurveFrequency(AQLObjectPool& objPool, const AQLString& marketNameUppercase, const AQLString& curveCollection, const AQLString& curveIndex, AQLString& curveFrequencyString, AQLString& curveFrequencyTenor, const bool enableThrow)
	{
        // Get the Frequency from the Curve Results Object if available, else from the object pool (slow)
        // --------------------------------------------------------------

        // New: Curve Frequency from the Thread-Safe Curve Results Object
        // --------------------------------------------------------------
        const bool isEnabledCurveResults = etrading::isEnabledCurveResults();
        const bool doesExistCurveDescription = etrading::doesExistCurveResultsDiscountFactors( curveCollection.getCString(), curveIndex.getCString() );

        if ( isEnabledCurveResults && doesExistCurveDescription )
        {
            CurveTenorEnum curveTenorEnum = getCurveDescriptionFromCurveResultsObject( curveCollection.getCString(), curveIndex.getCString() )->curveTenorEnum();
            
            // Expected Result: 1Y, 6M, 12M, 3M, 1M, 1D
            curveFrequencyTenor = toString( curveTenorEnum ).c_str();

            // Expected Result: ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, DAILY
            curveFrequencyString = toFrequencyFromCurveTenor( curveFrequencyTenor.getCString() ).c_str();
                        
            return;
        }

        // LEGACY: Get the Curve Frequency from the object pool ( Very Slow )
        // --------------------------------------------------------------
		AQLString curveType = getCurveType( objPool, curveCollection, marketNameUppercase );
		
		// Now determine the curveFrequency based on the input parameters
        if ( curveType == CURVETYPE_OIS || curveType == CURVETYPE_ARR || curveType == CURVETYPE_FWDFXCONST || curveType == CURVETYPE_CHEAPESTTODELIVER )
        {
            // It has been agreed that on a temperary basis we set the frequency to
			// '1D' for these curve types. 
            curveFrequencyString = AQLString( "DAILY" );
            curveFrequencyTenor = AQLString( "1D" );
        }
        else if( curveType == CURVETYPE_SWAP )
        {
            // Standard Swap Curve

            AQLString curveName = (marketNameUppercase == STD) ? "" : "_" + marketNameUppercase;
            AQLString swapCurveEntityName = curveCollection + "_SWAP_0" + curveName;

            // Get the Swap Curve from the Object Pool
            const AQLObjectHolder swapEntity = objPool.getObject( swapCurveEntityName );

            if ( !swapEntity.isDefined() )
            {
                AQ_THROW("Invalid Curve: STD Swap Curve does not exist");
            }

            // Get the Swap Market Data
            AQLObject* swapMarketData = &objPool.getObject( swapCurveEntityName ).get();

            if ( swapMarketData == nullptr )
            {
                AQ_THROW("Invalid Curve: Market data for STD Swap Curve does not exist");
            }

            curveFrequencyString = dynamic_cast< const AQLDataString& >( swapMarketData->getData( IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL ).get() );
            curveFrequencyTenor = getFrequencyTenor( toFrequencyEnum(curveFrequencyString.getCString()) );
        }
        else if( curveType == CURVETYPE_BASIS || curveType == "TENORBASIS" || curveType == XCCYBASIS )
        {
            // Get the Frequency from the Curve Results Object if available, else from the object pool (slow)
            const bool isEnabledCurveResults = etrading::isEnabledCurveResults();
            const bool doesExistCurveDescription = etrading::doesExistCurveResultsDescription( curveCollection.getCString(), curveIndex.getCString() );

            if ( isEnabledCurveResults && doesExistCurveDescription )
            {
                // New: Curve Frequency from the Thread-Safe Curve Results Object
                curveFrequencyTenor = getCurveDescriptionFromCurveResultsObject( curveCollection.getCString(), curveIndex.getCString() )->curveTenor().c_str();
            }
            else
            {
                // LA: Object Pool Very Slow !!!
                // ---------------------------------

                // Get the CurveCalibrationData object object
                CurveCalibrationData* curveCalibrationData = etrading::InitializeETrading::instance().ycStaticDataObject( curveCollection );
            
                // Get the CurveFrequencyTenor: This function populates curveFrequencyTenor given the curveIndex
                curveCalibrationData->getBasisCurveFrequency( curveIndex, curveFrequencyTenor );
            }
        }
        else
        {
            // Some LA functions require that we do not throw if we can't find the curve type and / or frequency
            if (enableThrow)
            {
                AQ_THROW("Invalid Curve Type - Unable to Retrieve the Curve Frequency")
            }
        }

	}

    /* @brief			Function to return the curve float daycount fraction given the curveCollection and curveName
    * @param [in]		curveCollection	    Curve collection Id
    * @param [in]		curveIndex		    Curve index name
    * @output			FloatDaycount
    */
    AQLString validateCurveAndGetFloatDaycount( const AQLString& curveCollection, const AQLString& curveIndex )
    {
        // Get the Market Name
        AQLString marketName = etrading::getCurveStaticDataTableName( curveCollection, curveIndex, false ); // uppercase result = false

        AQLString marketNameUppercase = marketName;
        marketNameUppercase.toUpper();

        // ------------------------------------------------------------------------------------------------
        // Create an instance of the yield curve procedure class that contains yield curve market data

        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
        if ( !objPool.getObject( curveCollection ).isDefined() )
        {
            AQ_THROW("Invalid Curve: CurveCollection '" + curveCollection + "' does not exist");
        }
        
        // Get the CurveCalibrationData object object
        CurveCalibrationData* curveCalibrationData = etrading::InitializeETrading::instance().ycStaticDataObject( curveCollection );
        
        // ------------------------------------------------------------------------------------------------

        // Default Curve Frequency to Match USD Swaps and Xccy Basis Instruments
        AQLString curveDaycount;

		AQLString curveType = getCurveType( objPool, curveCollection, marketNameUppercase );
        if ( curveType == CURVETYPE_OIS || curveType == CURVETYPE_ARR)
        {
            // OIS Curve
            AQLString currency           = getCurveCurrency( curveCollection );
            AQLString keyName            = currency + STATIC_DATA_KEY_YIELD_OIS_DAYCOUNT + "." + marketNameUppercase;
			keyName.toLower();
            AQLStaticData& irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();
            curveDaycount               = irStaticData.getStaticData( keyName );
        }
        else if( curveType == CURVETYPE_SWAP )
        {
            // Standard Swap Curve

			AQLString curveName = (marketNameUppercase == STD) ? "" : "_" + marketNameUppercase;
            AQLString swapCurveEntityName = curveCollection + "_SWAP_0" + curveName;

            // Get the Swap Curve from the Object Pool
            const AQLObjectHolder swapEntity = objPool.getObject( swapCurveEntityName );

            if ( !swapEntity.isDefined() )
            {
                AQ_THROW("Unable to find the STD Swap curve in the object pool cache");
            }

            // Get the Swap Market Data
            AQLObject* swapMarketData = &objPool.getObject( swapCurveEntityName ).get();

            if ( swapMarketData == nullptr )
            {
                AQ_THROW("Unable to find the market data for the STD Swap Curve");
            }

            curveDaycount =  dynamic_cast<const AQLPriceDataDayCount& >( swapMarketData->getData( IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL ).get() ).convertToString();

        }
        else if ( curveType == CURVETYPE_FWDFXCONST)
        {
            // FX Forward Curve ... market convention for FX Forward Curves is to match the daycount USD leg of the XCCY Basis
            curveDaycount = AQLString( "ACT/360" );
        }
        else
        {
            // Basis Curves
            AQLPriceDataDayCount      attrDayCount;
            AQLPriceDataSlidingRule   attrBusinessDayAdjustment;
            AQLPriceDataCalendar      attrCalendar;
            AQLString            curveFrequencyTenor;

            // Get the CurveFrequencyTenor: This function populates forward conventions given the curveIndex, which includes the curveFrequencyTenor
            curveCalibrationData->getForwardConvention( curveIndex, attrDayCount, attrBusinessDayAdjustment, attrCalendar, curveFrequencyTenor );
			curveDaycount = attrDayCount.convertToString();
        }

        return curveDaycount;
    }

	/* @brief	Function to return the StaticDataTable, CurveIndex, CurveTenor, for each curveIndex in a given curveCollection
	 * @param [in]		curveCollection	    Curve collection Id
	 * @param [out]		A vector of items, where each item consists of:  StaticDataTable, CurveIndex, CurveTenor
	 */
	std::vector<AQLStringVector> getCurveNamesInCurveCollection( const AQLString& curveCollection )
    {
		// The return value
        std::vector<AQLStringVector> curves;

		// First get a refererence to the Entitypool
		AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
        if ( objPool.getSize() == 0 || !objPool.getObject( curveCollection ).isDefined() )
        {
            AQ_THROW("Invalid Curve: CurveCollection '" + curveCollection + "' does not exist");
        }
		
		// Construct the yieldCurvePro name corresponding to this curveCollection (e.g. PRO_YIELD_EURYC)
		// and fetch the CurveCalibrationData from the EntityPool

        // Get the CurveCalibrationData object object
        CurveCalibrationData* curveCalibrationData = etrading::InitializeETrading::instance().ycStaticDataObject( curveCollection );

		// Get the assignedCurveMktMap from the CurveCalibrationData
		// This is a map from curveIndex to staticDataTable
		const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
		
		// For a given StaticDataTable, we would like return a single curveIndex name,
		// ideally the most recently added curveIndex.
		// So here we iterate over the curveIndexes and take the first one we find, for a given StaticDataTable.
		// To allow us to return a single curveIndex for each StaticDataTable, we maintain a set containing
		// the StaticDataTables already visited.
		std::set<AQLString> staticDataTablesVisited;
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			const AQLString& curveIndex = it->first;
			if ( curveIndex.isDefined() )
			{
				AQLString staticDataTable = it->second;
				if ( staticDataTable == IR_NO_DATA )
				{
					// This can happen if the curve has not been built.
					continue;
				}
		
				const bool alreadySeen = staticDataTablesVisited.find(staticDataTable) != staticDataTablesVisited.end();
				if (! alreadySeen)
				{
					// A new StaticDataTable. We will process it.
					staticDataTablesVisited.insert(staticDataTable);

					if ( staticDataTable == AQLString( "SWAP" ) )
					{
						staticDataTable = AQLString( "STD" );
					}

					// Determine the curve frequency tenor
			        AQLString curveFrequencyString;
			        AQLString curveFrequencyTenor;

                    getCurveFrequency(objPool, staticDataTable, curveCollection, curveIndex, curveFrequencyString, curveFrequencyTenor, false); // false = dont throw if can't find frequency

					curves.push_back( boost::assign::list_of(staticDataTable)(curveIndex)(curveFrequencyTenor));
				}
			}
		}

		return curves;
	}


    /* @brief			Return staticDataTable(marketName) from curveCollection and curveIndex
    * @param [in]		curveCollection	Curve collection Id
    * @param [in]		curveIndex		Curve index name
    * @param [in]		uppercaseResult		The market name result is returned uppercase by default (true), some functions require the marketname to be un touched (false)
    * @output			staticDataTable
    */
    AQLString getCurveStaticDataTableName( const AQLString& curveCollection, const AQLString& curveIndex, const bool& uppercaseResult )
    {

		AQ_REQUIRE(curveIndex.size() != 0, "Curve index must be provided.");

        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
        if ( objPool.getSize() == 0 || !objPool.getObject( curveCollection ).isDefined() )
        {
            AQ_THROW("Invalid Curve: CurveCollection '" + curveCollection + "' does not exist");
        }
        
        // Get Yield Curve Pro Object Object
        CurveCalibrationData* curveCalibrationData = etrading::InitializeETrading::instance().ycStaticDataObject( curveCollection );
        
        AQLString singleIndex = curveIndex;
        if ( curveIndex.findString( MULTI_STATIC_DATA_DELIMITER ) != -1 )
        {
            singleIndex = curveIndex.toToken( MULTI_STATIC_DATA_DELIMITER )[0];
        }
        AQLString staticDataTable = curveCalibrationData->getMarketForCurve( singleIndex );
        if ( staticDataTable == IR_NO_DATA )
        {
            AQ_THROW("Invalid Curve: CurveIndex '" + curveIndex + "' in collection " + curveCollection + " does not exist");
        }

        // For the Standard Swap Curve the Market Name is transformed from STD to SWAP, but STD is expected
        if ( staticDataTable == AQLString( "SWAP" ) )
        {
            staticDataTable = AQLString( "STD" );
        }

        // if the curve index = market name then don't modify it to be uppercase
        if ( uppercaseResult == false )
        {
            AQLString tempCurveIndex = curveIndex;
            AQLString tempStaticDataTable = staticDataTable;

            if ( tempCurveIndex.toLower() == tempStaticDataTable.toLower() )
            {
                return curveIndex;
            }
        }

        return staticDataTable;
    }

    /* @brief			Return the type of a yield curve
    * @param [in]		objPool		                    Object Pool
    * @param [in]		curveCollection		        Curve collection Id
    * @param [in]		upperCaseStaticDataTable    Curve staticDataTable / MarketName - must be in uppercase !!!
    * @output			Curve type
    */
    AQLString getCurveType( AQLObjectPool& objPool, const AQLString& curveCollection, const AQLString& upperCaseStaticDataTable )
    {
        //Note: CurveType suffix is always upper case
        AQLString curveType("");
        AQLDataHolder* dh;
		AQLString curveTypeAttributeName;

        // The static data table may be an alias for 'STD' so we have to check each of the  alias' for the STD curve type keyword
        AQLStringVector curveIndexAliasGroup = curveIndexAliasList( curveCollection, upperCaseStaticDataTable );
        bool isSTDCurve = false;
        for ( size_t i = 0; i < curveIndexAliasGroup.size(); ++i )
        {
            if ( curveIndexAliasGroup[i] == STD )
            {
                isSTDCurve = true;
                break;
            }
        }

        if ( isSTDCurve )
		{
            curveTypeAttributeName = AQLString( CALIBRATION_DATA_CURVETYPE );
		}
		else
		{
            curveTypeAttributeName = AQLString( CALIBRATION_DATA_CURVETYPE + AQLString("_") + upperCaseStaticDataTable );
        }
        
        // Initialize Yield Curve Pro
        CurveCalibrationData* curveCalibrationData = etrading::InitializeETrading::instance().ycStaticDataObject( curveCollection );
        
        dh = &curveCalibrationData->getData( curveTypeAttributeName );
        if ( dh->isDefined() && !dh->isNull() )
        {
            curveType = dynamic_cast<AQLDataString&> ( dh->get() );
        }

        // Fail if no curve type returned
        // AQ_REQUIRE( curveType != "", "Invalid Curve Type - Curve Index " + upperCaseStaticDataTable + " has no corresponding Curve Type Set" )

        return curveType;
    }
    
    /* @brief			Return the type of a yield curve
    * @param [in]		curveCollection		Curve collection Id
    * @param [in]		staticDataTable		Curve staticDataTable(MarketName)
    * @output			Curve type
    */
    AQLString getCurveType( const AQLString& curveCollection, const AQLString& staticDataTable )
    {
        // get yield curve set
        AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();
        AQLObjectPool& objPool = dataInstance->getObjectPool();

        if ( !objPool.getObject( curveCollection ).isDefined() )
        {
            AQ_THROW("Invalid Curve: CurveCollection "+ curveCollection + " does not exist");
        }

        //Note: CurveType suffix is always upper case
        AQLString upperCaseStaticDataTable( staticDataTable );
        upperCaseStaticDataTable.toUpper();
        
        return getCurveType( objPool, curveCollection, upperCaseStaticDataTable );
    }

	/* @brief			Return the type of a yield curve
	* @param [in]		curveCollection		Curve collection Id
	* @param [in]		curveIndex			Curve index
	* @output			Curve type
	*/
	CurveTypeEnum getCurveTypeFromCurveIndex(const AQLString& curveCollection, const AQLString& curveIndex)
	{
		const AQLString curveMarketName = getCurveStaticDataTableName(curveCollection, curveIndex);

		return toCurveTypeEnum(getCurveType(curveCollection, curveMarketName).getCString());
	}
	
	/* @brief			Return interpolation method of the curve

    Note: We use the property file to read the yield curve settings and parameters. Properties are loaded from the ip.properties config file and then
    overridden by the end user. Properties here are the overridden ones, not the defaults in the config file

    * @param [in]		curveCollection	Curve collection Id
    * @param [in]		staticDataTable	Curve staticDataTable(MarketName)
    * @output			Interpolation method
    */
    AQLString getCurveInterpolation( const AQLString& curveCollection, const AQLString& staticDataTable )
    {
		AQLStaticData& irStaticData = AQLCoreDataService::getStaticDataManager().getStaticData();

        //Get curve currency
        AQLString currency = getCurveCurrency( curveCollection );

		
		AQLString market = staticDataTable;
		AQLString keyName;
		if (market.toUpper() == "STD")
		{
			keyName = currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE;
		}
		else
		{
			keyName = currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + "." + market.toLower();
		}
		
		AQLString curveType = irStaticData.getStaticData(keyName).toUpper();
		
		bool isSTDCurve = (curveType == SWAP) ? true : false;

        AQLString suffix;
        if ( isSTDCurve )
        {
            suffix = "";
        }
        else
        {
            suffix = "." + market.toLower();
        }

        if( isSTDCurve || (curveType == CURVETYPE_OIS) || (curveType == CURVETYPE_ARR) || (curveType == CURVETYPE_FWDFXCONST) || (curveType == CURVETYPE_CHEAPESTTODELIVER))
        {
            //e.g. "eur.sde.yield.generator.yieldgen.interpolation.ois"
            keyName = currency + STATIC_DATA_KEY_YIELD_GENERATOR_YIELDGEN_INTERPOLATION + suffix;
        }
        else
        {
            //e.g. "eur.sde.yield.generator.basis.interpolation.3m6mbasis"
            //e.g. "eur.sde.yield.generator.basis.interpolation.xccybasis"
            //e.g. "usd.sde.yield.generator.basis.interpolation.fwdfxconsteur"
            keyName = currency + STATIC_DATA_KEY_YIELD_GENERATOR_BASIS_INTERPOLATION + suffix;
        }

        //Get interpolation method from the curve building, e.g. fn_monotoneconvexinterpolation
        AQLString interp = irStaticData.getStaticData( keyName );

        if ( interp == IR_NO_DATA )
        {
            AQ_THROW("Invalid Interpolation Method: Interpolation type '"+ keyName + "' is invalid");
        }

		// Undecorate legacy interpolation names: 'fn_' + type + 'interpolation' e.g. fn_splineinterpolation
		std::string interpString = interp.c_str();
		AQ_REQUIRE( interpString.size() > 3, "Invalid Interpolation Type: " + interpString )
		// Check for for 'fn_' prefix
		if( boost::iequals( interpString.substr(0,3), "fn_" ) )
		{
			//remove 'fn_' prefix
			interp.remove( 0, 3 );
			//remove 'interpolation' suffix
			int size = interp.size();
			interp.remove( size - 13, size - 1 );
		}

        return interp;
    }

	/* @brief			Return boolean to confirm if curve exists
    * @param [in]		curveCollection	Curve collection Id
    * @output			Boolean for Does Curve Exist?
    */
	bool doesCurveExist( const AQLString& curveCollection )
	{
		AQ_REQUIRE(curveCollection.size() !=0, "Invalid Curve: CurveCollection not provided.");
        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
		
		bool result = true;
		if( objPool.getSize() == 0 || !objPool.getObject( curveCollection ).isDefined() )
		{
			result = false;
		}
		return result;
	}

    /* @brief			Return asOfDate of the curve
    * @param [in]		curveCollection	Curve collection Id
    * @output			As of Date
    */
    AQLDate getCurveAsOfDate( const AQLString& curveCollection )
    {
		if ( !doesCurveExist( curveCollection ) )
		{
			AQ_THROW("Invalid Curve: CurveCollection '" + curveCollection + "' does not exist");
		}

		AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();
        AQLDate asOfDate = dynamic_cast<const AQLDataDate& >( objPool.getObject( curveCollection ).get().getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ).get() ).get();
        return asOfDate;
    }
    
	/* @brief			convertTermToDate function which converts discount factor terms or year fractions to payment dates
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		term                The term or year fraction
	*  @param [out]		paymentDate         The corresponding payment date
	*/
    AQLDate convertCurveTermToDate( const AQLDate& asOfDate, const double& term )
    {
        // Curve terms are always calculated using ACT/365_ISDA as defined in ConstantDeclarations.h [sic] - see #define AC_365I
        // We const_cast the asOfDate because the underlying function incorrectly wants a non-const asOfDate and we can't touch it
        // Update: We now use ACT/365 instead of ACT/365_ISDA which causes leap year irregularities in the yield curve.
		AQLString dayCount("ACT/365");
        AQLDate paymentDate = AQLDateSchedule::getDateFromTerm( const_cast<AQLDate&>(asOfDate), term, dayCount );
        return paymentDate;
    }

    /* @brief			convertTermsToDates function which converts discount factor terms or year fractions to payment dates
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		terms               The terms or year fractions corresponding
	*  @param [out]		paymentDates        The corresponding payment dates
	*/
    DateVector convertCurveTermsToDates( const AQLDate& asOfDate, const DoubleVector& terms )
    {
        // Imply the curve payment dates from their terms values
        DateVector paymentDates( terms.size() );
        for( size_t i = 0; i<terms.size(); ++i )
        {
            paymentDates[i] = convertCurveTermToDate( asOfDate, terms[i] );
        }
        return paymentDates;
    }

    /* @brief			convertCurveDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		terms               The terms or year fractions
	*  @param [out]		paymentDates        The corresponding payment dates
	*/
    DateVector convertCurveTermsToDates( const AQLString& curveCollection, const DoubleVector& terms )
    {
        const AQLDate asOfDate = getCurveAsOfDate( curveCollection );
        const DateVector paymentDates = convertCurveTermsToDates( asOfDate, terms );
        return paymentDates;
    }

    /* @brief			convertDateToTerm function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		paymentDate         The payment date
	*  @param [out]		term                The corresponding term value
	*/
    double convertCurveDateToTerm( const AQLDate& asOfDate, const AQLDate& paymentDate )
    {
        // Curve terms are always calculated using ACT/365_ISDA as defined in ConstantDeclarations.h [sic] - see #define AC_365I
        // Update: We have migrated curve dates to ACT/365 because ACT/365_ISDA causes leap year irregularities in curves.
         AQLPriceDataDayCount dc_act365(ACT_365); 
         double term = dc_act365.getTerm( asOfDate, paymentDate );
         return term;
    }

    /* @brief			convertDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		asOfDate            The start or asOfDate, which is the base date for our terms year fractions
    *  @param [in]		paymentDates        The payment dates
	*  @param [out]		terms               The corresponding terms values
	*/
    DoubleVector convertCurveDatesToTerms( const AQLDate& asOfDate, const DateVector& paymentDates )
    {
        DoubleVector terms( paymentDates.size() );
        for( size_t i = 0; i<paymentDates.size(); ++i )
        {
            terms[i] = convertCurveDateToTerm( asOfDate, paymentDates[i] );
        }
        return terms;
    }

    /* @brief			convertCurveDatesToTerms function which converts discount factor payment dates to terms or year fractions
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		paymentDates        The payment dates
	*  @param [out]		terms               The corresponding terms values
	*/
    DoubleVector convertCurveDatesToTerms( const AQLString& curveCollection, const DateVector& paymentDates )
    {
        const AQLDate asOfDate = getCurveAsOfDate( curveCollection );
        const DoubleVector terms = convertCurveDatesToTerms( asOfDate, paymentDates );
        return terms;
    }

    /* @brief			function to format and convert a curve index from uppercase to camel case - needed for object pool data searches
	*  @param [in]		curveIndex  	        The unformatted curve index, usually in uppercase
    *  @param [out]		formattedCurveIndex     The object pool formatted curve index, usually in camel case
	*/
    AQLString formatCurveIndex( const AQLString& curveIndex )
    {
        AQLString upperCaseCurveIndex = curveIndex;
        upperCaseCurveIndex.toUpper();

        // Rules:
        // 1. OIS Curve Index must remain in uppercase
        // 2. STD Curve Index must remain in uppercase
        // 3. Basis Curve Index must be camelcase for for the basis suffix i.e 3M6MBASIS should be 3M6MBasis
        // 4. Xccy Curve Index must be camelcase i.e XCCYBASIS should be XccyBasis


        // Basis Curves 1's
        // ----------------
        if ( upperCaseCurveIndex == "1M3MBASIS" )
        {
            return "1M3MBasis";
        }

        if ( upperCaseCurveIndex == "1M6MBASIS" )
        {
            return "1M6MBasis";
        }

        if ( upperCaseCurveIndex == "1M12MBASIS" )
        {
            return "1M12MBasis";
        }

        // Basis Curves 3's
        // ----------------
        if ( upperCaseCurveIndex == "3M6MBASIS" )
        {
            return "3M6MBasis";
        }

        if ( upperCaseCurveIndex == "3M12MBASIS" )
        {
            return "3M12MBasis";
        }

        // Basis Curves 6's
        // ----------------
        if ( upperCaseCurveIndex == "6M12MBASIS" )
        {
            return "6M12MBasis";
        }

        // Xccy Basis
        // ----------------
        if ( upperCaseCurveIndex == "XCCYBASIS" )
        {
            return "XccyBasis";
        }

        // Return the original curve index if no formatting required
        return curveIndex;

    }

    /* @brief			Function to get a complete list of curve index alias' given one of the curve indices
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [in]		throwOnError        throw on error if TRUE or return empty string vector if FALSE
	*  @param [out]		A vector of all curve index alias' used
	*/
    AQLStringVector curveIndexAliasList( const AQLString& curveCollection, const AQLString& curveIndex, const bool throwOnError )
    {
        // *** Important - We must format the curve index when searching the object pool, since the object pool is case sensitive and AQObj is all uppercase ***
        const AQLString formattedCurveIndex = formatCurveIndex( curveIndex );

        // Results Place Holder
        AQLStringVector curveIndexList;

        // 1. Initialize the Object Pool
        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();					
        
        // 2. Initialize Yield Curve Properties					
        AQLString yieldProName = "PRO_YIELD_" + curveCollection;					
        AQLObjectHolder ehYieldCurveProperties = objPool.getObject( yieldProName );				
        
		// Throw if Curve Index Not Found or Return Empty Alias List
		if ( !ehYieldCurveProperties.isDefined() )
		{
			if (throwOnError)
			{
				AQ_THROW("Curve '" + formattedCurveIndex + "' does not exist in collection '" + curveCollection + "'")
			}
			else
			{
				return curveIndexList; // empty list
			}
		}
        
        CurveCalibrationData *ycProperties = &dynamic_cast<CurveCalibrationData &>( ehYieldCurveProperties.get() );

        // 3. Get the Yield Curve Properties Map of CurveIndices and Corresponding Markets or Curve Types
	    const std::map<AQLString, AQLString>& mapOfCurveIndexAndMarket = ycProperties->getAssignedCurveMktMap();				
        AQLString searchForCurveMarketType = ycProperties->getMarketForCurve( formattedCurveIndex );

        // 4. Iterate Over the CurveIndexAndMarket map and extract all curve indices mapped to the market or curve type
        for ( std::map<AQLString, AQLString>::const_iterator it = mapOfCurveIndexAndMarket.begin(); it != mapOfCurveIndexAndMarket.end(); ++it )
		{
			AQLString currentMarketType = it->second;
            if ( currentMarketType == searchForCurveMarketType )
			{
                AQLString currentCurveIndex = it->first;
				curveIndexList.push_back( currentCurveIndex );
			}
		}

        // In the case where we do not find the curve in the object pool "getAssignedCurveMktMap"  in step 3.
        if( curveIndexList.empty() )
        {
            curveIndexList.push_back( curveIndex );
        }
        
        return curveIndexList;
    }

     /* @brief			Function to get a complete list of curve index alias' given one of the curve indices
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [out]		A vector of all curve index alias' used
	*/
    StandardStringVector curveIndexAliasListAsStandardString( const StandardString& curveCollection, const StandardString& curveIndex )
    {
        // *** Important - We must format the curve index when searching the object pool, since the object pool is case sensitive and AQObj is all uppercase ***
        const AQLString formattedCurveIndex = formatCurveIndex( curveIndex.c_str() );

        // Results Place Holder
        StandardStringVector curveIndexList;

        // 1. Initialize the Object Pool
        AQLObjectPool& objPool = etrading::getDataInstance()->getObjectPool();					
        
        // 2. Initialize Yield Curve Properties					
        StandardString yieldProName = "PRO_YIELD_" + curveCollection;					
        AQLObjectHolder ehYieldCurveProperties = objPool.getObject( yieldProName.c_str() );				
        
        AQ_REQUIRE( ehYieldCurveProperties.isDefined(), "Curve '" + formattedCurveIndex + "' does not exist in collection '" + curveCollection.c_str() + "'" )
        
        CurveCalibrationData *ycProperties = &dynamic_cast<CurveCalibrationData &>( ehYieldCurveProperties.get() );

        // 3. Get the Yield Curve Properties Map of CurveIndices and Corresponding Markets or Curve Types
	    const std::map<AQLString, AQLString>& mapOfCurveIndexAndMarket = ycProperties->getAssignedCurveMktMap();				
        AQLString searchForCurveMarketType = ycProperties->getMarketForCurve( formattedCurveIndex );

        // 4. Iterate Over the CurveIndexAndMarket map and extract all curve indices mapped to the market or curve type
        for ( std::map<AQLString, AQLString>::const_iterator it = mapOfCurveIndexAndMarket.begin(); it != mapOfCurveIndexAndMarket.end(); ++it )
		{
			AQLString currentMarketType = it->second;
            if ( currentMarketType == searchForCurveMarketType )
			{
                AQLString currentCurveIndex = it->first;
				curveIndexList.push_back( currentCurveIndex.getCString() );
			}
		}

        // In the case where we do not find the curve in the object pool "getAssignedCurveMktMap"  in step 3.
        if( curveIndexList.empty() )
        {
            curveIndexList.push_back( curveIndex );
        }
        
        return curveIndexList;
    }

    /* @brief			Function to identify if a curve is an STD curve
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveIndex          A single curve index in the curve collection
	*  @param [out]		A boolean of true if STD curve and false otherwise
	*/
    bool isSTDCurve( const AQLString& curveCollection, const AQLString& curveIndex )
    {
        const AQLString curveType = etrading::getCurveType( curveCollection, curveIndex );
		const bool isSTDCurve = (curveType == SWAP );
        return isSTDCurve;
    }
}