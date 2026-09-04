#include "SabrModel.h"
#include "TypeHelpers.h"
#include "Swap.h"

#include "AQOUtilities.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "AQLDateScheduleHelpers.h"
#include "AQLMathSwaptionVolUtility.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathDateUtilities.h"

namespace etrading
{
	/* @brief Main Constructor
	 * @param[in] objectName    The name of this SabrModel instance
	 * @param[in] propertyKeys  A vector containing the names of each configuration block
	 * @param[in] infoBlocks    A vector containing the configuation blocks
	 */
	SabrModel::SabrModel(const std::string& objectName, const std::vector<std::string>& propertyKeys, const std::vector<TableInfo>& infoBlocks)	: IsAQObject(objectName, SABR_MODEL), freeObject_(objectName)
	{
		// Create a FreeObject from each property label-value block, and concatenate to our FreeObject data member
		const bool allowJaggedData = false;
		for (unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++)
		{
			const std::vector<std::string>& columnNames = std::get<0>(infoBlocks[gridCounter]);
			const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>(infoBlocks[gridCounter]);
			const VariantMatrix& rangeData = std::get<2>(infoBlocks[gridCounter]);
			freeObject_ += createFreeObjectFromGrid(objectName, columnNames, colTypes, rangeData, propertyKeys[gridCounter], allowJaggedData);
		}

		// Verify that all SabrModel properties are valid and expected
		validateKeys();

		calibrate();
	}

	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this SabrModel instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	SabrModel::SabrModel(const std::string& objectName, const FreeObject& freeObject) : IsAQObject(objectName, SABR_MODEL), freeObject_(freeObject)
	{
		calibrate();
	}

	/* @brief Copy Constructor
	 */
	SabrModel::SabrModel(const SabrModel& rhs)
		: IsAQObject(rhs.getRefToName(), SABR_MODEL),
		freeObject_(rhs.freeObject_),
		asOfDate_(rhs.asOfDate_),
		businessDayAdjustment_(rhs.businessDayAdjustment_),
		calendar_(rhs.calendar_),
		dayCount_(rhs.dayCount_),
		approxMethod_(rhs.approxMethod_),
		forwardShift_(rhs.forwardShift_),
		isLognormal_(rhs.isLognormal_),
		curveCollection_(rhs.curveCollection_),
		expiryTermVector_(rhs.expiryTermVector_),
		tenorTermVector_(rhs.tenorTermVector_),
		alphaMatrix_(rhs.alphaMatrix_),
		betaMatrix_(rhs.betaMatrix_),
		nuMatrix_(rhs.nuMatrix_),
		rhoMatrix_(rhs.rhoMatrix_)
	{}

	std::shared_ptr<SabrModel> SabrModel::clone() const
	{
		auto data = std::make_shared<SabrModel>(*this);
		return data;
	}

	AQLString SabrModel::getValueFromGenerator(const SwapGeneratorPtr& swapGenerator, const std::string& lvbKey) const
	{
		std::string value;

		// Fetch the Swap Conventions block from the CDS Swap Generator
		AQLStringMatrix swapConventions = swapGenerator->viewInputParameters();

		for (size_t i = 0; i < swapConventions.size(); i++)
		{
			const AQLStringVector& row = swapConventions[i];
			std::string key(row[0].getCString());
			if (boost::iequals(key, lvbKey))
			{
				value = row[1].getCString();
				break;
			}
		}
		return value;
	}

	/* @brief Populates a default swap expression label value block
	*
	* @param [in]   effectiveDate	Swap effective date
	* @param [in]   maturityTenor	Swap maturity tenor
	*/
	LabelValueBlock SabrModel::setupSwapExpressionLVBforCalibration(const AQLDate& effectiveDate, const std::string& maturityTenor) const
	{
		// Set up the Swap Expression LVB used for repricing swap calibration instruments
		AQLStringVector keys;
		AQLStringVector values;

		keys.reserve(6);
		values.reserve(6);

		// We standardize these trade keys because we are only interested in par rate calculations 
		keys.push_back(IRS_KEY::PAY_RECEIVE.c_str());					values.push_back("PAY");
		keys.push_back(IRS_KEY::NOTIONAL.c_str());					    values.push_back("1.0");
		keys.push_back(IRS_KEY::EFFECTIVE_DATE.c_str());				values.push_back(std::to_string(static_cast<long long>(AQLDateScheduleHelpers::getExcelDate(effectiveDate))));
		keys.push_back(IRS_KEY::MATURITY_DATE.c_str());				    values.push_back(maturityTenor);
		keys.push_back(SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1.c_str());	values.push_back("0.0");
		keys.push_back(SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2.c_str());	values.push_back("0.0");

		LabelValueBlock swapExpressionLVB = etrading::populateLabelValueBlock(keys, values);

		return swapExpressionLVB;
	}


