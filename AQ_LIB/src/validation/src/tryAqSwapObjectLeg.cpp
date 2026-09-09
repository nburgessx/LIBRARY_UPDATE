#include "tryAqSwapObjectLeg.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"
#include "SwapUtilities.h"
#include "ObjectUtilities.h"
#include "AQObjUtilities.h"
#include "CoreEnumerations.h"
#include "SettingsValidation.h"

#include <memory>

using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::Swap;
using etrading::Schedule;

namespace validation
{
 
	/* @brief			validation interface for the aqSwapObjectLegCreate method
	*  @param [in]		legObjectName	Leg object name
	*  @param [in]		legLVB			Leg label value block with legs
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			legObjectName
	*/
	std::string tryAqSwapObjectLegCreate(const std::string& legObjectName, const LabelValueBlock& legLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		const std::string inputLVB = "swapLVB";

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( legObjectName.c_str(), "", legObjectName, legLVB, validateKeys );

        AQLString legName = legLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE);
        etrading::validateSwapLegLVBKeys(legName, legLVB.getKeys(), validateKeys);

		auto myLeg = etrading::createLegByLVB(legLVB, legObjectName);
		etrading::registerToCache<etrading::Leg>( myLeg );

        std::string ret = legObjectName;
		AQ_RECORD_DECORATED_OUTPUTS( legObjectName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectCreate method
	*  @param [in]		legObjectName		    Fee Leg object name
	*  @param [in]		feeProperties			Fee properties label value block
	*  @param [in]		feeScheduleLVB			Fee schedule label value block matrix
	*  @param [in]		validateKeys	        True to validate the all keys provided are valid. Default to True
	*  @return			legObjectName
	*/
	std::string tryAqCreditObjectFeeLegCreate(const std::string& legObjectName, const LabelValueBlock& feeProperties, const AQLStringMatrix& feeScheduleLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		const std::string inputLVB = "swapLVB";

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( legObjectName.c_str(), "", legObjectName, feeProperties, feeScheduleLVB, validateKeys );

		AQLString legName = feeProperties.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE);
        std::vector<LabelValueBlock> cashflowLVBs = etrading::buildMultiLabelValueBlock(feeScheduleLVB);
        
        if (validateKeys)
        {
            auto expectedKeys = feeProperties.getKeys();
            auto cashflowKeys = cashflowLVBs.at(0).getKeys();
		    expectedKeys.insert(expectedKeys.end(), cashflowKeys.begin(), cashflowKeys.end());

            etrading::validateSwapLegLVBKeys(legName, expectedKeys, validateKeys);
        }

        auto feeLeg = etrading::createFeeLeg(legObjectName, feeProperties, cashflowLVBs);
		etrading::registerToCache<etrading::Leg>( feeLeg);

        std::string ret = legObjectName;

		AQ_RECORD_DECORATED_OUTPUTS( legObjectName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

    
	/* @brief			validation interface for the aqSwapObjectLegCreateFromSchedule method
	*  @param [in]		legObjectName	Leg object name
	*  @param [in]		scheduleName	Schedule name
	*  @param [in]		legLVB			Leg label value block 
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			swapName
	*/
	std::string tryAqSwapObjectLegCreateFromSchedule(const std::string& legObjectName, const std::string& scheduleName, const LabelValueBlock& legLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START

		const std::string inputLVB = "swapLVB";

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( legObjectName.c_str(), "", legObjectName, scheduleName, legLVB, validateKeys );

		AQLString legName = legLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE);
		etrading::validateSwapLegLVBKeys(legName, legLVB.getKeys(), validateKeys, false);

		std::shared_ptr<Schedule> schedule = etrading::getSchedule(scheduleName);

        auto myLeg = etrading::createLegByLVB(legLVB, legObjectName, schedule);

		etrading::registerToCache<etrading::Leg>( myLeg);

        std::string ret = legObjectName;

		AQ_RECORD_DECORATED_OUTPUTS( legObjectName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectLegPV method
	*  @param [in]		legObjectName		Leg object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			Leg PV
	*/
	double tryAqSwapObjectLegPV(const std::string& legObjectName, const LabelValueBlock& valuationSettingsLVB, const std::string& fixingTableName)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( legObjectName.c_str(), "", legObjectName, valuationSettingsLVB, fixingTableName );

		auto leg = etrading::getLeg(legObjectName);
  
		etrading::DataProvider dataProvider(etrading::ValuationSettings(valuationSettingsLVB, etrading::fromStringToLVB(fixingTableName), leg->getLegName()));

		double ret = leg->pv( dataProvider, true);

		AQ_RECORD_DECORATED_OUTPUTS( legObjectName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectLegDisplay method, which display the INPUT parameters of the cached swap
	*  @param [in]		legObjectName		Leg object name
	*  @return			Leg display of the input parameters
	*/
	AQLStringMatrix tryAqSwapObjectLegDisplay(const std::string& legObjectName)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( legObjectName.c_str(), "", legObjectName );

		auto leg = etrading::getLeg(legObjectName);
		auto ret = leg->getInputParameters().toAQLStringMatrix();
		
		AQ_RECORD_DECORATED_OUTPUTS( legObjectName.c_str(), "", ret );

		return ret;

		VALID_EXCEPTION_END
	}


	/* @brief			validation interface for the aqSwapObjectLegDisplayCashflows method 
	*  @param [in]		legObjectName		Leg object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Leg display of leg output
	*/
	AnyTypeMatrix tryAqSwapObjectLegDisplayCashflows(const std::string& legObjectName, const LabelValueBlock& valuationSettingsLVB, const std::string& fixingTableName, bool showColumnHeaders, const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( legObjectName, valuationSettingsLVB, fixingTableName, showColumnHeaders, columnList );

		auto leg = etrading::getLeg(legObjectName);

		etrading::DataProvider dataProvider(etrading::ValuationSettings(valuationSettingsLVB, etrading::fromStringToLVB(fixingTableName), leg->getLegName()));

        auto result = leg->view(dataProvider, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

}

