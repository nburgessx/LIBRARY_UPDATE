#pragma once

#include "IsAQObject.h"
#include "SchemaObject.h"

#include "CoreEnumerations.h"
#include "CurveGenerator.h"
#include "CurveMarketData.h"
#include "FXCurveUtilities.h"

namespace etrading
{

    class FXCurve :	public IsAQObject
    {
    public:

        FXCurve( const std::string& objName );
        FXCurve(const std::string& objName, const std::shared_ptr<CurveGenerator>& curveGenerator, const std::shared_ptr<CurveMarketData>& curveMarketData);

        FXCurve( const FXCurve& rhs); 
        virtual ~FXCurve(){}; 

        //Get market quoted spot fx rates
		std::pair<double, double> getQuotedSpotFxRates() const;

        //Get market quoted forward fx rates, return a map with ternor as key, and fwdFxRate as value
        std::map<std::string, std::vector<FxFwd>> getQuotedForwardFxRates() const;

        //Imply fxForwards for a list of settlementDatesOrTenors from base/term discount curves
		std::vector<std::vector<FxFwd>> getFxForwardsFromDiscountCurves(const std::vector<std::string>& settlementDatesOrTenors,
																		const std::string& baseCurveCollection, 
																		const std::string& termCurveCollection);

		//Get fxForwards for a list of settleDates from the interpolated fxCurve
         std::vector<std::vector<FxFwd>> getFxForwardsFromFxCurve(const std::vector<std::string>& settlementDatesOrTenors);

	    //Get fxSwap based on the nearLeg/farLeg settleDatesOrTenors (which are implied from base/term discount curves)
		 std::vector<std::vector<FxFwd>> getFxSwapFromDiscountCurves(const std::vector<std::string>& nearLegSettlementDatesOrTenors,
																	const std::vector<std::string>& farLegSettlementDatesOrTenors, 
																	const std::string& baseCurveCollection, 
																	const std::string& termCurveCollection, 
																	bool outputFarLeg);

        //Get fxSwaps from the interpolated fxCurve based on the nearLeg/farLeg settleDateOrTenor
		 std::vector<std::vector<FxFwd>> getFxSwapFromFxCurve(const std::vector<std::string>& nearLegSettlementDatesOrTenors,
															const std::vector<std::string>& farLegSettlementDatesOrTenors,
															bool outputFarLeg);

    private:

        const SchemaObject toSchemaObject() const;

        //Helper functions
        std::pair<CCY, CCY> getBaseTermCurrencies() const;
        std::pair<std::string, std::string> getBaseTermDFCurves() const;
		AQLDate getDate(const AQLDate& fromDate, const std::string& tenor) const; 
		AQLDate getAsOfDate() const;
		AQLDate getSpotDate() const;

		double getFxPipSize() const;
		bool isOutright() const;

        //Get fxForwards for a given settleDate from the interpolated fxCurve
		FxFwd interpolateFxForwardOutrightsFromFxCurve(const AQLDate& settlementDate, const std::map<AQLDate, std::vector<FxFwd>>& fwdFxRateMap, const std::string& interpolationMethod);

        std::shared_ptr<CurveGenerator> curveGenerator_;
        std::shared_ptr<CurveMarketData> curveMarketData_;

    };

	typedef std::shared_ptr< FXCurve > FXCurvePtr;





}

