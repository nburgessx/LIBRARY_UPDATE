#include "tryAqSwapLeg.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

#include "CommonConstants.h"
#include "SwapValidation.h"
#include "SwapUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{

	/* @brief			return a set of expected keys for swap pv label value block
	*  @return			expected keys
	*/
	std::vector<std::string> tryAqSwapsLegLVBKeys(const AQLString& legName) 
	{
		return etrading::getSwapLegLVBKeys(legName);
	}

	/* @brief			validation interface for the aqSwapLegDisplay method
	*  @param [in]		legLVB			A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @param [in] 	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Matrix of leg schedule and cashflows
	*/
	AnyTypeMatrix tryAqSwapLegDisplay(const LabelValueBlock& legLVB, bool validateKeys, bool showColumnHeaders, const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		const std::string inputLVB = "LegLVB";
        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( legLVB, validateKeys, showColumnHeaders, columnList );

		AQLString legName = legLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
		etrading::validateKeysForLVB(tryAqSwapsLegLVBKeys(legName), legLVB.getKeys(), validateKeys);

		etrading::LegPtr leg = etrading::createLegByLVB(legLVB);

        const AQLString curveCollection = legLVB.getCompulsoryValueAsAQLString(etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB);
        LabelValueBlock valuationSettingsLVB( etrading::VALUATION_SETTING_KEYS::CURVE_COLLECTION, curveCollection.c_str() );

		etrading::DataProvider dataProvider(etrading::ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

		auto result = leg->view(dataProvider, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapLegPV method
	*  @param [in]		legLVB		A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swap Leg PV
	*/
	double tryAqSwapLegPV(const LabelValueBlock& legLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START
		
		const std::string inputLVB = "LegLVB";
		AQLString curveCollection = legLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB);

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeProductSwapLegPV_inputs", curveCollection));
			file.write("generatorFunction", "tryMeProductSwapLegPV");
			file.write("legLVB", legLVB);
			file.write("validateKeys", validateKeys);
		}

		AQLString legName = legLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
		etrading::validateKeysForLVB(tryAqSwapsLegLVBKeys(legName), legLVB.getKeys(), validateKeys);

		etrading::LegPtr leg = etrading::createLegByLVB(legLVB);

		LabelValueBlock valuationSettingsLVB( etrading::VALUATION_SETTING_KEYS::CURVE_COLLECTION, curveCollection.c_str() );

		etrading::DataProvider dataProvider(etrading::ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

		double ret = leg->pv( dataProvider);

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeProductSwapLegPV_outputs", curveCollection));
			file.write("output", ret);
		}

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapLegAnnuity method
	*  @param [in]		legLVB		A label value block defining the swap leg
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			Swap Leg Annuity
	*/
	double tryAqSwapLegAnnuity(const LabelValueBlock& legLVB, bool validateKeys)
	{
		VALID_EXCEPTION_START
		
		const std::string inputLVB = "LegLVB";
		AQLString curveCollection = legLVB.getCompulsoryValueAsAQLString( etrading::MARKET_KEY::CURVE_COLLECTION, inputLVB);

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeProductSwapLegAnnuity_inputs", curveCollection));
			file.write("generatorFunction", "tryMeProductSwapLegAnnuity");
			file.write("legLVB", legLVB);
			file.write("validateKeys", validateKeys);
		}

		AQLString legName = legLVB.getCompulsoryValueAsAQLString( etrading::IRS_KEY::LEG_TYPE, inputLVB);
		etrading::validateKeysForLVB(tryAqSwapsLegLVBKeys(legName), legLVB.getKeys(), validateKeys);

		etrading::LegPtr leg = etrading::createLegByLVB(legLVB);
		
		LabelValueBlock valuationSettingsLVB( etrading::VALUATION_SETTING_KEYS::CURVE_COLLECTION, curveCollection.c_str() );

		etrading::DataProvider dataProvider(etrading::ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

		double ret = leg->annuityWithNotional(dataProvider);

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeProductSwapLegAnnuity_outputs", curveCollection));
			file.write("output", ret);
		}

		return ret;

		VALID_EXCEPTION_END
	}

}








