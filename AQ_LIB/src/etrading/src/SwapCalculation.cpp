#include "SwapCalculation.h"
#include "ParameterValidation.h"
#include "LACurvePricingObject.h"
#include "LADateScheduleHelpers.h"
#include "Solvers.h"
#include "SettingsValidation.h"
#include "FXCurveUtilities.h"

namespace etrading
{


	//Helper method to set fixed leg's fixedRate
	void setSwapLegFixedRate(DataProvider& dataProviderFixedLeg, double fixedRate)
	{
		dataProviderFixedLeg.setCompoundRateOverride(fixedRate);
	}

    //Helper method to set float leg's spread/ fixed leg's fixedRate
    void setSwapLegSpreadOrFixedRate(DataProvider& dataProviderSpreadLeg, const ScheduleTypeEnum& legType, double spread)
    {
        if(isFloatLeg(legType))
        {
			dataProviderSpreadLeg.setFloatSpreadOverride(spread);
        }
        else if (isFixedLeg(legType))
        {
            double oneBasisPoint = 0.0001;
            auto fixedRate = spread * oneBasisPoint;

			setSwapLegFixedRate(dataProviderSpreadLeg, fixedRate);
		}
    }

   double getSpreadFromLeg(const LegPtr& leg)
    {
		double spread = 0.0;

		if(isFloatLeg(leg->getType()))
        {
            spread = leg->getSchedule()->getSpread();
        }
        else if (isFixedLeg(leg->getType()))
        {
            spread = leg->getSchedule()->getFixedRate() * 10000;
        }

		return spread;
    }

    std::pair<bool, double> solveSpread ( DataProvider& dataProviderSpreadLeg, double pv, double pvRefLeg, double deltaPV, double spread, double epsilonForPV, const LegPtr& spreadLeg)
    {
            double epsilonForDelta = 1e-20;
            int maxLoop = 200;
            int count = 0;
            bool solutionFound = false;
            
            while (std::fabs(pv) > epsilonForPV && count < maxLoop)
            {
                double oldPV = pv;
                if (std::fabs(deltaPV) < epsilonForDelta)
                {
                    break;
                }
                auto oldSpread = spread;
                spread = spread - oldPV/deltaPV;

                //Calculate deltaPV: dpv(s) = (pv(s')-pv(s))/(s'-s)
                
                setSwapLegSpreadOrFixedRate(dataProviderSpreadLeg, spreadLeg->getType(), spread);

                auto pvSpreadLeg = spreadLeg->pv(dataProviderSpreadLeg, false /* nativeCurrency */, false /* updateCurveData */);
                pv = pvSpreadLeg + pvRefLeg; 

                if (std::fabs(pv) <= epsilonForPV)
                {
                    //solution found
                    solutionFound = true;
                    break;
                }
                auto spreadDiff = spread - oldSpread;
                if (std::fabs(spreadDiff) < epsilonForDelta)
                {
                    break;
                }
                deltaPV = (pv - oldPV)/spreadDiff;
                count++;
            }

           return std::make_pair(solutionFound, spread);
    }

    std::pair<LegCollection, LegPtr> getBasisRefSpreadLegPair(const LegCollection& legs, const LAString& spreadLegName)
    {

        auto leg1 = legs.get(0);
        auto leg2 = legs.get(1);

        LegPtr spreadLeg;

        if (spreadLegName.size() != 0)
        {
            spreadLeg = legs.findLegByName(spreadLegName);

            if (spreadLeg == nullptr)
            {
			    throw LACoreInvalidData( "#Error: Invalid Leg Name: The specified leg name does not match any of the name of the Swap Leg", __FILE__, __LINE__ );
            }
        }
        else if (leg1->getType() == FIXED_SCHEDULE_TYPE && leg2->getType() == FLOAT_SCHEDULE_TYPE )
        {
            spreadLeg = leg2;
        }
        else
        {
            bool isLeg1TenorLowerOrSame           = (getFrequencyNumber(leg1->getSchedule()->getAccrualFrequency()) <= getFrequencyNumber(leg2->getSchedule()->getAccrualFrequency()));

            spreadLeg = isLeg1TenorLowerOrSame ? leg1 : leg2;
        }

        LegCollection refLegs = legs;
        refLegs.remove(spreadLeg);

        return std::make_pair(refLegs, spreadLeg);
    }

