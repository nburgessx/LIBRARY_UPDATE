#include "ql/math/integrals/gaussianquadratures.hpp"

#include <boost/math/distributions/normal.hpp>
#include <boost/algorithm/string.hpp>

#include "CreditBasketModel.h"
#include "NormalDistribution.h"
#include "LWOUtilities.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "ExceptionMacros.h"


namespace etrading
{

	CreditBasketModel::CreditBasketModel( const std::string& objectName,
										  const std::vector<std::string>& propertyKeys,
										  const std::vector<TableInfo>& infoBlocks ) 
						: IsLWOObject(objectName, CREDIT_BASKET_MODEL), 
						  freeObject_(objectName)
	{

		// Create a FreeObject from each property label-value block, and concatenate to our FreeObject data member
		const bool allowJaggedData = false;
        for( unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++ )
        {
            const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const VariantMatrix& rangeData = std::get<2>( infoBlocks[ gridCounter ] );
            freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, propertyKeys[ gridCounter ], allowJaggedData );
        }

		// Verify that all CreditBasketModel properties are valid and expected
		validateKeys();

		initialize();
	}


	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this CreditBasketModel instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	CreditBasketModel::CreditBasketModel( const std::string& objectName, const FreeObject& freeObject ) 
					: IsLWOObject(objectName, CREDIT_MODEL ), 
					  freeObject_(freeObject)
	{
		initialize();
    }


	/* @brief Copy Constructor
	 */
	CreditBasketModel::CreditBasketModel(const CreditBasketModel& rhs) 
		: IsLWOObject( rhs.getRefToName(), CREDIT_BASKET_MODEL ), 
		  freeObject_( rhs.freeObject_ ),
		  asOfDate_( rhs.asOfDate_ ),
		  creditModelNames_( rhs.creditModelNames_ ),
		  correlationBetas_( rhs.correlationBetas_ ),
		  basketType_( rhs.basketType_ ),
		  nthToDefault_( rhs.nthToDefault_ )
	{
	}


	std::shared_ptr<CreditBasketModel> CreditBasketModel::clone() const
    {
       auto data = std::make_shared<CreditBasketModel>(*this);
       return data;
    }

	/* @brief	Parses the CREDIT_MODELS data block and verifies that each credit model exists in the cache and is valid.
	 *			Also verifies that each credit is assigned a valid correlation-beta.
	 */
	void CreditBasketModel::validateUnderlyingCreditModels()
	{
		// Read the credit model names and correlations
		const bool throwIfKeyMissing = false;
		VariantMatrix creditModelConfigs = getVariantMatrixFromFreeObject( freeObject_, toString( CREDIT_MODELS ), throwIfKeyMissing );
		const size_t nCreditModelColumns = creditModelConfigs.size();

		if ( nCreditModelColumns != 2 )
		{
			AQ_THROW( "CREDIT_MODELS should contain 2 columns. Found " + std::to_string( static_cast<long long> ( nCreditModelColumns )) + " columns.");
		}

		const size_t nCreditModelRows = creditModelConfigs[0].size();
		for (size_t i=0; i<nCreditModelRows; i++)
		{
			// Only process the row if the data in column 0 is non-blank. i.e. trim blank rows
			const std::string& creditModelName = creditModelConfigs[0][i];
			if ( creditModelName != "" )
			{
				// Verify that the credit model actually exists
				auto creditModel = getCreditModel( creditModelName );
				LADate creditModelAsOfDate = creditModel->getAsOfDate();
				AQ_REQUIRE( creditModelAsOfDate == asOfDate_, "Credit Model: " + creditModelName + " has different asOf date to credit basket" );
				
				// Extract the correlation beta
				Variant value = creditModelConfigs[1][i];
				double correlationBeta = std::numeric_limits<double>::quiet_NaN();

				switch ( value.getType() )
				{
				case STRING_VALUE:
				{
					std::string stringValue = value.getValue<std::string>();
					boost::trim( stringValue );
					AQ_REQUIRE( stringValue != "", "Missing correlationBeta for credit model: " + creditModelName );
					char * pFirstNonNumber;
                    correlationBeta = strtod( stringValue.c_str(), &pFirstNonNumber );
					break;
				}
				case DOUBLE_VALUE:
					correlationBeta = value.getValue<double>();
					break;
				case INTEGER_VALUE:
					correlationBeta = static_cast<double> (value.getValue<int>() );
					break;
				default:
					AQ_THROW( "Unexpected data type in Credit Basket Model CREDIT_MODELS Correlation Beta column ");
				}
				
				AQ_REQUIRE( std::fabs( correlationBeta ) <= 1.0, "Correlation Beta must be in the range +/- 100%" );

				creditModelNames_.push_back( creditModelName );
				correlationBetas_.push_back( correlationBeta );
			}
		}
		AQ_REQUIRE( creditModelNames_.size() > 0, "The credit basket should contain at least one credit model.");
	}

	// @brief	Called by constructors to calibrate hazard rates from the provided market data
	void CreditBasketModel::initialize()
	{
		// Read Credit Model properties
		LabelValueBlock modelProperties = toLabelValueBlock( toString( MODEL_PROPERTIES ) );
		asOfDate_     = modelProperties.getCompulsoryValueAsDate( CREDITBASKETMODEL_MODEL_PROPERTIES_KEY::ASOF_DATE );
		basketType_   = toCreditBasketTypeEnum( modelProperties.getCompulsoryValueAsLAString( CREDITBASKETMODEL_MODEL_PROPERTIES_KEY::BASKET_TYPE ).getCString() );
		nthToDefault_ = (int) modelProperties.getCompulsoryValueAsDouble( CREDITBASKETMODEL_MODEL_PROPERTIES_KEY::NTH_TO_DEFAULT );

		AQ_REQUIRE( basketType_ == HOMOGENEOUS_LOSS, "Only HomogeneousLoss baskets are currently supported." );
		AQ_REQUIRE( nthToDefault_ == 1, "Only First to default ( NthToDefault=1 ) baskets are currently supported.");

		validateUnderlyingCreditModels();
	}

	/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be retrieved
	* @param [out]  A LabelValue block containing the properties
	*/
	LabelValueBlock CreditBasketModel::toLabelValueBlock( const std::string& propertyKey ) const
	{
		LAStringMatrix stringMatrix = getLAStringMatrixFromFreeObject( freeObject_, propertyKey );
		LabelValueBlock lvb( stringMatrix );

		return lvb;
	}

	/* @brief Validate that the supplied toDate and fromDate are consistent with each other and the model asOfDates
	 * @param [in]	toDate		The initial date for survival probability calculations
	 * @param [in]	fromDate	The final date for survival probability calculations
	 */
	void CreditBasketModel::validateDates( const LADate& toDate, const LADate& fromDate ) const
	{
		// If fromDate is specified, perform sanity checks
		if ( fromDate != LADate() )
		{
			if ( fromDate  < asOfDate_ )
			{
				AQ_THROW( "Invalid fromDate '" + toDate.stringWithFormat() + "' is earlier than model asOfDate '" + asOfDate_.stringWithFormat() + "'." );
			}
			if ( toDate < fromDate )
			{
				AQ_THROW( "Invalid toDate '" + toDate.stringWithFormat() + "' is earlier than fromDate '" + fromDate.stringWithFormat() + "'." );
			}
		}
	}

	/* @brief	Computes the "defaultFrontier" for each underlying credit model, given the specified calibrationDate.
	*			This quantity is the "C Parameter" in the Gaussian Latent Variable Model.
	*			The underlying credit has defaulted by time T if a random normal A is less than or equal to C(T).
	* @param[in]	calibrationDate	The date used to compute survivial probabilities from which the defaultFrontier is derived.
	* @returns		A vector of defaultFrontier values, one per credit in the basket.
	*/
	std::vector<double> CreditBasketModel::calibrateDefaultFrontier( const LADate& toDate ) const
	{
		const size_t nCreditModels = creditModelNames_.size();
		std::vector<double> defaultFrontier( nCreditModels);  // This represents C_i(T).

		for ( size_t i=0; i<nCreditModels; i++ )
		{
			auto creditModel = etrading::getCreditModel( creditModelNames_[i] );

			const double survivalProbability = creditModel->getSurvivalProbability( toDate );
			const double defaultFrontierParameter = standardNormalDistributionInverse ( 1.0 - survivalProbability );

			defaultFrontier[ i ] = defaultFrontierParameter;
		}

		return defaultFrontier;
	}

	/* @brief	This method implements First-To-Default baskets with homogeneous-loss from the book
	*           "Modelling Single-name and Multi-name Credit Derivatives" by Dominic Oane p285.
	*			In particular this method computes the product term in the homogeneous basket integral.
	*  @param[in]	zMarketFactor	The common market factor ( the integration parameter )
	*  @param[in]	defaultFrontier	The calibrated defaultFrontier. This is the "C-parameter" in the
	*				Gaussian Latent Variable Model.
	*  @returns	The integrand product. This is the product of terms from each underlying credit.
	*/
	double CreditBasketModel::calculateIntegrandProduct( const double zMarketFactor, const std::vector<double>& defaultFrontier ) const
	{
		double product = 1.0;
		const size_t nCorrelationBetas = correlationBetas_.size();
		for ( size_t i=0; i<nCorrelationBetas; i++ )
		{
			const double correlationBeta = correlationBetas_[i];

			const double numerator = ( correlationBeta * zMarketFactor - defaultFrontier[i] );
			const double divisor = sqrt( 1.0 - correlationBeta * correlationBeta );

			double correlationFactor = 0.0;
			if ( AQ_IS_EQUAL_ZERO( divisor ) )
			{
				if ( numerator > 0.0 )
				{
					// The quotient is +infinity, hence the cumulative distribution is +1.0
					correlationFactor = 1.0;
				}
				else
				{
					// The quotient is -infinity, hence the cumulative distribution is 0.0
					correlationFactor = 0.0;
				}
			}
			else
			{
				// Quotient is reasonable. Go ahead and perform the full calculation
				const double quotient = numerator / divisor;
				correlationFactor = standardNormalDistribution( quotient );
			}

			product *= correlationFactor;
		}

		return product;
	}


	/* @brief	Given a future date, calculates the probability of survival to that date.
	*			This method implements First-To-Default baskets with homogeneous-loss from the book
	*           "Modelling Single-name and Multi-name Credit Derivatives" by Dominic Oane p285
	*  @param[in]	toDate	The future date to use in the calculation. Must occur after the model as-of date.
	*  @returns	The survival probability
	*/
	double CreditBasketModel::getFirstToDefaultHomogeneousBasketSurvivalProbability( const LADate& toDate ) const
	{
		std::vector<double> defaultFrontier = calibrateDefaultFrontier( toDate );

		const size_t nCorrelationBetas = correlationBetas_.size();
		AQ_REQUIRE( nCorrelationBetas == defaultFrontier.size(), "Length of correlation vector does not match numberof credit basket xassets" );
		
		// Transform the integration limits from minLimit / maxLimit to +/-1 used by Gauss-Legendre integration routine.
		const double minLimit = -10.0;
		const double maxLimit = 10.0;
		const double limitSpread = 0.5 * ( maxLimit - minLimit );
		const double limitMidpoint = 0.5 * ( maxLimit + minLimit );

		// The homogeneous basket survival probability conditional on the marketFactor
		auto integrandFunction = [&] ( const double zMarketFactor ) -> double
		{
			const double transformedIntegrationVariable = limitSpread * zMarketFactor + limitMidpoint;
			const double densityFunction  = standardNormalDistributionPDF( transformedIntegrationVariable );
			const double integrandProduct = calculateIntegrandProduct( transformedIntegrationVariable, defaultFrontier );
			return limitSpread * densityFunction * integrandProduct;
		};

		/* NOTE: Although this is an infinite 1D integral, it is found that Gauss-Hermite is unstable
		*  when used with more than 16 integration points. Instead, use the Gauss-Legendre integration
		*  routine, which is found to be much more stable.
		*  http://quantlib.10058.n7.nabble.com/Issues-with-Gauss-Hermite-Integration-td6680.html
		*/
		const size_t nIntegrationPoints = 100;
		QuantLib::GaussLegendreIntegration integrator( nIntegrationPoints );

		// Integrate over the market factor Z
		const double survivalProbability = integrator( integrandFunction );

		AQ_REQUIRE( AQ_IS_LESS_THAN_OR_EQUAL(survivalProbability, 1.0) , "Problem with credit basket integration: Survival Probability value is greater than 1.0");
		AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL(survivalProbability, 0.0), "Problem with credit basket integration: Survival Probability value is less than 0.0");

		return survivalProbability;
	}

	/* @brief	Calculate the probability of survival to "toDate", given the contract has already survived up to "fromDate".
	*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
	*  @param[in]	fromDate	The future start date i.e. the probability of survival is 1 on this date.
	*							This parameter is allowed to be an empty date i.e. an optional paramweter.
	*  @returns	The survival probability
	*/
	double CreditBasketModel::getFirstToDefaultHomogeneousBasketSurvivalProbability( const LADate& toDate, const LADate& fromDate ) const
	{
		validateDates( toDate, fromDate );

		double survivalProbability = getFirstToDefaultHomogeneousBasketSurvivalProbability( toDate );

		// fromDate is optional: it is OK for it to contain a default empty date
		if ( fromDate != LADate() )
		{
			const double survivalFrom = getFirstToDefaultHomogeneousBasketSurvivalProbability( fromDate );
			survivalProbability /= survivalFrom;
		}
		return survivalProbability;
	}


	/* @brief	Calculate the probability of default in the time period defined by "fromDate" and "toDate".
	*  @param[in]	toDate	The future end date to use in the calculation. Must occur after the model as-of date.
	*  @param[in]	fromDate	The future period start date
	*
	*  @returns	The survival probability
	*/
	double CreditBasketModel::getFirstToDefaultHomogeneousBasketDefaultProbability( const LADate& toDate, const LADate& fromDate ) const
	{
		validateDates( toDate, fromDate );

		// We allow a missing value for 'fromDate'. In this case we default to 'asOfDate'.
		LADate fromDt = ( fromDate == LADate() ) ? asOfDate_ : fromDate;

		double defaultProbability = getFirstToDefaultHomogeneousBasketSurvivalProbability( fromDate ) - getFirstToDefaultHomogeneousBasketSurvivalProbability( toDate );
		return defaultProbability;
	}

	/*
	*  @brief  validates the property keys of this CreditBasketModel, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void CreditBasketModel::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for ( size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			LabelValueBlock inputLVB = toLabelValueBlock( propertyName );

			etrading::CreditModelEnum CreditModelEnum = etrading::toCreditModelEnum( propertyName );
			switch (CreditModelEnum)
			{
				case MODEL_PROPERTIES:
					validateKeysForLVB( model_properties_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				default:
					break;
			}
		}
	}

	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject CreditBasketModel::toSchemaObject() const
	{
		SchemaObject schemaObject( CREDIT_BASKET_MODEL, getRefToName());
        toSchemaObject(schemaObject);

        return schemaObject;
	}

    // This is a helper method to populate the supplied SchemaObject
    void const CreditBasketModel::toSchemaObject( SchemaObject& schemaObject ) const
   	{
        
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for (size_t i=0; i < freeObject_.numberOfSchemas(); i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

}
