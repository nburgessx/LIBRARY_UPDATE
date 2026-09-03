#include "FXCurve.h"
#include "ETradingException.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "ScheduleValidation.h"
#include "ParameterValidation.h"
#include "ExceptionMacros.h"
#include "SwapValidation.h"

#include <boost/algorithm/string.hpp>

namespace etrading
{

    FXCurve::FXCurve( const std::string& objName ) : IsLWOObject( objName, FX_CURVE ), curveGenerator_(), curveMarketData_()
    {};

    FXCurve::FXCurve(const std::string& objName, const std::shared_ptr<CurveGenerator>& curveGenerator, const std::shared_ptr<CurveMarketData>& curveMarketData)
        : IsLWOObject(objName, FX_CURVE), curveGenerator_(curveGenerator), curveMarketData_(curveMarketData)
    {}

    FXCurve::FXCurve( const FXCurve& rhs )
        : IsLWOObject( rhs.getRefToName(), FX_CURVE )
    {
		curveGenerator_ = (rhs.curveGenerator_ != nullptr) ? rhs.curveGenerator_->clone() : std::shared_ptr<CurveGenerator>();
		curveMarketData_ = (rhs.curveMarketData_ != nullptr) ? rhs.curveMarketData_->clone() : std::shared_ptr<CurveMarketData>();
    }

  	const SchemaObject FXCurve::toSchemaObject() const
	{
		SchemaObject schemaObject( FX_CURVE, getRefToName());

        curveGenerator_->toSchemaObject(schemaObject);
        curveMarketData_->toSchemaObject(schemaObject);

        return schemaObject;
	}

   std::pair<CCY, CCY> FXCurve::getBaseTermCurrencies() const
   {
		const LabelValueBlock curvePropertiesLVB = curveGenerator_->toLabelValueBlock( toString(CURVE_PROPERTIES) );
  		const std::string currencyPair         = curvePropertiesLVB.getCompulsoryValue( CURVEGENERATOR_CURVEPROPERTIES_KEY::CURRENCY );

		const std::string delimiter = "/";
		std::string baseCurrency = "";
		std::string termCurrency = "";

		if (currencyPair.find(delimiter) != std::string::npos)
		{
			baseCurrency = currencyPair.substr(0, currencyPair.find(delimiter));
			termCurrency = currencyPair.substr(currencyPair.find(delimiter) + 1);
		} 
		else if (currencyPair.size() == 6)
		{
			baseCurrency = currencyPair.substr(0, 3);
			termCurrency = currencyPair.substr(3);
		}
		else
		{
            throw LACoreInvalidData( "#Error: Currency format should be either 'BaseCcy,TermCcy' or 'BaseCcyTermCcy', e.g. 'EUR,USD' or 'EURUSD'" , __FILE__, __LINE__ );
		}

        return std::make_pair(toCCYEnum(baseCurrency), toCCYEnum(termCurrency));
   }

   std::pair<std::string, std::string> FXCurve::getBaseTermDFCurves() const
   {
		const LabelValueBlock curvePropertiesLVB = curveGenerator_->toLabelValueBlock( toString(CURVE_PROPERTIES) );
  		const std::string discountCurves         = curvePropertiesLVB.getCompulsoryValue( CURVEGENERATOR_CURVEPROPERTIES_KEY::DF_CURVE_NAME );

		const std::string delimiter = ",";

		MLIB_REQUIRE( discountCurves.find(delimiter) != std::string::npos, "DfCurveName format should be 'BaseDFCurve,TermDFCurve'" );

		std::string baseDiscountCurve = discountCurves.substr(0, discountCurves.find(delimiter));
		std::string termDiscountCurve = discountCurves.substr(discountCurves.find(delimiter) + 1);

		return std::make_pair(baseDiscountCurve, termDiscountCurve);
   }


   LADate FXCurve::getDate(const LADate& fromDate, const std::string& tenor) const 
    {
	   const LabelValueBlock curveFxConventionsLVB = curveGenerator_->toLabelValueBlock(toString(FXFWD_CONVENTIONS));

	   std::string calendar, businessDayAdjustment, rollDayInput;
	   populateFxFwdConventions(curveFxConventionsLVB, calendar, businessDayAdjustment, rollDayInput);

	   return getDateFromTenorWithFxConvention(fromDate, tenor, calendar, businessDayAdjustment, rollDayInput);

    }

