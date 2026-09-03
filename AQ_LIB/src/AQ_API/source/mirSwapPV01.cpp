#include "InitializeAQETrading.h"
#include "mirSwapPV01.h"
#include "TypeUtilities.h"
#include "tryMirSwapPV01.h"



/* @brief			swig interface for mirSwapPV01
*  @param [in]		dataInstance	Pointer to the object pool
*  @param [in]		notional		Swap notional
*  @param [in]		payRec			Payer swap or Receiver swap
*  @param [in]		effectDt	    The effective start date of the swap, ie, base date + spot date
*  @param [in]		maturity		Swap maturity date or tenor
*  @param [in]		curveId			ID of the curve set
*  @param [in]		xFreq			Fixed leg frequency
*  @param [in]		xDayCount		Fixed leg day count convention
*  @param [in]		xRollCnv		Fixed leg roll convention
*  @param [in]		xCalendar		Fixed leg calendar
*  @param [in]		xFirstStub		End date of the front stub period on fixed leg
*  @param [in]		xLastStub		Start date of the end stub period on fixed leg
*  @param [in]		xRollDay		Fixed leg rolling date
*  @param [in]		xPayLag			Fixed leg payment date lag
*  @param [in]		xStub			Fixed leg Stub type
*  @param [in]		tSpread			Floating Leg spread in basis points. This param is not used in the core functions.
*  @param [in]		tFreq			Floating leg frequency
*  @param [in]		tDayCount		Floating leg day count convention
*  @param [in]		tRollCnv		Floating leg rolling convention
*  @param [in]		tCalendar		Floating leg calendar
*  @param [in]		tFistStub		End date of the front stub period on floating leg
*  @param [in]		tLastStub		Start date of the end stub period on floating leg
*  @param [in]		tRollDay		Floating leg rolling day
*  @param [in]		tFixLag			Floating leg fixing day lag
*  @param [in]		tFistFix		Floating leg first fixing
*  @param [in]		tLastFix		Floating leg last fixing
*  @param [in]		tPayLag			Floating leg payment date lag
*  @param [in]		tStub			Floating leg stub type
*  @param [in]		interpolation	Interpolation method
*  @param [in]		forecastCurve	Forecasting curve
*  @param [in]		discountCurve	Discounting curve
*  @param [in]		interpFwds		Boolean that decides if direct interpolation on fwd rates is employed. This param is not used in the core functions.
*  @param [in]		eomRoll			Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
*  @return			swap pv01 
*/
double mirSwapPV01(double notional,
				const std::string& payRec, 
				const std::string& effectDt, 
				const std::string& maturity, 
				const std::string& curveId, 
				const std::string& xFreq, 
				const std::string& xDayCount, 
				const std::string& xRollCnv, 
				const std::string& xCalendar, 
				const std::string& xFirstStub, 
				const std::string& xLastStub, 
				const std::string& xRollDay, 
				const std::string& xPayLag, 
				const std::string& xStub, 
				double tSpread, // this param is not used
				const std::string& tFreq, 
				const std::string& tDayCount, 
				const std::string& tRollCnv, 
				const std::string& tCalendar,
				const std::string& tFirstStub,
				const std::string& tLastStub,
				const std::string& tRollDay,
				const std::string& tFixLag,
				double tFirstFix,
				double tLastFix,
				const std::string& tPayLag,
				const std::string& tStub,
				const std::string& interpolation,
				const std::string& forecastCurve,
				const std::string& discountCurve,
				bool interpFwds, // this param is not used
				bool eomRoll)
{
	double ret;
	try 
	{
		// marshall all inputs		
		using swig::fromStringToLAString;
		
		ret = validation::tryMirSwapPV01(etrading::InitializeAQETrading::instance().dataInstance(),
												notional,
												fromStringToLAString(payRec),
												fromStringToLAString(effectDt),
												fromStringToLAString(maturity),
												fromStringToLAString(curveId),
												fromStringToLAString(xFreq),
												fromStringToLAString(xDayCount),
												fromStringToLAString(xRollCnv),
												fromStringToLAString(xCalendar),
												fromStringToLAString(xFirstStub),
												fromStringToLAString(xLastStub),
												fromStringToLAString(xRollDay),
												fromStringToLAString(xPayLag),
												fromStringToLAString(xStub),
												tSpread,
												fromStringToLAString(tFreq),
												fromStringToLAString(tDayCount),
												fromStringToLAString(tRollCnv),
												fromStringToLAString(tCalendar),
												fromStringToLAString(tFirstStub),
												fromStringToLAString(tLastStub),
												fromStringToLAString(tRollDay),
												fromStringToLAString(tFixLag),
												tFirstFix,
												tLastFix,
												fromStringToLAString(tPayLag),
												fromStringToLAString(tStub),
												fromStringToLAString(interpolation),
												fromStringToLAString(forecastCurve),
												fromStringToLAString(discountCurve),
												interpFwds,
												eomRoll);

	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}


		
			