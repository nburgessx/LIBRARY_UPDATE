/* 
 * @brief			Swig interface to Java for mirGetParRate methods
 * @Created:		17 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirGetParRate4
*  @param [in]		EffDt			The effective start date of the swap, ie, base date + spot date
*  @param [in]		Mat				Swap maturity date or tenor
*  @param [in]		CrvID			ID of the curve set
*  @param [in]		XFreq			Fixed leg frequency
*  @param [in]		XDayCt			Fixed leg day count convention
*  @param [in]		XRollCnv		Fixed leg roll convention
*  @param [in]		XCal			Fixed leg calendar
*  @param [in]		XFirstStub		End date of the front stub period on fixed leg
*  @param [in]		XLastStub		Start date of the end stub period on fixed leg
*  @param [in]		XRollDay		Fixed leg rolling date
*  @param [in]		XPayLag			Fixed leg payment date lag
*  @param [in]		XStub			Fixed leg Stub type
*  @param [in]		TFreq			Floating leg frequency
*  @param [in]		TDayCt			Floating leg day count convention
*  @param [in]		TRollCnv		Floating leg rolling convention
*  @param [in]		TCal			Floating leg calendar
*  @param [in]		TFistStub		End date of the front stub period on floating leg
*  @param [in]		TLastStub		Start date of the end stub period on floating leg
*  @param [in]		TRollDay		Floating leg rolling day
*  @param [in]		TFixLag			Floating leg fixing day lag
*  @param [in]		TFistFix		Floating leg first fixing
*  @param [in]		TLastFix		Floating leg last fixing
*  @param [in]		TPayLag			Floating leg payment date lag
*  @param [in]		TStub			Floating leg stub type
*  @param [in]		Interp			Interpolation method
*  @param [in]		Fcrv			Forecasting curve
*  @param [in]		Dcrv			Discounting curve
*  @param [in]		InterpFwds		Boolean that decides if direct interpolation on fwd rates is employed
*  @param [in]		EomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
*  @param [in]		TSpd			Floating leg spread
*  @param [in]		UseFwdData		Use fwd rates directly or derive them from spot rates
*/
double mirGetParRate4(const std::string& EffDt, 
					const std::string& Mat, 
					const std::string& CrvID, 
					const std::string& XFreq, 
					const std::string& XDayCt, 
					const std::string& XRollCnv, 
					const std::string& XCal, 
					const std::string& XFirstStub, 
					const std::string& XLastStub, 
					const std::string& XRollDay, 
					const std::string& XPayLag, 
					const std::string& XStub, 
					const std::string& TFreq, 
					const std::string& TDayCt, 
					const std::string& TRollCnv, 
					const std::string& TCal,
					const std::string& TFirstStub,
					const std::string& TLastStub,
					const std::string& TRollDay,
					const std::string& TFixLag,
					double TFirstFix,
					double TLastFix,
					const std::string& TPayLag,
					const std::string& TStub,
					const std::string& Interp,
					const std::string& Fcrv,
					const std::string& Dcrv,
					bool InterpFwds,
					bool EomRoll,
					double TSpd,
					bool UseFwdData );



/* @brief			swig interface for mirGetParRate5
*  @param [in]		EffDt			The effective start date of the swap, ie, base date + spot date
*  @param [in]		Mat				Swap maturity date or tenor
*  @param [in]		CrvID			ID of the curve set
*  @param [in]		XFreq			Fixed leg frequency
*  @param [in]		XDayCt			Fixed leg day count convention
*  @param [in]		XRollCnv		Fixed leg roll convention
*  @param [in]		XCal			Fixed leg calendar
*  @param [in]		XFirstStub		End date of the front stub period on fixed leg
*  @param [in]		XLastStub		Start date of the end stub period on fixed leg
*  @param [in]		XRollDay		Fixed leg rolling date
*  @param [in]		XPayLag			Fixed leg payment date lag
*  @param [in]		XStub			Fixed leg Stub type
*  @param [in]		TFreq			Floating leg frequency
*  @param [in]		TDayCt			Floating leg day count convention
*  @param [in]		TRollCnv		Floating leg rolling convention
*  @param [in]		TCal			Floating leg calendar
*  @param [in]		TFistStub		End date of the front stub period on floating leg
*  @param [in]		TLastStub		Start date of the end stub period on floating leg
*  @param [in]		TRollDay		Floating leg rolling day
*  @param [in]		TFixLag			Floating leg fixing day lag
*  @param [in]		TFistFix		Floating leg first fixing
*  @param [in]		TLastFix		Floating leg last fixing
*  @param [in]		TPayLag			Floating leg payment date lag
*  @param [in]		TStub			Floating leg stub type
*  @param [in]		Interp			Interpolation method
*  @param [in]		Fcrv			Forecasting curve
*  @param [in]		Dcrv			Discounting curve
*  @param [in]		InterpFwds		Boolean that decides if direct interpolation on fwd rates is employed
*  @param [in]		EomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
*  @param [in]		TSpd			Floating leg spread
*  @param [in]		UseFwdData		Use fwd rates directly or derive them from spot rates
*  @param [in]		TFixCalc		Calendar for fixingd date schedule
*/
double mirGetParRate5(const std::string& EffDt, 
					const std::string& Mat, 
					const std::string& CrvID, 
					const std::string& XFreq, 
					const std::string& XDayCt, 
					const std::string& XRollCnv, 
					const std::string& XCal, 
					const std::string& XFirstStub, 
					const std::string& XLastStub, 
					const std::string& XRollDay, 
					const std::string& XPayLag, 
					const std::string& XStub, 
					const std::string& TFreq, 
					const std::string& TDayCt, 
					const std::string& TRollCnv, 
					const std::string& TCal,
					const std::string& TFirstStub,
					const std::string& TLastStub,
					const std::string& TRollDay,
					const std::string& TFixLag,
					double TFirstFix,
					double TLastFix,
					const std::string& TPayLag,
					const std::string& TStub,
					const std::string& Interp,
					const std::string& Fcrv,
					const std::string& Dcrv,
					bool InterpFwds,
					bool EomRoll,
					double TSpd,
					bool UseFwdData,
					const std::string& TFixCalc);



/* @brief			swig interface for mirGetParRate3
*  @param [in]		dataInstance			Pointer to the object pool
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		curveId			ID of the curve set
*  @param [in]		frequency		Frequency
*  @param [in]		dayCount		Day count convention
*  @param [in]		slidingRule		Sliding rule
*  @param [in]		calendar		Calendar
*  @param [in]		interpolation	Interpolation method
*  @param [in]		forecastCurve	Forecasting curve
*  @param [in]		discountCurve	Discounting curve
*  @param [in]		firstOddDate	End date of the front stub period
*  @param [in]		lastOddDate		Start date of the end stub period
*  @param [in]		paymentDay		PaymentDay
*  @param [in]		forecastCurve	Forecasting curve
*  @param [in]		discountCurve	Discounting curve
*  @param [in]		interpFwds		Boolean that decides if direct interpolation on fwd rates is employed
*  @param [in]		eomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
*  @return			par rate 
*/
double mirGetParRate3(const std::string& fromDate, 
						const std::string& toDate,
						const std::string& curveId, 
						const std::string& frequency,
						const std::string& dayCount, 
						const std::string& slidingRule,
						const std::string& calendar,
						const std::string& interpolation, 
						const std::string& firstOddDate, 
						const std::string& lastOddDate,
						const std::string& paymentDay,
						const std::string& foreCurveName, 
						const std::string& dfCurveName, 
						bool interpFwds,
						bool eomRoll);