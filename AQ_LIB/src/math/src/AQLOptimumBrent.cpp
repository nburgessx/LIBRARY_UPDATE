/*! @file
    @brief Class definition for a 1-D function optimization(minimization) with Brent method. 

    This class realize virtual method in abstract base class.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLOptimumBrent.h"
#include "AQLFunctionBase.h"
#include "AQLBasic.h"
#include <limits>

using namespace std;

//#define DEFAULT_REGION_MAX	numeric_limits<double>::max()
//#define DEFAULT_REGION_MIN	-numeric_limits<double>::max()


//================ AQLOptimumBrent ===================================
/*!
	@brief Default constructor
*/
AQLOptimumBrent::AQLOptimumBrent()
: AQLOptimumBase()/*, mRegion_max(DEFAULT_REGION_MAX), mRegion_min(DEFAULT_REGION_MIN)*/
{
    mControlRange = false;
}

AQLOptimumBrent::AQLOptimumBrent(double initPoint, double lwBound, double upBound, unsigned int maxIter, double tol)
    : AQLOptimumBase()
{
    mControlRange = true;
    mInitPoint = initPoint;
    mLwBound = lwBound;
    mUpBound = upBound;
    mOption.mIterMax = maxIter;
    mOption.mPrecision = tol;
}

/*!
	@brief Destructor
*/
AQLOptimumBrent::~AQLOptimumBrent() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLOptimumBrent::clone() const
{
    try 
	{
        return new AQLOptimumBrent();
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
AQLOptimumBrent::isTypeOf(function_t id) const
{
    return (id == FN_OPTIMUMBRENT ? true : AQLOptimumBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
AQLOptimumBrent::getType() const
{
    return FN_OPTIMUMBRENT;  
}

/*!
    @brief  Optimization(minimization) function by Brent method.

    @param[in] f target function
	@param[in,out] x starting point and minimum point
*/
double
AQLOptimumBrent::findMinimum(const AQLFunctionBase& f,
							DoubleArray& x) const
{
////	double ax = mRegion_min;
////	double bx = mRegion_max;
//	double ax = x[0];
//	double bx = ax + mOption.mWidth;
//	double cx;
//	mnbrak(ax, bx, cx, f);

    double ax, bx, cx;
    if (mControlRange)
    {
        ax = mLwBound;
        bx = mInitPoint;
        cx = mUpBound;
    }
    else
    {
        ax = x[0];
        bx = ax + mOption.mWidth;
        mnbrak(ax, bx, cx, f);
    }

	const double CGOLD = 2 - AQLMath::goldenValue();			//Golden Ratio

	unsigned int iter;
	double a, b, d = 0.0, etemp, fu, fv, fw, fx;
	double p, q, r, tol1, tol2, xu, xv, xw, xx, xm;
	double e = 0.0;

	a = (ax < cx ? ax : cx);
	b = (ax > cx ? ax : cx);
	
	xx = xw = xv = bx;
	fw = fv = fx = f(xx);
	for (iter = 0; iter < mOption.mIterMax; iter++)
	{
		xm = 0.5 * (a+b);
		tol2 = 2.0 * (tol1=mOption.mPrecision*AQLMath::abs(xx) + mOption.mZeps);
		if ( AQLMath::abs(xx-xm) <= (tol2-0.5*(b-a)) )
		{
			x[0] = xx;
			return fx;
		}
		if (AQLMath::abs(e) > tol1)
		{
			r = (xx-xw)*(fx-fv);
			q = (xx-xv)*(fx-fw);
			p = (xx-xv)*q-(xx-xw)*r;
			q = 2.0*(q-r);
			if (q > 0.0) p = -p;
			q = AQLMath::abs(q);
			etemp = e;
			e = d;
			if (AQLMath::abs(p) >= AQLMath::abs(0.5*q*etemp) || p <= q * (a-xx) || p >= q * (b-xx))
				d = CGOLD * (e = (xx >= xm ? a-xx : b-xx));
			else
			{
				d = p / q;
				xu = xx + d;
				if (xu-a < tol2 || b-xu < tol2)
					d = AQLMath::sign(tol1, xm-xx);
			}
		}
		else
		{
			d = CGOLD*(e = (xx >= xm ? a-xx : b-xx));
		}
		xu = (AQLMath::abs(d) >= tol1 ? xx+d : xx+AQLMath::sign(tol1,d));
		fu = f(xu);
		if (fu <= fx)
		{
			if (xu >= xx) a = xx; else b = xx;
			shft3(xv, xw, xx, xu);
			shft3(fv, fw, fx, fu);
		}
		else
		{
			if (xu < xx) a = xu; else b = xu;
			if (fu <= fw || xw == xx)
			{
				xv = xw;
				xw = xu;
				fv = fw;
				fw = fu;
			}
			else if (fu<=fv || xv==xx || xv==xw)
			{
				xv = xu;
				fv = fu;
			}
		}
	}
    x[0] = xx; //20070710--S.Nakatani
	
	if (iter >= mOption.mIterMax && mOption.mException)
	{
		throw AQLCoreInvalidData("too many iterations in AQLOptimumBrent::findMinimum", __FILE__, __LINE__);
	}
	return 0;

	//const double CGOLD = AQLMath::goldenValue();			//Golden Ratio

	//unsigned int iter;
	//double x0;
	//double a, b, d=0.0, etemp, fu, fv, fw, fx;
	//double p, q, r, tol1, tol2, xu, xv, xw, xx, xm;
	//double e=0.0;

	//a = mRegion_min;
	//b = mRegion_max;
	//mnbrak(a, b, x0, f);
	//
	//xx = xw = xv = x0;
	//fw = fv = fx = f(x0);
	//for (iter=0; iter<mOption.mIterMax; iter++){
	//	xm = 0.5 * (a+b);
	//	tol2 = 2.0 * (tol1=mOption.mPrecision*AQLMath::abs(xx) + mOption.mZeps);
	//	if (AQLMath::abs(xx-xm)<=(tol2-0.5*(b-a))){
	//		x[0] = xx;
	//		return fx;
	//	}
	//	if (AQLMath::abs(e)>tol1){
	//		r = (xx-xw)*(fx-fv);
	//		q = (xx-xv)*(fx-fw);
	//		p = (xx-xv)*q-(xx-xw)*r;
	//		q = 2.0*(q-r);
	//		if (q>0.0) p=-p;
	//		q = AQLMath::abs(q);
	//		etemp = e;
	//		e = d;
	//		if (AQLMath::abs(p)>=AQLMath::abs(0.5*q*etemp) || p<=q*(a-xx) || p>= q*(b-xx))
	//			d = CGOLD*(e=(xx>=xm ? a-xx : b-xx));
	//		else{
	//			d = p/q;
	//			xu = xx+d;
	//			if (xu-a<tol2 || b-xu<tol2)
	//				d = AQLMath::sign(tol1, xm-xx);
	//		}
	//	}
	//	else{
	//		d = CGOLD*(e=(xx>=xm ? a-xx : b-xx));
	//	}
	//	xu = (AQLMath::abs(d)>=tol1 ? xx+d : xx+AQLMath::sign(tol1,d));
	//	fu = f(xu);
	//	if (fu <= fx){
	//		if (xu>=xx) a=xx; else b=xx;
	//		shft3(xv,xw,xx,xu);
	//		shft3(fv,fw,fx,fu);
	//	}
	//	else{
	//		if (xu<xx) a=xu; else b=xu;
	//		if (fu<=fw || xw==xx){
	//			xv = xw;
	//			xw = xu;
	//			fv = fw;
	//			fw = fu;
	//		}
	//		else if (fu<=fv || xv==xx || xv==xw){
	//			xv = xu;
	//			fv = fu;
	//		}
	//	}
	//}
	//throw AQLCoreInvalidData("too many iterations in AQLOptimumBrent::findMinimum", __FILE__, __LINE__);

	//x[0] = xx;
	//return fx;

}

/*!
    @brief  select suitable initial points for Brent method

    @param[in] a minimum boundary of region
	@param[in] b maximum boundary of region
	@param[out] c suitable mid point
	@param[in] f target function
*/
void
AQLOptimumBrent::mnbrak(double &a, double &b, double &c, const AQLFunctionBase &f) const
{
	const double GOLD = AQLMath::goldenValue();
	const double GLIMIT = 100.0;
	const double TINY = 1.0e-20;
	
	double ulim, u, r, q, fu;

	double fa = f(a);
	double fb = f(b);
	if (fb > fa)
	{
		AQLMath::swap(a,b);
		AQLMath::swap(fb,fa);
	}
	c = b + GOLD * (b-a);
	double fc = f(c);
	while (fb > fc)
	{
		r = (b-a) * (fb-fc);
		q = (b-c) * (fb-fa);
		u = b - ((b-c) * q - (b-a) * r)
			/ (2.0 * AQLMath::sign(AQLMath::max(AQLMath::abs(q-r),TINY), q-r));
		ulim = b + GLIMIT * (c-b);
		if ((b-u) * (u-c) > 0.0)
		{
			fu = f(u);
			if (fu < fc)
			{
				a = b;
				b = u;
				fa = fb;
				fb = fu;
				return;
			}
			else if (fu>fb){
				c = u;
				fc = fu;
				return;
			}
			u = c + GOLD * (c-b);
			fu = f(u);
		}
		else if ((c-u) * (u-ulim) > 0.0)
		{
			fu = f(u);
			if (fu<fc)
			{
				shft3(b, c, u, u + GOLD * (u-c));
				shft3(fb, fc, fu, f(u));
			}
		}
		else if ((u-ulim) * (ulim-c) >= 0.0)
		{
			u = ulim;
			fu = f(u);
		}
		else
		{
			u = c + GOLD * (c-b);
			fu = f(u);
		}
		shft3(a, b, c, u);
		shft3(fa, fb, fc, fu);
	}
}

/*!
    @brief  set search regioin for finding minimum.

    @param[in] a area boundary
	@param[in] b area boundary
*/
/*void
AQLOptimumBrent::setRegion(const double a, const double b)
{
	if (a < b) { mRegion_min = a; mRegion_max = b;}
	else { mRegion_min = b; mRegion_max = a;}
}
*/
