/*! @file
    @brief Class definition for function optimization(minimization) by BFGS method. 

    This class realize virtual method in abstract base class.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLOptimumBFGS.h"
#include "AQLFunctionBase.h"
#include "AQLBasic.h"
using namespace std;

//================ AQLOptimumBFGS ===================================
/*!
	@brief Default constructor
*/
AQLOptimumBFGS::AQLOptimumBFGS()
: AQLOptimumBase()
{
}

/*!
	@brief Destructor
*/
AQLOptimumBFGS::~AQLOptimumBFGS() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLOptimumBFGS::clone() const
{
    try 
	{
        return new AQLOptimumBFGS();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLOptimumBFGS::isTypeOf(function_t id) const
{
    return (id == FN_OPTIMUMBFGS ? true : AQLOptimumBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
AQLOptimumBFGS::getType() const
{
    return FN_OPTIMUMBFGS;  
}

/*!
    @brief  Optimization(Minimization) function by BFGS method.

    @param[in] f target function
	@param[in,out] x starting point and minimum point
*/
double
AQLOptimumBFGS::findMinimum(const AQLFunctionBase& f,
								DoubleArray& x) const
{
	double EPS = AQLMath::epsValue(EPS);

	// temporary parameters
	unsigned int iter,its;
	double fret;
	bool check;
	int i,j;
	double den, fac, fad, fae, fp, stpmax, sum=0, sumdg, sumxi, temp, test;

	int n = x.size();
	DoubleArray dg(n), g(n), hdg(n), pnew(n), xi(n);
	DoubleMatrix hessin;
	hessin.resize(n);
	for (i=0; i<n; i++) hessin[i].resize(n);

	fp = f(x);
	f.gradient(x,g);

	for (i=0; i<n; i++)
	{
		for (j=0; j<n; j++) hessin[i][j] = 0.0;
		hessin[i][i] = 1.0;
		xi[i] = -g[i];
		sum += x[i]*x[i];
	}
	stpmax = mOption.mStepMax*AQLMath::max(AQLMath::sqrt(sum),double(n));
	for (its=0; its<mOption.mIterMax; its++)
	{
		iter = its;
		lnsrch(x, fp, g, xi, pnew, fret, stpmax, check, f);
		fp = fret;
		for (i = 0; i < n; i++) xi[i] = pnew[i] - x[i];
		for (i = 0; i < n; i++) x[i] = pnew[i];
		test = 0;
		for (i = 0; i < n; i++)
		{
			temp = AQLMath::abs(xi[i]) / AQLMath::max(AQLMath::abs(x[i]),1.0);
			if (temp > test) test = temp;
		}
		if (test < mOption.mPrecision) return fret;
		for(i = 0; i < n; i++) dg[i] = g[i];
		f.gradient(x,g);
		test = 0;
		den = AQLMath::max(fret, 1.0);
		for (i=0; i<n; i++)
		{
			temp = AQLMath::abs(g[i])*AQLMath::max(AQLMath::abs(x[i]),1.0) / den;
			if (temp > test) test = temp;
		}
		if (test < mOption.mGTol) return fret;
		for (i = 0; i < n; i++) dg[i] = g[i] - dg[i];
		for (i = 0; i < n; i++)
		{
			hdg[i] = 0;
			for (j = 0; j < n; j++) hdg[i] += hessin[i][j] * dg[j];
		}
		fac = fae = sumdg = sumxi = 0;
		for (i = 0; i < n; i++)
		{
			fac += dg[i] * xi[i];
			fae += dg[i] * hdg[i];
			sumdg += AQLMath::sqr(dg[i]);
			sumxi += AQLMath::sqr(xi[i]);
		}
		if (fac > AQLMath::sqrt(EPS*sumdg*sumxi))
		{
			fac = 1.0 / fac;
			fad = 1.0 / fae;
			for (i = 0; i < n; i++)  dg[i] = fac * xi[i] - fad * hdg[i];
			for (i = 0; i < n; i++)
			{
				for (j=i; j<n; j++)
				{
					hessin[i][j] += fac*xi[i] * xi[j]-fad * hdg[i] * hdg[j] + fae * dg[i] * dg[j];
					hessin[j][i] = hessin[i][j];
				}
			}
		}
		for (i=0; i<n; i++)
		{
			xi[i] = 0.0;
			for (j=0; j<n; j++) xi[i] -= hessin[i][j]*g[j];
		}		
	}
	
	if (its >= mOption.mIterMax && mOption.mException)
	{
		throw AQLCoreInvalidData("too many iterations in AQLOptimumBFGS::findMinimum", __FILE__, __LINE__);
	}
	return 0;
}

/*!
    @brief  inner function for line search

	@param[in] xold old value of points
	@param[in] fold old value of function
	@param[in] g gradient at old points
	@param[in,out] p direction at old points
	@param[out] x new points
	@param[out] f new function value
	@param[in] stpmax limitation of search area
	@param[out] check true when points are too close to new points
	@param[in] method target function
*/
void
AQLOptimumBFGS::lnsrch(DoubleArray &xold,
						const double fold,
						DoubleArray &g,
						DoubleArray &p,
						DoubleArray &x,
						double &f,
						const double stpmax,
						bool &check,
						const AQLFunctionBase &method
						) const
{
	int i;
	double a, alam,alam2=0, alamin, b, disc, f2=0;
	double rhs1, rhs2, slope, sum, temp, test, tmplam;
	bool issmall;
	double alamsmall;
	const double EPSL = 1E-7;

	int n = xold.size();
	check = false;
	sum = 0;
	for (i = 0; i < n; i++) sum += p[i] * p[i];
	sum = AQLMath::sqrt(sum);
	if (sum > stpmax) for (i = 0; i < n; i++) p[i] *= stpmax / sum;
	slope = 0;
	for (i = 0; i < n; i++) slope += g[i] * p[i];
	if (slope >= 0.0)
		throw AQLCoreInvalidData("Round Off Problem in AQLOptimumBFGS::lnsrch", __FILE__, __LINE__);
	test = 0;
	for (i=0; i<n; i++)
	{
		temp = AQLMath::abs(p[i]) / AQLMath::max(AQLMath::abs(xold[i]),1.0);
		if (temp > test) test = temp;
	}
	alamin = mOption.mPrecision/test;
	alam = 1.0;
	alamsmall = alam;

    double projection_alamsmall = 1.0;
  	for ( ; ; )
	{
		for (i = 0; i < n; i++) x[i] = xold[i] + alam * p[i];

		unsigned int counter_boundary = 0;
		while (mOption.mBoundary)
		{
			++counter_boundary;

			if (counter_boundary > 3)
			{
				for (i = 0; i < n; i++)
				{
					if (x[i] < mOption.mBoundLow[i])
					{
						x[i] = mOption.mBoundLow[i] + EPSL;
					}

					if (x[i] > mOption.mBoundHigh[i])
					{
						x[i] = mOption.mBoundHigh[i] - EPSL;
					}
				}
				break;
			}
			else
			{
				issmall = false;
				for (i = 0; i < n; i++)
				{
					if (x[i] < mOption.mBoundLow[i] || x[i] > mOption.mBoundHigh[i])
					{
						issmall=true;
						break;
					}
				}
				if (issmall)
				{
					alamsmall*=0.5;
					for (i = 0; i < n; i++) x[i] = xold[i] + alamsmall * p[i];
					if (alamsmall < EPSL)
					{
						// projection method
						int orderOnBorder;
						double lowDiff, highDiff, minDiff, minValue = 100;
						for (i = 0; i < n; i++)
						{
							lowDiff = AQLMath::abs(x[i] - mOption.mBoundLow[i]);
							highDiff = AQLMath::abs(x[i] - mOption.mBoundHigh[i]);
							minDiff = min(lowDiff, highDiff);
							minValue = min(minValue, minDiff);
							if (minDiff == minValue)  orderOnBorder = i;
						}
						for (i = 0; i < n; i++)
						{
							if (orderOnBorder != i)
							{
								alamsmall = 1.0;
								double tmp_projection_alamsmall = projection_alamsmall * 0.5;
								x[i] = xold[i] + tmp_projection_alamsmall * p[i];
							}
							if (projection_alamsmall < EPSL)
							{
								f=method(x);
								return;
							}
						}
						projection_alamsmall*=0.5;
						continue;
					}
					continue;
				}
				break;
			}
		}

		f = method(x);
		if (alam < alamin)
		{
			x = xold;
			check = true;
			return;
		}
		else if ( f<= fold + mOption.mAlf * alam * slope) return;
		else
		{
			if (alam == 1.0) tmplam = -slope / (2.0 * (f-fold-slope) );
			else
			{
				rhs1 = f-fold-alam*slope;
				rhs2 = f2-fold-alam2*slope;
				a = (rhs1/(alam*alam)-rhs2/(alam2*alam2))/(alam-alam2);
				b = (-alam2*rhs1/(alam*alam)+alam*rhs2/(alam2*alam2))/(alam-alam2);
				if (a == 0.0) tmplam = -slope/(2.0*b);
				else
				{
					disc = b*b-3.0 * a * slope;
					if (disc < 0.0) tmplam = 0.5 * alam;
					else if (b <= 0.0) tmplam = (-b + AQLMath::sqrt(disc)) / (3.0*a);
					else tmplam = -slope / (b+AQLMath::sqrt(disc));
				}
				if (tmplam > 0.5 * alam) tmplam = 0.5 * alam;
			}
		}
		alam2 = alam;
		f2 = f;
		alam = AQLMath::max(tmplam, 0.1*alam);
	}
}
				

