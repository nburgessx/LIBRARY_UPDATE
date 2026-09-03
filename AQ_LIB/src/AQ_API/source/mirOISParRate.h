#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirOISParRate
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
*  @param [in]		EomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
*  @param [in]		TSpd			Floating leg spread
*  @param [in]		CompMethod		Compounding method
*/
double mirOISParRate(const std::string& EffDt, 
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
					bool EomRoll,
					double TSpd,
					const std::string& CompMethod);