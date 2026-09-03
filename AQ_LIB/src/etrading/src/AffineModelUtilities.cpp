#include "AffineModelUtilities.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "ScheduleValidation.h"
#include "ExceptionMacros.h"

namespace etrading
{

	/* @brief			Calculate input date information
	*  @param [in]		fixingDates			Input fixing dates
	*  @param [in]		curveCollection	    Curve Collection
	*  @param [in]		curveIndex			Curve Index name
	*  @return			A list of InputDateInfos
	*/
	std::vector<InputDateInfo> calculateInputDateInfos(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex) 	{
		
		// Get ts and Ts of the forward rates:

		size_t expectedSize = fixingDates.size();
        AQ_REQUIRE( expectedSize >= 1, "The input fixing dates cannot be empty" );

		const LAString curveId (curveCollection.c_str()); 
		const LAString curveIndx (curveIndex.c_str()); 

		const auto curveTenor = validateCurveAndGetCurveFrequency(curveId, curveIndx);

		const LADate curveAsOfDate = getCurveAsOfDate( curveId );
		const DayCountEnum fwdCurveDayCount = toDayCountEnum(validateCurveAndGetFloatDaycount( curveId, curveIndx).getCString());

		//Should get the SlidingRule from Future Instrument of the curveIndex
		const LAString businessDayAdj = "MOD_FOLLOWING"; 

		const LAString calendar = getDefaultCalendarForEmptyString( "", curveId );

		const DateVector TDates = getDateFromTenor(fixingDates, curveTenor, businessDayAdj, calendar, "");

		std::vector<InputDateInfo> inputDates(expectedSize);

		for( size_t i = 0; i < expectedSize; ++i )
		{
			InputDateInfo inputInfo;
			
			// Calculate tDate, TDate
			const auto t = fixingDates[i];
			inputInfo.tDate = fixingDates[i];
			inputInfo.TDate = TDates[i];

			// year fraction from as of to tDate (t)
			inputInfo.t = getYearFraction(curveAsOfDate, inputInfo.tDate, fwdCurveDayCount);

			// year fraction from tDate to TDate (T-t)
			inputInfo.tao = getYearFraction( inputInfo.tDate, inputInfo.TDate, fwdCurveDayCount);

			// year fraction from as of to TDate (T)
			inputInfo.T = inputInfo.t + inputInfo.tao;

			inputDates[i] = inputInfo;
		}

		return inputDates;
	}

	/* @brief			Get forward rates from discount factors via Discrete Formula: F(t,T) = (P(0,t)/P(0,T) - 1) / (T- t) = (1/P(t,T) - 1) / (T- t) 
	*  @param [in]		inputDateInfos	    Input date information (t, T, T-t)
	*  @param [in]		P_t_Ts				Discount factors P(t,T)s
	*  @return			A list of F(t,T)s
	*/
	DoubleVector forwardRates(const std::vector<InputDateInfo>& inputDateInfos, const std::vector<double>& P_t_Ts) 
	{
		// Discrete Formula: F(t,T) = (P(0,t)/P(0,T) - 1) / (T- t) = (1/P(t,T) - 1) / (T- t) 

		const size_t expectedSize = inputDateInfos.size();

		DoubleVector fwdRates (expectedSize);

		for( size_t i = 0; i < expectedSize; ++i )
		{
			auto hwInputInfo = inputDateInfos[i];

			double P_t_T = P_t_Ts[i];
	
			double fwdRate = (1.0/P_t_T - 1.0) / hwInputInfo.tao;

			fwdRates[i] = fwdRate;
		}
		
		return fwdRates;
	}

	/* @brief			Check if the valuation is the same or later than asOfDate, throw error if it's smaller earlier than asOfdDate
	*  @param [in]		curveAsOfDate	    curveAsOfDate
	*  @param [in]		P_t_Ts				Discount factors P(t,T)s
	*  @return			True to indicate it's future curve (i.e. valuationDate > asOfDate), false to indicate it's today's curve (i.e. valuationDate == asOfDate)
	*/
	bool isFutureValuation(const LADate& curveAsOfDate, const LADate& valuationDate)
	{
		bool futureValuation = false;
		
		if (valuationDate == LADate() || valuationDate == curveAsOfDate)
		{
			futureValuation = false;
		}
		else if (valuationDate > curveAsOfDate)
		{
			futureValuation = true;
		}
		else
		{
			throw LACoreInvalidData("#Error: Valuation Date cannot be earlier than curve's asOfDate.",__FILE__,__LINE__);
		}
		return futureValuation;
	}

	/* @brief			Get the default short rate at asOfDate if its not provided 
	*  @param [in]		curveAsOfDate	    curveAsOfDate
	*  @param [in]		curveCollection	    Curve Collection
	*  @param [in]		curveIndex			Curve Index name
    *  @param [in]		futureValuation		False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
	*  @return			default short rate at asOfDate if its not provided
	*/
	double getDefaultShortRate(const LADate& curveAsOfDate, const std::string& curveCollection, const std::string& curveIndex, const double& rt, const bool& futureValuation)
	{
		double shortRate = rt;

		if (boost::math::isnan(shortRate))
		{
			if (futureValuation)
			{
				throw LACoreInvalidData("#Error: For future curve, rt must be provided.",__FILE__,__LINE__);
			}
			else
			{
				// If it's current curve, and the shortRate is not provided, the shortRate is default to be the shortRate at asOfDate
				shortRate = etrading::getCurveShortRates(boost::assign::list_of(curveAsOfDate), curveCollection, curveIndex)[0];
			}
		}

		return shortRate;
	}



}
