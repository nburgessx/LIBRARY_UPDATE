#include "tryAqSwapObjectCreation.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"

#include "SwapUtilities.h"
#include "ObjectUtilities.h"
#include "AQObjUtilities.h"
#include <memory>
#include "CoreEnumerations.h"
#include "CrossCurrencySwap.h"
#include "FeeLeg.h"
#include "SwapGenerator.h"
#include "SwapValidation.h"
#include "RecordMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::Swap;
using etrading::CrossCurrencySwap;
using etrading::Schedule;

namespace validation
{

    std::vector<std::string> tryAqSwapObjectLVBKeys()
    {
        std::vector<std::string> keys;
        keys.push_back("Swap Properties LVB Keys:");
        auto swapPropertiesKeys = CrossCurrencySwap::swapPropertiesKeys();
        keys.insert( keys.end(), swapPropertiesKeys.begin(), swapPropertiesKeys.end() );

        keys.push_back("Swap Leg LVB Keys:");
        auto keys1 = Swap::swapLegCommonLVBKeys();
        keys.insert( keys.end(), keys1.begin(), keys1.end() );

        keys.push_back("Fee Leg LVB Keys:");
        auto keys2 = etrading::FeeLeg::lvbKeys();
        keys.insert( keys.end(), keys2.begin(), keys2.end() );

        return keys;
    }
 