    std::pair<LegCollection, LegPtr> getXccyBasisRefSpreadLegPair(const LegCollection& legs, const LAString& notionalResetLegName, const LAString& spreadLegName)
    {

        auto leg1 = legs.get(0);
        auto leg2 = legs.get(1);

        LegPtr spreadLeg;
        if (spreadLegName.size() != 0)
        {
            spreadLeg = legs.findLegByName(spreadLegName);
            if (spreadLeg == nullptr)
            {
			    throw LACoreInvalidData( "#Error: Invalid Leg Name: The specified leg name does not match any of the Swap Legs", __FILE__, __LINE__ );
            }
        }
        else if (leg1->getType() == FIXED_SCHEDULE_TYPE && leg2->getType() == FLOAT_SCHEDULE_TYPE )
        {
            spreadLeg = leg2;
        }
        else
        {
            // Find the spread leg from the first two legs of the swap:

            //The spread is applied to the non-USD leg
		    if ( leg1->getStaticData()->getCurrency() == USD )
		    {
			    spreadLeg = leg2;
		    }
		    else if ( leg2->getStaticData()->getCurrency() == USD )
		    {
			    spreadLeg = leg1;
		    }
            //If none of the first two legs is USD, the spread is applied to the leg with same name as notionalResetLegName
		    else if (notionalResetLegName.size()!=0)
		    {
                if (same(leg1->getLegName(), notionalResetLegName))
			    {
				    spreadLeg = leg2;
			    }
			    else if (same(leg2->getLegName(), notionalResetLegName))
			    {
				    spreadLeg = leg1;
			    }
		    }
		    else
		    {
			    throw LACoreInvalidData( "#Error: Missing Leg Name: Please specified a leg name, so that the spread can be applied to", __FILE__, __LINE__ );
		    }
        }

        LegCollection refLegs = legs;
        refLegs.remove(spreadLeg);

        return std::make_pair(refLegs, spreadLeg);
    }

    double calculateSwapSpread(const LegCollection& refLegs, const LegPtr& spreadLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread)
	{
        double spread = 0.0;

		DataProvider dataProviderSpreadLeg(ValuationSettings(valuationSettingsLVB, fixingTableNames, spreadLeg->getLegName()));

		//Always set the spread to 0 for the calculation, if it is not a parSpread function the original spread will be added back in the end 
		setSwapLegSpreadOrFixedRate(dataProviderSpreadLeg, spreadLeg->getType(), 0);

		auto pvSpreadLeg = spreadLeg->pv( dataProviderSpreadLeg);

        auto pvRefLeg = refLegs.pv(valuationSettingsLVB, fixingTableNames);

        auto pv = pvSpreadLeg + pvRefLeg; 

        // *** IMPORTANT *** The trade notional is not unity. Therefore we must normalize the tolerance by notional. 
        double normalizationFactor = std::fabs( spreadLeg->getSchedule()->getNotional() );
        const double epsilonForPv = 1e-8 * normalizationFactor;
        
        if (std::fabs(pv) <= epsilonForPv)
        {
            // If the PV is smaller enough (close to zero), then we don't solve the spread
            return spread;
        }

        // Spread formula: pvSpreadLegWithZeroSpread + pvSpreadLegSpread + pvRefLeg = 0 => pvSpreadLegSpread = -1 * (pvSpreadLegWithZeroSpread + pvRefLeg) 
    
        // For fixed spread leg, the spread is actually the fixedRate, so the pv with zero fixedRate is the exchangeNotionalPV
        auto pvSpreadLegWithZeroSpread = 0.0;
        if (spreadLeg->getType() == FIXED_SCHEDULE_TYPE)
        {
            double spreadLegExchangeNotionalPV = 0.0;
            if (spreadLeg->getSchedule()->getNotionalExchangeEnum() != NONE_NE)
            {
                //get all the cashflows including the upfrontCashflow
                auto cashflows = spreadLeg->getSchedule()->getAllCashflows();

                for( size_t i = 0; i < cashflows.size(); i++ )
		        {			
			        auto cf = cashflows[i];
			        spreadLegExchangeNotionalPV += cf->getNotionalExchangePv( dataProviderSpreadLeg.getCashflowDataIncludingUpfront( i ) );
		        }
            }

            pvSpreadLegWithZeroSpread = spreadLegExchangeNotionalPV;
        }
        else
        {
            // pv function already includes the exchangeNotionalPV
            pvSpreadLegWithZeroSpread = spreadLeg->pv( dataProviderSpreadLeg);
        }

        auto pvWithZeroSpread = pvSpreadLegWithZeroSpread + pvRefLeg; 

        double annuityWithSign  = 0; 

		const LADate valuationDate = dataProviderSpreadLeg.getValuationSettings().getValuationDate();

		// If there is accruedInterest, needs to user solver
		bool noAccruedInterestRefLegs = true;
		for (size_t i=0; i < refLegs.size() ; ++i)
        {
            auto refLeg = refLegs.get(i);
            bool accruedInterest = refLeg->getSchedule()->hasAccruedInterest(valuationDate);
			if(accruedInterest)
			{
				noAccruedInterestRefLegs = false;
				break;
			}
        }
		bool noAccruedInterest = !spreadLeg->getSchedule()->hasAccruedInterest(valuationDate) && noAccruedInterestRefLegs;


		if (spreadLeg->getSchedule()->getBespokeScheduleType() != BESPOKE_SCHEDULE 
			&& !spreadLeg->getSchedule()->isAccrualFreqLessThanPaymentFreq() 
			&& !spreadLeg->getSchedule()->isPaymentFreqEnumAtMaturity() 
			&& noAccruedInterest)
        {  
            //Note: this is the case for EUR and GBP tenor basis swap, e.g. 3M leg pay Quartly
            annuityWithSign = spreadLeg->annuityWithNotional(dataProviderSpreadLeg) * spreadLeg->getSchedule()->getPayRecIndicator();
            if (annuityWithSign == 0)
		    {
			    throw LACoreInvalidData( "#Error: Zero Notional or PV: Cannot calculate spread since the notional is zero or one of the legs has zero PV.", __FILE__, __LINE__ );
		    }
            // Spread will be added to the LOWER tenor leg, in basis points
            spread = -1.0 * pvWithZeroSpread / annuityWithSign * 10000;

        }
        else
        {
            //Note: this is the case for USD tenor basis swap, e.h. 3M leg pay Semi-annually

            // First we will use the spread to the ref leg, as thes starting point for the solver to calculate the spread of spread leg
            auto firstRefLeg = refLegs.get(0);

			DataProvider dataProviderFirstRefLeg(ValuationSettings(valuationSettingsLVB, {}, firstRefLeg->getLegName()));

			annuityWithSign = firstRefLeg->annuityWithNotional(dataProviderFirstRefLeg) * firstRefLeg->getSchedule()->getPayRecIndicator();
            
            if (annuityWithSign == 0)
		    {
			    throw LACoreInvalidData( "#Error: Zero Notional or PV: Cannot calculate spread since the notional is zero or one of the legs has zero PV.", __FILE__, __LINE__ );
		    }
            //spread of the higher tenor leg, in basis points
            spread = -1.0 * pvWithZeroSpread / annuityWithSign * 10000;


            //1) Use the spread of the ref leg (with the opposite sign) as the starting point for the spread leg's spread
            if(spreadLeg->getSchedule()->getPayRecIndicator() != firstRefLeg->getSchedule()->getPayRecIndicator())
            {
                spread *= -1;
            }

            //2) Use Secant method to find the spread of lower tenor leg
            double ds = 0.0005; 
            
            setSwapLegSpreadOrFixedRate(dataProviderSpreadLeg, spreadLeg->getType(), spread - ds);

            auto pv0 = spreadLeg->pv( dataProviderSpreadLeg, false/* nativeCurrency */, false/* updateCurveData */ ) + pvRefLeg;
            
            setSwapLegSpreadOrFixedRate(dataProviderSpreadLeg, spreadLeg->getType(), spread + ds);

            auto pv1 = spreadLeg->pv( dataProviderSpreadLeg, false/* nativeCurrency */, false/* updateCurveData */ ) + pvRefLeg;

            auto deltaPV = (pv1-pv0)/(2.0*ds);

            // Solve For the Spread; Using a tolerance that has been normalized by the trade notional
            // ------------------------------------------------------
            auto result = solveSpread ( dataProviderSpreadLeg, pv, pvRefLeg, deltaPV, spread, epsilonForPv, spreadLeg);
            bool solutionFound = result.first;
            AQ_REQUIRE( solutionFound, "Unable to calculate the swap spread. Solver failed to converge to a solution.")
            spread = result.second;
            // ------------------------------------------------------
        }

        //When it is par spread, the calculation will not take the spread leg's existing spread into acount
		if (!isParSpread)
        {
			double originalSpread = getSpreadFromLeg(spreadLeg);
			spread -= originalSpread;
        }

        return spread;	
    }

