#include "InitializeAQETrading.h"
#include "mirOISSwapPV.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryMirOISSwapPV.h"



/* @brief			swig interface for mirSwapPV
*  @param [in]		Notional		Swap notional
*  @param [in]		PayRec			Payer swap or Receiver swap
*  @param [in]		EffDt			The effective start date of the swap, ie, base date + spot date
*  @param [in]		Mat				Swap maturity date or tenor
*  @param [in]		CrvID			ID of the curve set
*  @param [in]		XRt				Swap fixed leg rate
*  @param [in]		XFreq			Fixed leg frequency
*  @param [in]		XDayCt			Fixed leg day count convention
*  @param [in]		XRollCnv		Fixed leg roll convention
*  @param [in]		XCal			Fixed leg calendar
*  @param [in]		XFirstStub		End date of the front stub period on fixed leg
*  @param [in]		XLastStub		Start date of the end stub period on fixed leg
*  @param [in]		XRollDay		Fixed leg rolling date
*  @param [in]		XPayLag			Fixed leg payment date lag
*  @param [in]		XStub			Fixed leg Stub type
*  @param [in]		TSpd			Floating leg spread
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
*  @param [in]		CompMethod		Compounding methodology for OIS swap
*/
double mirOISSwapPV(double notional,
				const std::string& PayRec,
				const std::string& EffDt, 
				const std::string& Mat, 
				const std::string& CrvID, 
				double XRt,
				const std::string& XFreq, 
				const std::string& XDayCt, 
				const std::string& XRollCnv, 
				const std::string& XCal, 
				const std::string& XFirstStub, 
				const std::string& XLastStub, 
				const std::string& XRollDay, 
				const std::string& XPayLag, 
				const std::string& XStub, 
				double TSpd,
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
				const std::string& CompMethod)
{
	double ret;
	try 
	{
		// marshall all inputs		
		AQLString payRec			(PayRec.c_str());
		AQLString effectiveDate	(EffDt.c_str());
		AQLString maturity		(Mat.c_str());
		AQLString curveID		(CrvID.c_str());
		AQLString xFreq			(XFreq.c_str());
		AQLString xDayCount		(XDayCt.c_str());
		AQLString xRollConv		(XRollCnv.c_str());
		AQLString xCalendar		(XCal.c_str());
		AQLString xFirstStub		(XFirstStub.c_str());
		AQLString xLastStub		(XLastStub.c_str());
		AQLString xPayLag		(XPayLag.c_str());
		AQLString xStub			(XStub.c_str());
		AQLString tFreq			(TFreq.c_str());
		AQLString tDayCount		(TDayCt.c_str());
		AQLString tRollConv		(TRollCnv.c_str());
		AQLString tCalendar		(TCal.c_str());
		AQLString tFirstStub		(TFirstStub.c_str());
		AQLString tLastStub		(TLastStub.c_str());
		AQLString tFixLag		(TFixLag.c_str());
		AQLString tPayLag		(TPayLag.c_str());
		AQLString tStub			(TStub.c_str());
		AQLString interpolation	(Interp.c_str());
		AQLString forecastCurve	(Fcrv.c_str());
		AQLString discountCurve	(Dcrv.c_str());
		AQLString compMethod		(CompMethod.c_str());
		AQLString tRollDay		(TRollDay.c_str());
		AQLString xRollDay		(XRollDay.c_str());

		ret = validation::tryMirOISSwapPV(etrading::InitializeAQETrading::instance().dataInstance(),
												notional,
												payRec,
												effectiveDate, 
												maturity, 
												curveID, 
												XRt,
												xFreq, 
												xDayCount, 
												xRollConv, 
												xCalendar, 
												xFirstStub, 
												xLastStub, 
												xRollDay,
												xPayLag, 
												xStub, 
												tFreq, 
												tDayCount, 
												tRollConv, 
												tCalendar,
												tFirstStub,
												tLastStub,
												tRollDay,
												tFixLag,
												TFirstFix,
												TLastFix,
												tPayLag,
												tStub,
												interpolation,
												forecastCurve,
												discountCurve,
												EomRoll,
												TSpd,
												compMethod);
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}