	LADate FXCurve::getAsOfDate() const
	{
        const LabelValueBlock marketDataPropertiesLVB = curveMarketData_->toLabelValueBlock( toString(MARKETDATA_PROPERTIES) );
		const LADate asOfDate = marketDataPropertiesLVB.getCompulsoryValueAsDate( "ASOFDATE" );
		return asOfDate;
	}

    LADate FXCurve::getSpotDate() const
    {
   	    const LabelValueBlock curveFxConventionsLVB = curveGenerator_->toLabelValueBlock( toString(FXFWD_CONVENTIONS) );
  	    const std::string resetLag = curveFxConventionsLVB.getCompulsoryValue( CURVEGENERATOR_FXFWDS_KEY::RESET_LAG );
  	    const LADate spotDate = getDate(getAsOfDate(), resetLag);

        return spotDate;
    }

	

    double FXCurve::getFxPipSize() const
    {
		// FXForwardOutright = FxSpotOutright +/- swapPoints * pipSize 

		const LabelValueBlock marketDataPropertiesLVB = curveMarketData_->toLabelValueBlock( toString(MARKETDATA_PROPERTIES) );

		double pipSize = marketDataPropertiesLVB.getCompulsoryValueAsDouble( "PIPSIZE", "marketDataPropertiesLVB" );

		return pipSize;
    }

    bool FXCurve::isOutright() const
    {
   	    const LabelValueBlock curveFxConventionsLVB = curveGenerator_->toLabelValueBlock( toString(FXFWD_CONVENTIONS) );

		//Default to False: fxFwdRate is in points
  	    bool outright = curveFxConventionsLVB.getOptionalValueAsBool( CURVEGENERATOR_FXFWDS_KEY::IS_FX_OUTRIGHT, false );

        return outright;
    }

   //Get market quoted spot fx rates
   std::pair<double, double> FXCurve::getQuotedSpotFxRates() const
   {
        double spotFxBidRate = 0.0;
        double spotFxAskRate = 0.0;

         // Curve Market Data: Check if using Bid-Ask Data, otherwise use Mid Data
        const bool useFXForwardBidAsk = curveMarketData_->doesKeyExist( toString(FXSPOT_BIDASK_MARKETDATA) );
        const std::string marketDataKey = useFXForwardBidAsk ? toString(FXSPOT_BIDASK_MARKETDATA) : toString(FXSPOT_MARKETDATA);

        // Use Mid for both columns if not using Bid-Ask Data
        // Bid-Ask Format:  Bid, Ask
        // Mid Format:      Foreign Currency, Domestic Currency, Mid
        const size_t bidDataColumn = useFXForwardBidAsk ? 0 : 2;
        const size_t askDataColumn = useFXForwardBidAsk ? 1 : 2;

		LAStringMatrix spotFxRateMatrix = curveMarketData_->toLAStringMatrix( marketDataKey );
		
        for (unsigned int i = 0; i < spotFxRateMatrix.size(); ++i)
	    {
            if ( useFXForwardBidAsk )
            {
                //*** Expected input Columns: Bid, Ask
                MLIB_REQUIRE( spotFxRateMatrix[i].size() == 2, "Spot FX Bid-Ask market data must have 2 columns: Bid, Ask" );
            }
            else
            {
                //*** Expected input Columns: Foreign Currency, Domestic Currency, Mid
                MLIB_REQUIRE( spotFxRateMatrix[i].size() == 3, "Spot FX Mid market data must have 3 columns: Foreign Currency, Domestic Currency, Mid" );
            }

			spotFxBidRate = spotFxRateMatrix[i][bidDataColumn].getDoubleValue();
    	    spotFxAskRate = spotFxRateMatrix[i][askDataColumn].getDoubleValue();
	    }

		return std::make_pair(spotFxBidRate, spotFxAskRate);
   }

