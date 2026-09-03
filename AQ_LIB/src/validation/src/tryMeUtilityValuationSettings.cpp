/*
 * @brief			validation interface for the meUtilityValuationSettings
 * @Created:		5th February 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMeUtilityValuationSettings.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"
#include "ValuationSettings.h"
//#include "Variant.h"

#include <memory>

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{
    
	/* @brief Method to convert raw input into a valuation settings data block
	*
	* @param[in]	rawInput			Valuation Settings Data
	* @returns	The modified input used for pricing
	*/
	StandardStringMatrix tryMeUtilityValuationSettingsDisplay( const StandardStringMatrix & rawInput )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        //RECORD_INPUTS( rawInput );

		etrading::ValuationSettings valuationSettings( rawInput );
		StandardStringMatrix result = valuationSettings.getValuationSettingsLVB().toStandardStringMatrix();
		
		// Record Outputs AND Return the Result for logs, tests and playback
        // RECORD_OUTPUTS_AND_RETURN_RESULT( result );
		return result;

		VALID_EXCEPTION_END
	}

}