	/* @brief			validation interface for the aqSwapObjectCreateFromLegs method
	*  @param [in]		swapName		Swap name to be cached
	*  @param [in]		legObjectNames	Cached leg object names
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectCreateFromLegs(const std::string& swapName, const std::vector<std::string>& legObjectNames, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		AQ_RECORD_DECORATED_INPUTS(swapName, std::string(), swapName, legObjectNames, swapPropertiesLVB, isXccySwap, validateKeys)
	
		AQ_REQUIRE(legObjectNames.size() >= 2, "The swap should have at least two legs.")

		auto leg1 = etrading::getLeg(legObjectNames[0]);
		auto leg2 = etrading::getLeg(legObjectNames[1]);

		etrading::validateSwapStaticDataObject(swapPropertiesLVB, isXccySwap, validateKeys);
		etrading::validateSwapCurrency(isXccySwap, leg1->getStaticData()->getCurrency(), leg2->getStaticData()->getCurrency());

		auto mySwap = etrading::createSwapFromLegs(swapName, leg1, leg2, swapPropertiesLVB);

		for (size_t i = 2; i < legObjectNames.size(); ++i)
		{
			auto leg = etrading::getLeg(legObjectNames[i]);
			mySwap->addToLegCollection(leg);
		}

		etrading::registerToCache<etrading::Swap>(mySwap);

		std::string ret = swapName;

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		AQ_RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapName, std::string(), ret)

		VALID_EXCEPTION_END

	}



    /* @brief			validation interface for the aqSwapObjectCreateFromLegLVBs method
	*  @param [in]		swapName		Swap name
	*  @param [in]		leg1LVB			Leg1 label value block 
	*  @param [in]		leg2LVB			Leg2 label value block 
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectCreateFromLegLVBs(const std::string& swapName, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapName.c_str(), "", swapName, leg1LVB, leg2LVB, swapPropertiesLVB, isXccySwap, validateKeys );
    	
        const std::string inputLVB = "swapLVB";
        AQLString leg1Name = leg1LVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
		AQLString leg2Name = leg2LVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
        
        etrading::validateSwapLegLVBKeys(leg1Name, leg1LVB.getKeys(), validateKeys);
		etrading::validateSwapLegLVBKeys(leg2Name, leg2LVB.getKeys(), validateKeys);

        etrading::validateSwapStaticDataObject(swapPropertiesLVB, isXccySwap, validateKeys);

        etrading::validateSwapCurrency(isXccySwap, leg1LVB, leg2LVB);

        std::shared_ptr<Swap> mySwap = etrading::createSwap(swapName, leg1LVB, leg2LVB, swapPropertiesLVB);

		etrading::registerToCache<etrading::Swap>( mySwap );

        std::string ret = swapName;

		AQ_RECORD_DECORATED_OUTPUTS( swapName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectCreate method
	*  @param [in]		swapName		Swap name
	*  @param [in]		swapLVB			Swap label value block with legs
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectCreate(const std::string& swapName, const AQLStringMatrix& swapLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys)
	{
		VALID_EXCEPTION_START
        
        AQ_REQUIRE( !swapLVB.empty(), "The swap label Value Block is empty or contains errors" )

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapName.c_str(), "", swapName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );

		std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapLVB);

        // Access Violation Guard
        AQ_REQUIRE( legsLVB.size() >= 2, "Invalid Swap Input: Swap trades require at least 2 trade legs." )

 		if (validateKeys)
		{
    		const std::string inputLVB = "swapLVB";
			LabelValueBlock legLVB;
			AQLString legName = "";
			for (size_t i = 0; i < legsLVB.size(); ++i)
			{
				legLVB = legsLVB[i];
				legName = legLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
				etrading::validateSwapLegLVBKeys(legName, legLVB.getKeys(), validateKeys);
			}

            etrading::validateSwapStaticDataObject(swapPropertiesLVB, isXccySwap, validateKeys);
		}

        // Access Violation Guard is Above
        etrading::validateSwapCurrency( isXccySwap, legsLVB[0], legsLVB[1] );


		auto mySwap = etrading::createSwap(swapName, legsLVB, swapPropertiesLVB);
		etrading::registerToCache<etrading::Swap>( mySwap);

        std::string ret = swapName;

		AQ_RECORD_DECORATED_OUTPUTS( swapName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectCreate method
	*  @param [in]		swapName		Swap name
	*  @param [in]		swapLVB			Swap label value block with legs
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectCreateBackToBack(const std::string& fromSwapName, const std::string& toSwapName)
	{
		VALID_EXCEPTION_START

		const std::string inputLVB = "swapLVB";

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( toSwapName.c_str(), "", fromSwapName, toSwapName );

		auto originalSwap = etrading::getSwap(fromSwapName);

		std::shared_ptr<Swap> mySwap = etrading::createBackToBackSwap(originalSwap, toSwapName);

        etrading::registerToCache<etrading::Swap>( mySwap);

        std::string ret = toSwapName;

		AQ_RECORD_DECORATED_OUTPUTS( toSwapName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectCreateFromLegs method
	*  @param [in]		swapName		Swap name
	*  @param [in]		schedule1Name	Schedule1 name
	*  @param [in]		schedule2Name	Schedule2 name
	*  @param [in]		leg1LVB			Leg1 label value block 
	*  @param [in]		leg2LVB			Leg2 label value block 
	*  @param [in]		swapPropertiesLVB	Swap level properties
	*  @param [in]		isXccySwap	    True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectCreateFromSchedule(const std::string& swapName, const std::string& schedule1Name, const std::string& schedule2Name, 
                                               const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys)
	{
		VALID_EXCEPTION_START


		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapName.c_str(), "", swapName, schedule1Name, schedule2Name, leg1LVB, leg2LVB, swapPropertiesLVB, isXccySwap, validateKeys );

		std::shared_ptr<Schedule> schedule1 = etrading::getSchedule(schedule1Name);
		std::shared_ptr<Schedule> schedule2 = etrading::getSchedule(schedule2Name);

		const std::string inputLVB = "swapLVB";
   		AQLString leg1Name = leg1LVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
		AQLString leg2Name = leg2LVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);

		etrading::validateSwapLegLVBKeys(leg1Name, leg1LVB.getKeys(), validateKeys, false);
		etrading::validateSwapLegLVBKeys(leg2Name, leg2LVB.getKeys(), validateKeys, false);

        etrading::validateSwapStaticDataObject(swapPropertiesLVB, isXccySwap, validateKeys);

        etrading::validateSwapCurrency(isXccySwap, leg1LVB, leg2LVB);

        std::shared_ptr<Swap> mySwap = etrading::createSwap(swapName, leg1LVB, leg2LVB, swapPropertiesLVB, schedule1, schedule2);

		etrading::registerToCache<etrading::Swap>( mySwap);

        std::string ret = swapName;

		AQ_RECORD_DECORATED_OUTPUTS( swapName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}


	/* @brief			validation interface for the aqSwapObjectDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		swapName		Swap name
	*  @return			Swap display of the input parameters
	*/
	std::vector<AnyTypeMatrix> tryAqSwapObjectDisplay(const std::string& swapName)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapName.c_str(), "", swapName );

		auto swap = etrading::getSwap(swapName);

        auto headerBodyPairs = swap->viewInputParameters();

        auto ret =  etrading::mergeHeaderAndBodyPairs(headerBodyPairs);
		
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqSwapObjectDisplay_outputs", swapName.c_str()));
			for (size_t i = 0; i < ret.size(); ++i)
			{
				file.write("output", ret[i]);
			}
		}

		return ret;

		VALID_EXCEPTION_END
	}

 
   	/* @brief			validation interface for the aqSwapObjectAddLeg method
	*  @param [in]		swapName		The cached swap name
	*  @param [in]		legObjectName   The cached leg name
	*  @return			Swap name
	*/
	std::string tryAqSwapObjectAddLeg(const std::string& swapName, const std::string& legObjectName)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapName.c_str(), "", swapName, legObjectName );

		auto swap = etrading::getSwap(swapName);
		auto leg = etrading::getLeg(legObjectName);
        swap->addToLegCollection(leg);

   		etrading::registerToCache<etrading::Swap>(swap);
        std::string ret = swapName;

        AQ_RECORD_DECORATED_OUTPUTS( swapName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}


   	/* @brief			validation interface for the aqSwapObjectAddFee method, same as aqSwapObjectAddLeg() except for the extra Fee leg check
	*  @param [in]		swapName		The cached swap name
	*  @param [in]		feeName         The cached fee leg name
	*  @return			Swap name
	*/
	std::string tryAqSwapObjectAddFee(const std::string& swapName, const std::string& feeName)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapName.c_str(), "", swapName, feeName );

		auto swap = etrading::getSwap(swapName);
		auto leg = etrading::getLeg(feeName);

		AQ_THROW_IF( leg->getType() != etrading::FEE_SCHEDULE_TYPE, "The Leg is not a Fee" );
        swap->addToLegCollection(leg);

   		etrading::registerToCache<etrading::Swap>(swap);
        std::string ret = swapName;

        AQ_RECORD_DECORATED_OUTPUTS( swapName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapGeneratorCreate method
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @param [in]		swapGeneratorLVB	    Swap generator label value block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapGeneratorName
	*/
	std::string tryAqSwapGeneratorCreate(const std::string& swapGeneratorName, const AQLStringMatrix& swapGeneratorLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START
        
        AQ_REQUIRE( !swapGeneratorLVB.empty(), "The swap generator label Value Block is empty or contains errors" )

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapGeneratorName.c_str(), "", swapGeneratorName, swapGeneratorLVB, validateKeys );

		std::vector<LabelValueBlock> legsLVB = etrading::buildMultiLabelValueBlock(swapGeneratorLVB);

        AQ_THROW_IF( legsLVB.size() != 2, "SwapGeneratorLVB must have a key column and two value columns" );

        auto legGen1 = legsLVB.at(0);
        auto legGen2 = legsLVB.at(1);

        // Both legs have the same key set
        etrading::validateKeysForLVB(etrading::SwapGenerator::lvbKeys(), legGen1.getKeys(), validateKeys);

        etrading::SwapGenerator swapGen(swapGeneratorName, legGen1, legGen2);
            
        etrading::moveToCache< etrading::SwapGenerator >( std::move( swapGen ) );

        std::string ret = swapGeneratorName;

		AQ_RECORD_DECORATED_OUTPUTS( swapGeneratorName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapGeneratorDisplay method, which display the INPUT parameters of the cached swap generator
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @return			Swap generator display of the input parameters
	*/
	AQLStringMatrix tryAqSwapGeneratorDisplay(const std::string& swapGeneratorName)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( swapGeneratorName.c_str(), "", swapGeneratorName );

		auto swapGen = etrading::getSwapGenerator(swapGeneratorName);
		auto ret = swapGen->viewInputParameters();
		
		AQ_RECORD_DECORATED_OUTPUTS( swapGeneratorName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectCreateFromGenerator method
	*  @param [in]		swapName		        Swap name
	*  @param [in]		swapGeneratorName		Swap generator name
	*  @param [in]		expressionLVB		    Expression label value block to customize the swap
	*  @param [in]		swapPropertiesLVB	    Swap level properties
	*  @param [in]		isXccySwap	            True to enforce it is a cross currency swap
	*  @param [in]		validateKeys	        True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectCreateFromGenerator(const std::string& swapName, const std::string& swapGeneratorName, const LabelValueBlock& expressionLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys)
	{
		VALID_EXCEPTION_START
        
        AQ_REQUIRE( expressionLVB.size() != 0, "The swap expression label Value Block is empty or contains errors" )

        // Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
        AQ_RECORD_DECORATED_INPUTS(swapName, std::string(), swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap, validateKeys)
        
        // Validate the swap inputs
        etrading::validateKeysForLVB(etrading::SwapGenerator::expressionLvbKeys(), expressionLVB.getKeys(), validateKeys);
        etrading::validateSwapStaticDataObject(swapPropertiesLVB, isXccySwap, validateKeys);

        // Delegate to a helper function to create the swap
		auto mySwap = etrading::createSwapFromGenerator( swapName, swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccySwap );
        etrading::registerToCache<etrading::Swap>( mySwap);
        std::string result = swapName;
        
        // Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
        AQ_RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(swapName, std::string(), result)
		
		VALID_EXCEPTION_END
	}

}
