/*
 * @brief			swap utils for the LWO Swap object related classes
 * @Created:		22 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "SwapUtilities.h"
#include "LADefinitions.h"
#include "VanillaIRSwap.h"
#include "CrossCurrencySwap.h"
#include "ZeroCouponSwap.h"
#include "XccyZeroCouponSwap.h"
#include "CreditDefaultSwap.h"
#include "ConstantMaturitySwap.h"
#include "TotalReturnSwap.h"
#include "ZeroCouponInflationSwap.h"
#include "FixedSchedule.h"
#include "FloatSchedule.h"
#include "FeeSchedule.h"
#include "LWOUtilities.h"
#include "FeeLeg.h"
#include "ContainerUtilities.h"
#include "LADateScheduleHelpers.h"
#include "LADateHelpers.h"
#include "ObjectUtilities.h"
#include "Fra.h"
#include "PremiumLeg.h"
#include "ProtectionLeg.h"
#include "FixedBondLeg.h"
#include "FloatBondLeg.h"
#include "CMSLeg.h"
#include "InflationLeg.h"
#include "SwapCalculation.h"

namespace etrading
{

	/* @brief Helper function which takes an input value string.
	*		  If the string is populated, copies the string value into value1 and value2.
	*		  Used for copying common swap parameters into both legs of a swap
	* @param[in]	value			The input string value
	* @param[inout]	value1			String parameter to set
	* @param[inout]	value2			String parameter to set
	* @param[out]	mandatoryField	Boolean flag which specifies whether a value should be provided
	*/
    void updateValuesFromGeneratorExpression( const LAString& value, LAString& value1, LAString& value2, const LAString& key, bool mandatoryField )
    {
        bool provideBoth = value.size() != 0 && (value1.size() != 0 || value2.size() != 0);
        bool provideNone = value.size() == 0 && value1.size() == 0 && value2.size() == 0;

        if (provideBoth || (mandatoryField && provideNone))
        {
            MLIB_THROW( "#Error: Please provide either: key '" + key + "' for both legs, or '" + key + "1'/'" + key + "2' for leg1/leg2");
        } 
        
        else if (value.size() != 0)
        {
            value1 = value;
            value2 = value;
        }
    }


	/* @brief			Get the schedule type enum from the given legName
    * @param [in]		legName  Leg name, e.g. leg1:fixed
    * @output			Schedule type enum
    */
    ScheduleTypeEnum getScheduleTypeFromLegName( const LAString& legName)
	{
        LAString schType = legName;
        ScheduleTypeEnum result;
        bool invalidFormat = false;

        if ( legName.findString( MULTI_STATIC_DATA_DELIMITER ) != -1 )
        {
            schType = legName.toToken( MULTI_STATIC_DATA_DELIMITER )[1];
            try
            {
                result = toScheduleTypeEnum(schType.getCString());
            }
            catch ( ETradingException& )
            {
                invalidFormat = true;
            }
        }
        else
        {
            invalidFormat = true; 
        }

        if (invalidFormat)
        {
            throw LACoreInvalidData("#Error: LegType needs to be in the format of 'LegId:Type', e.g. 'Leg1:Fixed', 'Leg2:Float', 'Leg3:Fee'",__FILE__,__LINE__);
        }
		return result;
	}

    /* @brief			Return a leg pointer based on the Leg Label Value Block and Schedule
    *  @param [in]		legLVB              Leg label value block
    *  @param [in]		legObjectName Leg   object name
    *  @param [in]		schedule            Schedule of the leg
    *  @return			Leg pointer
    */
	LegPtr createLegByLVB(const LabelValueBlock& legLVB, const std::string& legObjectName, const SchedulePtr& schedule)
	{
		LAString legName = legLVB.getCompulsoryValueAsLAString(IRS_KEY::LEG_TYPE);

		ScheduleTypeEnum legScheduleType = getScheduleTypeFromLegName(legName);

		if (schedule != nullptr && legScheduleType != schedule->getScheduleType())
        {
            throw LACoreInvalidData("#Error: The scheduleType from Leg and Schedule should be the same",__FILE__,__LINE__);
        }
	
		LegPtr ret; 
		switch(legScheduleType)
		{
            case FIXED_SCHEDULE_TYPE:
				ret = LegPtr(new FixedLeg(legLVB, legObjectName, schedule));
	            break;
            case FLOAT_SCHEDULE_TYPE:
				ret = LegPtr(new FloatLeg(legLVB, legObjectName, schedule));
	            break;
			case FEE_SCHEDULE_TYPE:
				ret = LegPtr(new FeeLeg(legLVB, legObjectName, schedule));
				break;
			case FRA_SCHEDULE_TYPE:
				ret = LegPtr(new Fra(legObjectName, legLVB, schedule));
				break;
			case PREMIUM_SCHEDULE_TYPE:
				ret = LegPtr( new PremiumLeg( legLVB, legObjectName, schedule) );
				break;
			case PROTECTION_SCHEDULE_TYPE:
				ret = LegPtr( new ProtectionLeg( legLVB, legObjectName, schedule) );
				break;
			case CMS_SCHEDULE_TYPE:
				ret = LegPtr( new CMSLeg( legLVB, legObjectName, schedule ) );
				break;
			case INFLATION_SCHEDULE_TYPE:
				ret = LegPtr(new InflationLeg(legLVB, legObjectName, schedule));
				break;
			case SWAPSCHEDULE_FIXEDBOND:
				ret = LegPtr(new FixedBondLeg(legLVB, legObjectName, schedule));
				break;
			case SWAPSCHEDULE_FLOATBOND:
				ret = LegPtr(new FloatBondLeg(legLVB, legObjectName, schedule));
				break;
			default:
		        throw LACoreInvalidData("#Error: Leg type can only be either 'FIXED', 'FLOAT', 'FEE', 'FRA', 'PREMIUM', 'PROTECTION', 'CMS', FIXEDBOND, or 'FLOATBOND'",__FILE__,__LINE__);
                break;
        }

		return ret;
	}

	/* @brief Create Swap object  on the leg label value blocks
	*  @param [in]	swapName		Swap name
	*  @param [in]	leg1			Leg1
	*  @param [in]	leg2			Leg2 
	*  @param [in]	swapPropertiesLVB	Swap level properties
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwapFromLegs(const std::string& swapName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB)
	{

        auto firstLeg = leg1;
        auto secondLeg = leg2;
        if (isFloatLeg(leg1->getType()) && isFixedLeg(leg2->getType()))
        {
            //So the first leg is always fixed
            firstLeg = leg2;
            secondLeg = leg1;
        }

		std::shared_ptr<Swap> mySwap;

        SwapTypeEnum swapTypeEnum;

		if ( firstLeg->getType() == CMS_SCHEDULE_TYPE || 
			 secondLeg->getType() == CMS_SCHEDULE_TYPE )
		{
			swapTypeEnum = CONSTANT_MATURITY_SWAP;
		}
		else if ( firstLeg->getType() == PREMIUM_SCHEDULE_TYPE )
		{
			if ( secondLeg->getType() == PROTECTION_SCHEDULE_TYPE )
			{
				swapTypeEnum = CREDIT_DEFAULT_SWAP;
			}
			else
			{
				swapTypeEnum = TOTAL_RETURN_SWAP;
			}
		}
		else if ( secondLeg->getType() == PREMIUM_SCHEDULE_TYPE )
		{
			if ( firstLeg->getType() == PROTECTION_SCHEDULE_TYPE )
			{
				swapTypeEnum = CREDIT_DEFAULT_SWAP;
			}
			else
			{
				swapTypeEnum = TOTAL_RETURN_SWAP;
			}
		}
		else if ( ( firstLeg->getType() == INFLATION_SCHEDULE_TYPE ) || ( secondLeg->getType() == INFLATION_SCHEDULE_TYPE ) )
		{
			// If we have an Inflation leg, check that the opposite leg is a fixed leg
			if ( ( secondLeg->getType() == FIXED_SCHEDULE_TYPE ) || (firstLeg->getType() == FIXED_SCHEDULE_TYPE ) )
			{
				swapTypeEnum = ZERO_COUPON_INFLATION_SWAP;
			}
			else
			{
				MLIB_THROW( "Error: Only Fixed Leg Zero Coupon Inflation Swaps are supported." );
			}
		}
		else
		{
			// 1) Using both legs' currencies to decide if it is cross currency swap
			// 2) Using first leg's getPaymentFreqEnum flag to decide if it is a zero coupon swap

			bool withAtMaturityFreq = (firstLeg->getSchedule()->isPaymentFreqEnumAtMaturity() || secondLeg->getSchedule()->isPaymentFreqEnumAtMaturity());

			if ( firstLeg->getStaticData()->getCurrency() != secondLeg->getStaticData()->getCurrency() )
			{
				if (withAtMaturityFreq)
				{
					swapTypeEnum = XCCY_ZERO_COUPON_SWAP;
				}
				else
				{
					swapTypeEnum = CROSS_CURRENCY_SWAP;
				}
			}
			else
			{
				if (withAtMaturityFreq)
				{
					swapTypeEnum = ZERO_COUPON_SWAP;
				}
				else
				{
					swapTypeEnum = VANILLA_SWAP;
				}
			}
		}

        switch(swapTypeEnum)
		{
			case VANILLA_SWAP:
                mySwap = std::shared_ptr<Swap>(new VanillaIRSwap(swapName, firstLeg, secondLeg, swapPropertiesLVB));
				break;
			case CROSS_CURRENCY_SWAP:
				mySwap = std::shared_ptr<Swap>(new CrossCurrencySwap(swapName, firstLeg, secondLeg, swapPropertiesLVB));
				break;
			case ZERO_COUPON_SWAP:
				mySwap = std::shared_ptr<Swap>(new ZeroCouponSwap(swapName, firstLeg, secondLeg, swapPropertiesLVB));
				break;
			case XCCY_ZERO_COUPON_SWAP:
				mySwap = std::shared_ptr<Swap>(new XccyZeroCouponSwap(swapName, firstLeg, secondLeg, swapPropertiesLVB));
				break;
			case CREDIT_DEFAULT_SWAP:
				mySwap = std::shared_ptr<Swap>( new CreditDefaultSwap( swapName, firstLeg, secondLeg, swapPropertiesLVB ));
				break;
			case CONSTANT_MATURITY_SWAP:
				mySwap = std::shared_ptr<Swap>( new ConstantMaturitySwap( swapName, firstLeg, secondLeg, swapPropertiesLVB ));
				break;
			case TOTAL_RETURN_SWAP:
				mySwap = std::shared_ptr<Swap>( new TotalReturnSwap( swapName, firstLeg, secondLeg, swapPropertiesLVB ));
				break;
			case ZERO_COUPON_INFLATION_SWAP:
				mySwap = std::shared_ptr<Swap>(new ZeroCouponInflationSwap( swapName, firstLeg, secondLeg, swapPropertiesLVB ));
				break;
			default:
			    throw LACoreInvalidData("#Error: Swap type must be 'VanillaSwap', 'CrossCurrencySwap', 'ZeroCouponSwap', 'XccyZeroCouponSwap', 'CreditDefaultSwap', 'ConstantMaturitySwap', 'TotalReturnSwap' or 'ZeroCouponInflationSwap'.",__FILE__,__LINE__);
				break;
		}

		return mySwap;
	}

    /* @brief Create Swap object from leg objects
    *  @param [in]	swapName		Swap name
	*  @param [in]	legs			Legs
	*  @param [in]	swapPropertiesLVB	Swap level properties
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwapFromLegs(const std::string& swapName, const std::vector<LegPtr>& legs, const LabelValueBlock& swapPropertiesLVB)
    {
   		if (legs.size() < 2) 
		{
			throw LACoreInvalidData( "#Error: Swap must have at least two legs", __FILE__, __LINE__ );
		}

        auto mySwap = createSwapFromLegs(swapName, legs[0], legs[1], swapPropertiesLVB);
        for (size_t i = 2; i < legs.size(); ++i)
		{
            auto leg = legs[i];
			mySwap->addToLegCollection(leg);
		}
        return mySwap;
	}

	/* @brief Create Swap object based on the leg label value blocks
	*  @param [in]	swapName		Swap name
	*  @param [in]	leg1LVB			Leg1 label value block 
	*  @param [in]	leg2LVB			Leg2 label value block 
	*  @param [in]	swapPropertiesLVB	Swap level properties
	*  @param [in]	schedule1		Schedule1	
	*  @param [in]	schedule2		Schedule2	
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwap(const std::string& swapName, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, const SchedulePtr& schedule1, const SchedulePtr& schedule2)
	{

		ScheduleTypeEnum leg1ScheduleType = getScheduleTypeFromLegName(leg1LVB.getCompulsoryValueAsLAString(IRS_KEY::LEG_TYPE));
		ScheduleTypeEnum leg2ScheduleType = getScheduleTypeFromLegName(leg2LVB.getCompulsoryValueAsLAString(IRS_KEY::LEG_TYPE));

		LegPtr leg1;
		LegPtr leg2;

		//For a FixedFloatSwap, check if it is a ZeroCouponSwap and if FVNotional is provided, if so the floatLeg's notional need to have a default value
		if ( (isFixedLeg(leg1ScheduleType) && isFloatLeg(leg2ScheduleType)) || (isFloatLeg(leg1ScheduleType) && isFixedLeg(leg2ScheduleType) ))
		{
			auto fixedLegLVB = isFixedLeg(leg1ScheduleType) ? leg1LVB : leg2LVB;
			auto floatLegLVB = isFloatLeg(leg1ScheduleType) ? leg1LVB : leg2LVB;

			auto floatLegPaymentFreq = toFrequencyEnum(floatLegLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FLOAT_PAYMENTFREQUENCY, IRS_KEY::PAYMENTFREQUENCY).getCString());
			if (floatLegPaymentFreq == AT_MATURITY_FREQUENCY)
			{
				auto floatLegNotional = floatLegLVB.getOptionalValueAsDouble(IRS_KEY::NOTIONAL, std::numeric_limits<double>::quiet_NaN());
				auto fixedLegFutureValueNotional  = fixedLegLVB.getOptionalValueAsDouble(IRS_KEY::FV_NOTIONAL, std::numeric_limits<double>::quiet_NaN());

				if (boost::math::isnan(floatLegNotional) && !boost::math::isnan(fixedLegFutureValueNotional) )
				{
					const std::string defaultNotional = boost::lexical_cast<std::string>(1e6);
					floatLegLVB = LabelValueBlock( floatLegLVB, IRS_KEY::NOTIONAL, defaultNotional );
				}
			}
			leg1 = createLegByLVB(fixedLegLVB, swapName, schedule1);
			leg2 = createLegByLVB(floatLegLVB, swapName, schedule2);
		}
		else
		{
			leg1 = createLegByLVB(leg1LVB, swapName, schedule1);
			leg2 = createLegByLVB(leg2LVB, swapName, schedule2);
		}

		return createSwapFromLegs(swapName, leg1, leg2, swapPropertiesLVB);
	}

	/* @brief Create Swap object based on the legs' label value block
	*  @param [in]	swapName		Swap name
	*  @param [in]	legsLVB			The label value block for multiple legs
	*  @param [in]	schedules		Schedules for multiple legs 
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwap(const std::string& swapName, const std::vector<LabelValueBlock>& legsLVB, const LabelValueBlock& swapPropertiesLVB, const std::vector<SchedulePtr>& schedules)
	{

		if (legsLVB.size() < 2) 
		{
			throw LACoreInvalidData( "#Error: Swap must have at least two legs", __FILE__, __LINE__ );
		}

		SchedulePtr schedule1;
		SchedulePtr schedule2;
		if (schedules.size() != 0) 
		{
			if (legsLVB.size() != schedules.size())
			{
				throw LACoreInvalidData( "#Error: The size of legLVBs and schedules are not matched", __FILE__, __LINE__ );
			}
			schedule1 = schedules[0];
			schedule2 = schedules[1];
		}

		auto mySwap = createSwap(swapName, legsLVB[0], legsLVB[1], swapPropertiesLVB, schedule1, schedule2);

		// When the swap has more than 2 legs, add extra legs:
		SchedulePtr schedule;
		for (size_t i = 2; i < legsLVB.size(); ++i)
		{
			if (schedules.size()!=0)
			{
				schedule = schedules[i];
			}
			LegPtr leg = createLegByLVB(legsLVB[i], swapName, schedule);
			mySwap->addToLegCollection(leg);
		}

		return mySwap;
	}

	void updateLVB(LabelValueBlock& legLVB, const std::string lvbKey, const LAString& overrideValue)
	{
		if (overrideValue.size() != 0)
		{
            legLVB = LabelValueBlock( legLVB, LAString( lvbKey ), overrideValue );
		}
	}


	/* @brief Create Swap object based on a swapGenerator config
	*  @param [in]	swapName			Swap name
	*  @param [in]	swapGeneratorName	Name of the LWO SwapGenerator to use
	*  @param [in]	expressionLVB		Additional swap config
	*  @param [in]	swapPropertiesLVB	Swap level properties
	*  @param [in]	isXccySwap			Boolean flag which specifies whether this is an ordinary IRS or a Xccy swap
    *  @Return		a pointer to the Swap object
	*/
	std::shared_ptr<Swap> createSwapFromGenerator(const std::string& swapName, const std::string& swapGeneratorName, const LabelValueBlock& expressionLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap )
	{	
        const std::string inputLVB = "expressionLVB";

        // Raw Trade Parameters before Enrichment
        auto swapGen = etrading::getSwapGenerator(swapGeneratorName);
        
        // Leg LVB Key-Value Placeholders
        LAStringVector leg1LVBKeys;
        LAStringVector leg1LVBValues;
        
        LAStringVector leg2LVBKeys;
        LAStringVector leg2LVBValues;

        // Reserve Vector Sizes to speed up the data initialization push_backs below
        leg1LVBKeys.reserve(100);
        leg1LVBValues.reserve(100);
        leg2LVBKeys.reserve(100);
        leg2LVBValues.reserve(100);

        // Get Legname from Leg1LVB
        LabelValueBlock leg1LVB = swapGen->getLegGenerator(0).getInputParameters();
        auto leg1Name = leg1LVB.getOptionalValueAsLAString(etrading::IRS_KEY::LEG_TYPE);

        etrading::ScheduleTypeEnum schedule1Type = etrading::getScheduleTypeFromLegName(leg1Name);
		switch(schedule1Type)
		{
            case etrading::FIXED_SCHEDULE_TYPE:
			{
				auto rate1 = expressionLVB.getCompulsoryValueAsString(etrading::SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1, inputLVB);
                leg1LVBKeys.push_back(etrading::IRS_KEY::FIXED_RATE);
                leg1LVBValues.push_back(rate1);
	            break;
			}
            case etrading::FLOAT_SCHEDULE_TYPE:
			{
				auto rate1 = expressionLVB.getCompulsoryValueAsString(etrading::SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1, inputLVB);
                leg1LVBKeys.push_back(etrading::IRS_KEY::FLOAT_SPREAD);
                leg1LVBValues.push_back(rate1);
	            break;
			}
			case etrading::PREMIUM_SCHEDULE_TYPE:
			{
				// if a bondName has been provided, we will fetch the coupon rate from the bond
				auto bondName = expressionLVB.getOptionalValueAsString( TRS_KEY::BOND_NAME );
				leg1LVBKeys.push_back(TRS_KEY::BOND_NAME);
                leg1LVBValues.push_back(bondName);
				
                auto assetPerformanceChoice = expressionLVB.getOptionalValueAsString( TRS_KEY::ASSET_PERFORMANCE );
				leg1LVBKeys.push_back(TRS_KEY::ASSET_PERFORMANCE);
                leg1LVBValues.push_back(assetPerformanceChoice);

				// rate or spread is optional for the premium leg, in the case we have been given a bondName.
				auto rate1 = expressionLVB.getOptionalValueAsString(etrading::SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1 );
				leg1LVBKeys.push_back(etrading::CDS_KEY::CDS_SPREAD);
                leg1LVBValues.push_back(rate1);
				break;
			}

			case etrading::CMS_SCHEDULE_TYPE:
			{
				auto cmsIndexMaturity1   = expressionLVB.getCompulsoryValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MATURITY1 );
				leg1LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MATURITY1);
                leg1LVBValues.push_back(cmsIndexMaturity1.getCString());
				
				auto cmsIndexMultiplier1 = expressionLVB.getCompulsoryValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER1 );
				leg1LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER1);
                leg1LVBValues.push_back(cmsIndexMultiplier1.getCString());

				auto cmsIndexMaturity2   = expressionLVB.getOptionalValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MATURITY2 );
				leg1LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MATURITY2);
                leg1LVBValues.push_back(cmsIndexMaturity2.getCString());

				auto cmsIndexMultiplier2 = expressionLVB.getOptionalValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER2 );
				leg1LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER2);
                leg1LVBValues.push_back(cmsIndexMultiplier2.getCString());
				break;
			}
			case etrading::INFLATION_SCHEDULE_TYPE:
			case etrading::PROTECTION_SCHEDULE_TYPE:
			case etrading::SWAPSCHEDULE_FIXEDBOND:
			case etrading::SWAPSCHEDULE_FLOATBOND:
				// Nothing additional to add.
				break;
			default:
		        throw LACoreInvalidData("#Error: Leg type can only be either 'FIXED', 'FLOAT', 'PREMIUM', 'PROTECTION', 'CMS', 'INFLATION', 'FLOATBOND', or 'FIXEDBOND''",__FILE__,__LINE__);
                break;
        }

        // Get Legname from Leg2LVB
        LabelValueBlock leg2LVB = swapGen->getLegGenerator(1).getInputParameters();
        auto leg2Name = leg2LVB.getOptionalValueAsLAString(etrading::IRS_KEY::LEG_TYPE);
        auto rate2 = expressionLVB.getCompulsoryValueAsLAString(etrading::SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2, inputLVB);

        etrading::ScheduleTypeEnum schedule2Type = etrading::getScheduleTypeFromLegName(leg2Name);
		switch(schedule2Type)
		{
            case etrading::FIXED_SCHEDULE_TYPE:
                leg2LVBKeys.push_back(etrading::IRS_KEY::FIXED_RATE);
                leg2LVBValues.push_back(rate2.getCString());
	            break;
            case etrading::FLOAT_SCHEDULE_TYPE:
                leg2LVBKeys.push_back(etrading::IRS_KEY::FLOAT_SPREAD);
                leg2LVBValues.push_back(rate2.getCString());
	            break;
			case etrading::PREMIUM_SCHEDULE_TYPE:
                leg2LVBKeys.push_back(etrading::CDS_KEY::CDS_SPREAD);
                leg2LVBValues.push_back(rate2.getCString());
				break;
			case etrading::INFLATION_SCHEDULE_TYPE:
				// *** TODO: Any special handingly required here?
				//leg2LVBKeys.push_back(etrading::CDS_KEY::CDS_SPREAD);
				//leg2LVBValues.push_back(rate2.getCString());
				break;

			case etrading::CMS_SCHEDULE_TYPE:
			{
				auto cmsIndexMaturity1   = expressionLVB.getCompulsoryValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MATURITY1 );
				leg2LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MATURITY1);
                leg2LVBValues.push_back(cmsIndexMaturity1.getCString());

				auto cmsIndexMultiplier1 = expressionLVB.getCompulsoryValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER1 );
				leg2LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER1);
                leg2LVBValues.push_back(cmsIndexMultiplier1.getCString());

				auto cmsIndexMaturity2   = expressionLVB.getOptionalValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MATURITY2 );
				leg2LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MATURITY2);
                leg2LVBValues.push_back(cmsIndexMaturity2.getCString());

				auto cmsIndexMultiplier2 = expressionLVB.getOptionalValueAsLAString(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER2 );
				leg2LVBKeys.push_back(etrading::CMS_KEY::CMS_INDEX_MULTIPLIER2);
                leg2LVBValues.push_back(cmsIndexMultiplier2.getCString());
				break;
			}
			case etrading::PROTECTION_SCHEDULE_TYPE:
			case etrading::SWAPSCHEDULE_FIXEDBOND:
			case etrading::SWAPSCHEDULE_FLOATBOND:
				// Nothing additional to add.
				break;
			default:
		        throw LACoreInvalidData("#Error: Leg type can only be either 'FIXED', 'FLOAT', 'PREMIUM', 'PROTECTION', 'CMS', 'INFLATION', 'FLOATBOND', or ''FIXEDBOND",__FILE__,__LINE__);
                break;
        }

        //payReceive
        auto payReceive = toPayReceiveEnum(expressionLVB.getOptionalValueAsLAString(etrading::IRS_KEY::PAY_RECEIVE).getCString());
        auto payReceive1 = toPayReceiveEnum(expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::PAY_RECEIVE1).getCString());
        auto payReceive2 = toPayReceiveEnum(expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::PAY_RECEIVE2).getCString());

        bool provideBoth = payReceive != NONE_PAYRECEIVE_ENUM && (payReceive1 != NONE_PAYRECEIVE_ENUM || payReceive2 != NONE_PAYRECEIVE_ENUM);
        bool provideNone = payReceive == NONE_PAYRECEIVE_ENUM && payReceive1 == NONE_PAYRECEIVE_ENUM && payReceive2 == NONE_PAYRECEIVE_ENUM;
        if (provideBoth || provideNone)
        {
            throw LACoreInvalidData( "#Error: Please provide either 'PayReceive' or 'PayReceive1/PayReceive2'", __FILE__, __LINE__ );
        } 
        else if (payReceive != NONE_PAYRECEIVE_ENUM)
        {
            payReceive1 = payReceive;
            payReceive2 = etrading::flipPayReceive(payReceive1);
        }
        else if (payReceive2 == NONE_PAYRECEIVE_ENUM)
        {
            payReceive2 = etrading::flipPayReceive(payReceive1);
        }
        else if (payReceive1 == NONE_PAYRECEIVE_ENUM)
        {
            payReceive1 = etrading::flipPayReceive(payReceive2);
        }
        else // both payReceive1 & payReceive2 are provided
        {
            //do nothing
        }

        // Pay Receive Flags
        leg1LVBKeys.push_back(etrading::IRS_KEY::PAY_RECEIVE);
        leg1LVBValues.push_back(toString(payReceive1));
        
        leg2LVBKeys.push_back(etrading::IRS_KEY::PAY_RECEIVE);
        leg2LVBValues.push_back(toString(payReceive2));

         //notional
        auto notional = expressionLVB.getOptionalValueAsLAString(etrading::IRS_KEY::NOTIONAL);
        auto notional1 = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::NOTIONAL1);
        auto notional2 = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::NOTIONAL2);

        updateValuesFromGeneratorExpression(notional, notional1, notional2, etrading::IRS_KEY::NOTIONAL.c_str(), true);

        leg1LVBKeys.push_back(etrading::IRS_KEY::NOTIONAL);
        leg1LVBValues.push_back(notional1.getCString());
        
        leg2LVBKeys.push_back(etrading::IRS_KEY::NOTIONAL);
        leg2LVBValues.push_back(notional2.getCString());

         //firstFixing
        auto firstFixing = expressionLVB.getOptionalValueAsLAString(etrading::IRS_KEY::FIRSTFIXING);
        auto firstFixing1 = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::FIRSTFIXING1);
        auto firstFixing2 = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::FIRSTFIXING2);

        updateValuesFromGeneratorExpression(firstFixing, firstFixing1, firstFixing2,etrading::IRS_KEY::FIRSTFIXING.c_str(), false);

        leg1LVBKeys.push_back(etrading::IRS_KEY::FIRSTFIXING);
        leg1LVBValues.push_back(firstFixing1.getCString());
        
        leg2LVBKeys.push_back(etrading::IRS_KEY::FIRSTFIXING);
        leg2LVBValues.push_back(firstFixing2.getCString());

        //lastFixing
        auto lastFixing = expressionLVB.getOptionalValueAsLAString(etrading::IRS_KEY::LASTFIXING);
        auto lastFixing1 = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::LASTFIXING1);
        auto lastFixing2 = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::LASTFIXING2);

        updateValuesFromGeneratorExpression(lastFixing, lastFixing1, lastFixing2, etrading::IRS_KEY::LASTFIXING.c_str(), false);

        leg1LVBKeys.push_back(etrading::IRS_KEY::LASTFIXING);
        leg1LVBValues.push_back(lastFixing1.getCString());
        
        leg2LVBKeys.push_back(etrading::IRS_KEY::LASTFIXING);
        leg2LVBValues.push_back(lastFixing2.getCString());

        //EffectiveDate & MaturityDate
        auto effectiveDate = expressionLVB.getCompulsoryValueAsLAString(etrading::IRS_KEY::EFFECTIVE_DATE, inputLVB);
        auto maturity = expressionLVB.getCompulsoryValueAsLAString(etrading::IRS_KEY::MATURITY_DATE, inputLVB);
        
        leg1LVBKeys.push_back(etrading::IRS_KEY::EFFECTIVE_DATE);
        leg1LVBValues.push_back(effectiveDate.getCString());
        
        leg1LVBKeys.push_back(etrading::IRS_KEY::MATURITY_DATE);
        leg1LVBValues.push_back(maturity.getCString());
        
        leg2LVBKeys.push_back(etrading::IRS_KEY::EFFECTIVE_DATE);
        leg2LVBValues.push_back(effectiveDate.getCString());
        
        leg2LVBKeys.push_back(etrading::IRS_KEY::MATURITY_DATE);
        leg2LVBValues.push_back(maturity.getCString());

        //fwdInter
        auto isFwdInter     = expressionLVB.getOptionalValueAsLAString(etrading::IRS_KEY::IS_FWD_INTER);
        auto isFwdInter1    = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::IS_FWD_INTER1);
        auto isFwdInter2    = expressionLVB.getOptionalValueAsLAString(etrading::SWAP_EXPRESSION_KEY::IS_FWD_INTER2);

        updateValuesFromGeneratorExpression(isFwdInter, isFwdInter1, isFwdInter2, IRS_KEY::IS_FWD_INTER.c_str(), false);

        // Create New Leg LVBs by adding above keys to default input parameters from the leg generators
        LabelValueBlock updatedLeg1LVB( leg1LVB, leg1LVBKeys, leg1LVBValues );
        LabelValueBlock updatedLeg2LVB( leg2LVB, leg2LVBKeys, leg2LVBValues );

		// Only update if the flag is specified, if not, use the default one in the Generator File
		updateLVB( updatedLeg1LVB, etrading::IRS_KEY::IS_FWD_INTER, isFwdInter1 );
		updateLVB( updatedLeg2LVB, etrading::IRS_KEY::IS_FWD_INTER, isFwdInter2 );
        
        //Create swaps
        auto leg1 = etrading::createLegByLVB( updatedLeg1LVB );
        auto leg2 = etrading::createLegByLVB( updatedLeg2LVB );
        etrading::validateSwapCurrency( isXccySwap, updatedLeg1LVB, updatedLeg2LVB );

        std::shared_ptr<Swap> mySwapObject = etrading::createSwapFromLegs( swapName, leg1, leg2, swapPropertiesLVB );
        
        return mySwapObject;
	}

	/* @brief Create a swap that is back to back to the original swap
	*  @param [in]	originalSwap The original swap
	*  @param [in]	swapName	 The new swap name

    *  @Return		a pointer to a new Swap object, 
    */
	std::shared_ptr<Swap> createBackToBackSwap(const std::shared_ptr<Swap> originalSwap, const std::string& swapName)
	{
        auto swap = originalSwap->clone();
        swap->changeName(swapName);
        for (size_t i = 0; i < swap->getLegSize(); ++i)
        {
            LegPtr leg = swap->getLeg(i);
            leg->flipPayerReceiver();
        }
        
        return swap;
	}

	/* @brief Create Schedule object based on the label value block
    *  @param [in] scheduleName	   Schedule Name
    *  @param [in] swapScheduleLVB Schedule Label Value Block
    *  @Return     a pointer to the schedule object
    */
	std::shared_ptr<Schedule> createSchedule(const std::string& scheduleName, const LabelValueBlock& swapScheduleLVB)
	{
		const std::string inputLVB = "swapScheduleLVB";

		LAString scheduleTypeString = swapScheduleLVB.getOptionalValueAsLAString( IRS_KEY::SCHEDULE_TYPE, "");

		std::shared_ptr<Schedule> mySchedule;
		if (scheduleTypeString.size() != 0)
        {
			ScheduleTypeEnum schType = toScheduleTypeEnum(scheduleTypeString.getCString());
			switch(schType)
			{
				case FIXED_SCHEDULE_TYPE:
					mySchedule = std::shared_ptr<Schedule>(new FixedSchedule(swapScheduleLVB, scheduleName));
					break;
				case FLOAT_SCHEDULE_TYPE:
					mySchedule = std::shared_ptr<Schedule>(new FloatSchedule(swapScheduleLVB, scheduleName));
					break;
				case SWAPSCHEDULE_FIXEDBOND:
					mySchedule = std::shared_ptr<Schedule>(new FixedBondLegSchedule(swapScheduleLVB, scheduleName));
					break;
				case SWAPSCHEDULE_FLOATBOND:
					mySchedule = std::shared_ptr<Schedule>(new FloatBondLegSchedule(swapScheduleLVB, scheduleName));
					break;
				default:
			        throw LACoreInvalidData("#Error: Schedule type can only be either 'FIXED', 'FLOAT' or 'FIXEDBOND'",__FILE__,__LINE__);
					break;
			}
		} 
		else
		{
			mySchedule = std::shared_ptr<Schedule>(new Schedule(swapScheduleLVB, scheduleName));
		}

		return mySchedule;
	}

	/* @brief Create a Bespoke Schedule object based on the label value blocks of schedule properties and schedule cashflows 
	*  @param [in]		scheduleName				Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties	Bespoke schedule properties label value block
	*  @param [in]		cashflowLVBs				Bespoke schedule cashflow label value block
	*  @param [in]		bespokeScheduleType			Bespoke type - internally used
	*  @Return     a pointer to the schedule object
    */
	std::shared_ptr<Schedule> createScheduleBespoke(const std::string& scheduleName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType)
	{

		MLIB_REQUIRE(bespokeScheduleProperties.size() > 0, "BespokeScheduleProperties cannot be empty");
		MLIB_REQUIRE(cashflowLVBs.size() > 0, "BespokeSchedule Cashflow LVB cannot be empty");

		const std::string inputLVB = "bespokeScheduleProperties";

		LAString scheduleTypeString = bespokeScheduleProperties.getCompulsoryValueAsLAString( IRS_KEY::SCHEDULE_TYPE, inputLVB);
		if (scheduleTypeString.size() == 0)
        {
            throw LACoreInvalidData("#Error: Schedule type can only be either 'FIXED', 'FLOAT' for BespokeSchedule",__FILE__,__LINE__);
        }

		std::shared_ptr<Schedule> mySchedule;
        ScheduleTypeEnum schType = toScheduleTypeEnum(scheduleTypeString.getCString());
		switch(schType)
		{
			case FIXED_SCHEDULE_TYPE:
				mySchedule = SchedulePtr(new FixedSchedule(scheduleName, bespokeScheduleProperties, cashflowLVBs, bespokeScheduleType));
				break;
			case FLOAT_SCHEDULE_TYPE:
				mySchedule = SchedulePtr(new FloatSchedule(scheduleName, bespokeScheduleProperties, cashflowLVBs, bespokeScheduleType));
				break;
			default:
			    throw LACoreInvalidData("#Error: Schedule type can only be either 'FIXED', 'FLOAT' for BespokeSchedule",__FILE__,__LINE__);
				break;
		}

        return mySchedule;
	}

	/* @brief label value block keys for the swap leg
    *  @param [in]	legName				Leg name, e.g. leg1:fixed
	*  @param [in]	withScheduleKeys	True to include keys from Schedule (Fixed/Float). Default to true
    *  @Return     label value block keys for the swap leg
    */
	std::vector<std::string> getSwapLegLVBKeys(const LAString& legName, bool withScheduleKeys) 
	{
		ScheduleTypeEnum schType = getScheduleTypeFromLegName(legName);
        switch(schType)
		{
			case FIXED_SCHEDULE_TYPE:
			case SWAPSCHEDULE_FIXEDBOND:
				return (withScheduleKeys ? FixedLeg::lvbKeys() : FixedLeg::legLVBWithoutScheduleKeys());
			case FLOAT_SCHEDULE_TYPE:
			case SWAPSCHEDULE_FLOATBOND:
				return (withScheduleKeys ? FloatLeg::lvbKeys() : FloatLeg::legLVBWithoutScheduleKeys());
			case FEE_SCHEDULE_TYPE:
				return (withScheduleKeys ? FeeLeg::lvbKeys() : FeeLeg::legLVBWithoutScheduleKeys());
			case PREMIUM_SCHEDULE_TYPE:
				return (withScheduleKeys ? PremiumLeg::lvbKeys() : PremiumLeg::legLVBWithoutScheduleKeys());
			case PROTECTION_SCHEDULE_TYPE:
				return (withScheduleKeys ? ProtectionLeg::lvbKeys() : ProtectionLeg::legLVBWithoutScheduleKeys());
			case CMS_SCHEDULE_TYPE:
				return (withScheduleKeys ? CMSLeg::lvbKeys() : CMSLeg::legLVBWithoutScheduleKeys());
			case INFLATION_SCHEDULE_TYPE:
				return (withScheduleKeys ? InflationLeg::lvbKeys() : InflationLeg::legLVBWithoutScheduleKeys());
			case FRA_SCHEDULE_TYPE:
        		return (withScheduleKeys ? Fra::lvbKeys() : Fra::descriptionLVBKeys());  
			default:
			    throw LACoreInvalidData("#Error: Schedule type can only be either 'FIXED', 'FLOAT', 'FEE', 'PREMIUM', 'PROTECTION', 'CMS', 'FRA', 'FIXEDBOND', or 'FLOATBOND''",__FILE__,__LINE__);
				break;
		}
	}

	/* @brief label value block keys for the swap schedule
    *  @param [in] scheduleType	   Schedule type, e.g. fixed/float/fee
    *  @Return     label value block keys for the swap schedule
    */
	std::vector<std::string> getScheduleLVBKeys(const LAString& scheduleType) 
	{
		if (scheduleType.size() == 0)
		{
			return Schedule::lvbKeys();
		}

		ScheduleTypeEnum schType = toScheduleTypeEnum(scheduleType.getCString());
        switch(schType)
		{
			case FIXED_SCHEDULE_TYPE:
			case SWAPSCHEDULE_FIXEDBOND:
				return FixedSchedule::lvbKeys();
			case FLOAT_SCHEDULE_TYPE:
        		return FloatSchedule::lvbKeys(); 
			case FEE_SCHEDULE_TYPE:
        		return FeeSchedule::lvbKeys();  
			case FRA_SCHEDULE_TYPE:
        		return FraSchedule::lvbKeys();  
			default:
			    throw LACoreInvalidData("#Error: Schedule type can only be either 'FIXED', 'FIXEDBOND', 'FLOAT', 'FEE', or 'FRA'",__FILE__,__LINE__);
				break;
		}
	}

	/* @brief			Validate if the swap keys, with or without prefix "fixed"/"float" 
	*  @param [in]		legName				Leg name, e.g. leg1:fixed
	*  @param [in]		keysFromUser		A list of keys from user input
	*  @param [in]		validateKeys		True to do the verification. Default to true
	*  @param [in]		withScheduleKeys	True to include keys from Schedule (Fixed/Float). Default to true
	*/
	void validateSwapLegLVBKeys(const LAString& legName, const std::vector<std::string>& keysFromUser, bool validateKeys, bool withScheduleKeys)
	{
		if (!validateKeys)
		{
			return;
		}

		std::vector<std::string> expectedKeys = getSwapLegLVBKeys(legName, withScheduleKeys);
		std::vector<std::string> keys = keysFromUser;

		ScheduleTypeEnum schType = getScheduleTypeFromLegName(legName);
		if (isFixedLeg(schType) || schType == PREMIUM_SCHEDULE_TYPE || schType == PROTECTION_SCHEDULE_TYPE || schType == INFLATION_SCHEDULE_TYPE)
		{
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::FLOAT_SPREAD), keys.end()); 
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT), keys.end());
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::FIXINGCALENDAR), keys.end());
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::FIXINGLAG), keys.end());
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::FIRSTFIXING), keys.end());
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::LASTFIXING), keys.end()); 
		}
		else if (isFloatLeg(schType) || schType == CMS_SCHEDULE_TYPE )
		{
			keys.erase(std::remove(keys.begin(), keys.end(), IRS_KEY::FIXED_RATE), keys.end()); 
		}
		else if (schType == FEE_SCHEDULE_TYPE || schType == FRA_SCHEDULE_TYPE)
		{
			//Do nothing
		}
		else 
		{
	        throw LACoreInvalidData("#Error: Leg type can only be either 'FIXED', 'FIXEDBOND', 'FLOAT', 'PREMIUM', 'PROTECTION', 'CMS', 'INFLATION' or 'FEE' leg",__FILE__,__LINE__);
		}

		validateKeysForLVB(expectedKeys, keys, validateKeys);
	}

	/* @brief Create a Bespoke Schedule object based on the label value blocks of schedule properties and schedule cashflows
	*  @param [in]		scheduleName				Fee schedule name
	*  @param [in]		cashflowLVBs				Fee cashflow label value blocks
	*  @Return     a pointer to the schedule object
	*/
	std::shared_ptr<Schedule> createFeeSchedule(const std::string& scheduleName, const std::vector<LabelValueBlock>& cashflowLVBs)
	{
		std::shared_ptr<Schedule> mySchedule = SchedulePtr(new FeeSchedule(scheduleName, cashflowLVBs));
		return mySchedule;
	}

    /* @brief Create a Bespoke Schedule object based on the label value blocks of schedule properties and schedule cashflows 
	*  @param [in]		feeName				        Fee leg name
	*  @param [in]		feeProperties			    Fee properties label value block
	*  @param [in]		cashflowLVBs				Fee cashflow label value blocks
    *  @Return     a pointer to the schedule object
    */
	LegPtr createFeeLeg(const std::string& feeName, const LabelValueBlock& feeProperties, const std::vector<LabelValueBlock>& cashflowLVBs)
	{
        auto feeSchedule = createFeeSchedule(feeName, cashflowLVBs);

        auto feeLeg = createLegByLVB(feeProperties, feeName, feeSchedule);

		return feeLeg;
	}

    /* @brief Create a LegStaticData pointer
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a pointer to the LegS object
    */
    std::map<std::string, LabelValueBlock> createLegInputParametersFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex)
    {
        std::map<std::string, LabelValueBlock> legProperties;
        const auto schemaNames = freeObject.keyNames();
        for(unsigned int i =0; i < schemaNames.size(); i++)
		{
            //Get Leg Static Data
            auto schemaName = schemaNames[i];
            std::string index = getIndexFromSchemaName(schemaName, schemaNameHasIndex);
            std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, schemaNameHasIndex);
            if(schemaNameWithoutIndex == toString(LEG))
		    {
			    auto keys = freeObject.getValuesCopy(schemaName,0);
			    auto values = freeObject.getValuesCopy(schemaName,1);
                LabelValueBlock legLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
	            legProperties[index] = legLVB;
            }    
        }
        return legProperties;
	}	

    /* @brief Create a LegGenerator
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a pointer to the LegGenerator
    */
    std::map<std::string, LegGenerator> createLegGeneratorFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex)
    {
        std::map<std::string, LegGenerator> legGemerators;
        const auto schemaNames = freeObject.keyNames();
        for(unsigned int i =0; i < schemaNames.size(); i++)
		{
            //Get Leg Static Data
            auto schemaName = schemaNames[i];
            std::string index = getIndexFromSchemaName(schemaName, schemaNameHasIndex);
            std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, schemaNameHasIndex);
            if(schemaNameWithoutIndex == toString(LEG_GENERATOR))
		    {
			    auto keys = freeObject.getValuesCopy(schemaName,0);
			    auto values = freeObject.getValuesCopy(schemaName,1);
                LabelValueBlock legLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
	            LegGenerator legGen(legLVB);
                legGemerators[index] = legGen;
            }    
        }
        return legGemerators;
	}	

    /* @brief Create a Schedule map
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a map of schedule object
    */
    std::map<std::string, SchedulePtr> createSchedulesFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex)
    {
        std::map<std::string, SchedulePtr> schedules;
        const auto schemaNames = freeObject.keyNames();
        for(unsigned int i =0; i < schemaNames.size(); i++)
		{
            auto schemaName = schemaNames[i];
            std::string index = getIndexFromSchemaName(schemaName, schemaNameHasIndex);
            std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, schemaNameHasIndex);
		    if(schemaNameWithoutIndex == toString(SCHEDULE))
		    {
			    auto keys = freeObject.getValuesCopy(schemaName,0);
                auto values = freeObject.getValuesCopy(schemaName,1);
                LabelValueBlock scheduleLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
		        auto schedule = createSchedule(objectName, scheduleLVB);
                schedules[index]= schedule;
            }    
            else if(schemaNameWithoutIndex == toString(FEE_SCHEDULE))
		    {
                auto keys = freeObject.viewSchema(freeObject.schemaIdx(schemaName)).getColumnNames();
                std::vector<std::vector<std::string>> inputMatrixStd;
                std::vector<std::string> row;
      	        for (size_t i=0; i<keys.size(); ++i)
                {
                    auto values = fromVariantToStdStringVector(freeObject.getValuesCopy(schemaName,i));
                    row.clear();
                    row.push_back(keys[i]);
                    row.insert(row.end(), values.begin(), values.end());
                    inputMatrixStd.push_back(row);                
                }

                LAStringMatrix inputMatrix =  fromStdMatrixToStringMatrix(inputMatrixStd);
                std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(inputMatrix);
                auto schedule = createFeeSchedule(objectName, cashflowLVBs);
                schedules[index]= schedule;
            }
        }

        if(hasBespokeSchedule(freeObject, schemaNameHasIndex))
        {
            auto bespokeMap = createBespokeSchedulesFromFreeObject(freeObject, objectName, schemaNameHasIndex);
            schedules.insert(bespokeMap.begin(), bespokeMap.end());
        }

        return schedules;
	}

    /* @brief Create a Schedule map
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a map of schedule object
    */
    std::map<std::string, SchedulePtr> createBespokeSchedulesFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex)
    {

        const auto schemaNames = freeObject.keyNames();

		auto bespokeSchedulePropertiesNames = getMatchingSchemaNames(schemaNames, { toString(BESPOKE_SCHEDULE_PROPERTIES) }, schemaNameHasIndex);
        std::map<std::string, LabelValueBlock> bespokeSchedulePropertiesMap;
        for(unsigned int i =0; i < bespokeSchedulePropertiesNames.size(); i++)
		{
            auto schemaName = bespokeSchedulePropertiesNames[i];
            std::string index = getIndexFromSchemaName(schemaName, schemaNameHasIndex);

            auto keys = freeObject.getValuesCopy(schemaName,0);
            auto values = freeObject.getValuesCopy(schemaName,1);
            auto schedulePropertiesLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
            bespokeSchedulePropertiesMap[index] = schedulePropertiesLVB;
        }

		auto bespokeScheduleCashflowNames = getMatchingSchemaNames(schemaNames, { toString(BESPOKE_SCHEDULE_WITH_PROPERTIES), toString(BESPOKE_SCHEDULE) }, schemaNameHasIndex);
		
		std::map<std::string, LAStringMatrix> cashflowMatrixMap;
        for(unsigned int i =0; i < bespokeScheduleCashflowNames.size(); i++)
		{
            auto schemaName = bespokeScheduleCashflowNames[i];
            auto keys = freeObject.viewSchema(freeObject.schemaIdx(schemaName)).getColumnNames();
            std::vector<std::vector<std::string>> inputMatrixStd;
            std::vector<std::string> row;
      	    for (size_t i=0; i<keys.size(); ++i)
            {
                auto values = fromVariantToStdStringVector(freeObject.getValuesCopy(schemaName,i));
                row.clear();
                row.push_back(keys[i]);
                row.insert(row.end(), values.begin(), values.end());
                inputMatrixStd.push_back(row);                
            }
            auto cashflowMatrix =  fromStdMatrixToStringMatrix(inputMatrixStd);
            cashflowMatrixMap[schemaName] = cashflowMatrix;
        }

		size_t schedulePropertySize = bespokeSchedulePropertiesMap.size();

		// If bespokeScheduleProperties are there, need to match the number
		if (schedulePropertySize > 0 && schedulePropertySize != cashflowMatrixMap.size())
        {
	        throw LACoreInvalidData("#Error: bespokeScheduleProperties and cashflowMatrix need to have the same size",__FILE__,__LINE__);
        }
        
        std::map<std::string, SchedulePtr> schedules;

		for (auto iter = cashflowMatrixMap.begin(); iter != cashflowMatrixMap.end(); iter++)
		{
			auto cfMatrix = iter->second;
			auto schemaName = iter->first;
			std::string index = getIndexFromSchemaName(schemaName, schemaNameHasIndex);
			const std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, schemaNameHasIndex);
			const LabelValueBlock scheduleProperties = bespokeSchedulePropertiesMap[index];

			auto cashflowLVBs = etrading::buildMultiLabelValueBlock(cfMatrix);

			SchedulePtr schedule;
			switch (toBespokeScheduleTypeEnum(schemaNameWithoutIndex))
			{
			case BESPOKE_SCHEDULE_WITH_PROPERTIES:
				schedule = createScheduleBespoke(objectName, scheduleProperties, cashflowLVBs, BESPOKE_SCHEDULE_WITH_PROPERTIES);
				break;
			case BESPOKE_SCHEDULE:
				schedule = createScheduleBespoke(objectName, scheduleProperties, cashflowLVBs, BESPOKE_SCHEDULE);
				break;
			default:
				throw LACoreInvalidData("#Error: bespokeScheduleType can only be either 'BESPOKE_SCHEDULE_WITH_PROPERTIES' or 'BESPOKE_SCHEDULE'", __FILE__, __LINE__);
				break;
			}
		
			schedules[index]=schedule;
        }
        
        return schedules;
     
     }	

    std::string getIndexFromSchemaName(const std::string& schemaName, bool schemaNameHasIndex)
    {
        std::string index = "NA";
        if (schemaNameHasIndex)
        {
            std::string::size_type idxInString = schemaName.find_last_of("_");
            if( idxInString != std::string::npos )
            {
                index = schemaName.substr( idxInString, schemaName.length() );
            }
        }
        return index;
    }

    std::string getSchemaNameWithoutIndex(const std::string& schemaName, bool schemaNameHasIndex)
    {
        std::string name = schemaName;
        if (schemaNameHasIndex)
        {
            dropLast(name , "_");
        }
        return name;
    }

	std::vector<std::string> getMatchingSchemaNames(const std::vector<std::string>& schemaNames, const std::unordered_set<std::string> searchNames, bool schemaNameHasIndex)
	{
		std::vector<std::string> names;
		for (unsigned int i = 0; i < schemaNames.size(); i++)
		{
			auto schemaName = schemaNames[i];
			std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, schemaNameHasIndex);
			if (searchNames.find(schemaNameWithoutIndex) != searchNames.end())
			{
				names.push_back(schemaName);
			}
		}
		return names;
	}
	
	bool hasBespokeSchedule(const FreeObject& freeObject, bool schemaNameHasIndex)
    {
        const auto schemaNames = freeObject.keyNames();
		auto bespokeCashflowNames = getMatchingSchemaNames(schemaNames, { toString(BESPOKE_SCHEDULE_WITH_PROPERTIES), toString(BESPOKE_SCHEDULE) }, schemaNameHasIndex);
        bool withBespoke =  (bespokeCashflowNames.size() > 0);
        return withBespoke;
    }

    void validateSwapStaticDataObject(const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys)
    {
        std::vector<std::string> swapProperties = isXccySwap ? CrossCurrencySwap::swapPropertiesKeys() : Swap::swapPropertiesKeys();
        validateKeysForLVB(swapProperties, swapPropertiesLVB.getKeys(), validateKeys);
    }

    /* @brief			Function to check if a swap input LVB is equal to an existing swap LVB registered on the cache
    * @param [in]		swapName			Existing Swap Name
    * @param [in]		leg1ScheduleType	Leg1 Schedule Type: FIXED, FLOAT, FEE
    * @param [in]		leg1LVB		        Leg1 Label Value Block
    * @param [in]		leg2ScheduleType	Leg2 Schedule Type: FIXED, FLOAT, FEE
    * @param [in]		leg2LVB		        Leg2 Label Value Block
    * @output			Returns swap shared pointer if the swap exists already and it's inputs are unchanged
    */
    std::shared_ptr<Swap> useExistingSwapIfGeneratorInputsUnchanged( const std::string& swapName, const ScheduleTypeEnum& leg1Type, const LabelValueBlock& leg1LVB, const ScheduleTypeEnum& leg2Type, const LabelValueBlock& leg2LVB )
    {
        // Return nullptr if the swap does not exist
        // ---------------------------
        auto existingSwap = etrading::Environment::defaultEnv().accessObject< etrading::Swap >(  swapName );
        if ( !existingSwap )
        {
            return std::shared_ptr<Swap>();
        }

        // Swap Leg Definitions
        auto leg1 = existingSwap->getLeg( 0 );
        auto leg2 = existingSwap->getLeg( 1 );

        const bool isLeg1Pay = leg1->getSchedule()->getPayerReceiver() == etrading::PAY_PAYRECEIVE_ENUM ? true : false;
        const double leg1IndicatorFunction = isLeg1Pay ? -1.0 : 1.0; // -1 for Pay and 1 for Receive
        
        // Check Swap Generator Inputs
        // ---------------------------

        // PayRecLeg1
        if ( leg1->getSchedule()->getPayerReceiver() != toPayReceiveEnum( leg1LVB.getOptionalValueAsLAString( etrading::IRS_KEY::PAY_RECEIVE ).getCString() ) )
        {
            return std::shared_ptr<Swap>();
        }

        // Notional
        if ( leg1->getSchedule()->getNotional() != leg1LVB.getOptionalValueAsDouble( etrading::IRS_KEY::NOTIONAL ) * leg1IndicatorFunction )
        {
            return std::shared_ptr<Swap>();
        }

        // EffectiveDate
        if ( leg1->getSchedule()->getEffectiveDate() != leg1LVB.getOptionalValueAsDate( etrading::IRS_KEY::EFFECTIVE_DATE ) )
        {
            return std::shared_ptr<Swap>();
        }

        // MaturityDate - Note Maturity Date may be quoted as a Tenor String!
        if ( leg1->getSchedule()->getMaturityDate() != leg1LVB.getOptionalDateOrTenorAsDate( etrading::IRS_KEY::MATURITY_DATE, leg1->getSchedule()->getEffectiveDate() ) )
        {
            return std::shared_ptr<Swap>();
        }
        
        // Leg 1 Checks
        // ------------
        if ( isFixedLeg(leg1Type) )
        {
            // RateOrSpreadLeg1
            if ( leg1->getSchedule()->getFixedRate() != leg1LVB.getOptionalValueAsDouble( etrading::IRS_KEY::FIXED_RATE ) )
            {
                return std::shared_ptr<Swap>();
            }
        }
        else if (isFloatLeg(leg1Type) || leg1Type == etrading::CMS_SCHEDULE_TYPE )
        {
            // RateOrSpreadLeg1
            if ( leg1->getSchedule()->getSpread() != leg1LVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_SPREAD ) )
            {
                return std::shared_ptr<Swap>();
            }
            
            // FirstFixingLeg1 - Note we must handle NaN case
            if ( leg1->getStaticData()->getFirstFixing() != leg1LVB.getOptionalValueAsDouble( IRS_KEY::FIRSTFIXING )
                && boost::math::isnan( leg1->getStaticData()->getFirstFixing() ) != boost::math::isnan( leg1LVB.getOptionalValueAsDouble( IRS_KEY::FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() ) ) )
            {
                return std::shared_ptr<Swap>();
            }

            // LastFixingLeg1 - Note we must handle NaN case
            if ( leg1->getStaticData()->getLastFixing() != leg1LVB.getOptionalValueAsDouble( IRS_KEY::LASTFIXING )
                && boost::math::isnan( leg1->getStaticData()->getLastFixing() ) != boost::math::isnan( leg1LVB.getOptionalValueAsDouble( IRS_KEY::LASTFIXING, std::numeric_limits<double>::quiet_NaN() ) ) )
            {
                return std::shared_ptr<Swap>();
            }
            
            // IsFwdInter1
            if ( leg1->getStaticData()->getFwdInter() != toBooleanEnum( leg1LVB.getOptionalValueAsLAString( IRS_KEY::IS_FWD_INTER ).getCString() ) )
            {
                return std::shared_ptr<Swap>();
            }

			// PaymentTrigger
			if ( leg1->getPaymentTrigger() != toPaymentTriggerEnum( leg1LVB.getOptionalValueAsLAString( IRS_KEY::PAYMENT_TRIGGER, "PAYALWAYS" ).getCString() ) )
			{
				return std::shared_ptr<Swap>();
			}
        }
		else if ( leg1Type == etrading::PREMIUM_SCHEDULE_TYPE )
        {
            // CDS Spread
			std::shared_ptr<Schedule> schedule = leg1->getSchedule();
			std::shared_ptr<PremiumSchedule> premiumSchedule = std::dynamic_pointer_cast<PremiumSchedule>(schedule);
			if ( premiumSchedule == nullptr )
			{
				throw ETradingException( ( boost::format( "#Error: Incorrect Schedule type for CDS Premium leg '%s'." ) % leg1->getLegName() ).str()  );
			}
            if ( premiumSchedule->getCDSSpread() != leg1LVB.getOptionalValueAsDouble( etrading::CDS_KEY::CDS_SPREAD ) )
            {
                return std::shared_ptr<Swap>();
            }
        }

        // Leg 2 Checks
        // ------------
        if ( isFixedLeg(leg2Type) )
        {
            // RateOrSpreadLeg2
            if ( leg2->getSchedule()->getFixedRate() != leg2LVB.getOptionalValueAsDouble( etrading::IRS_KEY::FIXED_RATE ) )
            {
                return std::shared_ptr<Swap>();
            }
        }
        else if (isFloatLeg(leg2Type) || leg2Type == etrading::CMS_SCHEDULE_TYPE )
        {
            // RateOrSpreadLeg2
            if ( leg2->getSchedule()->getSpread() != leg2LVB.getOptionalValueAsDouble( etrading::IRS_KEY::FLOAT_SPREAD ) )
            {
                return std::shared_ptr<Swap>();
            }

            // FirstFixingLeg2 - Note we must handle NaN case
            if ( leg2->getStaticData()->getFirstFixing() != leg2LVB.getOptionalValueAsDouble( IRS_KEY::FIRSTFIXING ) 
                && boost::math::isnan( leg2->getStaticData()->getFirstFixing() ) != boost::math::isnan( leg2LVB.getOptionalValueAsDouble( IRS_KEY::FIRSTFIXING, std::numeric_limits<double>::quiet_NaN() ) ) )
            {
                return std::shared_ptr<Swap>();
            }

            // LastFixingLeg2 - Note we must handle NaN case
            if ( leg2->getStaticData()->getLastFixing() != leg2LVB.getOptionalValueAsDouble( IRS_KEY::LASTFIXING )
                && boost::math::isnan( leg2->getStaticData()->getLastFixing() ) != boost::math::isnan( leg2LVB.getOptionalValueAsDouble( IRS_KEY::LASTFIXING, std::numeric_limits<double>::quiet_NaN() ) ) )
            {
                return std::shared_ptr<Swap>();
            }
            
            // IsFwdInter2
            if ( leg2->getStaticData()->getFwdInter() != toBooleanEnum( leg2LVB.getOptionalValueAsLAString( IRS_KEY::IS_FWD_INTER ).getCString() ) )
            {
                return std::shared_ptr<Swap>();
            }

			// PaymentTrigger
			if ( leg2->getPaymentTrigger() != toPaymentTriggerEnum( leg2LVB.getOptionalValueAsLAString( IRS_KEY::PAYMENT_TRIGGER, "PAYALWAYS" ).getCString() ) )
			{
				return std::shared_ptr<Swap>();
			}
        }
		else if ( leg2Type == etrading::PREMIUM_SCHEDULE_TYPE )
        {
            // CDS Spread
			std::shared_ptr<Schedule> schedule = leg2->getSchedule();
			std::shared_ptr<PremiumSchedule> premiumSchedule = std::dynamic_pointer_cast<PremiumSchedule>(schedule);
			if ( premiumSchedule == nullptr )
			{
				throw ETradingException( ( boost::format( "#Error: Incorrect Schedule type for CDS Premium leg '%s'." ) % leg2->getLegName() ).str()  );
			}
            if ( premiumSchedule->getCDSSpread() != leg2LVB.getOptionalValueAsDouble( etrading::CDS_KEY::CDS_SPREAD ) )
            {
                return std::shared_ptr<Swap>();
            }
        }

        // Return the existingSwap if all the Generator Results Match
        return existingSwap;
    }


	/* @brief Finds the leg of the specified type within a provided LegCollection.
	*         If the legName is provided, attempts to find that leg and verifies the legType
	*         If the legName is empty and more than leg of the specified type exists, the method throws an exception
	*
	* @param[in] legName	Optional, allowed to be an empty string
	* @param[in] legType	Mandatory, specifies the desired leg type
	* @param[in] legs		Mandatory, specifies a collection of legs to search for the required legType
	* @returns The validated leg name matching the specified legType
	*/
	LAString validateLegName( const LAString& legName, ScheduleTypeEnum legType, const LegCollection& legs )
	{
		LAString validatedLegName;

		if ( legName.size() > 0 )
		{
			// A leg name has been provided. Verify that it exists in the CDS

			auto leg = legs.findLegByName( legName );
			if ( leg == nullptr )
			{
				MLIB_THROW( "Leg name '" + legName + "' does not exist." );
			}
			if ( leg->getType() == legType )
			{
				// Found a matching leg
				validatedLegName = legName;
			}
			else
			{
				MLIB_THROW( "Leg name '" + legName + "' does not have the required leg type '" + toString( legType ).c_str() + "'." );
			}
        }
		else
		{
			// Leg name has not been provided. Search for a leg of the specified type
			int numMatches = 0;
			for ( size_t i = 0; i < legs.size(); ++i )
			{
				auto leg = legs.get( i );
				if ( leg->getType() == legType )
				{
					validatedLegName = leg->getLegName();
					numMatches ++;	
				}
			}

			if ( numMatches == 0 )
			{
				MLIB_THROW( "Credit Default Swap does not contain any legs of type '" + toString( legType ) + "'." );
			}
			if ( numMatches > 1 )
			{
				MLIB_THROW( "CreditDefaultSwap contains more than one leg of type '" + toString( legType ) + "'. Please specify the leg name to use." );
			}
		}

		return validatedLegName;
	}

	/* @brief Given an accrual frequency, returns the corresponding number of coupons per year.
	*
	* @param[in] accrualFrequency	Accrual frequency
	* @returns   the number of coupons per year. For example SEMI_ANNUAL_FREQUENCY returns 2.
	*/
	size_t convertFrequenyToCouponsPerYear( const FrequencyEnum accrualFrequency )
	{
		size_t couponsPerYear;
		switch ( accrualFrequency )
        {
            case ANNUAL_FREQUENCY:
                couponsPerYear = 1;
                return couponsPerYear;
                break;

            case SEMI_ANNUAL_FREQUENCY:
                couponsPerYear = 2;
                return couponsPerYear;
                break;

            case QUARTERLY_FREQUENCY:
                couponsPerYear = 4;
                return couponsPerYear;
                break;

            case MONTHLY_FREQUENCY:
                couponsPerYear = 12;
                return couponsPerYear;
                break;

            case WEEKLY_FREQUENCY:
                couponsPerYear = 52;
                return couponsPerYear;
                break;

            case DAILY_FREQUENCY:
                couponsPerYear = 365;
                return couponsPerYear;
                break;

            default:
		        throw LACoreInvalidData("#Error: Invalid accrual frequency. Only 'ANNUAL', 'SEMI-ANNUAL', 'QUARTERLY', 'MONTHLY', 'WEEKLY' or 'DAILY' supported.",__FILE__,__LINE__);
                break;
        }
	
	}

	// Check if a swap is a fixed float swap
	bool isFixedFloatSwap(const SwapPtr& swap)
	{
		auto type1 = swap->getLeg(0)->getType();
		auto type2 = swap->getLeg(1)->getType();

		return (isFixedLeg(type1) && isFloatLeg(type2)) || (isFloatLeg(type1) && isFixedLeg(type2));
	}

	LegPtr getFixedLeg(const SwapPtr& swap)
	{
		MLIB_REQUIRE(isFixedFloatSwap(swap), "The Swap must be a Fixed Float Swap.");

		return isFixedLeg(swap->getLeg(0)->getType()) ? swap->getLeg(0) : swap->getLeg(1);
	}

	LegPtr getFloatLeg(const SwapPtr& swap)
	{
		MLIB_REQUIRE(isFixedFloatSwap(swap), "The Swap must be a Fixed Float Swap.");

		return isFloatLeg(swap->getLeg(0)->getType()) ? swap->getLeg(0) : swap->getLeg(1);
	}


}