    //Get market quoted forward fx rates, return a map with settleDate as key, and fxFw points and outrights as value
    std::map<std::string, std::vector<FxFwd>> FXCurve::getQuotedForwardFxRates() const
    {
        // Curve Market Data: Check if using Bid-Ask Data, otherwise use Mid Data
        const bool useFXForwardBidAsk = curveMarketData_->doesKeyExist( toString(FXFWD_BIDASK_MARKETDATA) );
        const std::string marketDataKey = useFXForwardBidAsk ? toString(FXFWD_BIDASK_MARKETDATA) : toString(FXFWD_MARKETDATA);

        // Use Mid for both columns if not using Bid-Ask Data
        // Bid-Ask Format:  Term, Bid, Ask, Use
        // Mid Format:      Term, Mid, Use
        const size_t tenorDataColumn = 0;
        const size_t bidDataColumn = 1;
        const size_t askDataColumn = useFXForwardBidAsk ? 2 : 1;

        LAStringMatrix fwdFxRateMatrix = curveMarketData_->toLAStringMatrix( marketDataKey );

        std::map<std::string, std::vector<FxFwd>> fwdFxRateMap;

		const LADate asOfDate = getAsOfDate();
		const LADate spotDate = getSpotDate();

		auto spotFxRates = getQuotedSpotFxRates();

		FxSpotRate fxSpot;
		fxSpot.bid = std::get<0>(spotFxRates);
		fxSpot.ask = std::get<1>(spotFxRates);
		fxSpot.setMidFromBidAsk();
		
        double pipSize = getFxPipSize();
		bool isRateOutright = isOutright();

		std::string calendar, businessDayAdjustment, rollDayInput;
		populateFxFwdConventions(curveGenerator_->toLabelValueBlock(toString(FXFWD_CONVENTIONS)), calendar, businessDayAdjustment, rollDayInput);

		for (unsigned int i = 0; i < fwdFxRateMatrix.size();i++)
	    {
            if (useFXForwardBidAsk)
            {
                //*** Expected input Columns: Term, Bid, Ask, Use (Optional)
                MLIB_REQUIRE(fwdFxRateMatrix[i].size() == 3 || fwdFxRateMatrix[i].size() == 4, "Forward FX market data matrix must have 3-4 columns: Term, Bid, Ask, Use (Optional)");
            }
            else
            {
                //*** Expected input Columns: Term, Mid, Use (Optional)
                MLIB_REQUIRE(fwdFxRateMatrix[i].size() == 2 || fwdFxRateMatrix[i].size() == 3, "Forward FX market data matrix must have 2-3 columns: Term, Mid, Use (Optional)");
            }

			std::string tenor = fwdFxRateMatrix[i][tenorDataColumn].getCString();

			FxFwd fxFwd;

			fxFwd.isOutright = isRateOutright;
			fxFwd.bid = fwdFxRateMatrix[i][bidDataColumn].getDoubleValue();
			fxFwd.ask = fwdFxRateMatrix[i][askDataColumn].getDoubleValue();
			fxFwd.setMidFromBidAsk();

			auto settleDate = fromSettleDateOrTenorToDate(tenor, asOfDate, spotDate, calendar, businessDayAdjustment, rollDayInput);
			bool settleEarly = isSettleEarlierThanSpotDate(tenor, settleDate, spotDate);

			auto fxFwds = populateFxForwardPointsAndOutrights(fxSpot, fxFwd, pipSize, settleEarly);

			fwdFxRateMap[tenor] = fxFwds;
	    }

        return fwdFxRateMap;
    }

	//Get fxForwards for a given settleDate from the interpolated fxCurve
	FxFwd FXCurve::interpolateFxForwardOutrightsFromFxCurve(const LADate& settlementDate, const std::map<LADate, std::vector<FxFwd>>& fwdFxRateMap, const std::string& interpolationMethod)
    {
        
        std::vector<double> settleDates;
        std::vector<double> fwdFxBidOutrights;
        std::vector<double> fwdFxAskOutrights;

        for(auto it = fwdFxRateMap.begin(); it != fwdFxRateMap.end(); ++it) 
		{
			settleDates.push_back(fromLADateToDouble(it->first));
			auto fxFwdRate = it->second[1];
			fwdFxBidOutrights.push_back(fxFwdRate.bid);
			fwdFxAskOutrights.push_back(fxFwdRate.ask);
        }

        auto settleDateInDouble = fromLADateToDouble(settlementDate);
		auto interpMethod = toInterpolationEnum(interpolationMethod);

		FxFwd fwdFxRatesInterp;

		fwdFxRatesInterp.isOutright = true;
		fwdFxRatesInterp.bid = interpolate(settleDates, fwdFxBidOutrights, settleDateInDouble, interpMethod);
		fwdFxRatesInterp.ask = interpolate(settleDates, fwdFxAskOutrights, settleDateInDouble, interpMethod);
		fwdFxRatesInterp.setMidFromBidAsk();


        return fwdFxRatesInterp;
    }


