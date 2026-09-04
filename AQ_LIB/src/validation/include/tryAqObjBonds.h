#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Bond.h"

using etrading::LabelValueBlock; 

namespace validation
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

    /* @brief			validation interface for the aqObjBondsCreate method
	*  @param [in]		bondObjectName	    Bond object name
	*  @param [in]		bondLVB			    Bond Label Value Block
    *  @param [in]		scheduleLVB		    Schedule Label Value Block
	*  @param [in]		validateKeys	    True to validate the all keys provided are valid. Default to True
	*  @return			Bond Handle
	*/
	std::string tryAqObjBondsCreate( const std::string& bondObjectName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB, bool validateKeys=true );

   /* @brief			validation interface for the aqObjBondsCreateFromGeneratorLVB method. Create a Bond from a BondGenerator
    *  @param [in]		bondObjectName          Bond cached name to use
	*  @param [in]		bondGeneratorName       Bond Generator name to use
    *  @param [in]		bondExpressionLVB       A Label Value Block containing dataValues specific to this bond
    *  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
    *  @return			Bond Handle string
	*/
	std::string tryAqObjBondsCreateFromGenerator( const std::string& bondObjectName, const std::string& bondGeneratorName, const LabelValueBlock& bondExpressionLVB, bool validateKeys );

    /* @brief			validation interface for the aqObjBondsDisplay method, which display the INPUT parameters of the cached bond
	*  @param [in]		bondObjectName		Bond object name
	*  @return			Bond display of the input parameters
	*/
	AnyTypeMatrix tryAqObjBondsDisplay( const std::string& bondObjectName );

    /* @brief			validation interface for the aqObjBondsDisplaySchedule method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		showColumnHeaders	Show Schedule Column Headers; Defaults to True
	*  @param [in]  	columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Bond Schedule
	*/
	AnyTypeMatrix tryAqObjBondsDisplaySchedule( const std::string& bondObjectName, const bool& showColumnHeaders = true, const std::vector<std::string>& columnList=std::vector<std::string>() );

    /* @brief			validation interface for the aqObjBondsDisplayCashflows method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDate		settlement date
    *  @param [in]		yield		        yield
    *  @param [in]		yieldCalculationType = "" yield calculation type       
    *  @param [in]		showColumnHeaders	Show Schedule Column Headers; Defaults to True
	*  @param [in]  	columnList              Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Bond Cashflows
	*/
	AnyTypeMatrix tryAqObjBondsDisplayCashflows( const std::string& bondObjectName, const AQLDate& settlementDate, const double& yield, const std::string& yieldCalculationType = "", const bool& showColumnHeaders = true, const std::vector<std::string>& columnList=std::vector<std::string>() );

    /* @brief			validation interface for the aqObjBondsDirtyPrice method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryAqObjBondsDirtyPrice( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType = "" );

    /* @brief			validation interface for the aqObjBondsAccruedInterestDays method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @return			Bond Accrual Day(s)
	*/
	std::vector< int > tryAqObjBondsAccruedInterestDays( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates );

    /* @brief			validation interface for the aqObjBondsAccruedInterest method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @return			The bond accrued interest
	*/
	std::vector< double > tryAqObjBondsAccruedInterest( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates );

    /* @brief			validation interface for the aqObjBondsAccruedInterest method
	*  @param [in]		bondObjectName			Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @return			The bond accrued interest
	*/
	double tryAqObjBondsAccruedInterest( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB );
    
	/* @brief			validation interface for the aqObjBondsCleanPrice method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryAqObjBondsCleanPrice( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType = "" );

    /* @brief			validation interface for the aqObjBondsPrice method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryAqObjBondsPrice( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType = "" );

    /* @brief			validation interface for the aqObjBondsQuote method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Dirty Price(s)
	*/
	std::vector< double > tryAqObjBondsQuote( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType = "" );

    /* @brief			Helper function to calculate the yield of a bond given the bond object
	*  @param [in]		bondObject		    Bond Object
    *  @param [in]		settlementDate		settlement date
    *  @param [in]		price		        bond price
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Yield
	*/
    double tryAqObjBondsYieldFromObject( const etrading::BondPtr bondObject, const AQLDate & settlementDate, const double & price, const std::string& yieldCalculationType = "" );

    /* @brief			Helper function to optimize the performance of the bond yield calculation
	*  @param [in]		bondObject		    Bond object pointer
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		prices		        bond prices, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Yield(s)
	*/
    std::vector< double > tryAqObjBondsYieldOptimized( const etrading::BondPtr bondObject, const std::vector< AQLDate >& settlementDates, const std::vector< double >& prices, const std::string& yieldCalculationType = ""  );

    /* @brief			validation interface for the aqObjBondsYield method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		settlementDates		    settlement dates
    *  @param [in]		prices		            bond prices, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @param [in]      optimizePerformance     Use multi-threading to optimize performance, defaults to FALSE
    *  @return			Bond Yield(s)
	*/
	std::vector< double > tryAqObjBondsYield( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& prices, const std::string& yieldCalculationType = "" , const bool & optimizePerformance = false);

    /* @brief			validation interface for the aqObjBondsYield method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		price		            bond price, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond Yield
	*/
	double tryAqObjBondsYield( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double price, const std::string& yieldCalculationType = "" );

	/* @brief			validation interface for the tryAqObjBondsBPVPerTick method
	*  @param [in]		bondObjectName		    Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		price		            bond price, internally we check for clean or dirty
    *  @param [in]		yieldCalculationType	yield calculation type	        
	*  @param [in]		tickSize				bond tick size, defaults to 1/32 for US Treasuries       
    *  @return			Bond Basis Point Change per TickSize
	*/
	double tryAqObjBondsBPVPerTick( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double price, const std::string& yieldCalculationType = "", const double tickSize = 0.01325 );

    /* @brief			validation interface for the aqObjBondsCompoundYield method
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		Settlement dates
    *  @param [in]		prices		        User input prices
    *  @param [in]		yieldCalculationType	yield calculation type	        
    *  @return			Compound Yields used as bond discount factors
	*/
    std::vector< double > tryAqObjBondsCompoundYields( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& prices, const std::string& yieldCalculationType = "" );

	/* @brief			validation interface for the aqObjBondsDV01Numerical method. Calculate the DV01(s) via numerical bumping.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
	*  @param [in]		bumpSize		    size of the yield bump in bps
	*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
    *  @return			Bond DV01(s)
	*/
	std::vector< double > tryAqObjBondsDV01Numerical( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const double bumpSize, const AQLString& bumpMode, const std::string& yieldCalculationType = "" );

	/* @brief			validation interface for the aqObjBondsDV01 method. Calculate the DV01(s) analytically.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
	*  @param [in]		bumpSize		    size of the yield bump in bps
	*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
    *  @return			Bond DV01(s)
	*/
	std::vector< double > tryAqObjBondsDV01( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType = "" );

	/* @brief			validation interface for the aqObjBondsModifiedDuration method. Calculate the Modified Duration analytically.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		settlement dates
    *  @param [in]		yields		        yields
    *  @param [in]		yieldCalculationType = ""	yield calculation type	        
	*  @param [in]		bumpSize		    size of the yield bump in bps
	*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
    *  @return			Bond Duration(s)
	*/
	std::vector< double > tryAqObjBondsModifiedDuration( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields, const std::string& yieldCalculationType = "" );

    /* @brief			validation interface for the aqObjBondsSpreadOIS method. Calculate the O-Spread (BondYield to OIS ParRate spread) using linear interpolation on the OIS ParRate on standard tenors.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		marketObjectName	Market Data object name
    *  @param [in]		settlementDates		Settle Dates for the bond 
    *  @param [in]		bondYields		    Bond yields
    *  @return			A vector of O-Spreads
	*/
    std::vector< double > tryAqObjBondsOisSpread( const std::string& bondObjectName, const std::string& marketObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& yields );

   /* @brief			validation interface for the aqObjBondsOisSpread method. Calculate the O-Spread (BondYield to OIS ParRate spread) using linear interpolation on the OIS ParRate on standard tenors.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		marketObjectName	Market Data object name
    *  @param [in]		settleDate		    Settle Date for the bond 
    *  @param [in]		bondYield		    Bond yield
    *  @return			Bond Yield and OIS ParRate Spread
	*/
    double tryAqObjBondsOisSpread( const std::string& bondObjectName, const std::string& marketObjectName, const AQLDate& settleDate, const double& bondYield);

	/* @brief			Validation interface for the aqObjBondsLastCouponDate method. Calculate the bond's last coupon date.
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		settlementDates		Bond's Settlement Dates 
    *  @return			The bond's last coupon date.
	*/
    std::vector< double > tryAqObjBondsLastCouponDate( const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates);

	/* @brief			validation interface for the aqObjBondsPriceFromDirtyToClean method. 
	*  @param [in]		bondObjectName			Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		dirtyPrice				Bond dirtyPrice
    *  @return			Bond Clean Price
	*/
    double tryAqObjBondsPriceFromDirtyToClean( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double dirtyPrice );

    /* @brief			validation interface for the aqObjBondsPriceFromCleanToDirty method. 
	*  @param [in]		bondObjectName			Bond object name
    *  @param [in]		valuationSettingsLVB	Specifies the settlementDate, plus other parameters required for more complex bonds such as FRNs.
    *  @param [in]		cleanPrice				Bond cleanPrice
    *  @return			Bond Dirty Price
	*/
    double tryAqObjBondsPriceFromCleanToDirty( const std::string& bondObjectName, const LabelValueBlock& valuationSettingsLVB, const double cleanPrice );

	 /* @brief			validation interface for the aqObjBondsPriceFromCreditModel function. This calculates the price of a defaultable bond.
	 *  @param [in]		bondObjectName		Bond object name
     *  @param [in]		settlementDate		Bond's Settlement Date
     *  @param [in]		creditModelName		Credit model name
     *  @return			The calculated Bond price
	 */
	 double tryAqObjBondsPriceFromCreditModel( const std::string& bondObjectName, const AQLDate& settlementDate, const std::string& creditModelName );

	 /* @brief Creates a AQOBondGenerator object, containing the bond static properties.
	 * @param [in] objectName        The name of the Bond Generator object
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryAqObjBondsGeneratorCreate( const std::string& objectName,
                                             const std::vector<std::string>& propertyNames,
                                             const std::vector<TableInfo>& infoBlocks );

	 /* @brief Displays the specified property of a AQOBondGenerator. If propertyName is blank, all properties are returned.
	 * @param [in] objectName        The BondGenerator object you wish to display
	 * @param [in] propertyName      The name of the property label-value block that you wish to display
	 * @param [out]                  A VariantMatrix containing a LabelValue block of properties
	 */
    const etrading::VariantMatrix  tryAqObjBondsGeneratorDisplay( const std::string& objectName,
															     const std::string& propertyName );

	/* @brief			validation interface for the aqObjBondsCreateFromLVB method
	*  @param [in]		bondName		Bond name
	*  @param [in]		bondLVB			Bond input as single label value block 
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			bondName
	*/
	std::string tryAqObjBondsCreateFromLVB(const std::string& bondObjectName, const LabelValueBlock& bondLVB, bool validateKeys = true);

	/* @brief			Helper interface for the tryAqObjBondsZSpread method. Bond Z Spread.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settlementDate		Bond's Settlement Date (Valuation Date)
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		curveCollection     Curve collection
	*  @param [in]		forecastCurve       Forecast curve
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			Z Spread
	*/
	double tryAqObjBondsZSpread(const std::string& bondObjectName, const AQLDate& settlementDate, const double& bondPrice, const std::string& curveCollection, const std::string& forecastCurve, const bool& continuouslyCompounding);

	/* @brief			Validation interface for the aqObjBondsZSpread method. Bond Z Spread.
	*  @param [in]		bondObjectNames		Bond object name
	*  @param [in]		settlementDates		Bond's Settlement Date (Valuation Date)
	*  @param [in]		bondPrices		    Bond price
	*  @param [in]		curveCollections    Curve collection
	*  @param [in]		forecastCurves      Forecast curves
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			Z Spread
	*/
	DoubleVector tryAqObjBondsZSpreads(const std::string& bondObjectName, const std::vector< AQLDate >& settlementDates, const std::vector< double >& bondPrices, const std::vector< std::string >& curveCollections, const std::vector< std::string >& forecastCurves, const bool& continuouslyCompounding);

	/* @brief			Validation interface for the aqObjBondsZSpreadFromRates method. Bond Z Spread.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settlementDate		Bond's Settlement Date (Valuation Date)
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		zeroRates			External ZeroRates
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			Z Spread
	*/
	double tryAqObjBondsZSpreadFromRates(const std::string& bondObjectName, const AQLDate& settlementDate, const double& bondPrice, const DoubleVector& zeroRates, const bool& continuouslyCompounding);

	/* @brief			Validation interface for the aqObjBondsForwardPrice method. Calculate Bond's forward price at forward settle date, from repo rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond forward price
	*/
	double tryAqObjBondsForwardPrice(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& price, const double& repoRate, const std::string& repoDayCount);

	/* @brief			Validation interface for the aqObjBondsForwardReinvestedCoupon method. Calculate Bond's recieved coupon value at forward settle date, from repo rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond's recieved coupon value at forward settle date
	*/
	double tryAqObjBondsForwardReinvestedCoupon(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& price, const double& repoRate, const std::string& repoDayCount);

	/* @brief			Validation interface for the aqObjBondsRepoRate method. Calculate Bond's implied repo rate from bond's forward price at settle date.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		forwardSettleDate   Bond forward settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		forwardPrice		Bond's forward price at forward settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond implied repo rate
	*/
	double tryAqObjBondsRepoRate(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& price, const double& forwardPrice, const std::string& repoDayCount);

	/* @brief			Validation interface for the aqObjBondsRepoRateFromFuture method. Calculate Bond's implied repo rate from bond's FUTURE price at settle date.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		deliveryDate		Bond future settle date
	*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		futurePrice			Bond's future price at forward settle date
	*  @param [in]		conversionFactor	Bond's conversion factor
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @return			Bond implied repo rate
	*/
	double tryAqObjBondsRepoRateFromFuture(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& deliveryDate, const double& price, const double& futurePrice, const double& conversionFactor, const std::string& repoDayCount);

	/* @brief			Validation interface for the aqObjBondsFuturePrice method. Calculate Bond's Future price at future settle date, from repo rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		deliveryDate		Bond future settle date
	*  @param [in]		bondPrice			Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		repoRate			Bond repo rate
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		conversionFactor	Bond's conversion factor
	*  @return			Bond Future price
	*/
	double tryAqObjBondsFuturePrice(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& deliveryDate, const double& bondPrice, const double& repoRate, const std::string& repoDayCount, const double& conversionFactor);

	/* @brief			Validation interface for the aqObjBondsConversionFactor method. Calculate Bond conversionFactor based on future settle date and future's notional coupon rate.
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		firstFutureSettleDate	Bond's first future settle date
	*  @param [in]		notionalBondCouponRate	Bond future notionalBondCouponRate
	*  @return			Bond conversionFactor against the future contract
	*/
	double tryAqObjBondsConversionFactor(const std::string& bondObjectName, const AQLDate& firstFutureSettleDate, const double& notionalBondCouponRate);

	/* @brief			Validation interface for the aqObjBondsGrossBasis method. Calculate the Bond Gross Basis
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		settleDate			Bond current settle date
	*  @param [in]		prices				Bond prices at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		futurePrice			Bond future price at future settle date, always clean
	*  @param [in]		conversionFactor	Conversion factor
	*  @return			Bond Gross Basis
	*/
	double tryAqObjBondsGrossBasis(const std::string& bondObjectName, const AQLDate& settleDate, const double& price, const double& futurePrice, const double& conversionFactor);

	/* @brief			Validation interface for the aqObjBondsNetBasis method. Calculate the Bond Gross Basis
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		deliveryDate		Bond future settle date
	*  @param [in]		forwardPrice		Bond forwardPrice at delivery date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		futurePrice			Bond future price at delivery date, always clean
	*  @param [in]		conversionFactor	Conversion factor
	*  @return			Bond Gross Basis
	*/
	double tryAqObjBondsNetBasis(const std::string& bondObjectName, const AQLDate& settleDate, const AQLDate& deliveryDate, const double& bondPrice, const double& actualRepoRate, const std::string& repoDayCount, const double& futurePrice, const double& conversionFactor);

	/* @brief			Validation interface for the aqObjBondsCheapestToDeliverByImpliedRepoRate method. Calculate the Cheapest to deliver (CTD) bond via Implied Repo Rates, return the bond name.
	*  @param [in]		futurePrice			Future price
	*  @param [in]		settleDate			Bond's settle date
	*  @param [in]		deliveryDate		Bond's future settle date/delivery date
	*  @param [in]		repoDayCount		Day count for repo year fraction
	*  @param [in]		bondObjectNames		A list of Bond object names
	*  @param [in]		bondPrices			A list of bond prices at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
	*  @param [in]		conversionFactors	A list of conversion factors
	*  @return			The Cheapest to deliver (CTD) bond's name
	*/
	std::string tryAqObjBondsCheapestToDeliver(const double& futurePrice, const AQLDate& settleDate, const AQLDate& deliveryDate, const std::string& repoDayCount, const std::vector<std::string>& bondObjectNames, const std::vector<double>& bondPrices, const std::vector<double>& conversionFactors);

	/* @brief			Validation interface for the aqObjBondsCheapestToDeliverByNetBasis method. Calculate the Cheapest to deliver (CTD) bond via Net Basis, return the bond name.
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
	std::string tryAqObjBondsCheapestToDeliverByNetBasis(const double& futurePrice, const AQLDate& settleDate, const AQLDate& deliveryDate, const std::string& repoDayCount, const std::vector<std::string>& bondObjectNames, const std::vector<double>& bondPrices, const std::vector<double>& conversionFactors, const std::vector<double>& actualRepoRates);

	/* @brief			validation interface for the tryAqObjBondsCreateAUDNotionalBond method
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
	std::string tryAqObjBondsCreateAUDNotionalBond(const std::string& bondObjectName, const AQLDate& settleDate, const std::string& maturityDate, const std::string& couponRate, const std::string& payReceive, const std::string& calendar, const std::string& frequency, const std::string& dayCount, const std::string& bondQuoteConv);

	/* @brief			validation interface for the aqToolsBondAverageYield method, calculate the average of underlying bonds' yields
	*  @param [in]		underlyingBondYields	Bond future's underlying bond yields
	*  @return			Aussie bond future notional bond's yield
	*/
	double tryAqToolsBondAverageYield(const std::vector<double>& underlyingBondYields);

	/* @brief			validation interface for the aqToolsBondYieldFromFuturePrice method, calculate the 1 - futurePrice/100
	*  @param [in]		futurePrice		Bond future price
	*  @return			Aussie bond future notional bond's yield
	*/
	double tryAqToolsBondYieldFromFuturePrice(const double& futurePrice);


	/* @brief	validation interface for the aqObjBondsFRNPriceFromDiscountMargin method
	*			Computes the price of a floating rate bond without requiring curves discount and forecast curves.
	*			The price is found given the following assumed values:  the discountMargin, assumedIndex, indexToNextCoupon, annualiedNextCouponRate.
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
	double tryAqObjBondsFRNPriceFromDiscountMargin( const std::string& bondObjectName, const AQLDate& settlementDate, const double& discountMargin, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate );

	/* @brief	validation interface for the aqObjBondsFRNPriceFromYield method
	*			Computes the price of a floating rate bond from the yield, without requiring curves discount and forecast curves.
	*			The price is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualiedNextCouponRate.
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
	double tryAqObjBondsFRNPriceFromYield( const std::string& bondObjectName, const AQLDate& settlementDate, const double& yield, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate );

	/* @brief	validation interface for the aqObjBondsFRNYieldFromPrice method
	*			Computes the yield of a floating rate bond from quoted price, without requiring discount and forecast curves.
	*			The yield is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualiedNextCouponRate.
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
	double tryAqObjBondsFRNYieldFromPrice( const std::string& bondObjectName, const AQLDate& settlementDate, const double& price, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate );

	/* @brief	validation interface for the aqObjBondsFRNDiscountMarginFromPrice method
	*			Computes the discount margin of a floating rate bond from quoted price, without requiring discount and forecast curves.
	*			The discount margin is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualiedNextCouponRate.
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
	double tryAqObjBondsFRNDiscountMarginFromPrice( const std::string& bondObjectName, const AQLDate& settlementDate, const double& price, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate );

}
