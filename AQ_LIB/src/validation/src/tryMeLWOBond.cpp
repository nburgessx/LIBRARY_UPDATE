#include "tryMeLWOBond.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ObjectUtilities.h"
#include "AQOUtilities.h"
#include "RecordMacros.h"
#include "BondFactory.h"
#include "Bond.h"
#include "BondQuote.h"
#include "FixedBond.h"
#include "FloatingBond.h"
#include "SwapValidation.h"
#include "BondSpreadCalculation.h"
#include <omp.h>

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{

    /* @brief			validation interface for the meLWOBondCreate method
	*  @param [in]		bondObjectName	    Bond object name
	*  @param [in]		bondLVB			    Bond Label Value Block
    *  @param [in]		scheduleLVB			Schedule Label Value Block
	*  @param [in]		validateKeys	    True to validate the all keys provided are valid. Default to True
	*  @return			Bond Handle
	*/
	std::string tryMeLWOBondCreate( const std::string& bondObjectName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB, bool validateKeys )
    {
        VALID_EXCEPTION_START
        
		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, bondLVB, scheduleLVB, validateKeys );

        // Use the Bond Factory to validate and create the bond
        etrading::BondPtr bondPtr = etrading::createBond( bondObjectName, bondLVB, scheduleLVB, validateKeys ); // bondSchedule = nullptr; The schedule class will build the schedule object if null. Note sometimes the end-user will want to provide the schedule

        // Register the Bond in the LWO Cache
        etrading::registerToCache< etrading::Bond >( bondPtr );

        std::string result = bondObjectName;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result );
        
        VALID_EXCEPTION_END
    }

   /* @brief			validation interface for the meLWOBondCreateFromGeneratorLVB method. Create a Bond from a BondGenerator
    *  @param [in]		bondObjectName          Bond cached name to use
	*  @param [in]		bondGeneratorName       Bond Generator name to use
    *  @param [in]		bondExpressionLVB       A Label Value Block containing dataValues specific to this bond
    *  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
    *  @return			Bond Handle string
	*/
	std::string tryMeLWOBondCreateFromGenerator( const std::string& bondObjectName, const std::string& bondGeneratorName, const LabelValueBlock& expressionLVB, bool validateKeys )
	{
	    VALID_EXCEPTION_START
        
		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, bondGeneratorName, expressionLVB, validateKeys );
		
		etrading::validateKeysForLVB( etrading::BondGenerator::bond_expression_lvbKeys(), expressionLVB.getKeys(), validateKeys );

		// Use the Bond Factory to validate and create the bond
        etrading::BondPtr bondPtr = etrading::createBondFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, validateKeys ); 

		// Register the Bond in the LWO Cache
        etrading::registerToCache< etrading::Bond >( bondPtr );
		
		std::string result = bondObjectName;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result );
        
        VALID_EXCEPTION_END
	
	}

    /* @brief			validation interface for the meLWOBondDisplay method
	*  @param [in]		bondObjectName		Bond object name
	*  @return			Bond Input Parameters
	*/
	AnyTypeMatrix tryMeLWOBondDisplay( const std::string& bondObjectName )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName );

		auto bond = etrading::getBond( bondObjectName );
        auto result = bond->getInputParameters().toAnyTypeMatrix();

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result );

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the meLWOBondDisplaySchedule method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		showColumnHeaders	Show Schedule Column Headers; Defaults to True
	*  @return			Bond Schedule
	*/
	AnyTypeMatrix tryMeLWOBondDisplaySchedule( const std::string& bondObjectName, const bool& showColumnHeaders, const std::vector<std::string>& columnList )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, showColumnHeaders );

		auto bond = etrading::getBond( bondObjectName );

        // Get the column headers and row data pair and convert to anyMatrixType
        bool showBespokeProperties = false;
        auto headerBodyPair = bond->getSchedule()->view( showBespokeProperties, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList) );
        auto result = etrading::mergeHeaderAndBodyPair( headerBodyPair );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result );

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meLWOBondDisplayCashflows method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		settlementDate		    settlement date
    *  @param [in]		yield		            yield
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @param [in]		showColumnHeaders	Show Schedule Column Headers; Defaults to True
	*  @param [in]  	columnList              Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Bond Cashflows
	*/
	AnyTypeMatrix tryMeLWOBondDisplayCashflows( const std::string& bondObjectName, const AQLDate& settlementDate, const double& yield, const std::string& yieldCalculationType, const bool& showColumnHeaders, const std::vector<std::string>& columnList )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDate, yield, showColumnHeaders, yieldCalculationType);

        auto bond   = etrading::getBond( bondObjectName );

        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);

        auto result = bond->view( settlementDate, yield, yieldCalcType, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList) );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result );

		VALID_EXCEPTION_END
	}


    /* @brief			validation interface for the meLWOBondDirtyPrice method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryMeLWOBondDirtyPrice( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType );

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );

        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);

        std::vector< double > dirtyPrices;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
            double dirtyPrice         = bond->dirtyPrice( settlementDate, yield, yieldCalcType );

            dirtyPrices.push_back( dirtyPrice );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), dirtyPrices );

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the meLWOBondAccruedInterestDays method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @return			Bond Accrual Day(s)
	*/
	std::vector< int > tryMeLWOBondAccruedInterestDays( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates );

        auto bond                                   = etrading::getBond( bondObjectName );

        std::vector< int > results;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate             = settlementDates[i];
            const int accruedInterestDays           = bond->accruedInterestDays( settlementDate );
            
            results.push_back( accruedInterestDays );
        }

         // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( results );

		VALID_EXCEPTION_END
    }

     /* @brief			validation interface for the meLWOBondAccruedInterest method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryMeLWOBondAccruedInterest( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates );

        auto bond                           = etrading::getBond( bondObjectName );

        std::vector< double > results;

		etrading::ValuationSettings valuationSettings;

        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
			valuationSettings.setSettlementDate( settlementDate );
			etrading::DataProvider dataProvider(valuationSettings);

            double accruedInterest    = bond->accruedInterest( dataProvider );

            //accruedInterest = etrading::roundToNearest(accruedInterest, bond->getCurrency().c_str());

            results.push_back( accruedInterest );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), results );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondAccruedInterest method
	*  @param [in]		bondObjectName			Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @return			The bond accrued interest
	*/
	double tryMeLWOBondAccruedInterest( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, valuationSettingsLVB );

        auto bond = etrading::getBond( bondObjectName );

		etrading::ValuationSettings valuationSettings( valuationSettingsLVB );
		etrading::DataProvider dataProvider( valuationSettings );

        double accruedInterest    = bond->accruedInterest( dataProvider );

        //accruedInterest = etrading::roundToNearest(accruedInterest, bond->getCurrency().c_str());

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), accruedInterest );

		VALID_EXCEPTION_END
	}


    /* @brief			validation interface for the meLWOBondCleanPrice method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryMeLWOBondCleanPrice( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType );

        if ( yields.size() == 0 )
        {
            throw AQLCoreInvalidData("#Error: No Yield data provided.", __FILE__, __LINE__ );
        }

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );
        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);

        std::vector< double > cleanPrices;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
            double cleanPrice                = bond->cleanPrice( settlementDate, yield, yieldCalcType );
            
            //cleanPrice = bond->truncatePriceOrYield(cleanPrice);

            cleanPrices.push_back( cleanPrice );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), cleanPrices );

		VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meLWOBondPrice method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryMeLWOBondPrice( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType );

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );
        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);

        std::vector< double > prices;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
            double price         = bond->price( settlementDate, yield, yieldCalcType );
            
            prices.push_back( price );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), prices );

		VALID_EXCEPTION_END
    }
    

    /* @brief			validation interface for the meLWOBondQuote method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryMeLWOBondQuote( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType );

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );
        auto yieldCalcType                  = bond->getYieldCalulationType(yieldCalculationType);

        // Calculate the Bond Prices
        std::vector< double > prices;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
            double price                    = bond->price( settlementDate, yield, yieldCalcType );
            
            prices.push_back( price );
        }

        // Format the Raw Prices the Bond Quotes
        etrading::BondQuoteConventionEnum quoteConvention = bond->getBondQuoteConventionEnum() ;
        std::vector< double > quotes = etrading::convertPricesToQuotes( prices, quoteConvention );
        return quotes;
        
        
		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT( bondObjectName, std::string(), quotes );

		VALID_EXCEPTION_END
    }


    /* @brief			Helper function to calculate the yield of a bond given the bond object
	*  @param [in]		bondObject		    Bond Object
    *  @param [in]		settlementDate		settlement date
    *  @param [in]		price		        bond price
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond Yield
	*/
    double tryMeLWOBondYieldFromObject( const etrading::BondPtr bondObject, const AQLDate & settlementDate, const double & price, const std::string& yieldCalculationType )
    {

        auto yieldCalcType = bondObject->getYieldCalulationType(yieldCalculationType);        
        
        double yield = bondObject->yield( settlementDate, price, yieldCalcType );
        
        return yield;
    }

 
    /* @brief			Helper function to optimize the performance of the bond yield calculation
	*  @param [in]		bondObject		    Bond object pointer
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		prices		        bond prices, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond Yield(s)
	*/
    std::vector< double > tryMeLWOBondYieldOptimized( const etrading::BondPtr bondObject, const std::vector< AQLDate >& settlementDates, const std::vector< double >& prices, const std::string& yieldCalculationType  )
    {

		// Find the maximum number of hardware threads (independent cores) on this machine
        const int nThreads          = omp_get_max_threads();
        const int nSettlementDates	= static_cast<int>( settlementDates.size() );

        // Clone Bonds for Threading as a Multi-Process. We allocate one bond per thread.
        std::vector< etrading::BondPtr > myBonds;
        for ( int h = 0; h < nThreads; ++h )
        {
            auto thisBond = bondObject->clone();
            std::string newName = thisBond->getBondObjectName() + std::to_string( static_cast<long long>( h ) );
            thisBond->changeName( newName );
            myBonds.push_back( thisBond );
        }

		// Run the yield calculation on multiple threads
		std::vector< double > yields( nSettlementDates );
		#pragma omp parallel for num_threads(nThreads)
		for (int i=0; i<nSettlementDates; i++)
		{
			int id=omp_get_thread_num();
			yields[i] = tryMeLWOBondYieldFromObject( myBonds[id], settlementDates[i], prices[i], yieldCalculationType );
		}

        return yields;
    }

    /* @brief			validation interface for the meLWOBondYield method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		settlementDates		    settlement dates
    *  @param [in]		prices		            bond prices, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @param [in]      optimizePerformance     Use multi-threading to optimize performance, defaults to FALSE
    *  @return			Bond Yield(s)
	*/
	std::vector< double > tryMeLWOBondYield( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& prices, const std::string& yieldCalculationType , const bool & optimizePerformance )
    {
        VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, prices, yieldCalculationType , optimizePerformance );

        if( settlementDates.size() != prices.size() )
        {
            throw AQLCoreInvalidData("#Error: Inconsistent number of dates and prices.", __FILE__, __LINE__ );
        }

        auto bondObject = etrading::getBond( bondObjectName );
        std::vector< double > yields;
        
        // Set Minimum Number of Calculations for Optimization
        bool calculateUsingThreads = false;
        const unsigned int minCalculationsForOptimization = 100;
        if ( settlementDates.size() > minCalculationsForOptimization ) 
        { 
            calculateUsingThreads = true;
        }

        // Optimize & thread the Bond Yield Calculation, minimum of 100 yield calculations are required
        if( calculateUsingThreads && optimizePerformance )
        {
            // Use Optimized Threaded Calculation
            // ----------------------------------
            yields = tryMeLWOBondYieldOptimized( bondObject, settlementDates, prices, yieldCalculationType );
        }
        else
        {
            // Regular Sequential Calculation
            // ------------------------------
            auto yieldCalcType = bondObject->getYieldCalulationType(yieldCalculationType);        

            for ( unsigned int i = 0; i < settlementDates.size(); ++i )
            {
                const AQLDate settlementDate     = settlementDates[i];
                const double price              = prices[i];
                const double yield              = bondObject->yield( settlementDate, price, yieldCalcType );
                
                yields.push_back( yield );
            }            
        }
		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), yields );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondYield method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		price		            bond price, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Yield
	*/
	double tryMeLWOBondYield( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double price, const std::string& yieldCalculationType )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, valuationSettingsLVB, price, yieldCalculationType );

		auto bondObject = etrading::getBond( bondObjectName );
		auto yieldCalcType = bondObject->getYieldCalulationType(yieldCalculationType);

		etrading::ValuationSettings valuationSettings( valuationSettingsLVB );
		etrading::DataProvider dataProvider( valuationSettings );

		double yield = bondObject->yield( dataProvider, price, yieldCalcType );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), yield );

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the tryMeLWOBondBPVPerTick method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		price		            bond price, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType	yield calculation type	        
	*  @param [in]		tickSize				bond tick size, defaults to 1/32 for US Treasuries       
    *  @return			Bond Basis Point Change per TickSize
	*/
	double tryMeLWOBondBPVPerTick( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double price, const std::string& yieldCalculationType, const double tickSize )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, valuationSettingsLVB, price, yieldCalculationType, tickSize );

		auto bondObject = etrading::getBond( bondObjectName );
		auto yieldCalcType = bondObject->getYieldCalulationType(yieldCalculationType);

		etrading::ValuationSettings valuationSettings( valuationSettingsLVB );
		etrading::DataProvider dataProvider( valuationSettings );

		const double yield			= bondObject->yield( dataProvider, price, yieldCalcType );
		const double yieldPlusTick	= bondObject->yield( dataProvider, price+tickSize, yieldCalcType );

		// Result for a down price bump to make the change in yield positive
		const double changeInYieldInBasisPoints	= ( yield - yieldPlusTick ) * 10000;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), changeInYieldInBasisPoints );

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the meLWOBondCompoundYield method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		Settlement dates
    *  @param [in]		prices		        User input prices
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Compound Yields used as bond discount factors
	*/
    std::vector< double > tryMeLWOBondCompoundYields( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& prices, const std::string& yieldCalculationType )
    {
      	VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, prices, yieldCalculationType  );

        if ( settlementDates.size() != prices.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );

        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);        

        std::vector< double > compoundYields;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double price              = prices[i];
            double compoundYield         = bond->compoundYieldFromQuotedPrice( settlementDate, price, yieldCalcType );

            //compoundYield = bond->truncatePriceOrYield(compoundYield*100)/100;
            
            compoundYields.push_back( compoundYield );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), compoundYields );

		VALID_EXCEPTION_END

    }

	/* @brief			validation interface for the meLWOBondDV01Numerical method. Calculate the DV01(s) via numerical bumping.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
	*  @param [in]		bumpSize		    size of the yield bump in bps
	*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Bond DV01(s)
	*/
	std::vector< double > tryMeLWOBondDV01Numerical( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const double bumpSize, const AQLString& bumpMode, const std::string& yieldCalculationType )
	{

	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType  );

		double bumpAmount = ( bumpSize == 0.0 ) ? 0.01 : bumpSize;
		AQLString bumpType = ( bumpMode == "" )  ? "UP" : bumpMode;

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );
        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);        

        std::vector< double > dv01s;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
			double dv01                     = bond->dv01Numerical( settlementDate, yield, yieldCalcType, bumpAmount, bumpType );
                        
            dv01s.push_back( dv01 );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), dv01s );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondDV01 method. Calculate the DV01(s) analytically.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType	yield calculation type	        
	*  @param [in]		bumpSize		    size of the yield bump in bps
	*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
    *  @return			Bond DV01(s)
	*/
	std::vector< double > tryMeLWOBondDV01( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType )
	{

	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType  );

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );
        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);        

        std::vector< double > dv01s;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
			double dv01                     = bond->dv01( settlementDate, yield, yieldCalcType );
                        
            dv01s.push_back( dv01 );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), dv01s );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondModifiedDuration method. Calculate the Modified Duration analytically.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType	yield calculation type	        
	*  @param [in]		bumpSize		    size of the yield bump in bps
	*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
    *  @return			Bond Duration(s)
	*/
	std::vector< double > tryMeLWOBondModifiedDuration( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType )
	{

	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates, yields, yieldCalculationType  );

        if ( settlementDates.size() != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        auto bond                           = etrading::getBond( bondObjectName );
        auto yieldCalcType = bond->getYieldCalulationType(yieldCalculationType);        

        std::vector< double > durations;
        for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
			double duration                 = bond->modifiedDuration( settlementDate, yield, yieldCalcType );
                        
            durations.push_back( duration );
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), durations );

		VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meLWOBondOisSpread method. Calculate the O-Spread (BondYield to OIS ParRate spread) using linear interpolation on the OIS ParRate on standard tenors.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		marketObjectName	Market Data object name
    *  @param [in]		settlementDates		Settle Dates for the bond 
    *  @param [in]		bondYields		    Bond yields
    *  @return			A vector of O-Spreads
	*/
    std::vector< double > tryMeLWOBondOisSpread( const std::string& bondObjectName, const std::string& marketObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields )
	{

		size_t settleDatesSize = settlementDates.size();

	    if ( settleDatesSize != yields.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Yield Input and/or Inconsistent number of dates and yields.", __FILE__, __LINE__ );
        }

        std::vector< double > spreads(settleDatesSize);

		for ( unsigned int i = 0; i < settleDatesSize; ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];
            const double yield              = yields[i];
			double spread                   = tryMeLWOBondOisSpread( bondObjectName, marketObjectName, settlementDate, yield );
                        
            spreads[i] = spread;
        }

        return spreads;

    }

  

    /* @brief			Helper method for the tryMeLWOBondSpreadOIS method. Calculate the O-Spread (BondYield to OIS ParRate spread) using linear interpolation on the OIS ParRate on standard tenors.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		marketObjectName	Market Data object name
	*  @param [in]		settleDate		    Settle Date for the bond 
    *  @param [in]		bondYield		    Bond yield
    *  @return			Bond Yield and OIS ParRate Spread
	*/
    double tryMeLWOBondOisSpread( const std::string& bondObjectName, const std::string& marketObjectName, const AQLDate& settleDate, const double& bondYield)
    {
	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(marketObjectName, std::string(), marketObjectName, bondObjectName, bondYield);

        auto marketObject = etrading::getCurveMarketData( marketObjectName );

        auto settleDates = boost::assign::list_of(settleDate);

        AQLStringMatrix oisParRateMatrix = marketObject->toLAStringMatrix( toString(etrading::OIS_MARKETDATA) );

        auto bond = etrading::getBond( bondObjectName );

		auto bondEffectiveDt = bond->getSchedule()->getEffectiveDate();
        auto bondMaturityDt = bond->getSchedule()->getMaturityDate();

        auto rollDayInput = bond->getSchedule()->getRollDayInput();
        auto rollConvention = etrading::getRollConvection(bondEffectiveDt, bondMaturityDt, rollDayInput);
        auto calendar = bond->getSchedule()->getAccrualCalendar();

        auto dayCount = bond->getSchedule()->getAccrualDaycount();

		const AQLString businessDayAdj = "NO_CHANGE"; // swap maturity is always unadjsted when using swap tenor string as input.

        size_t parRateSize = oisParRateMatrix.size();
        std::map<int, double> oisRates;

        for ( size_t i = 0; i < parRateSize; ++i )
	    {
            //*** Expected input Columns at least two columns: Tenor, Rate
            auto curRow = oisParRateMatrix[i];

            if (curRow.size() < 2)
            {
              throw AQLCoreInvalidData( "#Error: OIS par rate matrix must have at least 2 columns: Tenor, Rate" , __FILE__, __LINE__ );
            }

			//Tenor
			AQLString tenor = curRow[0];

			//OIS Rate
			AQLDate maturityDate;

			if (   tenor.findString("BOJ") != -1       // Bank of Japan
                || tenor.findString("EUSF") != -1      // European Central Bank (Bloomberg Ticker)
                || tenor.findString("ECB") != -1       // European Central Bank
                || tenor.findString("BOE") != -1       // Bank of England
                || tenor.findString("CB") != -1        // *** GENERIC *** Central Bank Swaps
                || tenor.findString("MPC") != -1 )     // *** GENERIC *** Monetary Policy Committee Swaps
			{
				if (curRow.size() < 4)
				{
					throw AQLCoreInvalidData("#Error: OIS par rate matrix short term needs StartDate and EndDate",__FILE__,__LINE__);
				}

				const AQLDate& startDate = etrading::stringToDate( curRow[2], "#Error: Invalid 'startDate'." ); 
				const AQLDate& endDate = etrading::stringToDate( curRow[3], "#Error: Invalid 'endDate'." ); 
				double yearFraction = etrading::getYearFraction(startDate, endDate, dayCount);

				maturityDate = etrading::getDateFromYearFraction(settleDate, yearFraction, dayCount);

			}
			else
			{
				maturityDate = etrading::getDateFromTenor(settleDates, tenor, businessDayAdj, calendar, rollConvention)[0];
			}

		   int	maturityDateInteger = (int)etrading::fromLADateToDouble(maturityDate);
	
		   AQLString rawRate = curRow[1];
		   double rate = 0.0;
		   if (rawRate.size() != 0)
			{
				rate = rawRate.getDoubleValue();
			}
			else
			{
				continue;
			}

            oisRates[maturityDateInteger] = rate;
        }

		if (oisRates.size() == 0)
		{
	        throw AQLCoreInvalidData( "#Error: OIS par rate matrix has no par rate." , __FILE__, __LINE__ );
		}

		// Retrieve all keys
		std::vector<int> maturityDates;
		maturityDates.reserve(oisRates.size());
		for(auto it = oisRates.begin(); it != oisRates.end(); ++it) 
		{
		  maturityDates.push_back(it->first);
		}

        // Check bondMaturity is between which two standard OIS Swap Maturity Date
        int bondMaturity = (int)etrading::fromLADateToDouble(bondMaturityDt);

		// Find uppper maturity date, which is the First date in maturityDates GREATER THAN OR EQUAL to the bondMaturity
        auto it = std::lower_bound( maturityDates.begin(), maturityDates.end(), bondMaturity );

        int lowerMaturity = 0;
        int upperMaturity = 0;

		// upper maturity date NOT found, or only one date in the list
		if ( it == maturityDates.end() || maturityDates.size() == 1)
        {
			upperMaturity = maturityDates.back();
            lowerMaturity = upperMaturity;
        }
		// upper maturity date found
		else
		{
			upperMaturity = *it;

			auto upperMaturityIndex   = std::distance( maturityDates.begin(), it );
			lowerMaturity = maturityDates[upperMaturityIndex - 1];
		}
       
        std::vector<double> matchedMaturities = boost::assign::list_of(lowerMaturity)(upperMaturity);
        std::vector<double> matchedOisRates = boost::assign::list_of(oisRates[lowerMaturity])(oisRates[upperMaturity]);

        double interpolatedOISParRate = etrading::interpolate(matchedMaturities, matchedOisRates, bondMaturity, etrading::LINEAR_INTERPOLATION);

        //spread in basis point
        double oSpread = (bondYield  - interpolatedOISParRate) * 10000;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), oSpread );

		VALID_EXCEPTION_END

    }


	/* @brief			Validation interface for the meLWOBondLastCouponDate method. Calculate the bond's last coupon date.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		Bond's Settlement Dates 
    *  @return			The bond's last coupon date.
	*/
    std::vector< double > tryMeLWOBondLastCouponDate( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates)
    {
	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDates);

		auto bond = etrading::getBond( bondObjectName );

		std::vector< double > lastCouponDates(settlementDates.size());
	
		for ( unsigned int i = 0; i < settlementDates.size(); ++i )
        {
            const AQLDate settlementDate     = settlementDates[i];

			AQLDate lastCouponDate = bond->getBondLastCouponDate(settlementDate);

			lastCouponDates[i] = etrading::fromLADateToDouble(lastCouponDate);
        }

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), lastCouponDates );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondPriceFromDirtyToClean method. 
	*  @param [in]		bondObjectName			Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		dirtyPrice				Bond dirtyPrice
    *  @return			Bond Clean Price
	*/
    double tryMeLWOBondPriceFromDirtyToClean( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double dirtyPrice )
	{
	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, valuationSettingsLVB, dirtyPrice);

		etrading::ValuationSettings valuationSettings( valuationSettingsLVB );
		etrading::DataProvider dataProvider( valuationSettings );

		auto bond = etrading::getBond( bondObjectName );
		const double cleanPrice = bond->priceFromDirtyToClean( dirtyPrice, dataProvider );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), cleanPrice);

		VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meLWOBondPriceFromCleanToDirty method. 
	*  @param [in]		bondObjectName			Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		cleanPrice				Bond cleanPrice
    *  @return			Bond Dirty Price
	*/
    double tryMeLWOBondPriceFromCleanToDirty( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double cleanPrice )
	{
	     VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, valuationSettingsLVB, cleanPrice);

		etrading::ValuationSettings valuationSettings( valuationSettingsLVB );
		etrading::DataProvider dataProvider( valuationSettings );

		auto bond = etrading::getBond( bondObjectName );
		const double dirtyPrice = bond->priceFromCleanToDirty( cleanPrice, dataProvider );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), dirtyPrice );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondPriceFromCreditModel function. This calculates the price of a defaultable bond.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDate		Bond's Settlement Date
    *  @param [in]		creditModelName		Credit model name
    *  @return			The calculated Bond price
	*/
	double tryMeLWOBondPriceFromCreditModel( const std::string& bondObjectName, const AQLDate& settlementDate, const std::string& creditModelName )
	{
		VALID_EXCEPTION_START
	
		// Recording of inputs for testing and playback
		RECORD_INPUTS( bondObjectName, settlementDate, creditModelName );

		auto bond = etrading::getBond( bondObjectName );
		auto creditModel = etrading::getCreditModel( creditModelName );

		const double price = bond->priceFromCreditModel( settlementDate, *creditModel );

		// Recording of outputs for testing and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( price );

		VALID_EXCEPTION_END
	}


	/* @brief Creates an AQOBondGenerator object, containing all of the curve properties.
	 * @param [in] objectName        The name of the Curve Configuration object
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryMeLWOBondGeneratorCreate( const std::string& objectName,
                                             const std::vector<std::string>& propertyNames,
                                             const std::vector<TableInfo>& infoBlocks )
    {
		VALID_EXCEPTION_START
		
		// Perform initial basic sanity checks
        if ( infoBlocks.size() == 0 )
        {
			throw AQLCoreInvalidData( "#Error: No Property grid data provided to tryMeLWOBondGeneratorCreate", __FILE__, __LINE__ );
        }

        if ( propertyNames.size() != infoBlocks.size() )
        {
			throw AQLCoreInvalidData( ( boost::format( "#Error: Number of Property Names (%i) does not match number of grid data ranges (%i)." )
                                   % propertyNames.size()
                                   % infoBlocks.size() ).str().c_str(), __FILE__, __LINE__ );
        }

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file( decorateFilename( "tryMeLWOBondGeneratorCreate_inputs", objectName.c_str() ));
			file.write( "generatorFunction", "tryMeLWOBondGeneratorCreate" );
			file.write( "objectName", objectName );

			// Write out each propertyName and corresponding block of property config data
			for ( unsigned int i = 0; i < propertyNames.size(); i++ )
			{
				const AQLString propertyName( propertyNames[i].c_str() );
				const etrading::VariantMatrix& configData = std::get<2>( infoBlocks[ i ] );
				file.write( propertyName, transpose(configData) );
			}
		}

		// Verify that the supplied propertyNames have been set
        const bool hasAnEmptyName = std::any_of( propertyNames.cbegin(),
												 propertyNames.cend(),
												 []( const std::string & propertyName ) -> bool
        {
            return ( propertyName.empty() || propertyName == "" );
        } );

        if ( hasAnEmptyName )
        {
			throw AQLCoreInvalidData( ( boost::format( "#Error: One of the individual Property Names is empty or invalid (%s)" )
                                   % etrading::containerAsString( propertyNames ).c_str() ).str().c_str(), __FILE__, __LINE__ );
        }


		// Verify that the supplied propertyNames match the BondGeneratorEnum
		std::set<etrading::BondGeneratorEnum> enumSet;
		std::for_each( propertyNames.cbegin(),
					   propertyNames.cend(),
					   [&enumSet] ( const std::string & propertyName )
		{	
			etrading::BondGeneratorEnum bondGeneratorEnum = etrading::toBondGeneratorEnum( propertyName );
			enumSet.insert( bondGeneratorEnum );
		});

		// Create the BondGenerator object
		etrading::BondGenerator bondGenerator( objectName, propertyNames, infoBlocks );

		// ..  and store in the cache
        etrading::copyToCache<etrading::BondGenerator>( bondGenerator );

		if ( CreateDataFile::recordEnabled() )
		{
			CreateDataFile file( decorateFilename( "tryMeLWOBondGeneratorCreate_outputs", objectName.c_str() ));
			file.write( "output", objectName );
		}
				
		return objectName;

		VALID_EXCEPTION_END
    };

	/* @brief Displays the specified property of a AQOBondGenerator. If propertyName is blank, all properties are returned.
	 * @param [in] objectName        The BondGenerator object you wish to display
	 * @param [in] propertyName      The name of the property label-value block that you wish to display
	 * @param [out]                  A VariantMatrix containing a LabelValue block of properties
	 */
	const etrading::VariantMatrix tryMeLWOBondGeneratorDisplay( const std::string& objectName, const std::string& propertyKey )
    {
		VALID_EXCEPTION_START
			        
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, propertyKey );
		
		// Attempt to retrieve AQOCurveGenerator object from the LWO Cache
		auto bondGenerator = etrading::getBondGenerator( objectName );

		// extract just the data for the specified propertyKey
		return bondGenerator->viewInputParameters( propertyKey );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the meLWOBondCreateFromLVB method
	*  @param [in]		bondName		Bond name
	*  @param [in]		bondLVB			Bond input as single label value block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			bondName
	*/
	std::string tryMeLWOBondCreateFromLVB(const std::string& bondObjectName, const LabelValueBlock& bondLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, bondLVB, validateKeys);

		// Use the Bond Factory to validate and create the bond
		etrading::BondPtr bondPtr = etrading::createBondFromSingleLVB(bondObjectName, bondLVB, validateKeys);
		
		// Register the Bond in the LWO Cache
		etrading::registerToCache< etrading::Bond >(bondPtr);

		std::string result = bondObjectName;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result);

		VALID_EXCEPTION_END
	}


	/* @brief			Helper interface for the meLWOBondZSpread method. Calculate the Bond Z Spread.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settlementDate		Bond's Settlement Date (Valuation Date)
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		curveCollection     Curve collection
	*  @param [in]		forecastCurve       Forecast  curve
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			Z Spread
	*/
	double tryMeLWOBondZSpread(const std::string& bondObjectName, const AQLDate& settlementDate, const double& bondPrice, const std::string& curveCollection, const std::string& forecastCurve, const bool& continuouslyCompounding)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDate, bondPrice, curveCollection, forecastCurve, continuouslyCompounding);

		auto bond = etrading::getBond(bondObjectName);
		
		double bondDirtyPrice = bondPrice;
		if (bond->isCleanPrice())
		{
			bondDirtyPrice = bond->priceFromCleanToDirty(bondPrice, settlementDate);
		}

		double zSpread = etrading::calculateBondZSpread(bondDirtyPrice, bond, curveCollection, forecastCurve, settlementDate, continuouslyCompounding);
		
		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), zSpread);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondZSpread method. Bond Z Spread.
	*  @param [in]		bondObjectNames		Bond object name
	*  @param [in]		settlementDates		Bond's Settlement Date (Valuation Date)
	*  @param [in]		bondPrices		    Bond price
	*  @param [in]		curveCollections    Curve collection
	*  @param [in]		forecastCurves      Forecast Curve
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			Z Spreads
	*/
	DoubleVector tryMeLWOBondZSpreads(const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& bondPrices, const std::vector< std::string >& curveCollections, const std::vector< std::string >& forecastCurves, const bool& continuouslyCompounding)
	{

		size_t settleDatesSize = settlementDates.size();

		if (settleDatesSize != bondPrices.size() || settleDatesSize != curveCollections.size() || settleDatesSize != forecastCurves.size())
		{
			throw AQLCoreInvalidData("#Error: Inconsistent number of settlement dates, bondPrices, curveCollections, and forecastCurves.", __FILE__, __LINE__);
		}

		std::vector< double > zSpreads(settleDatesSize);

		for (unsigned int i = 0; i < settleDatesSize; ++i)
		{
			double zSpread = tryMeLWOBondZSpread(bondObjectName, settlementDates[i], bondPrices[i], curveCollections[i], forecastCurves[i], continuouslyCompounding);

			zSpreads[i] = zSpread;
		}

		return zSpreads;

	}

	/* @brief			Validation interface for the meLWOBondZSpreadFromRates method. Bond Z Spread.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settlementDate		Bond's Settlement Date (Valuation Date)
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		zeroRates			External ZeroRates
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			Z Spread
	*/
	double tryMeLWOBondZSpreadFromRates(const std::string& bondObjectName, const AQLDate& settlementDate, const double& bondPrice, const DoubleVector& zeroRates, const bool& continuouslyCompounding)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settlementDate, bondPrice, zeroRates, continuouslyCompounding);

		auto bond = etrading::getBond(bondObjectName);

		double bondDirtyPrice = bondPrice;
		if (bond->isCleanPrice())
		{
			bondDirtyPrice = bond->priceFromCleanToDirty(bondPrice, settlementDate);
		}

		double zSpread = etrading::calculateBondZSpread(bondDirtyPrice, bond, zeroRates, settlementDate, continuouslyCompounding);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), zSpread);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondForwardPrice method. Calculate Bond's forward price at forward settle date, from repo rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond forward price
	*/
	double tryMeLWOBondForwardPrice(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& price, const double& repoRate, const std::string& repoDayCount)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, forwardSettleDate, price, repoRate, repoDayCount);

		auto bond = etrading::getBond(bondObjectName);

		const double fwdPrice = bond->forwardPrice(price, settleDate, forwardSettleDate, repoRate, etrading::toDayCountEnum(repoDayCount));

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), fwdPrice);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondForwardReinvestedCoupon method. Calculate Bond's recieved coupon value at forward settle date, from repo rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond's recieved coupon value at forward settle date
	*/
	double tryMeLWOBondForwardReinvestedCoupon(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& price, const double& repoRate, const std::string& repoDayCount)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, forwardSettleDate, price, repoRate, repoDayCount);

		auto bond = etrading::getBond(bondObjectName);

		auto sumOfReceivedCoupons = bond->forwardReinvestedCouponValue(price, settleDate, forwardSettleDate, etrading::toDayCountEnum(repoDayCount), repoRate);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), sumOfReceivedCoupons);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondRepoRate method. Calculate Bond's implied repo rate from bond's forward price at settle date.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		forwardPrice		Bond's forward price at forward settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond implied repo rate
	*/
	double tryMeLWOBondRepoRate(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& price, const double& forwardPrice, const std::string& repoDayCount)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, forwardSettleDate, price, forwardPrice, repoDayCount);

		auto bond = etrading::getBond(bondObjectName);

		const double repoRate = bond->impliedRepoRate(price, settleDate, forwardSettleDate, forwardPrice, etrading::toDayCountEnum(repoDayCount));

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), repoRate);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondRepoRateFromFuture method. Calculate Bond's implied repo rate from bond's FUTURE price at settle date.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		deliveryDate		Bond future settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		futurePrice			Bond's future price at forward settle date 
	*  @param [in]		conversionFactor	Bond's conversion factor
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond implied repo rate
	*/
	double tryMeLWOBondRepoRateFromFuture(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& deliveryDate, const double& price, const double& futurePrice, const double& conversionFactor, const std::string& repoDayCount)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, deliveryDate, price, futurePrice, conversionFactor, repoDayCount);

		auto bond = etrading::getBond(bondObjectName);

		const double breakevenRepoRate = bond->impliedRepoRateFromFuture(price, settleDate, deliveryDate, futurePrice, conversionFactor, etrading::toDayCountEnum(repoDayCount));

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), breakevenRepoRate);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondFuturePrice method. Calculate Bond's Future price at future settle date, from repo rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		deliveryDate		Bond future settle date
	*  @param [in]		bondPrice			Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		conversionFactor	Bond's conversion factor
	*  @return			Bond Future price
	*/
	double tryMeLWOBondFuturePrice(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& deliveryDate, const double& bondPrice, const double& repoRate, const std::string& repoDayCount, const double& conversionFactor)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, deliveryDate, bondPrice, repoRate, repoDayCount, conversionFactor);

		auto bond = etrading::getBond(bondObjectName);

		const double futPrice = bond->futurePrice(bondPrice, settleDate, deliveryDate, repoRate, etrading::toDayCountEnum(repoDayCount), conversionFactor);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), futPrice);

		VALID_EXCEPTION_END
	}

	/* @brief			Help function for tryMeLWOBondConversionFactors with multiple settle dates as params. Calculate Bond conversionFactor based on future settle date and future's notional coupon rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		firstFutureSettleDate	Bond's first future settle date
	*  @param [in]		notionalBondCouponRate	Bond future notionalBondCouponRate
	*  @return			Bond conversionFactor against the future contract
	*/
	double tryMeLWOBondConversionFactor(const std::string& bondObjectName, const AQLDate& firstFutureSettleDate, const double& notionalBondCouponRate)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the object name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, firstFutureSettleDate, notionalBondCouponRate);

		auto bond = etrading::getBond(bondObjectName);

		const double cFactor = bond->conversionFactor(firstFutureSettleDate, notionalBondCouponRate);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), cFactor);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondGrossBasis method. Calculate the Bond Gross Basis
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		prices				Bond prices at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		futurePrice			Bond future price at future settle date, always clean
	*  @param [in]		conversionFactor	Conversion factor
	*  @return			Bond Gross Basis
	*/
	double tryMeLWOBondGrossBasis(const std::string& bondObjectName, const AQLDate& settleDate, const double& price, const double& futurePrice, const double& conversionFactor)
	{

		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, price, futurePrice, conversionFactor);

		auto bond = etrading::getBond(bondObjectName);

		const double grossBas = bond->grossBasis(price, settleDate, futurePrice, conversionFactor);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), grossBas);


		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondNetBasis method. Calculate the Bond Gross Basis
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		deliveryDate		Bond future settle date
	*  @param [in]		forwardPrice		Bond forwardPrice at delivery date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		futurePrice			Bond future price at delivery date, always clean
	*  @param [in]		conversionFactor	Conversion factor
	*  @return			Bond Gross Basis
	*/
	double tryMeLWOBondNetBasis(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& deliveryDate, const double& bondPrice, const double& actualRepoRate, const std::string& repoDayCount, const double& futurePrice, const double& conversionFactor)
	{

		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, deliveryDate, bondPrice, actualRepoRate, repoDayCount, futurePrice, conversionFactor);

		auto bond = etrading::getBond(bondObjectName);

		const double fwdPrice = bond->forwardPrice(bondPrice, settleDate, deliveryDate, actualRepoRate, etrading::toDayCountEnum(repoDayCount));

		const double netBas = bond->netBasis(fwdPrice, deliveryDate, futurePrice, conversionFactor);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), netBas);

		VALID_EXCEPTION_END
	}

	/* @brief			Validation interface for the meLWOBondCheapestToDeliverByImpliedRepoRate method. Calculate the Cheapest to deliver (CTD) bond via Implied Repo Rates, return the bond name.
	*  @param [in]		futurePrice			Future price
	*  @param [in]		settleDate			Bond's settle date
	*  @param [in]		deliveryDate		Bond's future settle date/delivery date
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		bondObjectNames		A list of Bond object names
	*  @param [in]		bondPrices			A list of bond prices at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		conversionFactors	A list of conversion factors
	*  @return			The Cheapest to deliver (CTD) bond's name
	*/
	std::string tryMeLWOBondCheapestToDeliver(const double& futurePrice, const AQLDate& settleDate, const AQLDate& deliveryDate, const std::string& repoDayCount, const std::vector<std::string>& bondObjectNames, const std::vector<double>& bondPrices, const std::vector<double>& conversionFactors)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback
		RECORD_INPUTS(futurePrice, settleDate, deliveryDate, repoDayCount, bondObjectNames, bondPrices, conversionFactors);

		const size_t expectedSize = bondObjectNames.size();
		AQ_REQUIRE(expectedSize != 0, "At least one underlying bond object is required");
		AQ_REQUIRE(expectedSize == bondPrices.size() && expectedSize == conversionFactors.size(), "The number of underlying bond object names, bond prices, and conversionFactors not matched");

		const auto repoDayC = etrading::toDayCountEnum(repoDayCount);

		double maxImpliedRepo = 0.0;
		int ctdIndex = 0;

		for (size_t i = 0; i < expectedSize; ++i)
		{
			auto bond = etrading::getBond(bondObjectNames[i]);
			const double bondPrice = bondPrices[i];
			const double conversionFactor = conversionFactors[i];

			const double impliedRepo = bond->impliedRepoRateFromFuture(bondPrice, settleDate, deliveryDate, futurePrice, conversionFactor, repoDayC);

			//initially assign to first index
			if (i == 0)
			{
				maxImpliedRepo = impliedRepo;
				ctdIndex = i;
			}
			else if (impliedRepo > maxImpliedRepo)
			{
				maxImpliedRepo = impliedRepo;
				ctdIndex = i;
			}
		}

		const std::string cheapestToDeliverBondName = bondObjectNames[ctdIndex];

		// Recording of outputs for testing and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(cheapestToDeliverBondName);

		VALID_EXCEPTION_END

	}

	/* @brief			Validation interface for the meLWOBondCheapestToDeliverByNetBasis method. Calculate the Cheapest to deliver (CTD) bond via Net Basis, return the bond name.
	*  @param [in]		futurePrice			Future price
	*  @param [in]		settleDate			Bond's settle date
	*  @param [in]		deliveryDate		Bond's future settle date/delivery date
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		bondObjectNames		A list of Bond object names
	*  @param [in]		bondPrices			A list of bond prices at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		conversionFactors	A list of conversion factors
	*  @param [in]		actualRepoRates		Actual repo rates
	*  @return			The Cheapest to deliver (CTD) bond's name
	*/
	std::string tryMeLWOBondCheapestToDeliverByNetBasis(const double& futurePrice, const AQLDate& settleDate, const AQLDate& deliveryDate, const std::string& repoDayCount, const std::vector<std::string>& bondObjectNames, const std::vector<double>& bondPrices, const std::vector<double>& conversionFactors, const std::vector<double>& actualRepoRates)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback
		RECORD_INPUTS(futurePrice, settleDate, deliveryDate, repoDayCount, bondObjectNames, bondPrices, conversionFactors, actualRepoRates);

		const size_t expectedSize = bondObjectNames.size();
		AQ_REQUIRE(expectedSize != 0, "At least one underlying bond object is required");
		AQ_REQUIRE(expectedSize == bondPrices.size() && expectedSize == conversionFactors.size() && expectedSize == actualRepoRates.size(), "The number of underlying bond object names, bond prices, conversionFactors, and actualRepoRates not matched");

		const auto repoDayC = etrading::toDayCountEnum(repoDayCount);

		double minNetBasis = 0.0;
		int ctdIndex = 0;

		for (size_t i = 0; i < expectedSize; ++i)
		{
			auto bond = etrading::getBond(bondObjectNames[i]);
			const double bondPrice = bondPrices[i];
			const double conversionFactor = conversionFactors[i];
			const double actualRepoRate = actualRepoRates[i];

			const double fwdPrice = bond->forwardPrice(bondPrice, settleDate, deliveryDate, actualRepoRate, repoDayC);

			const double netBasis = bond->netBasis(fwdPrice, deliveryDate, futurePrice, conversionFactor);

			//initially assign to first index
			if (i == 0)
			{
				minNetBasis = netBasis;
				ctdIndex = i;
			}
			else if (netBasis < minNetBasis)
			{
				minNetBasis = netBasis;
				ctdIndex = i;
			}
		}

		const std::string cheapestToDeliverBondName = bondObjectNames[ctdIndex];

		// Recording of outputs for testing and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(cheapestToDeliverBondName);

		VALID_EXCEPTION_END

	}

	/* @brief			validation interface for the meLWOBondCreateAUDNotionalBond method
	*  @param [in]		bondName			Bond object name
	*  @param [in]		settleDate			Settle date
	*  @param [in]		maturityDate		Maturity date or tenor
	*  @param [in]		couponRate			Coupon rate, default to 6%
	*  @param [in]		payReceive			Pay receive flag, default to RECEIVE
	*  @param [in]		calendar			Calendar, default to SYB
	*  @param [in]		frequency			Frequency, default to SEMI_ANNUAL
	*  @param [in]		dayCount			Daycount, default to ACT/ACT
	*  @param [in]		bondQuoteConv		Bond quote convention, default to blank
	*  @return			bondName
	*/
	std::string tryMeLWOBondCreateAUDNotionalBond(const std::string& bondObjectName, const AQLDate& settleDate, const std::string& maturityDate, const std::string& couponRate, const std::string& payReceive, const std::string& calendar, const std::string& frequency, const std::string& dayCount, const std::string& bondQuoteConv)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(bondObjectName, std::string(), bondObjectName, settleDate, maturityDate, couponRate, payReceive, calendar, frequency, dayCount, bondQuoteConv);

        StandardStringVector keys(17);
        keys[0]  = etrading::BOND_KEY::ISSUE_DATE;
        keys[1]  = etrading::IRS_KEY::MATURITY_DATE;
        keys[2]  = etrading::BOND_KEY::COUPON;
        keys[3]  = etrading::IRS_KEY::PAY_RECEIVE;
        keys[4]  = etrading::IRS_KEY::CALENDAR;
        keys[5]  = etrading::IRS_KEY::FREQUENCY;
        keys[6]  = etrading::IRS_KEY::DAYCOUNT;
        keys[7]  = etrading::BOND_KEY::BOND_QUOTE_CONVENTION;
        keys[8]  = etrading::BOND_KEY::BOND_TYPE;
        keys[9]  = etrading::BOND_KEY::CURRENCY;
        keys[10] = etrading::BOND_KEY::ISIN;
        keys[11] = etrading::BOND_KEY::YIELD_TYPE;
        keys[12] = etrading::BOND_KEY::IS_CLEAN_PRICE;
        keys[13] = etrading::BOND_KEY::CALCULATION_TYPE;
        keys[14] = etrading::BOND_KEY::FACE_VALUE;
        keys[15] = etrading::IRS_KEY::NOTIONAL_EXCHANGE;
        keys[16] = etrading::IRS_KEY::BUSINESSDAYADJUSTMENT; 

        StandardStringVector values(17);
        values[0]  = settleDate.stringWithFormat().getCString();
        values[1]  = maturityDate;
        values[2]  = couponRate.empty() ? "0.06" : couponRate; // coupon rate is always 6% but we still allow user to override it
        values[3]  = payReceive.empty() ? toString(etrading::RECEIVE_PAYRECEIVE_ENUM) : payReceive;
        values[4]  = calendar.empty() ? "SYB" : calendar;
        values[5]  = frequency.empty() ? toString(etrading::SEMI_ANNUAL_FREQUENCY) : frequency;
        values[6]  = dayCount.empty() ? toString(etrading::ACT_ACT_DAYCOUNT) : dayCount;
        values[7]  = bondQuoteConv.empty() ? "" : bondQuoteConv;
        values[8]  = toString(etrading::FIXED_BOND);
        values[9]  = toString(etrading::AUD);
        values[10] = "FUTURE_NOTIONAL_BOND_ISIN";
        values[11] = toString(etrading::YIELD_TO_MATURITY);
        values[12] = "TRUE";
        values[13] = toString(etrading::TYPE23_AUSTRALIAN_GOVERNMENT_BONDS);
        values[14] = "1000000";
        values[15] = toString(etrading::START_AND_END_NE);
        values[16] = toString(etrading::NO_CHANGE);

        // Create LVB
		LabelValueBlock bondLVB( keys, values );

		// Use the Bond Factory to validate and create the bond
		etrading::BondPtr notionalBond = etrading::createBondFromSingleLVB(bondObjectName, bondLVB, false);

		// Register the Bond in the LWO Cache
		etrading::registerToCache< etrading::Bond >(notionalBond);

		std::string result = bondObjectName;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the meUtilityBondAverageYield method, calculate the yield by averaging the underlying bonds' yields
	*  @param [in]		underlyingBondYields	Bond future's underlying bond yields
	*  @return			Aussie bond future notional bond's yield
	*/
	double tryMeUtilityBondAverageYield(const std::vector<double>& underlyingBondYields)
	{
		VALID_EXCEPTION_START

		RECORD_INPUTS(underlyingBondYields);

		const double expectedSize = underlyingBondYields.size();

		AQ_REQUIRE(expectedSize != 0, "At least one underlying bond yield is required");

		const double averageYield = std::accumulate(underlyingBondYields.begin(), underlyingBondYields.end(), 0.0) / expectedSize;

		RECORD_OUTPUTS_AND_RETURN_RESULT(averageYield);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the meUtilityBondYieldFromFuturePrice method, calculate the yield from futurePrice
	*  @param [in]		futurePrice		Bond future price
	*  @return			Aussie bond future notional bond's yield
	*/
	double tryMeUtilityBondYieldFromFuturePrice(const double& futurePrice)
	{
		VALID_EXCEPTION_START

		RECORD_INPUTS(futurePrice);

		const double notionalBondYield = 1.0 - futurePrice / 100.0;

		RECORD_OUTPUTS_AND_RETURN_RESULT(notionalBondYield);

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOBondFRNPriceFromDiscountMargin method
	*			Computes the price of a floating rate bond without requiring curves discount and forecast curves.
	*			The price is found given the following assumed values:  the discountMargin, assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
	*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
	*			See BBG CalcType 21 for calculation details.
	*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
	*
	* @param[in]	bondObjectName				The name of the cached floating bond object
	* @param[in]	settlementDate				settlement date
	* @param[in]	discountMargin				Additional discount rate for projected coupons
	* @param[in]	assumedRate					Assumed index rate used for projected coupons
	* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
	* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
	* @returns	The clean / dirty floating bond price	
	*/
	double tryMeLWOBondFRNPriceFromDiscountMargin( const std::string& bondObjectName, const AQLDate& settlementDate, const double& discountMargin, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDate, discountMargin, assumedRate, indexToNextCoupon, annualizedNextCouponRate )

		etrading::BondPtr bondPtr = etrading::getBond(bondObjectName);

		std::shared_ptr<etrading::FloatingBond> floatingBondPtr = std::dynamic_pointer_cast<etrading::FloatingBond>( bondPtr );
		AQ_REQUIRE( floatingBondPtr != nullptr, "Bond '" + bondObjectName + "' is not a floating rate bond." )

		const double price = floatingBondPtr->priceFromDiscountMargin( settlementDate, discountMargin, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT( bondObjectName, std::string(), price )

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOBondFRNPriceFromYield method
	*			Computes the price of a floating rate bond from the yield, without requiring curves discount and forecast curves.
	*			The price is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
	*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
	*			See BBG CalcType 21 for calculation details.
	*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
	*
	* @param[in]	bondObjectName				The name of the cached floating bond object
	* @param[in]	settlementDate				settlement date
	* @param[in]	yield						the bond yield to maturity
	* @param[in]	assumedRate					Assumed index rate used for projected coupons
	* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
	* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
	* @returns	The clean / dirty floating bond price	
	*/
	double tryMeLWOBondFRNPriceFromYield( const std::string& bondObjectName, const AQLDate& settlementDate, const double& yield, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDate, yield, assumedRate, indexToNextCoupon, annualizedNextCouponRate )

		etrading::BondPtr bondPtr = etrading::getBond(bondObjectName);

		std::shared_ptr<etrading::FloatingBond> floatingBondPtr = std::dynamic_pointer_cast<etrading::FloatingBond>( bondPtr );
		AQ_REQUIRE( floatingBondPtr != nullptr, "Bond '" + bondObjectName + "' is not a floating rate bond." )

		const double price = floatingBondPtr->priceFromYield( settlementDate, yield, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT( bondObjectName, std::string(), price )

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOBondFRNYieldFromPrice method
	*			Computes the yield of a floating rate bond from quoted price, without requiring discount and forecast curves.
	*			The yield is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
	*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
	*			See BBG CalcType 21 for calculation details.
	*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
	*
	* @param[in]	bondObjectName				The name of the cached floating bond object
	* @param[in]	settlementDate				settlement date
	* @param[in]	price						The price of the bond, in the quoted convention (clean / dirty )
	* @param[in]	assumedRate					Assumed index rate used for projected coupons
	* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
	* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
	* @returns	The bond yield	
	*/
	double tryMeLWOBondFRNYieldFromPrice( const std::string& bondObjectName, const AQLDate& settlementDate, const double& price, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDate, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate )

		etrading::BondPtr bondPtr = etrading::getBond(bondObjectName);

		std::shared_ptr<etrading::FloatingBond> floatingBondPtr = std::dynamic_pointer_cast<etrading::FloatingBond>( bondPtr );
		AQ_REQUIRE( floatingBondPtr != nullptr, "Bond '" + bondObjectName + "' is not a floating rate bond." )

		const double yield = floatingBondPtr->yieldFromPrice( settlementDate, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT( bondObjectName, std::string(), yield )

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the meLWOBondFRNDiscountMarginFromPrice method
	*			Computes the discount margin of a floating rate bond from quoted price, without requiring discount and forecast curves.
	*			The discount margin is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
	*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
	*			See BBG CalcType 21 for calculation details.
	*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
	*
	* @param[in]	bondObjectName				The name of the cached floating bond object
	* @param[in]	settlementDate				settlement date
	* @param[in]	price						The price of the bond, in the quoted convention (clean / dirty )
	* @param[in]	assumedRate					Assumed index rate used for projected coupons
	* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
	* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
	* @returns	The bond discount margin
	*/
	double tryMeLWOBondFRNDiscountMarginFromPrice( const std::string& bondObjectName, const AQLDate& settlementDate, const double& price, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDate, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate )

		etrading::BondPtr bondPtr = etrading::getBond(bondObjectName);

		std::shared_ptr<etrading::FloatingBond> floatingBondPtr = std::dynamic_pointer_cast<etrading::FloatingBond>( bondPtr );
		AQ_REQUIRE( floatingBondPtr != nullptr, "Bond '" + bondObjectName + "' is not a floating rate bond." )

		const double discountMargin = floatingBondPtr->discountMarginFromPrice( settlementDate, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT( bondObjectName, std::string(), discountMargin )

		VALID_EXCEPTION_END
	}
}
