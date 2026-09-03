/*! @file
    @brief Class definition for dataInstance finding of a 1-D function with Brent method. 

    This class realize virtual method in abstract base class.

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLFindRootBrent.cpp
//
//  SYNOPSIS    :       AQLFindRootBrent
//  DESCRIPTION :       Class definition for dataInstance finding of a 1-D function with Brent method.
//						This class realize virtual method in abstract base class.
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLFindRootBrent.h"
#include "AQLFunctionBase.h"
#include "AQLBasic.h"

using namespace std;
//================ AQLFindRootBrent ===================================
/*!
	@brief Default constructor
*/
AQLFindRootBrent::AQLFindRootBrent()
: AQLFindRootBase()
{
}

/*!
	@brief Destructor
*/
AQLFindRootBrent::~AQLFindRootBrent() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLFindRootBrent::clone() const
{
    try 
	{
        return new AQLFindRootBrent();
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
AQLFindRootBrent::isTypeOf(function_t id) const
{
    return (id == FN_FINDROOTBRENT ? true : AQLFindRootBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
AQLFindRootBrent::getType() const
{
    return FN_FINDROOTBRENT;  
}

/*!
    @brief  DataInstance finding function by Brent method.

    @param[in] f target function
	@param[in] x search region for arguments of target function.
	@param[out] out arguments which realize the minimum function value.
*/
void
AQLFindRootBrent::findRoot(const AQLFunctionBase& f,
						const std::vector<std::pair<double,double> >& x,
						DoubleArray& out)const
{
	unsigned int iter;
	double a = x[0].first, b = x[0].second, c = x[0].second;
	double d = 0, e = 0, min1, min2;
	double fa = f(a), fb = f(b), fc, p, q, r, s, tol1, xm;
	const double EPS = numeric_limits<double>::epsilon();

	out.resize(1);

	//try
	//{
	//	if ( (fa > 0.0 && fb > 0.0) || (fa < 0.0 && fb < 0.0) )
	//		throw AQLCoreNumericalError("Wrong Specified Region in AQLFindRootBrent::findRoot",
	//								__FILE__, __LINE__);
	//}
	//catch(AQLCoreError &err)
	//{
	bool isCorrectRegion = false;
	for(unsigned int i = 0; i < 6; ++i)
	{
		if ( (fa > 0.0 && fb > 0.0) || (fa < 0.0 && fb < 0.0) )
		{
			a = a * 1.2;
			b = b * 1.2;
			fa = f(a), fb = f(b);
		}
		else 
		{
			isCorrectRegion = true;
			break;
		}
	}
	//}
	if ( /*(fa > 0.0 && fb > 0.0) || (fa < 0.0 && fb < 0.0)*/!isCorrectRegion )
		throw AQLCoreNumericalError("Wrong Specified Region in AQLFindRootBrent::findRoot",
								__FILE__, __LINE__);
	fc = fb;
	for (iter = 0; iter < mOption.mIterMax; iter++)
	{
		if ( (fb > 0.0 && fc > 0.0) || (fb < 0.0 && fc < 0.0) )
		{
			c = a;
			fc = fa;
			e = d = b-a;
		}
		if (AQLMath::abs(fc) < AQLMath::abs(fb))
		{
			a = b;
			b = c;
			c = a;
			fa = fb;
			fb = fc;
			fc = fa;
		}
		tol1 = 2.0 * EPS * AQLMath::abs(b) + 0.5 * mOption.mPrecision;
		xm = 0.5 * (c-b);
		if (AQLMath::abs(xm) <= tol1 || fb == 0.0)
		{
			out[0] = b;
			break;
		}
		if (AQLMath::abs(e) >= tol1 && AQLMath::abs(fa) > AQLMath::abs(fb))
		{
			s = fb / fa;
			if (a == c)
			{
				p = 2.0 * xm * s;
				q = 1.0 - s;
			}
			else
			{
				q = fa / fc;
				r = fb / fc;
				p = s * (2.0 * xm * q * (q-r) - (b-a) * (r-1.0));
				q = (q-1.0) * (r-1.0) * (s-1.0);
			}
			if (p > 0.0) q = -q;
			p = AQLMath::abs(p);
			min1 = 3.0 * xm * q - AQLMath::abs(tol1 * q);
			min2 = AQLMath::abs(e * q);
			if (2.0 * p < (min1 < min2 ? min1 : min2))
			{
				e = d;
				d = p / q;
			}
			else
			{
				d = xm;
				e = d;
			}
		}
		else
		{
			d = xm;
			e = d;
		}
		a = b;
		fa = fb;
		if (AQLMath::abs(d) > tol1) b += d;
		else b += AQLMath::sign(tol1,xm);
		fb = f(b);
	}
	if (iter == mOption.mIterMax)
	{
		throw AQLCoreNumericalError("Too many iterations in AQLOptimumBrent::findMinimum",
								__FILE__, __LINE__);
	}
}