    double calculateXccySwapSpread(const LegCollection& legs, const LAString& spreadLegName, const LAString& notionalResetLegName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread)
    {
        double spread = 0;

        auto refSpreadPair = getXccyBasisRefSpreadLegPair(legs, notionalResetLegName, spreadLegName);
        auto refLegs = refSpreadPair.first;
        auto spreadLeg = refSpreadPair.second;

		bool validFirstRefLegType = (refLegs.get(0)->getType() == FIXED_SCHEDULE_TYPE || refLegs.get(0)->getType() == FLOAT_SCHEDULE_TYPE);
		bool validSpreadLegType = (spreadLeg->getType() == FIXED_SCHEDULE_TYPE || spreadLeg->getType() == FLOAT_SCHEDULE_TYPE);

		AQ_REQUIRE(validFirstRefLegType &&  validSpreadLegType, "#Spread function supports a Swap with Fixed or Float legs.");
		
		spread = calculateSwapSpread(refLegs, spreadLeg, valuationSettingsLVB, fixingTableNames, isParSpread);

        return spread;	
    }

    double calculateBasisSwapSpread(const LegCollection& legs, const LAString& spreadLegName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread)
    {

        auto refSpreadPair = getBasisRefSpreadLegPair(legs, spreadLegName);

        auto refLegs = refSpreadPair.first;
        LegPtr spreadLeg = refSpreadPair.second;

		bool validFirstRefLegType = (refLegs.get(0)->getType() == FIXED_SCHEDULE_TYPE || refLegs.get(0)->getType() == FLOAT_SCHEDULE_TYPE);
		bool validSpreadLegType = (spreadLeg->getType() == FIXED_SCHEDULE_TYPE || spreadLeg->getType() == FLOAT_SCHEDULE_TYPE);

		AQ_REQUIRE(validFirstRefLegType &&  validSpreadLegType, "#Spread function supports a Swap with Fixed or Float legs.");

        double spread = calculateSwapSpread(refLegs, spreadLeg, valuationSettingsLVB, fixingTableNames, isParSpread); 
    
        return spread;	

    }