	bool SabrModel::doesKeyExists(const std::string& propertyKey)
	{
		return freeObject_.doesKeyExist(propertyKey);
	}

	void SabrModel::populateMarketDataExpiryTenorTerms(const SabrMarketDataPtr&  sabrMktDataObject)
	{
		auto expiryStrVector = sabrMktDataObject->getExpiryStrVector();
		auto tenorStrVector = sabrMktDataObject->getTenorStrVector();

		auto expirySize = expiryStrVector.size();
		auto tenorSize = tenorStrVector.size();

		expiryTermVector_.reserve(expirySize);
		tenorTermVector_.reserve(tenorSize);

		//Core function hard code the dayCount to be AC_365I, which is the same as ACT/ACT
		AQLString dayCountToUse = toString(dayCount_).c_str();
		
		for (size_t i = 0; i < expirySize; i++)
		{
			const AQLDate expiryDate = etrading::getDateFromTenor(boost::assign::list_of(asOfDate_), expiryStrVector[i], businessDayAdjustment_, calendar_, "")[0];

			//TODO: the core function use AQLMathDateUtilities::getTerm() which has different result from etrading::getYearFraction().
			// We use the same code for now, as it is also used in AQLMathSwaptionVolUtility::getExpiryPoint()
			expiryTermVector_.push_back(AQLMathDateUtilities::getTerm(asOfDate_, expiryDate, dayCountToUse, true));
			//expiryTermVector_.push_back(etrading::getYearFraction(asOfDate_, expiryDate, dayCount_));
		}

		
		//TODO: the core function use the following code to get year fractions of swap tenors, which has different result from etrading::getYearFraction().
		// We use the same code for now, as it is also used in AQLMathSwaptionVolUtility::getTenorPoint()
		int y, m, d, w;
		for (size_t i = 0; i < tenorSize; i++)
		{
			//This is the calc from core function, leave it for reference
			AQLMathDateCalculations::termStrtoYMDW(tenorStrVector[i], y, m, d, w);
			tenorTermVector_.push_back(static_cast<double> (y) + static_cast<double> (m) / 12.0);

			//const AQLDate tenorDate = etrading::getDateFromTenor(boost::assign::list_of(asOfDate_), tenorStrVector[i], businessDayAdjustment_, calendar_, "")[0];
			//tenorTermVector_.push_back(etrading::getYearFraction(asOfDate_, tenorDate, dayCountToUse));

		}
	}

	void SabrModel::populateATMFSwapRateAndAnnuity(DoubleMatrix& strikeAtmMat, DoubleMatrix& annuityMat, const SabrMarketDataPtr& volAtmObject, const std::string& swapGeneratorName, const bool includeSwapRate, const bool includeAnnuity)
	{

		const bool isXccySwap = false;
		LabelValueBlock swapPropertiesLVB;
		LabelValueBlock valuationSettingsLVB(VALUATION_SETTING_KEYS::CURVE_COLLECTION, curveCollection_);
		LabelValueBlock fixingTableNames;

		auto volAtmMat = volAtmObject->getInstrumentQuoteMatrix();
		size_t rowSize = volAtmMat.size();
		size_t columnSize = volAtmMat[0].size();

		//If the block is not provided by user, calculate strikeATM matrix and annuity matrix using SwapGenerator
		for (size_t i = 0; i < rowSize; i++)
		{
			//Swap effective date
			AQLString expiryTenor = volAtmObject->getExpiryStrVector()[i];
			const AQLDate effectiveDate = etrading::getDateFromTenor(boost::assign::list_of(asOfDate_), expiryTenor, businessDayAdjustment_, calendar_, "")[0];

			for (size_t j = 0; j < columnSize; j++)
			{
				std::string swapName = i + "_" + j;

				LabelValueBlock swapExpressionLVB(setupSwapExpressionLVBforCalibration(effectiveDate, volAtmObject->getTenorStrVector()[j].getCString()));

				SwapPtr swap = etrading::createSwapFromGenerator(swapName, swapGeneratorName, swapExpressionLVB, swapPropertiesLVB, false);

				if (includeSwapRate)
				{
					double parRate = swap->parRate(valuationSettingsLVB, fixingTableNames);
					strikeAtmMat[i][j] = parRate;
				}

				if (includeAnnuity)
				{
					//annuity of the float leg
					double annuity = swap->annuity(valuationSettingsLVB, swap->getLeg(1)->getLegName());
					annuityMat[i][j] = annuity;
				}

			}
		}

	}

