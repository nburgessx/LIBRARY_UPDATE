#ifndef LAMathAnalyticalBlack2_h
#define LAMathAnalyticalBlack2_h

#include <cmath>
#include "AQLFunction.h"
#include "LAAnalyticFormula.h"
#include "AQLDist.h"

namespace local
{
//
//--------------------------------------------------------------------------------
// BlackFormula

//
//-----
//

//
/*!
	@brief BlackFormula

	@param[in] mean_LN    Mean of Lognoraml distribution
	@param[in] stdDev_LN  Standard deviation of corresponding normal distribution  
	@param[in] margin     Margin
	@param[in] lower      Indicator condition / lower limit for integration
    @param[in] sgn        {-1,1}

    @return 
*/
template <typename T1, typename T2, typename T3, typename T4>
double BlackFormula( T1 mean_LN,   //[in] Mean of Lognoraml distribution 
                     T2 stdDev_LN, //[in] Standard deviation of corresponding normal distribution
                     T3 margin,    //[in] Margin
                     T4 lower,     //[in] Indicator condition / lower limit for integration
                     int sgn       //[in] {-1,1}
                    )
{
    //error check
    if ( sgn != -1 && sgn != 1 )
    {
        AQLString msg("Sign must be -1 or 1. (Last argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( mean_LN < 0 )
    {
        AQLString msg("Mean negative! (1st argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    
    if ( margin == lower && lower < 0 )
    {
        AQLString msg("Margin negative! (3rd argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( lower < 0 )
    {
        AQLString msg("Lower bound negative! (4th argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    //
    double tmp = 0 < lower ? log( static_cast<double>(mean_LN) / static_cast<double>(lower) ) : -10000000;

    double var_LN = stdDev_LN * stdDev_LN;
    double d1 = 0.000000001 < var_LN ? (tmp + 0.5 * var_LN) / stdDev_LN : 0 < tmp ? 10000000 : -10000000;
    double d2 = d1 - stdDev_LN;

    return sgn * ( mean_LN * AQLDist::normsdist( sgn * d1 ) - margin *AQLDist::normsdist( sgn * d2 ) );
};

//
//-----
//

//
/*!
	@brief BlackFormula

	@param[in] mean_LN    Mean of Lognoraml distribution
	@param[in] stdDev_LN  Standard deviation of corresponding normal distribution  
	@param[in] margin     Margin
    @param[in] sgn        {-1,1}

    @return 
*/

template <typename T1, typename T2, typename T3>
double BlackFormula( T1 mean_LN,   //[in] Mean of Lognoraml distribution
                     T2 stdDev_LN, //[in] Standard deviation of corresponding normal distribution
                     T3 margin,    //[in] Margin
                     int sgn       //[in] {-1,1}
                    )
{
//    return BlackFormula( mean_LN, var_LN, margin, margin, sgn );

    if ( sgn != -1 && sgn != 1 )
    {
        AQLString msg("Sign must be -1 or 1. (Last argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( stdDev_LN == 0. || mean_LN <= 0 || margin <= 0 ) return max( sgn * (mean_LN - margin), 0.0 );

    double tmp = log( static_cast<double>(mean_LN) / static_cast<double>(margin) );

    double var_LN = stdDev_LN * stdDev_LN;
    double d1 = (tmp + 0.5 * var_LN) / stdDev_LN;
    double d2 = d1 - stdDev_LN;

    return sgn * ( mean_LN * AQLDist::normsdist( sgn * d1 ) - margin * AQLDist::normsdist( sgn * d2 ) );
};


//
//-----
// BlackFormula in the Displaced Diffusion model

/*!
	@brief BlackFormula in the Displaced Diffusion model

	@param[in] mean_LN    Mean of Lognoraml distribution
	@param[in] stdDev_LN  Standard deviation of corresponding normal distribution  
	@param[in] margin     Margin
    @param[in] sgn        {-1,1}
    @param[in] beta       Skew parameter

    @return 
*/
template <typename T1, typename T2, typename T3, typename T4>
double BlackFormulaDD( T1 mean_LN,   //[in] Mean of Lognoraml distribution
                       T2 stdDev_LN, //[in] Standard deviation of corresponding normal distribution
                       T3 margin,    //[in] Margin
                       int sgn,      //[in] {-1,1}
                       T4 beta       //[in] Beta for Displaced Diffusion Method
                      )
{
    if ( beta == 0 )
    {
        double s = fabs(stdDev_LN);
        double tmp = (mean_LN - margin) / s;
        return s * (AnalyticFormulae::diffNormdist(tmp) + sgn * tmp * AQLDist::normsdist(sgn * tmp));
    }

    double s = mean_LN * (1. - beta) / static_cast<double>(beta);
    double mean_LN_beta = mean_LN + s;
    double margin_beta = margin + s;

    return BlackFormula( mean_LN_beta, beta * stdDev_LN, margin_beta, sgn );
};

//
//-----
//  Black Implied Vol
template <typename T1, typename T2, typename T3>
class Sub_func_BlackImplVol : public AQLFunction
{
public:
    Sub_func_BlackImplVol( T1 prem_, T2 mean_LN_, T3 margin_, int sgn_ )
        : prem(prem_), mean_LN(mean_LN_), margin(margin_), sgn(sgn_) {}
    double operator()(double x) const { return BlackFormula( mean_LN, x, margin, sgn) / prem - 1.0; }
private:
    T1 prem;
    T2 mean_LN;
    T3 margin;
    int sgn;
};
/*!
	@brief Black Implied Vol

	@param[in] prem     premium
	@param[in] mean_LN  Mean of Lognoraml distribution  
	@param[in] margin   Margin
    @param[in] sgn      {-1,1}

    @return Implied Vol
*/
template <typename T1, typename T2, typename T3>
double BlackImplVol( T1 prem,
                     T2 mean_LN,
                     T3 margin,
                     int sgn
                   )
{
    Sub_func_BlackImplVol<T1, T2, T3> sub_func(prem, mean_LN, margin, sgn);

    return sub_func.SolveBR( 0.000001, 10.0, 100000, 0.000000000000001 );
};

//
//-----
// Black Implied Vol in the Displaced Diffusion model
template <typename T1, typename T2, typename T3, typename T4>
class Sub_func_BlackImplVolDD : public AQLFunction
{
public:
    Sub_func_BlackImplVolDD( T1 prem_, T2 mean_LN_, T3 margin_, int sgn_, T4 beta_ )
        : prem(prem_), mean_LN(mean_LN_), margin(margin_), sgn(sgn_), beta(beta_) {}
    double operator()(double x) const { return BlackFormulaDD( mean_LN, x, margin, sgn, beta) / prem - 1.0; }
private:
    T1 prem;
    T2 mean_LN;
    T3 margin;
    int sgn;
    T4 beta;
};

/*!
	@brief Black Implied Vol in the Displaced Diffusion model

	@param[in] prem     premium
	@param[in] mean_LN  Mean of Lognoraml distribution  
	@param[in] margin   Margin
    @param[in] sgn      {-1,1}
    @param[in] beta     Skew parameter

    @return Implied Vol
*/
template <typename T1, typename T2, typename T3, typename T4>
double BlackImplVolDD( T1 prem,
                       T2 mean_LN,
                       T3 margin,
                       int sgn,
                       T4 beta
                      )
{
    Sub_func_BlackImplVolDD<T1, T2, T3, T4> sub_func(prem, mean_LN, margin, sgn, beta);

    return sub_func.SolveBR( 0.00000001, 10.0, 100000, 0.000000000000001 );
};

//
//--------------------------------------------------------------------------------
// vega

//
/*!
	@brief BlackFormula

	@param[in] mean_LN    Mean of Lognoraml distribution
	@param[in] stdDev_LN  Standard deviation of corresponding normal distribution  
	@param[in] margin     Margin
	@param[in] lower      Indicator condition / lower limit for integration
    @param[in] sgn        {-1,1}

    @return 
*/
template <typename T1, typename T2, typename T3, typename T4>
double FW_Vega( T1 mean_LN,   //[in] Initial value
                T2 sigma,     //[in] Standard deviation
                T3 T,         //[in] Maturity
                T4 margin     //[in] Margin
              )
{
    //error check
    if ( mean_LN < 0 )
    {
        AQLString msg("mean_LN negative! (1st argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( sigma < 0 )
    {
        AQLString msg("sigma! (2nd argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( T < 0 )
    {
        AQLString msg("T negative! (3rd argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    if ( margin < 0 )
    {
        AQLString msg("margin negative! (4th argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    double tmp = log( static_cast<double>(mean_LN) / static_cast<double>(margin) );
    
    double stdDev_LN = sigma * sqrt(T);
    double var_LN = stdDev_LN * stdDev_LN;
    double d1 = 0.000000001 < var_LN ? (tmp + 0.5 * var_LN) / stdDev_LN : 0 < tmp ? 10000000 : -10000000;

    return mean_LN * sqrt(T) * AnalyticFormulae::diffNormdist( d1 );
};

};

#endif
