#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

//include
#include <iostream>
#include <algorithm>
#include <cmath>
#include "AQLBasic.h"
#include "AQLDist.h"
#include <AQLCoreUtility.h>
#include <AQLMathDefine.h>
#include "AQLFunction.h"
#include <AQLMathInterpolationUtilities.h>
#include <AQLMathYieldCurve.h>
#include <AQLDataReference.h>
#include <AQLMathDateUtilities.h>
#include "AQLFunctionVector.h"
#include "AQLNl2sol.h"
#include <AQLCoreTemplateType.h>
#include <AQLMathDisplacedHestonTDP.h>
#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include <AQLPriceDataCalendar.h>

using namespace std; 

#if !defined(WIN32) && !defined(WIN64) && !defined(_isnan)
#include <cmath>
#define _isnan isnan
#endif


void AQLMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( const AQLString& target, InterpolationTarget& target_ )
{
    if( target == "LOGSTRIKE" ) { target_ = TargetLogStrike;}
    else if( target == "DELTAPUT" ) {target_ = TargetDeltaPut;}
    else if( target == "STRIKE" ) {target_ = TargetStrike;}
    else 
    {
        AQLString msg("InterpolationTarget is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
};

void AQLMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( const AQLString& target, ATMInterpolationMethod& target_ )
{
    if( target == "TERM" ) { target_ = TermWeighted;}
    else if( target == "DAILY" || target == "BUSINESS_DAYS") {target_ = DailyWeighted;}
    else if( target == "SQUAREDAILY" ) {target_ = SquareDailyWeighted;}
    else if( target == "TERMNORMAL") { target_ = TermNoWeighted;}
    else 
    {
        AQLString msg("ATMInterpolationMethod is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
};

void AQLMathFXVolatilitySurfaceGenerate::SetInterpolationMethod( const AQLString& target, InterpolationMethod& target_ )
{
    if( target == "SPLINE" ) { target_ = SPLINE_FXVOL;}
    else if( target == "CONSTRAINEDSPLINE" ) {target_ = CONSTRAINEDSPLINE_FXVOL;}
    else if( target == "LINEAR" ) {target_ = LINEAR_FXVOL;}
    else 
    {
        AQLString msg("InterpolationMethod is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
};

void AQLMathFXVolatilitySurfaceGenerate::SetInterpolationVariable( const AQLString& variable, InterpolationVariable& variable_ )
{
    if( variable == "DELTAPUT" ) { variable_ = VariableDeltaPut;}
    else if( variable == "DELTACALL" ) {variable_ = VariableDeltaCall;}
    else if( variable == "LOGSTRIKE" ) {variable_ = VariableLogStrike;}
    else if( variable == "STRIKE" ) {variable_ = VariableLogStrike;}
    else 
    {
        AQLString msg("InterpolationVariable is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
};

void AQLMathFXVolatilitySurfaceGenerate::SetFXOptionParam( AQLStringVector& str, FXOptionData& x )
{
    if( str.size() != 7 ) 
    {
        AQLString msg("Size of FXOptionParam is 7!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };

    x.T = AQLCoreUtility::changeDoubleFromString(str)[0];
    x.Pd = AQLCoreUtility::changeDoubleFromString(str)[1];
    x.Pf = AQLCoreUtility::changeDoubleFromString(str)[2];
    x.F = AQLCoreUtility::changeDoubleFromString(str)[3];

    if( str[4] == "FWDNONPRE" )  { x.deltaType = FWD_NONPRE;}
    else if( str[4] == "FWDPRE" )  { x.deltaType = FWD_PRE;}
    else if( str[4] == "SPOTNONPRE" )  { x.deltaType = SPOT_NONPRE;}
    else if( str[4] == "SPOTPRE" )  { x.deltaType = SPOT_PRE;}
    else 
    {
        AQLString msg("DeltaType is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };
    
    if( str[5] == "FORWARDATM" )  { x.atmType = FORWARDATM;}
    else if( str[5] == "SPOTATM" )  { x.atmType = SPOTATM;}
    else if( str[5] == "DELTANEUTRALNONPRE" )  { x.atmType = DELTANEUTRAL_NONPRE;}
    else if( str[5] == "DELTANEUTRALPRE" )  { x.atmType = DELTANEUTRAL_PRE;}
    else 
    {
        AQLString msg("FXATMStrikeType is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };

    x.Days = (unsigned int)AQLCoreUtility::changeDoubleFromString(str)[6];

    if( ( x.deltaType == FWD_NONPRE && x.atmType == DELTANEUTRAL_PRE ) || 
        ( x.deltaType == FWD_PRE && x.atmType == DELTANEUTRAL_NONPRE ) )
    {
        AQLString msg("Option type is PRE or NONPRE!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };
};

FXOptionData AQLMathFXVolatilitySurfaceGenerate::SetFXOptionParam( AQLDataInstance& dataInstance,
                                                              const AQLString& dCurveID,
                                                              const AQLString& fCurveID,
                                                              const AQLDate& maturityDate,
                                                              const AQLDate& deliveryDate,
                                                              double spotFX,
                                                              const AQLString& deltaType,
                                                              const AQLString& atmType,
                                                              const AQLPriceDataCalendar& cal )
{
    FXOptionData x;

    AQLString daycount(AC_365I);
    //DOMESTICCURVEID
	AQLMathYieldCurve dcurve(&dataInstance);
	dcurve.getYieldData().convertFromString(dCurveID);	
	dcurve.setInterpolation("fn_splineinterpolation");
    dcurve.getDayCount().setDayCount(daycount);
    const AQLDate asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance.getObjectPool().getObject(dCurveID,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
	
	//FOREIGNCURVEID
	AQLMathYieldCurve fcurve(&dataInstance);
	fcurve.getYieldData().convertFromString(fCurveID);	
	fcurve.setInterpolation("fn_splineinterpolation");
    fcurve.getDayCount().setDayCount(daycount);
    const AQLDate& asOfDate2 = dynamic_cast<const AQLDataDate& >(dataInstance.getObjectPool().getObject(fCurveID,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
    if( asOfDate != asOfDate2 )
    {
        AQLString msg("AsOfDates are not equal!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
	AQLString blackdaycount(AC_365I);
	x.T = AQLMathDateUtilities::getTerm(asOfDate,maturityDate,blackdaycount,true);
    double Td = AQLMathDateUtilities::getTerm(asOfDate,deliveryDate,daycount,true);
    x.Pd = dcurve.getBasisDF(Td);
    x.Pf = fcurve.getBasisDF(Td);
    x.F = spotFX * x.Pf / x.Pd;
    x.spotFX = spotFX;

    if( deltaType == "FWDNONPRE" )  { x.deltaType = FWD_NONPRE;}
    else if( deltaType == "FWDPRE" )  { x.deltaType = FWD_PRE;}
    else if( deltaType == "SPOTNONPRE" )  { x.deltaType = SPOT_NONPRE;}
    else if( deltaType == "SPOTPRE" )  { x.deltaType = SPOT_PRE;}
    else 
    {
        AQLString msg("DeltaType is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };
    
    if( atmType == "FORWARDATM" )  { x.atmType = FORWARDATM;}
    else if( atmType == "SPOTATM" )  { x.atmType = SPOTATM;}
    else if( atmType == "DELTANEUTRALNONPRE" )  { x.atmType = DELTANEUTRAL_NONPRE;}
    else if( atmType == "DELTANEUTRALPRE" )  { x.atmType = DELTANEUTRAL_PRE;}
    else 
    {
        AQLString msg("FXATMStrikeType is not supported!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };

    x.Days = AQLMathDateUtilities::getExcelDate( maturityDate ) - AQLMathDateUtilities::getExcelDate( asOfDate )
        -cal.getCalendar().countHoliday(maturityDate, asOfDate);

    if( ( x.deltaType == FWD_NONPRE && x.atmType == DELTANEUTRAL_PRE ) || 
        ( x.deltaType == FWD_PRE && x.atmType == DELTANEUTRAL_NONPRE ) )
    {
        AQLString msg("Option type is PRE or NONPRE!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    };

    return x;
};

//FXOptionData AQLMathFXVolatilitySurfaceGenerate::SetFXOptionParam( AQLDataInstance& dataInstance,
//                                                              const AQLString& dCurveID,
//                                                              const AQLString& fCurveID,
//                                                              double termPoint,
//                                                              double spotFX,
//                                                              ATMInterpolationMethod method,
//                                                              const AQLString& spotLag,
//                                                              const AQLString& calendar )
//{
//    FXOptionData x;
//
//    AQLString daycount(AC_365I);
//    AQLString fol(FOL);
//    //DOMESTICCURVEID
//	AQLMathYieldCurve dcurve(&dataInstance);
//	dcurve.getYieldData().convertFromString(dCurveID);	
//	dcurve.setInterpolation("fn_splineinterpolation");
//    dcurve.getDayCount().setDayCount(daycount);
//    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance.getObjectPool().getObject(dCurveID,ENCHKTYPE_ISDEFINED).
//                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//	
//	//FOREIGNCURVEID
//	AQLMathYieldCurve fcurve(&dataInstance);
//	fcurve.getYieldData().convertFromString(fCurveID);	
//	fcurve.setInterpolation("fn_splineinterpolation");
//    fcurve.getDayCount().setDayCount(daycount);
//    const AQLDate& asOfDate2 = dynamic_cast<const AQLDataDate& >(dataInstance.getObjectPool().getObject(fCurveID,ENCHKTYPE_ISDEFINED).get().
//                        getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//    if( asOfDate != asOfDate2 )
//    {
//        AQLString msg("AsOfDates are not equal!");
//	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
//    }
//
//    AQLDate matuDate,deliDate;
//    AQLPriceDataCalendar cal;
//    cal.convertFromString(calendar);
//    switch (method)
//	{
//	case TermWeighted:
//		x.T = termPoint;
//		break;		
//	case DailyWeighted:
//        matuDate = cal.getBusinessDay(asOfDate, static_cast<int>(termPoint));
//        deliDate = AQLMathDateUtilities::getDate(maturityDate,spotLag,fol,cal);
//        x.T = AQLMathDateUtilities::getTerm(asOfDate,matuDate,daycount,true);
//        x.Days = termPoint;
//		break;
//	case SquareDailyWeighted:
//        matuDate = cal.getBusinessDay(asOfDate, static_cast<int>(termPoint));
//        deliDate = AQLMathDateUtilities::getDate(maturityDate,spotLag,fol,cal);
//        x.T = AQLMathDateUtilities::getTerm(asOfDate,matuDate,daycount,true);
//        x.Days = termPoint;
//		break;
//    case TermNoWeighted:
//		x.T = termPoint;
//        break;
//	default:
//        AQLString msg("ATMInterpolationMethodType is not supported");
//		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
//	}
//
//    x.Pd = dcurve.getBasisDF(x.T);
//    x.Pf = fcurve.getBasisDF(x.T);
//    x.F = spotFX * x.Pf / x.Pd;
//
//    return x;
//};

void AQLMathFXVolatilitySurfaceGenerate::SetSmileParam( AQLStringVector& str, SmileParam& x )
{
    if( str.size() < 5 )
    {
        AQLString msg("SmileParam Size is not 5!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    x.atmVol = AQLCoreUtility::changeDoubleFromString(str)[0]/100;
    x.highRR = AQLCoreUtility::changeDoubleFromString(str)[1]/100;
    x.highBF = AQLCoreUtility::changeDoubleFromString(str)[2]/100;
    x.lowRR = AQLCoreUtility::changeDoubleFromString(str)[3]/100;
    x.lowBF = AQLCoreUtility::changeDoubleFromString(str)[4]/100;
};

double AQLMathFXVolatilitySurfaceGenerate::GetATMStrike( double V, const FXOptionData& x )
{
    //error check
    if( V < 0 ) 
    {
        AQLString msg("V negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double ret;
    switch (x.atmType)
		{
		case FORWARDATM:
			ret = x.F;
			break;		
		case SPOTATM:
		    ret = x.spotFX;
			break;
		case DELTANEUTRAL_NONPRE:
            ret = x.F * AQLMath::exp( 0.5 * V * V * x.T );
			break;
        case DELTANEUTRAL_PRE:
            ret = x.F * AQLMath::exp( -0.5 * V * V * x.T );
			break;
		default:
            AQLString msg("ATMStrikeType is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

    return ret;
};                      

double AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( double K,
                                                          double V,
                                                          int sgn,
                                                          const FXOptionData& x )
{
    //error check
    if( sgn != -1 && sgn != 1 )
    {
        AQLString msg("sgn is not 1 or -1!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if( K < 0 ) 
    {
        AQLString msg("K negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if( V < 0 ) 
    {
        AQLString msg("V negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double std_Dev = V * sqrt( x.T );
    double d_plus = AQLMath::log( x.F / K ) / std_Dev + 0.5 * std_Dev;
    double d_minus = AQLMath::log( x.F / K ) / std_Dev - 0.5 * std_Dev;

    return x.Pd * ( sgn * x.F * AQLDist::normsdist( sgn * d_plus ) - sgn * K * AQLDist::normsdist( sgn * d_minus ) );
};

double AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( double T,
                                                          double F,
                                                          double Pd,
                                                          double K,
                                                          double V,
                                                          int sgn)
{
    //error check
    if( sgn != -1 && sgn != 1 )
    {
        AQLString msg("sgn is not 1 or -1!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if( T<0 || F<0 || Pd<0 || K<0 || V<0 ) 
    {
        AQLString msg("Parameter negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double std_Dev = V * sqrt( T );
    double d_plus = AQLMath::log( F / K ) / std_Dev + 0.5 * std_Dev;
    double d_minus = AQLMath::log( F / K ) / std_Dev - 0.5 * std_Dev;

    return Pd * ( sgn * F * AQLDist::normsdist( sgn * d_plus ) - sgn * K * AQLDist::normsdist( sgn * d_minus ) );
};

/*!
    @brief the operator returns the derivation of forward FX delta with premium 
*/
class MinimumCallDeltaLogStrikeFunc : public AQLFunction
{
public:
    MinimumCallDeltaLogStrikeFunc( double V_, const FXOptionData& x_ )
        :  V(V_), x(x_) {};
    virtual ~MinimumCallDeltaLogStrikeFunc(){};
    double operator()(double y) const // y is logstrike
    { 
        double std_Dev = V * sqrt( x.T );
        double d_minus = ( -y ) / std_Dev- 0.5 * std_Dev;
        return AQLDist::normsdist(d_minus) / ( AQLMath::exp( - d_minus * d_minus / 2 ) / AQLMath::sqrt( 2 * AQLMath::pi() ) / std_Dev ) - 1.; 
    };
private:
    double V;
    FXOptionData x;
};

/*!
    @brief function to get logstrike whose delta call is minimum.
*/
double AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( double V, const FXOptionData& x )
{
    //error check
    if( V < 0 ) {
        AQLString msg("V negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
     
    MinimumCallDeltaLogStrikeFunc sub_func( V, x);

    double upper = 1.;
    double lower = -1.;
    size_t i=0;

    for(i=0;i<40;i++) 
    {
        if( sub_func( upper )  > 0.000001 ) upper *= 2.;
        //else if( AQLMath::abs( sub_func( upper ) ) <= 0.000001 || AQLTime::isError( sub_func( upper )  ) == ERMATHNAN_ ) upper /= 1.5;
		// fixed INF error
		else if( AQLMath::abs( sub_func( upper ) ) <= 0.000001 || AQLTime::isError( sub_func( upper )  ) == ERMATHNAN_ || AQLTime::isError( sub_func( upper )  ) == ERMATHINF_ ) upper /= 1.5;
        else break;
    }

    for(i=0;i<40;i++)
    {
        if( sub_func( lower )  < -0.000001  )  lower *= 2.;
        //else if( AQLMath::abs( sub_func( lower ) ) <= 0.000001 || AQLTime::isError( sub_func( upper ) ) == ERMATHNAN_ ) lower /= 1.5;
		// fixed INF error
		else if( AQLMath::abs( sub_func( lower ) ) <= 0.000001 || AQLTime::isError( sub_func( upper ) ) == ERMATHNAN_ || AQLTime::isError( sub_func( upper ) ) == ERMATHINF_ ) lower /= 1.5;
        else break;
    }
    return sub_func.SolveBR( lower , upper , 10000, 1.0e-8 );
};   

double AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( double k,
                                                        double V,
                                                        int sgn,
                                                        const FXOptionData& x )
{
     //error check
    if( sgn != -1 && sgn != 1 ) 
    {
        AQLString msg("sgn is not 1 or -1!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if( V < 0 ) 
    {
        AQLString msg("V negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double std_Dev = V * sqrt( x.T );
    double d_plus = ( -k ) / std_Dev + 0.5 * std_Dev;
    double d_minus = ( -k ) / std_Dev- 0.5 * std_Dev;
    
    double ret;
    switch (x.deltaType)
		{
		case FWD_NONPRE:
			ret = sgn * AQLDist::normsdist( sgn * d_plus );
			break;		
		case FWD_PRE:
		    ret = sgn * AQLDist::normsdist( sgn * d_minus ) * exp( k ) ;
			break;
		case SPOT_NONPRE:
            ret = sgn * AQLDist::normsdist( sgn * d_plus ) * x.Pf;
			break;
        case SPOT_PRE:
            ret = sgn * AQLDist::normsdist( sgn * d_minus ) * exp( k ) * x.Pf;
			break;
		default:
            AQLString msg("DeltaType is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

    return ret;
};

class FindLogStrikeFromDeltaFunc : public AQLFunction
{
public:
    FindLogStrikeFromDeltaFunc( double delta_, double V_, int sgn_ , const FXOptionData& x_ )
        : delta(delta_), V(V_), sgn(sgn_), x(x_) {};
    virtual ~FindLogStrikeFromDeltaFunc(){};

    double operator()(double y) const 
    { 
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( y, V, sgn, x ) / delta - 1.0; 
    };
private:
    double delta;
    double V;
    int sgn;
    FXOptionData x;
};

double AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta( double delta, 
                                                              double V,
                                                              int sgn,
                                                              const FXOptionData& x )
{   
    //error check
    if( sgn != -1 && sgn != 1 )
    {
        AQLString msg("sgn is not 1 or -1!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if ( ( sgn == -1 && delta >= 0. ) || ( sgn == 1 && delta <= 0. ) )
    {
        AQLString msg("sign of delta is absurd!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if( V < 0 ) 
    {
        AQLString msg("V negative!");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    FindLogStrikeFromDeltaFunc sub_func(delta, V, sgn, x);

    double lower, upper;
    size_t i=0;

    if ( sgn == 1 ) 
    {
        if( x.deltaType == FWD_NONPRE || x.deltaType == SPOT_NONPRE ) 
        {
            lower = -100.;
            upper = 100.;

            for(i=0;i<20;i++) 
            {
                if( sub_func( lower )  < 0. )  lower *= 2.0;
                else if( AQLMath::abs( sub_func( lower ) + 1. ) < 0.000001 )  lower /= 1.5;
                else break;
            }

            for(i=0;i<20;i++)
            {
                if( sub_func( upper )  > 0.  )  upper *= 2.0;
                else if( AQLMath::abs( sub_func( upper ) + 1. ) < 0.000001 )  upper /= 1.5;
                else break;
            };
        }
        else 
        {
            lower = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( V, x );
            upper = 100.;

            if( delta > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( lower, V, 1, x ) )
            {
                AQLString msg("call delta is over limit!");
	            throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
            }

            for(i=0;i<20;i++)
            {
                if( sub_func( upper ) > 0. )  upper *= 2.0;
                else if( AQLMath::abs( sub_func( upper ) + 1. ) < 0.000001 )  upper /= 1.5;
                else break;
            }          
        }
    }
    else
    {
        lower = -20.;
        upper = 20.;

        for(i=0;i<20;i++) 
        {
            if( sub_func( lower ) > 0. )  lower *= 2.0;
            else if( AQLMath::abs( sub_func( lower ) + 1. ) < 0.0000001 )  lower /= 1.5;
            else break;
        }

        for(i=0;i<20;i++)
        {
            if( sub_func( upper )  < 0.  )  upper *= 2.0;
            else if( AQLMath::abs( sub_func( upper ) + 1. ) < 0.0000001 )  upper /= 1.5;
            else break;
        }
    }

    return sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
};

/*!
	    @param[in] impliedVol[1]    LowPutVol
        @param[in] impliedVol[2]    HighPutVol
	    @param[in] impliedVol[3]    ATMVol
        @param[in] impliedVol[4]    HighCallVol
        @param[in] impliedVol[5]    LowCallVol
        @param[in] wf       WingFactor

        @return 
    */
void AQLMathFXVolatilitySurfaceGenerate::GetWingSmile( DoubleVector& impliedVol, 
                                                  double atmDelta, 
                                                  double wf )
{   
    atmDelta *= -100.;
    double alpha, beta, RR, BF, tmp1, tmp2, tmp3;

    if( wf < 0 )
    {
         impliedVol[0] = ( (10 - 0.01) * impliedVol[2] + (25 - 0.01) * impliedVol[1] ) / 15;
         impliedVol[6] = ( (10 - 0.01) * impliedVol[4] + (25 - 0.01) * impliedVol[5] ) / 15;
    }
    else
    {
        tmp1 = impliedVol[4] - impliedVol[2];
        tmp2 = impliedVol[5] - impliedVol[1];
        //error check
        if( tmp1 * tmp2 <= 0.0 ) 
        {
            AQLString msg("signs of risk reversal are not same!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        tmp3 = AQLMath::abs( atmDelta - 25 ) / AQLMath::abs( atmDelta - 10 );
        alpha = log( tmp1 / tmp2 ) / log( tmp3 );

        RR = ( impliedVol[4] - impliedVol[2] ) / AQLMath::pow( AQLMath::abs( atmDelta - 25 ), alpha );

        tmp1 = ( impliedVol[4] + impliedVol[2] ) / 2 - impliedVol[3];
        tmp2 = ( impliedVol[5] + impliedVol[1] ) / 2 - impliedVol[3];
        //error check
        if( tmp1 * tmp2 <= 0.0 ) 
        {
            AQLString msg("signs of butterfly are not same!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        tmp3 = AQLMath::abs( atmDelta - 25 ) / AQLMath::abs( atmDelta - 10 );
        beta = log( tmp1 / tmp2 ) / log( tmp3 );

        BF = ( ( impliedVol[4] + impliedVol[2] ) / 2 - impliedVol[3] ) / AQLMath::pow( AQLMath::abs( atmDelta - 25 ), beta );

        impliedVol[0] = impliedVol[3] - 0.5 * RR * AQLMath::pow( AQLMath::abs( atmDelta - 0.01 ), alpha ) 
                                    + wf * BF * AQLMath::pow( AQLMath::abs( atmDelta - 0.01 ), beta );
        impliedVol[6] = impliedVol[3] + 0.5 * RR * AQLMath::pow(AQLMath::abs( atmDelta - 0.01 ), alpha ) 
                                    + wf * BF * AQLMath::pow( AQLMath::abs( atmDelta - 0.01 ), beta );
    }
};

SmileData AQLMathFXVolatilitySurfaceGenerate::BuildSmile( const SmileParam& y,
                                                     const FXOptionData& x, 
													 bool isWing,
                                                     double wf,
                                                     AQLString& warningMSG )
{
    SmileData z;
    z.logStrikes.resize(7);
    z.vols.resize(7);
    z.deltaPuts.resize(7);
    z.strikes.resize(7);

    z.vols[1] = y.atmVol - 0.5 * y.lowRR + y.lowBF;
    z.vols[2] = y.atmVol - 0.5 * y.highRR + y.highBF;
    z.vols[3] = y.atmVol;
    z.vols[4] = y.atmVol + 0.5 * y.highRR + y.highBF;
    z.vols[5] =  y.atmVol + 0.5 * y.lowRR + y.lowBF;

    z.logStrikes[3] = AQLMath::log( GetATMStrike( y.atmVol , x ) / x.F );
    z.deltaPuts[3] = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z.logStrikes[3], y.atmVol, -1, x );
	if( isWing ) GetWingSmile( z.vols, z.deltaPuts[3], wf );

    z.logStrikes[0] = FindLogStrikeFromDelta( -0.0001, z.vols[0], -1, x );
    z.logStrikes[1] = FindLogStrikeFromDelta( -0.1, z.vols[1], -1, x );
    z.logStrikes[2] = FindLogStrikeFromDelta( -0.25, z.vols[2], -1, x );

    double minLogStrike;

    minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vols[4], x );
    if( 0.25 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[4], 1, x ) && isErrorMessage == true )
    {
		AQLString msg("max high call delta is under 0.25!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    else if( 0.25 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[4], 1, x ) && isErrorMessage == false )
    {
        z.logStrikes[4] = minLogStrike;
        warningMSG  += AQLString( x.T, 3 ) + " high call delta under 0.25,";
    }
    else
    {      
        z.logStrikes[4] = FindLogStrikeFromDelta( 0.25, z.vols[4], 1, x );
    }

    minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vols[5], x );
    if( 0.1 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[5], 1, x ) && isErrorMessage == true )
    {
		AQLString msg("max low call delta is under 0.1!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    else if( 0.1 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[5], 1, x ) && isErrorMessage == false )
    {
        z.logStrikes[5] = minLogStrike;
        warningMSG  += AQLString( x.T, 3 ) + " low call delta under 0.1,";
    }
    else
    {      
        z.logStrikes[5] = FindLogStrikeFromDelta( 0.1, z.vols[5], 1, x );
    }

	if( isWing )
	{
		minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vols[6], x );
		if( 0.0001 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[6], 1, x ) && isErrorMessage == true )
		{
			AQLString msg("max wing call delta is under 0.0001!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
		}
        else if( 0.0001 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[6], 1, x ) && isErrorMessage == false )
        {
            z.logStrikes[6] = minLogStrike;
            warningMSG  += AQLString( x.T, 3 ) + " wing call delta under 0.001,";
        }
		else
		{      
			z.logStrikes[6] = FindLogStrikeFromDelta( 0.0001, z.vols[6], 1, x );
		}
	}

    if( isWing ) z.deltaPuts[0] = -0.0001;
    z.deltaPuts[1] = -0.1;
    z.deltaPuts[2] = -0.25;
    z.deltaPuts[4] = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z.logStrikes[4], z.vols[4], -1, x );
    z.deltaPuts[5] = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z.logStrikes[5], z.vols[5], -1, x );
    if( isWing ) z.deltaPuts[6] = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z.logStrikes[6], z.vols[6], -1, x );

    //set strike
    for(size_t i=0; i<7; i++)
    {
        z.strikes[i] = exp( z.logStrikes[i] ) * x.F;
    }

    if( isWing == true )
    {
        return z;
    }
    else
    {
        z.deltaPuts.pop_back();
        z.logStrikes.pop_back();
        z.strikes.pop_back();
        z.vols.pop_back();
        z.deltaPuts.erase( z.deltaPuts.begin() );     
        z.logStrikes.erase( z.logStrikes.begin() );     
        z.strikes.erase( z.strikes.begin() );      
        z.vols.erase( z.vols.begin() );

        return z;
    }
};

void AQLMathFXVolatilitySurfaceGenerate::SmileDataCheck( SmileData& x, bool isWing )
{
    if( isWing )
    {
        if( x.deltaPuts.size() != 7 ||  x.logStrikes.size() != 7 || x.vols.size() != 7 || x.strikes.size() != 7 )
        {
            AQLString msg("SmileData size is not 7!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }
    else
    {
        if( x.deltaPuts.size() != 5 ||  x.logStrikes.size() != 5 || x.vols.size() != 5 || x.strikes.size() != 5 )
        {
            AQLString msg("SmileData size is not 5!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }
    
    size_t i;
    for( i=0;i<x.vols.size();i++ )
    {
        if( x.vols[i] < 0. )
        {
            AQLString msg("Volatility Negative!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        if( x.deltaPuts[i] > 0. )
        {
            AQLString msg("DeltaPut Positive!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }

    for( i=0;i<x.vols.size()-1;i++ )
    {
        if( x.logStrikes[i+1] - x.logStrikes[i] < 0. )
        {
            AQLString msg("LogStrikes decrease ");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        if( x.deltaPuts[i+1] - x.deltaPuts[i] > 0. )
        {
            AQLString msg("deltaPuts increase ");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }

    if( isWing )
    {
        if( x.deltaPuts[0] != -0.0001 || x.deltaPuts[1] != -0.1 || x.deltaPuts[2] != -0.25 )
        {
            AQLString msg("deltaPuts is not satisfied the condition {-0.0001,-0.1,-0.25,....}!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }
    else
    {
        if( x.deltaPuts[0] != -0.1 || x.deltaPuts[1] != -0.25 )
        {
            AQLString msg("deltaPuts is not satisfied the condition {-0.0001,-0.1,-0.25,....}!");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }
};

bool AQLMathFXVolatilitySurfaceGenerate::SmileDataCheck_Bool( const SmileParam& y,
														 const FXOptionData& x,
                                                         double wf )
{
    SmileData z;
    z.logStrikes.resize(7);
    z.vols.resize(7);
    z.deltaPuts.resize(7);

    z.vols[1] = y.atmVol - 0.5 * y.lowRR + y.lowBF;
    z.vols[2] = y.atmVol - 0.5 * y.highRR + y.highBF;
    z.vols[3] = y.atmVol;
    z.vols[4] = y.atmVol + 0.5 * y.highRR + y.highBF;
    z.vols[5] = y.atmVol + 0.5 * y.lowRR + y.lowBF;

    z.logStrikes[3] = AQLMath::log( GetATMStrike( y.atmVol , x ) / x.F );
    z.deltaPuts[3] = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z.logStrikes[3], y.atmVol, -1, x );
    
    GetWingSmile( z.vols, z.deltaPuts[3], wf );

	if( z.vols[0] < 0.0 || z.vols[6] < 0.0 ) return true;

    z.logStrikes[0] = FindLogStrikeFromDelta( -0.0001, z.vols[0], -1, x );
    z.logStrikes[1] = FindLogStrikeFromDelta( -0.1, z.vols[1], -1, x );

    double minLogStrike;

    minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vols[5], x );
    if( 0.1 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[5], 1, x ) )
    {
		z.logStrikes[5] = minLogStrike;
    }
    else
    {      
        z.logStrikes[5] = FindLogStrikeFromDelta( 0.1, z.vols[5], 1, x );
    }

    minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vols[6], x );
    if( 0.0001 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vols[6], 1, x ) )
    {
		z.logStrikes[6] = minLogStrike;
    }
    else
    {      
        z.logStrikes[6] = FindLogStrikeFromDelta( 0.0001, z.vols[6], 1, x );
    }

	if( z.logStrikes[1] - z.logStrikes[0] < 0.0 || z.logStrikes[6] - z.logStrikes[5] < 0.0 ) return true;

	
    z.deltaPuts[6] = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z.logStrikes[6], z.vols[6], -1, x );

	if( z.deltaPuts[5] - z.deltaPuts[6] < 0.0 ) return true;

	return false;
};

double AQLMathFXVolatilitySurfaceGenerate::Interpolate( DoubleArray& array1,
                                                   DoubleArray& array2,
                                                   double point,
                                                   const InterpolationMethod& method,
                                                   bool isReverse )
{
    size_t n = array1.size();
    double ret;

    switch(method)
    {
        case SPLINE_FXVOL:
            if( isReverse == false )
            {
                ret = AQLMathInterpolationUtilities::spline(array1, array2, point);
            }
            else
            {
                ret = AQLMathInterpolationUtilities::splineReverse(array1, array2, point);
            }
            break;
        case CONSTRAINEDSPLINE_FXVOL:
            if( isReverse == false )
            {
                ret = AQLMathInterpolationUtilities::constrainedSpline( array1, array2, point);
            }
            else
            {
                ret = AQLMathInterpolationUtilities::constrainedSplineReverse( array1, array2, point);
            }
            break;
        case LINEAR_FXVOL:
            if( isReverse == false )
            {
                ret = AQLMathInterpolationUtilities::linear(array1,array2,point);
            }
            else
            {
                ret = AQLMathInterpolationUtilities::linearReverse(array1,array2,point);
            }
            break;
        default:
                AQLString msg("InterpolationMethod is not supported");
			    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
    }

    if(ret<0.) ret=0.001;
    return ret;
};

class DeltaPutDeltaCallPreFunc : public AQLFunction
{
public:
    DeltaPutDeltaCallPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_), method(method_), x(x_), y(y_){};
    virtual ~DeltaPutDeltaCallPreFunc(){};
    double operator()(double z) const//z is delta put
    { 
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.deltaPuts, y.vols, z, method, true );
        double k = AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta( z, V, -1, x );
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( k,  V, 1, x ) / point - 1.0; 
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class DeltaPutLogStrikePreFunc : public AQLFunction
{
public:
    DeltaPutLogStrikePreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~DeltaPutLogStrikePreFunc(){};
    double operator()(double z) const//z is delta put
    { 
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.deltaPuts, y.vols, z, method, true );
        return AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta( z, V, -1, x ) / point - 1.;                    
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class LogStrikeDeltaCallFwdNonPreFunc : public AQLFunction
{
public:
    LogStrikeDeltaCallFwdNonPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_)
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~LogStrikeDeltaCallFwdNonPreFunc(){};
    double operator()(double z) const//z is logStrike
    { 
        double d_plus = AQLDist::invNormdist( point );
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.logStrikes, y.vols, z, method );
        double std_Dev =  V * AQLMath::sqrt( x.T );
        return ( ( -z ) / std_Dev + 0.5 * std_Dev )  / d_plus - 1.0; 
    }
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class LogStrikeDeltaCallSpotNonPreFunc : public AQLFunction
{
public:
    LogStrikeDeltaCallSpotNonPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~LogStrikeDeltaCallSpotNonPreFunc(){};
    double operator()(double z) const//z is logStrike
    { 
        double d_plus = AQLDist::invNormdist( point / x.Pf );
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.logStrikes, y.vols, z, method );
        double std_Dev =  V * AQLMath::sqrt( x.T );
        return ( ( -z ) / std_Dev + 0.5 * std_Dev )  / d_plus - 1.0; 
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class LogStrikeDeltaCallPreFunc : public AQLFunction
{
public:
    LogStrikeDeltaCallPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~LogStrikeDeltaCallPreFunc(){};
    double operator()(double z) const//z is logStrike
    { 
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.logStrikes, y.vols, z, method );
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z, V, 1, x ) / point - 1.0; 
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class LogStrikeDeltaPutFunc : public AQLFunction
{
public:
    LogStrikeDeltaPutFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_),method(method_), x(x_), y(y_) {};
    virtual ~LogStrikeDeltaPutFunc(){};
    double operator()(double z) const//z is logStrike
    { 
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.logStrikes, y.vols, z, method );
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( z, V, -1, x ) / point - 1.0;                             
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class StrikeDeltaCallFwdNonPreFunc : public AQLFunction
{
public:
    StrikeDeltaCallFwdNonPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_)
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~StrikeDeltaCallFwdNonPreFunc(){};
    double operator()(double z) const//z is strike
    { 
        double d_plus = AQLDist::invNormdist( point );
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.strikes, y.vols, z, method );
        double std_Dev =  V * AQLMath::sqrt( x.T );
        return ( ( -log( z / x.F ) ) / std_Dev + 0.5 * std_Dev )  / d_plus - 1.0; 
    }
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class StrikeDeltaCallSpotNonPreFunc : public AQLFunction
{
public:
    StrikeDeltaCallSpotNonPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~StrikeDeltaCallSpotNonPreFunc(){};
    double operator()(double z) const//z is strike
    { 
        double d_plus = AQLDist::invNormdist( point / x.Pf );
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.strikes, y.vols, z, method );
        double std_Dev =  V * AQLMath::sqrt( x.T );
        return ( ( -log( z / x.F ) ) / std_Dev + 0.5 * std_Dev )  / d_plus - 1.0; 
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class StrikeDeltaCallPreFunc : public AQLFunction
{
public:
    StrikeDeltaCallPreFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_), method(method_), x(x_), y(y_) {};
    virtual ~StrikeDeltaCallPreFunc(){};
    double operator()(double z) const//z is strike
    { 
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.strikes, y.vols, z, method );
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( log( z / x.F ), V, 1, x ) / point - 1.0; 
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

class StrikeDeltaPutFunc : public AQLFunction
{
public:
    StrikeDeltaPutFunc( double point_, const InterpolationMethod& method_, const FXOptionData& x_, SmileData& y_ )
        : point(point_),method(method_), x(x_), y(y_) {};
    virtual ~StrikeDeltaPutFunc(){};
    double operator()(double z) const//z is strike
    { 
        double V = AQLMathFXVolatilitySurfaceGenerate::Interpolate( y.strikes, y.vols, z, method );
        return AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( log( z / x.F ), V, -1, x ) / point - 1.0;                             
    };
private:
    double point;
    InterpolationMethod method;
    FXOptionData x;
    SmileData& y;
};

double AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( double point,
                                                             const InterpolationMethod& method,
                                                             const InterpolationTarget& target,
                                                             const InterpolationVariable& variable,
                                                             const FXOptionData& x,
                                                             SmileData& y,
                                                             bool isWing )
{
    //check atmpoint
	unsigned int atmpoint = 0;
	if ( isWing ) atmpoint = 3;
	else atmpoint = 2;

    double ret;
    if(target == TargetDeltaPut)
    {
        switch (variable)
		    {
		    case VariableDeltaPut:
                //error check
                if( point >= 0. )
                {
                    AQLString msg("DeltaPut is not positive!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                }

                if( point >= y.deltaPuts.front() && isWing == true )
                {
                    ret = y.vols.front();
                }
                else if( point <= y.deltaPuts.back() && isWing == true )
                {
                    ret = y.vols.back();
                }
                else
                {
                    ret = Interpolate( y.deltaPuts, y.vols, point, method, true );
                }
                //ret = Interpolate( y.deltaPuts, y.vols, point, method, true );
			    break;		
		    case VariableDeltaCall:
		        //error check
                if( point <= 0. )
                {
                    AQLString msg("DeltaCall is not negative!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                }
                
                if( x.deltaType == FWD_NONPRE )
                {
                    //error check
                    if( point >= 1. )
                    {
                        AQLString msg("DeltaCall is not over 1!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    double point_Put = point - 1.;
                    ret = Interpolate( y.deltaPuts, y.vols, point_Put, method, true );
                }
                else if( x.deltaType == SPOT_NONPRE )
                {
                    //error check
                    if( point >= x.Pf )
                    {
                        AQLString msg("DeltaCall is not over Pf!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    double point_Put = point - x.Pf;
                    ret = Interpolate( y.deltaPuts, y.vols, point_Put, method, true );
                }
                else
                {
                    double atmDeltaCall = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta(y.logStrikes[atmpoint], y.vols[atmpoint], 1, x);
                    if( point > atmDeltaCall )
                    {
                        AQLString msg("Point is over atmDeltaCall!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    if( isWing )
                    {
                        if( point < -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else
                        {
                            DeltaPutDeltaCallPreFunc sub_func(point, method, x, y );
                            double x_Solve = sub_func.SolveBR( y.deltaPuts.back() - EPS_Vol, y.deltaPuts[atmpoint] + EPS_Vol, 10000, 1.0e-8 );
                            ret = Interpolate( y.deltaPuts, y.vols, x_Solve, method, true );
                        }
                    }
                    else
                    {
                        DeltaPutDeltaCallPreFunc sub_func(point, method, x, y );
                        double lower = y.deltaPuts.back() - EPS_Vol;
                        double upper = y.deltaPuts[atmpoint] + EPS_Vol;
                        for(size_t j=0;j<20;j++)
                        {
                            if( sub_func( lower ) > 0. ) lower *= 2.;
                            else break;
                        }        
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.deltaPuts, y.vols, x_Solve, method, true );
                    }
                }
			    break;
		    case VariableLogStrike:
                if( isWing )
                {
                    if( point <= y.logStrikes.front() )
                    {
                        ret = y.vols.front();
                    }
                    else if( point >= y.logStrikes.back() )
                    {
                        ret = y.vols.back();
                    }
                    else
                    {
					    size_t i = 1;
					    for( i=1;i<y.logStrikes.size()-1;i++ )
					    {
						    if( (y.logStrikes[i-1]<=point) && (point<y.logStrikes[i]) ) break;
					    }
					    if( AQLMath::abs(y.logStrikes[i-1]-point) < 0.000001 )
					    {
						    ret = y.vols[i-1];
						    break;
					    }
					    else if( AQLMath::abs(y.logStrikes[i]-point) < 0.000001 )
					    {
						    ret = y.vols[i];
						    break;
					    }
					    double upper = y.deltaPuts[i-1];
					    double lower = y.deltaPuts[i];
					    DeltaPutLogStrikePreFunc sub_func( point, method, x, y );
                        double x_Solve = sub_func.SolveBR( lower, upper , 10000, 1.0e-8 );               
                        ret = Interpolate( y.deltaPuts, y.vols, x_Solve, method, true);
                    }
                }
                else
                {
                    DeltaPutLogStrikePreFunc sub_func( point, method, x, y );
                    double upper,lower;
                    if( point <= y.logStrikes.front() )
                    {
                        upper = y.deltaPuts.front() / 2.;
					    lower = y.deltaPuts.front() - 0.000001;
                        for(size_t j=0; j<20; j++)
                        {
                            if( sub_func( upper )  < 0.  )  upper /= 2.0;
                            else break;
                        }
                    }
                    else if( point >= y.logStrikes.back() )
                    {
                        upper = y.deltaPuts.back() + 0.000001;
                        if( x.deltaType == FWD_NONPRE )
                        {
                            lower = -1. + 0.0001;
                        }
                        else if( x.deltaType == SPOT_NONPRE ) 
                        {
                            lower = - x.Pf + 0.0001;
                        }
                        else
                        {
					        lower = y.deltaPuts.back() * 2.;
                            for(size_t j=0; j<20; j++) 
                            {
                                if( sub_func( lower )  < 0. )  lower *= 2.0;
                                else break;
                            }
                        }
                    }
                    else
                    {
                        size_t i=1;
                        for(i=1; i<y.logStrikes.size(); i++)
                        {
                            if( (y.logStrikes[i-1]<=point) && (point<y.logStrikes[i]) ) break;
					    }
                        upper = y.deltaPuts[i-1] + 0.000001;
					    lower = y.deltaPuts[i] - 0.000001;
                    }
                    double x_Solve = sub_func.SolveBR( lower, upper , 10000, 1.0e-8 );
                    ret = Interpolate( y.deltaPuts, y.vols, x_Solve, method, true);
                }
			    break;
		    default:
                AQLString msg("InterpolationVariable is not supported");
			    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		    }
    }
    else if( target == TargetLogStrike )
    {
        switch (variable)
		    {
		    case VariableLogStrike:
                if( point <= y.logStrikes.front() && isWing == true )
                {
                    ret = y.vols.front();
                }
                else if( point >= y.logStrikes.back() && isWing == true )
                {
                    ret = y.vols.back();
                }
                else
                {
                    ret = Interpolate( y.logStrikes, y.vols, point, method );
                }
                //ret = Interpolate( y.logStrikes, y.vols, point, method );
			    break;		
		    case VariableDeltaCall:
                //error check
                if( point <= 0. )
                {
                    AQLString msg("DeltaCall is not negative!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                }
                
                if( x.deltaType == FWD_NONPRE )
                {
                    //error check
                    if( point >= 1. )
                    {
                        AQLString msg("DeltaCall is not over 1!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    double point_Put = point - 1.;
                    if( isWing )
                    {
                        if( point < -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else if( point_Put > y.deltaPuts.front() )
                        {
                            ret = y.vols.front();
                        }
                        else
                        {
                            LogStrikeDeltaCallFwdNonPreFunc sub_func( point, method, x, y );
                            double x_Solve = sub_func.SolveBR( y.logStrikes.front() - EPS_Vol,y.logStrikes.back() + EPS_Vol, 10000, 1.0e-8 );
                            return ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                        }
                    }
                    else
                    {
                        LogStrikeDeltaCallFwdNonPreFunc sub_func( point, method, x, y );
                        double lower = y.logStrikes.front() - 0.01;
                        double upper = y.logStrikes.back() + 0.01;
                        if( point < -y.deltaPuts.front() || point_Put > y.deltaPuts.front()  )
                        {
                            AQLString msg("DeltaCall is over limit!");
	                        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                        }
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                    }
                }
                else if( x.deltaType == SPOT_NONPRE )
                {
                    //error check
                    if( point >= x.Pf )
                    {
                        AQLString msg("DeltaCall is not over Pf!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    double point_Put = point - x.Pf;
                    if( isWing )
                    {
                        if( point <= -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else if( point_Put >= y.deltaPuts.front() )
                        {
                            ret = y.vols.front();
                        }
                        else
                        {
                            LogStrikeDeltaCallSpotNonPreFunc sub_func( point, method, x, y  );
                            double x_Solve = sub_func.SolveBR( y.logStrikes.front() - EPS_Vol, y.logStrikes.back() + EPS_Vol, 10000, 1.0e-8 );
                            return ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                        }
                    }
                    else
                    {
                        LogStrikeDeltaCallSpotNonPreFunc sub_func( point, method, x, y );
                        double lower = y.logStrikes.front() - 0.01;
                        double upper = y.logStrikes.back() + 0.01;
                        if( point < -y.deltaPuts.front() || point_Put > y.deltaPuts.front()  )
                        {
                            AQLString msg("DeltaCall is over limit!");
	                        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                        }
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                    }
                }
                else
                {
                    double atmDeltaCall = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( y.logStrikes[atmpoint] - EPS_Vol, 
                                                    y.vols[atmpoint] + EPS_Vol, 1, x );
                    if( point > atmDeltaCall )
                    {
                        AQLString msg("Point is over atmDeltaCall!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    if( isWing )
                    {
                        if( point < -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else
                        {
                            LogStrikeDeltaCallPreFunc sub_func( point, method, x, y );
                            double x_Solve = sub_func.SolveBR( y.logStrikes[atmpoint] - EPS_Vol, y.logStrikes.back() + EPS_Vol, 10000, 1.0e-8 );
                            ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                        }
                    }
                    else
                    {
                        LogStrikeDeltaCallPreFunc sub_func( point, method, x, y );
                        double upper;
                        if( point < -y.deltaPuts.front() )
                        {
                            upper = y.logStrikes.back() * 2;
                            for(size_t j=0;j<20;j++) 
                            {
                                if( sub_func( upper )  > 0. )  upper *= 2.;
                                else break;
                            }
                        }
                        else
                        {
                            upper = y.logStrikes.back() + 0.01;
                        }
                        double x_Solve = sub_func.SolveBR( y.logStrikes[atmpoint] - 0.01, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                    }
                }
			    break;
		    case VariableDeltaPut:
                //error check
                if( point >= 0. )
                {
                    AQLString msg("DeltaPut is not positive!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                }

                if( isWing )
                {
                    if( point >= y.deltaPuts.front() )
                    {
                        ret = y.vols.front();
                    }
                    else if( point <= y.deltaPuts.back() )
                    {
                        ret = y.vols.back();
                    }
                    else
                    {
					    size_t i = 1;
					    for(i=1;i<y.deltaPuts.size();i++)
					    {
						    if( (y.deltaPuts[i]<=point) && (point<y.deltaPuts[i-1]) ) break;
					    }
					    if( AQLMath::abs(y.deltaPuts[i-1]-point) < 0.000001 )
					    {
						    ret = y.vols[i-1];
						    break;
					    }
					    else if( AQLMath::abs(y.deltaPuts[i]-point) < 0.000001 )
					    {
						    ret = y.vols[i];
						    break;
					    }

					    double upper = y.logStrikes[i];
					    double lower = y.logStrikes[i-1];
                        LogStrikeDeltaPutFunc sub_func( point, method, x, y );     
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                    }
                }
                else
                {
                    LogStrikeDeltaPutFunc sub_func( point, method, x, y );
                    double upper,lower;
                    if( point >= y.deltaPuts.front() )
                    {
                        upper = y.logStrikes.front() + 0.000001;
					    lower = y.logStrikes.front() * 2.;
                        for(size_t j=0;j<20;j++) 
                        {
                            if( sub_func( lower )  > 0. )  lower *= 2.;
                            else break;
                        }
                    }
                    else if( point <= y.deltaPuts.back() )
                    {
                        upper = y.logStrikes.back() * 2;
					    lower = y.logStrikes.back() - 0.000001;  
                        for(size_t j=0;j<20;j++)
                        {
                            if( sub_func( upper )  < 0.  )  upper *= 2.;
                            else break;
                        }                        
                    }
                    else
                    {
                        size_t i = 1;
					    for(i=1;i<y.deltaPuts.size();i++)
					    {
						    if( (y.deltaPuts[i]<=point) && (point<y.deltaPuts[i-1]) ) break;
					    }
                        upper = y.logStrikes[i] + 0.000001;
					    lower = y.logStrikes[i-1]  - 0.000001;
                    }
                    double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                    ret = Interpolate( y.logStrikes, y.vols, x_Solve, method );
                }
			    break;
		    default:
                AQLString msg("InterpolationVariable is not supported");
			    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		    }
    }
    else if( target ==  TargetStrike )
    {
        switch (variable)
		    {
		    case VariableLogStrike:
                if( point <= y.logStrikes.front() && isWing == true )
                {
                    ret = y.vols.front();
                }
                else if( point >= y.logStrikes.back() && isWing == true )
                {
                    ret = y.vols.back();
                }
                else
                {
                    ret = Interpolate( y.strikes, y.vols, exp( point ) * x.F, method );
                }
                /*{
                    ret = Interpolate( y.strikes, y.vols, point, method );
                }*/
			    break;		
		    case VariableDeltaCall:
                //error check
                if( point <= 0. )
                {
                    AQLString msg("DeltaCall is not negative!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                }
                
                if( x.deltaType == FWD_NONPRE )
                {
                    //error check
                    if( point >= 1. )
                    {
                        AQLString msg("DeltaCall is not over 1!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    double point_Put = point - 1.;
                    if( isWing )
                    {
                        if( point <= -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else if( point_Put >= y.deltaPuts.front() )
                        {
                            ret = y.vols.front();
                        }
                        else
                        {
                            StrikeDeltaCallFwdNonPreFunc sub_func( point, method, x, y );
                            double x_Solve = sub_func.SolveBR( y.strikes.front() - 0.01, y.strikes.back() + 0.01, 10000, 1.0e-8 );
                            ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                        }
                    }
                    else
                    {
                        StrikeDeltaCallFwdNonPreFunc sub_func( point, method, x, y  );
                        double lower = y.strikes.front() - 0.01;
                        double upper = y.strikes.back() + 0.01;
                        if( point < -y.deltaPuts.front() || point_Put > y.deltaPuts.front()  )
                        {
                            AQLString msg("DeltaCall is over limit!");
	                        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                        }
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                    }
                }
                else if( x.deltaType == SPOT_NONPRE )
                {
                    //error check
                    if( point >= x.Pf )
                    {
                        AQLString msg("DeltaCall is not over Pf!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    double point_Put = point - x.Pf;
                    if( isWing )
                    {
                        if( point <= -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else if( point_Put >= y.deltaPuts.front() )
                        {
                            ret = y.vols.front();
                        }
                        else
                        {
                            StrikeDeltaCallSpotNonPreFunc sub_func( point, method, x, y  );
                            double x_Solve = sub_func.SolveBR( y.strikes.front() - 0.01, y.strikes.back() + 0.01, 10000, 1.0e-8 );
                            ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                        }
                    }
                    else
                    {
                        StrikeDeltaCallSpotNonPreFunc sub_func( point, method, x, y  );
                        double lower = y.strikes.front() - 0.01;
                        double upper = y.strikes.back() + 0.01;
                        if( point < -y.deltaPuts.front() || point_Put > y.deltaPuts.front()  )
                        {
                            AQLString msg("DeltaCall is over limit!");
	                        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                        }
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                    }
                }
                else
                {
                    double atmDeltaCall = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( y.logStrikes[atmpoint] - EPS_Vol, 
                                                    y.vols[atmpoint] + EPS_Vol, 1, x );
                    if( point > atmDeltaCall )
                    {
                        AQLString msg("Point is over atmDeltaCall!");
	                    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                    }

                    if( isWing )
                    {
                        if( point < -y.deltaPuts.front() )
                        {
                            ret = y.vols.back();
                        }
                        else
                        {
                            StrikeDeltaCallPreFunc sub_func( point, method, x, y  );
                            double x_Solve = sub_func.SolveBR( y.strikes[atmpoint] - 0.01, y.strikes.back() + 0.01, 10000, 1.0e-8 );
                            ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                        }
                    }
                    else
                    {
                        StrikeDeltaCallPreFunc sub_func( point, method, x, y );
                        double upper;
                        if( point < -y.deltaPuts.front() )
                        {
                            upper = y.strikes.back() * 2;
                            for(size_t j=0;j<20;j++) 
                            {
                                if( sub_func( upper )  > 0. )  upper *= 2.;
                                else break;
                            }
                        }
                        else
                        {
                            upper = y.strikes.back() + 0.01;
                        }
                        double x_Solve = sub_func.SolveBR( y.strikes[atmpoint] - 0.01, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                    }
                }
			    break;
		    case VariableDeltaPut:
                //error check
                if( point >= 0. )
                {
                    AQLString msg("DeltaPut is not positive!");
	                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
                }

                if( isWing )
                {
                    if( point >= y.deltaPuts.front() )
                    {
                        ret = y.vols.front();
                    }
                    else if( point <= y.deltaPuts.back() )
                    {
                        ret = y.vols.back();
                    }
                    else
                    {
					    size_t i = 1;
					    for(i=1;i<y.deltaPuts.size();i++)
					    {
						    if( (y.deltaPuts[i]<=point) && (point<y.deltaPuts[i-1]) ) break;
					    }
					    if( AQLMath::abs(y.deltaPuts[i-1]-point) < 0.000001 )
					    {
						    ret = y.vols[i-1];
						    break;
					    }
					    else if( AQLMath::abs(y.deltaPuts[i]-point) < 0.000001 )
					    {
						    ret = y.vols[i];
						    break;
					    }

					    double upper = y.strikes[i];
					    double lower = y.strikes[i-1];
                        StrikeDeltaPutFunc sub_func( point, method, x, y );                    
                        double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                        ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                    }
                }
                else
                {
                    StrikeDeltaPutFunc sub_func( point, method, x, y );
                    double upper,lower;
                    if( point >= y.deltaPuts.front() )
                    {
                        upper = y.strikes.front() + 0.0001;
					    lower = y.strikes.front() * 2.;
                        for(size_t j=0;j<20;j++) 
                        {
                            if( sub_func( lower )  > 0. )  lower *= 2.;
                            else break;
                        }
                    }
                    else if( point <= y.deltaPuts.back() )
                    {
                        upper = y.strikes.back() * 2;
					    lower = y.strikes.back() - 0.0001;  
                        for(size_t j=0;j<20;j++)
                        {
                            if( sub_func( upper )  < 0.  )  upper *= 2.;
                            else break;
                        }                        
                    }
                    else
                    {
                        size_t i = 1;
					    for(i=1;i<y.deltaPuts.size();i++)
					    {
						    if( (y.deltaPuts[i]<=point) && (point<y.deltaPuts[i-1]) ) break;
					    }
                        upper = y.strikes[i] + 0.0001;
					    lower = y.strikes[i-1]  - 0.0001;
                    }
                    double x_Solve = sub_func.SolveBR( lower, upper, 10000, 1.0e-8 );
                    ret = Interpolate( y.strikes, y.vols, x_Solve, method );
                }
			    break;
		    default:
                AQLString msg("InterpolationVariable is not supported");
			    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		    }
        }
        else
        {   
            AQLString msg("InterpolationTarget is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}
    return ret;
};

double AQLMathFXVolatilitySurfaceGenerate::GetModelPrem( const FXOptionData& x,
                                                    SmileData& y,
                                                    const Strangle& z,
                                                    const InterpolationMethod& method,
                                                    const InterpolationTarget& target,
                                                    bool isWing)
{
    double kPut = AQLMath::log( z.strikePut / x.F );
    double kCall = AQLMath::log( z.strikeCall / x.F );
    double volPut = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( kPut, method, target, VariableLogStrike, x, y, isWing );
    double volCall = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( kCall, method, target, VariableLogStrike, x, y, isWing );
    double premPut = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( z.strikePut, volPut, -1, x );
    double premCall = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( z.strikeCall, volCall, 1, x );

    return premPut + premCall;
};

Strangle AQLMathFXVolatilitySurfaceGenerate::GetHighStrangle(  double atmVol,
                                                          double BF,
                                                          const FXOptionData& x )
{
    Strangle z;

    z.vol = atmVol + BF;
    z.strikePut = exp( AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta( -0.25, z.vol, -1, x ) ) * x.F;

    double minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vol, x );
    if( 0.25 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vol, 1, x ) && isErrorMessage == true )
    {
		AQLString msg("max high strangle delta is under 0.25");
		    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    if( 0.25 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vol, 1, x ) && isErrorMessage == false )
    {
        z.strikeCall = exp( minLogStrike ) * x.F;
    }
    else
    {      
        z.strikeCall = exp( AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta( 0.25, z.vol, 1, x ) ) * x.F;
    }

    z.prem = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( z.strikePut, z.vol, -1, x ) + 
              AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( z.strikeCall, z.vol, 1, x ); 

    return z;
};

Strangle AQLMathFXVolatilitySurfaceGenerate::GetLowStrangle(  double atmVol,
                                                         double BF,
                                                         const FXOptionData& x )
{
    Strangle z;

    z.vol = atmVol + BF;
    z.strikePut = exp( FindLogStrikeFromDelta( -0.1, z.vol, -1, x ) ) * x.F;

    double minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( z.vol, x );
    if( 0.1 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vol, 1, x ) && isErrorMessage == true )
    {
		AQLString msg("max low strangle delta is under 0.1");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    else if( 0.1 > AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, z.vol, 1, x ) && isErrorMessage == false )
    {
        z.strikeCall = exp( minLogStrike ) * x.F;;
    }
    else
    {      
        z.strikeCall = exp( AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta( 0.1, z.vol, 1, x ) ) * x.F;
    }

    z.prem = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( z.strikePut, z.vol, -1, x ) + 
             AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( z.strikeCall, z.vol, 1, x ); 

    return z;
};

class FindStrangleVolFunc : public AQLFunctionVector
{
public:

    /// constructor
    FindStrangleVolFunc( const FXOptionData& x_,
                         const SmileParam& y_,
                         const Strangle& lowSt_,
                         const Strangle& highSt_,
                         const InterpolationMethod& method_,
                         const InterpolationTarget& target_,
                         bool isWing_,
                         double wf_ )
            : x(x_), y(y_), lowSt(lowSt_), highSt(highSt_), method(method_), target( target_ ),isWing(isWing_), wf(wf_)
			{
				low.resize(2);
				high.resize(2);
                double multi = 3.;
				/*low[0] = AQLMath::max( y.lowBF - 0.02, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				low[1] = AQLMath::max( y.highBF - 0.02, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				high[0] = y.lowBF + 0.07;
				high[1] = y.highBF + 0.03;*/
                if( y.lowBF>=0.0 && y.highBF<=0.0 && y.lowBF>-y.highBF )
                {
                    low[0] = AQLMath::max( 0., AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    low[1] = AQLMath::max( 0., AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    high[0] = y.lowBF * multi;
				    high[1] = - y.highBF * multi;
                }
                else if( y.lowBF>=0.0 && y.highBF<=0.0 && y.lowBF<-y.highBF )
                {
                    low[0] = AQLMath::max( - y.lowBF * multi, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    low[1] = AQLMath::max( y.highBF * multi, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    high[0] = 0.;
				    high[1] = 0.;
                }
                else if( y.lowBF<=0.0 && y.highBF>=0.0 && -y.lowBF>y.highBF )
                {
                    low[0] = AQLMath::max( y.lowBF * multi, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    low[1] = AQLMath::max( - y.highBF * multi, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    high[0] = 0.;
				    high[1] = 0.;
                }
                else if( y.lowBF<=0.0 && y.highBF>=0.0 && -y.lowBF<y.highBF )
                {
                    low[0] = AQLMath::max( 0., AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    low[1] = AQLMath::max( 0., AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    high[0] = - y.lowBF * multi;
				    high[1] = y.highBF * multi;
                }
                else if( y.lowBF>0.0 && y.highBF>0.0 )
                {
                    low[0] = AQLMath::max( 0., AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    low[1] = AQLMath::max( 0., AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    high[0] = y.lowBF * multi;
				    high[1] = y.highBF * multi;
                }
                else if( y.lowBF<0.0 && y.highBF<0.0 )
                {
                    low[0] = AQLMath::max( y.lowBF * multi, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    low[1] = AQLMath::max( y.highBF * multi, AQLMath::max( -y.atmVol-0.5*y.highRR, -y.atmVol+0.5*y.highRR ) );
				    high[0] = 0.;
				    high[1] = 0.;
                }
			};
    virtual unsigned long lengthOfArgumentVector( ) { return 2;}
    virtual unsigned long lengthOfFunctionVector( ) { return 2;}
    virtual unsigned long maximumNumberOfIterations() { return 150;}

    virtual void operator()(std::vector<double> & f, const std::vector<double> & z)
    {
        f.resize( 2 );
        SmileParam yOpt = y;
        yOpt.lowBF = z[0];
        yOpt.highBF = z[1];
        AQLString msg;
        SmileData w = AQLMathFXVolatilitySurfaceGenerate::BuildSmile( yOpt, x, isWing, wf, msg );
        f[0] = AQLMathFXVolatilitySurfaceGenerate::GetModelPrem( x, w, lowSt, method, target, isWing ) / lowSt.prem - 1.0;
        f[1] = AQLMathFXVolatilitySurfaceGenerate::GetModelPrem( x, w, highSt, method, target, isWing ) / highSt.prem - 1.0;
    };

    virtual bool constraintsAreViolated(const std::vector <double > &z)
    {
        if( z[0]<low[0] || z[0]>high[0] || z[1]<low[1] || z[1]>high[1] || 
            z[0] - z[1] > y.lowBF - y.highBF + 0.05 || z[1] - z[0] > y.highBF - y.lowBF + 0.05 ) return true;
        if( ( z[0] < 0.0 && z[1] > 0.0 ) || ( z[0] > 0.0 && z[1] < 0.0 ) ) return true;
        if( AQLMath::abs(z[0])<EPS_Vol1 || AQLMath::abs(z[1])<EPS_Vol1 ) return true;
        
        if( isWing )
        {
            SmileParam y_ = y;
            y_.lowBF = z[0];
            y_.highBF = z[1];
            return AQLMathFXVolatilitySurfaceGenerate::SmileDataCheck_Bool( y_, x, wf ); 
        }
        else
        {
            return false;
        }
    };
private:
    FXOptionData x;
    SmileParam y;
    Strangle lowSt;
    Strangle highSt;
    InterpolationMethod method;
    InterpolationTarget target;
    bool isWing;
    double wf;
	DoubleArray low;
	DoubleArray high;
};

SmileData AQLMathFXVolatilitySurfaceGenerate::FindStrangleVol( const FXOptionData& x,
                                                          const SmileParam& y,
                                                          const InterpolationMethod& method,
                                                          const InterpolationTarget& target,
                                                          bool isWing,
                                                          double wf)
{
    Strangle lowSt = AQLMathFXVolatilitySurfaceGenerate::GetLowStrangle( y.atmVol, y.lowBF, x );
    Strangle highSt = AQLMathFXVolatilitySurfaceGenerate::GetHighStrangle( y.atmVol, y.highBF, x );

    DoubleArray z(2);
    z[0] = y.lowBF;
    z[1] = y.highBF;
    if( y.lowBF>0.0 && y.highBF<0.0 && y.lowBF>-y.highBF )
    {
        z[1] = EPS_Vol1;
    }
    else if( y.lowBF>0.0 && y.highBF<0.0 && y.lowBF<-y.highBF )
    {
        z[0] = -EPS_Vol1;
    }
    else if( y.lowBF<0.0 && y.highBF>0.0 && -y.lowBF>y.highBF )
    {
        z[1] = -EPS_Vol1;
    }
    else if( y.lowBF<0.0 && y.highBF>0.0 && -y.lowBF<y.highBF )
    {
        z[0] = EPS_Vol1;
    }

    if( AQLMath::abs(y.lowBF)<=EPS_Vol1 && AQLMath::abs(y.highBF)<=EPS_Vol1 )
    {
        z[0] = EPS_Vol1;
        z[1] = EPS_Vol1;
    }
    else if( AQLMath::abs(y.lowBF)<=EPS_Vol1 )
    {
        z[0] = AQLMath::sign(EPS_Vol1, y.highBF);
    }
    else if( AQLMath::abs(y.highBF)<=EPS_Vol1 )
    {
        z[1] = AQLMath::sign(EPS_Vol1, y.lowBF);
    }

    FindStrangleVolFunc suv_func( x, y, lowSt, highSt, method, target, isWing, wf );
    NL2SOL solver( suv_func );

    solver.tryToSolve( z );
    SmileParam yOpt = y;
    yOpt.lowBF = z[0];
    yOpt.highBF = z[1];
    AQLString msg;
    return AQLMathFXVolatilitySurfaceGenerate::BuildSmile( yOpt, x, isWing, wf, msg );
};

double AQLMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( double point,
                                                               double termPoint,
                                                               const InterpolationMethod& method,
                                                               const InterpolationTarget& target,
                                                               const InterpolationVariable& variable,
                                                               const ATMInterpolationMethod& atmMethod,
                                                               const vector<FXOptionData >& x,
                                                               vector<SmileData >& y,
                                                               bool isWing )
{
    //check atmpoint
	unsigned int atmpoint = 0;
	if ( isWing ) atmpoint = 3;
	else atmpoint = 2;

    size_t i;
    size_t n = x.size();

    //error check
    if( termPoint<0. )
    {
        AQLString msg("Term Point is negative!");
        throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
    }
    if( y.size() != n )
    {
        AQLString msg("data sizes are not equal !");
        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    else if( atmMethod == DailyWeighted || atmMethod == SquareDailyWeighted )
    {
        for( i=0;i<n;i++ )
        {
            if( x[i].Days == NULL )
            {
                AQLString msg("No days maturity data!");
                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
            }
        }
        for( i=0;i<n-1;i++ )
        {
            if( x[i].Days > x[i+1].Days )
            {
                AQLString msg(" the order of Days is not correct!");
                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
            }
        }
    }
    else if( atmMethod == TermWeighted || atmMethod == TermNoWeighted )
    {
        for( i=0;i<n;i++ )
        {
            if( x[i].T == NULL )
            {
                AQLString msg("No days maturity data!");
                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
            }
        }
        for( i=0;i<n-1;i++ )
        {
            if( x[i].T > x[i+1].T )
            {
                AQLString msg("bad the order of FXOptionParams!");
                throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
            }
        }
     
    }

    for( i=0;i<n;i++ ){ SmileDataCheck( y[i], isWing );  }

    size_t index;
    switch ( atmMethod )
        {
        case TermWeighted:
            for( index=0;index<n;index++ )
            {
                if( AQLMath::abs(x[index].T-termPoint)<EPS_Vol ) 
                {
                    return AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, 
                        x[index], y[index], isWing );
                }
                if(x[index].T>termPoint || index==n ) break;
            }
            break;
        case DailyWeighted:
            for( index=0;index<n;index++ )
            {
                if( AQLMath::abs(x[index].Days-termPoint)<EPS_Vol ) 
                {
                    return AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, 
                        x[index], y[index], isWing );
                }
                if(x[index].Days>termPoint) break;
            }
            break;
        case SquareDailyWeighted:
            for( index=0;index<n;index++ )
            {
                if( AQLMath::abs(x[index].Days-termPoint)<EPS_Vol ) 
                {
                    return AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, 
                        x[index], y[index], isWing );
                }
                if(x[index].Days>termPoint) break;
            }
            break;               
        case TermNoWeighted:
            for( index=0;index<n;index++ )
            {
                if( AQLMath::abs(x[index].T-termPoint)<EPS_Vol ) 
                {
                    return AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, 
                        x[index], y[index], isWing );
                }
                if(x[index].T>termPoint || index==n ) break;
            }
            break;
		default:
            AQLString msg("ATMInterpolationMethod is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
        }

    vector<double > weightedVar(n, 0.), mat_Term(n, 0.), mat_Day(n, 0.), smileSpreadData( n, 0.0 );
    double atmVol,smileSpread;  
    switch ( atmMethod )
        {
        case TermWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Term[i] = x[i].T;
                weightedVar[i] = AQLMath::pow( y[i].vols[atmpoint], 2 ) * mat_Term[i];
            }

            if( index==0 )
            {
                atmVol = y[0].vols[atmpoint];
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[0], 
                    y[0], isWing ) - y[0].vols[atmpoint];                
            }
            else if( index==n )
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false ) / termPoint );
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[n-1], 
                    y[n-1], isWing ) - y[n-1].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false ) / termPoint );
                smileSpreadData[index-1] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index-1], y[index-1], isWing ) - y[index-1].vols[atmpoint];
                smileSpreadData[index] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index], y[index], isWing ) - y[index].vols[atmpoint];
                smileSpread = AQLMathInterpolationUtilities::linear( mat_Term, smileSpreadData, termPoint, false );
            }
            break;
        case DailyWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Day[i] = x[i].Days;
                weightedVar[i] = AQLMath::pow( y[i].vols[atmpoint], 2)  * mat_Day[i];
            }
            
            if( index==0 )
            {
                atmVol = y[0].vols[atmpoint];
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[0], 
                    y[0], isWing ) - y[0].vols[atmpoint];
            }
            else if( index==n )
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / termPoint );
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[n-1], 
                    y[n-1], isWing ) - y[n-1].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / termPoint );
                smileSpreadData[index-1] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index-1], y[index-1], isWing ) - y[index-1].vols[atmpoint];
                smileSpreadData[index] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index], y[index], isWing ) - y[index].vols[atmpoint];
                smileSpread = AQLMathInterpolationUtilities::linear( mat_Day, smileSpreadData, termPoint, false );
            }
            break;
        case SquareDailyWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Day[i] = x[i].Days;
                weightedVar[i] = AQLMath::pow( y[i].vols[atmpoint], 2 ) * AQLMath::sqrt( mat_Day[i] );
            }

            if( index==0 )
            {
                atmVol = y[0].vols[atmpoint];
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[0], 
                    y[0], isWing ) - y[0].vols[atmpoint];
            }
            else if( index==n )
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / AQLMath::sqrt( termPoint ) );
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[n-1], 
                    y[n-1], isWing ) - y[n-1].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / AQLMath::sqrt( termPoint ) );
                smileSpreadData[index-1] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index-1], y[index-1], isWing ) - y[index-1].vols[atmpoint];
                smileSpreadData[index] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index], y[index], isWing ) - y[index].vols[atmpoint];
                smileSpread = AQLMathInterpolationUtilities::linear( mat_Day, smileSpreadData, termPoint, false );
            }
            break;
		case TermNoWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Term[i] = x[i].T;
                weightedVar[i] = y[i].vols[atmpoint];
            }

            if( index==0 )
            {
                atmVol = y[0].vols[atmpoint];
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[0], 
                    y[0], isWing ) - y[0].vols[atmpoint];                
            }
            else if( index==n )
            {
                atmVol = AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false );
                smileSpread = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, variable, x[n-1], 
                    y[n-1], isWing ) - y[n-1].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false );
                smileSpreadData[index-1] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index-1], y[index-1], isWing ) - y[index-1].vols[atmpoint];
                smileSpreadData[index] = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( point, method, target, 
                    variable, x[index], y[index], isWing ) - y[index].vols[atmpoint];
                smileSpread = AQLMathInterpolationUtilities::linear( mat_Term, smileSpreadData, termPoint, false );
            }

            break;
		default:
            AQLString msg("ATMInterpolationMethod is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
        }
    return atmVol + smileSpread;
};

double AQLMathFXVolatilitySurfaceGenerate::GetATMVolatility( double termPoint,
                                                        const ATMInterpolationMethod& atmMethod,
                                                        const vector<FXOptionData >& x,
                                                        vector<SmileData >& y ) 
{
    size_t i;
    size_t n = x.size();
    if( n != y.size() || n == 0)
    {
        AQLString msg("Data sizes are not same!");
        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

	//check atmpoint
	unsigned int atmpoint = 0;
	if (7 == y[0].vols.size())
		atmpoint = 3;
	else if (5 == y[0].vols.size())
		atmpoint = 2;
	else if (3 == y[0].vols.size())
		atmpoint = 1;
	// vanna-volga
	else 
		throw AQLCoreInvalidData("Volatility Setting Error",__FILE__,__LINE__);

	DoubleArray weightedVar(n, 0.), mat_Term(n, 0.), mat_Day(n, 0.);
    double atmVol;  
    switch ( atmMethod )
        {
        case TermWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Term[i] = x[i].T;
                weightedVar[i] = AQLMath::pow( y[i].vols[atmpoint], 2 ) * mat_Term[i];
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].T )
            {
                atmVol = y[0].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false ) / termPoint );
            }
            break;
        case DailyWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Day[i] = x[i].Days;
                weightedVar[i] = AQLMath::pow( y[i].vols[atmpoint], 2)  * mat_Day[i];
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].Days )
            {
                atmVol = y[0].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / termPoint );
            }
            break;
        case SquareDailyWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Day[i] = x[i].Days;
                weightedVar[i] = AQLMath::pow( y[i].vols[atmpoint] , 2 ) * AQLMath::sqrt( mat_Day[i] );
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].Days )
            {
                atmVol = y[0].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / AQLMath::sqrt( termPoint ) );
            }
            break;          
        case TermNoWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Term[i] = x[i].T;
                weightedVar[i] = y[i].vols[atmpoint];
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].T )
            {
                atmVol = y[0].vols[atmpoint];
            }
            else
            {
                atmVol = AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false );
            }
            break;
		default:
            AQLString msg("ATMInterpolationMethod is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);		
        }
    return atmVol;
};

// vanna-volga
double AQLMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp( double termPoint,
                                                        const ATMInterpolationMethod& atmMethod,
														const std::vector<FXOptionData >& x,
														const std::vector<SmileData >& y,
														int volpos) 
{
    size_t i;
    size_t n = x.size();
    if( n != y.size() || n == 0)
    {
        AQLString msg("Data sizes are not same!");
        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

	if (volpos >= y[0].vols.size())
		throw AQLCoreInvalidData("vol position is not consistent",__FILE__,__LINE__);

	
	DoubleArray weightedVar(n, 0.), mat_Term(n, 0.), mat_Day(n, 0.);
    double retVol;  
    switch ( atmMethod )
        {
        case TermWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Term[i] = x[i].T;
                weightedVar[i] = AQLMath::pow( y[i].vols[volpos], 2 ) * mat_Term[i];
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].T )
            {
                retVol = y[0].vols[volpos];
            }
            else
            {
                retVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false ) / termPoint );
            }
            break;
        case DailyWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Day[i] = x[i].Days;
                weightedVar[i] = AQLMath::pow( y[i].vols[volpos], 2)  * mat_Day[i];
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].Days )
            {
                retVol = y[0].vols[volpos];
            }
            else
            {
                retVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / termPoint );
            }
            break;
        case SquareDailyWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Day[i] = x[i].Days;
                weightedVar[i] = AQLMath::pow( y[i].vols[volpos] , 2 ) * AQLMath::sqrt( mat_Day[i] );
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].Days )
            {
                retVol = y[0].vols[volpos];
            }
            else
            {
                retVol = AQLMath::sqrt( AQLMathInterpolationUtilities::linear( mat_Day, weightedVar, termPoint, false ) / AQLMath::sqrt( termPoint ) );
            }
            break;          
        case TermNoWeighted:
            for( i=0;i<n;i++ )
            {
                mat_Term[i] = x[i].T;
                weightedVar[i] = y[i].vols[volpos];
            }
            if( termPoint<0. )
            {
                AQLString msg("Term Point is negative!");
              	throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
            }
            else if( 0.<= termPoint &&  termPoint <= x[0].T )
            {
                retVol = y[0].vols[volpos];
            }
            else
            {
                retVol = AQLMathInterpolationUtilities::linear( mat_Term, weightedVar, termPoint, false );
            }
            break;
		default:
            AQLString msg("ATMInterpolationMethod is not supported");
			throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);		
        }
    return retVol;
};

class FindLowButterflyFunc : public AQLFunctionVector
{
public:
    FindLowButterflyFunc( const InterpolationMethod& method_, 
                          const InterpolationTarget& target_, 
		                  const FXOptionData& x_ ,
                          SmileData& y_ ,
                          bool isWing_ )
        : method(method_), target(target_), x(x_), y(y_), isWing(isWing_) {};
    virtual ~FindLowButterflyFunc(){};

	virtual unsigned long lengthOfArgumentVector( ) { return 1; }
    virtual unsigned long lengthOfFunctionVector( ) { return 1; }
    virtual unsigned long maximumNumberOfIterations() { return 150; }

    virtual void operator()(std::vector<double> & f, const std::vector<double> & z)
    {
        f.resize( 1 );
        
        Strangle strangle;
        if( isWing ) 
        {
            strangle = AQLMathFXVolatilitySurfaceGenerate::GetLowStrangle(y.vols[3], z[0], x);
        }
        else
        {
            strangle = AQLMathFXVolatilitySurfaceGenerate::GetLowStrangle(y.vols[2], z[0], x);
        }
        double putVol,callVol;
		putVol = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( AQLMath::log(strangle.strikePut/x.F), 
								method, target, VariableLogStrike, x, y, isWing );
		callVol = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( AQLMath::log(strangle.strikeCall/x.F), 
								method, target, VariableLogStrike, x, y, isWing );
       
		double modelPrem = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( strangle.strikePut, putVol, -1, x ) + 
						   AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( strangle.strikeCall, callVol, 1, x ); 
        f[0] = strangle.prem / modelPrem - 1.0; 
    };

    virtual bool constraintsAreViolated(const std::vector <double > &z)
    {
        if( isWing ) 
        {
		    Strangle strangle = AQLMathFXVolatilitySurfaceGenerate::GetLowStrangle( y.vols[3], z[0], x );
		    double minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( strangle.vol, x );
            if( z[0] <= (y.vols[1]+y.vols[5])/2-y.vols[3]-0.1 || z[0] >= (y.vols[1]+y.vols[5])/2-y.vols[3]+0.1  ) return true;
		    if( AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, strangle.vol, 1, x ) < 0.1 ) return true;
        }
        else
        {
            Strangle strangle = AQLMathFXVolatilitySurfaceGenerate::GetLowStrangle( y.vols[2], z[0], x );
		    double minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( strangle.vol, x );
            if( z[0] <= (y.vols[0]+y.vols[4])/2-y.vols[2]-0.1 || z[0] >= (y.vols[0]+y.vols[4])/2-y.vols[2]+0.1  ) return true;
		    if( AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, strangle.vol, 1, x ) < 0.1 ) return true;
        }

        return false;       
    };

private:
	InterpolationMethod method;
	InterpolationTarget target;
    FXOptionData x;
    SmileData y;
    bool isWing;
};

class FindHighButterflyFunc : public AQLFunctionVector
{
public:
    FindHighButterflyFunc( const InterpolationMethod& method_, 
                           const InterpolationTarget& target_, 
		                   const FXOptionData& x_ , 
                           SmileData& y_,
                           bool isWing_ )
        : method(method_), target(target_), x(x_), y(y_), isWing(isWing_) {};
    virtual ~FindHighButterflyFunc(){};

	virtual unsigned long lengthOfArgumentVector( ) { return 1; }
    virtual unsigned long lengthOfFunctionVector( ) { return 1; }
    virtual unsigned long maximumNumberOfIterations() { return 150; }

    virtual void operator()(std::vector<double> & f, const std::vector<double> & z)
    {
        f.resize( 1 );
        Strangle strangle;
        if( isWing )
        {
            strangle = AQLMathFXVolatilitySurfaceGenerate::GetHighStrangle(y.vols[3], z[0], x);
        }
        else
        {
            strangle = AQLMathFXVolatilitySurfaceGenerate::GetHighStrangle(y.vols[2], z[0], x);
        }
        double callVol,putVol;
		putVol = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( AQLMath::log(strangle.strikePut/x.F), 
								method, target, VariableLogStrike, x, y, isWing );
		callVol = AQLMathFXVolatilitySurfaceGenerate::GetSmileInterpolation( AQLMath::log(strangle.strikeCall/x.F), 
								method, target, VariableLogStrike, x, y, isWing );
  
		double modelPrem = AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( strangle.strikePut, putVol, -1, x ) + 
						   AQLMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( strangle.strikeCall, callVol, 1, x ); 
        f[0] = strangle.prem / modelPrem - 1.0; 
    };

    virtual bool constraintsAreViolated(const std::vector <double > &z)
    {
        if( isWing )
        {
		    Strangle strangle = AQLMathFXVolatilitySurfaceGenerate::GetHighStrangle( y.vols[3], z[0], x );
		    double minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( strangle.vol, x );
            if( z[0] <= (y.vols[2]+y.vols[4])/2-y.vols[3]-0.1 || z[0] >= (y.vols[2]+y.vols[4])/2-y.vols[3]+0.1 ) return true;
		    if( AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, strangle.vol, 1, x ) < 0.25 ) return true;
        }
        else
        {
            Strangle strangle = AQLMathFXVolatilitySurfaceGenerate::GetHighStrangle( y.vols[2], z[0], x );
		    double minLogStrike = AQLMathFXVolatilitySurfaceGenerate::GetMinimumCallDeltaLogStrike( strangle.vol, x );
            if( z[0] <= (y.vols[1]+y.vols[3])/2-y.vols[2]-0.1 || z[0] >= (y.vols[1]+y.vols[3])/2-y.vols[2]+0.1 ) return true;
		    if( AQLMathFXVolatilitySurfaceGenerate::GetFXOptionDelta( minLogStrike, strangle.vol, 1, x ) < 0.25 ) return true;
        }

        return false;       
    };

private:
	InterpolationMethod method;
	InterpolationTarget target;
    FXOptionData x;
    SmileData y;
    bool isWing;
};

double AQLMathFXVolatilitySurfaceGenerate::FindLowButterfly( const InterpolationMethod& method,
														const InterpolationTarget& target, 
														const FXOptionData& x,
											            SmileData& y,
                                                        bool isWing )
{   
	FindLowButterflyFunc sub_func( method, target, x, y, isWing );
	DoubleArray z(1);
    if( isWing) z[0] = (y.vols[1] + y.vols[5]) / 2 - y.vols[3];
    else z[0] = (y.vols[0] + y.vols[4]) / 2 - y.vols[2];

    NL2SOL solver( sub_func );
	solver.tryToSolve( z );
	return z[0];
};

double AQLMathFXVolatilitySurfaceGenerate::FindHighButterfly( const InterpolationMethod& method,
											         	 const InterpolationTarget& target, 
											 			 const FXOptionData& x,
											             SmileData& y,
                                                         bool isWing )
{   
	FindHighButterflyFunc sub_func( method, target, x, y, isWing );
	DoubleArray z(1);
	if( isWing ) z[0] = (y.vols[2] + y.vols[4]) / 2 - y.vols[3];
    else z[0] = (y.vols[1] + y.vols[3]) / 2 - y.vols[2];

    NL2SOL solver( sub_func );
	solver.tryToSolve( z );
	return z[0];
};