	/* @brief Calculate AsOfDateFxRate
	*
	* @param[in]	valuationSettingsLVB		valuationSettingsLVB
	* @param[in]	valuationCurrency			valuation currency chosed by user
	* @param[in]	leg1						First leg
	* @param[in]	leg2						Second leg
	* @returns		The asOfDateFxRate
	*/
	double calculateAsOfDateFxRate(const LabelValueBlock& valuationSettingsLVB, const CCY& valuationCurrency, const LegPtr& leg1, const LegPtr& leg2)
	{

		double fxAsOfDate = getFXAsOfDateFromValuationSettings(valuationSettingsLVB, false);

		//Use FX_AS_OF_DATE if it is specified by user
		if (!std::isnan(fxAsOfDate))
		{
			return fxAsOfDate;
		}

		//Otherwise, calculate FxAsOfDateRate from FxSpot
		
		LegPtr valCcyLeg;
		LegPtr nonValCcyLeg;
		if (valuationCurrency == leg1->getStaticData()->getCurrency())
		{
			valCcyLeg = leg1;
			nonValCcyLeg = leg2;
		}
		else if (valuationCurrency == leg2->getStaticData()->getCurrency())
		{
			valCcyLeg = leg2;
			nonValCcyLeg = leg1;
		}
		else
		{
			AQ_THROW("Valuation Currency must be one of the leg currency of the swap's first two legs.");
		}

		LAString valCcyLegCurveCollection = getLWOCurveCollectionFromValuationSettings(valuationSettingsLVB, valCcyLeg->getLegName());
		LAString nonValCcyLegCurveCollection = getLWOCurveCollectionFromValuationSettings(valuationSettingsLVB, nonValCcyLeg->getLegName());

	   // Both legs should be nased on the same asOfDate
		LADate asOfDate = getCurveAsOfDate(nonValCcyLegCurveCollection);

		// fxSpotDate = asOfDate + 1D(TGT) + 1D(TGT:NY)
		// get the spotDate of leg1
		LADate fxSpotDate = etrading::getDateFromTenor(boost::assign::list_of(asOfDate), "1D",
													toString(nonValCcyLeg->getSchedule()->getAccrualbusinessDayAdj()).c_str(),
													nonValCcyLeg->getSchedule()->getAccrualCalendar(),
													getRollConvection(nonValCcyLeg->getSchedule()->getRollDayInput()))[0];

		// update the the spotDate of leg2
		fxSpotDate = etrading::getDateFromTenor(boost::assign::list_of(fxSpotDate), "1D",
												toString(valCcyLeg->getSchedule()->getFxFixingBusinessDayAdj()).c_str(),
												valCcyLeg->getSchedule()->getFxFixingCalendar(),
												getRollConvection(valCcyLeg->getSchedule()->getRollDayInput()))[0];


		// FX Spot Rate is Mandatory for Xccy Swap, from non-Valuation Ccy to valuation Ccy
		const double spotFxRate = getFXSpotFromValuationSettings(valuationSettingsLVB, true); // true = Mandatory and Throw if missing
		
		//Note that we expect fxSpot from Val setting is always from non-Val cccy to val ccy, so non-val ccy is baseCcy, val ccy is termCcy
		const double asOfDateFxRate = fxRateFromSpotToAsOfDate(spotFxRate, fxSpotDate, nonValCcyLegCurveCollection.getCString(), nonValCcyLeg->getStaticData()->getDiscountCurve(), valCcyLegCurveCollection, valCcyLeg->getStaticData()->getDiscountCurve());

		return asOfDateFxRate;
	}

	

