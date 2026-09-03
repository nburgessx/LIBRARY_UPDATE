#include "LAFunction.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>

#include "LAFunctionBase.h"
#include "LAGaussLegendre.h"
#include "LAFindRootBrent.h"
#include "LAOptimumBFGS.h"
#include "LABasic.h"
#include "LAGaussLaguerre.h"
#include "LAEulerIntegral.h"
#include "LACoreUtil.h"
#include "LAGaussLobattointegral.h"


/*!
	@brief Brent's Method

    @param[in] a			Lower bound
	@param[in] b			Upper bound
	@param[in] num_srch	    Maximum number of iteration
    @param[in] tol          Tolerance

    @return Minimum x
*/
double LAFunction::SolveBR( double x1,
                            double x2,
                            size_t num_srch,
                            double tol
                          )
{
        //
        LAFindRootBrent BR;
        LAFindRootOption& option = BR.getOption();
        option.mIterMax = num_srch;
        option.mPrecision = tol;

        std::vector<std::pair<double,double> > x;
        x.push_back( std::pair<double,double>(x1, x2) );

        DoubleArray ret;
        BR.findRoot(*this, x, ret);

        return ret[0];
}

DoubleArray LAFunction::SolveBR2( const DoubleArray& low,
                                  const DoubleArray& high,
                                  const DoubleArray& initial,
                                  size_t num_srch,
                                  double tol
                                  )
{
        size_t n = initial.size();
        //error check
        if( low.size() != n || high.size() != n )
        {
            LAString msg("sizes are not equall!");
            throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
        }
        //
        LAOptimumBFGS bfgs;
        LAOptimumOption& option = bfgs.getOption();
        option.mIterMax = num_srch;
        option.mPrecision = tol;
        option.mException = false;
		option.mBoundary = true;
        option.mBoundLow = low;
		option.mBoundHigh = high;

        DoubleArray ret(n);
        for(size_t i=0;i<n;i++){
            ret[i] = initial[i];
        }
            
        bfgs.findMinimum(*this,ret);
        return ret;
}

/*!
	@brief Gauss-Legendre Integration

    @param[in] a			Integral start
	@param[in] b			Integral end
	@param[in] n	        Num integral points

    @return Integrated value
*/
double LAFunction::IntegrateGLegendre(double a, double b, size_t n) const
{
    if ( a == b ) return 0;

    if ( b < a ) return -IntegrateGLegendre(b, a, n);

	LAGaussLegendre GQ(static_cast<int>(n));
 
    return GQ.integrate(*this, a, b);
}


/*!
	@brief PC Integration

    @param[in] a			Integral start
	@param[in] b			Integral end
	@param[in] n	        Num integral points

    @return Integrated value
*/
double LAFunction::Integrate_GL_piecewise(double a, double b, const vector<double>& x, size_t n) const
{
//    if ( a == b ) return 0;
    if ( eq(a,b) ) return 0;

    if ( b < a ) return -Integrate_GL_piecewise(b, a, x, n);
 
    double ret;
    size_t i_s;
    size_t i_e;
    
    if ( LAMath::abs(a - x[0]) < 0.000001 )
    {
        i_s = 0;
    }
    else
    {
        i_s = upper_bound( x.begin(), x.end(), a ) - x.begin();
        if ( LAMath::abs(a - x[i_s - 1]) < 0.000001 || x.size() <= i_s )
        {
            i_s--;
        }
    }

    if ( LAMath::abs(b - x[0]) < 0.000001 )
    {
        i_e = 0;
    }
    else
    {
        i_e = upper_bound( x.begin(), x.end(), b ) - x.begin();
        if ( LAMath::abs(b - x[i_e - 1]) < 0.000001 || x.size() <= i_e )
        {
            i_e--;
        }
    }

    if ( i_s == i_e)
    {
        ret = IntegrateGLegendre(a, b, n);
    }
    else 
    {
        ret = IntegrateGLegendre(a, x[i_s], n);
        for ( size_t j = i_s; j < i_e - 1; j++)
        {
            ret += IntegrateGLegendre(x[j], x[j+1], n);
        }
        ret += IntegrateGLegendre(x[i_e - 1], b, n);
    }

    return ret;
}

//
double LAFunction::Integrate_Array(double a, double b, const vector<double>& x, const vector<double>& y ) const
{
//    if ( a == b ) return 0;
    if ( eq(a,b) ) return 0;

    if ( b < a ) return -Integrate_Array( b, a, x, y );
 
    double ret;
    size_t i_s;
    size_t i_e;
    
    if ( LAMath::abs(a - x[0]) < 0.000001 )
    {
        i_s = 0;
    }
    else
    {
        i_s = upper_bound( x.begin(), x.end(), a ) - x.begin();
        if ( LAMath::abs(a - x[i_s - 1]) < 0.000001 || x.size() <= i_s )
        {
            i_s--;
        }
    }

    if ( LAMath::abs(b - x[0]) < 0.000001 )
    {
        i_e = 0;
    }
    else
    {
        i_e = upper_bound( x.begin(), x.end(), b ) - x.begin();
        if ( LAMath::abs(b - x[i_e - 1])<0.000001 )
        {
            i_e--;
        }
    }

    if ( i_s == i_e)
    {
        ret = (b - a) * y[i_e];
    }
    else 
    {
        ret = (x[i_s] - a) * y[i_s];
        for ( size_t j = i_s; j < i_e - 1; j++)
        {
            ret += (x[j+1] - x[j]) * y[j];
        }
        ret += (b - x[i_e - 1]) * y[i_e-1];
    }

    return ret;
}


/*!
	@brief Euler Integration

    @param[in] a			Integral start
	@param[in] b			Integral end
	@param[in] n	        Num integral points

    @return Integrated value
*/
double LAFunction::IntegrateEuler(double a, double b, size_t n) const
{
    if ( a == b ) return 0;

    if ( b < a ) return -IntegrateEuler(b, a, n);

	LAEulerIntegral EI(static_cast<int>(n));
 
    return EI.integrate(*this, a, b);
}

/*!
	@brief Gauss-Legendre Integration

    @param[in] a			Integral start
	@param[in] b			Integral end
	@param[in] n	        Num integral points

    @return Integrated value
*/
double LAFunction::IntegrateGLaguerre(size_t n, double alpha) const
{
	if(n == 144 || n == 180 || n == 192)
	{
		LAGaussLaguerre GQ(static_cast<int>(n));
		return GQ.integrate(*this, 0., 10000000000.);
	}
	else
	{
		QuantLib::GaussLaguerreIntegration GL(n, alpha);
		return GL.operator()(*this);
	}
}

/*!
	@brief Gauss-Lobatto Integration

    @param[in] a			Integral start
	@param[in] b			Integral end
	@param[in] n	        Num integral points

    @return Integrated value
*/
double LAFunction::IntegrateGLobatto(double a, double b, size_t n) const
{
	GaussLobattoIntegral GL(n, 0.000001);
	return GL.operator()(*this, a, b);
}

double LAFunction::Solve1D_Brent()
{
	QuantLib::Brent solver = QuantLib::Brent();
	solver.setMaxEvaluations(10000);
	return solver.solve(*this, 0.001, 1., 0.001);
}