	//Helper function to populate the SABR parameter matrix
	DoubleMatrix getSabrParamMatrix(const size_t rowSize, const size_t columnSize, const std::string& initValueStr)
	{

		if (isValid< double >(initValueStr))
		{
			DoubleMatrix paramMat(rowSize, DoubleVector(columnSize, StringToNumber<double>(initValueStr)));
			return paramMat;
		}
		else
		{
			//initValueStr is the object handle name
			return getSabrMarketData(initValueStr)->getInstrumentQuoteMatrix();

		}
	}

	// @brief	Called by constructors to calibrate hazard rates from the provided market data
	void SabrModel::calibrate()
	{
		//Calibration Properties Block
		LabelValueBlock modelProperties = toLabelValueBlock(toString(SABR_MODEL_PROPERTIES));

		asOfDate_ = modelProperties.getCompulsoryValueAsDate(SABR_MODEL_PROPERTIES_KEY::AS_OF_DATE);
		curveCollection_ = modelProperties.getCompulsoryValue(SABR_MODEL_PROPERTIES_KEY::CURVE_COLLECTION);
		
		auto swapGeneratorName = modelProperties.getCompulsoryValue(SABR_MODEL_PROPERTIES_KEY::SWAP_GENERATOR);
		auto swapGenerator = etrading::getSwapGenerator(swapGeneratorName);

		dayCount_ = toDayCountEnum(modelProperties.getCompulsoryValue(SABR_MODEL_PROPERTIES_KEY::DAYCOUNT));
		businessDayAdjustment_ = modelProperties.getCompulsoryValue(SABR_MODEL_PROPERTIES_KEY::BUSINESSDAY_ADJUSTMENT);
		calendar_ = modelProperties.getCompulsoryValue(SABR_MODEL_PROPERTIES_KEY::CALENDAR);

		approxMethod_ = toSABRApproxMethodEnum(modelProperties.getCompulsoryValueAsLAString(SABR_MODEL_PROPERTIES_KEY::APPROX_METHOD).getCString());
		AQ_REQUIRE(approxMethod_ == HAGAN_APPROX_METHOD, "Approximation Method only support 'HAGAN' at the moment.");

		auto calibMethod = modelProperties.getCompulsoryValueAsLAString(SABR_MODEL_PROPERTIES_KEY::CALIBRATION_METHOD);

		auto solverEpsilon = modelProperties.getOptionalValueAsDouble(SABR_MODEL_PROPERTIES_KEY::SOLVER_EPSILON, 1e-10);

		auto volType = modelProperties.getOptionalValueAsString(SABR_MODEL_PROPERTIES_KEY::VOL_TYPE, "LOGNORMAL");
		isLognormal_ = (etrading::toVolatilityTypeEnum(volType) != etrading::NORMAL_VOLATILITY);
		
		// forward shift value
		forwardShift_ = modelProperties.getOptionalValueAsDouble(SABR_MODEL_PROPERTIES_KEY::SHIFT, 0.0);

		//set the shift to zero if it's normal vol
		if (!isLognormal_)
		{
			forwardShift_ = 0.0;
		}

		auto calibrationTarget = toSABRCalibrationTargetEnum(modelProperties.getCompulsoryValueAsString(SABR_MODEL_PROPERTIES_KEY::TARGET));
		AQ_REQUIRE(calibrationTarget == SABR_CALIB_VOLATILITY, "Calibration Target only support 'Volatility' at the moment.");

		bool alphaFromAtmVol = modelProperties.getOptionalValueAsBool(SABR_MODEL_PROPERTIES_KEY::ALPHA_FROM_ATM_VOL, true);
			
		//Vol MarketData Block
		auto volMarketDataBlock = getLAStringMatrixFromFreeObject(freeObject_, toString(SABR_MODEL_VOL_MKTDATA));

		if (calibrationTarget == SABR_CALIB_VOLATILITY)
		{
			AQ_REQUIRE(volMarketDataBlock.size() > 0 && volMarketDataBlock[0].size() >= 2, "volMarketDataBlock should have at least 2 columns: volObjectName, weight.")
		}
		else
		{
			AQ_REQUIRE(volMarketDataBlock.size() > 0 && volMarketDataBlock[0].size() == 3, "volMarketDataBlock should have 3 columns: volObjectName, weight, sign.")
		}

		//find ATM vol market data
		std::string volObjectAtmName;
		for (size_t i = 0; i < volMarketDataBlock.size(); i++)
		{
			auto volObjectName = volMarketDataBlock[i][0].getCString();
			auto volObject = getSabrMarketData(volObjectName);
			if (volObject->isATM())
			{
				volObjectAtmName = volObjectName;
				break;
			}
		}
		AQ_REQUIRE(volObjectAtmName.size() > 0, "ATM VOL Market Data Name is required.")

		auto volAtmObject = getSabrMarketData(volObjectAtmName);

		// Populate expiry and tenor as yearFractions
		populateMarketDataExpiryTenorTerms(volAtmObject);

		auto volAtmMat = volAtmObject->getInstrumentQuoteMatrix();
		size_t rowSize = volAtmMat.size();
		size_t columnSize = volAtmMat[0].size();

		//forwards
		DoubleMatrix strikeAtmMat(rowSize, DoubleVector(columnSize, 0.0));

		//numeraire, this is only used when calibration target is premium, default to 1.0
		DoubleMatrix annuityMat(rowSize, DoubleVector(columnSize, 1.0));

		//Allow user to provide Strike MarketData Block, don't throw if empty 
		AQLStringMatrix swapMarketDataMatrix = getLAStringMatrixFromFreeObject(freeObject_, toString(SABR_MODEL_MKTDATA), true /*trimBlankRows*/, false /* throwIfMissing */);

		if (swapMarketDataMatrix.size() == 0)
		{
			bool includeSwapRate = true;
			bool includeAnnuity = (calibrationTarget != SABR_CALIB_VOLATILITY);
			populateATMFSwapRateAndAnnuity(strikeAtmMat, annuityMat, volAtmObject, swapGeneratorName, includeSwapRate, includeAnnuity);
		}
		else
		{
			//If the block is provided by user, get strikeATM and annuity object handle names
			LabelValueBlock swapMarketDataBlock(swapMarketDataMatrix);

			std::string strikeAtmObjectName = swapMarketDataBlock.getCompulsoryValueAsString(SABR_MODEL_PROPERTIES_KEY::ATMF_SWAPRATES);

			//forward
			strikeAtmMat = getSabrMarketData(strikeAtmObjectName)->getInstrumentQuoteMatrix();

			//numeraire
			std::string annuityObjectName = swapMarketDataBlock.getOptionalValueAsString(SABR_MODEL_PROPERTIES_KEY::ANNUITY);
			if (annuityObjectName.empty())
			{
				bool includeSwapRate = false;
				bool includeAnnuity = (calibrationTarget != SABR_CALIB_VOLATILITY);
				populateATMFSwapRateAndAnnuity(strikeAtmMat, annuityMat, volAtmObject, swapGeneratorName, includeSwapRate, includeAnnuity);
			}
			else
			{
				annuityMat = getSabrMarketData(annuityObjectName)->getInstrumentQuoteMatrix();
			}

			//Check size
			AQ_REQUIRE(strikeAtmMat.size() == annuityMat.size() && strikeAtmMat[0].size() == annuityMat[0].size(), "strike matrix and annuity matrix are inconsistent!");
		}

		DoubleVector weights;
		IntVector signs;
		std::vector<DoubleMatrix> volMatrixVector;
		std::vector<DoubleMatrix> strikeMatrixVector;

		auto volMktDataSize = volMarketDataBlock.size();
		weights.reserve(volMktDataSize);
		signs.reserve(volMktDataSize);
		volMatrixVector.reserve(volMktDataSize);
		strikeMatrixVector.reserve(volMktDataSize);
			
		//double totalWeight = 0.0;

		for (size_t i = 0; i < volMarketDataBlock.size(); i++)
		{
			auto volObjectName = volMarketDataBlock[i][0].getCString();

			auto volObject = getSabrMarketData(volObjectName);

			DoubleMatrix volMatrix = volObject->getInstrumentQuoteMatrix();

			//Check size
			AQ_REQUIRE(rowSize == volMatrix.size() && columnSize == volMatrix[0].size(), "The sizes of vol matrix and ATM vol matrix are inconsistent!");

			// If the vol matrix is relative, then change the value to absolute by adding the spread to ATM VOL
			if (volObject->getValyeType() == RELATIVE_SABR_VALUETYPE)
			{
				for (size_t i = 0; i < rowSize; i++)
				{
					for (size_t j = 0; j < columnSize; j++)
					{
						// add the spread to ATM VOL to become absolute value
						volMatrix[i][j] += volAtmMat[i][j];
					}
				}
			}

			//Create a strike matrix from strike_atm and spread
			DoubleMatrix strikeMatrix(strikeAtmMat.size(), DoubleVector(strikeAtmMat[0].size()));

			if (!volObject->isATM())
			{
				const double spread = getSabrMarketData(volObjectName)->getStrikeAdjustAmount();
				for (size_t i = 0; i < strikeAtmMat.size(); i++)
				{
					for (size_t j = 0; j < strikeAtmMat[0].size(); j++)
					{
						strikeMatrix[i][j] = strikeAtmMat[i][j] + spread;
					}
				}
			}
			else
			{
				strikeMatrix = strikeAtmMat;
			}

			// If the weight is zero, don't include the instruments to the calibration
			double weight = StringToNumber<double>(volMarketDataBlock[i][1].getCString());
			if (AQ_IS_EQUAL_ZERO(weight))
			{
				continue;
			}

			//poulate vols and strikes for calibration
			weights.push_back(weight);
			//totalWeight += weight;
			
			volMatrixVector.push_back(volMatrix);
			strikeMatrixVector.push_back(strikeMatrix);

			//sign is not used when the calibration target is vol
			int signValue = (calibrationTarget != SABR_CALIB_VOLATILITY) ? StringToNumber<int>(volMarketDataBlock[i][2].getCString()) : 1.0;
			signs.push_back(signValue);
		}

		//AQ_REQUIRE(AQ_IS_LESS_THAN_OR_EQUAL(totalWeight, 1.0), "Total weights cannot be greater than 100%")

		//Model parameter block: alpha, beta, nu, rho - first column true to calibrate this paramter; second column - initial values.
		std::vector<LabelValueBlock> modelParametersMatrix = etrading::buildMultiLabelValueBlock(getLAStringMatrixFromFreeObject(freeObject_, toString(SABR_MODEL_PARAMETERS)));
		AQ_REQUIRE(modelParametersMatrix.size() == 2, "modelParametersMatrix should have two columns for values.")

		std::vector<bool> calibFlag_bool(4);

		bool calibrateAlpha = modelParametersMatrix[0].getCompulsoryValueAsBool(SABR_MODEL_PROPERTIES_KEY::ALPHA_PARAM);

		//When this flag is true, alpha is implied from ATM Vol and other model parameters.
		if (alphaFromAtmVol)
		{
			// *** The core function expects the calibrateAlpha flag to be FALSE so that Alpha will be implied from atmVols.
			if (calibrateAlpha)
			{
				calibrateAlpha = false;
			}
			//If calibrateAlpha is FALSE, then leave alpha unchanged the the core function, by setting the alphaFromAtmVol to FALSE
			else
			{
				alphaFromAtmVol = false;
			}
		}

		calibFlag_bool[0] = calibrateAlpha;
		calibFlag_bool[1] = modelParametersMatrix[0].getCompulsoryValueAsBool(SABR_MODEL_PROPERTIES_KEY::BETA_PARAM);
		calibFlag_bool[2] = modelParametersMatrix[0].getCompulsoryValueAsBool(SABR_MODEL_PROPERTIES_KEY::NU_PARAM);
		calibFlag_bool[3] = modelParametersMatrix[0].getCompulsoryValueAsBool(SABR_MODEL_PROPERTIES_KEY::RHO_PARAM);

		auto alphaInit = modelParametersMatrix[1].getCompulsoryValueAsString(SABR_MODEL_PROPERTIES_KEY::ALPHA_PARAM);
		auto betaInit = modelParametersMatrix[1].getCompulsoryValueAsString(SABR_MODEL_PROPERTIES_KEY::BETA_PARAM);
		auto nuInit = modelParametersMatrix[1].getCompulsoryValueAsString(SABR_MODEL_PROPERTIES_KEY::NU_PARAM);
		auto rhoInit = modelParametersMatrix[1].getCompulsoryValueAsString(SABR_MODEL_PROPERTIES_KEY::RHO_PARAM);

		DoubleMatrix alphaMat = getSabrParamMatrix(rowSize, columnSize, alphaInit);
		DoubleMatrix betaMat = getSabrParamMatrix(rowSize, columnSize, betaInit);
		DoubleMatrix nuMat = getSabrParamMatrix(rowSize, columnSize, nuInit);
		DoubleMatrix rhoMat = getSabrParamMatrix(rowSize, columnSize, rhoInit);

		// If all calibration flags are false, then create a sabr model with initial alpa, beta, nu, rho paramters
		bool calibrateSabr = (calibFlag_bool[0] || calibFlag_bool[1] || calibFlag_bool[2] || calibFlag_bool[3]);

		if (calibrateSabr)
		{
			AQLString erroMsg;
			AQLMathSwaptionVolUtility::calibrateSABRMatrix(alphaMat,
				betaMat,
				nuMat,
				rhoMat,
				toString(approxMethod_).c_str(),
				calibFlag_bool,
				calibMethod,
				volMatrixVector,
				strikeMatrixVector,
				expiryTermVector_,
				toString(calibrationTarget).c_str(),
				weights,
				signs,
				solverEpsilon,
				strikeAtmMat, //forward
				forwardShift_,
				annuityMat, //numeraire
				volAtmMat,
				alphaFromAtmVol,
				erroMsg,
				0, //Use the default value
				isLognormal_);

			//Throw error if there is any
			AQ_REQUIRE(erroMsg.size() == 0, erroMsg.getCString());
		}
		
		//Save calibrated paramters
		alphaMatrix_ = alphaMat;
		betaMatrix_ = betaMat;
		nuMatrix_ = nuMat;
		rhoMatrix_ = rhoMat;

	}


