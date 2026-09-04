/*!
    \file AQLMathAnalyticalFormula.h
    \brief Black formula.
    \author
    \date 2008/25
    \version 1.0

     2008, AlgoQuantHub. All rights reserved.
*/

#ifndef __AQLMATHANALYTICALFORMULA_H__
#define __AQLMATHANALYTICALFORMULA_H__

#include <cmath>
#include <vector>
#include "AQLFunction.h"
#include "AQLDist.h"
#include "AQLBasic.h"
#include "AQLAnalyticFormula.h"

class AQLMathAnalyticalFormula
{
public:
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
	static double BlackFormula( T1 mean_LN,   //[in] Mean of Lognoraml distribution 
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

		return sgn * ( mean_LN * AQLDist::normsdist( sgn * d1 ) - margin * AQLDist::normsdist( sgn * d2 ) );
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
	static double BlackFormula( T1 mean_LN,   //[in] Mean of Lognoraml distribution
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

		//if ( stdDev_LN == 0. || mean_LN <= 0 || margin <= 0 )
		if ( stdDev_LN == 0.0 || (mean_LN <= 0.0 && margin >= 0.0) || (mean_LN >= 0.0 && margin <= 0.0))
		{
			return AQLMath::max( sgn * (mean_LN - margin), 0.0 );
		}

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
	static double BlackFormulaDD( T1 mean_LN,   //[in] Mean of Lognoraml distribution
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
		double operator()(double x) { return BlackFormula( mean_LN, x, margin, sgn) / prem - 1.0; }
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
	static double BlackImplVol( T1 prem,
						T2 mean_LN,
						T3 margin,
						int sgn,
						double low = 0.000001,
						double high= 10.
					)
	{
		Sub_func_BlackImplVol<T1, T2, T3> sub_func(prem, mean_LN, margin, sgn);

		return sub_func.SolveBR(low, high, 100000, 0.000000000000001 );
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
		double operator()(double x) const
		{ 
			return BlackFormulaDD( mean_LN, x, margin, sgn, beta) / prem - 1.0; 
		}

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
	static double BlackImplVolDD( T1 prem,
						T2 mean_LN,
						T3 margin,
						int sgn,
						T4 beta
						)
	{
		Sub_func_BlackImplVolDD<T1, T2, T3, T4>sub_func(prem, mean_LN, margin, sgn, beta);

		return sub_func.SolveBR( 0.000000000000001, 10.0, 100000, 0.000000000000001 );
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
	static double FW_Vega( T1 mean_LN,   //[in] Initial value
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
	static double BlackFormulaCapFloor( const vector<T1>& P0,       //[in] Initial Discount Bond stream
								const vector<T2>& L0,       //[in] Initial LIBOR stream 
								const vector<T3>& tau,      //[in] Year flaction of accrual period
								const vector<T4>& stdDev,   //[in] Standard deviation of caplet
								const vector<T5>& margin,   //[in] Margin
								int               sgn)       //[in] {-1,1 
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
			tmp += P0[i] * tau[i] * AQLMathAnalyticalFormula::BlackFormula(L0[i], stdDev[i], margin[i], sgn);
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
	static double BlackFormulaCapFloorDD( const vector<T1>& P0,       //[in] Initial Discount Bond stream
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
	static double CapFloorImplVol( T1 prem,
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
	static double CapFloorImplVolDD( T1 prem,
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