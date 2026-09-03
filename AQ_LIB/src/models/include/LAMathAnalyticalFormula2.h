#ifndef LAMathAnalyticalFormula2_h
#define LAMathAnalyticalFormula2_h

#include <cmath>
#include <vector>
#include "AQLFunction.h"
#include "AQLDist.h"
#include <LAMathAnalyticalBlack2.h>

namespace local
{
//
/*!
	@brief Cap/Floor PV

	@param[in] P0      Initial Discount Bond stream
	@param[in] L0      Initial LIBOR stream   
	@param[in] tau     Year flaction of accrual period
    @param[in] stdDev  Standard deviation of caplets
    @param[in] margin  Margin
    @param[in] sgn     {-1,1}

    @return Cap/Floor PV
*/
template <typename T1, typename T2, typename T3, typename T4, typename T5>
double BlackFormulaCapFloor( const vector<T1>& P0,       //[in] Initial Discount Bond stream
                             const vector<T2>& L0,       //[in] Initial LIBOR stream 
                             const vector<T3>& tau,      //[in] Year flaction of accrual period
                             const vector<T4>& stdDev,   //[in] Standard deviation of caplet
                             const vector<T5>& margin,   //[in] Margin
                             int               sgn       //[in] {-1,1}
                           )
{
    if ( sgn != -1 && sgn != 1 ) 
    {   
        AQLString msg("Sign must be -1 or 1. (Last argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    
    size_t n = P0.size();
    if( n != L0.size() )
    {
        AQLString msg("Error : P0.size() != L0.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }    
    if( n != stdDev.size() )
    {
        AQLString msg("Error : P0.size() != stdDev_LN.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if( n != margin.size() )
    {
        AQLString msg("Error : P0.size() != margin.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    for(size_t i = 0; i < n; ++i)
    {
        if( L0[i] < 0 )
        {
            AQLString msg("Error : L0[i] < 0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        
        if( P0[i] < 0 )
        {
            AQLString msg("Error : L0[i] < 0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        if( stdDev[i] < 0 )
        {
            AQLString msg("Error : stdDev[i] < 0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }

    double tmp = 0.0;
    for(size_t i = 0; i < n; ++i)
    {
        tmp += P0[i] * tau[i] * BlackFormula(L0[i], stdDev[i], margin[i], sgn);
    }
    
    return tmp;
};

/*!
	@brief Cap/Floor PV in the Displaced Diffusion model

	@param[in] P0      Initial Discount Bond stream
	@param[in] L0      Initial LIBOR stream   
	@param[in] tau     Year flaction of accrual period
    @param[in] stdDev  Standard deviation of caplets
    @param[in] margin  Margin
    @param[in] sgn     {-1,1}
    @param[in] beta     Skew parameter

    @return Cap/Floor PV
*/
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
double BlackFormulaCapFloorDD( const vector<T1>& P0,       //[in] Initial Discount Bond stream
                               const vector<T2>& L0,       //[in] Initial LIBOR stream 
                               const vector<T3>& tau,      //[in] Year flaction of accrual period
                               const vector<T4>& stdDev,   //[in] Standard deviation of caplet
                               const vector<T5>& margin,   //[in] Margin
                               int               sgn,      //[in] {-1,1}
                               T6                beta      //[in] Beta for Displaced Diffusion Method
                             )
{
    if ( sgn != -1 && sgn != 1 )
    {   
        AQLString msg("Sign must be -1 or 1. (Last argument)");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    
    size_t n = P0.size();
    if( n != L0.size() )
    {
        AQLString msg("Error : P0.size() != L0.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }    
    if( n != stdDev.size() )
    {
        AQLString msg("Error : P0.size() != stdDev_LN.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }
    if( n != margin.size() )
    {
        AQLString msg("Error : P0.size() != margin.size()");
	    throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
    }

    for(size_t i = 0; i < n; ++i)
    {
        if( L0[i] < 0 )
        {
            AQLString msg("Error : L0[i] < 0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        
        if( P0[i] < 0 )
        {
            AQLString msg("Error : L0[i] < 0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }

        if( stdDev[i] < 0 )
        {
            AQLString msg("Error : stdDev[i] < 0");
	        throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
    }

    double tmp = 0.0;
    for(size_t i = 0; i < n; ++i)
    {
        tmp += P0[i] * tau[i] * BlackFormulaDD(L0[i], stdDev[i], margin[i], sgn, beta);
    }
    
    return tmp;
};

//
//--------------------------------------------------
// Cap/Floor Implied Vol

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class Sub_func_CapFloorImplVol : public AQLFunction
{
public:
    Sub_func_CapFloorImplVol( T1 prem_,
                              const vector<T2>& P0_,
                              const vector<T3>& L0_,
                              const vector<T4>& tau_,
                              const vector<T5>& T_fix_,
                              const vector<T6>& margin_,
                              int sgn_
                            )
        //: prem(prem_), P0(P0_), L0(L0_), tau(tau_), T_Pay(T_Pay_), margin(margin_), sgn(sgn_) {}
    : prem(prem_), P0(P0_), L0(L0_), tau(tau_), T_fix(T_fix_), margin(margin_), sgn(sgn_) {}
    double operator()(double x) const
    {
        size_t n = P0.size();
        vector<T4> stdDev(n, 0.0);
        for( size_t i = 0; i < n; ++i ) stdDev[i] = x * sqrt(T_fix[i]); 
        return BlackFormulaCapFloor( P0, L0, tau, stdDev, margin, sgn) / prem - 1.0; 
    }
private:
    T1 prem;
    vector<T2> P0;
    vector<T3> L0;
    vector<T4> tau;
    vector<T5> T_fix;
    vector<T6> margin;
    int sgn;
};

/*!
	@brief Cap/Floor Implied Vol

	@param[in] P0      Initial Discount Bond stream
	@param[in] L0      Initial LIBOR stream   
	@param[in] tau     Year flaction of accrual period
    @param[in] stdDev  Standard deviation of caplets
    @param[in] margin  Margin
    @param[in] sgn     {-1,1}

    @return Implied Vol
*/
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
double CapFloorImplVol( T1 prem,
                        const vector<T2>& P0,
                        const vector<T3>& L0,
                        const vector<T4>& tau,
                        const vector<T5>& T_fix,
                        const vector<T6>& margin,
                        int               sgn
                      )
{
    Sub_func_CapFloorImplVol<T1, T2, T3, T4, T5, T6> sub_func(prem, P0, L0, tau, T_fix, margin, sgn);

    return sub_func.SolveBR( 0.000001, 10.0, 100000, 0.000000000000001 );
};

//
//--------------------------------------------------
// Cap/Floor Implied Vol in the Displaced Diffusion model
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class Sub_func_CapFloorImplVolDD : public AQLFunction
{
public:
    Sub_func_CapFloorImplVolDD( T1 prem_,
                                const vector<T2>& P0_,
                                const vector<T3>& L0_,
                                const vector<T4>& tau_,
                                const vector<T5>& T_fix_,
                                const vector<T6>& margin_,
                                int               sgn_,
                                T7                beta_
                                )
        : prem(prem_), P0(P0_), L0(L0_), tau(tau_), T_fix(T_fix_), margin(margin_), sgn(sgn_), beta(beta_) {}
    
    double operator()(double x) const
    {
        size_t n = P0.size();
        vector<T4> stdDev(n, 0.0);
        for( size_t i = 0; i < n; ++i ) stdDev[i] = x * sqrt(T_fix[i]); 
        return BlackFormulaCapFloorDD( P0, L0, tau, stdDev, margin, sgn, beta) / prem - 1.0; 
    }
private:
    T1 prem;
    vector<T2> P0;
    vector<T3> L0;
    vector<T4> tau;
    vector<T5> T_fix;
    vector<T6> margin;
    T7 beta;
    int sgn;
};

/*!
	@brief Cap/Floor Implied Vol in the Displaced Diffusion model

	@param[in] P0      Initial Discount Bond stream
	@param[in] L0      Initial LIBOR stream   
	@param[in] tau     Year flaction of accrual period
    @param[in] stdDev  Standard deviation of caplets
    @param[in] margin  Margin
    @param[in] sgn     {-1,1}

    @return Implied Vol
*/
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
double CapFloorImplVolDD( T1 prem,
                          const vector<T2>& P0,
                          const vector<T3>& L0,
                          const vector<T4>& tau,
                          const vector<T5>& T_fix,
                          const vector<T6>& margin,
                          int               sgn,
                          T7                beta
                        )
{
    Sub_func_CapFloorImplVolDD<T1, T2, T3, T4, T5, T6, T7> sub_func(prem, P0, L0, tau, T_fix, margin, sgn, beta);

    return sub_func.SolveBR( 0.000001, 10.0, 100000, 0.000000000000001 );
};

};

#endif