	/*
	*  @brief  validates the property keys of this SabrModel, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void SabrModel::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for (size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];

			etrading::SabrModelEnum sabrModelEnum = etrading::toSabrModelEnum(propertyName);
			switch (sabrModelEnum)
			{
			case SABR_MODEL_PROPERTIES:
				validateKeysForLVB(model_properties_lvbKeys(), toLabelValueBlock(propertyName).getKeys(), validateKeys, propertyName);
				break;
			case SABR_MODEL_PARAMETERS:
			{
				std::vector<LabelValueBlock> modelParametersMatrix = etrading::buildMultiLabelValueBlock(getLAStringMatrixFromFreeObject(freeObject_, propertyName));
				AQ_REQUIRE(modelParametersMatrix.size() == 2, "modelParametersMatrix should have two columns for values.")
				validateKeysForLVB(model_parameters_lvbKeys(), modelParametersMatrix[0].getKeys(), validateKeys, propertyName);
				break;
			}
			case SABR_MODEL_MKTDATA:
				validateKeysForLVB(model_mktdata_lvbKeys(), toLabelValueBlock(propertyName).getKeys(), validateKeys, propertyName);
				break;
			case SABR_MODEL_VOL_MKTDATA:
				//do nothing
				break;

			default:
				break;
			}
		}

	}


	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject SabrModel::toSchemaObject() const
	{
		SchemaObject schemaObject(SABR_MODEL, getRefToName());
		toSchemaObject(schemaObject);

		return schemaObject;
	}

	// This is a helper method to populate the supplied SchemaObject
	void const SabrModel::toSchemaObject(SchemaObject& schemaObject) const
	{

		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for (size_t i = 0; i < freeObject_.numberOfSchemas(); i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema(propertyName);
			VariantMatrix variantMatrix;
			std::for_each(columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView](const int idx)
			{
				variantMatrix.push_back(allDataView[idx]);
			});

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	VariantMatrix SabrModel::viewInputParameters(const std::string& propertyKey) const
	{
		return viewInputParametersFromFreeObject(freeObject_, propertyKey);
	}

	/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A LabelValue block containing the properties
	*/
	LabelValueBlock SabrModel::toLabelValueBlock(const std::string& propertyKey) const
	{
		AQLStringMatrix stringMatrix = getLAStringMatrixFromFreeObject(freeObject_, propertyKey);
		LabelValueBlock lvb(stringMatrix);

		return lvb;
	}