	void updateMTMXccyNotionalResetByFxLeg(const LabelValueBlock& valuationSettingsLVB, const LegPtr& leg1, const LegPtr& leg2, const LAString& notionalResetLegName, bool isMTM, const LAString& legName)
	{

		bool isNotResetLeg = (legName.size() != 0 && !same(legName, notionalResetLegName));

		if (!isMTM || isNotResetLeg)
		{

			return;
		}

		//MTM XccySwap...

		LegPtr resetLeg;
		LegPtr normalLeg;

		if (same(leg1->getLegName(), notionalResetLegName))
		{
			resetLeg = leg1;
			normalLeg = leg2;
		}
		else if (same(leg2->getLegName(), notionalResetLegName))
		{
			resetLeg = leg2;
			normalLeg = leg1;
		}
		else
		{
			//It should not happen, as the check has been done in the swap constructor
			throw LACoreInvalidData("#Error: Invalid NotionalResetLeg: The NotionalResetLeg need to match the name (LegType) one of the swap first two legs", __FILE__, __LINE__);
		}

		LAString resetLegCurveCollection = getLWOCurveCollectionFromValuationSettings(valuationSettingsLVB, resetLeg->getLegName());
		LAString normalLegCurveCollection = getLWOCurveCollectionFromValuationSettings(valuationSettingsLVB, normalLeg->getLegName());

		//TODO: Enhance refresh logic to allow curves to tick, must be based on the object build time rather than the object name changes
		// See Swap.cpp Swap::updateNotionalResetByFxLeg line # 257 and Leg.cpp Leg::updateCashflows method line #62
		// --------------------------------------------------------------
		resetLeg->getStaticData()->setCurveDataLoaded(false);
		normalLeg->getStaticData()->setCurveDataLoaded(false);

		// If curveCollection is changed, then always reload the curve data
		/*if (!same(resetLeg->getStaticData()->getCurveCollection(), resetLegCurveCollection))
		{
			resetLeg->getStaticData()->setCurveDataLoaded(false);
		}
		if (!same(normalLeg->getStaticData()->getCurveCollection(), normalLegCurveCollection))
		{
			normalLeg->getStaticData()->setCurveDataLoaded(false);
		}
		}*/
		// --------------------------------------------------------------

		// If both first two legs' curve date have been loaded (getting DFs & FloatRates from the curves), the same the reset leg's notionals must been updated once already, so don't need to update again.  
		if (resetLeg->getStaticData()->isCurveDataLoaded() && normalLeg->getStaticData()->isCurveDataLoaded())
		{
			return;
		}

		//*** For resetLeg, the notionals are ajusted by formula: ResetNotional(t) = NormalLegNotional * fwdFxRateOnFxFixingDate(t)
		//, where fwdFxRateOnFxFixingDate(t) uses formula: FWD_FX_RATE( EUR/USD, 0, t ) = SPOT_FX( EUR/USD ) * DF( EUR_USDCSA, 0, t ) / DF( USD_USDCSA, 0, t)

		// 1) Get fxFixingDates
		DateVector fxFixingDates = resetLeg->getSchedule()->calculateFxFixingDates();


		// 2) Get Discount Factors of both legs based on the fxFixingDates, e.g. normal leg DF( EUR_USDCSA, 0, T), reset leg DF( USD_USDCSA, 0, T)

		//validate the DF curves have been built, and get he asOfDate from the curves
		resetLeg->getStaticData()->validateCurveInput(resetLegCurveCollection);
		normalLeg->getStaticData()->validateCurveInput(normalLegCurveCollection);

		// Both legs should be nased on the same asOfDate
		const LADate asOfDate = getCurveAsOfDate(resetLegCurveCollection);
		auto firstNonpastIndex = getFirstNonpastDateIndex(fxFixingDates, asOfDate);

		auto resetLegDFs = getCurveDiscountFactors(firstNonpastIndex, fxFixingDates, resetLegCurveCollection, resetLeg->getStaticData()->getDiscountCurve());
		auto normalLegDFs = getCurveDiscountFactors(firstNonpastIndex, fxFixingDates, normalLegCurveCollection, normalLeg->getStaticData()->getDiscountCurve());

		auto resetLegCashflowSize = resetLeg->getSchedule()->getCashflowSize();

		if (resetLegCashflowSize != resetLegDFs.size() || resetLegCashflowSize != normalLegDFs.size())
		{
			throw LACoreInvalidData("#Error: Inconsistent Data: The size of DiscountFactors based on FxFixingDates from two legs do not match", __FILE__, __LINE__);
		}

		auto resetLegSign = resetLeg->getSchedule()->getPayRecIndicator();

		// 3) asOfDate FxRate 
		const double asOfDateFxRate = getFXAsOfDateFromValuationSettings(valuationSettingsLVB, true); // true = Mandatory and Throw if missing

		// If isResetLegCurrencyDiffFromValuationCurrency is true, we need to revert the Swap spotFxRate (non-ValuationCurrency to ValuationCurrency) so that it becomes an exchange rate from normalLeg (ValuationCurrency) to resetLeg (non-ValuationCurrency)
		bool isResetLegCurrencyDiffFromValuationCurrency = (resetLeg->getStaticData()->getCurrency() != resetLeg->getStaticData()->getValuationCurrency());
		double asOfDateFxRateFromNormalLegToResetLeg = isResetLegCurrencyDiffFromValuationCurrency ? 1.0 / asOfDateFxRate : asOfDateFxRate;


		// 4) Get Notional of the normal leg 
		auto fxFixingDateSize = fxFixingDates.size();

		DoubleVector normalLegNotionals;

		//handling variable notionals 
		auto isNormalLegVariableNotional = normalLeg->getSchedule()->isVariableNotional();
		if (isNormalLegVariableNotional)
		{
			normalLegNotionals.reserve(fxFixingDateSize);
			auto resetPaymentDates = resetLeg->getSchedule()->getPaymentDates();
			auto normalCashflowSize = normalLeg->getSchedule()->getCashflowSize();
			auto absNormalLegNotional = std::numeric_limits<double>::quiet_NaN();

			for (size_t i = 0; i < resetPaymentDates.size(); ++i)
			{
				auto resetPayDt = resetPaymentDates[i];
				for (size_t j = 0; j < normalCashflowSize; ++j)
				{
					auto normalCashflow = normalLeg->getSchedule()->getCashflow(j);
					auto normalPayDt = normalCashflow->getPaymentDate();
					//always can find a normal payment date to match, as two legs will have the same final payment date
					if (resetPayDt <= normalPayDt)
					{
						absNormalLegNotional = std::abs(normalCashflow->getNotional());
						break;
					}
				}

				if (boost::math::isnan(absNormalLegNotional))
				{
					//Should not happen
					throw LACoreInvalidData("#Error: Invalid Notional: The Non-Reset leg's cashflow notional cannot be found for the fxFixingDate", __FILE__, __LINE__);
				}

				normalLegNotionals.push_back(absNormalLegNotional);
			}

		}
		else
		{
			auto absNormalLegNotional = std::abs(normalLeg->getSchedule()->getNotional());
			normalLegNotionals.push_back(absNormalLegNotional);
		}

		//Update the notionals of the reset leg cashflows
		double epsilon = 1e-20;
		for (size_t i = 0; i < resetLegCashflowSize; ++i)
		{
			auto cashflow = resetLeg->getSchedule()->getCashflow(i);

			auto resetLegDF = resetLegDFs[i];
			auto normalLegDF = normalLegDFs[i];
			auto fxFixingDate = fxFixingDates[i];
			auto absNormalLegNotional = isNormalLegVariableNotional ? normalLegNotionals[i] : normalLegNotionals[0];

			cashflow->setFxFixingDate(fxFixingDate);

			if (std::abs(resetLegDF) > epsilon)
			{
				//5) Calculate fwdFxRate (fx rate from normal leg to reset leg): FWD_FX_RATE( EUR/USD, 0, T ) = SPOT_FX( EUR/USD ) * DF( EUR_USDCSA, 0, T ) / DF( USD_USDCSA, 0, T)
				double fwdFxRate = asOfDateFxRateFromNormalLegToResetLeg * normalLegDF / resetLegDF;

				//6) Calculate Notional
				double notional = absNormalLegNotional * fwdFxRate * resetLegSign;
				cashflow->setNotional(notional);

				//*** Update resetLegfwdFxRate (fx rate from reset leg to normal leg), this is just for display purpose to be consistent with Bloomberg's FxRate
				double resetLegfwdFxRate = 1.0 / fwdFxRate;
				cashflow->setFwdFxRate(resetLegfwdFxRate);
			}
		}

		//Since the Notionals have been updated, we also need to update the NotionalExhanges
		resetLeg->getSchedule()->updateNotionalExchange();

	}