   //Imply fxForwards for a list of settlementDatesOrTenors from base/term discount curves
	std::vector<std::vector<FxFwd>> FXCurve::getFxForwardsFromDiscountCurves(const std::vector<std::string>& settlementDatesOrTenors, const std::string& baseCurveCollection, const std::string& termCurveCollection)
    {

		std::string calendar, businessDayAdjustment, rollDayInput;
		populateFxFwdConventions(curveGenerator_->toLabelValueBlock(toString(FXFWD_CONVENTIONS)), calendar, businessDayAdjustment, rollDayInput);

		double pipSize = getFxPipSize();

		CurveFxFwdDataProvider fxInfo;

		fxInfo.asOfDate = getAsOfDate();
		fxInfo.spotDate = getSpotDate();

		fxInfo.calendar = calendar;
		fxInfo.businessDayAdjustment = businessDayAdjustment;
		fxInfo.rollDayInput = rollDayInput;

		fxInfo.dayCount = NONE_DAYCOUNT;
		fxInfo.pipSize = getFxPipSize();

		FxSpotRate fxSpot;
		auto spotFxRates = getQuotedSpotFxRates();
		fxSpot.bid = std::get<0>(spotFxRates);
		fxSpot.ask = std::get<1>(spotFxRates);
		fxSpot.setMidFromBidAsk();
		fxInfo.fxSpot = fxSpot;

		fxInfo.baseCurveCollection = baseCurveCollection;
		fxInfo.termCurveCollection = termCurveCollection;
		auto dfCurves = getBaseTermDFCurves();
		fxInfo.baseDiscountCurveIndex = std::get<0>(dfCurves);
		fxInfo.termDiscountCurveIndex = std::get<1>(dfCurves);

		return calculateFxForwardsFromDiscountCurves(settlementDatesOrTenors, fxInfo, 0.0);

   }

    //Get fxForwards for a list of settlementDatesOrTenors from the interpolated fxCurve
	std::vector<std::vector<FxFwd>> FXCurve::getFxForwardsFromFxCurve(const std::vector<std::string>& settlementDatesOrTenors)
    {

        const std::map<std::string, std::vector<FxFwd>> fwdFxRateMap = getQuotedForwardFxRates();
		const LabelValueBlock curvePropertiesLVB = curveGenerator_->toLabelValueBlock( toString(CURVE_PROPERTIES) );
  		const std::string interpolationMethod    = curvePropertiesLVB.getOptionalValue( CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION, toString(LINEAR_INTERPOLATION));

		FxSpotRate fxSpot;
		auto spotFxRates = getQuotedSpotFxRates();
		fxSpot.bid = std::get<0>(spotFxRates);
		fxSpot.ask = std::get<1>(spotFxRates);
		fxSpot.setMidFromBidAsk();

        double pipSize = getFxPipSize();
        
		const LADate asOfDate = getAsOfDate();
  		const LADate spotDate = getSpotDate();

		std::string calendar, businessDayAdjustment, rollDayInput;
		populateFxFwdConventions(curveGenerator_->toLabelValueBlock(toString(FXFWD_CONVENTIONS)), calendar, businessDayAdjustment, rollDayInput);

		// Populate quoteFxRateMap using date as the key
        std::map<LADate, std::vector<FxFwd>> fwdFxRateMapWithDateAsKeys;
        for(auto it = fwdFxRateMap.begin(); it != fwdFxRateMap.end(); ++it) 
		{
			auto settleDate = fromSettleDateOrTenorToDate(it->first, asOfDate, spotDate, calendar, businessDayAdjustment, rollDayInput);
			fwdFxRateMapWithDateAsKeys[settleDate] = it->second;
        }

		//Calculate FX Forwards
		const LADate latestQuotedSettleDate  = fwdFxRateMapWithDateAsKeys.rbegin()->first;

		bool isSettleTenor = isMaturityDateTenor(settlementDatesOrTenors.at(0).c_str());

		size_t expectedSize = settlementDatesOrTenors.size();

		std::vector<std::vector<FxFwd>> allForwardFXs (expectedSize);

        for (size_t i = 0; i < expectedSize; ++i)
        {        
	        std::string settleOrTenor = settlementDatesOrTenors[i];
			LADate settleDate = fromSettleDateOrTenorToDate(settleOrTenor, asOfDate, spotDate, calendar, businessDayAdjustment, rollDayInput);

			// When settle is tenor and the tenor is in the quoted fxfwd rates, return the fxfwd rates directly
			if (isSettleTenor && fwdFxRateMap.find(settleOrTenor) != fwdFxRateMap.end())
			{
				allForwardFXs[i] = fwdFxRateMap.at(settleOrTenor);
				continue;
			} 
			// When settle is date and the date is in the quoted fxfwd rates, return the fxfwd rates directly
			else if (!isSettleTenor && (settleDate != spotDate) && fwdFxRateMapWithDateAsKeys.find(settleDate) != fwdFxRateMapWithDateAsKeys.end())
			{
				allForwardFXs[i] = fwdFxRateMapWithDateAsKeys.at(settleDate);
				continue;

			}
			// Otherwise, interpolate the fxfwd rates via settleDate
			else
			{
				//We do not extrapolate, throw an error instead
				if (settleDate > latestQuotedSettleDate)
				{
					throw ETradingException(( boost::format( "#Error: Extrapolation not supported: settleDate '%s' is later than the largest quoted settle date '%s'." ) % settleDate % latestQuotedSettleDate).str()  );
				}

				FxFwd fxFwdRate;

				// If settle date is spotDate, return the SP rates
				fxFwdRate.isOutright = true;
				fxFwdRate.bid = fxSpot.bid;
				fxFwdRate.ask = fxSpot.ask;

				//Interpolate when settle date is not spot date
				if (settleDate != spotDate)
				{
					auto  fwdFxRatesInterp = interpolateFxForwardOutrightsFromFxCurve(settleDate, fwdFxRateMapWithDateAsKeys, interpolationMethod);
					fxFwdRate.bid = fwdFxRatesInterp.bid;
					fxFwdRate.ask = fwdFxRatesInterp.ask;
				}

				fxFwdRate.setMidFromBidAsk();

				bool settleEarly = isSettleEarlierThanSpotDate(settleOrTenor, settleDate, spotDate);

				//The interpolated rate is outrights
				std::vector<FxFwd> forwardFXs = populateFxForwardPointsAndOutrights(fxSpot, fxFwdRate, pipSize, settleEarly);

				allForwardFXs[i] = forwardFXs;
			}

        }

		return allForwardFXs;

	}


