/* 
 * @brief			Swig interface to Java for function mirSwapStubRate
 * @Created:		30 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "InitializeMLibETrading.h"
#include "mirSwapStubRate.h"
#include "LADate.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryMirSwapStubRate.h"




/* @brief			swig interface for mirSwapStubRate
*  @param [in]		EffDt				The effective start date of the swap, ie, base date + spot date
*  @param [in]		Mat					Swap maturity date or tenor
*  @param [in]		Freq				Floating leg frequency
*  @param [in]		DayCt				Floating leg day count convention
*  @param [in]		RollCnv				Floating leg rolling convention
*  @param [in]		Cal					Floating leg calendar
*  @param [in]		RollDay				Floating leg rolling day
*  @param [in]		FixLag				Floating leg fixing day lag
*  @param [in]		EomRoll				Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
*  @param [in]		Interpolation		Interpolation method
*  @param [in]		FistStub			End date of the front stub period on floating leg
*  @param [in]		LastStub			Start date of the end stub period on floating leg
*  @param [in]		StubType			Floating leg stub type
*  @param [in]		CrvID				ID of the curve set
*  @param [in]		CurveNames			A list of curves to interpolate from
*  @param [in]		CurveTenors			A list of tenors corresponding to curve names
*  @param [in]		TenorCurveFixings	A list of tenors corresponding to curve names
*  @param [in]		UseCurveName		A curve specifically chosen by user to use
*  @param [in]		ToleranceTenor		Tenor that defines if a nearby curve should be chosen for use
*  @param [in]		IsFwdInterp			Boolean that decides if direct interpolation on fwd rates is employed
*  @param [in]		UseFwdData			Use fwd rates directly or derive them from spot rates
*/
double mirSwapStubRate(const std::string& EffDt, 
						const std::string& Mat, 
						const std::string& Freq, 
						const std::string& DayCt, 
						const std::string& RollCnv, 
						const std::string& Cal,
						const std::string& RollDay,
						const std::string& FixLag,
						bool EomRoll,
						const std::string& Interpolation,
						const std::string& FirstStub,
						const std::string& LastStub,
						const std::string& StubType,
						const std::string& CrvID,
						const std::vector<std::string>& CurveNames,
						const std::vector<std::string>& CurveTenors,
						const std::vector<double>& TenorCurveFixings,
						const std::string& UseCurveName,
						const std::string& ToleranceTenor,
						bool IsFwdInterp,
						bool UseFwdData ) 
{
	double ret;
	try 
	{
		// marshall all inputs		
		LAString effDt		(EffDt.c_str());
		LAString mat		(Mat.c_str());
		LAString freq		(Freq.c_str());
		LAString dayCt		(DayCt.c_str());
		LAString rollCnv	(RollCnv.c_str());
		LAString cal		(Cal.c_str());
		LAString fixLag		(FixLag.c_str());
		LAString interpolation	(Interpolation.c_str());
		LAString firstStub		(FirstStub.c_str());
		LAString lastStub		(LastStub.c_str());
		LAString stubType		(StubType.c_str());
		LAString crvID			(CrvID.c_str());
		LAString useCurveName	(UseCurveName.c_str());
		LAString toleranceTenor	(ToleranceTenor.c_str());
		LAString rollDay		(RollDay.c_str());

		LAStringVector curveNames;
		swig::buildStringVector(curveNames, CurveNames);

		LAStringVector curveTenors;
		swig::buildStringVector(curveTenors, CurveTenors);

		ret = validation_api::tryMirSwapStubRate(etrading::InitializeMLibETrading::instance().dataInstance(),
												effDt, 
												mat, 
												freq, 
												dayCt, 
												rollCnv, 
												cal, 
												rollDay, 
												fixLag, 
												EomRoll, 
												interpolation, 
												firstStub, 
												lastStub, 
												stubType, 
												crvID, 
												curveNames, 
												curveTenors,
												TenorCurveFixings,
												useCurveName,
												toleranceTenor,
												IsFwdInterp,
												UseFwdData);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}