    double calculateParRate(const LabelValueBlock& valuationSettingsLVB, const LegCollection& legs, const LabelValueBlock& fixingTableNames)
    {
        double parRate = 0.;

        if(legs.size() < 2)
		{
			throw LACoreInvalidData( "#Error: Invalid Swap Leg: One of the legs is NOT set on the Swap", __FILE__, __LINE__ );
		}
		
		LegPtr leg1 = legs.get(0);
		LegPtr leg2 = legs.get(1);

		LegPtr firstFixedLeg = leg1->getType() == FIXED_SCHEDULE_TYPE ? leg1 : leg2;
		LegPtr secondLeg = leg1->getType() == FIXED_SCHEDULE_TYPE ? leg2 : leg1;

        bool fixedFloatSwap = (firstFixedLeg->getType() == FIXED_SCHEDULE_TYPE && secondLeg->getType() == FLOAT_SCHEDULE_TYPE);
        bool fixedFixedSwap = (firstFixedLeg->getType() == FIXED_SCHEDULE_TYPE && secondLeg->getType() == FIXED_SCHEDULE_TYPE);

        if (!fixedFloatSwap && !fixedFixedSwap)
        {
		    throw LACoreInvalidData( "#Error: ParRate function only supports a Swap with FixedFloat legs or FixedFixed legs", __FILE__, __LINE__ );
        }

		// The first fixed leg is the spread leg, other legs are reference legs:
		LegCollection refLegs;
		refLegs.add(secondLeg);
		for (size_t i = 2 ; i < legs.size(); ++i)
		{
			refLegs.add(legs.get(i));
		}

        double spread = calculateSwapSpread(refLegs, firstFixedLeg, valuationSettingsLVB, fixingTableNames, true);

		//Spread is in basis point, so need to scale back
		parRate = spread * 0.0001;
			
		return parRate;

	}

   std::pair<bool, double> solveIRR (DataProvider& dataProviderFixedLeg, double origPv, double pvFloatLeg, double origDeltaPV, double origIRR, double epsilonForPV, const LegPtr& fixedLeg)
    {
            double epsilonForDelta = 1e-20;
            int maxLoop = 200;
            int count = 0;
            bool solutionFound = false;

            double pv = origPv;
            double deltaPV = origDeltaPV;
            double irr  = origIRR;

            while (std::fabs(pv) > epsilonForPV && count < maxLoop)
            {
                double oldPV = pv;
                if (std::fabs(deltaPV) < epsilonForDelta)
                {
                    break;
                }
                auto oldIRR = irr;
                irr = irr - oldPV/deltaPV;

                //Calculate deltaPV: dpv(s) = (pv(s')-pv(s))/(s'-s)
				setSwapLegFixedRate(dataProviderFixedLeg, irr);

				auto pvfixedLeg = fixedLeg->pv(dataProviderFixedLeg);
                pv = pvfixedLeg + pvFloatLeg; 

                if (std::fabs(pv) <= epsilonForPV)
                {
                    //solution found
                    solutionFound = true;
                    break;
                }
                auto irrDiff = irr - oldIRR;
                if (std::fabs(irrDiff) < epsilonForDelta)
                {
                    break;
                }
                deltaPV = (pv - oldPV)/irrDiff;
                count++;
            }

           return std::make_pair(solutionFound, irr);
    }