    //Get fxSwap based on the nearLeg/farLeg settleDatesOrTenors (which are implied from base/term discount curves)
	std::vector<std::vector<FxFwd>> FXCurve::getFxSwapFromDiscountCurves(const std::vector<std::string>& nearLegSettlementDatesOrTenors,
																		const std::vector<std::string>& farLegSettlementDatesOrTenors, 
																		const std::string& baseCurveCollection, 
																		const std::string& termCurveCollection, 
																		bool outputFarLeg)
	{
		std::vector<std::vector<FxFwd>> nearLegFwdFxBidAskMatrix;
		std::vector<std::vector<FxFwd>> farLegFwdFxBidAskMatrix;
		if (outputFarLeg)
		{
			nearLegFwdFxBidAskMatrix = getFxForwardsFromDiscountCurves(nearLegSettlementDatesOrTenors, baseCurveCollection, termCurveCollection);
			farLegFwdFxBidAskMatrix = getFxForwardsFromDiscountCurves(farLegSettlementDatesOrTenors, baseCurveCollection, termCurveCollection);
		}
		else
		{
 			nearLegFwdFxBidAskMatrix = getFxForwardsFromDiscountCurves(nearLegSettlementDatesOrTenors, baseCurveCollection, termCurveCollection);
		}

		auto forwardForwardFxRates = populateFxForwardForwards(nearLegFwdFxBidAskMatrix, farLegFwdFxBidAskMatrix, getFxPipSize());

		return forwardForwardFxRates;

	}

	//Get fxSwaps from the interpolated fxCurve based on the nearLeg/farLeg settleDateOrTenor
	std::vector<std::vector<FxFwd>> FXCurve::getFxSwapFromFxCurve(const std::vector<std::string>& nearLegSettlementDatesOrTenors, const std::vector<std::string>& farLegSettlementDatesOrTenors, bool outputFarLeg)
    {

		std::vector<std::vector<FxFwd>> nearLegFwdFxBidAskMatrix;
		std::vector<std::vector<FxFwd>> farLegFwdFxBidAskMatrix;
		if (outputFarLeg)
		{
			nearLegFwdFxBidAskMatrix = getFxForwardsFromFxCurve(nearLegSettlementDatesOrTenors);
			farLegFwdFxBidAskMatrix = getFxForwardsFromFxCurve(farLegSettlementDatesOrTenors);

		}
		else
		{
			nearLegFwdFxBidAskMatrix = getFxForwardsFromFxCurve(nearLegSettlementDatesOrTenors);
		}

		auto forwardForwardFxRates = populateFxForwardForwards(nearLegFwdFxBidAskMatrix, farLegFwdFxBidAskMatrix, getFxPipSize());

		return forwardForwardFxRates;
		
    }


}
