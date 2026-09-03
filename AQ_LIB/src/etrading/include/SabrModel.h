#pragma once

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "SabrMarketData.h"
#include "SwapGenerator.h"

namespace etrading
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

	class SabrModel : public IsLWOObject
	{
	public:

		/* @brief Main Constructor
		 * @param[in] objectName    The name of this SabrModel instance
		 * @param[in] propertyKeys  A vector containing the names of each configuration block
		 * @param[in] infoBlocks    A vector containing the configuation blocks
		 */
		SabrModel(const std::string& objectName, const std::vector<std::string>& propertyKeys, const std::vector<TableInfo>& infoBlocks);

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this SabrModel instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		SabrModel(const std::string& objectName, const FreeObject& freeObject);

		/* @brief Copy Constructor
		 */
		SabrModel(const SabrModel& rhs);

		virtual ~SabrModel() {}

		std::shared_ptr<SabrModel> clone() const;

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

		// This is a helper method to populate the supplied SchemaObject
		virtual void const toSchemaObject(SchemaObject& schemaObject) const;

		/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [out]  A VariantMatrix containing a LabelValue block of properties
		 */
		VariantMatrix viewInputParameters(const std::string& propertyKey) const;

		/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
		 * @param [out]  A LabelValue block containing the properties
		 */
		LabelValueBlock toLabelValueBlock(const std::string& propertyKey) const;

		static std::vector<std::string> model_properties_lvbKeys()
		{
			std::vector<std::string> expectedKeys =
			{
				SABR_MODEL_PROPERTIES_KEY::AS_OF_DATE
				, SABR_MODEL_PROPERTIES_KEY::CURVE_COLLECTION
				, SABR_MODEL_PROPERTIES_KEY::SWAP_GENERATOR
				, SABR_MODEL_PROPERTIES_KEY::CALENDAR
				, SABR_MODEL_PROPERTIES_KEY::DAYCOUNT
				, SABR_MODEL_PROPERTIES_KEY::BUSINESSDAY_ADJUSTMENT
				, SABR_MODEL_PROPERTIES_KEY::APPROX_METHOD
				, SABR_MODEL_PROPERTIES_KEY::CALIBRATION_METHOD
				, SABR_MODEL_PROPERTIES_KEY::TARGET
				, SABR_MODEL_PROPERTIES_KEY::VOL_TYPE
				, SABR_MODEL_PROPERTIES_KEY::SHIFT
				, SABR_MODEL_PROPERTIES_KEY::SOLVER_EPSILON
				, SABR_MODEL_PROPERTIES_KEY::ALPHA_FROM_ATM_VOL
			};

			return expectedKeys;
		}

		static std::vector<std::string> model_parameters_lvbKeys()
		{
			std::vector<std::string> expectedKeys =
			{
				SABR_MODEL_PROPERTIES_KEY::ALPHA_PARAM
				, SABR_MODEL_PROPERTIES_KEY::BETA_PARAM
				, SABR_MODEL_PROPERTIES_KEY::NU_PARAM
				, SABR_MODEL_PROPERTIES_KEY::RHO_PARAM
			};

			return expectedKeys;
		}

		static std::vector<std::string> model_mktdata_lvbKeys()
		{
			std::vector<std::string> expectedKeys =
			{
				SABR_MODEL_PROPERTIES_KEY::ATMF_SWAPRATES
				, SABR_MODEL_PROPERTIES_KEY::ANNUITY
			};

			return expectedKeys;
		}


		static std::vector<std::string> marketData_properties_lvbKeys()
		{
			const std::string arr[] =
			{
				SABR_MARKETDATA_PROPERTIES_KEY::MARKET_DATA_TYPE
				, SABR_MARKETDATA_PROPERTIES_KEY::STRIKE_ADJUST_AMOUNT
				, SABR_MARKETDATA_PROPERTIES_KEY::VALUE_TYPE
			};
			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

			return expectedKeys;
		}

		static std::vector<std::string> calib_param_lvbKeys()
		{
			const std::string arr[] =
			{
				SABR_MODEL_PROPERTIES_KEY::ALPHA_PARAM
				, SABR_MODEL_PROPERTIES_KEY::BETA_PARAM
				, SABR_MODEL_PROPERTIES_KEY::NU_PARAM
				, SABR_MODEL_PROPERTIES_KEY::RHO_PARAM
			};
			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

			return expectedKeys;
		}

		double getVol(const std::string& expiry, const std::string& tenor, const double strike, const double forward);

		double getParam(const double expiryTerm, const double tenorTerm, const SabrParamEnum& paramName);

		double getParam(const std::string& expiry, const std::string& tenor, const SabrParamEnum& paramName);

	private:

		AQLString getValueFromGenerator(const SwapGeneratorPtr& swapGenerator, const std::string& lvbKey) const;

		/* @brief Populates a default swap expression label value block
		*
		* @param [in]   effectiveDate	Swap effective date
		* @param [in]   maturityTenor	Swap maturity tenor
		*/
		LabelValueBlock setupSwapExpressionLVBforCalibration(const AQLDate& effectiveDate, const std::string& maturityTenor) const;

		bool doesKeyExists(const std::string& propertyKey);

		void populateMarketDataExpiryTenorTerms(const SabrMarketDataPtr&  sabrMktDataObject);

		void populateATMFSwapRateAndAnnuity(DoubleMatrix& strikeAtmMat, DoubleMatrix& annuityMat, const SabrMarketDataPtr& volAtmObject, const std::string& swapGeneratorName, const bool includeSwapRate, const bool includeAnnuity);

		/*
		*  @brief  Utility method which validates the property keys of this SabrModel, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const;

		// @brief	Called by constructors to calibrate sabr parameters to the provided market data
		void calibrate();

		// The main data store of this object. Allows easy serialization
		FreeObject freeObject_;

		//Calibration parameters
		DoubleMatrix alphaMatrix_;
		DoubleMatrix betaMatrix_;
		DoubleMatrix nuMatrix_;
		DoubleMatrix rhoMatrix_;
		SABRApproxMethodEnum approxMethod_;
		double forwardShift_ = 0.0;
		bool isLognormal_;

		//use in diff methods
		AQLDate asOfDate_;
		std::string businessDayAdjustment_;
		std::string calendar_;
		DayCountEnum dayCount_;
		std::string curveCollection_;
		DoubleVector expiryTermVector_;
		DoubleVector tenorTermVector_;

	};

	typedef std::shared_ptr< SabrModel > SabrModelPtr;
}






