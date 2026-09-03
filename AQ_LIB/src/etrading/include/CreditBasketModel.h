/*
 * @brief			Class which defines the Credit Basket Model.
 *                  The basket is constructed from underlying Credit Models.
 *                  Currently an implementation of the Gaussian Latent Variable Model.
 * @Created:		27 Feb 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LADate.h"

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"

#include <string>
#include <vector>

namespace etrading
{

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;


	class CreditBasketModel : public IsLWOObject
	{
	public:

		/* @brief Main Constructor
		 * @param[in] objectName    The name of this CreditBasketModel instance
		 * @param[in] propertyKeys  A vector containing the names of each configuration block
		 * @param[in] infoBlocks    A vector containing the configuation blocks
		 */
		CreditBasketModel(  const std::string& objectName,
							const std::vector<std::string>& propertyKeys,
							const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this CreditBasketModel instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		CreditBasketModel( const std::string& objectName, const FreeObject& freeObject );

		/* @brief Copy Constructor
		 */
		CreditBasketModel(const CreditBasketModel& rhs);

		~CreditBasketModel() {}

		std::shared_ptr<CreditBasketModel> clone() const;

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

        // This is a helper method to populate the supplied SchemaObject
        virtual void const toSchemaObject( SchemaObject& schemaObject ) const;

		/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be retrieved
	     * @param [out]  A LabelValue block containing the properties
		 */
		LabelValueBlock toLabelValueBlock( const std::string& propertyKey ) const;


		// ------- Accessor methods --------

		/* @brief	Given a future date, calculates the probability of survival to that date.
		*  @param[in]	toDate	The future date to use in the calculation. Must occur after the model as-of date.
		*  @returns	The survival probability
		*/
		double getFirstToDefaultHomogeneousBasketSurvivalProbability( const LADate& toDate ) const;

		/* @brief	Calculate the probability of survival to "toDate", given the contract has already survived up to "fromDate".
		*  @param[in]	toDate		The future end date to use in the calculation. Must occur after the model as-of date.
		*  @param[in]	fromDate	The future start date i.e. the probability of survival is 1 on this date.
		*							This parameter is allowed to be an empty date i.e. an optional paramweter.
		*  @returns	The survival probability
		*/
		double getFirstToDefaultHomogeneousBasketSurvivalProbability( const LADate& toDate, const LADate& fromDate ) const;

		/* @brief	Calculate the probability of default in the time period defined by "fromDate" and "toDate".
		*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
		*  @param[in]	fromDate	The future period start date
		*
		*  @returns	The survival probability
		*/
		double getFirstToDefaultHomogeneousBasketDefaultProbability( const LADate& toDate, const LADate& fromDate ) const;

		static std::vector<std::string> model_properties_lvbKeys()
		{
			const std::string arr[] =
			{
				CREDITBASKETMODEL_MODEL_PROPERTIES_KEY::ASOF_DATE,
				CREDITBASKETMODEL_MODEL_PROPERTIES_KEY::BASKET_TYPE,	// HomogeneousLoss or InHomogeneousLoss
				CREDITBASKETMODEL_MODEL_PROPERTIES_KEY::NTH_TO_DEFAULT	// First to default indicated by 1, etc
			};

			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}


	private:
		
		/* @brief	Parses the CREDIT_MODELS data block and verifies that each credit model exists in the cache and is valid.
		 *			Also verifies that each credit is assigned a valid correlation-beta.
		 */
		void validateUnderlyingCreditModels();

		// @brief	Called by constructors to perform initialization of data members
		void initialize();

		/* @brief Validate that the supplied toDate and fromDate are consistent with each other and the model asOfDates
		 * @param [in]	toDate		The initial date for survival probability calculations
		 * @param [in]	fromDate	The final date for survival probability calculations
		 */
		void validateDates( const LADate& toDate, const LADate& fromDate ) const;

		/*
		*  @brief  Utility method which validates the property keys of this CreditBasketModel, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const;

		/* @brief	Computes the "defaultFrontier" for each underlying credit model, given the specified calibrationDate.
		*			This quantity is the "C Parameter" in the Gaussian Latent Variable Model.
		*			The underlying credit has defaulted by time T if a random normal A is less than or equal to C(T).
		* @param[in]	calibrationDate	The date used to compute survivial probabilities from which the defaultFrontier is derived.
		* @returns		A vector of defaultFrontier values, one per credit in the basket.
		*/
		std::vector<double> calibrateDefaultFrontier( const LADate& calibrationDate ) const;

		/* @brief	Computes the product term in the homogeneous basket integral.
		*  @param[in]	zMarketFactor	The common market factor ( the integration parameter )
		*  @param[in]	defaultFrontier	The calibrated defaultFrontier. This is the "C-parameter" in the
		*				Gaussian Latent Variable Model.
		*  @returns	The integrand product. This is the product of terms from each underlying credit.
		*/
		double calculateIntegrandProduct( const double zMarketFactor, const std::vector<double>& defaultFrontier ) const;

		// The main data store of this object. Allows easy serialization
		FreeObject freeObject_;

		// Parameters populated during initialization step
		LADate asOfDate_;
		std::vector<std::string> creditModelNames_;	// The name of each credit model in the basket
		std::vector<double> correlationBetas_;		// The correlation of each credit to a common "market factor"
		CreditBasketTypeEnum basketType_;			// HOMOGENEOUS_LOSS or INHOMOGENEOUS_LOSS
		int nthToDefault_;							// First to default indicated by 1, etc

	};

	typedef std::shared_ptr< CreditBasketModel > CreditBasketModelPtr;
}

