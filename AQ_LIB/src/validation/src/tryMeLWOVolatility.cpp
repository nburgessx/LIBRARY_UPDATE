/*
 * @brief			validation interface for the Bond analytics 
 * @Created:		18th January 2017
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeLWOVolatility.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ObjectUtilities.h"
#include "LWOUtilities.h"
#include "RecordMacros.h"
#include "OptionFactory.h"
#include "SabrModel.h"
#include "SabrMarketData.h"
#include <omp.h>

using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::Volatility;


namespace validation_api
{

	/* @brief			validation interface for the meLWOVolatilityCreate method
	*  @param [in]		objectName	    Volatility object name
	*  @param [in]		volLVB			Volatility Label Value Block
	*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
	*  @return			CapFloor Handle	
	*/
	std::string tryMeLWOVolatilityCreate(const std::string& objectName, const LabelValueBlock& volLVB, bool validateKeys)
    {
        VALID_EXCEPTION_START
        
        // Record Inputs for logs, tests and playback
        RECORD_INPUTS(objectName, volLVB, validateKeys );

		std::shared_ptr<etrading::Volatility> optionPtr = etrading::createVolatility(objectName, volLVB, validateKeys);

        // Register the Option in the LWO Cache
        etrading::registerToCache< Volatility >(optionPtr);

        std::string result = objectName;

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );
        
        VALID_EXCEPTION_END
    }


	/* @brief			validation interface for the meLWOSabrMarketDataCreate method
	* @param [in] objectName        The name of the SABR Market Data object to create
	* @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	* @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	* @param [out]                  The objectName
	*/
	std::string tryMeLWOSabrMarketDataCreate(const std::string& objectName, const std::vector<std::string>& dataBlockNames, const etrading::JSONInfoBlockTuples& infoBlocks)
	{
		VALID_EXCEPTION_START

		// Perform initial basic sanity checks
		size_t nColumnHeaders = dataBlockNames.size();
		size_t nDataColumns = infoBlocks.size();

		MLIB_REQUIRE(nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided")
			MLIB_REQUIRE(nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " + std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " + std::to_string(static_cast<long long>(nDataColumns)))

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOSabrMarketDataCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryMeLWOSabrMarketDataCreate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for (unsigned int i = 0; i < dataBlockNames.size(); i++)
			{
				const LAString dataBlockName(dataBlockNames[i].c_str());
				const etrading::VariantMatrix& configData = std::get<2>(infoBlocks[i]);
				file.write(dataBlockName, transpose(configData));
			}
		}

		// Verify that the supplied propertyNames have been set
		const bool hasAnEmptyName = std::any_of(dataBlockNames.cbegin(),
			dataBlockNames.cend(),
			[](const std::string & dataBlockName) -> bool
		{
			return (dataBlockName.empty() || dataBlockName == "");
		});

		MLIB_REQUIRE(!hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString(dataBlockNames))

		etrading::SabrMarketData sabrMarketData(objectName, dataBlockNames, infoBlocks);

		etrading::copyToCache<etrading::SabrMarketData>(sabrMarketData);

		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOSabrMarketDataCreate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

		return objectName;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the meLWOSabrModelCalibrate method
	 * @param [in] objectName        The name of the SABR Model object, calibrated to market data
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
	std::string tryMeLWOSabrModelCalibrate(const std::string& objectName, const std::vector<std::string>& dataBlockNames, const etrading::JSONInfoBlockTuples& infoBlocks)
	{
		VALID_EXCEPTION_START

		// Perform initial basic sanity checks
		size_t nColumnHeaders = dataBlockNames.size();
		size_t nDataColumns = infoBlocks.size();

		MLIB_REQUIRE(nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided")
		MLIB_REQUIRE(nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " + std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " + std::to_string(static_cast<long long>(nDataColumns)))

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOSabrModelCalibrate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryMeLWOSabrModelCalibrate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for (unsigned int i = 0; i < dataBlockNames.size(); i++)
			{
				const LAString dataBlockName(dataBlockNames[i].c_str());
				const etrading::VariantMatrix& configData = std::get<2>(infoBlocks[i]);
				file.write(dataBlockName, transpose(configData));
			}
		}

		// Verify that the supplied propertyNames have been set
		const bool hasAnEmptyName = std::any_of(dataBlockNames.cbegin(),
			dataBlockNames.cend(),
			[](const std::string & dataBlockName) -> bool
		{
			return (dataBlockName.empty() || dataBlockName == "");
		});

		MLIB_REQUIRE(!hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString(dataBlockNames))

		// Create the Credit Model object
		etrading::SabrModel sabrModel(objectName, dataBlockNames, infoBlocks);

		// ..  and store in the cache
		etrading::copyToCache<etrading::SabrModel>(sabrModel);

		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOSabrModelCalibrate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

		return objectName;

		VALID_EXCEPTION_END
	}


	/* @brief			validation interface for meLWOSabrVolatility
	*  @param [in]		volSurfaceName	Vol surface name
	*  @param [in]		expiry			Option expiry
	*  @param [in]		tenor			Tenor
	*  @param [in]		strike			Strike
	*  @param [in]		forward			Forward
	*/
	double tryMeLWOSabrVolatility(const std::string& volSurfaceName, const std::string& expiry, const std::string& tenor, const double strike, const double forward)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(volSurfaceName, expiry, tenor, strike, forward);

		auto sabrModel = etrading::getSabrModel(volSurfaceName);

		double ret = sabrModel->getVol(expiry, tenor, strike, forward);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for meLWOSabrParameter
	*  @param [in]		volSurfaceName	Vol surface name
	*  @param [in]		expiry			Option expiry
	*  @param [in]		tenor			Tenor
	*  @param [in]		paramName		Parameter name, i.e. alpha, beta, nu, rho
	*/
	double tryMeLWOSabrParameter(const std::string& volSurfaceName, const std::string& expiry, const std::string& tenor, const std::string& paramName)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(volSurfaceName, expiry, tenor, paramName);

		auto sabrModel = etrading::getSabrModel(volSurfaceName);

		auto ret = sabrModel->getParam(expiry, tenor, etrading::toSabrParamEnum(paramName));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		return ret;

		VALID_EXCEPTION_END

	}

	/* @brief validation interface for meLWOSabrMarketDataDisplay
	* @param [in] marketDataObjectName	The market data object you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOSabrMarketDataDisplay(const std::string& marketDataObjectName, const std::string& marketDataKey)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(marketDataObjectName, marketDataKey);

		auto sabrMarketData = etrading::getSabrMarketData(marketDataObjectName);

		// extract just the data for the specified marketDataKey
		return sabrMarketData->viewInputParameters(marketDataKey.c_str());

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for meLWOSabrModelDisplay
	* @param [in] modelObjectName	The model object you wish to display
	* @param [in] modelKey			The name of the model label-value block that you wish to display
	* @param [out]					A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOSabrModelDisplay(const std::string& modelObjectName, const std::string& modelKey)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(modelObjectName, modelKey);

		auto sabrModel = etrading::getSabrModel(modelObjectName);

		// extract just the data for the specified modelKey
		return sabrModel->viewInputParameters(modelKey.c_str());

		VALID_EXCEPTION_END
	}



}