	double SabrModel::getVol(const std::string& expiry, const std::string& tenor, const double strike, const double forward)
	{
		AQLPriceDataSlidingRule paySlr;
		paySlr.convertFromString(businessDayAdjustment_);

		AQLPriceDataCalendar fixCal;
		fixCal.convertFromString(calendar_);

		double expiryTerm = AQLMathSwaptionVolUtility::getExpiryPoint(expiry, asOfDate_, paySlr, fixCal);
		double tenorTerm = AQLMathSwaptionVolUtility::getTenorPoint(tenor);

		double alpha = getParam(expiryTerm, tenorTerm, ALPHA_SABR);
		double beta = getParam(expiryTerm, tenorTerm, BETA_SABR);
		double rho = getParam(expiryTerm, tenorTerm, RHO_SABR); 
		double nu = getParam(expiryTerm, tenorTerm, NU_SABR);

		double vol = AQLMathSwaptionVolUtility::calcSABRVol(alpha, beta, nu, rho, expiryTerm, strike, forward, forwardShift_, toString(approxMethod_), isLognormal_);

		return vol;

	}

	double SabrModel::getParam(const double expiryTerm, const double tenorTerm, const SabrParamEnum& paramName)
	{
		switch (paramName)
		{
		case ALPHA_SABR:
			return AQLMathSwaptionVolUtility::calcSABRParam(alphaMatrix_, expiryTerm, tenorTerm, expiryTermVector_, tenorTermVector_);
		case BETA_SABR:
			return AQLMathSwaptionVolUtility::calcSABRParam(betaMatrix_, expiryTerm, tenorTerm, expiryTermVector_, tenorTermVector_);
		case RHO_SABR:
			return AQLMathSwaptionVolUtility::calcSABRParam(rhoMatrix_, expiryTerm, tenorTerm, expiryTermVector_, tenorTermVector_);
		case NU_SABR:
			return AQLMathSwaptionVolUtility::calcSABRParam(nuMatrix_, expiryTerm, tenorTerm, expiryTermVector_, tenorTermVector_);
		default:
			AQ_THROW("Only suport parameters: ALPHA, BETA, RHO, NU.");
		}

		return 0;
	}

	double SabrModel::getParam(const std::string& expiry, const std::string& tenor, const SabrParamEnum& paramName)
	{
		AQLPriceDataSlidingRule paySlr;
		paySlr.convertFromString(businessDayAdjustment_);

		AQLPriceDataCalendar fixCal;
		fixCal.convertFromString(calendar_);

		double expiryTerm = AQLMathSwaptionVolUtility::getExpiryPoint(expiry, asOfDate_, paySlr, fixCal);
		double tenorTerm = AQLMathSwaptionVolUtility::getTenorPoint(tenor);

		return getParam(expiryTerm, tenorTerm, paramName);
	}

}