    double calculateIRR(const LabelValueBlock& valuationSettingsLVB, const LegCollection& legs, const LabelValueBlock& fixingTableNames) 
    {
        double irr = 0.; //internal rate of return

        if(legs.size() < 2)
		{
			throw LACoreInvalidData( "#Error: Invalid Swap Leg: One of the legs is NOT set on the Swap", __FILE__, __LINE__ );
		}
		LegPtr leg1 = legs.get(0);
		LegPtr leg2 = legs.get(1);

	
		bool fixedFloatSwap = (leg1->getType() == FIXED_SCHEDULE_TYPE && leg2->getType() == FLOAT_SCHEDULE_TYPE);

        if (!fixedFloatSwap)
        {
		    throw LACoreInvalidData( "#Error: ParRate function only supports a Swap with FixedFloat legs", __FILE__, __LINE__ );
        }

		DataProvider dataProvider1(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg1->getLegName()));
		DataProvider dataProvider2(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg2->getLegName()));

        //Legs PV excluding the first fixed leg
		double otherLegsPV = leg2->pv( dataProvider2);

        //For more than two legs   
        for (size_t i = 2; i < legs.size(); ++i)
        {
            auto leg = legs.get(i);

			DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

			otherLegsPV += leg->pv( dataProvider);
        }

        //formula: firstFixedPV + firstFixedExchangeNotionalPV + otherLegsTotalPV = 0, where otherLegsTotalPV includes exchangeNotionalPV 
        if (leg1->getSchedule()->getNotional() ==0)
		{
			throw LACoreInvalidData( "#Error: Zero Notional: Cannot calculate IRR since the notional is zero", __FILE__, __LINE__ );
		}

		//Get DF from curve and update compoundInterest
        leg1->initializeDataProvider( dataProvider1 );

        double firstFixedExchangeNotionalPV = 0;
        if (leg1->getSchedule()->getNotionalExchangeEnum() != NONE_NE)
        {
            //get all the cashflows including the upfrontCashflow
            auto cashflows = leg1->getSchedule()->getAllCashflows();

            for( size_t i = 0; i < cashflows.size(); i++ )
		    {			
			    auto cf = cashflows[i];
			    firstFixedExchangeNotionalPV += cf->getNotionalExchangePv( dataProvider1.getCashflowDataIncludingUpfront( i ) );
		    }
        }

        // Fomula: fixedPV = Notional * [compoundInterest - 1]*DF_lastCashflow + firstFixedExchangeNotionalPV
        // Analytical calculation: compoundInterest = (1+IRR/m) ^ (m*n). For instance, compoundFreq is semi-annual and tenor is 5Y, then m=2 and n=5
        // Solver calculation: compoundInterest = (1+r_1*tao_1)(1+r_2*tao_2)..(1+r_mn-1*tao_mn-1) 

        // We use analytical calculation as a starting point, and then use solver.
                        
        //--- 1) Using this analytical IRR as a starting point: ---

        //=> Notional * [(1+IRR/m) ^ (m*n) - 1]*DF_lastCashflow + firstFixedExchangeNotionalPV  + floatsPV = 0
        //Let temp = (-floatsPV - fixedExchnageNotionalPV)/(fixedNotional*DF_lastCashflow) + 1
        // =>IRR = [(temp)^(1/mn) - 1] * m

        unsigned int m =  getFrequencyNumber(ANNUAL_FREQUENCY) / getFrequencyNumber(leg1->getSchedule()->getAccrualFrequency());
        double n = leg1->getSchedule()->getYearFractionFromEffectiveToMaturityDates();
        auto mn = m * n;

		// getCashflowSize() excludes the upfront cashflow
        auto firstFixedCashflowSize = leg1->getSchedule()->getCashflowSize();
		auto firstFixedLastCashflowData = dataProvider1.getCashflowDataExcludingUpfront( firstFixedCashflowSize - 1 );
        //Note: leg1 Notional already contains the sign
        double temp =  (-1.0 * otherLegsPV - firstFixedExchangeNotionalPV)/(leg1->getSchedule()->getNotional() * firstFixedLastCashflowData.discountFactor) + 1; 

        double rootValue = std::pow(temp, 1.0/mn);

        irr = (rootValue - 1.0) * m;

        //--- 2) If the analytical IRR is not good enough, start the Solver ---

		setSwapLegFixedRate(dataProvider1, irr);

        auto pvfixedLeg = leg1->pv( dataProvider1);
        auto pv = pvfixedLeg + otherLegsPV; 

        double epsilonForPvMax = 1e-8;
        if (std::fabs(pv) <= epsilonForPvMax)
        {
            // If the PV is smaller enough (close to zero), then we don't solve the irr
            return irr;
        }


        //2) Use Secant method to find the spread of lower tenor leg
        double ds = 0.0002; 
            
		setSwapLegFixedRate(dataProvider1, irr - ds);

        auto pv0 = leg1->pv( dataProvider1) + otherLegsPV;
            
		setSwapLegFixedRate(dataProvider1, irr + ds);

		auto pv1 = leg1->pv( dataProvider1) + otherLegsPV;

        auto deltaPV = (pv1-pv0)/(2.0*ds);

        //error allowance should be smaller so that the solver can solve better result
        double epsilonForPV = 1e-15;

        auto result = solveIRR (dataProvider1, pv, otherLegsPV, deltaPV, irr, epsilonForPV, leg1);

        while (result.first == false && epsilonForPV <= epsilonForPvMax)
        {
            //If solution cannot be found, we increase the error allowance and try to solve again, until it hit the max error allowance
            epsilonForPV *=10;
            result = solveIRR (dataProvider1, pv, otherLegsPV, deltaPV, irr, epsilonForPV, leg1);
        }

        bool solutionFound = result.first;
        if (!solutionFound)
        {
   			throw LACoreInvalidData( "#Error: Solver Fails to Converge to a Solution: Cannot find a IRR to make the swap pv close to zero (smaller or equal to 1-e8)", __FILE__, __LINE__ );
        }
           
        irr = result.second;

        return irr;
	}

	// This function only used for fixed legs
    double calculateNotionalFromFutureValueNotional(const LegPtr& leg)
    {
        auto cashflowSize = leg->getSchedule()->getCashflowSize();
        
        //need to update previous interests before calling compoundInterest.
		ValuationSettings valSettings;
		DataProvider dataProvider(valSettings);
        leg->getSchedule()->updateCashflowsCompoundingCoupons(dataProvider);

        //Calculate CompoundInterest, by: Notional *(CompoundInterest - 1) = CouponAtMaturity
        auto atMaturityCashflow = leg->getSchedule()->getCashflow(cashflowSize -1);
		CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront( cashflowSize - 1 );
        double couponAtMaturity = atMaturityCashflow->getCoupon( cashflowData );
        double compoundInterest = couponAtMaturity/atMaturityCashflow->getNotional()  + 1.0;

        // Formula: FVNotional = Notional * compoundedInterest;
        double notional = leg->getSchedule()->getFutureValueNotional() * 1.0/compoundInterest;

        return notional;

    }

    void setNotionalFromFutureValueNotional(const LegPtr& leg1, const LegPtr& leg2, const CCY& valuationCurrency, double asOfDateFxRate)
    {

        bool leg1WithFVNotional = !boost::math::isnan(leg1->getSchedule()->getFutureValueNotional());
        bool leg2WithFVNotional = !boost::math::isnan(leg2->getSchedule()->getFutureValueNotional());

        bool fixedFixedSwap = (leg1->getType() == FIXED_SCHEDULE_TYPE && leg2->getType() == FIXED_SCHEDULE_TYPE);
        if (fixedFixedSwap && (leg1WithFVNotional || leg2WithFVNotional))
        {
		    throw LACoreInvalidData( "#Error: Invalid Notional: For FixedFixed Zero Coupon Swap, FVNotional is not supported, please specify Notional", __FILE__, __LINE__ );
        }

        //*** Note that for a FixedFixedSwap, we only allow the first fixedLeg to have FVNotional
        if (leg2WithFVNotional)
        {
            throw LACoreInvalidData("#Error: Invalid Notional: Only the first fixed leg can have FV Notional.",__FILE__,__LINE__);
        }

        // The case when FVNotional and Notinoal are both provided have been validated in Schedule's populateNotionalAndPaymentFreqEnum()
        // When leg1WithFVNotinoal, it cannot be a float leg. For a fixedfloatSwap/fixedfixedSwap, we know that the first leg is always fixed, from createSwapFromLegs() of SwapUtilities.cpp
        if (leg1WithFVNotional)
        {

            // Calculate the Notional from FVNotional
            double leg1Notional = calculateNotionalFromFutureValueNotional(leg1);
            double leg2Notional = leg1Notional;
    
            if (valuationCurrency != NO_CCY)
            {
                leg2Notional = (valuationCurrency == leg1->getStaticData()->getCurrency()) ? (leg1Notional / asOfDateFxRate) : (leg1Notional * asOfDateFxRate);
            }

            // update notional 
            leg1->getSchedule()->setNotional(leg1Notional * leg1->getSchedule()->getPayRecIndicator());
            leg2->getSchedule()->setNotional(leg2Notional * leg2->getSchedule()->getPayRecIndicator());
        }
        else
        {
            if (boost::math::isnan(leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional()))
            {
        	    throw LACoreInvalidData( "#Error: Invalid Notional: If FVNotional is not provided for a Zero Coupon Swap, notional for both legs must be provided.", __FILE__, __LINE__ );
            }
        }
    }

	}