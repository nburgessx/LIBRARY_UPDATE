#include "tryAqObjectsAssetSwap.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

#include "SwapUtilities.h"
#include "AQOUtilities.h"
#include "BondSpreadCalculation.h"
#include "ExceptionMacros.h"
#include "BondFactory.h"
#include "FixedBondLegSchedule.h"
#include "FloatBondLegSchedule.h"
#include "ParameterValidation.h"
#include "SettingsValidation.h"
#include "FloatingBond.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::Swap;

namespace validation
{

	void validateAssetSwapInputs(const etrading::BondPtr& bond, const etrading::LegPtr& swapFixedLeg, const etrading::LegPtr& swapFloatLeg)
	{

		switch (bond->getBondTypeEnum())
		{
		case etrading::FIXED_BOND:
		{
			// Validate Bond's MaturityDate, Coupon, DayCount, Frequency are matching Swap's Fixed Leg

			// OK to use static_pointer_cast here because we know the cashflow type for sure.
			const std::shared_ptr<etrading::BondSchedule>& bondSchedule = std::static_pointer_cast<etrading::BondSchedule>(bond->getSchedule());
			const std::shared_ptr<etrading::FixedBondLegSchedule>& fixedSchedule = std::static_pointer_cast<etrading::FixedBondLegSchedule>(swapFixedLeg->getSchedule());

			AQ_REQUIRE(AQ_IS_EQUAL(std::fabs(bondSchedule->getNotional()), std::fabs(fixedSchedule->getNotional())), "The Notional of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(AQ_IS_EQUAL(bondSchedule->getFixedRate(), fixedSchedule->getFixedRate()), "Bond's Coupon Rate and Swap Fixed Leg's Fixed Rate are not matched.");

			AQ_REQUIRE(bondSchedule->getMaturityDate()				== fixedSchedule->getMaturityDate(), "The Maturity Date of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getAccrualCalendar()				== fixedSchedule->getAccrualCalendar(), "The Accrual Calendar of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getAccrualDaycount()				== fixedSchedule->getAccrualDaycount(), "The Accrual Daycount of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getAccrualFrequency()			== fixedSchedule->getAccrualFrequency(), "The Accrual Frequency of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getPaymentFrequency()			== fixedSchedule->getPaymentFrequency(), "The Payment Frequency of Bond and Swap Float Leg are not matched.");

			AQ_REQUIRE(bondSchedule->getBondCalculationType()			== fixedSchedule->getBondCalculationType(), "The CalculationType of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getExDividendTenor()				== fixedSchedule->getExDividendTenor(), "The ExDividendTenor of Bond and Swap Fixed Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getExDividendBusinessDayAdj()	== fixedSchedule->getExDividendBusinessDayAdj(), "The ExDividendBusinessDayAdj of Bond and Swap Fixed Leg are not matched.");

			break;
		}
		case etrading::FLOATER_BOND:
		{
			// OK to use static_pointer_cast here because we know the cashflow type for sure.
			const std::shared_ptr<etrading::BondSchedule>& bondSchedule = std::static_pointer_cast<etrading::BondSchedule>(bond->getSchedule());
			const std::shared_ptr<etrading::FloatBondLegSchedule>& floatSchedule = std::static_pointer_cast<etrading::FloatBondLegSchedule>(swapFloatLeg->getSchedule());

			AQ_REQUIRE(AQ_IS_EQUAL(std::fabs(bondSchedule->getNotional()), std::fabs(floatSchedule->getNotional()) ), "The Notional of Bond and Swap Float Leg are not matched.");

			AQ_REQUIRE(bondSchedule->getMaturityDate()			== floatSchedule->getMaturityDate(), "The Maturity Date of Bond and Swap Float Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getAccrualCalendar()			== floatSchedule->getAccrualCalendar(), "The Accrual Calendar of Bond and Swap Float Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getAccrualDaycount()			== floatSchedule->getAccrualDaycount(), "The Accrual Daycount of Bond and Swap Float Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getAccrualFrequency()		== floatSchedule->getAccrualFrequency(), "The Accrual Frequency of Bond and Swap Float Leg are not matched.");
			AQ_REQUIRE(bondSchedule->getPaymentFrequency()		== floatSchedule->getPaymentFrequency(), "The Payment Frequency of Bond and Swap Float Leg are not matched.");

			// Check bond parameters
			auto floatingBond = std::dynamic_pointer_cast<etrading::FloatingBond>(bond);
			AQ_REQUIRE( floatingBond != nullptr, "Bond '" + bond->getBondObjectName() + "' is not a floating rate bond." )
		
			const double assetSwapQuotedMargin = floatSchedule->getQuotedMargin();
			const double floatBondQuotedMargin = floatingBond->getQuotedMargin();
			AQ_REQUIRE( AQ_IS_EQUAL( assetSwapQuotedMargin, floatBondQuotedMargin ), "Asset Swap QuotedMargin should match the Floating Bond QuotedMargin");

			break;
		}
		default:
			throw AQLCoreInvalidData("#Error: BondType - only Fixed or Floater is supported.", __FILE__, __LINE__);
			break;
		};

	}


	/* @brief			Helper method to calculate the Par/Par Asset Swap Spread.
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Par/Par Asset Swap Spread
	*/
	double assetSwapSpread(const std::string& swapObjectName, const etrading::BondPtr& bond, const double& bondPrice, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		auto swap = etrading::getSwap(swapObjectName);
		auto swapFixedLeg = etrading::getFixedLeg(swap);
		auto swapFloatLeg = etrading::getFloatLeg(swap);

		validateAssetSwapInputs(bond, swapFixedLeg, swapFloatLeg);

		double bondCleanPrice = bondPrice;

		if (!bond->isCleanPrice())
		{
			// *** TODO: Should this be settlementDate?
			const AQLDate valuationDate = etrading::getValuationDateFromValuationSettings(valuationSettingsLVB);

			etrading::ValuationSettings valuationSettings( valuationSettingsLVB);
			valuationSettings.setSettlementDate( valuationDate );
			etrading::DataProvider dataProvider( valuationSettings );
			bondCleanPrice = bond->priceFromDirtyToClean( bondPrice, dataProvider );
		}

		double aswSpread = 0.0;

		switch (bond->getBondTypeEnum())
		{
		case etrading::FIXED_BOND:
		{
			aswSpread = etrading::calculateAssetSwapSpreadParParFromBondCleanPrice(bondCleanPrice, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames);

			break;
		}
		case etrading::FLOATER_BOND:
		{
			aswSpread = etrading::calculateFloatingBondAssetSwapSpread(bondCleanPrice, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames);

			break;
		}
		default:
			throw AQLCoreInvalidData("#Error: BondType - only Fixed or Floater is supported.", __FILE__, __LINE__);
			break;
		};

		return aswSpread;
	}
 
	/* @brief			validation interface for the aqObjectsAssetSwapParParFromCleanPrice method. Calculate the Par/Par Asset Swap Spreads.
    *  @param [in]		bondPrices			Bond prices
	*  @param [in]		bondAccrualStartDates	Bond Accrual Start Dates
	*  @param [in]		isCleanPrice		Bond price is clean or dirty
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Par/Par Asset Swap Spreads
	*/
    std::vector< double > tryAqObjectsAssetSwapSpreadFromPrice(const std::vector< double >& bondPrices, const std::vector< AQLDate >& bondAccrualStartDates, const std::vector< bool >& isCleanPrices,
														const std::string& swapObjectName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
    {

		size_t expectedSize = bondPrices.size();

		const bool isCleanPriceEmpty = isCleanPrices.empty();
		if (!isCleanPriceEmpty)
		{
			AQ_REQUIRE(expectedSize == isCleanPrices.size(), "Inconsistent number of settlement dates and isCleanPrices");
		}

		const bool emptyBondAccrualStartDate = bondAccrualStartDates.empty();
		if (!emptyBondAccrualStartDate)
		{
			AQ_REQUIRE(expectedSize == bondAccrualStartDates.size(), "Inconsistent number of settlement dates and bondAccrualStartDates");
		}

        std::vector< double > spreads(expectedSize);

		const AQLDate dummyDate = AQLDate();

        for ( unsigned int i = 0; i < expectedSize; ++i )
        {
            const double bondPrice					= bondPrices[i];

			// If it's not provided, use dummy value
			const AQLDate bondAccrualStartDate		= emptyBondAccrualStartDate ? dummyDate : bondAccrualStartDates[i];

			// If it's not provided, then assume it's clean
			const bool isCleanPrice					= isCleanPriceEmpty ? true : isCleanPrices[i]; 

			double spread							= tryAqObjectsAssetSwapSpreadFromPrice( bondPrice, bondAccrualStartDate, isCleanPrice, swapObjectName, valuationSettingsLVB, fixingTableNames);
                        
            spreads[i] = spread;
        }

        return spreads;

    }

	/* @brief			Helper interface for the tryAqObjectsAssetSwapSpreadFromPrice method. Calculate the Par/Par Asset Swap Spread.
	*  @param [in]		bondAccrualStartDate	Bond Accrual Start Date
	*  @param [in]		isCleanPrice		Bond price is clean or dirty
	*  @param [in]		bondPrice			Bond price
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Par/Par Asset Swap Spread
	*/
	double tryAqObjectsAssetSwapSpreadFromPrice(const double& bondPrice, const AQLDate& bondAccrualStartDate, const bool& isCleanPrice,
											const std::string& swapObjectName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
    {
	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(swapObjectName, std::string(), bondPrice, bondAccrualStartDate, isCleanPrice, swapObjectName, valuationSettingsLVB, fixingTableNames);

        auto swap = etrading::getSwap( swapObjectName );
		auto swapFixedLeg = etrading::getFixedLeg(swap);
		auto swapFloatLeg = etrading::getFloatLeg(swap);
		
		AQ_REQUIRE(swapFixedLeg->getType() == etrading::SWAPSCHEDULE_FIXEDBOND, "Asset Swaps must be contain a FixedBond.");

		double aswSpread = 0.0;

		if (isCleanPrice)
		{
			aswSpread = etrading::calculateAssetSwapSpreadParParFromBondCleanPrice(bondPrice, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames);
		}
		else
		{
			//Create a dummy bond, so that clean price can be calculated from the dirty price

			AQ_REQUIRE(bondAccrualStartDate != AQLDate(), "bondAccrualStartDate is required to calculate Bond's clean price from dirty price.");

			const std::string bondName = swapObjectName + "_Bond";

			LabelValueBlock bondLVBSwapFixedLeg = swapFixedLeg->getInputParameters();

            StandardStringVector addKeys(6);
            addKeys[0] = etrading::BOND_KEY::ISIN;
            addKeys[1] = etrading::BOND_KEY::CURRENCY;
            addKeys[2] = etrading::BOND_KEY::BOND_TYPE;
            addKeys[3] = etrading::BOND_KEY::YIELD_TYPE;
            addKeys[4] = etrading::BOND_KEY::IS_CLEAN_PRICE;
            addKeys[5] = etrading::BOND_KEY::ISSUE_DATE;

            StandardStringVector addValues(6);
            addValues[0] = "DUMMYISIN";
            addValues[1] = toString(swapFixedLeg->getStaticData()->getCurrency());
            addValues[2] = toString(etrading::FIXED_BOND);
            addValues[3] = toString(etrading::YIELD_TO_MATURITY);
            addValues[4] = isCleanPrice ? "TRUE" : "FALSE";
            addValues[5] = bondAccrualStartDate.stringWithFormat().c_str();

            // Create New LVB appending additional keys and values
            LabelValueBlock bondLVB( bondLVBSwapFixedLeg, addKeys, addValues );
			
            // Use the Bond Factory to validate and create the bond
			etrading::BondPtr bond = etrading::createBondFromSingleLVB(bondName, bondLVB, false);

			const double aswSpread = assetSwapSpread(swapObjectName, bond, bondPrice, valuationSettingsLVB, fixingTableNames);

		}

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapObjectName, std::string(), aswSpread );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the aqObjectsAssetSwapSpread method. Calculate the Par/Par Asset Swap Spreads.
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spreads
	*/
    std::vector< double > tryAqObjectsAssetSwapSpread( const std::string& swapObjectName, const std::string& bondObjectName, const std::vector< double >& bondPrices, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames )
    {

		size_t exptectedSize = bondPrices.size();

        std::vector< double > spreads(exptectedSize);
		                        
        for ( unsigned int i = 0; i < exptectedSize; ++i )
        {
            const double bondPrice          = bondPrices[i];
			double spread                 = tryAqObjectsAssetSwapSpread( swapObjectName, bondObjectName, bondPrice, valuationSettingsLVB, fixingTableNames);
                        
            spreads[i] = spread;
        }

        return spreads;

    }

	/* @brief			Helper interface for the aqObjectsAssetSwapSpread method. Calculate the Par/Par Asset Swap Spread.
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
    *  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spread
	*/
    double tryAqObjectsAssetSwapSpread( const std::string& swapObjectName, const std::string& bondObjectName, const double& bondPrice, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames )
    {
	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(swapObjectName, std::string(), swapObjectName, bondObjectName, bondPrice, valuationSettingsLVB,  fixingTableNames);

		auto bond = etrading::getBond(bondObjectName);

		const double aswSpread = assetSwapSpread(swapObjectName, bond, bondPrice, valuationSettingsLVB, fixingTableNames);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapObjectName, std::string(), aswSpread );

		VALID_EXCEPTION_END
    }

	/* @brief			validation interface for the aqObjectsAssetSwapFixedEqvCoupon method. Calculate the asset swap fixed leg's swap rate, so that the swap PV (excluding accrued interest) matches the bond's parParAdjustment
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		bondPrice		    Bond price
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			AssetSwap's swap rate (Fixed Equivalent Coupon)
	*/
	double tryAqObjectsAssetSwapFixedEqvCoupon(const std::string& swapObjectName, const std::string& bondObjectName, const double& bondPrice, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(swapObjectName, std::string(), swapObjectName, bondObjectName, bondPrice, valuationSettingsLVB, fixingTableNames);

		auto bond = etrading::getBond(bondObjectName);

		AQ_REQUIRE(bond->getBondTypeEnum() == etrading::FLOATER_BOND, "AssetSwap's Fixed Equivalent Coupon calculation is only supported for Floating Bond.");

		auto swap = etrading::getSwap(swapObjectName);
		auto swapFixedLeg = etrading::getFixedLeg(swap);
		auto swapFloatLeg = etrading::getFloatLeg(swap);

		validateAssetSwapInputs(bond, swapFixedLeg, swapFloatLeg);

		double bondCleanPrice = bondPrice;

		if (!bond->isCleanPrice())
		{
			// *** TODO: Shouldn't this use settlementDate?
			const AQLDate valuationDate = etrading::getValuationDateFromValuationSettings(valuationSettingsLVB);

			etrading::ValuationSettings valuationSettings( valuationSettingsLVB );
			valuationSettings.setSettlementDate( valuationDate );
			etrading::DataProvider dataProvider( valuationSettings );

			bondCleanPrice = bond->priceFromDirtyToClean( bondPrice, dataProvider );
		}

		const double fixedRate = etrading::calculateFloatingBondAssetSwapFixedEqvCoupon(bondCleanPrice, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapObjectName, std::string(), fixedRate);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjectsAssetSwapSpreadFromFixedEqvCoupon method. Calculate the asset swap spread based on the FixedEqvCoupon
	*  @param [in]		swapObjectName		Swap object name
	*  @param [in]		bondObjectName		Bond object name
	*  @param [in]		fixedEqvCoupon		Asset Swap's fixed equivalent coupon (fixed rate that make the swap PV as parParAdjustment)
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Floating Bond Asset Swap Spread based on FixedEqvCoupon
	*/
	double tryAqObjectsAssetSwapSpreadFromFixedEqvCoupon(const std::string& swapObjectName, const double& fixedEqvCoupon, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(swapObjectName, std::string(), swapObjectName, fixedEqvCoupon, valuationSettingsLVB, fixingTableNames);

		auto swap = etrading::getSwap(swapObjectName);
		auto swapFixedLeg = etrading::getFixedLeg(swap);
		auto swapFloatLeg = etrading::getFloatLeg(swap);

		const double aswSpread = etrading::calculateFloatingBondAssetSwapSpreadFromFixedEqvCoupon(fixedEqvCoupon, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames);

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapObjectName, std::string(), aswSpread);

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqObjectsAssetSwapSpreadToCleanPrice method. Calculate the Bond Clean Prices.
	*  @param [in]		swapObjectName		Swap object name
    *  @param [in]		assetSwapParParSpread	Asset Swap Par Par Spreads
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spreads
	*/
    std::vector< double > tryAqObjectsAssetSwapSpreadToCleanPrice( const std::string& swapObjectName, const std::vector< double >& assetSwapParParSpreads, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames )
    {

		size_t expectedSize = assetSwapParParSpreads.size();

        std::vector< double > bondCleanPrices(expectedSize);

        for ( unsigned int i = 0; i < expectedSize; ++i )
        {
            const double assetSwapParParSpread  = assetSwapParParSpreads[i];
			double bondCleanPrice               = tryAqObjectsAssetSwapSpreadToCleanPrice( swapObjectName, assetSwapParParSpread, valuationSettingsLVB, fixingTableNames);
                        
            bondCleanPrices[i] = bondCleanPrice;
        }

        return bondCleanPrices;

    }

	/* @brief			Helper interface for the aqObjectsAssetSwapSpreadToCleanPrice method. Calculate the Bond Clean Price.
	*  @param [in]		swapObjectName		Swap object name
    *  @param [in]		assetSwapParParSpread	Asset Swap Par Par Spread
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @return			Par/Par Asset Swap Spread
	*/
    double tryAqObjectsAssetSwapSpreadToCleanPrice( const std::string& swapObjectName, const double& assetSwapParParSpread, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames )
    {
	    VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS(swapObjectName, std::string(), swapObjectName, assetSwapParParSpread, valuationSettingsLVB,  fixingTableNames);

        auto swap = etrading::getSwap( swapObjectName );

		auto swapFixedLeg = etrading::getFixedLeg(swap);
		auto swapFloatLeg = etrading::getFloatLeg(swap);

		const bool isFixedFloatAssetSwap = (swapFixedLeg->getType() == etrading::FIXED_SCHEDULE_TYPE && swapFloatLeg->getType() == etrading::SWAPSCHEDULE_FLOATBOND)
								|| (swapFixedLeg->getType() == etrading::SWAPSCHEDULE_FIXEDBOND && swapFloatLeg->getType() == etrading::FLOAT_SCHEDULE_TYPE);

		AQ_REQUIRE(isFixedFloatAssetSwap, "Invalid Asset Swap Set-Up - Asset Swaps must be contain a FixedLeg and a FloatBondLeg, or a FixedBondLeg and FloatLeg")
			
		double bondCleanPrice = 0.0;
		if (swapFixedLeg->getType() == etrading::SWAPSCHEDULE_FIXEDBOND)
		{
			bondCleanPrice = etrading::calculateBondCleanPriceFromAssetSwapSpread(assetSwapParParSpread, swap, valuationSettingsLVB, fixingTableNames);
		}
		else if (swapFloatLeg->getType() == etrading::SWAPSCHEDULE_FLOATBOND)
		{
			bondCleanPrice = etrading::calculateBondCleanPriceFromFloatingBondAssetSwapSpread(assetSwapParParSpread, swap, valuationSettingsLVB, fixingTableNames);
		}

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapObjectName, std::string(), bondCleanPrice );

		VALID_EXCEPTION_END
    }

}
